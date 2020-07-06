/*
  ==============================================================================

    PulseTrain.h
    Created: 10 Mar 2020 9:15:47pm
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
#include "Slider.h"
#include "IPulseReceiver.h"
#include "UIGrid.h"

class PatchCableSource;

class PulseTrain : public IDrawableModule, public ITimeListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IAudioPoller, public IPulseSource, public IPulseReceiver, public UIGridListener
{
public:
   PulseTrain();
   virtual ~PulseTrain();
   static IDrawableModule* Create() { return new PulseTrain(); }
   
   string GetTitleLabel() override { return "pulse train"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   void Step(double time, float velocity, int flags);
   
   static const int kMaxSteps = 32;
   float mVels[kMaxSteps];
   int mLength;
   IntSlider* mLengthSlider;
   int mStep;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   bool mResetOnStart;
   Checkbox* mResetOnStartCheckbox;
   
   static const int kIndividualStepCables = 16;
   PatchCableSource* mStepCables[kIndividualStepCables];
   
   UIGrid* mVelocityGrid;
};
