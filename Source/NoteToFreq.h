//
//  NoteToFreq.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/17/15.
//
//

#ifndef __Bespoke__NoteToFreq__
#define __Bespoke__NoteToFreq__

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "IModulator.h"

class PatchCableSource;

class NoteToFreq : public IDrawableModule, public INoteReceiver, public IModulator
{
public:
   NoteToFreq();
   virtual ~NoteToFreq();
   static IDrawableModule* Create() { return new NoteToFreq(); }
   
   string GetTitleLabel() override { return "notetofreq"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 0; }
   bool Enabled() const override { return mEnabled; }
   
   float mPitch;
   ModulationChain* mPitchBend;
};

#endif /* defined(__Bespoke__NoteToFreq__) */
