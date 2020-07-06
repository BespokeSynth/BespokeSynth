/*
  ==============================================================================

    NoteToPulse.h
    Created: 20 Sep 2018 9:43:00pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IPulseReceiver.h"

class PatchCableSource;

class NoteToPulse : public IDrawableModule, public INoteReceiver, public IPulseSource
{
public:
   NoteToPulse();
   virtual ~NoteToPulse();
   static IDrawableModule* Create() { return new NoteToPulse(); }
   
   string GetTitleLabel() override { return "note to pulse"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 0; }
   bool Enabled() const override { return mEnabled; }
};
