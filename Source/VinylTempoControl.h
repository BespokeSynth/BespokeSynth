//
//  VinylTempoControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/18/14.
//
//

#ifndef __Bespoke__VinylTempoControl__
#define __Bespoke__VinylTempoControl__

#include <iostream>
#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IModulator.h"
#include "vinylcontrol/vinylcontrol.h"

class VinylTempoControl;

extern VinylTempoControl* TheVinylTempoControl;

class VinylTempoControl : public IDrawableModule, public IAudioProcessor, public IModulator
{
public:
   VinylTempoControl();
   ~VinylTempoControl();
   static IDrawableModule* Create() { return new VinylTempoControl(); }
   static bool CanCreate() { return TheVinylTempoControl == nullptr; }
   
   string GetTitleLabel() override { return "vinylcontrol"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void CreateUIControls() override;
   
   void Process(double time) override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   bool CanAdjustRange() const override { return false; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   bool CanStartVinylControl();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   bool mUseVinylControl;
   Checkbox* mUseVinylControlCheckbox;
   float mReferencePitch;
   vinylcontrol mVinylControl;
   //float* mModulationBuffer;
   float mSpeed;
};

#endif /* defined(__Bespoke__VinylTempoControl__) */
