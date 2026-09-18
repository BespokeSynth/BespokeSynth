#pragma once

#include "IDrawableModule.h"
#include "IVisualNode.h"
#include "VizGL.h"
#include "PatchCableSource.h"
#include "Slider.h"
#include "Checkbox.h"
#include "DropdownList.h"

class PatchCableSource;
class IntSlider;

class VisualEffectsViz : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IVisualNode
{
public:
   VisualEffectsViz();
   virtual ~VisualEffectsViz();

   static IDrawableModule* Create() { return new VisualEffectsViz(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = MAX(280, w);
      mHeight = MAX(450, h);
      if (mTargetCable)
         mTargetCable->SetManualPosition(mWidth - 12, 12);
   }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   //IVisualNode
   unsigned int GetOutputTexture() override { return VizGL::FboTexture(mOut); }
   int GetOutputWidth() const override { return mResW; }
   int GetOutputHeight() const override { return mResH; }
   void CookIfNeeded(int frameNum) override
   {
      if (mLastCookFrame != frameNum)
      {
         mLastCookFrame = frameNum;
         Cook();
      }
   }
   void Cook();

   void SaveState(FileStreamOut& out) override {}
   void LoadState(FileStreamIn& in, int rev) override {}

   bool EnsureShader();

private:
   int mLastCookFrame{ -1 };
   VizGL::Fbo mOut;
   int mResW{ 0 };
   int mResH{ 0 };

   PatchCableSource* mTargetCable{ nullptr };

   unsigned int mProgram{ 0 };
   int mLocTime{ -1 };
   int mLocRes{ -1 };
   int mLocTexRes{ -1 };

   // Uniform locations
   int mLocNoiseAmount{ -1 };
   int mLocNoiseColor{ -1 };
   int mLocLensDistortX{ -1 };
   int mLocLensDistortY{ -1 };
   int mLocSymmetry{ -1 };
   int mLocPixelation{ -1 };
   int mLocBlur{ -1 };
   int mLocOil{ -1 };
   int mLocHalftone{ -1 };
   int mLocGlow{ -1 };

   // Parameters
   float mNoiseAmount{ 0.0f };
   bool mNoiseColor{ false };
   float mLensDistortX{ 0.0f };
   float mLensDistortY{ 0.0f };
   int mSymmetry{ 1 };
   float mPixelation{ 1.0f };
   float mBlur{ 0.0f };
   float mOil{ 0.0f };
   float mHalftone{ 0.0f };
   float mGlow{ 0.0f };

   // UI
   FloatSlider* mNoiseAmountSlider{ nullptr };
   Checkbox* mNoiseColorCheckbox{ nullptr };
   FloatSlider* mLensDistortXSlider{ nullptr };
   FloatSlider* mLensDistortYSlider{ nullptr };
   IntSlider* mSymmetrySlider{ nullptr };
   FloatSlider* mPixelationSlider{ nullptr };
   FloatSlider* mBlurSlider{ nullptr };
   FloatSlider* mOilSlider{ nullptr };
   FloatSlider* mHalftoneSlider{ nullptr };
   FloatSlider* mGlowSlider{ nullptr };

   float mWidth{ 280 };
   float mHeight{ 450 };
};
