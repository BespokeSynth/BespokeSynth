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
//  ChaosEngine.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/18/13.
//
//

#pragma once

#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "ClickButton.h"
#include "ofxJSONElement.h"
#include "Scale.h"
#include "RadioButton.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Chord.h"

class ChaosEngine;

extern ChaosEngine* TheChaosEngine;

class ChaosEngine : public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener, public INoteSource, public IIntSliderListener
{
public:
   ChaosEngine();
   ~ChaosEngine();
   static IDrawableModule* Create() { return new ChaosEngine(); }
   static bool CanCreate() { return TheChaosEngine == nullptr; }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void Poll() override;
   void AudioUpdate();
   void RestartProgression()
   {
      mRestarting = true;
      mChordProgressionIdx = -1;
   }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   struct ProgressionChord
   {
      ProgressionChord(int degree, int beatLength = -1)
      : mDegree(degree)
      , mBeatLength(beatLength)
      {}

      ProgressionChord(int degree, std::vector<Accidental> accidentals, int beatLength = -1)
      : mDegree(degree)
      , mAccidentals(accidentals)
      , mBeatLength(beatLength)
      {}

      ProgressionChord(const ofxJSONElement& chordInfo, ScalePitches scale);

      bool SameChord(const ProgressionChord& chord)
      {
         return mDegree == chord.mDegree &&
                mAccidentals == chord.mAccidentals;
      }
      int mDegree{ 0 };
      int mBeatLength{ -1 };
      std::vector<Accidental> mAccidentals;
      int mInversion{ 0 };
   };

   struct SongSection
   {
      std::string mName;
      std::vector<ProgressionChord> mChords;
   };

   struct Song
   {
      std::string mName;
      float mTempo{ 120 };
      int mTimeSigTop{ 4 };
      int mTimeSigBottom{ 4 };
      int mScaleRoot{ 0 };
      std::string mScaleType;
      std::vector<SongSection> mSections;
   };

   void SetPitchColor(int pitch);
   void DrawKeyboard(float x, float y);
   void DrawGuitar(float x, float y);
   bool IsChordRoot(int pitch);
   void ReadSongs();
   void UpdateProgression(int beat);
   void GenerateRandomProgression();
   std::vector<int> GetCurrentChordPitches();
   ofRectangle GetKeyboardKeyRect(int pitch, bool& isBlackKey);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 610;
      height = 700;
   }
   void OnClicked(float x, float y, bool right) override;

   ClickButton* mChaosButton{ nullptr };
   bool mTotalChaos{ false };
   Checkbox* mTotalChaosCheckbox{ nullptr };
   double mChaosArrivalTime{ -1 };
   int mLastAudioUpdateMeasure{ -1 };
   int mLastAudioUpdateBeat{ -1 };
   int mBeatsLeftToChordChange{ -1 };
   bool mRestarting{ false };

   int mChordProgressionIdx{ -1 };
   std::vector<ProgressionChord> mChordProgression;
   ofMutex mProgressionMutex;
   IntSlider* mChordProgressionSlider{ nullptr };

   int mSectionIdx{ 0 };
   RadioButton* mSectionDropdown{ nullptr };

   int mSongIdx{ -1 };
   DropdownList* mSongDropdown{ nullptr };
   std::vector<Song> mSongs;
   ClickButton* mReadSongsButton{ nullptr };

   bool mPlayChord{ false };
   Checkbox* mPlayChordCheckbox{ nullptr };
   bool mProgress{ true };
   Checkbox* mProgressCheckbox{ nullptr };

   int mDegree{ 0 };
   IntSlider* mDegreeSlider{ nullptr };
   Chord mInputChord;
   DropdownList* mRootNoteList{ nullptr };
   DropdownList* mChordTypeList{ nullptr };
   IntSlider* mInversionSlider{ nullptr };

   std::vector<Chord> mInputChords;
   ClickButton* mAddChordButton{ nullptr };
   ClickButton* mRemoveChordButton{ nullptr };
   ClickButton* mSetProgressionButton{ nullptr };

   ClickButton* mRandomProgressionButton{ nullptr };

   bool mHideBeat{ false };
   Checkbox* mHideBeatCheckbox{ nullptr };
};
