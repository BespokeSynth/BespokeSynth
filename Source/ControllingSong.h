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
//  ControllingSong.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/29/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "Slider.h"
#include "IAudioSource.h"
#include "MidiReader.h"
#include "Sample.h"
#include "ofxJSONElement.h"

class FollowingSong;

class ControllingSong : public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public IIntSliderListener, public IAudioSource, public IFloatSliderListener
{
public:
   ControllingSong();
   ~ControllingSong();
   static IDrawableModule* Create() { return new ControllingSong(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void Poll() override;

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   virtual void SaveLayout(ofxJSONElement& moduleInfo) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 560;
      height = 160;
   }

   void LoadSong(int index);

   ofMutex mLoadSongMutex;
   bool mLoadingSong{ true };

   int mCurrentSongIndex{ -1 };
   MidiReader mMidiReader;
   Sample mSample;
   float mVolume{ .8 };
   FloatSlider* mVolumeSlider{ nullptr };
   bool mNeedNewSong{ true };
   double mSongStartTime{ 0 };
   ofxJSONElement mSongList;
   int mTestBeatOffset{ 0 };
   IntSlider* mTestBeatOffsetSlider{ nullptr };
   bool mPlay{ false };
   Checkbox* mPlayCheckbox{ nullptr };
   bool mShuffle{ false };
   Checkbox* mShuffleCheckbox{ nullptr };
   ClickButton* mPhraseForwardButton{ nullptr };
   ClickButton* mPhraseBackButton{ nullptr };
   float mSpeed{ 1 };
   FloatSlider* mSpeedSlider{ nullptr };
   bool mMute{ false };
   Checkbox* mMuteCheckbox{ nullptr };

   DropdownList* mSongSelector{ nullptr };
   ClickButton* mNextSongButton{ nullptr };
   int mShuffleIndex{ 0 };
   std::vector<int> mShuffleList{};
   std::vector<FollowingSong*> mFollowSongs{};
};
