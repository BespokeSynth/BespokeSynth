/*
  ==============================================================================

    FubbleModule.h
    Created: 8 Aug 2020 10:03:56am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Curve.h"
#include "Checkbox.h"
#include "IModulator.h"
#include "Slider.h"
#include "PerlinNoise.h"

class PatchCableSource;

class FubbleModule : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   FubbleModule();
   ~FubbleModule();
   static IDrawableModule* Create() { return new FubbleModule(); }
   
   string GetTitleLabel() override { return "fubble"; }
   void CreateUIControls() override;
   
   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   float GetPlaybackTime(double time);
   ofRectangle GetFubbleRect();
   ofVec2f GetFubbleMouseCoord();
   void RecordPoint();
   bool IsHovered();
   void Clear();
   float GetPerlinNoiseValue(double time, float x, float y, bool horizontal);
   void UpdatePerlinSeed() { mPerlinSeed = gRandom() % 1000; }
   
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   struct FubbleAxis : public IModulator
   {
      FubbleAxis(FubbleModule* owner, bool horizontal)
      : mOwner(owner)
      , mIsHorizontal(horizontal)
      , mHasRecorded(false)
      {
      }
      void UpdateControl() { OnModulatorRepatch(); }
      void SetCableSource(PatchCableSource* cableSource) { mTargetCable = cableSource; }
      PatchCableSource* GetCableSource() const { return mTargetCable; }
      
      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->Enabled() && (mHasRecorded || mOwner->mIsRightClicking); }
      
      FubbleModule* mOwner;
      bool mIsHorizontal;
      Curve mCurve;
      bool mHasRecorded;
   };
   
   FubbleAxis mAxisH;
   FubbleAxis mAxisV;
   float mLength;
   bool mQuantizeLength;
   Checkbox* mQuantizeLengthCheckbox;
   NoteInterval mQuantizeInterval;
   DropdownList* mQuantizeLengthSelector;
   float mSpeed;
   FloatSlider* mSpeedSlider;
   ClickButton* mClearButton;
   float mWidth;
   float mHeight;
   double mRecordStartOffset;
   bool mIsDrawing;
   bool mIsRightClicking;
   float mMouseX;
   float mMouseY;
   PerlinNoise mNoise;
   float mPerlinStrength;
   FloatSlider* mPerlinStrengthSlider;
   float mPerlinScale;
   FloatSlider* mPerlinScaleSlider;
   float mPerlinSpeed;
   FloatSlider* mPerlinSpeedSlider;
   int mPerlinSeed;
   ClickButton* mUpdatePerlinSeedButton;
};
