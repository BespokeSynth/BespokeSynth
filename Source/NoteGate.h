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
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return true; }
   
   bool mGate;
   Checkbox* mGateCheckbox;
   std::array<NoteInputElement, 128> mActiveNotes{ false };
   std::array<NoteInputElement, 128> mPendingNotes{ false };
};


#endif /* defined(__Bespoke__NoteGate__) */
