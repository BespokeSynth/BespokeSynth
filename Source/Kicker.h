//
//  Kicker.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/7/13.
//
//

#ifndef __modularSynth__Kicker__
#define __modularSynth__Kicker__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "TextEntry.h"

class DrumPlayer;

class Kicker : public NoteEffectBase, public IDrawableModule
{
public:
   Kicker();
   static IDrawableModule* Create() { return new Kicker(); }
   
   string GetTitleLabel() override { return "kicker"; }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void SetDrumPlayer(DrumPlayer* drumPlayer) { mDrumPlayer = drumPlayer; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 0; }
   bool Enabled() const override { return mEnabled; }
   
   DrumPlayer* mDrumPlayer;
};

#endif /* defined(__modularSynth__Kicker__) */
