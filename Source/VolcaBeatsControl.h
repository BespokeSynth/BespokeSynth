/*
  ==============================================================================

    VolcaBeatsControl.h
    Created: 28 Jan 2017 10:48:13pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifndef VOLCABEATSCONTROL_H_INCLUDED
#define VOLCABEATSCONTROL_H_INCLUDED

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Transport.h"

class VolcaBeatsControl : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   VolcaBeatsControl();
   virtual ~VolcaBeatsControl();
   static IDrawableModule* Create() { return new VolcaBeatsControl(); }
   
   string GetTitleLabel() override { return "volca beats control"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 263; height = 170; }
   bool Enabled() const override { return mEnabled; }
   
   float mClapSpeed;
   float mClaveSpeed;
   float mAgogoSpeed;
   float mCrashSpeed;
   float mStutterTime;
   float mStutterDepth;
   float mTomDecay;
   float mClosedHatDecay;
   float mOpenHatDecay;
   float mHatGrain;
   
   FloatSlider* mClapSpeedSlider;
   FloatSlider* mClaveSpeedSlider;
   FloatSlider* mAgogoSpeedSlider;
   FloatSlider* mCrashSpeedSlider;
   FloatSlider* mStutterTimeSlider;
   FloatSlider* mStutterDepthSlider;
   FloatSlider* mTomDecaySlider;
   FloatSlider* mClosedHatDecaySlider;
   FloatSlider* mOpenHatDecaySlider;
   FloatSlider* mHatGrainSlider;
   
   float mLevels[10];
   FloatSlider* mLevelSliders[10];
};


#endif  // VOLCABEATSCONTROL_H_INCLUDED
