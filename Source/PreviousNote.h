//
//  PreviousNote.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__PreviousNote__
#define __Bespoke__PreviousNote__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class PreviousNote : public NoteEffectBase, public IDrawableModule
{
public:
   PreviousNote();
   static IDrawableModule* Create() { return new PreviousNote(); }
   
   string GetTitleLabel() override { return "previous note"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   int mNote;
   int mVelocity;
};

#endif /* defined(__Bespoke__PreviousNote__) */
