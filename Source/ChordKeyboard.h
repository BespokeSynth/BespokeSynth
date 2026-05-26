/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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
//  ChordKeyboard.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/18/26.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "Scale.h"
#include "Slider.h"
#include "KeyboardDisplay.h"
#include "AbletonDeviceShared.h"
#include "IAudioPoller.h"
#include "LockFreeQueue.h"

class ChordKeyboard : public NoteEffectBase, public IDrawableModule, public IDropdownListener, public IIntSliderListener, public IAbletonGridController, public IAudioPoller, public ITimeListener
{
public:
   ChordKeyboard();
   virtual ~ChordKeyboard();
   static IDrawableModule* Create() { return new ChordKeyboard(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void Init() override;
   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IAbletonGridController
   bool OnAbletonGridControl_InputThread(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue) override;
   bool OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue) override;
   void UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid) override;
   bool UpdateAbletonMoveScreen(IAbletonGridDevice* abletonGrid, AbletonMoveLCD* lcd, LCDDrawPass drawPass) override;
   bool IsInAbletonGridFocusMode() const override { return true; }

   bool HasPush2OverrideControls() const override { return true; }
   void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* dropdown, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;

   enum class ChordNoteType
   {
      Any,
      Bass,
      Middle,
      Top
   };

   struct OutputChord
   {
      OutputChord() { Reset(); }
      void Reset() { mPitches.fill(-1); }
      int GetNumPitches() const
      {
         if (mPitches[0] == -1)
            return 0;
         for (int i = 0; i < (int)mPitches.size() - 1; ++i)
         {
            if (mPitches[i + 1] < -1)
               return i + 1;
         }
         return (int)mPitches.size();
      }
      std::array<int, 10> mPitches;
   };

   enum class PlayOptions
   {
      ChordAndBass,
      ChordOnly,
      BassOnly,
      ChordOnPress,
      ImmediateChord,
      ImmediateChordOnly
   };

   enum class ChordStyle
   {
      Closed,
      SpreadThird,
      SpreadSeventh,
      SpreadThirdAndSeventh,
      DoubledRoot,
      DoubledRootAndThird,
      Doubled
   };

   struct ChordSettings
   {
      bool mDim{ false };
      bool mMinor{ false };
      bool mMajor{ false };
      bool mSus{ false };
      bool mSix{ false };
      bool mMin7{ false };
      bool mMaj7{ false };
      bool mNine{ false };
      PlayOptions mPlayOptions{ PlayOptions::ChordAndBass };

      bool IsChordButtonPressed() const
      {
         return mDim || mMinor || mMajor || mSus;
      }

      bool ShouldPlayBass() const
      {
         return mPlayOptions == PlayOptions::ChordAndBass ||
                mPlayOptions == PlayOptions::BassOnly ||
                mPlayOptions == PlayOptions::ChordOnPress ||
                mPlayOptions == PlayOptions::ImmediateChord;
      }

      bool ShouldPlayChord() const
      {
         if (mPlayOptions == PlayOptions::ChordOnPress)
            return IsChordButtonPressed();
         return mPlayOptions == PlayOptions::ChordAndBass ||
                mPlayOptions == PlayOptions::ChordOnly ||
                mPlayOptions == PlayOptions::ImmediateChord ||
                mPlayOptions == PlayOptions::ImmediateChordOnly;
      }
   };

   ChordSettings mChordSettings;
   ChordSettings mLastPlayedChordSettings;

   std::list<int> GetChordPitches(int forPitch);
   int GridToPitch(int x, int y) const;
   void UpdateOutputNotes(double time);
   static int AdjustForVoicing(int pitch, int voicing);
   const ChordSettings& GetChordSettings() const;
   std::string GetChordDisplayString() const;

   bool mScaleMode{ false };
   Checkbox* mScaleModeCheckbox{ nullptr };
   NoteInterval mQuantizeInterval{ kInterval_None };
   DropdownList* mQuantizeIntervalSelector{ nullptr };
   int mVelocityOverride{ -1 };
   IntSlider* mVelocityOverrideSlider{ nullptr };
   Checkbox* mDimCheckbox{ nullptr };
   Checkbox* mMinorCheckbox{ nullptr };
   Checkbox* mMajorCheckbox{ nullptr };
   Checkbox* mSusCheckbox{ nullptr };
   Checkbox* mSixCheckbox{ nullptr };
   Checkbox* mMin7Checkbox{ nullptr };
   Checkbox* mMaj7Checkbox{ nullptr };
   Checkbox* mNineCheckbox{ nullptr };
   DropdownList* mPlayOptionsSelector{ nullptr };
   bool mLatchKey{ false };
   Checkbox* mLatchKeyCheckbox{ nullptr };
   int mVoicing{ 41 };
   IntSlider* mVoicingSlider{ nullptr };
   int mBassVoicing{ 41 };
   IntSlider* mBassVoicingSlider{ nullptr };
   DropdownList* mChordStyleSelector{ nullptr };
   ChordStyle mChordStyle{ ChordStyle::Closed };

   int mInputPitchWrapped{ -1 };
   int mLastInputVelocity{ 0 };
   std::array<bool, 128> mOutputNotes;
   int mBassOutputPitch{ -1 };
   bool mLatchChord{ false };
   double mLastKeyPressTime{ -1 };

   AdditionalNoteCable* mBassCable{ nullptr };

   KeyboardDisplay::KeyboardDrawOptions mKeyboardDrawOptions;
   std::array<double, 128> mLastNoteOnTime{};
   std::array<double, 128> mLastNoteOffTime{};

   LockFreeQueue<NoteMessage> mNoteMessageQueue{};
};
