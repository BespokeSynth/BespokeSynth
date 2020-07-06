//
//  Pressure.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__Pressure__
#define __Bespoke__Pressure__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"

class Pressure : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   Pressure();
   virtual ~Pressure();
   static IDrawableModule* Create() { return new Pressure(); }
   
   string GetTitleLabel() override { return "pressure"; }
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
   
   float mPressure;
   FloatSlider* mPressureSlider;
   
   Modulations mModulation;
};

#endif /* defined(__Bespoke__Pressure__) */
