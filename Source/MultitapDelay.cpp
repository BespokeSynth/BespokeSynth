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
/*
  ==============================================================================

    MultitapDelay.cpp
    Created: 25 Nov 2018 11:16:38am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "MultitapDelay.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"

const float mBufferX = 5;
const float mBufferY = 50;
const float mBufferW = 800;
const float mBufferH = 200;

MultitapDelay::MultitapDelay()
: IAudioProcessor(gBufferSize)
, mWriteBuffer(gBufferSize)
, mDelayBuffer(5 * gSampleRate)
{
   mTaps.resize(mNumTaps);
   for (int i = 0; i < mNumTaps; ++i)
      mTaps[i].mOwner = this;

   for (int i = 0; i < kNumMPETaps; ++i)
      mMPETaps[i].mOwner = this;
}

void MultitapDelay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDryAmountSlider = new FloatSlider(this, "dry", 5, 10, 150, 15, &mDryAmount, 0, 1);
   mDisplayLengthSlider = new FloatSlider(this, "display length", mDryAmountSlider, kAnchor_Below, 150, 15, &mDisplayLength, .1f, mDelayBuffer.Size() / gSampleRate);
   mDisplayLength = mDisplayLengthSlider->GetMax();

   for (int i = 0; i < mNumTaps; ++i)
   {
      float y = mBufferY + mBufferH + 10 + i * 100;
      mTaps[i].mDelayMsSlider = new FloatSlider(this, ("delay " + ofToString(i + 1)).c_str(), 10, y, 150, 15, &mTaps[i].mDelayMs, gBufferSize / gSampleRateMs, mDelayBuffer.Size() / gSampleRateMs);
      mTaps[i].mGainSlider = new FloatSlider(this, ("gain " + ofToString(i + 1)).c_str(), mTaps[i].mDelayMsSlider, kAnchor_Below, 150, 15, &mTaps[i].mGain, 0, 1);
      mTaps[i].mFeedbackSlider = new FloatSlider(this, ("feedback " + ofToString(i + 1)).c_str(), mTaps[i].mGainSlider, kAnchor_Below, 150, 15, &mTaps[i].mFeedback, 0, 1);
      mTaps[i].mPanSlider = new FloatSlider(this, ("pan " + ofToString(i + 1)).c_str(), mTaps[i].mFeedbackSlider, kAnchor_Below, 150, 15, &mTaps[i].mPan, -1, 1);
   }
}

MultitapDelay::~MultitapDelay()
{
}

void MultitapDelay::Process(double time)
{
   PROFILER(MultitapDelay);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();

   if (!mEnabled)
   {
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Reset();
      return;
   }

   mWriteBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   mDelayBuffer.SetNumChannels(GetBuffer()->NumActiveChannels());
   for (int t = 0; t < mNumTaps; ++t)
      mTaps[t].mTapBuffer.SetNumActiveChannels(mWriteBuffer.NumActiveChannels());

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      BufferCopy(mWriteBuffer.GetChannel(ch), GetBuffer()->GetChannel(ch), bufferSize);
      Mult(mWriteBuffer.GetChannel(ch), mDryAmount, bufferSize);
      mDelayBuffer.WriteChunk(GetBuffer()->GetChannel(ch), bufferSize, ch);
   }

   for (int i = 0; i < bufferSize; ++i)
   {
      ComputeSliders(i);

      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         for (int t = 0; t < mNumTaps; ++t)
            mTaps[t].Process(&mWriteBuffer.GetChannel(ch)[i], i, ch);
         for (int t = 0; t < kNumMPETaps; ++t)
            mMPETaps[t].Process(&mWriteBuffer.GetChannel(ch)[i], i, ch);
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), bufferSize);

      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
   }

   GetBuffer()->Reset();
}

void MultitapDelay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDryAmountSlider->Draw();
   mDisplayLengthSlider->Draw();

   for (int i = 0; i < mNumTaps; ++i)
   {
      mTaps[i].mDelayMsSlider->Draw();
      mTaps[i].mGainSlider->Draw();
      mTaps[i].mFeedbackSlider->Draw();
      mTaps[i].mPanSlider->Draw();
   }

   for (int ch = 0; ch < mDelayBuffer.NumChannels(); ++ch)
      mDelayBuffer.Draw(mBufferX, mBufferY + mBufferH / mDelayBuffer.NumChannels() * ch, mBufferW, mBufferH / mDelayBuffer.NumChannels(), mDisplayLength * gSampleRate, ch);

   ofPushMatrix();
   ofTranslate(mBufferX, mBufferY);
   for (int i = 0; i < kNumMPETaps; ++i)
      mMPETaps[i].Draw(mBufferW, mBufferH);
   for (int i = 0; i < mNumTaps; ++i)
      mTaps[i].Draw(mBufferW, mBufferH);
   ofPopMatrix();
}

void MultitapDelay::DropdownClicked(DropdownList* list)
{
}

void MultitapDelay::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void MultitapDelay::ButtonClicked(ClickButton* button, double time)
{
}

void MultitapDelay::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void MultitapDelay::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool MultitapDelay::MouseMoved(float x, float y)
{
   return IDrawableModule::MouseMoved(x, y);
}

void MultitapDelay::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void MultitapDelay::GetModuleDimensions(float& width, float& height)
{
   width = mBufferW + 10;
   height = mBufferY + mBufferH + 10 + 100 * mNumTaps;
}

void MultitapDelay::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void MultitapDelay::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void MultitapDelay::PlayNote(NoteMessage note)
{
   if (note.voiceIdx == -1 || note.voiceIdx >= kNumMPETaps)
      return;

   /*if (velocity > 0)
      mMPETaps[voiceIdx].mADSR.Start(time, 1);
   else
      mMPETaps[voiceIdx].mADSR.Stop(time);
   mMPETaps[voiceIdx].mPitch = pitch;
   mMPETaps[voiceIdx].mPlay = 0;
   mMPETaps[voiceIdx].mPitchBend = modulation.pitchBend;
   mMPETaps[voiceIdx].mPressure = modulation.pressure;
   mMPETaps[voiceIdx].mModWheel = modulation.modWheel;*/
}

void MultitapDelay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void MultitapDelay::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void MultitapDelay::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mDelayBuffer.SaveState(out);
}

void MultitapDelay::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mDelayBuffer.LoadState(in);
}


void MultitapDelay::DelayMPETap::Process(float* sampleOut, int offset, int ch)
{
   /*if (!mADSR.IsDone(gTime) && sampleLength > 0)
   {
      double time = gTime;
      for (int i=0; i<outLength; ++i)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(i) : ModulationParameters::kDefaultPitchBend;
         float pressure = mPressure ? mPressure->GetValue(i) : ModulationParameters::kDefaultPressure;
         float modwheel = mModWheel ? mModWheel->GetValue(i) : ModulationParameters::kDefaultModWheel;
         if (pressure > 0)
         {
            mGranulator.mGrainOverlap = ofMap(pressure * pressure, 0, 1, 3, MAX_GRAINS);
            mGranulator.mPosRandomizeMs = ofMap(pressure * pressure, 0, 1, 100, .03f);
         }
         mGranulator.mGrainLengthMs = ofMap(modwheel, -1, 1, 150-140, 150+140);
         
         float blend = .0005f;
         mGain = mGain * (1-blend) + pressure * blend;
         
         ChannelBuffer temp(sample, sampleLength);
         float outSample[1];
         outSample[0] = 0;
         float pos = (mPitch + pitchBend + MIN(.125f, mPlay) - mOwner->mKeyboardBasePitch) / mOwner->mKeyboardNumPitches;
         mGranulator.Process(time, &temp, sampleLength, ofLerp(mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples, pos), outSample);
         outSample[0] *= sqrtf(mGain);
         outSample[0] *= mADSR.Value(time);
         out[i] += outSample[0];
         time += gInvSampleRateMs;
         mPlay += .001f;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }*/
}

void MultitapDelay::DelayMPETap::Draw(float w, float h)
{
   /*if (!mADSR.IsDone(gTime))
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
         ofVertex(keyXTop +10, h - pressure * h);
         ofVertex(keyX+10, h);
         ofEndShape();
         ofPopStyle();
      }
      
      mGranulator.Draw(0, 0, w, h, mOwner->mDisplayStartSamples, mOwner->mDisplayEndSamples - mOwner->mDisplayStartSamples, mOwner->mSample->LengthInSamples());
   }*/
}


MultitapDelay::DelayTap::DelayTap()
: mTapBuffer(gBufferSize)
{
}

void MultitapDelay::DelayTap::Process(float* sampleOut, int offset, int ch)
{
   if (mGain > 0)
   {
      float delaySamps = mDelayMs / gInvSampleRateMs;
      delaySamps = ofClamp(delaySamps - offset, 0.1f, mOwner->mDelayBuffer.Size() - 2);

      int sampsAgoA = int(delaySamps);
      int sampsAgoB = sampsAgoA + 1;

      float sample = mOwner->mDelayBuffer.GetSample(sampsAgoA, ch);
      float nextSample = mOwner->mDelayBuffer.GetSample(sampsAgoB, ch);
      float a = delaySamps - sampsAgoA;
      float delayedSample = (1 - a) * sample + a * nextSample; //interpolate

      float outputSample = delayedSample * mGain;
      mTapBuffer.GetChannel(ch)[offset] = outputSample;

      *sampleOut += outputSample;
      float panGain = ch == 0 ? GetLeftPanGain(mPan) : GetRightPanGain(mPan);
      mOwner->mDelayBuffer.Accum(gBufferSize - offset, outputSample * mFeedback * panGain, ch);
   }
}

void MultitapDelay::DelayTap::Draw(float w, float h)
{
   ofPushStyle();
   ofFill();
   float x = ofClamp(1 - (mDelayMs * gSampleRateMs) / (mOwner->mDisplayLength * gSampleRate), 0, 1) * w;
   float y = h - mGain * h;
   ofLine(x, y, x, h);
   ofRect(x - 5, y - 5, 10, 10);
   ofPopStyle();
}
