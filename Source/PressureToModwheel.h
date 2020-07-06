//
//  PressureToModwheel.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__PressureToModwheel__
#define __Bespoke__PressureToModwheel__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ModulationChain.h"

class PressureToModwheel : public NoteEffectBase, public IDrawableModule
{
public:
   PressureToModwheel();
   virtual ~PressureToModwheel();
   static IDrawableModule* Create() { return new PressureToModwheel(); }
   
   string GetTitleLabel() override { return "pressure to modwheel"; }
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 120; height = 0; }
   bool Enabled() const override { return mEnabled; }
};

#endif /* defined(__Bespoke__PressureToModwheel__) */
