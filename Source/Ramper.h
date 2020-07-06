//
//  Ramper.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__Ramper__
#define __Bespoke__Ramper__

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"

class PatchCableSource;

class Ramper : public IDrawableModule, public IDropdownListener, public IAudioPoller, public IButtonListener, public IFloatSliderListener
{
public:
   Ramper();
   ~Ramper();
   static IDrawableModule* Create() { return new Ramper(); }
   
   string GetTitleLabel() override { return "ramper"; }
   void CreateUIControls() override;
   
   IUIControl* GetUIControl() const { return mUIControl; }
   
   //IDrawableModule
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   IUIControl* mUIControl;
   NoteInterval mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   ClickButton* mTriggerButton;
   FloatSlider* mTargetValueSlider;
   float mTargetValue;
   float mStartMeasure;
   float mStartValue;
   bool mRamping;
};

#endif /* defined(__Bespoke__Ramper__) */
