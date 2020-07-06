//
//  PitchDive.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#ifndef __Bespoke__PitchDive__
#define __Bespoke__PitchDive__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"

class PitchDive : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   PitchDive();
   virtual ~PitchDive();
   static IDrawableModule* Create() { return new PitchDive(); }
   
   string GetTitleLabel() override { return "pitchdive"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 120; height = 40; }
   bool Enabled() const override { return mEnabled; }
   
   float mStart;
   FloatSlider* mStartSlider;
   float mTime;
   FloatSlider* mTimeSlider;

   Modulations mModulation;
};

#endif /* defined(__Bespoke__PitchDive__) */
