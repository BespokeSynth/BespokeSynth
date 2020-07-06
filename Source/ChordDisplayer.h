/*
  ==============================================================================

    ChordDisplayer.h
    Created: 27 Mar 2018 9:23:27pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class ChordDisplayer : public NoteEffectBase, public IDrawableModule
{
public:
   ChordDisplayer();
   static IDrawableModule* Create() { return new ChordDisplayer(); }
   
   string GetTitleLabel() override { return "chord displayer"; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 200; height = 20; }
   bool Enabled() const override { return true; }
};
