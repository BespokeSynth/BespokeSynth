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

class Stutter : public IAudioEffect, public ITimeListener, public IIntSliderListener
{
public:
   Stutter();
   ~Stutter();
   
   static IAudioEffect* Create() { return new Stutter(); }
   
   string GetTitleLabel() override { return "stutter"; }
   void CreateUIControls() override;
   
   void DrawStutterBuffer(float x, float y, float width, float height);
   void StartStutter(StutterParams stutter);
   void EndStutter(StutterParams stutter);
   
   //IAudioEffect
   void ProcessAudio(double time, ChannelBuffer* buffer) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "stutter"; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;
   
   float mFreeStutterLength;
   float mFreeStutterSpeed;
   
private:
   void DoCapture();
   void UpdateEnabled();
   float GetStutterSampleWithWraparoundBlend(int pos, int ch);
   void DoStutter(StutterParams stutter);
   void StopStutter();
   float GetBufferReadPos(float stutterPos);

   //IDrawableModule
   void GetModuleDimensions(float& width, float& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   RollingBuffer mRecordBuffer;
   ChannelBuffer mStutterBuffer;
   
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
   list<StutterParams> mStutterStack;
   Ramp mStutterLengthRamp;
   bool mFadeStutter;
   Checkbox* mFadeCheckbox;
};


#endif /* defined(__modularSynth__Stutter__) */

