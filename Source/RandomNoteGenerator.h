//
//  RandomNoteGenerator.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#ifndef __Bespoke__RandomNoteGenerator__
#define __Bespoke__RandomNoteGenerator__

#include <stdio.h>
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"

class RandomNoteGenerator : public IDrawableModule, public INoteSource, public ITimeListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   RandomNoteGenerator();
   ~RandomNoteGenerator();
   static IDrawableModule* Create() { return new RandomNoteGenerator(); }
   
   string GetTitleLabel() override { return "random note"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=92; }
   bool Enabled() const override { return mEnabled; }
   
   
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   float mProbability;
   FloatSlider* mProbabilitySlider;
   int mPitch;
   IntSlider* mPitchSlider;
   float mVelocity;
   FloatSlider* mVelocitySlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mSkip;
   IntSlider* mSkipSlider;
   int mSkipCount;
};

#endif /* defined(__Bespoke__RandomNoteGenerator__) */
