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

#include <iostream>
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "INoteSource.h"
#include "Slider.h"
#include "UIGrid.h"
#include "Scale.h"
#include "GridController.h"

class PatchCableSource;

class NoteTable : public IDrawableModule, public INoteSource, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public UIGridListener, public IScaleListener, public INoteReceiver, public IGridControllerListener
{
public:
   NoteTable();
   virtual ~NoteTable();
   static IDrawableModule* Create() { return new NoteTable(); }
   
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
   
   //IScaleListener
   void OnScaleChanged() override;
   
   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   void UpdateGridControllerLights(bool force);
   
   void PlayColumn(double time, int column, int velocity, int voiceIdx, ModulationParameters modulation);
   void SetUpColumnControls();
   void SyncGridToSeq();
   float ExtraWidth() const;
   float ExtraHeight() const;
   void RandomizePitches(bool fifths);
   
   enum NoteMode
   {
      kNoteMode_Scale,
      kNoteMode_Chromatic,
      kNoteMode_Pentatonic,
      kNoteMode_Fifths
   };

   UIGrid* mGrid;
   int mOctave;
   IntSlider* mOctaveSlider;
   NoteMode mNoteMode;
   DropdownList* mNoteModeSelector;
   int mLength;
   IntSlider* mLengthSlider;
   bool mSetLength;
   int mNoteRange;
   bool mShowColumnControls;
   int mRowOffset;
   
   ClickButton* mRandomizePitchButton;
   float mRandomizePitchChance;
   float mRandomizePitchRange;
   FloatSlider* mRandomizePitchChanceSlider;
   FloatSlider* mRandomizePitchRangeSlider;

   static constexpr int kMaxLength = 32;
   
   int mTones[kMaxLength];
   std::array<double, kMaxLength> mLastColumnPlayTime{ -1 };
   std::array<int, kMaxLength> mLastColumnNoteOnPitch{ -1 };
   std::array<DropdownList*, kMaxLength> mToneDropdowns{ nullptr };
   std::array<AdditionalNoteCable*, kMaxLength> mColumnCables{ nullptr };

   GridControlTarget* mGridControlTarget;
   int mGridControlOffsetX;
   int mGridControlOffsetY;
   IntSlider* mGridControlOffsetXSlider;
   IntSlider* mGridControlOffsetYSlider;
};
