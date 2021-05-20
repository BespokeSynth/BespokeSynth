/*
  ==============================================================================

    NoteCounter.h
    Created: 24 Apr 2021 3:47:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"

class NoteCounter : public IDrawableModule, public INoteSource, public ITimeListener, public IIntSliderListener, public IDropdownListener
{
public:
   NoteCounter();
   ~NoteCounter();
   static IDrawableModule* Create() { return new NoteCounter(); }
   
   string GetTitleLabel() override { return "note counter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   float mWidth;
   float mHeight;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   int mStart;
   IntSlider* mStartSlider;
   int mLength;
   IntSlider* mLengthSlider;
   int mStep;
   Checkbox* mSyncCheckbox;
   bool mSync;
   int mCustomDivisor;
   IntSlider* mCustomDivisorSlider;
   bool mRandom;
   Checkbox* mRandomCheckbox;
   
   TransportListenerInfo* mTransportListenerInfo;
};
