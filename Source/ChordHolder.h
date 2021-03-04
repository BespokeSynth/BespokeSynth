/*
  ==============================================================================

    ChordHold.h
    Created: 3 Mar 2021 9:56:09pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ClickButton.h"

class ChordHolder : public NoteEffectBase, public IDrawableModule, public IButtonListener
{
public:
   ChordHolder();
   static IDrawableModule* Create() { return new ChordHolder(); }

   string GetTitleLabel() override { return "chord holder"; }
   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 21; }
   bool Enabled() const override { return true; }

   std::array<bool, 128> mNoteInputHeld{ false };
   std::array<bool, 128> mNotePlaying{ false };

   ClickButton* mStopButton;
};
