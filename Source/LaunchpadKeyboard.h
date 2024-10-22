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
//  LaunchpadKeyboard.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#pragma once

#include "Scale.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Transport.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "GridController.h"
#include "Push2Control.h"

class LaunchpadNoteDisplayer;

class Chorder;

class LaunchpadKeyboard : public IDrawableModule, public INoteSource, public IScaleListener, public IIntSliderListener, public ITimeListener, public IDropdownListener, public IFloatSliderListener, public IGridControllerListener, public IPush2GridController
{
public:
   LaunchpadKeyboard();
   ~LaunchpadKeyboard();
   static IDrawableModule* Create() { return new LaunchpadKeyboard(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool HasDebugDraw() const override { return true; }

   void SetDisplayer(LaunchpadNoteDisplayer* displayer) { mDisplayer = displayer; }
   void DisplayNote(int pitch, int velocity);
   void SetChorder(Chorder* chorder) { mChorder = chorder; }

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IScaleListener
   void OnScaleChanged() override;

   //IDrawableModule
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   void Exit() override;
   void Poll() override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   enum LaunchpadLayout
   {
      kChromatic,
      kMajorThirds,
      kDiatonic,
      kChordIndividual,
      kChord,
      kGuitar,
      kSeptatonic,
      kDrum,
      kAllPads
   };

   enum ArrangementMode
   {
      kFull,
      kFive,
      kSix
   };

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 120;
      height = 74;
   }

   void PlayKeyboardNote(double time, int pitch, int velocity);
   void UpdateLights(bool force = false);
   GridColor GetGridSquareColor(int x, int y);
   int GridToPitch(int x, int y);
   void HandleChordButton(int pitch, bool bOn);
   bool IsChordButtonPressed(int pitch);
   void PressedNoteFor(int x, int y, int velocity);
   void ReleaseNoteFor(int x, int y);
   int GridToPitchChordSection(int x, int y);
   int GetHeldVelocity(int pitch)
   {
      if (pitch >= 0 && pitch < 128)
         return mCurrentNotes[pitch];
      else
         return 0;
   }

   int mRootNote{ 4 }; // 4 = E

   int mCurrentNotes[128]{};
   bool mTestKeyHeld{ false };
   int mOctave{ 3 };
   IntSlider* mOctaveSlider{ nullptr };
   bool mLatch{ false };
   Checkbox* mLatchCheckbox{ nullptr };
   LaunchpadLayout mLayout{ LaunchpadLayout::kChromatic };
   DropdownList* mLayoutDropdown{ nullptr };
   int mCurrentChord{ 0 };
   std::vector<std::vector<int> > mChords;
   LaunchpadNoteDisplayer* mDisplayer{ nullptr };
   ArrangementMode mArrangementMode{ ArrangementMode::kFull };
   DropdownList* mArrangementModeDropdown{ nullptr };
   std::list<int> mHeldChordTones;
   Chorder* mChorder{ nullptr };
   bool mLatchChords{ false };
   Checkbox* mLatchChordsCheckbox{ nullptr };
   bool mWasChorderEnabled{ false };
   bool mPreserveChordRoot{ true };
   Checkbox* mPreserveChordRootCheckbox{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
};
