//
//  WhiteKeys.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#ifndef __modularSynth__WhiteKeys__
#define __modularSynth__WhiteKeys__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"

class WhiteKeys : public NoteEffectBase, public IDrawableModule
{
public:
   WhiteKeys();
   static IDrawableModule* Create() { return new WhiteKeys(); }
   
   string GetTitleLabel() override { return "white keys"; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 0; }
   bool Enabled() const override { return mEnabled; }
};

#endif /* defined(__modularSynth__WhiteKeys__) */
