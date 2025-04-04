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
//  BeatBloks.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/9/13.
//
//

#pragma once

#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "JumpBlender.h"

class Sample;

#define MEASURE_ZONE_HEIGHT 40
#define BEAT_ZONE_HEIGHT 60

class BeatBloks : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   BeatBloks();
   ~BeatBloks();
   static IDrawableModule* Create() { return new BeatBloks(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IDrawableModule
   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void Poll() override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;


   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   enum BlokType
   {
      kBlok_Bar,
      kBlok_Beat,
      kBlok_Tatum,
      kBlok_Segment,
      kBlok_Section
   };

   struct Blok
   {
      Blok(float startTime, float duration, float confidence)
      : mStartTime(startTime)
      , mDuration(duration)
      , mConfidence(confidence)
      {}
      float mStartTime;
      float mDuration;
      float mConfidence;
      BlokType mType{ BlokType::kBlok_Bar };
   };

   enum ReadState
   {
      kReadState_Start,
      kReadState_Bars,
      kReadState_Beats,
      kReadState_Tatums,
      kReadState_Sections,
      kReadState_Segments
   } mReadState{ kReadState_Start };

   void UpdateSample();
   void DoWrite();
   void UpdateZoomExtents();
   void ResetRead();
   void ReadEchonestLine(const char* line);
   float StartTime(const Blok& blok);
   float GetInsertPosition(int& insertIndex);
   void PlaceHeldBlok();
   Blok* RemoveBlokAt(int x);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   Sample* mSample{ nullptr };

   float mVolume{ .6 };
   FloatSlider* mVolumeSlider{ nullptr };
   float* mWriteBuffer;
   bool mPlay{ false };
   Checkbox* mPlayCheckbox{ nullptr };
   bool mLoop{ true };
   Checkbox* mLoopCheckbox{ nullptr };
   int mMeasureEarly{ 0 };
   float mClipStart{ 0 };
   FloatSlider* mClipStartSlider{ nullptr };
   float mClipEnd{ 1 };
   FloatSlider* mClipEndSlider{ nullptr };
   float mZoomStart{ 0 };
   FloatSlider* mZoomStartSlider{ nullptr };
   float mZoomEnd{ 1 };
   FloatSlider* mZoomEndSlider{ nullptr };
   float mOffset{ 0 };
   FloatSlider* mOffsetSlider{ nullptr };
   int mNumBars{ 1 };
   IntSlider* mNumBarsSlider{ nullptr };
   ClickButton* mWriteButton{ nullptr };
   float mPlayheadRemainder{ 0 };
   int mPlayheadWhole{ 0 };
   bool mWantWrite{ false };
   ClickButton* mDoubleLengthButton{ nullptr };
   ClickButton* mHalveLengthButton{ nullptr };
   std::vector<Blok> mBars;
   std::vector<Blok> mBeats;
   std::vector<Blok> mTatums;
   std::vector<Blok> mSections;
   std::vector<Blok> mSegments;
   std::vector<Blok> mNothing;
   BlokType mDrawBlokType{ BlokType::kBlok_Bar };
   DropdownList* mDrawBlokTypeDropdown{ nullptr };
   bool mLoading{ false };
   Blok* mHeldBlok{ nullptr };
   float mMouseX{ 0 };
   float mMouseY{ 0 };
   float mGrabOffsetX{ 0 };
   float mGrabOffsetY{ 0 };
   ClickButton* mGetLuckyButton{ nullptr };
   ClickButton* mLoseYourselfButton{ nullptr };

   float mRemixPlayhead{ 0 };
   bool mPlayRemix{ false };
   Checkbox* mPlayRemixCheckbox{ nullptr };
   std::list<Blok*> mRemixBloks;
   JumpBlender mRemixJumpBlender;
   Blok* mLastPlayedRemixBlok{ nullptr };
   float mLastLookupPlayhead{ 0 };
   ClickButton* mClearRemixButton{ nullptr };
   float mRemixZoomStart{ 0 };
   FloatSlider* mRemixZoomStartSlider{ nullptr };
   float mRemixZoomEnd;
   FloatSlider* mRemixZoomEndSlider{ nullptr };
   bool mBlockMultiPlaceEngaged{ false };

   bool mPlayBlokPreview{ false };
   float mBlokPreviewPlayhead{ 0 };
   Ramp mBlokPreviewRamp;

   bool mDrawSources{ false };
   Checkbox* mDrawSourcesCheckbox{ nullptr };

   int mLastRemovedRemixBlokIdx{ -1 };
};
