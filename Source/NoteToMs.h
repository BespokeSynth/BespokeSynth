//
//  NoteToMs.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/2/16.
//
//

#ifndef __Bespoke__NoteToMs__
#define __Bespoke__NoteToMs__

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "Transport.h"

class PatchCableSource;
class IUIControl;

class NoteToMs : public IDrawableModule, public INoteReceiver, public IAudioPoller
{
public:
   NoteToMs();
   virtual ~NoteToMs();
   static IDrawableModule* Create() { return new NoteToMs(); }
   
   string GetTitleLabel() override { return "note to ms"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 0; }
   bool Enabled() const override { return mEnabled; }
   
   PatchCableSource* mControlCable;
   IUIControl* mTarget;
   float mPitch;
   float mBend;
   ModulationChain* mPitchBend;
};


#endif /* defined(__Bespoke__NoteToMs__) */
