//
//  NoteRouter.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/24/13.
//
//

#ifndef __modularSynth__NoteRouter__
#define __modularSynth__NoteRouter__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "RadioButton.h"

class NoteRouter : public NoteEffectBase, public IDrawableModule, public IRadioButtonListener
{
public:
   NoteRouter();
   static IDrawableModule* Create() { return new NoteRouter(); }
   
   string GetTitleLabel() override { return "router"; }
   void CreateUIControls() override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void SetActiveIndex(int index) { mRouteMask = 1 << index; }
   void SetSelectedMask(int mask);
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IRadioButtonListener
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return true; }

   int mRouteMask;
   RadioButton* mRouteSelector;
   vector<AdditionalNoteCable*> mDestinationCables;
   bool mRadioButtonMode;
};

#endif /* defined(__modularSynth__NoteRouter__) */

