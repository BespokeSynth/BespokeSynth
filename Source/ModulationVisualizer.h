//
//  ModulationVisualizer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/15.
//
//

#ifndef __Bespoke__ModulationVisualizer__
#define __Bespoke__ModulationVisualizer__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class ModulationVisualizer : public NoteEffectBase, public IDrawableModule
{
public:
   ModulationVisualizer();
   static IDrawableModule* Create() { return new ModulationVisualizer(); }
   
   string GetTitleLabel() override { return "modulation visualizer"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 300; height = 100; }
   bool Enabled() const override { return mEnabled; }
   
   struct VizVoice
   {
      VizVoice() : mActive(false), mPitchBend(nullptr), mModWheel(nullptr), mPressure(nullptr) {}
      string GetInfoString();
      bool mActive;
      ModulationChain* mPitchBend;
      ModulationChain* mModWheel;
      ModulationChain* mPressure;
   };
   
   VizVoice mGlobalModulation;
   vector<VizVoice> mVoices;
};

#endif /* defined(__Bespoke__ModulationVisualizer__) */
