//
//  ADSRTrigger.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/14.
//
//

#ifndef __Bespoke__ADSRTrigger__
#define __Bespoke__ADSRTrigger__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "TextEntry.h"
#include "INoteSource.h"

class ADSRTrigger : public NoteEffectBase, public IDrawableModule
{
public:
   ADSRTrigger();
   static IDrawableModule* Create() { return new ADSRTrigger(); }
   
   string GetTitleLabel() override { return "ADSR trigger "+ofToString(mADSRIndex); }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 90; height = 0; }
   bool Enabled() const override { return true; }
   
   int mADSRIndex;
};

#endif /* defined(__Bespoke__ADSRTrigger__) */
