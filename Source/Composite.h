/**
    bespoke synth - Composite

    Combines the output of multiple visualizer modules into one, TouchDesigner-Composite-TOP style.
    Cable any number of visualizers into it; each becomes a layer. The bottom layer is the base, and
    every layer above it is blended onto the stack with a single global operand. Select a layer row
    and use up/down to reorder. Only visualizer-category modules are composited.

    Rendering: each layer is rendered once into its own offscreen texture, then a single fragment
    shader composites all layers in one pass (normal / add / screen / multiply / lighten / darken /
    difference / overlay / soft light / hard light). FBOs are reused and only reallocated on resize,
    and the shader is compiled once - so the per-frame cost is N layer renders + one composite pass.
**/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "Slider.h"
#include "Checkbox.h"
#include <vector>

#include "IVisualNode.h"

class PatchCableSource;
struct NVGLUframebuffer;

class Composite : public IDrawableModule, public IButtonListener, public IDropdownListener, public IFloatSliderListener, public IVisualNode
{
public:
   Composite();
   virtual ~Composite();
   static IDrawableModule* Create() { return new Composite(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = MAX(240, w);
      mHeight = MAX(160, h);
   }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //IDrawableModule
   void OnClicked(float x, float y, bool right) override;
   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IVisualNode
   unsigned int GetOutputTexture() override;
   int GetOutputWidth() const override { return mFBW; }
   int GetOutputHeight() const override { return mFBH; }
   void CookIfNeeded(int frameId) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void CheckboxUpdated(Checkbox* checkbox, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   static const int kMaxLayers = 6;

private:
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void SyncLayers();
   void EnsureResources(int w, int h);
   bool EnsureShader();
   void CompositeLayers(int count, int w, int h);
   void DrawPreview(float px, float py, float pw, float ph, int count);

   struct Layer
   {
      IDrawableModule* module = nullptr;
      float opacity = 1.0f;
      int blendMode = 0; //0=normal, 1=add, 2=screen, 3=multiply, etc
      FloatSlider* opacitySlider = nullptr;
      DropdownList* blendDropdown = nullptr;
   };

   Layer mLayerData[kMaxLayers];
   int mSelectedLayer{ 0 };
   int mAspect{ 0 }; //0 source, 1 16:9, 2 1:1, 3 9:16
   bool mFill{ false }; //false = contain (letterbox), true = cover (fill frame, crop)
   int mMaxFps{ 20 }; //cap the (expensive) composite recompute rate
   int mLastCookFrame{ -1 };
   double mLastCompositeMs{ -10000 };
   float mLayerAspect[kMaxLayers]{}; //w/h of each layer's source, for aspect-correct fitting

   //global post-process applied to the composited result (in the shader)
   float mExposure{ 1.0f };
   float mContrast{ 1.0f };
   float mBlack{ 0.0f };
   float mSaturation{ 1.0f };

   PatchCableSource* mCableSource{ nullptr };
   ClickButton* mMoveUpButton{ nullptr };
   ClickButton* mMoveDownButton{ nullptr };
   DropdownList* mFpsSelector{ nullptr };
   DropdownList* mAspectSelector{ nullptr };
   Checkbox* mFillCheckbox{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mContrastSlider{ nullptr };
   FloatSlider* mBlackSlider{ nullptr };
   FloatSlider* mSaturationSlider{ nullptr };

   //gpu resources (created lazily on the GL thread)
   NVGLUframebuffer* mLayerFB[kMaxLayers]{};
   unsigned int mLayerTex[kMaxLayers]{}; //resolved per frame: node output tex, or the legacy FBO tex
   NVGLUframebuffer* mResultFB{ nullptr };
   int mFBW{ 0 };
   int mFBH{ 0 };
   int mResultImage{ -1 }; //nanovg image wrapping mResultFB's texture (main context)

   unsigned int mProgram{ 0 };
   unsigned int mVBO{ 0 };
   unsigned int mVAO{ 0 };
   int mLocLayers{ -1 };
   int mLocCount{ -1 };
   int mLocPos{ -1 };
   int mLocUv{ -1 };
   int mLocExposure{ -1 };
   int mLocContrast{ -1 };
   int mLocBlack{ -1 };
   int mLocSaturation{ -1 };
   int mLocFrameAspect{ -1 };
   int mLocLayerAspect{ -1 };
   int mLocFill{ -1 };
   int mLocOpacity{ -1 };
   int mLocLayerMode{ -1 };
   bool mShaderTried{ false };
   bool mShaderOk{ false };

   float mWidth{ 380 };
   float mHeight{ 320 };
};
