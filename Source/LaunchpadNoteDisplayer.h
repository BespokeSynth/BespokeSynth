//
//  LaunchpadNoteDisplayer.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#ifndef __modularSynth__LaunchpadNoteDisplayer__
#define __modularSynth__LaunchpadNoteDisplayer__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "TextEntry.h"

class LaunchpadKeyboard;

class LaunchpadNoteDisplayer : public NoteEffectBase, public IDrawableModule
{
public:
   LaunchpadNoteDisplayer();
   static IDrawableModule* Create() { return new LaunchpadNoteDisplayer(); }
   
   string GetTitleLabel() override { return "grid display"; }

   void SetLaunchpad(LaunchpadKeyboard* launchpad) { mLaunchpad = launchpad; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
   
private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override { width = 80; height = 0; }
   bool Enabled() const override { return true; }

   LaunchpadKeyboard* mLaunchpad;
};

#endif /* defined(__modularSynth__LaunchpadNoteDisplayer__) */
