/*
  ==============================================================================

    NoteHocket.h
    Created: 19 Dec 2019 10:40:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "RadioButton.h"

class NoteHocket : public INoteReceiver, public INoteSource, public IDrawableModule, public IRadioButtonListener
{
public:
   NoteHocket();
   static IDrawableModule* Create() { return new NoteHocket(); }
   
   string GetTitleLabel() override { return "hocket"; }
   void CreateUIControls() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   
   void AddReceiver(INoteReceiver* receiver, const char* name);
   void SelectNewReceiver();

   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return true; }

   int mCurrentReceiver;
   RadioButton* mRouteSelector;
   vector<INoteReceiver*> mReceivers;
};
