//
//  Presets.h
//  modularSynth
//
//  Created by Ryan Challinor on 7/29/13.
//
//

#ifndef __modularSynth__Presets__
#define __modularSynth__Presets__

#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "FloatSliderLFOControl.h"
#include "Transport.h"
#include "Slider.h"
#include "Ramp.h"
#include "DropdownList.h"

class Presets : public IDrawableModule, public IButtonListener, public IAudioPoller, public IFloatSliderListener, public IDropdownListener
{
public:
   Presets();
   virtual ~Presets();
   static IDrawableModule* Create() { return new Presets(); }
   
   string GetTitleLabel() override { return "presets"; }
   void CreateUIControls() override;
   
   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   void OnTransportAdvanced(float amount) override;

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;
   
   static vector<IUIControl*> sPresetHighlightControls;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   void SetPreset(int idx);
   void Store(int idx);
   void UpdateGridValues();
   void Save();
   void Load();
   void SetGridSize(float w, float h);

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   
   struct Preset
   {
      Preset() {}
      Preset(string path, float val) : mControlPath(path), mValue(val), mHasLFO(false) {}
      Preset(IUIControl* control);
      string mControlPath;
      float mValue;
      bool mHasLFO;
      LFOSettings mLFOSettings;
   };
   
   struct PresetCollection
   {
      std::vector<Preset> mPresets;
      string mDescription;
   };
   
   struct ControlRamp
   {
      IUIControl* mUIControl;
      Ramp mRamp;
   };
   
   UIGrid* mGrid;
   std::vector<PresetCollection> mPresetCollection;
   ClickButton* mSaveButton;
   int mDrawSetPresetsCountdown;
   vector<IDrawableModule*> mPresetModules;
   vector<IUIControl*> mPresetControls;
   bool mBlending;
   float mBlendTime;
   FloatSlider* mBlendTimeSlider;
   float mBlendProgress;
   vector<ControlRamp> mBlendRamps;
   ofMutex mRampMutex;
   int mCurrentPreset;
   DropdownList* mCurrentPresetSelector;
   PatchCableSource* mModuleCable;
   PatchCableSource* mUIControlCable;
};


#endif /* defined(__modularSynth__Presets__) */

