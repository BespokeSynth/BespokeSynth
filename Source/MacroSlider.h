//
//  MacroSlider.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/15.
//
//

#ifndef __Bespoke__MacroSlider__
#define __Bespoke__MacroSlider__

#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;
class IUIControl;

class MacroSlider : public IDrawableModule, public IFloatSliderListener
{
public:
   MacroSlider();
   virtual ~MacroSlider();
   static IDrawableModule* Create() { return new MacroSlider(); }
   
   string GetTitleLabel() override { return "macroslider"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   float GetValue() const { return mValue; }
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   const static int kMappingSpacing = 32;
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 25+(int)mMappings.size()*kMappingSpacing; }
   bool Enabled() const override { return mEnabled; }
   
   struct Mapping : public IModulator
   {
      Mapping(MacroSlider* owner, int index);
      ~Mapping();
      void CreateUIControls();
      void UpdateControl();
      void Draw();
      PatchCableSource* GetCableSource() const { return mTargetCable; }
      
      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->Enabled(); }
      
      MacroSlider* mOwner;
      int mIndex;
   };
   
   FloatSlider* mSlider;
   float mValue;
   vector<Mapping*> mMappings;
};

#endif /* defined(__Bespoke__MacroSlider__) */
