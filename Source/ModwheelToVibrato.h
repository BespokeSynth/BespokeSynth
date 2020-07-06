//
//  ModwheelToVibrato.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__ModwheelToVibrato__
#define __Bespoke__ModwheelToVibrato__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "DropdownList.h"

class ModwheelToVibrato : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IDropdownListener
{
public:
   ModwheelToVibrato();
   virtual ~ModwheelToVibrato();
   static IDrawableModule* Create() { return new ModwheelToVibrato(); }
   
   string GetTitleLabel() override { return "modwheel to vibrato"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 138; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   NoteInterval mVibratoInterval;
   DropdownList* mIntervalSelector;
   float mVibratoAmount;
   FloatSlider* mVibratoSlider;
   
   Modulations mModulation;
};


#endif /* defined(__Bespoke__ModwheelToVibrato__) */
