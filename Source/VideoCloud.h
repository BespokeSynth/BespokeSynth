/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  VideoCloud.h
//  Bespoke
//
//  Drop an mp4 or gif and it becomes an animated, stylised point/pixel field. Frames are extracted
//  once (via ffmpeg) into small in-memory grids at drop time, so playback is as cheap as PixelCloud.
//  A temporal-average background model gives a rough auto subject mask (motion / luma / off), and
//  all the effects apply to the masked subject: pixelation, halftone, symmetry (kaleidoscope), grain,
//  chromatic aberration, glitch, distortion, posterize (oil) and edges (comic). Colour comes from the
//  video, the shared palettes, or mono. CPU immediate-mode, capped cell count, no feedback buffer.
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "IVisualNode.h"
#include "VizGL.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

class VideoCloud : public IAudioProcessor, public IDrawableModule, public IVisualNode, public ITimeListener, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener
{
public:
   VideoCloud();
   virtual ~VideoCloud();
   static IDrawableModule* Create() { return new VideoCloud(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = w;
      mHeight = h;
   }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override { }
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   //IVisualNode
   void CookIfNeeded(int frameId) override;
   unsigned int GetOutputTexture() override;
   int GetOutputWidth() const override { return mOutputFbo.w; }
   int GetOutputHeight() const override { return mOutputFbo.h; }

   void LoadWorker(std::string path); //runs on a background thread: ffmpeg + frame decode
   std::string FindFfmpeg() const;
   void BuildBackground(const std::vector<std::vector<unsigned char>>& frames, int w, int hh, std::vector<unsigned char>& outBg) const;
   void PaletteColor(float t, float& rOut, float& gOut, float& bOut) const;
   void UpdateTransportListener();

   //decoded frames, all at mSrcW x mSrcH, tightly packed RGB
   int mSrcW{ 0 };
   int mSrcH{ 0 };
   std::vector<std::vector<unsigned char>> mFrames; //each mSrcW*mSrcH*3
   std::vector<unsigned char> mBackground; //temporal average, mSrcW*mSrcH*3
   std::string mStatus{ "drop an mp4 or gif" };
   double mStartTime{ 0 };
   int mSyncFrame{ 0 };

   //background decode: the loader thread builds frames off the UI thread, then swaps them in
   std::thread mLoaderThread;
   std::atomic<bool> mLoading{ false };
   std::mutex mFramesMutex;

   //audio (for optional reactive glitch)
   float mAmplitude{ 0 };

   //controls
   int mDensity{ 90 };
   float mFps{ 15.0f };
   bool mSync{ false };
   bool mLoop{ true };
   NoteInterval mInterval{ kInterval_16n };
   int mMaskMode{ 1 }; //0 off, 1 motion, 2 luma
   float mThreshold{ 0.18f };
   int mSymmetry{ 1 };
   int mKaleido{ 1 }; //radial kaleidoscope segments (1 = off)
   float mReact{ 0.0f }; //how much the audio drives glitch/distort/brightness/spin
   float mExposure{ 1.0f };
   float mDistort{ 0.0f };
   float mGlitch{ 0.0f };
   float mChroma{ 0.0f };
   float mGrain{ 0.04f };
   int mPosterize{ 0 }; //0 = off, else colour levels (oil)
   float mEdges{ 0.0f }; //comic outline amount
   bool mHalftone{ false };
   int mColorMode{ 0 }; //0 video, 1 palette, 2 mono
   float mHueShift{ 0.0f };
   int mPaletteIndex{ 0 };

   IntSlider* mDensitySlider{ nullptr };
   FloatSlider* mFpsSlider{ nullptr };
   Checkbox* mSyncCheckbox{ nullptr };
   Checkbox* mLoopCheckbox{ nullptr };
   DropdownList* mIntervalSelector{ nullptr };
   DropdownList* mMaskSelector{ nullptr };
   FloatSlider* mThresholdSlider{ nullptr };
   IntSlider* mSymmetrySlider{ nullptr };
   IntSlider* mKaleidoSlider{ nullptr };
   FloatSlider* mReactSlider{ nullptr };
   FloatSlider* mExposureSlider{ nullptr };
   FloatSlider* mDistortSlider{ nullptr };
   FloatSlider* mGlitchSlider{ nullptr };
   FloatSlider* mChromaSlider{ nullptr };
   FloatSlider* mGrainSlider{ nullptr };
   IntSlider* mPosterizeSlider{ nullptr };
   FloatSlider* mEdgesSlider{ nullptr };
   Checkbox* mHalftoneCheckbox{ nullptr };
   DropdownList* mColorModeSelector{ nullptr };
   FloatSlider* mHueShiftSlider{ nullptr };
   DropdownList* mPaletteSelector{ nullptr };

   float mWidth{ 470 };
   float mHeight{ 388 };

   VizGL::Fbo mOutputFbo;
   int mLastCookFrame{ -1 };
};
