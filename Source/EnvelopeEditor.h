/*
  ==============================================================================

    EnvelopeEditor.h
    Created: 9 Nov 2017 5:20:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "ADSR.h"
#include "ADSRDisplay.h"

class EnvelopeControl
{
public:
   EnvelopeControl(ofVec2f position, ofVec2f dimensions);
   void SetADSR(::ADSR* adsr) { mAdsr = adsr; }
   void OnClicked(int x, int y, bool right);
   void MouseMoved(float x, float y);
   void MouseReleased();
   void Draw();
   void SetViewLength(float length) { mViewLength = length; }
   ofVec2f GetPosition() const { return mPosition; }
   ofVec2f GetDimensions() const { return mDimensions; }
   void SetPosition(ofVec2f pos) { mPosition = pos; }
   void SetDimensions(ofVec2f dim) { mDimensions = dim; }
   void SetFixedLengthMode(bool fixed) { mFixedLengthMode = fixed; }
private:
   void AddVertex(float x, float y);
   float GetPreSustainTime();
   float GetReleaseTime();
   float GetTimeForX(float x);
   float GetValueForY(float y);
   float GetXForTime(float time);
   float GetYForValue(float value);
   
   ofVec2f mPosition;
   ofVec2f mDimensions;
   ::ADSR* mAdsr;
   ::ADSR mViewAdsr;
   ::ADSR mClickAdsr;
   bool mClick;
   ofVec2f mClickStart;
   float mViewLength;
   int mHighlightPoint;
   int mHighlightCurve;
   double mLastClickTime;
   bool mFixedLengthMode;
};

class EnvelopeEditor : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IIntSliderListener
{
public:
   EnvelopeEditor();
   static IDrawableModule* Create() { return new EnvelopeEditor(); }
   void Delete() { delete this; }
   void DrawModule() override;
   
   void SetEnabled(bool enabled) override {} //don't use this one
   bool Enabled() const override { return true; }
   bool HasTitleBar() const override { return mPinned; }
   string GetTitleLabel() override { return mPinned ? "envelope editor" : ""; }
   bool IsSaveable() override { return mPinned; }
   void CreateUIControls() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   bool IsPinned() const { return mPinned; }
   void SetADSRDisplay(ADSRDisplay* adsrDisplay);
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   void GetModuleDimensions(float& width, float& height) override { width = 400; height = 300; }
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
protected:
   ~EnvelopeEditor();
   
private:
   void OnClicked(int x, int y, bool right) override;
   void Pin();
   
   EnvelopeControl mEnvelopeControl;
   
   ADSRDisplay* mADSRDisplay;
   ClickButton* mPinButton;
   bool mPinned;
   float mADSRViewLength;
   FloatSlider* mADSRViewLengthSlider;
   Checkbox* mHasSustainStageCheckbox;
   IntSlider* mSustainStageSlider;
   FloatSlider* mMaxSustainSlider;
   Checkbox* mFreeReleaseLevelCheckbox;
   PatchCableSource* mTargetCable;
};
