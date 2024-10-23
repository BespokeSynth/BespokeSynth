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
//  MultitrackRecorder.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/13/14.
//
//

#include "MultitrackRecorder.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Sample.h"
#include "UIControlMacros.h"
#include "PatchCableSource.h"
#include "UserPrefs.h"

MultitrackRecorder::MultitrackRecorder()
{
   mModuleContainer.SetOwner(this);
}

void MultitrackRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   CHECKBOX(mRecordCheckbox, "record", &mRecord);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mClearButton, "clear");
   UIBLOCK_NEWLINE();
   BUTTON(mAddTrackButton, "add track");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mBounceButton, "bounce");
   ENDUIBLOCK0();
}

MultitrackRecorder::~MultitrackRecorder()
{
}

void MultitrackRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mAddTrackButton->SetShowing(!mRecord);
   mBounceButton->SetShowing(!mRecord);

   mRecordCheckbox->Draw();
   mClearButton->Draw();
   mAddTrackButton->Draw();
   mBounceButton->Draw();

   if (mStatusStringTime + 5000 > gTime)
      DrawTextNormal(mStatusString, 120, 33);

   float posX = 5;
   float posY = 42;
   for (auto* track : mTracks)
   {
      track->SetPosition(posX, posY);
      posY += 100;
   }

   mHeight = posY;

   mModuleContainer.DrawModules();
}

void MultitrackRecorder::AddTrack()
{
   int recordingLength = GetRecordingLength();

   ModuleFactory::Spawnable spawnable;
   spawnable.mLabel = "multitrackrecordertrack";
   MultitrackRecorderTrack* track = dynamic_cast<MultitrackRecorderTrack*>(TheSynth->SpawnModuleOnTheFly(spawnable, 0, 0, true));
   track->Setup(this, recordingLength);
   track->SetName(GetUniqueName("track", mModuleContainer.GetModuleNames<MultitrackRecorderTrack*>()).c_str());
   mModuleContainer.TakeModule(track);
   mTracks.push_back(track);
}

void MultitrackRecorder::RemoveTrack(MultitrackRecorderTrack* track)
{
   if (!mRecord)
   {
      RemoveFromVector(track, mTracks);
      RemoveChild(track);
      mModuleContainer.DeleteModule(track);
   }
}

int MultitrackRecorder::GetRecordingLength()
{
   int recordingLength = 0;
   for (auto* track : mTracks)
   {
      if (track->GetRecordingLength() > recordingLength)
         recordingLength = track->GetRecordingLength();
   }

   return recordingLength;
}

void MultitrackRecorder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mAddTrackButton)
   {
      AddTrack();
   }

   if (button == mBounceButton)
   {
      std::string save_prefix = "multitrack_";
      if (!TheSynth->GetLastSavePath().empty())
      {
         // This assumes that mCurrentSaveStatePath always has a valid filename at the end
         std::string filename = juce::File(TheSynth->GetLastSavePath()).getFileNameWithoutExtension().toStdString();
         save_prefix = filename + "_";
      }
      // Crude way of checking if the filename does not have a date/time in it.
      if (std::count(save_prefix.begin(), save_prefix.end(), '-') < 3)
      {
         save_prefix += "%Y-%m-%d_%H-%M_";
      }
      std::string filenamePrefix = ofGetTimestampString(UserPrefs.recordings_path.Get() + save_prefix);

      int numFiles = 0;
      for (int i = 0; i < (int)mTracks.size(); ++i)
      {
         Sample* sample = mTracks[i]->BounceRecording();

         if (sample)
         {
            std::string filename = filenamePrefix + ofToString(i + 1) + ".wav";
            sample->Write(filename.c_str());
            ++numFiles;
         }
      }

      if (numFiles > 0)
      {
         mStatusString = "wrote " + ofToString(numFiles) + " files to " + filenamePrefix + "*.wav";
         mStatusStringTime = gTime;
      }
   }

   if (button == mClearButton)
   {
      for (auto* track : mTracks)
         track->Clear();
   }
}

void MultitrackRecorder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRecordCheckbox)
   {
      for (auto* track : mTracks)
         track->SetRecording(mRecord);
   }
}

void MultitrackRecorder::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["modules"] = mModuleContainer.WriteModules();
}

void MultitrackRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleContainer.LoadModules(moduleInfo["modules"]);

   for (auto* child : mModuleContainer.GetModules())
   {
      MultitrackRecorderTrack* track = dynamic_cast<MultitrackRecorderTrack*>(child);
      if (!VectorContains(track, mTracks))
      {
         track->Setup(this, GetRecordingLength());
         mTracks.push_back(track);
      }
   }

   if (mTracks.size() == 0)
      AddTrack();

   SetUpFromSaveData();
}

void MultitrackRecorder::SetUpFromSaveData()
{
   mRecord = false;
   for (auto* track : mTracks)
      track->SetRecording(false);
}

void MultitrackRecorder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;

   //preserve order
   out << (int)mTracks.size();
   for (auto* track : mTracks)
      out << (std::string)track->Name();
}

void MultitrackRecorder::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;

   //preserve order
   int numTracks;
   in >> numTracks;
   std::vector<std::string> sortOrder;
   for (int i = 0; i < numTracks; ++i)
   {
      std::string name;
      in >> name;
      sortOrder.push_back(name);
   }

   std::vector<MultitrackRecorderTrack*> sortedTracks;
   for (auto& name : sortOrder)
   {
      for (auto& track : mTracks)
      {
         if (track->Name() == name)
         {
            sortedTracks.push_back(track);
            break;
         }
      }
   }

   mTracks = sortedTracks;
}

//////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
   const int kRecordingChunkSize = 48000 * 5;
   const int kMinRecordingChunks = 2;
};

MultitrackRecorderTrack::MultitrackRecorderTrack()
: IAudioProcessor(gBufferSize)
{
}

MultitrackRecorderTrack::~MultitrackRecorderTrack()
{
}

void MultitrackRecorderTrack::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   BUTTON(mDeleteButton, " X ");
   ENDUIBLOCK0();

   GetPatchCableSource()->SetManualSide(PatchCableSource::Side::kRight);
}

void MultitrackRecorderTrack::Process(double time)
{
   int numChannels = GetBuffer()->NumActiveChannels();
   for (size_t i = 0; i < mRecordChunks.size(); ++i)
   {
      if (mRecordChunks[i]->NumActiveChannels() > numChannels)
         numChannels = mRecordChunks[i]->NumActiveChannels();
   }

   ComputeSliders(0);
   SyncBuffers(numChannels);

   if (mDoRecording)
   {
      for (size_t i = 0; i < mRecordChunks.size(); ++i)
         mRecordChunks[i]->SetNumActiveChannels(numChannels);

      for (int i = 0; i < GetBuffer()->BufferSize(); ++i)
      {
         int chunkIndex = mRecordingLength / kRecordingChunkSize;
         int chunkPos = mRecordingLength % kRecordingChunkSize;
         for (int ch = 0; ch < numChannels; ++ch)
            mRecordChunks[chunkIndex]->GetChannel(ch)[chunkPos] = GetBuffer()->GetChannel(MIN(ch, GetBuffer()->NumActiveChannels() - 1))[i];
         ++mRecordingLength;
      }
   }

   if (GetTarget())
   {
      for (int ch = 0; ch < GetTarget()->GetBuffer()->NumActiveChannels(); ++ch)
      {
         float* buffer = GetBuffer()->GetChannel(MIN(ch, GetBuffer()->NumActiveChannels() - 1));
         Add(GetTarget()->GetBuffer()->GetChannel(ch), buffer, GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(buffer, GetBuffer()->BufferSize(), MIN(ch, GetVizBuffer()->NumChannels() - 1));
      }
   }

   GetBuffer()->Reset();
}

void MultitrackRecorderTrack::Poll()
{
   IDrawableModule::Poll();

   int chunkIndex = mRecordingLength / kRecordingChunkSize;
   if (chunkIndex >= (int)mRecordChunks.size() - 1)
   {
      mRecordChunks.push_back(new ChannelBuffer(kRecordingChunkSize));
      mRecordChunks[mRecordChunks.size() - 1]->GetChannel(0); //set up buffer
   }
}

void MultitrackRecorderTrack::DrawModule()
{
   mDeleteButton->Draw();

   float width, height;
   GetModuleDimensions(width, height);

   ofPushMatrix();
   ofTranslate(5, 3);
   float sampleWidth = width - 10;

   ofSetColor(255, 255, 255, 30);
   ofFill();
   ofRect(0, 0, sampleWidth, height - 6);

   if (mDoRecording)
   {
      ofSetColor(255, 0, 0, 100);
      ofNoFill();
      ofRect(0, 0, sampleWidth, height - 6);
   }

   ofPushMatrix();
   int numChunks = mRecordingLength / kRecordingChunkSize + 1;
   float chunkWidth = sampleWidth / numChunks;
   for (int i = 0; i < numChunks; ++i)
   {
      if (i < (int)mRecordChunks.size())
         DrawAudioBuffer(chunkWidth, height - 6, mRecordChunks[i], 0, kRecordingChunkSize, -1);
      ofTranslate(chunkWidth, 0);
   }
   ofPopMatrix();

   ofPopMatrix();
}

void MultitrackRecorderTrack::Setup(MultitrackRecorder* recorder, int minLength)
{
   mRecorder = recorder;
   mRecordingLength = minLength;
}

void MultitrackRecorderTrack::SetRecording(bool record)
{
   if (record)
   {
      if (mRecordingLength == 0)
      {
         mRecordingLength = 0;

         for (size_t i = mRecordChunks.size(); i < kMinRecordingChunks; ++i)
         {
            mRecordChunks.push_back(new ChannelBuffer(kRecordingChunkSize));
            mRecordChunks[i]->GetChannel(0); //set up buffer
         }

         for (size_t i = 0; i < mRecordChunks.size(); ++i)
            mRecordChunks[i]->Clear();
      }

      mDoRecording = true;
   }
   else
   {
      mDoRecording = false;
   }
}

Sample* MultitrackRecorderTrack::BounceRecording()
{
   Sample* sample = nullptr;
   if (mRecordingLength > 0)
   {
      sample = new Sample();
      sample->Create(mRecordingLength);
      ChannelBuffer* data = sample->Data();
      int channelCount = mRecordChunks[0]->NumActiveChannels();
      data->SetNumActiveChannels(channelCount);

      int numChunks = mRecordingLength / kRecordingChunkSize + 1;
      for (int i = 0; i < numChunks; ++i)
      {
         int samplesLeftToRecord = mRecordingLength - i * kRecordingChunkSize;
         int samplesToCopy;
         if (samplesLeftToRecord > kRecordingChunkSize)
            samplesToCopy = kRecordingChunkSize;
         else
            samplesToCopy = samplesLeftToRecord;
         for (int ch = 0; ch < channelCount; ++ch)
            BufferCopy(data->GetChannel(ch) + i * kRecordingChunkSize, mRecordChunks[i]->GetChannel(ch), samplesToCopy);
      }
   }

   return sample;
}

void MultitrackRecorderTrack::Clear()
{
   for (auto* recordChunk : mRecordChunks)
      delete recordChunk;
   mRecordChunks.clear();
   mRecordingLength = 0;
   MultitrackRecorderTrack::Poll();
}

void MultitrackRecorderTrack::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void MultitrackRecorderTrack::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void MultitrackRecorderTrack::ButtonClicked(ClickButton* button, double time)
{
   if (button == mDeleteButton)
      mRecorder->RemoveTrack(this);
}

void MultitrackRecorderTrack::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void MultitrackRecorderTrack::SetUpFromSaveData()
{
}

void MultitrackRecorderTrack::GetModuleDimensions(float& width, float& height)
{
   if (mRecorder)
   {
      float parentWidth, parentHeight;
      mRecorder->GetDimensions(parentWidth, parentHeight);

      width = parentWidth - 15;
      height = 95;
   }
   else
   {
      width = 10;
      height = 10;
   }
}