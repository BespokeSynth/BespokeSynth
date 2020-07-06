//
//  ModWheel.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__ModWheel__
#define __Bespoke__ModWheel__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"

class ModWheel : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   ModWheel();
   virtual ~ModWheel();
   static IDrawableModule* Create() { return new ModWheel(); }
   
   string GetTitleLabel() override { return "modwheel"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 120; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   float mModWheel;
   FloatSlider* mModWheelSlider;
   
   Modulations mModulation;
};

#endif /* defined(__Bespoke__ModWheel__) */
