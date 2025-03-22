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
//  SeaOfGrain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#include "SeaOfGrain.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"

#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_gui_basics/juce_gui_basics.h"

const float mBufferX = 5;
const float mBufferY = 100;
const float mBufferW = 800;
const float mBufferH = 200;

SeaOfGrain::SeaOfGrain()
: IAudioProcessor(gBufferSize)
, mRecordBuffer(10 * gSampleRate)
{
   mSample = new Sample();

   for (int i = 0; i < kNumMPEVoices; ++i)
      mMPEVoices[i].mOwner = this;

   for (int i = 0; i < kNumManualVoices; ++i)
      mManualVoices[i].mOwner = this;
}

void SeaOfGrain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLoadButton = new ClickButton(this, "load", 5, 3);
   mRecordInputCheckbox = new Checkbox(this, "record", 50, 3, &mRecordInput);
   mVolumeSlider = new FloatSlider(this, "volume", 5, 20, 150, 15, &mVolume, 0, 2);
   mDisplayOffsetSlider = new FloatSlider(this, "offset", 5, 40, 150, 15, &mDisplayOffset, 0, 10);
   mDisplayLengthSlider = new FloatSlider(this, "display length", 5, 60, 150, 15, &mDisplayLength, 1, 10);
   mKeyboardBasePitchSelector = new DropdownList(this, "keyboard base pitch", 5, 80, &mKeyboardBasePitch, 60);
   mKeyboardNumPitchesSelector = new DropdownList(this, "keyboard num pitches", mKeyboardBasePitchSelector, kAnchor_Right, &mKeyboardNumPitches);

   mKeyboardBasePitchSelector->AddLabel("0", 0);
   mKeyboardBasePitchSelector->AddLabel("12", 12);
   mKeyboardBasePitchSelector->AddLabel("24", 24);
   mKeyboardBasePitchSelector->AddLabel("36", 36);
   mKeyboardBasePitchSelector->AddLabel("48", 48);
   mKeyboardBasePitchSelector->AddLabel("60", 60);

   mKeyboardNumPitchesSelector->AddLabel("12", 12);
   mKeyboardNumPitchesSelector->AddLabel("24", 24);
   mKeyboardNumPitchesSelector->AddLabel("36", 36);
   mKeyboardNumPitchesSelector->AddLabel("48", 48);
   mKeyboardNumPitchesSelector->AddLabel("60", 60);

   for (int i = 0; i < kNumManualVoices; ++i)
   {
      float x = 10 + i * 130;
      mManualVoices[i].mGainSlider = new FloatSlider(this, ("gain " + ofToString(i + 1)).c_str(), x, mBufferY + mBufferH + 12, 120, 15, &mManualVoices[i].mGain, 0, 1);
      mManualVoices[i].mPositionSlider = new FloatSlider(this, ("pos " + ofToString(i + 1)).c_str(), mManualVoices[i].mGainSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mPosition, 0, 1);
      mManualVoices[i].mOverlapSlider = new FloatSlider(this, ("overlap " + ofToString(i + 1)).c_str(), mManualVoices[i].mPositionSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mGrainOverlap, .25, MAX_GRAINS);
      mManualVoices[i].mSpeedSlider = new FloatSlider(this, ("speed " + ofToString(i + 1)).c_str(), mManualVoices[i].mOverlapSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mSpeed, -3, 3);
      mManualVoices[i].mLengthMsSlider = new FloatSlider(this, ("len ms " + ofToString(i + 1)).c_str(), mManualVoices[i].mSpeedSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mGrainLengthMs, 1, 1000);
      mManualVoices[i].mPosRandomizeSlider = new FloatSlider(this, ("pos r " + ofToString(i + 1)).c_str(), mManualVoices[i].mLengthMsSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mPosRandomizeMs, 0, 200);
      mManualVoices[i].mSpeedRandomizeSlider = new FloatSlider(this, ("speed r " + ofToString(i + 1)).c_str(), mManualVoices[i].mPosRandomizeSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mSpeedRandomize, 0, .3f);
      mManualVoices[i].mSpacingRandomizeSlider = new FloatSlider(this, ("spacing r " + ofToString(i + 1)).c_str(), mManualVoices[i].mSpeedRandomizeSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mSpacingRandomize, 0, 1);
      mManualVoices[i].mOctaveCheckbox = new Checkbox(this, ("octaves " + ofToString(i + 1)).c_str(), mManualVoices[i].mSpacingRandomizeSlider, kAnchor_Below, &mManualVoices[i].mGranulator.mOctaves);
      mManualVoices[i].mWidthSlider = new FloatSlider(this, ("width " + ofToString(i + 1)).c_str(), mManualVoices[i].mOctaveCheckbox, kAnchor_Below, 120, 15, &mManualVoices[i].mGranulator.mWidth, 0, 1);
      mManualVoices[i].mPanSlider = new FloatSlider(this, ("pan " + ofToString(i + 1)).c_str(), mManualVoices[i].mWidthSlider, kAnchor_Below, 120, 15, &mManualVoices[i].mPan, -1, 1);
   }
}

SeaOfGrain::~SeaOfGrain()
{
   delete mSample;
}

void SeaOfGrain::Poll()
{
}

void SeaOfGrain::Process(double time)
{
   PROFILER(SeaOfGrain);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr || (mSample == nullptr && !mHasRecordedInput) || mLoading)
      return;

   if (!mEnabled)
   {
      SyncBuffers();

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   ComputeSliders(0);
   int numChannels = 2;
   SyncBuffers(numChannels);
   mRecordBuffer.SetNumChannels(numChannels);

   int bufferSize = target->GetBuffer()->BufferSize();
   ChannelBuffer* out = target->GetBuffer();
   assert(bufferSize == gBufferSize);

   if (mRecordInput)
   {
      for (int ch = 0; ch < numChannels; ++ch)
         mRecordBuffer.WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
   }

   gWorkChannelBuffer.SetNumActiveChannels(numChannels);
   gWorkChannelBuffer.Clear();
   for (int i = 0; i < kNumMPEVoices; ++i)
      mMPEVoices[i].Process(&gWorkChannelBuffer, bufferSize);
   for (int i = 0; i < kNumManualVoices; ++i)
      mManualVoices[i].Process(&gWorkChannelBuffer, bufferSize);
   for (int ch = 0; ch < numChannels; ++ch)
   {
      Mult(gWorkChannelBuffer.GetChannel(ch), mVolume, bufferSize);
      GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      Add(out->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
   }

   GetBuffer()->Reset();
}

void SeaOfGrain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mLoadButton->Draw();
   mRecordInputCheckbox->Draw();
   mVolumeSlider->Draw();
   mDisplayOffsetSlider->Draw();
   mDisplayLengthSlider->Draw();
   mKeyboardBasePitchSelector->Draw();
   mKeyboardNumPitchesSelector->Draw();

   for (int i = 0; i < kNumManualVoices; ++i)
   {
      mManualVoices[i].mGainSlider->Draw();
      mManualVoices[i].mPositionSlider->Draw();
      mManualVoices[i].mOverlapSlider->Draw();
      mManualVoices[i].mSpeedSlider->Draw();
      mManualVoices[i].mLengthMsSlider->Draw();
      mManualVoices[i].mPosRandomizeSlider->Draw();
      mManualVoices[i].mSpeedRandomizeSlider->Draw();
      mManualVoices[i].mSpacingRandomizeSlider->Draw();
      mManualVoices[i].mOctaveCheckbox->Draw();
      mManualVoices[i].mWidthSlider->Draw();
      mManualVoices[i].mPanSlider->Draw();
   }

   if (mSample || mHasRecordedInput)
   {
      ofPushMatrix();
      ofTranslate(mBufferX, mBufferY);
      ofPushStyle();

      if (mHasRecordedInput)
      {
         mRecordBuffer.Draw(0, 0, mBufferW, mBufferH, mDisplayLength * gSampleRate);
      }
      else
      {
         mSample->LockDataMutex(true);
         DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), mDisplayStartSamples, mDisplayEndSamples, 0);
         DrawTextNormal(std::string(mSample->Name()), 5, 10);
         mSample->LockDataMutex(false);
      }

      ofPushStyle();
      ofFill();
      for (int i = 0; i < mKeyboardNumPitches; ++i)
      {
         ofSetColor(i % 2 * 200, 200, 0);
         ofRect(mBufferW * float(i) / mKeyboardNumPitches, mBufferH, mBufferW / mKeyboardNumPitches, 10);
      }
      ofPopStyle();

      for (int i = 0; i < kNumMPEVoices; ++i)
         mMPEVoices[i].Draw(mBufferW, mBufferH);
      for (int i = 0; i < kNumManualVoices; ++i)
         mManualVoices[i].Draw(mBufferW, mBufferH);

      ofPopStyle();
      ofPopMatrix();
   }
}

float SeaOfGrain::GetSampleRateRatio() const
{
   if (mHasRecordedInput)
      return 1.0f;
   else
      return mSample->GetSampleRateRatio();
}

ChannelBuffer* SeaOfGrain::GetSourceBuffer()
{
   if (mHasRecordedInput)
      return mRecordBuffer.GetRawBuffer();
   else
      return mSample->Data();
}

float SeaOfGrain::GetSourceStartSample()
{
   if (mHasRecordedInput)
      return -mDisplayLength * gSampleRate;
   else
      return mDisplayStartSamples;
}

float SeaOfGrain::GetSourceEndSample()
{
   if (mHasRecordedInput)
      return 0;
   else
      return mDisplayEndSamples;
}

float SeaOfGrain::GetSourceBufferOffset()
{
   if (mHasRecordedInput)
      return mRecordBuffer.GetRawBufferOffset(0);
   else
      return 0;
}

void SeaOfGrain::FilesDropped(std::vector<std::string> files, int x, int y)
{
   mLoading = true;

   mSample->Reset();

   mSample->Read(files[0].c_str());
   UpdateSample();

   mRecordInput = false;
   mHasRecordedInput = false;
   mLoading = false;
}

void SeaOfGrain::SampleDropped(int x, int y, Sample* sample)
{
   mLoading = true;

   mSample->CopyFrom(sample);
   UpdateSample();

   mRecordInput = false;
   mHasRecordedInput = false;
   mLoading = false;
}

void SeaOfGrain::DropdownClicked(DropdownList* list)
{
}

void SeaOfGrain::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void SeaOfGrain::UpdateSample()
{
   float sampleLengthSeconds = mSample->LengthInSamples() / mSample->GetSampleRateRatio() / gSampleRate;
   mDisplayLength = MIN(mDisplayLength, MIN(10, sampleLengthSeconds));
   mDisplayLengthSlider->SetExtents(0, sampleLengthSeconds);
   UpdateDisplaySamples();
}

void SeaOfGrain::UpdateDisplaySamples()
{
   float ratio = 1;
   if (!mHasRecordedInput)
      ratio = mSample->GetSampleRateRatio();
   mDisplayStartSamples = mDisplayOffset * gSampleRate * ratio;
   mDisplayEndSamples = mDisplayLength * gSampleRate * ratio + mDisplayStartSamples;
}

void SeaOfGrain::LoadFile()
{
   using namespace juce;
   auto file_pattern = TheSynth->GetAudioFormatManager().getWildcardForAllFormats();
   if (File::areFileNamesCaseSensitive())
      file_pattern += ";" + file_pattern.toUpperCase();
   FileChooser chooser("Load sample", File(ofToSamplePath("")),
                       file_pattern, true, false, TheSynth->GetFileChooserParent());
   if (chooser.browseForFileToOpen())
   {
      auto file = chooser.getResult();

      std::vector<std::string> fileArray;
      fileArray.push_back(file.getFullPathName().toStdString());
      FilesDropped(fileArray, 0, 0);
   }
}

void SeaOfGrain::ButtonClicked(ClickButton* button, double time)
{
   if (button == mLoadButton)
      LoadFile();
}

void SeaOfGrain::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void SeaOfGrain::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool SeaOfGrain::MouseMoved(float x, float y)
{
   return IDrawableModule::MouseMoved(x, y);
}

void SeaOfGrain::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRecordInputCheckbox)
   {
      if (mRecordInput)
      {
         mHasRecordedInput = true;
         mDisplayLength = mRecordBuffer.Size() / gSampleRate;
      }
   }
}

void SeaOfGrain::GetModuleDimensions(float& width, float& height)
{
   width = mBufferW + 10;
   height = mBufferY + mBufferH + 202;
}

void SeaOfGrain::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mDisplayOffsetSlider || slider == mDisplayLengthSlider)
      UpdateDisplaySamples();
}

void SeaOfGrain::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void SeaOfGrain::PlayNote(NoteMessage note)
{
   if (note.voiceIdx == -1 || note.voiceIdx >= kNumMPEVoices)
      return;

   if (note.velocity > 0)
      mMPEVoices[note.voiceIdx].mADSR.Start(note.time, 1);
   else
      mMPEVoices[note.voiceIdx].mADSR.Stop(note.time);
   mMPEVoices[note.voiceIdx].mPitch = note.pitch;
   mMPEVoices[note.voiceIdx].mPlay = 0;
   mMPEVoices[note.voiceIdx].mPitchBend = note.modulation.pitchBend;
   mMPEVoices[note.voiceIdx].mPressure = note.modulation.pressure;
   mMPEVoices[note.voiceIdx].mModWheel = note.modulation.modWheel;
}

void SeaOfGrain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void SeaOfGrain::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void SeaOfGrain::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mHasRecordedInput;
   if (mHasRecordedInput)
      mRecordBuffer.SaveState(out);
   else
      mSample->SaveState(out);
}

void SeaOfGrain::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mHasRecordedInput = false;
   if (rev > 0)
      in >> mHasRecordedInput;

   if (mHasRecordedInput)
   {
      mRecordBuffer.LoadState(in);
   }
   else
   {
      mSample->LoadState(in);
      UpdateSample();
   }
}


SeaOfGrain::GrainMPEVoice::GrainMPEVoice()
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainMPEVoice::Process(ChannelBuffer* output, int bufferSize)
{
   if (!mADSR.IsDone(gTime) && mOwner->GetSourceBuffer()->BufferSize() > 0)
   {
      double time = gTime;
      float speed = mOwner->GetSampleRateRatio();
      for (int i = 0; i < bufferSize; ++i)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(i) : ModulationParameters::kDefaultPitchBend;
         float pressure = mPressure ? mPressure->GetValue(i) : ModulationParameters::kDefaultPressure;
         float modwheel = mModWheel ? mModWheel->GetValue(i) : ModulationParameters::kDefaultModWheel;
         if (pressure > 0)
         {
            mGranulator.mGrainOverlap = ofMap(pressure * pressure, 0, 1, 3, MAX_GRAINS);
            mGranulator.mPosRandomizeMs = ofMap(pressure * pressure, 0, 1, 100, .03f);
         }
         mGranulator.mGrainLengthMs = ofMap(modwheel, -1, 1, 10, 700);

         float blend = .0005f;
         mGain = mGain * (1 - blend) + pressure * blend;

         float outSample[ChannelBuffer::kMaxNumChannels];
         Clear(outSample, ChannelBuffer::kMaxNumChannels);
         float pos = (mPitch + pitchBend + MIN(.125f, mPlay) - mOwner->mKeyboardBasePitch) / mOwner->mKeyboardNumPitches;
         mGranulator.ProcessFrame(time, mOwner->GetSourceBuffer(), mOwner->GetSourceBuffer()->BufferSize(), ofLerp(mOwner->GetSourceStartSample(), mOwner->GetSourceEndSample(), pos) + mOwner->GetSourceBufferOffset(), speed, outSample);
         for (int ch = 0; ch < output->NumActiveChannels(); ++ch)
            output->GetChannel(ch)[i] += outSample[ch] * sqrtf(mGain) * mADSR.Value(time);

         time += gInvSampleRateMs;
         mPlay += .001f;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainMPEVoice::Draw(float w, float h)
{
   if (!mADSR.IsDone(gTime))
   {
      if (mPitch - mOwner->mKeyboardBasePitch >= 0 && mPitch - mOwner->mKeyboardBasePitch < mOwner->mKeyboardNumPitches)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(0) : 0;
         float pressure = mPressure ? mPressure->GetValue(0) : 0;

         ofPushStyle();
         ofFill();
         float keyX = (mPitch - mOwner->mKeyboardBasePitch) / mOwner->mKeyboardNumPitches * w;
         float keyXTop = keyX + pitchBend * w / mOwner->mKeyboardNumPitches;
         ofBeginShape();
         ofVertex(keyX, h);
         ofVertex(keyXTop, h - pressure * h);
         ofVertex(keyXTop + 10, h - pressure * h);
         ofVertex(keyX + 10, h);
         ofEndShape();
         ofPopStyle();
      }

      mGranulator.Draw(0, 0, w, h, mOwner->GetSourceStartSample() + mOwner->GetSourceBufferOffset(), mOwner->GetSourceEndSample() - mOwner->GetSourceStartSample(), mOwner->GetSourceBuffer()->BufferSize());
   }
}


SeaOfGrain::GrainManualVoice::GrainManualVoice()
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainManualVoice::Process(ChannelBuffer* output, int bufferSize)
{
   if (mGain > 0 && mOwner->GetSourceBuffer()->BufferSize() > 0)
   {
      double time = gTime;
      float panLeft = GetLeftPanGain(mPan);
      float panRight = GetRightPanGain(mPan);
      float speed = mOwner->GetSampleRateRatio();
      for (int i = 0; i < bufferSize; ++i)
      {
         float outSample[ChannelBuffer::kMaxNumChannels];
         Clear(outSample, ChannelBuffer::kMaxNumChannels);
         mGranulator.ProcessFrame(time, mOwner->GetSourceBuffer(), mOwner->GetSourceBuffer()->BufferSize(), ofLerp(mOwner->GetSourceStartSample(), mOwner->GetSourceEndSample(), mPosition) + mOwner->GetSourceBufferOffset(), speed, outSample);
         for (int ch = 0; ch < output->NumActiveChannels(); ++ch)
            output->GetChannel(ch)[i] += outSample[ch] * mGain * (ch == 0 ? panLeft : panRight);
         time += gInvSampleRateMs;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainManualVoice::Draw(float w, float h)
{
   if (mGain > 0)
   {
      ofPushStyle();
      ofFill();
      float x = mPosition * w;
      float y = h - mGain * h;
      ofLine(x, y, x, h);
      ofRect(x - 5, y - 5, 10, 10);
      ofPopStyle();
      mGranulator.Draw(0, 0, w, h, mOwner->GetSourceStartSample() + mOwner->GetSourceBufferOffset(), mOwner->GetSourceEndSample() - mOwner->GetSourceStartSample(), mOwner->GetSourceBuffer()->BufferSize());
   }
}
