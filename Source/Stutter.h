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

#include "IAudioProcessor.h"
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
class StutterControl;
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

class Stutter : public IAudioProcessor, public ITimeListener, public IIntSliderListener
{
public:
   Stutter();
   ~Stutter();
   
   static IAudioProcessor* Create() { return new Stutter(); }
   
   string GetTitleLabel() override { return "stutter"; }
   void CreateUIControls() override;
   
   void StartStutter(StutterParams stutter);
   void EndStutter(StutterParams stutter);
   
   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "stutter"; }
   
   //ITimeListener
   void OnTimeEvent(int samplesTo) override;
   
   void PostRepatch(PatchCableSource* cableSource) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& info) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& info) override;
private:
   void DoCapture();
   void UpdateEnabled();
   float GetStutterSampleWithWraparoundBlend(int pos);
   void DoStutter(StutterParams stutter);
   void StopStutter();
   void SetController(StutterControl* controller);
   float GetBufferReadPos(float stutterPos);

   //IDrawableModule
   void GetModuleDimensions(int& width, int& height) override;
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   
   RollingBuffer mRecordBuffer;
   float* mStutterBuffer;
   
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
   JumpBlender mJumpBlender;
   int mNanopadScene;
   StutterControl* mControl;
   list<StutterParams> mStutterStack;
   Ramp mStutterLengthRamp;
   bool mFadeStutter;
   Checkbox* mFadeCheckbox;
   
   PatchCableSource* mStutterControlCable;
};


#endif /* defined(__modularSynth__Stutter__) */

