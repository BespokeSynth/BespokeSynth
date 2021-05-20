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
#include "IPulseReceiver.h"

class ChordHolder : public NoteEffectBase, public IDrawableModule, public IButtonListener, public IPulseReceiver
{
public:
   ChordHolder();
   static IDrawableModule* Create() { return new ChordHolder(); }

   string GetTitleLabel() override { return "chord holder"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 131; height = 21; }
   bool Enabled() const override { return mEnabled; }

   void Stop();

   std::array<bool, 128> mNoteInputHeld{ false };
   std::array<bool, 128> mNotePlaying{ false };

   ClickButton* mStopButton;
   bool mOnlyPlayWhenPulsed;
   Checkbox* mOnlyPlayWhenPulsedCheckbox;
};
