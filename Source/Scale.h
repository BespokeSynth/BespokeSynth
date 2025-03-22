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
//  Scale.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#pragma once

#include "IDrawableModule.h"
#include "DropdownList.h"
#include "LFO.h"
#include "Chord.h"
#include "ChordDatabase.h"
#include <atomic>

class IScaleListener
{
public:
   virtual ~IScaleListener() {}
   virtual void OnScaleChanged() = 0;
};

struct Accidental
{
   Accidental(int pitch, int direction)
   : mPitch(pitch)
   , mDirection(direction)
   {}
   int mPitch;
   int mDirection;
};

inline bool operator==(const Accidental& lhs, const Accidental& rhs)
{
   return lhs.mPitch == rhs.mPitch && lhs.mDirection == rhs.mDirection;
}

struct ScalePitches
{
   int mScaleRoot{ 0 };
   std::string mScaleType;
   std::vector<int> mScalePitches[2]; //double-buffered to avoid thread safety issues when modifying
   std::atomic<int> mScalePitchesFlip{ 0 };
   std::vector<Accidental> mAccidentals;

   void SetRoot(int root);
   void SetScaleType(std::string type);
   void SetAccidentals(const std::vector<Accidental>& accidentals);

   const std::vector<int>& GetPitches() const { return mScalePitches[mScalePitchesFlip]; }
   int ScaleRoot() const { return mScaleRoot; }
   std::string GetType() const { return mScaleType; }
   void GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals) const;
   int GetScalePitch(int index) const;

   bool IsRoot(int pitch) const;
   bool IsInPentatonic(int pitch) const;
   bool IsInScale(int pitch) const;
   int GetPitchFromTone(int n) const;
   int GetToneFromPitch(int pitch) const;
   int NumTonesInScale() const;
};

class MTSClient;

class Scale : public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   Scale();
   ~Scale();
   void Init() override;

   void CreateUIControls() override;

   bool IsSingleton() const override { return true; }

   int MakeDiatonic(int pitch);
   bool IsRoot(int pitch);
   bool IsInPentatonic(int pitch);
   bool IsInScale(int pitch);
   int GetPitchFromTone(int n);
   int GetToneFromPitch(int pitch);
   void SetScale(int root, std::string type);
   int ScaleRoot() { return mScale.mScaleRoot; }
   std::string GetType() { return mScale.mScaleType; }
   void SetRoot(int root, bool force = true);
   void SetScaleType(std::string type, bool force = true);
   void AddListener(IScaleListener* listener);
   void RemoveListener(IScaleListener* listener);
   void ClearListeners();
   void Poll() override;
   void SetScaleDegree(int degree);
   int GetScaleDegree() { return mScaleDegree; }
   void SetAccidentals(const std::vector<Accidental>& accidentals);
   void GetChordDegreeAndAccidentals(const Chord& chord, int& degree, std::vector<Accidental>& accidentals);
   ScalePitches& GetScalePitches() { return mScale; }
   std::vector<int> GetPitchesForScale(std::string type);
   void SetRandomSeptatonicScale();
   int GetNumScaleTypes() { return (int)mScales.size(); }
   std::string GetScaleName(int index) { return mScales[index].mName; }
   int NumTonesInScale() const { return mScale.NumTonesInScale(); }
   int GetPitchesPerOctave() const { return MAX(1, mPitchesPerOctave); }

   float PitchToFreq(float pitch);
   float FreqToPitch(float freq);

   const ChordDatabase& GetChordDatabase() const { return mChordDatabase; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void TextEntryComplete(TextEntry* entry) override;

   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return true; }

private:
   struct ScaleInfo
   {
      ScaleInfo() {}
      ScaleInfo(std::string name, std::vector<int> pitches)
      : mName(name)
      , mPitches(pitches)
      {}
      std::string mName;
      std::vector<int> mPitches;
   };

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   void NotifyListeners();

   void SetUpRootList();
   float RationalizeNumber(float input);
   void UpdateTuningTable();
   float GetTuningTableRatio(int semitonesFromCenter);
   void SetRandomRootAndScale();

   enum IntonationMode
   {
      kIntonation_Equal,
      kIntonation_Just,
      kIntonation_Pythagorean,
      kIntonation_Meantone,
      kIntonation_Rational,
      kIntonation_SclFile,
      kIntonation_Oddsound
   };

   ScalePitches mScale;
   std::list<IScaleListener*> mListeners;
   DropdownList* mRootSelector{ nullptr };
   DropdownList* mScaleSelector{ nullptr };
   IntSlider* mScaleDegreeSlider{ nullptr };
   int mScaleDegree{ 0 };

   ClickButton* mLoadSCLButton{ nullptr };
   ClickButton* mLoadKBMButton{ nullptr };
   ClickButton* mQueuedButtonPress{ nullptr };

   std::vector<ScaleInfo> mScales;
   int mNumSeptatonicScales{ 0 };
   int mScaleIndex{ 0 };

   int mPitchesPerOctave{ 12 };
   float mReferenceFreq{ 440 };
   float mReferencePitch{ 69 };
   TextEntry* mPitchesPerOctaveEntry{ nullptr };
   TextEntry* mReferenceFreqEntry{ nullptr };
   TextEntry* mReferencePitchEntry{ nullptr };
   IntonationMode mIntonation{ IntonationMode::kIntonation_Equal };
   DropdownList* mIntonationSelector{ nullptr };

   std::array<float, 256> mTuningTable{};

   ChordDatabase mChordDatabase;

   MTSClient* mOddsoundMTSClient{ nullptr };

   std::string mSclContents;
   std::string mKbmContents;
   std::string mCustomScaleDescription;
   bool mWantSetRandomRootAndScale{ false };
};

extern Scale* TheScale;
