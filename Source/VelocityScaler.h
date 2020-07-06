//
//  VelocityScaler.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/6/16.
//
//

#ifndef __Bespoke__VelocityScaler__
#define __Bespoke__VelocityScaler__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"

class VelocityScaler : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   VelocityScaler();
   static IDrawableModule* Create() { return new VelocityScaler(); }
   
   string GetTitleLabel() override { return "velocity scaler"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 108; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   float mScale;
   FloatSlider* mScaleSlider;
};


#endif /* defined(__Bespoke__VelocityScaler__) */
