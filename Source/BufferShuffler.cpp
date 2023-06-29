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
//  BufferShuffler.cpp
//
//  Created by Ryan Challinor on 6/23/23.
//
//

#include "BufferShuffler.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"

BufferShuffler::BufferShuffler()
: IAudioProcessor(gBufferSize)
, mInputBuffer(20 * 48000)
{
}

void BufferShuffler::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   INTSLIDER(mNumBarsSlider, "num bars", &mNumBars, 1, 8);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mIntervalSelector, "interval", (int*)&mInterval, 40);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mPlaybackStyleDropdown, "playback style", (int*)&mPlaybackStyle, 80);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mFreezeInputCheckbox, "freeze input", &mFreezeInput);
   ENDUIBLOCK0();

   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);

   mPlaybackStyleDropdown->AddLabel("normal", (int)PlaybackStyle::Normal);
   mPlaybackStyleDropdown->AddLabel("double", (int)PlaybackStyle::Double);
   mPlaybackStyleDropdown->AddLabel("half", (int)PlaybackStyle::Half);
   mPlaybackStyleDropdown->AddLabel("reverse", (int)PlaybackStyle::Reverse);
   mPlaybackStyleDropdown->AddLabel("double reverse", (int)PlaybackStyle::DoubleReverse);
   mPlaybackStyleDropdown->AddLabel("half reverse", (int)PlaybackStyle::HalfReverse);
}

BufferShuffler::~BufferShuffler()
{
}

void BufferShuffler::Process(double time)
{
   PROFILER(BufferShuffler);

   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   SyncBuffers();
   mInputBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());

   int bufferSize = target->GetBuffer()->BufferSize();

   if (mEnabled)
   {
      int writePosition = GetWritePositionInSamples(time);

      for (int i = 0; i < bufferSize; ++i)
      {
         ComputeSliders(0);

         if (mPlaybackSampleStartTime != -1 && time >= mPlaybackSampleStartTime)
         {
            int numSlices = GetNumSlices();
            int slicePosIndex = mQueuedSlice;
            if (mQueuedPlaybackStyle != PlaybackStyle::None)
               mPlaybackStyle = mQueuedPlaybackStyle;
            mQueuedPlaybackStyle = PlaybackStyle::None;
            if (GetSlicePlaybackRate() < 0)
               slicePosIndex += 1;
            float slicePos = (slicePosIndex % numSlices) / (float)numSlices;
            mPlaybackSample = int(GetLengthInSamples() * slicePos);
            mPlaybackSampleStartTime = -1;
            mSwitchAndRamp.StartSwitch();
         }

         if (mPlaybackSampleStopTime != -1 && time >= mPlaybackSampleStopTime)
         {
            mPlaybackSample = -1;
            mPlaybackSampleStopTime = -1;
            mSwitchAndRamp.StartSwitch();
         }

         for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
         {
            if (!mFreezeInput)
               mInputBuffer.GetChannel(ch)[writePosition] = GetBuffer()->GetChannel(ch)[i];

            float outputSample = GetBuffer()->GetChannel(ch)[i];
            if (mPlaybackSample != -1)
               outputSample = GetInterpolatedSample(mPlaybackSample, mInputBuffer.GetChannel(ch), GetLengthInSamples());
            else if (mFreezeInput)
               outputSample = mInputBuffer.GetChannel(ch)[writePosition];
            GetBuffer()->GetChannel(ch)[i] = mSwitchAndRamp.Process(ch, outputSample);
         }

         if (mPlaybackSample != -1)
            mPlaybackSample = FloatWrap(mPlaybackSample + GetSlicePlaybackRate(), GetLengthInSamples());

         writePosition = (writePosition + 1) % GetLengthInSamples();

         time += gInvSampleRateMs;
      }
   }

   for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
   {
      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void BufferShuffler::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mNumBarsSlider->Draw();
   mIntervalSelector->Draw();
   mFreezeInputCheckbox->Draw();
   mPlaybackStyleDropdown->Draw();

   DrawBuffer(5, 20, mWidth - 10, mHeight - 28);
}

void BufferShuffler::DrawBuffer(float x, float y, float w, float h)
{
   ofPushMatrix();
   ofTranslate(x, y);
   DrawAudioBuffer(w, h, &mInputBuffer, 0, GetLengthInSamples(), -1);
   ofPopMatrix();

   ofPushStyle();
   ofFill();

   float writePosX = x + GetWritePositionInSamples(gTime) / (float)GetLengthInSamples() * w;
   ofSetColor(200, 200, 200);
   ofCircle(writePosX, y, 3);
   if (mPlaybackSample != -1)
   {
      float playPosX = x + mPlaybackSample / (float)GetLengthInSamples() * w;
      ofSetColor(0, 255, 0);
      ofLine(playPosX, y, playPosX, y + h);
   }

   ofSetColor(255, 255, 255, 35);
   int numSlices = GetNumSlices();
   for (int i = 0; i < numSlices; i += 2)
      ofRect(x + i * w / numSlices, y, w / numSlices, h);

   ofPopStyle();
}

void BufferShuffler::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mQueuedSlice = pitch;
      mQueuedPlaybackStyle = VelocityToPlaybackStyle(velocity);
      mPlaybackSampleStartTime = time;
      mPlaybackSampleStopTime = -1;
   }
   else
   {
      if (mQueuedSlice == pitch)
      {
         mQueuedSlice = -1;
         mPlaybackSampleStopTime = time;
      }
   }
}

int BufferShuffler::GetNumSlices()
{
   return TheTransport->CountInStandardMeasure(mInterval) * mNumBars;
}

void BufferShuffler::OnClicked(float x, float y, bool right)
{
   if (!right && x >= 5 && x <= mWidth - 5 && y > 20)
   {
      float bufferWidth = mWidth - 10;
      float pos = (x - 5) / bufferWidth;
      int slice = int(pos * GetNumSlices());
      PlayOneShot(slice);
   }
}

void BufferShuffler::PlayOneShot(int slice)
{
   mQueuedSlice = slice;
   double sliceSizeMs = TheTransport->GetMeasureFraction(mInterval) * TheTransport->MsPerBar();
   double currentTime = NextBufferTime(false);
   double remainderMs;
   TransportListenerInfo timeInfo(nullptr, mInterval, OffsetInfo(0, false), false);
   TheTransport->GetQuantized(currentTime, &timeInfo, &remainderMs);
   double timeUntilNextInterval = sliceSizeMs - remainderMs;
   mPlaybackSampleStartTime = currentTime + timeUntilNextInterval;
   mPlaybackSampleStopTime = mPlaybackSampleStartTime + sliceSizeMs / abs(GetSlicePlaybackRate());
}

int BufferShuffler::GetWritePositionInSamples(double time)
{
   return GetLengthInSamples() / mNumBars * ((TheTransport->GetMeasure(time) % mNumBars) + TheTransport->GetMeasurePos(time));
}

int BufferShuffler::GetLengthInSamples()
{
   return mNumBars * TheTransport->MsPerBar() * gSampleRateMs;
}

BufferShuffler::PlaybackStyle BufferShuffler::VelocityToPlaybackStyle(int velocity) const
{
   if (velocity > 100)
      return PlaybackStyle::Normal;
   if (velocity > 80)
      return PlaybackStyle::Double;
   if (velocity > 60)
      return PlaybackStyle::Half;
   if (velocity > 40)
      return PlaybackStyle::Reverse;
   if (velocity > 20)
      return PlaybackStyle::DoubleReverse;
   else
      return PlaybackStyle::HalfReverse;
}

float BufferShuffler::GetSlicePlaybackRate() const
{
   switch (mPlaybackStyle)
   {
      case PlaybackStyle::Normal:
         return 1;
      case PlaybackStyle::Double:
         return 2;
      case PlaybackStyle::Half:
         return .5f;
      case PlaybackStyle::Reverse:
         return -1;
      case PlaybackStyle::DoubleReverse:
         return -2;
      case PlaybackStyle::HalfReverse:
         return -.5f;
      default:
         return 1;
   }
}

bool BufferShuffler::OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue)
{
   if (type == kMidiMessage_Note)
   {
      if (controlIndex >= 36 && controlIndex <= 99)
      {
         int gridIndex = controlIndex - 36;
         int x = gridIndex % 8;
         int y = 7 - gridIndex / 8;
         int index = x + y * 8;

         if (y == 7)
         {
            if (midiValue > 0 && x < 5)
               mPlaybackStyle = PlaybackStyle(x + 1);
            else
               mPlaybackStyle = PlaybackStyle::Normal;
         }
         else if (index < GetNumSlices() && midiValue > 0)
         {
            PlayOneShot(index);
         }

         return true;
      }
   }

   return false;
}

void BufferShuffler::UpdatePush2Leds(Push2Control* push2)
{
   for (int x = 0; x < 8; ++x)
   {
      for (int y = 0; y < 8; ++y)
      {
         int pushColor = 0;
         int index = x + y * 8;
         int writeSlice = GetWritePositionInSamples(gTime) * GetNumSlices() / GetLengthInSamples();
         int playSlice = mPlaybackSample * GetNumSlices() / GetLengthInSamples();
         if (y == 7)
         {
            if (x < 5)
               pushColor = (x == (int)mPlaybackStyle - 1) ? 2 : 1;
         }
         else if (index < GetNumSlices())
         {
            if (index == mQueuedSlice && mPlaybackSampleStartTime != -1)
               pushColor = 32;
            else if (mPlaybackSample >= 0 && index == playSlice)
               pushColor = 126;
            else if (mPlaybackSample == -1 && index == writeSlice)
               pushColor = 120;
            else
               pushColor = 16;
         }

         push2->SetLed(kMidiMessage_Note, x + (7 - y) * 8 + 36, pushColor);
      }
   }
}

bool BufferShuffler::DrawToPush2Screen()
{
   DrawBuffer(371, 10, 400, 60);
   return false;
}

void BufferShuffler::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BufferShuffler::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void BufferShuffler::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;

   out << gSampleRate;
   out << GetLengthInSamples();
   out << mInputBuffer.NumActiveChannels();
   mInputBuffer.Save(out, GetLengthInSamples());
}

void BufferShuffler::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);
   if (rev < 0)
      return;

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;
   in >> mHeight;
   Resize(mWidth, mHeight);

   int savedSampleRate;
   in >> savedSampleRate;
   int savedLength;
   in >> savedLength;
   int savedChannelCount;
   in >> savedChannelCount;
   if (savedSampleRate == gSampleRate)
   {
      mInputBuffer.Load(in, savedLength, ChannelBuffer::LoadMode::kAnyBufferSize);
   }
   else
   {
      ChannelBuffer readBuffer(savedLength);
      readBuffer.Load(in, savedLength, ChannelBuffer::LoadMode::kAnyBufferSize);

      float sampleRateRatio = (float)gSampleRate / savedSampleRate;
      int adjustedLength = savedLength * sampleRateRatio;
      mInputBuffer.SetNumActiveChannels(savedChannelCount);
      for (int ch = 0; ch < savedChannelCount; ++ch)
      {
         float* destBuffer = mInputBuffer.GetChannel(ch);
         float* srcBuffer = readBuffer.GetChannel(ch);
         for (int i = 0; i < adjustedLength; ++i)
            destBuffer[i] = GetInterpolatedSample(i / sampleRateRatio, srcBuffer, savedLength);
      }
   }
}
