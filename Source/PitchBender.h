//
//  PitchBender.h
//  Bespoke
//
//  Created by Ryan Challinor on 9/7/14.
//
//

#ifndef __Bespoke__PitchBender__
#define __Bespoke__PitchBender__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "Transport.h"

class PitchBender : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   PitchBender();
   virtual ~PitchBender();
   static IDrawableModule* Create() { return new PitchBender(); }
   
   string GetTitleLabel() override { return "pitchbend"; }
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
   
   float mBend;
   FloatSlider* mBendSlider;
   float mRange;
   //Checkbox mBendingCheckbox;
   
   Modulations mModulation;
};

#endif /* defined(__Bespoke__PitchBender__) */

