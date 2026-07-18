/**
    bespoke synth - TransformViz
**/

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IVisualNode.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "VizGL.h"
#include "PatchCableSource.h"

class TransformViz : public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IVisualNode
{
public:
   TransformViz();
   ~TransformViz() override;
   static IDrawableModule* Create() { return new TransformViz(); }
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
      mWidth = MAX(180, w);
      mHeight = MAX(180, h);
      if (mTargetCable)
         mTargetCable->SetManualPosition(mWidth - 12, 12);
   }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override { }

   //IVisualNode
   unsigned int GetOutputTexture() override { return VizGL::FboTexture(mOut); }
   int GetOutputWidth() const override { return mResW; }
   int GetOutputHeight() const override { return mResH; }
   void CookIfNeeded(int frameId) override;

private:
   bool EnsureShader();
   void Cook(); //render the shader into mOut (throttled = memoized)

   VizGL::Fbo mOut;
   unsigned int mProgram{ 0 };
   bool mShaderTried{ false };
   int mResW{ 256 };
   int mResH{ 256 };
   int mLastCookFrame{ -1 };

   float mTransX{ 0.0f };
   float mTransY{ 0.0f };
   float mScaleX{ 1.0f };
   float mScaleY{ 1.0f };
   float mRotate{ 0.0f };

   int mLocTrans{ -1 };
   int mLocScale{ -1 };
   int mLocRotate{ -1 };
   int mLocRes{ -1 };
   int mLocTexRes{ -1 };
   int mLocFill{ -1 };

   FloatSlider* mTransXSlider{ nullptr };
   FloatSlider* mTransYSlider{ nullptr };
   FloatSlider* mScaleXSlider{ nullptr };
   FloatSlider* mScaleYSlider{ nullptr };
   FloatSlider* mRotateSlider{ nullptr };

   DropdownList* mAspectSelector{ nullptr };
   int mAspect{ 0 };
   Checkbox* mFillCheckbox{ nullptr };
   bool mFill{ false };

   PatchCableSource* mTargetCable{ nullptr };

   float mWidth{ 280 };
   float mHeight{ 320 };
};
