/**
    bespoke synth - LiquidifierViz
**/

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "IVisualNode.h"
#include "Slider.h"
#include "DropdownList.h"
#include "VizGL.h"
#include "PatchCableSource.h"

class LiquidifierViz : public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IVisualNode
{
public:
   LiquidifierViz();
   virtual ~LiquidifierViz();
   static IDrawableModule* Create() { return new LiquidifierViz(); }
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
      mHeight = MAX(320, h);
      if (mTargetCable)
         mTargetCable->SetManualPosition(mWidth - 12, 12);
   }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

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

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;

private:
   bool EnsureShader();

   int mLastCookFrame{ -1 };
   VizGL::Fbo mOut;
   int mResW{ 0 };
   int mResH{ 0 };
   unsigned int mProgram{ 0 };

   float mTwisting{ 1.0f };
   float mDistortion{ 0.1f };
   float mSpeed{ 1.0f };
   int mPalette{ 0 };

   int mLocTwisting{ -1 };
   int mLocDistortion{ -1 };
   int mLocSpeed{ -1 };
   int mLocPalette{ -1 };
   int mLocTime{ -1 };
   int mLocRes{ -1 };
   int mLocTexRes{ -1 };

   FloatSlider* mTwistSlider{ nullptr };
   FloatSlider* mDistSlider{ nullptr };
   FloatSlider* mSpeedSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   PatchCableSource* mTargetCable{ nullptr };

   float mWidth{ 280 };
   float mHeight{ 320 };
};
