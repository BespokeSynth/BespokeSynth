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

#ifndef __modularSynth__BeatBloks__
#define __modularSynth__BeatBloks__

#include <iostream>
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
   
   std::string GetTitleLabel() override { return "BEAT BLOKS by @awwbees"; }
   void CreateUIControls() override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
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
   
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
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
      : mStartTime(startTime), mDuration(duration), mConfidence(confidence) {}
      float mStartTime;
      float mDuration;
      float mConfidence;
      BlokType mType;
   };
   
   enum ReadState
   {
      kReadState_Start,
      kReadState_Bars,
      kReadState_Beats,
      kReadState_Tatums,
      kReadState_Sections,
      kReadState_Segments
   } mReadState;
   
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
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   
   Sample* mSample;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   float* mWriteBuffer;
   bool mPlay;
   Checkbox* mPlayCheckbox;
   bool mLoop;
   Checkbox* mLoopCheckbox;
   int mMeasureEarly;
   float mClipStart;
   FloatSlider* mClipStartSlider;
   float mClipEnd;
   FloatSlider* mClipEndSlider;
   float mZoomStart;
   FloatSlider* mZoomStartSlider;
   float mZoomEnd;
   FloatSlider* mZoomEndSlider;
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mNumBars;
   IntSlider* mNumBarsSlider;
   ClickButton* mWriteButton;
   float mPlayheadRemainder;
   int mPlayheadWhole;
   bool mWantWrite;
   ClickButton* mDoubleLengthButton;
   ClickButton* mHalveLengthButton;
   std::vector<Blok> mBars;
   std::vector<Blok> mBeats;
   std::vector<Blok> mTatums;
   std::vector<Blok> mSections;
   std::vector<Blok> mSegments;
   std::vector<Blok> mNothing;
   BlokType mDrawBlokType;
   DropdownList* mDrawBlokTypeDropdown;
   bool mLoading;
   Blok* mHeldBlok;
   float mMouseX;
   float mMouseY;
   float mGrabOffsetX;
   float mGrabOffsetY;
   ClickButton* mGetLuckyButton;
   ClickButton* mLoseYourselfButton;
   
   float mRemixPlayhead;
   bool mPlayRemix;
   Checkbox* mPlayRemixCheckbox;
   std::list<Blok*> mRemixBloks;
   JumpBlender mRemixJumpBlender;
   Blok* mLastPlayedRemixBlok;
   float mLastLookupPlayhead;
   ClickButton* mClearRemixButton;
   float mRemixZoomStart;
   FloatSlider* mRemixZoomStartSlider;
   float mRemixZoomEnd;
   FloatSlider* mRemixZoomEndSlider;
   bool mBlockMultiPlaceEngaged;
   
   bool mPlayBlokPreview;
   float mBlokPreviewPlayhead;
   Ramp mBlokPreviewRamp;
   
   bool mDrawSources;
   Checkbox* mDrawSourcesCheckbox;
   
   int mLastRemovedRemixBlokIdx;
};

#endif /* defined(__modularSynth__BeatBloks__) */

