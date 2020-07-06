//
//  NoteFilter.h
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#pragma once

#include <stdio.h>
#include "NoteEffectBase.h"

class NoteFilter : public NoteEffectBase, public IDrawableModule
{
public:
   NoteFilter();
   virtual ~NoteFilter();
   static IDrawableModule* Create() { return new NoteFilter(); }
   
   string GetTitleLabel() override { return "note filter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   bool mGate[128];
   float mLastPlayTime[128];
   vector<Checkbox*> mGateCheckboxes;
   int mMinPitch;
   int mMaxPitch;
};
