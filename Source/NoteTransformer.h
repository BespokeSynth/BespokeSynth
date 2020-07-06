//
//  NoteTransformer.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/13/14.
//
//

#ifndef __modularSynth__NoteTransformer__
#define __modularSynth__NoteTransformer__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class NoteTransformer : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   NoteTransformer();
   ~NoteTransformer();
   static IDrawableModule* Create() { return new NoteTransformer(); }
   
   string GetTitleLabel() override { return "transformer"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 120; height = 135; }
   bool Enabled() const override { return mEnabled; }
   
   
   
   int mToneMod[7];
   IntSlider* mToneModSlider[7];
   double mLastTimeTonePlayed[7];
   int mLastNoteOnForPitch[127];
};


#endif /* defined(__modularSynth__NoteTransformer__) */
