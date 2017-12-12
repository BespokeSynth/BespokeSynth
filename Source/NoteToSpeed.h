/*
  ==============================================================================

    NoteToSpeed.h
    Created: 3 Dec 2017 11:14:10pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IModulator.h"
#include "TextEntry.h"

class PatchCableSource;

class NoteToSpeed : public IDrawableModule, public INoteReceiver, public IModulator, public ITextEntryListener
{
public:
   NoteToSpeed();
   virtual ~NoteToSpeed();
   static IDrawableModule* Create() { return new NoteToSpeed(); }
   
   string GetTitleLabel() override { return "note to speed"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
   void TextEntryComplete(TextEntry* text) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   float mPitch;
   ModulationChain* mPitchBend;
   
   int mBasePitch;
   TextEntry* mBasePitchEntry;
};
