//
//  StutterControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/25/15.
//
//

#ifndef __Bespoke__StutterControl__
#define __Bespoke__StutterControl__

#include <iostream>
#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Checkbox.h"
#include "Stutter.h"
#include "Slider.h"

class StutterControl : public IDrawableModule, public IFloatSliderListener
{
public:
   StutterControl();
   ~StutterControl();
   static IDrawableModule* Create() { return new StutterControl(); }
   
   string GetTitleLabel() override { return "stutter control"; }
   void CreateUIControls() override;
   
   void KeyPressed(int key) override;
   void KeyReleased(int key) override;
   
   void AddListener(Stutter* stutter);
   void RemoveListener(Stutter* stutter);
   float GetFreeLength() const { return mFreeLength; }
   float GetFreeSpeed() const { return mFreeSpeed; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   enum StutterType
   {
      kHalf,
      kQuarter,
      k8th,
      k16th,
      k32nd,
      k64th,
      kReverse,
      kRampIn,
      kRampOut,
      kTumbleUp,
      kTumbleDown,
      kHalfSpeed,
      kDoubleSpeed,
      kQuarterTriplets,
      kDotted8th,
      kFree,
      kNumStutterTypes
   };
   
   StutterType GetStutterFromKey(int key);
   void SendStutter(StutterParams stutter, bool on);
   StutterParams GetStutter(StutterType type);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& x, int& y) override;
   
   list<Stutter*> mListeners;
   Checkbox* mStutterCheckboxes[kNumStutterTypes];
   bool mStutter[kNumStutterTypes];
   float mFreeLength;
   FloatSlider* mFreeLengthSlider;
   float mFreeSpeed;
   FloatSlider* mFreeSpeedSlider;
};

#endif /* defined(__Bespoke__StutterControl__) */
