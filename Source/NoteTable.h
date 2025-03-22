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
//  NoteTable.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/3/13.
//
//

#pragma once

#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteSource.h"
#include "Slider.h"
#include "UIGrid.h"
#include "Scale.h"
#include "GridController.h"
#include "Push2Control.h"

class PatchCableSource;

class NoteTable : public IDrawableModule, public INoteSource, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public UIGridListener, public INoteReceiver, public IGridControllerListener, public IPush2GridController
{
public:
   NoteTable();
   virtual ~NoteTable();
   static IDrawableModule* Create() { return new NoteTable(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   void Poll() override;

   UIGrid* GetGrid() const { return mGrid; }

   int RowToPitch(int row);
   int PitchToRow(int pitch);

   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   void UpdateGridControllerLights(bool force);

   void PlayColumn(NoteMessage note);
   float ExtraWidth() const;
   float ExtraHeight() const;
   void RandomizePitches(bool fifths);
   void GetPush2Layout(int& sequenceRows, int& pitchCols, int& pitchRows);
   void SetColumnRow(int column, int row);

   enum NoteMode
   {
      kNoteMode_Scale,
      kNoteMode_Chromatic,
      kNoteMode_Pentatonic,
      kNoteMode_Fifths
   };

   UIGrid* mGrid{ nullptr };
   int mOctave{ 3 };
   IntSlider* mOctaveSlider{ nullptr };
   NoteMode mNoteMode{ NoteMode::kNoteMode_Scale };
   DropdownList* mNoteModeSelector{ nullptr };
   int mLength{ 8 };
   IntSlider* mLengthSlider{ nullptr };
   int mNoteRange{ 15 };
   bool mShowColumnCables{ false };
   int mRowOffset{ 0 };

   ClickButton* mRandomizePitchButton{ nullptr };
   float mRandomizePitchChance{ 1 };
   float mRandomizePitchRange{ 1 };
   FloatSlider* mRandomizePitchChanceSlider{ nullptr };
   FloatSlider* mRandomizePitchRangeSlider{ nullptr };
   ClickButton* mClearButton{ nullptr };

   static constexpr int kMaxLength = 128;

   std::array<double, kMaxLength> mLastColumnPlayTime{};
   std::array<bool[128], kMaxLength> mLastColumnNoteOnPitches{};
   std::array<AdditionalNoteCable*, kMaxLength> mColumnCables{ nullptr };
   std::array<double, 128> mPitchPlayTimes{};
   std::array<bool, 128> mQueuedPitches{};

   GridControlTarget* mGridControlTarget{ nullptr };
   int mGridControlOffsetX{ 0 };
   int mGridControlOffsetY{ 0 };
   IntSlider* mGridControlOffsetXSlider{ nullptr };
   IntSlider* mGridControlOffsetYSlider{ nullptr };

   int mPush2HeldStep{ -1 };
   enum class Push2GridDisplayMode
   {
      PerStep,
      GridView
   };
   Push2GridDisplayMode mPush2GridDisplayMode{ Push2GridDisplayMode::PerStep };
};
