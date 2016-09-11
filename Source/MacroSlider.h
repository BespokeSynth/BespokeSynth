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
#include "Transport.h"
#include <mutex>

class PatchCableSource;
class IUIControl;

class MacroSlider : public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   MacroSlider();
   virtual ~MacroSlider();
   static IDrawableModule* Create() { return new MacroSlider(); }
   
   string GetTitleLabel() override { return "macroslider"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void PostLoadState() override;
private:
   const static int kMappingSpacing = 32;
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 25+mMappings.size()*kMappingSpacing; }
   bool Enabled() const override { return mEnabled; }
   
   void UpdateValues();
   
   struct Mapping
   {
      Mapping(MacroSlider* owner, int index);
      ~Mapping();
      void CreateUIControls();
      void UpdateControl();
      void UpdateValue(float value);
      void Draw();
      
      FloatSlider* mStartSlider;
      FloatSlider* mEndSlider;
      float mStart;
      float mEnd;
      PatchCableSource* mCableSource;
      FloatSlider* mControl;
      MacroSlider* mOwner;
      int mIndex;
   };
   
   FloatSlider* mSlider;
   float mValue;
   recursive_mutex mMappingMutex;
   vector<Mapping*> mMappings;
};

#endif /* defined(__Bespoke__MacroSlider__) */
