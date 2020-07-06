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

struct ModulationParameters;

class ModulationVisualizer : public NoteEffectBase, public IDrawableModule
{
public:
   ModulationVisualizer();
   static IDrawableModule* Create() { return new ModulationVisualizer(); }
   
   string GetTitleLabel() override { return "modulation visualizer"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 300; height = 100; }
   bool Enabled() const override { return mEnabled; }
   
   struct VizVoice
   {
      VizVoice() : mActive(false) {}
      string GetInfoString();
      bool mActive;
      ModulationParameters mModulators;
   };
   
   VizVoice mGlobalModulation;
   vector<VizVoice> mVoices;
};

#endif /* defined(__Bespoke__ModulationVisualizer__) */
