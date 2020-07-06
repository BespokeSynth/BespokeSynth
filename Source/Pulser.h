/*
  ==============================================================================

    Sequencer.h
    Created: 17 Oct 2018 9:38:03pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"
#include "IPulseReceiver.h"

class PatchCableSource;

class Pulser : public IDrawableModule, public ITimeListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IAudioPoller, public IPulseSource
{
public:
   Pulser();
   virtual ~Pulser();
   static IDrawableModule* Create() { return new Pulser(); }
   
   string GetTitleLabel() override { return "pulser"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float GetOffset();
   
   enum TimeMode
   {
      kTimeMode_Step,
      kTimeMode_Sync,
      kTimeMode_Downbeat,
      kTimeMode_Downbeat2,
      kTimeMode_Downbeat4,
      kTimeMode_Free
   };
   
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   
   TimeMode mTimeMode;
   DropdownList* mTimeModeSelector;
   
   bool mRandomStep;
   Checkbox* mRandomStepCheckbox;
   
   bool mWaitingForDownbeat;
   float mOffset;
   FloatSlider* mOffsetSlider;
   
   FloatSlider* mFreeTimeSlider;
   float mFreeTimeStep;
   float mFreeTimeCounter;
};

