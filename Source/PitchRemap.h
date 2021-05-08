/*
  ==============================================================================

    PitchRemap.h
    Created: 7 May 2021 10:17:52pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "TextEntry.h"

class PitchRemap : public NoteEffectBase, public IDrawableModule, public ITextEntryListener
{
public:
   PitchRemap();
   static IDrawableModule* Create() { return new PitchRemap(); }
   
   string GetTitleLabel() override { return "pitch remap"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void TextEntryComplete(TextEntry* entry) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   struct NoteInfo
   {
      NoteInfo() : mOn(false), mVelocity(0), mVoiceIdx(-1) {}
      bool mOn;
      int mVelocity;
      int mVoiceIdx;
   };
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   struct Remap
   {
      int mFromPitch;
      TextEntry* mFromPitchEntry;
      int mToPitch;
      TextEntry* mToPitchEntry;
   };
   
   float mWidth;
   float mHeight;
   std::array<Remap, 8> mRemaps;
   std::array<NoteInfo, 128> mInputNotes;
};
