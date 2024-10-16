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
//  Stutter.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/24/12.
//
//

#pragma once

#include "MidiDevice.h"
#include "RollingBuffer.h"
#include "Checkbox.h"
#include "Ramp.h"
#include "Transport.h"
#include "Slider.h"
#include "JumpBlender.h"

#define STUTTER_BLEND_WRAPAROUND_SAMPLES 100
#define STUTTER_START_BLEND_MS 3
#define STUTTER_BUFFER_SIZE 5 * gSampleRate

class Looper;
class PatchCableSource;

struct StutterParams
{
   StutterParams() {}
   StutterParams(NoteInterval _interval, float _speed)
   : interval(_interval)
   , speedStart(_speed)
   , speedEnd(_speed)
   {}
   StutterParams(NoteInterval _interval, float _speedStart, float _speedEnd, float _speedBlendTime)
   : interval(_interval)
   , speedStart(_speedStart)
   , speedEnd(_speedEnd)
   , speedBlendTime(_speedBlendTime)
   {}
   bool operator==(const StutterParams& other) const
   {
      return interval == other.interval &&
             speedStart == other.speedStart &&
             speedEnd == other.speedEnd &&
             speedBlendTime == other.speedBlendTime;
   }

   NoteInterval interval{ NoteInterval::kInterval_16n };
   float speedStart{ 1 };
   float speedEnd{ 1 };
   float speedBlendTime{ 0 };
};

class Stutter : public ITimeListener
{
public:
   Stutter();
   ~Stutter();

   void Init();

   void DrawStutterBuffer(float x, float y, float width, float height);
   void StartStutter(double time, StutterParams stutter);
   void EndStutter(double time, StutterParams stutter);
   void SetEnabled(double time, bool enabled);

   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer);

   //ITimeListener
   void OnTimeEvent(double time) override;

   float mFreeStutterLength{ .1 };
   float mFreeStutterSpeed{ 1 };

private:
   void DoCapture();
   float GetStutterSampleWithWraparoundBlend(int pos, int ch);
   void DoStutter(double time, StutterParams stutter);
   void StopStutter(double time);
   float GetBufferReadPos(float stutterPos);

   RollingBuffer mRecordBuffer;
   ChannelBuffer mStutterBuffer;

   bool mEnabled{ true };
   bool mStuttering{ false };
   int mCaptureLength{ 1 };
   int mStutterLength{ 1 };
   Ramp mStutterSpeed;
   float mStutterPos{ 0 };
   bool mAutoStutter{ false };
   Checkbox* mAutoCheckbox{ nullptr };
   Ramp mBlendRamp;
   static bool sQuantize;
   ofMutex mMutex;
   StutterParams mCurrentStutter;
   static int sStutterSubdivide;
   IntSlider* mSubdivideSlider{ nullptr };
   JumpBlender mJumpBlender[ChannelBuffer::kMaxNumChannels]{};
   int mNanopadScene{ 0 };
   std::list<StutterParams> mStutterStack;
   Ramp mStutterLengthRamp;
   bool mFadeStutter{ false };
   Checkbox* mFadeCheckbox{ nullptr };
};
