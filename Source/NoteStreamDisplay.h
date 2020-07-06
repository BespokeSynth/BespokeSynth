/*
  ==============================================================================

    NoteStreamDisplay.h
    Created: 21 May 2020 11:13:12pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "ClickButton.h"

class NoteStreamDisplay : public NoteEffectBase, public IDrawableModule, public IButtonListener
{
public:
   NoteStreamDisplay();
   static IDrawableModule* Create() { return new NoteStreamDisplay(); }
   void CreateUIControls() override;
   
   string GetTitleLabel() override { return "note stream"; }
   
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override { w=mWidth; h=mHeight; }
   bool Enabled() const override { return true; }
   bool IsElementActive(int index) const;
   float GetYPos(int pitch, float noteHeight) const;
   
   struct NoteStreamElement
   {
      int pitch;
      int velocity;
      double timeOn;
      double timeOff;
   };
   
   static const int kNoteStreamCapacity = 50;
   NoteStreamElement mNoteStream[kNoteStreamCapacity];
   float mWidth;
   float mHeight;
   float mDurationMs;
   int mPitchMin;
   int mPitchMax;
   ClickButton* mResetButton;
};
