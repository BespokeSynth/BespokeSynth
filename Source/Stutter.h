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

#ifndef __modularSynth__Stutter__
#define __modularSynth__Stutter__

#include <iostream>

#include "IAudioEffect.h"
#include "MidiDevice.h"
#include "RollingBuffer.h"
#include "Checkbox.h"
#include "Ramp.h"
#include "Transport.h"
#include "Slider.h"
#include "JumpBlender.h"
#include "ofxJSONElement.h"

#define STUTTER_BLEND_WRAPAROUND_SAMPLES 100
#define STUTTER_START_BLEND_MS 3
#define STUTTER_BUFFER_SIZE 5*gSampleRate

class Looper;
class PatchCableSource;

struct StutterParams
{
   StutterParams(NoteInterval _interval, float _speed) : interval(_interval), speedStart(_speed), speedEnd(_speed), speedBlendTime(0) {}
   StutterParams(NoteInterval _interval, float _speedStart, float _speedEnd, float _speedBlendTime) : interval(_interval), speedStart(_speedStart), speedEnd(_speedEnd),speedBlendTime(_speedBlendTime) {}
   bool operator==(const StutterParams& other) const
   {
      return interval == other.interval &&
      speedStart == other.speedStart &&
      speedEnd == other.speedEnd &&
      speedBlendTime == other.speedBlendTime;
   }
   
   NoteInterval interval;
   float speedStart;
   float speedEnd;
   float speedBlendTime;
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
   void SetEnabled(bool enabled);
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer);
   
   //ITimeListener
   void OnTimeEvent(double time) override;

   float mFreeStutterLength;
   float mFreeStutterSpeed;
   
private:
   void DoCapture();
   float GetStutterSampleWithWraparoundBlend(int pos, int ch);
   void DoStutter(double time, StutterParams stutter);
   void StopStutter(double time);
   float GetBufferReadPos(float stutterPos);
   
   RollingBuffer mRecordBuffer;
   ChannelBuffer mStutterBuffer;
   
   bool mEnabled;
   bool mStuttering;
   int mCaptureLength;
   int mStutterLength;
   Ramp mStutterSpeed;
   float mStutterPos;
   bool mAutoStutter;
   Checkbox* mAutoCheckbox;
   Ramp mBlendRamp;
   static bool sQuantize;
   ofMutex mMutex;
   StutterParams mCurrentStutter;
   static int sStutterSubdivide;
   IntSlider* mSubdivideSlider;
   JumpBlender mJumpBlender[ChannelBuffer::kMaxNumChannels];
   int mNanopadScene;
   std::list<StutterParams> mStutterStack;
   Ramp mStutterLengthRamp;
   bool mFadeStutter;
   Checkbox* mFadeCheckbox;
};


#endif /* defined(__modularSynth__Stutter__) */

