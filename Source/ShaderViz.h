/**
    bespoke synth - ShaderViz

    The first GPU visual node: an audio-reactive fragment-shader generator built on VizGL. All the
    per-pixel work happens on the GPU, so it holds framerate far better than the CPU visualizers and
    leaves the CPU free for audio. "react" is a normal (modulatable) slider - drive it from any Bespoke
    modulator, e.g. an audio-amplitude follower, to make it move with the music.

    Implements IVisualNode so it can feed the visual node graph (Composite etc.) once that's wired up.
**/

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IVisualNode.h"
#include "VizGL.h"

class ShaderViz : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IDropdownListener, public IVisualNode
{
public:
   ShaderViz();
   virtual ~ShaderViz();
   static IDrawableModule* Create() { return new ShaderViz(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = MAX(200, w);
      mHeight = MAX(140, h);
   }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   //IVisualNode
   unsigned int GetOutputTexture() override { return VizGL::FboTexture(mOut); }
   int GetOutputWidth() const override { return mOut.w; }
   int GetOutputHeight() const override { return mOut.h; }
   void CookIfNeeded(int frameId) override { Cook(); }

private:
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   bool EnsureShader();
   void Cook(); //render the shader into mOut (throttled = memoized)

   VizGL::Fbo mOut;
   unsigned int mProgram{ 0 };
   bool mShaderTried{ false };
   int mResW{ 480 };
   int mResH{ 360 };

   float mReact{ 0.4f };
   float mHue{ 0.0f };
   float mSpeed{ 1.0f };
   int mMaxFps{ 30 };
   double mLastCookMs{ -10000 };
   float mAmplitude{ 0.0f }; //audio RMS (written on the audio thread, read for the shader uniform)

   int mLocTime{ -1 };
   int mLocReact{ -1 };
   int mLocHue{ -1 };
   int mLocSpeed{ -1 };
   int mLocRes{ -1 };

   FloatSlider* mReactSlider{ nullptr };
   FloatSlider* mHueSlider{ nullptr };
   FloatSlider* mSpeedSlider{ nullptr };
   DropdownList* mFpsSelector{ nullptr };

   float mWidth{ 360 };
   float mHeight{ 320 };
};
