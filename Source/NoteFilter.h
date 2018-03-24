//
//  NoteFilter.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#ifndef __Bespoke__NoteFilter__
#define __Bespoke__NoteFilter__

#include <stdio.h>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class NoteFilter : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   NoteFilter();
   static IDrawableModule* Create() { return new NoteFilter(); }
   
   string GetTitleLabel() override { return "note filter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 90; height = 38; }
   bool Enabled() const override { return mEnabled; }
   
   int mMinPitch;
   IntSlider* mMinPitchSlider;
   int mMaxPitch;
   IntSlider* mMaxPitchSlider;
};

#endif /* defined(__Bespoke__NoteFilter__) */
