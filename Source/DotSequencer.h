/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2023 Ryan Challinor (contact: awwbees@gmail.com)

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
//  DotSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/23.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "INoteSource.h"
#include "Slider.h"
#include "IDrivableSequencer.h"
#include "DotGrid.h"
#include "Transport.h"

class DotSequencer : public IDrawableModule, public IButtonListener, public IDropdownListener, public IIntSliderListener, public ITimeListener, public INoteSource, public IDrivableSequencer, public IAudioPoller
{
public:
   DotSequencer();
   virtual ~DotSequencer();
   static IDrawableModule* Create() { return new DotSequencer(); }

   void CreateUIControls() override;
   void Init() override;

   //IDrawableModule
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool IsEnabled() const override { return mEnabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   void OnStep(double time, float velocity, int flags);
   int RowToPitch(int row) const;

   enum class NoteMode
   {
      Scale,
      Chromatic
   };

   float mWidth{ 400 }, mHeight{ 200 };
   bool mHasExternalPulseSource{ false };
   int mStepIdx{ -1 };
   TransportListenerInfo* mTransportListenerInfo{ nullptr };
   bool mShouldStopAllNotes{ false };

   NoteInterval mInterval{ NoteInterval::kInterval_8n };
   DropdownList* mIntervalSelector{ nullptr };
   NoteMode mNoteMode{ NoteMode::Scale };
   DropdownList* mNoteModeSelector{ nullptr };
   ClickButton* mClearButton{ nullptr };
   int mOctave{ 4 };
   IntSlider* mOctaveSlider{ nullptr };
   int mCols{ 16 };
   IntSlider* mColsSlider{ nullptr };
   int mRows{ 13 };
   IntSlider* mRowsSlider{ nullptr };
   int mRowOffset{ 0 };
   IntSlider* mRowOffsetSlider{ nullptr };
   ClickButton* mDoubleButton{ nullptr };

   DotGrid* mDotGrid{ nullptr };

   struct PlayingDot
   {
      int mRow{ 0 };
      int mCol{ 0 };
      int mPitch{ -1 };
      double mPlayedTime{ 0 };
   };
   std::array<PlayingDot, 100> mPlayingDots;
};
