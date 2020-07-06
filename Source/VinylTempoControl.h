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
#include "Transport.h"
#include "vinylcontrol/vinylcontrol.h"

class VinylTempoControl;

extern VinylTempoControl* TheVinylTempoControl;

class VinylTempoControl : public IDrawableModule, public IAudioPoller
{
public:
   VinylTempoControl();
   ~VinylTempoControl();
   static IDrawableModule* Create() { return new VinylTempoControl(); }
   static bool CanCreate() { return TheVinylTempoControl == nullptr; }
   
   string GetTitleLabel() override { return "vinylcontrol"; }
   
   void SetVinylControlInput(float* left, float* right, int numSamples);
   int GetLeftChannel() const { return mLeftChannel; }
   int GetRightChannel() const { return mRightChannel; }
   void Stop() { mUseVinylControl = false; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void CreateUIControls() override;
   
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   bool CanStartVinylControl();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 90; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   bool mUseVinylControl;
   Checkbox* mUseVinylControlCheckbox;
   float mReferencePitch;
   float mReferenceTempo;
   vinylcontrol mVinylControl;
   float* mVinylControlInLeft;
   float* mVinylControlInRight;
   int mLeftChannel;
   int mRightChannel;
};

#endif /* defined(__Bespoke__VinylTempoControl__) */
