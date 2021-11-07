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

#ifndef __modularSynth__StepSequencer__
#define __modularSynth__StepSequencer__

#include <iostream>
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
   UIGrid* mGrid;
   int mRow;
   StepSequencer* mSeq;
   float mOffset;

   struct PlayedStep
   {
      PlayedStep() : time(-1) {}
      int step;
      double time;
   };
   std::array<PlayedStep, 5> mPlayedSteps;
   int mPlayedStepsRoundRobin;
   TextEntry* mRowPitchEntry;
   int mRowPitch;
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
   int mRow;
   StepSequencer* mSeq;
   NoteInterval mInterval;
   float mOffset;
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
   
   
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   void PlayStepNote(double time, int note, float val);
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool Enabled() const override { return mEnabled; }
   int GetPadPressure(int row) { return mPadPressures[row]; }
   NoteInterval GetStepInterval() const { return mStepInterval; }
   int GetStepNum(double time);
   void Flush(double time) { if (mEnabled) mNoteOutput.Flush(time); }
   int GetStep(int step, int pitch);
   void SetStep(int step, int pitch, int velocity);
   int GetRowPitch(int row) const { return mRows[row]->GetRowPitch(); }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
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
   bool OnPush2Control(MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;
   
   bool IsMetaStepActive(double time, int col, int row);

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   void Exit() override;
   void KeyPressed(int key, bool isRepeat) override;
   
   void UpdateLights(bool force = false);
   void UpdateVelocityLights();
   void UpdateMetaLights();
   
   void DrawRowLabel(const char* label, int row, int x, int y);
   void SetPreset(int preset);
   int GetNumSteps(NoteInterval interval) const;
   Vec2i ControllerToGrid(const Vec2i& controller);
   int GetNumControllerChunks(); //how many vertical chunks of the sequence are there to fit multi-rowed on the controller?
   int GetMetaStep(double time);
   int GetMetaStepMaskIndex(int col, int row) { return MIN(col, META_STEP_MAX-1) + row * META_STEP_MAX; }
   GridColor GetGridColor(int x, int y);
   void Step(double time, float velocity, int pulseFlags);
   bool HasGridController();
   int GetGridControllerRows();
   int GetGridControllerCols();
   
   struct HeldButton
   {
      HeldButton(int col, int row) { mCol = col; mRow = row; mTime = gTime; }
      int mCol;
      int mRow;
      double mTime;
   };

   enum class NoteInputMode
   {
      PlayStepIndex,
      RepeatHeld
   };
   
   UIGrid* mGrid;
   float mStrength;
   FloatSlider* mStrengthSlider;
   int mGridYOff;
   DropdownList* mPresetDropdown;
   int mPreset;
   int mColorOffset;
   DropdownList* mGridYOffDropdown;
   std::array<StepSequencerRow*, NUM_STEPSEQ_ROWS> mRows;
   bool mAdjustOffsets;
   Checkbox* mAdjustOffsetsCheckbox;
   std::array<float, NUM_STEPSEQ_ROWS> mOffsets;
   std::array<FloatSlider*, NUM_STEPSEQ_ROWS> mOffsetSlider;
   std::array<bool, NUM_STEPSEQ_ROWS> mRandomLock;
   std::array<Checkbox*, NUM_STEPSEQ_ROWS> mRandomLockCheckbox;
   std::map<int,int> mPadPressures;
   NoteInterval mRepeatRate;
   DropdownList* mRepeatRateDropdown;
   std::array<NoteRepeat*, NUM_STEPSEQ_ROWS> mNoteRepeats;
   int mNumRows;
   int mNumMeasures;
   IntSlider* mNumMeasuresSlider;
   NoteInterval mStepInterval;
   DropdownList* mStepIntervalDropdown;
   GridControlTarget* mGridControlTarget;
   GridControlTarget* mVelocityGridController;
   GridControlTarget* mMetaStepGridController;
   int mCurrentColumn;
   IntSlider* mCurrentColumnSlider;
   StepSequencerNoteFlusher mFlusher;
   ClickButton* mShiftLeftButton;
   ClickButton* mShiftRightButton;
   std::list<HeldButton> mHeldButtons;
   juce::uint32* mMetaStepMasks;
   bool mIsSetUp;
   NoteInputMode mNoteInputMode;
   bool mHasExternalPulseSource;
   bool mPush2Connected;
   float mRandomizationAmount;
   FloatSlider* mRandomizationAmountSlider;
   float mRandomizationDensity;
   FloatSlider* mRandomizationDensitySlider;
   ClickButton* mRandomizeButton;

   TransportListenerInfo* mTransportListenerInfo;
};


#endif /* defined(__modularSynth__StepSequencer__) */

