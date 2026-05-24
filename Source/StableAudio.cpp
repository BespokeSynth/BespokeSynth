/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  StableAudio.cpp
//  StableAudioSynth
//
//  Created by David Horner on 5/24/26.
//
//

#include "StableAudio.h"
#include "FileStream.h"
#include "IAudioReceiver.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

#if BESPOKE_STABLE_AUDIO_ENABLED
#ifdef __cplusplus
extern "C" {
#endif
#include "stable_audio.h"
#ifdef __cplusplus
}
#endif
#endif

#include "juce_core/juce_core.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <random>
#include <vector>

StableAudio::StableAudio()
: IAudioProcessor(gBufferSize)
, IDrawableModule(620, 180)
{
#if defined(BESPOKE_STABLE_AUDIO_DEFAULT_MODEL_DIR)
   mModelDir = BESPOKE_STABLE_AUDIO_DEFAULT_MODEL_DIR;
   std::replace(mModelDir.begin(), mModelDir.end(), '\\', '/');
   if (ModelFilesExist("sa3-medium-dit.gguf", "sa3-medium-same-l-decoder.gguf"))
      mModelSelection = kModel_Medium;
   else if (ModelFilesExist("sa3-small-music-dit.gguf", "sa3-small-music-same-s-decoder.gguf"))
      mModelSelection = kModel_SmallMusic;
   else if (ModelFilesExist("sa3-small-sfx-dit.gguf", "sa3-same-s-decoder.gguf"))
      mModelSelection = kModel_SmallSfx;
   ApplyModelSelection();
#endif
}

StableAudio::~StableAudio()
{
   if (mGenerationFuture.valid())
      mGenerationFuture.wait();

   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      delete mSample;
      delete mPreviousSample;
      mSample = nullptr;
      mPreviousSample = nullptr;
   }
   FreeModel();
}

void StableAudio::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mGeneratedWavDropdown, "wav", &mGeneratedWavIndex, 240);
   mGeneratedWavDropdown->DrawLabel(true);
   mGeneratedWavDropdown->SetUnknownItemString("no generated wavs");
   RefreshGeneratedWavList();
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mLoadWavButton, "load");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mDeleteWavButton, "delete");
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mUseMetadataWavLabelsCheckbox, "details", &mUseMetadataWavLabels);
   UIBLOCK_NEWLINE();
   TEXTENTRY(mPromptEntry, "prompt", 64, &mPrompt);
   mPromptEntry->DrawLabel(true);
   UIBLOCK_NEWLINE();
   DROPDOWN(mPromptDropdown, "ideas", &mPromptChoice, 300);
   mPromptDropdown->DrawLabel(true);
   mPromptDropdown->SetUnknownItemString("choose prompt");
   RefreshPromptChoices();
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mMoreIdeasButton, "more ideas...");
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mAutoplayCheckbox, "autoplay", &mAutoplay);
   UIBLOCK_NEWLINE();
   DROPDOWN(mModelDropdown, "model", &mModelSelection, 120);
   mModelDropdown->DrawLabel(true);
   AddAvailableModelLabels();
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mGenerateButton, "generate");
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mPlayButton, "play", ButtonDisplayStyle::kPlay);
   UIBLOCK_SHIFTRIGHT();
   BUTTON_STYLE(mStopButton, "stop", ButtonDisplayStyle::kStop);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLoopCheckbox, "loop", &mLoop);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mVolumeSlider, "volume", &mVolume, 0, 2);
   UIBLOCK_NEWLINE();

   FLOATSLIDER_DIGITS(mSecondsSlider, "duration", &mSeconds, 1, GetSelectedModelMaxSeconds(), 2);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mStepsSlider, "steps", &mSteps, 1, 250);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mSeedSlider, "seed", &mSeed, 0, INT_MAX);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mTransitionDropdown, "transition", &mTransitionMode, 90);
   mTransitionDropdown->DrawLabel(true);
   mTransitionDropdown->AddLabel("normal", kTransition_Normal);
   mTransitionDropdown->AddLabel("crossfade", kTransition_Crossfade);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER_DIGITS(mCrossfadeSlider, "xfade", &mCrossfadeSeconds, .25f, GetMaxCrossfadeSeconds(), 2);
   UpdateCrossfadeSlider();
   UIBLOCK_NEWLINE();
   TEXTENTRY(mDitPathEntry, "dit", 64, &mDitPath);
   mDitPathEntry->DrawLabel(true);
   UIBLOCK_NEWLINE();
   TEXTENTRY(mDecoderPathEntry, "decoder", 64, &mDecoderPath);
   mDecoderPathEntry->DrawLabel(true);
   UIBLOCK_NEWLINE();
   TEXTENTRY(mTextEncoderPathEntry, "textenc", 64, &mTextEncoderPath);
   mTextEncoderPathEntry->DrawLabel(true);
   ENDUIBLOCK0();

   UpdatePlaybackControls();
}

void StableAudio::Init()
{
   IDrawableModule::Init();
}

void StableAudio::Poll()
{
   IDrawableModule::Poll();

   if (mGenerationInProgress && mGenerationFuture.valid() &&
       mGenerationFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
   {
      GenerationResult result = mGenerationFuture.get();
      mGenerationInProgress = false;

      if (result.success)
      {
         WriteGeneratedWavMetadata(result);
         LoadGeneratedSample(result.outputPath);
         RefreshGeneratedWavList();
         RefreshPromptChoices();
         ScheduleNextAutoplay();
         mStatusString = "generated " + juce::File(result.outputPath).getFileName().toStdString();
      }
      else
      {
         mStatusString = result.error;
         TheSynth->LogEvent("stableaudio: " + result.error, kLogEventType_Error);
      }
   }

   bool hasSample = false;
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      hasSample = mSample != nullptr;
   }

   if (mAutoplay && !mGenerationInProgress && hasSample &&
       mAutoplayNextGenerationTime > 0 && gTime >= mAutoplayNextGenerationTime)
   {
      AutoplayNextPrompt();
   }
}

void StableAudio::Process(double time)
{
   PROFILER(StableAudio);

   IAudioReceiver* target = GetTarget();
   if (mEnabled && target != nullptr)
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      if (mSample == nullptr)
      {
         GetBuffer()->Reset();
         return;
      }

      ComputeSliders(0);
      const int numChannels = mPreviousSample != nullptr ? MAX(mSample->NumChannels(), mPreviousSample->NumChannels()) : mSample->NumChannels();
      SyncBuffers(numChannels);

      const int bufferSize = target->GetBuffer()->BufferSize();
      gWorkChannelBuffer.SetNumActiveChannels(numChannels);

      if (mPlay)
      {
         mSample->SetLooping(mLoop);
         if (mSample->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
         {
            if (mPreviousSample != nullptr)
            {
               const double crossfadeMs = std::clamp(mCrossfadeSeconds, .25f, GetMaxCrossfadeSeconds()) * 1000.0;
               mPreviousSample->SetLooping(true);
               mCrossfadeBuffer.SetNumActiveChannels(numChannels);
               mCrossfadeBuffer.Clear();
               const bool previousSamplePlaying = mPreviousSample->ConsumeData(time, &mCrossfadeBuffer, bufferSize, true);

               for (int i = 0; i < bufferSize; ++i)
               {
                  const float fadeIn = ofClamp(float((time + i * gInvSampleRateMs - mCrossfadeStartTime) / crossfadeMs), 0, 1);
                  const float fadeOut = 1 - fadeIn;
                  for (int ch = 0; ch < gWorkChannelBuffer.NumActiveChannels(); ++ch)
                     gWorkChannelBuffer.GetChannel(ch)[i] = gWorkChannelBuffer.GetChannel(ch)[i] * fadeIn + mCrossfadeBuffer.GetChannel(ch)[i] * fadeOut;
               }

               if (!previousSamplePlaying || time + bufferSize * gInvSampleRateMs >= mCrossfadeStartTime + crossfadeMs)
               {
                  delete mPreviousSample;
                  mPreviousSample = nullptr;
                  mCrossfadeStartTime = -1;
               }
            }

            const float volSq = mVolume * mVolume;
            for (int ch = 0; ch < gWorkChannelBuffer.NumActiveChannels(); ++ch)
            {
               for (int i = 0; i < bufferSize; ++i)
                  gWorkChannelBuffer.GetChannel(ch)[i] *= volSq;
            }
         }
         else
         {
            gWorkChannelBuffer.Clear();
            mPlay = false;
            mSample->SetPlayPosition(0);
         }
      }
      else
      {
         gWorkChannelBuffer.Clear();
      }

      for (int ch = 0; ch < gWorkChannelBuffer.NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      }
   }

   GetBuffer()->Reset();
}

void StableAudio::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
      Trigger(note.time, note.velocity / 127.0f);
}

void StableAudio::OnPulse(double time, float velocity, int flags)
{
   Trigger(time, velocity);
}

void StableAudio::Trigger(double time, float velocity)
{
   std::lock_guard<std::mutex> sampleLock(mSampleMutex);
   if (!mEnabled || mSample == nullptr)
      return;

   mSample->SetPlayPosition(0);
   mPlay = true;
}

void StableAudio::ButtonClicked(ClickButton* button, double time)
{
   if (button == mGenerateButton)
      StartGeneration();

   if (button == mLoadWavButton)
      LoadSelectedGeneratedWav();

   if (button == mDeleteWavButton)
      DeleteSelectedGeneratedWav();

   if (button == mMoreIdeasButton)
      GenerateMorePromptIdeas();

   if (button == mPlayButton)
      Trigger(time, 1);

   if (button == mStopButton)
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      delete mPreviousSample;
      mPreviousSample = nullptr;
      mCrossfadeStartTime = -1;
      if (mSample != nullptr)
      {
         mPlay = false;
         mSample->SetPlayPosition(0);
      }
   }
}

void StableAudio::StartGeneration()
{
   if (mGenerationInProgress)
      return;

   if (mPrompt.empty())
   {
      mStatusString = "enter a prompt first";
      return;
   }

   std::string outputPath = BuildOutputPath();
   juce::File(outputPath).getParentDirectory().createDirectory();
   mStatusString = "generating...";
   mGenerationInProgress = true;
   mAutoplayNextGenerationTime = -1;

   std::string modelLabel = GetSelectedModelLabel();
   mGenerationFuture = std::async(std::launch::async, [this, prompt = mPrompt, modelLabel, ditPath = mDitPath, decoderPath = mDecoderPath, textEncoderPath = mTextEncoderPath, seconds = mSeconds, steps = mSteps, seed = mSeed, outputPath]
                                  {
                                     GenerationResult result = GenerateToFile(prompt, ditPath, decoderPath, textEncoderPath, seconds, steps, seed, outputPath);
                                     result.model = modelLabel;
                                     result.prompt = prompt;
                                     result.seconds = seconds;
                                     return result;
                                  });
}

StableAudio::GenerationResult StableAudio::GenerateToFile(std::string prompt, std::string ditPath, std::string decoderPath, std::string textEncoderPath, float seconds, int steps, int seed, std::string outputPath)
{
   GenerationResult result;
   result.outputPath = outputPath;

#if BESPOKE_STABLE_AUDIO_ENABLED
   if (ditPath.empty() || decoderPath.empty() || textEncoderPath.empty())
   {
      result.error = "set dit, decoder, and textenc paths";
      return result;
   }

   if (mModel == nullptr ||
       mLoadedDitPath != ditPath ||
       mLoadedDecoderPath != decoderPath ||
       mLoadedTextEncoderPath != textEncoderPath ||
       mLoadedSteps != steps ||
       mLoadedSeed != seed)
   {
      FreeModel();
      mModel = stable_audio_model_load(ditPath.c_str(), decoderPath.c_str(), textEncoderPath.c_str(), (size_t)steps, (uint64_t)seed);
      if (mModel == nullptr)
      {
         const char* error = stable_audio_last_error();
         result.error = error != nullptr ? error : "couldn't load stableaudio model";
         return result;
      }

      mLoadedDitPath = ditPath;
      mLoadedDecoderPath = decoderPath;
      mLoadedTextEncoderPath = textEncoderPath;
      mLoadedSteps = steps;
      mLoadedSeed = seed;
   }

   int generateResult = stable_audio_generate_wav(mModel, prompt.c_str(), seconds, (size_t)steps, (uint64_t)seed, outputPath.c_str());
   if (generateResult != 0)
   {
      const char* error = stable_audio_last_error();
      result.error = error != nullptr ? error : "stableaudio generation failed";
      return result;
   }

   result.success = true;
#else
   result.error = "BespokeSynth was built without stableaudio support";
#endif

   return result;
}

void StableAudio::LoadGeneratedSample(const std::string& path)
{
   Sample* sample = new Sample();
   if (sample->Read(path.c_str()))
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      delete mPreviousSample;
      mPreviousSample = nullptr;
      mCrossfadeStartTime = -1;

      if (mSample != nullptr && mPlay && mTransitionMode == kTransition_Crossfade)
      {
         mPreviousSample = mSample;
         mCrossfadeStartTime = gTime;
      }
      else
      {
         delete mSample;
      }

      mSample = sample;
      mSample->SetPlayPosition(0);
      mCurrentSamplePath = path;
      mPlay = true;
      UpdatePlaybackControls();
   }
   else
   {
      delete sample;
      mStatusString = "couldn't load generated wav";
   }
}

void StableAudio::FreeModel()
{
#if BESPOKE_STABLE_AUDIO_ENABLED
   if (mModel != nullptr)
   {
      stable_audio_model_free(mModel);
      mModel = nullptr;
   }
#else
   mModel = nullptr;
#endif
}

std::string StableAudio::BuildOutputPath() const
{
   juce::String filename = "stableaudio_" + juce::String(juce::Time::getCurrentTime().toMilliseconds()) + ".wav";
   return GetGeneratedAudioDirectory() + "/" + filename.toStdString();
}

std::string StableAudio::GetGeneratedAudioDirectory() const
{
   return ofToDataPath("stableaudio");
}

void StableAudio::RefreshGeneratedWavList()
{
   if (mGeneratedWavDropdown == nullptr)
      return;

   struct WavFileEntry
   {
      std::string path;
      juce::Time modified;
   };

   std::vector<WavFileEntry> files;
   juce::File directory(GetGeneratedAudioDirectory());
   directory.createDirectory();
   for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*.wav", juce::File::findFiles))
   {
      juce::File file = entry.getFile();
      files.push_back({ file.getFullPathName().toStdString(), file.getLastModificationTime() });
   }

   std::sort(files.begin(), files.end(), [](const WavFileEntry& a, const WavFileEntry& b)
             {
                return a.modified > b.modified;
             });

   std::string selectedPath = mCurrentSamplePath;
   if (selectedPath.empty() && mGeneratedWavIndex >= 0 && mGeneratedWavIndex < (int)mGeneratedWavPaths.size())
      selectedPath = mGeneratedWavPaths[mGeneratedWavIndex];

   mGeneratedWavPaths.clear();
   mGeneratedWavDropdown->Clear();
   mGeneratedWavDropdown->SetUnknownItemString("no generated wavs");
   mGeneratedWavIndex = -1;

   for (size_t i = 0; i < files.size(); ++i)
   {
      juce::File file(files[i].path);
      mGeneratedWavPaths.push_back(files[i].path);
      mGeneratedWavDropdown->AddLabel(GetGeneratedWavLabel(files[i].path), (int)i);
      if (files[i].path == selectedPath)
         mGeneratedWavIndex = (int)i;
   }

   if (mGeneratedWavPaths.empty())
      mGeneratedWavIndex = -1;
   else if (mGeneratedWavIndex < 0 || mGeneratedWavIndex >= (int)mGeneratedWavPaths.size())
      mGeneratedWavIndex = 0;
}

void StableAudio::LoadSelectedGeneratedWav()
{
   if (mGeneratedWavIndex < 0 || mGeneratedWavIndex >= (int)mGeneratedWavPaths.size())
   {
      mStatusString = "no generated wav selected";
      return;
   }

   LoadGeneratedSample(mGeneratedWavPaths[mGeneratedWavIndex]);
   mStatusString = "loaded " + juce::File(mGeneratedWavPaths[mGeneratedWavIndex]).getFileName().toStdString();
}

void StableAudio::DeleteSelectedGeneratedWav()
{
   if (mGeneratedWavIndex < 0 || mGeneratedWavIndex >= (int)mGeneratedWavPaths.size())
   {
      mStatusString = "no generated wav selected";
      return;
   }

   const std::string deletedPath = mGeneratedWavPaths[mGeneratedWavIndex];
   const int deletedIndex = mGeneratedWavIndex;
   juce::File wavFile(deletedPath);
   juce::File metadataFile = wavFile.withFileExtension(".meta");

   if (mCurrentSamplePath == deletedPath)
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      delete mSample;
      delete mPreviousSample;
      mSample = nullptr;
      mPreviousSample = nullptr;
      mCurrentSamplePath.clear();
      mPlay = false;
      mCrossfadeStartTime = -1;
      UpdatePlaybackControls();
   }

   const bool deletedWav = wavFile.deleteFile();
   metadataFile.deleteFile();

   RefreshGeneratedWavList();

   if (deletedWav && !mGeneratedWavPaths.empty())
   {
      mGeneratedWavIndex = std::min(deletedIndex, (int)mGeneratedWavPaths.size() - 1);
      LoadSelectedGeneratedWav();
      mStatusString = "deleted " + wavFile.getFileName().toStdString() + ", loaded " + juce::File(mGeneratedWavPaths[mGeneratedWavIndex]).getFileName().toStdString();
   }
   else
   {
      mStatusString = deletedWav ? "deleted " + wavFile.getFileName().toStdString() : "couldn't delete selected wav";
   }
}

void StableAudio::UpdatePlaybackControls()
{
   const bool hasSample = mSample != nullptr;
   if (mPlayButton != nullptr)
      mPlayButton->SetShowing(hasSample);
   if (mStopButton != nullptr)
      mStopButton->SetShowing(hasSample);
}

void StableAudio::RefreshPromptChoices()
{
   if (mPromptDropdown == nullptr)
      return;

   const std::string selectedPrompt =
   mPromptChoice >= 0 && mPromptChoice < (int)mPromptChoices.size() ? mPromptChoices[mPromptChoice] : "";

   mPromptChoices.clear();
   mPromptDropdown->Clear();
   mPromptDropdown->SetUnknownItemString("choose prompt");
   mPromptChoice = -1;

   AddPromptChoice("lo-fi hip hop beat, warm tape saturation, 90 BPM");
   AddPromptChoice("driving synthwave bassline, analog drums, 124 BPM");
   AddPromptChoice("ambient guitar swells, soft piano, spacious reverb");
   AddPromptChoice("cinematic whoosh impact, metallic tail");
   AddPromptChoice("rain on glass, distant thunder, cozy room tone");
   AddPromptChoice("punchy kick drum fill");

   juce::File directory(GetGeneratedAudioDirectory());
   for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*.meta", juce::File::findFiles))
   {
      juce::StringArray lines;
      entry.getFile().readLines(lines);
      for (auto line : lines)
      {
         if (line.startsWith("prompt="))
         {
            AddPromptChoice(line.fromFirstOccurrenceOf("=", false, false).toStdString());
            break;
         }
      }
   }

   for (int i = 0; i < (int)mPromptChoices.size(); ++i)
   {
      if (mPromptChoices[i] == selectedPrompt)
      {
         mPromptChoice = i;
         break;
      }
   }
}

void StableAudio::GenerateMorePromptIdeas()
{
   if (mPromptDropdown == nullptr)
      return;

   mPromptChoices.clear();
   mPromptDropdown->Clear();
   mPromptDropdown->SetUnknownItemString("choose prompt");
   mPromptChoice = -1;

   for (int i = 0; i < 10; ++i)
      AddPromptChoice(MakeGeneratedPromptIdea());

   mStatusString = "generated more prompt ideas";
}

void StableAudio::AutoplayNextPrompt()
{
   if (!mAutoplay || mGenerationInProgress)
      return;

   GenerateMorePromptIdeas();
   if (mPromptChoices.empty())
      return;

   static std::mt19937 rng{ std::random_device{}() };
   std::uniform_int_distribution<int> promptDist(0, (int)mPromptChoices.size() - 1);
   mPromptChoice = promptDist(rng);
   ApplyPromptChoice();
   StartGeneration();
}

void StableAudio::ScheduleNextAutoplay()
{
   if (!mAutoplay)
   {
      mAutoplayNextGenerationTime = -1;
      return;
   }

   static std::mt19937 rng{ std::random_device{}() };
   float duration = mSeconds;
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      if (mSample != nullptr && mSample->LengthInSamples() > 0)
         duration = mSample->LengthInSamples() / (gSampleRate * mSample->GetSampleRateRatio());
   }

   float latestGenerationStart = duration;
   if (mTransitionMode == kTransition_Crossfade)
      latestGenerationStart = std::max(.1f, duration - std::clamp(mCrossfadeSeconds, .25f, duration * .5f));

   const float earliestGenerationStart = std::min(duration * .5f, latestGenerationStart);
   std::uniform_real_distribution<float> playDurationDist(earliestGenerationStart, latestGenerationStart);
   mAutoplayNextGenerationTime = gTime + playDurationDist(rng) * 1000.0f;
}

std::string StableAudio::MakeGeneratedPromptIdea()
{
   static std::mt19937 rng{ std::random_device{}() };
   const auto pick = [](const auto& values) -> const char*
   {
      std::uniform_int_distribution<int> dist(0, (int)values.size() - 1);
      return values[dist(rng)];
   };

   static const std::array<const char*, 10> genres{
      "dark ambient industrial drone",
      "chiptune funk",
      "lo-fi hip hop",
      "industrial techno",
      "cinematic drone",
      "dream pop synthwave",
      "minimal dub techno",
      "orchestral tension cue",
      "glitchy IDM",
      "cozy field recording ambience"
   };
   static const std::array<const char*, 9> instruments{
      "analog bass, soft pads, vinyl drums",
      "square wave lead, pulse bass, 8-bit drums",
      "distorted synth bass, metallic percussion, noise sweeps",
      "warm electric piano, tape hiss, brushed drums",
      "low strings, processed percussion, sub pulses",
      "granular pads, distant bells, filtered noise",
      "FM plucks, syncopated drums, rubbery bass",
      "muted guitar, upright bass, dusty breakbeat",
      "rain texture, distant thunder, soft room tone"
   };
   static const std::array<const char*, 9> moods{
      "tense",
      "playful",
      "melancholic",
      "hypnotic",
      "euphoric",
      "mysterious",
      "cozy",
      "urgent",
      "dreamy"
   };
   static const std::array<const char*, 8> productionStyles{
      "wide stereo reverb",
      "crunchy 8-bit production",
      "warm tape saturation",
      "clean modern club mix",
      "lo-fi cassette texture",
      "cinematic low-end impact",
      "soft analog compression",
      "glossy digital shimmer"
   };
   static const std::array<const char*, 7> structures{
      "slow evolving intro",
      "short intro into main loop",
      "no intro, immediate groove",
      "sparse opening into heavier middle section",
      "steady pulse with subtle variations",
      "rising tension then clean loop point",
      "gentle bed with occasional melodic accents"
   };
   static const std::array<const char*, 7> uses{
      "loopable game background",
      "menu music",
      "boss fight cue",
      "ambient soundscape",
      "podcast transition",
      "sci-fi UI bed",
      "relaxing focus loop"
   };
   static const std::array<int, 6> bpms{ 70, 90, 110, 124, 128, 140 };
   static const std::array<int, 3> durations{ 30, 45, 60 };

   std::uniform_int_distribution<int> bpmDist(0, (int)bpms.size() - 1);
   std::uniform_int_distribution<int> durationDist(0, (int)durations.size() - 1);

   return std::string(pick(genres)) + ", " +
          std::to_string(bpms[bpmDist(rng)]) + " BPM, " +
          pick(instruments) + ", " +
          pick(moods) + " mood, " +
          pick(productionStyles) + ", " +
          pick(structures) + ", " +
          pick(uses) + ", seamless loop, " +
          std::to_string(durations[durationDist(rng)]) + " seconds";
}

void StableAudio::AddPromptChoice(const std::string& prompt)
{
   if (prompt.empty())
      return;

   if (std::find(mPromptChoices.begin(), mPromptChoices.end(), prompt) != mPromptChoices.end())
      return;

   const int index = (int)mPromptChoices.size();
   mPromptChoices.push_back(prompt);

   juce::String label(prompt);
   label = label.replaceCharacter('\r', ' ').replaceCharacter('\n', ' ');
   if (label.length() > 70)
      label = label.substring(0, 70) + "...";
   mPromptDropdown->AddLabel(label.toStdString(), index);
}

void StableAudio::ApplyPromptChoice()
{
   if (mPromptChoice < 0 || mPromptChoice >= (int)mPromptChoices.size())
      return;

   SetPromptText(mPromptChoices[mPromptChoice]);
}

void StableAudio::SetPromptText(const std::string& prompt)
{
   mPrompt = prompt;
   if (mPromptEntry != nullptr)
   {
      mPromptEntry->SetText(mPrompt);
      mPromptEntry->UpdateDisplayString();
   }
}

std::string StableAudio::GetGeneratedWavLabel(const std::string& path) const
{
   if (mUseMetadataWavLabels)
      return ReadGeneratedWavMetadataLabel(path);

   return juce::File(path).getFileNameWithoutExtension().toStdString();
}

std::string StableAudio::ReadGeneratedWavMetadataLabel(const std::string& path) const
{
   juce::File metadataFile = juce::File(path).withFileExtension(".meta");
   if (!metadataFile.existsAsFile())
      return juce::File(path).getFileNameWithoutExtension().toStdString();

   juce::StringArray lines;
   metadataFile.readLines(lines);

   juce::String model = "unknown";
   juce::String prompt = "metadata missing";
   juce::String seconds = "?s";

   for (auto line : lines)
   {
      if (line.startsWith("model="))
         model = line.fromFirstOccurrenceOf("=", false, false);
      else if (line.startsWith("prompt="))
         prompt = line.fromFirstOccurrenceOf("=", false, false);
      else if (line.startsWith("seconds="))
         seconds = line.fromFirstOccurrenceOf("=", false, false) + "s";
   }

   prompt = prompt.replaceCharacter('\r', ' ').replaceCharacter('\n', ' ');
   if (prompt.length() > 80)
      prompt = prompt.substring(0, 80) + "...";

   return (model + " - " + prompt + " - " + seconds).toStdString();
}

void StableAudio::WriteGeneratedWavMetadata(const GenerationResult& result) const
{
   juce::String prompt(result.prompt);
   prompt = prompt.replaceCharacter('\r', ' ').replaceCharacter('\n', ' ');

   juce::File(result.outputPath)
   .withFileExtension(".meta")
   .replaceWithText("model=" + juce::String(result.model) + "\n" +
                    "prompt=" + prompt + "\n" +
                    "seconds=" + juce::String(result.seconds, 2) + "\n");
}

void StableAudio::TextEntryComplete(TextEntry* entry)
{
}

void StableAudio::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mUseMetadataWavLabelsCheckbox)
      RefreshGeneratedWavList();

   if (checkbox == mAutoplayCheckbox)
   {
      if (mAutoplay)
         AutoplayNextPrompt();
      else
         mAutoplayNextGenerationTime = -1;
   }
}

void StableAudio::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mSecondsSlider)
      UpdateCrossfadeSlider();
   if (slider == mCrossfadeSlider && mAutoplay && mAutoplayNextGenerationTime > 0)
      ScheduleNextAutoplay();
   if (slider == mSecondsSlider && mAutoplay && mAutoplayNextGenerationTime > 0)
      ScheduleNextAutoplay();
}

void StableAudio::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void StableAudio::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mModelDropdown)
      ApplyModelSelection();
   if (list == mPromptDropdown)
      ApplyPromptChoice();
   if (list == mGeneratedWavDropdown)
      LoadSelectedGeneratedWav();
   if (list == mTransitionDropdown && mTransitionMode == kTransition_Normal)
   {
      std::lock_guard<std::mutex> sampleLock(mSampleMutex);
      delete mPreviousSample;
      mPreviousSample = nullptr;
      mCrossfadeStartTime = -1;
   }
   if (list == mTransitionDropdown && mCrossfadeSlider != nullptr)
      UpdateCrossfadeSlider();
   if (list == mTransitionDropdown && mAutoplay && mAutoplayNextGenerationTime > 0)
      ScheduleNextAutoplay();
}

void StableAudio::AddAvailableModelLabels()
{
   if (mModelDropdown == nullptr)
      return;

   if (ModelFilesExist("sa3-small-music-dit.gguf", "sa3-small-music-same-s-decoder.gguf"))
      mModelDropdown->AddLabel("small music", kModel_SmallMusic);
   if (ModelFilesExist("sa3-small-sfx-dit.gguf", "sa3-same-s-decoder.gguf"))
      mModelDropdown->AddLabel("small sfx", kModel_SmallSfx);
   if (ModelFilesExist("sa3-medium-dit.gguf", "sa3-medium-same-l-decoder.gguf"))
      mModelDropdown->AddLabel("medium", kModel_Medium);
}

bool StableAudio::ModelFilesExist(const std::string& ditFilename, const std::string& decoderFilename) const
{
   if (mModelDir.empty())
      return false;

   return juce::File(mModelDir + "/" + ditFilename).existsAsFile() &&
          juce::File(mModelDir + "/" + decoderFilename).existsAsFile() &&
          juce::File(mModelDir + "/t5gemma-b-b-ul2-encoder.gguf").existsAsFile();
}

void StableAudio::ApplyModelSelection()
{
   if (mModelDir.empty())
      return;

   switch (mModelSelection)
   {
      case kModel_SmallSfx:
         mDitPath = mModelDir + "/sa3-small-sfx-dit.gguf";
         mDecoderPath = mModelDir + "/sa3-same-s-decoder.gguf";
         break;
      case kModel_Medium:
         mDitPath = mModelDir + "/sa3-medium-dit.gguf";
         mDecoderPath = mModelDir + "/sa3-medium-same-l-decoder.gguf";
         break;
      case kModel_SmallMusic:
      default:
         mDitPath = mModelDir + "/sa3-small-music-dit.gguf";
         mDecoderPath = mModelDir + "/sa3-small-music-same-s-decoder.gguf";
         break;
   }

   mTextEncoderPath = mModelDir + "/t5gemma-b-b-ul2-encoder.gguf";
   const float maxSeconds = GetSelectedModelMaxSeconds();
   mSeconds = std::clamp(mSeconds, 1.0f, maxSeconds);
   if (mSecondsSlider != nullptr)
      mSecondsSlider->SetExtents(1, maxSeconds);
   UpdateCrossfadeSlider();
   RefreshModelPathEntries();
}

float StableAudio::GetSelectedModelMaxSeconds() const
{
   switch (mModelSelection)
   {
      case kModel_Medium:
         return 380;
      case kModel_SmallSfx:
      case kModel_SmallMusic:
      default:
         return 120;
   }
}

float StableAudio::GetMaxCrossfadeSeconds() const
{
   return std::max(.25f, mSeconds * .5f);
}

void StableAudio::UpdateCrossfadeSlider()
{
   mCrossfadeSeconds = std::clamp(mCrossfadeSeconds, .25f, GetMaxCrossfadeSeconds());
   if (mCrossfadeSlider != nullptr)
   {
      mCrossfadeSlider->SetExtents(.25f, GetMaxCrossfadeSeconds());
      mCrossfadeSlider->SetShowing(mTransitionMode == kTransition_Crossfade);
   }
}

const char* StableAudio::GetSelectedModelLabel() const
{
   switch (mModelSelection)
   {
      case kModel_SmallSfx:
         return "small-sfx";
      case kModel_Medium:
         return "medium";
      case kModel_SmallMusic:
      default:
         return "small-music";
   }
}

const char* StableAudio::GetSelectedModelDescription() const
{
   switch (mModelSelection)
   {
      case kModel_SmallSfx:
         return "small-sfx: sound effects, cpu, 120s max";
      case kModel_Medium:
         return "medium: high quality, gpu, 380s max";
      case kModel_SmallMusic:
      default:
         return "small-music: music, cpu, 120s max";
   }
}

void StableAudio::RefreshModelPathEntries()
{
   if (mDitPathEntry != nullptr)
   {
      mDitPathEntry->SetText(mDitPath);
      mDitPathEntry->UpdateDisplayString();
   }
   if (mDecoderPathEntry != nullptr)
   {
      mDecoderPathEntry->SetText(mDecoderPath);
      mDecoderPathEntry->UpdateDisplayString();
   }
   if (mTextEncoderPathEntry != nullptr)
   {
      mTextEncoderPathEntry->SetText(mTextEncoderPath);
      mTextEncoderPathEntry->UpdateDisplayString();
   }
}

void StableAudio::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mPromptEntry->Draw();
   mPromptDropdown->Draw();
   mMoreIdeasButton->Draw();
   mAutoplayCheckbox->Draw();
   if (mAutoplay)
   {
      std::string autoplayStatus = "next: queued";
      if (mGenerationInProgress)
         autoplayStatus = "next: generating";
      else if (mAutoplayNextGenerationTime > 0)
         autoplayStatus = "next: " + ofToString(std::max(0.0, (mAutoplayNextGenerationTime - gTime) / 1000.0), 1) + "s";

      const ofRectangle autoplayRect = mAutoplayCheckbox->GetRect(true);
      DrawTextNormal(autoplayStatus, autoplayRect.getMaxX() + 6, autoplayRect.y + 12, 9);
   }
   mModelDropdown->Draw();
   mGenerateButton->Draw();
   if (mPlayButton->IsShowing())
      mPlayButton->Draw();
   if (mStopButton->IsShowing())
      mStopButton->Draw();
   mLoopCheckbox->Draw();
   mVolumeSlider->Draw();
   mGeneratedWavDropdown->Draw();
   mLoadWavButton->Draw();
   mDeleteWavButton->Draw();
   mUseMetadataWavLabelsCheckbox->Draw();
   mSecondsSlider->Draw();
   mStepsSlider->Draw();
   mSeedSlider->Draw();
   mTransitionDropdown->Draw();
   if (mTransitionMode == kTransition_Crossfade)
      mCrossfadeSlider->Draw();
   mDitPathEntry->Draw();
   mDecoderPathEntry->Draw();
   mTextEncoderPathEntry->Draw();

   ofPushStyle();
   ofFill();
   ofSetColor(255, 255, 255, 50);
   ofRect(5, 142, mWidth - 10, 22);
   ofSetColor(40, 40, 40);

   if (mGenerationInProgress)
      DrawTextNormal("generating...", 10, 157, 9);
   else if (!mStatusString.empty())
      DrawTextNormal(mStatusString, 10, 157, 9);
   else if (mSample != nullptr)
      DrawTextNormal(mSample->Name(), 10, 157, 9);
   else
      DrawTextNormal(GetSelectedModelDescription(), 10, 157, 9);

   ofPopStyle();
}

void StableAudio::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void StableAudio::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void StableAudio::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void StableAudio::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);

   out << std::string(mPromptEntry != nullptr ? mPromptEntry->GetText() : mPrompt);
   out << std::string(mDitPathEntry != nullptr ? mDitPathEntry->GetText() : mDitPath);
   out << std::string(mDecoderPathEntry != nullptr ? mDecoderPathEntry->GetText() : mDecoderPath);
   out << std::string(mTextEncoderPathEntry != nullptr ? mTextEncoderPathEntry->GetText() : mTextEncoderPath);
   out << mModelSelection;
   out << mTransitionMode;
   out << mCrossfadeSeconds;
}

void StableAudio::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (rev >= 1)
   {
      in >> mPrompt;
      in >> mDitPath;
      in >> mDecoderPath;
      in >> mTextEncoderPath;

      if (mPromptEntry != nullptr)
      {
         mPromptEntry->SetText(mPrompt);
         mPromptEntry->UpdateDisplayString();
      }
      if (mDitPathEntry != nullptr)
      {
         mDitPathEntry->SetText(mDitPath);
         mDitPathEntry->UpdateDisplayString();
      }
      if (mDecoderPathEntry != nullptr)
      {
         mDecoderPathEntry->SetText(mDecoderPath);
         mDecoderPathEntry->UpdateDisplayString();
      }
      if (mTextEncoderPathEntry != nullptr)
      {
         mTextEncoderPathEntry->SetText(mTextEncoderPath);
         mTextEncoderPathEntry->UpdateDisplayString();
      }
   }

   if (rev >= 2)
      in >> mModelSelection;
   if (rev >= 3)
      in >> mTransitionMode;
   if (rev >= 4)
      in >> mCrossfadeSeconds;
   UpdateCrossfadeSlider();

   RefreshGeneratedWavList();
   RefreshPromptChoices();
   UpdatePlaybackControls();
}

std::vector<IUIControl*> StableAudio::ControlsToIgnoreInSaveState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mGenerateButton);
   ignore.push_back(mMoreIdeasButton);
   ignore.push_back(mLoadWavButton);
   ignore.push_back(mDeleteWavButton);
   ignore.push_back(mPlayButton);
   ignore.push_back(mStopButton);
   return ignore;
}
