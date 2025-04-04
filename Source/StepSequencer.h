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
//  StepSequencer.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#pragma once

#include "Transport.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "Slider.h"
#include "GridController.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "RadioButton.h"
#include "SynthGlobals.h"
#include "Push2Control.h"
#include "IPulseReceiver.h"
#include "TextEntry.h"
#include "IDrivableSequencer.h"

#define NUM_STEPSEQ_ROWS 16
#define META_STEP_MAX 64

class StepSequencer;

class StepSequencerRow : public ITimeListener
{
public:
   StepSequencerRow(StepSequencer* seq, UIGrid* grid, int row);
   ~StepSequencerRow();
   void CreateUIControls();
   void OnTimeEvent(double time) override;
   void PlayStep(double time, int step);
   void SetOffset(float offset);
   void UpdateTimeListener();
   void Draw(float x, float y);
   int GetRowPitch() const { return mRowPitch; }

private:
   UIGrid* mGrid{ nullptr };
   int mRow{ 0 };
   StepSequencer* mSeq{ nullptr };
   float mOffset{ 0 };

   struct PlayedStep
   {
      int step{ 0 };
      double time{ -1 };
   };
   std::array<PlayedStep, 5> mPlayedSteps{};
   int mPlayedStepsRoundRobin{ 0 };
   TextEntry* mRowPitchEntry{ nullptr };
   int mRowPitch{ 0 };
   Checkbox* mPlayRowCheckbox{ nullptr };
   bool mPlayRow{ true };
};

class NoteRepeat : public ITimeListener
{
public:
   NoteRepeat(StepSequencer* seq, int note);
   ~NoteRepeat();
   void OnTimeEvent(double time) override;
   void SetInterval(NoteInterval interval);
   void SetOffset(float offset);

private:
   int mRow{ 0 };
   StepSequencer* mSeq{ nullptr };
   NoteInterval mInterval{ NoteInterval::kInterval_None };
   float mOffset{ 0 };
};

class StepSequencerNoteFlusher : public ITimeListener
{
public:
   StepSequencerNoteFlusher(StepSequencer* seq);
   ~StepSequencerNoteFlusher();
   void SetInterval(NoteInterval interval);
   void OnTimeEvent(double time) override;

private:
   StepSequencer* mSeq;
};

class StepSequencer : public IDrawableModule, public INoteSource, public ITimeListener, public IFloatSliderListener, public IGridControllerListener, public IButtonListener, public IDropdownListener, public INoteReceiver, public IRadioButtonListener, public IIntSliderListener, public IPush2GridController, public IPulseReceiver, public ITextEntryListener, public IDrivableSequencer
{
public:
   StepSequencer();
   ~StepSequencer();
   static IDrawableModule* Create() { return new StepSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void Init() override;
   void Poll() override;
   void PlayStepNote(double time, int note, float val);
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }
   int GetPadPressure(int row) { return mPadPressures[row]; }
   NoteInterval GetStepInterval() const { return mStepInterval; }
   int GetStepNum(double time);
   void Flush(double time)
   {
      if (mEnabled)
         mNoteOutput.Flush(time);
   }
   int GetStep(int step, int pitch);
   void SetStep(int step, int pitch, int velocity);
   int GetRowPitch(int row) const { return mRows[row]->GetRowPitch(); }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

   bool IsMetaStepActive(double time, int col, int row);

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   void Exit() override;
   void KeyPressed(int key, bool isRepeat) override;

   void UpdateLights(bool force = false);
   void UpdateVelocityLights();
   void UpdateMetaLights();

   void DrawRowLabel(const char* label, int row, int x, int y);
   int GetNumSteps(NoteInterval interval, int numMeasures) const;
   Vec2i ControllerToGrid(const Vec2i& controller);
   int GetNumControllerChunks(); //how many vertical chunks of the sequence are there to fit multi-rowed on the controller?
   int GetMetaStep(double time);
   int GetMetaStepMaskIndex(int col, int row) { return MIN(col, META_STEP_MAX - 1) + row * META_STEP_MAX; }
   GridColor GetGridColor(int x, int y);
   void Step(double time, float velocity, int pulseFlags);
   bool HasGridController();
   int GetGridControllerRows();
   int GetGridControllerCols();
   void RandomizeRow(int row);

   struct HeldButton
   {
      HeldButton(int col, int row)
      {
         mCol = col;
         mRow = row;
      }
      int mCol{ 0 };
      int mRow{ 0 };
   };

   enum class NoteInputMode
   {
      PlayStepIndex,
      RepeatHeld
   };

   enum class StepVelocityEntryMode
   {
      Dropdown,
      Slider
   };

   enum class GridControllerMode
   {
      FitMultipleRows,
      SingleRow
   };

   UIGrid* mGrid{ nullptr };
   float mStrength{ kVelocityNormal };
   FloatSlider* mStrengthSlider{ nullptr };
   StepVelocityType mVelocityType{ StepVelocityType::Normal };
   DropdownList* mVelocityTypeDropdown{ nullptr };
   StepVelocityEntryMode mStepVelocityEntryMode{ StepVelocityEntryMode::Dropdown };
   int mGridYOff{ 0 };
   ClickButton* mClearButton{ nullptr };
   int mColorOffset{ 3 };
   DropdownList* mGridYOffDropdown{ nullptr };
   std::array<StepSequencerRow*, NUM_STEPSEQ_ROWS> mRows{};
   bool mAdjustOffsets{ false };
   Checkbox* mAdjustOffsetsCheckbox{ nullptr };
   std::array<float, NUM_STEPSEQ_ROWS> mOffsets{};
   std::array<FloatSlider*, NUM_STEPSEQ_ROWS> mOffsetSlider{};
   std::array<ClickButton*, NUM_STEPSEQ_ROWS> mRandomizeRowButton{};
   std::map<int, int> mPadPressures{};
   NoteInterval mRepeatRate{ NoteInterval::kInterval_None };
   DropdownList* mRepeatRateDropdown{ nullptr };
   std::array<NoteRepeat*, NUM_STEPSEQ_ROWS> mNoteRepeats{};
   int mNumRows{ 8 };
   int mNumMeasures{ 1 };
   IntSlider* mNumMeasuresSlider{ nullptr };
   NoteInterval mStepInterval{ NoteInterval::kInterval_16n };
   DropdownList* mStepIntervalDropdown{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
   GridControlTarget* mVelocityGridController{ nullptr };
   GridControlTarget* mMetaStepGridController{ nullptr };
   int mCurrentColumn{ 0 };
   IntSlider* mCurrentColumnSlider{ nullptr };
   StepSequencerNoteFlusher mFlusher;
   ClickButton* mShiftLeftButton{ nullptr };
   ClickButton* mShiftRightButton{ nullptr };
   std::list<HeldButton> mHeldButtons{};
   juce::uint32* mMetaStepMasks;
   bool mIsSetUp{ false };
   NoteInputMode mNoteInputMode{ NoteInputMode::PlayStepIndex };
   bool mHasExternalPulseSource{ false };
   bool mPush2Connected{ false };
   float mRandomizationAmount{ 1 };
   FloatSlider* mRandomizationAmountSlider{ nullptr };
   float mRandomizationDensity{ .25 };
   FloatSlider* mRandomizationDensitySlider{ nullptr };
   ClickButton* mRandomizeButton{ nullptr };
   GridControllerMode mGridControllerMode{ GridControllerMode::FitMultipleRows };

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
