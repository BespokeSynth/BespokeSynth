//
//  NoteGate.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#ifndef __Bespoke__NoteGate__
#define __Bespoke__NoteGate__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class NoteGate : public NoteEffectBase, public IDrawableModule
{
public:
   NoteGate();
   virtual ~NoteGate();
   static IDrawableModule* Create() { return new NoteGate(); }
   
   string GetTitleLabel() override { return "note gate"; }
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
   
   bool mGate;
   Checkbox* mGateCheckbox;
};


#endif /* defined(__Bespoke__NoteGate__) */
