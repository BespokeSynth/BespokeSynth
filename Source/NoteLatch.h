/*
  ==============================================================================

    NoteLatch.h
    Created: 11 Apr 2020 3:28:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "TextEntry.h"

class NoteLatch : public NoteEffectBase, public IDrawableModule
{
public:
   NoteLatch();
   static IDrawableModule* Create() { return new NoteLatch(); }
   
   string GetTitleLabel() override { return "note latch"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
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
   
   bool mNoteState[128];
};
