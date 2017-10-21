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
#include "Grid.h"
#include "Slider.h"
#include "GridController.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "RadioButton.h"
#include "SynthGlobals.h"

#define NUM_STEPSEQ_ROWS 16

class StepSequencer;

class StepSequencerRow : public ITimeListener
{
public:
   StepSequencerRow(StepSequencer* seq, Grid* grid, int row);
   ~StepSequencerRow();
   void OnTimeEvent(int samplesTo) override;
   void SetOffset(float offset);
   void UpdateTimeListener();
private:
   Grid* mGrid;
   int mRow;
   StepSequencer* mSeq;
   float mOffset;
};

class NoteRepeat : public ITimeListener
{
public:
   NoteRepeat(StepSequencer* seq, int note);
   ~NoteRepeat();
   void OnTimeEvent(int samplesTo) override;
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
   void OnTimeEvent(int samplesTo) override;
private:
   StepSequencer* mSeq;
};

class StepSequencer : public IDrawableModule, public INoteSource, public ITimeListener, public IFloatSliderListener, public IGridControllerListener, public IButtonListener, public IDropdownListener, public INoteReceiver, public IRadioButtonListener, public IIntSliderListener
{
public:
   StepSequencer();
   ~StepSequencer();
   static IDrawableModule* Create() { return new StepSequencer(); }
   
   string GetTitleLabel() override { return "drum sequencer"; }
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   void PlayNote(int note, float val);
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool Enabled() const override { return mEnabled; }
   int GetPadPressure(int row) { return mPadPressures[row]; }
   NoteInterval GetStepInterval() const { return mStepInterval; }
   int GetStep(float offsetMs);
   void Flush() { if (mEnabled) mNoteOutput.Flush(); }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //ITimeListener
   void OnTimeEvent(int samplesTo) override;
   
   //IGridControllerListener
   void ConnectGridController(IGridController* grid) override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   void OnClicked(int x, int y, bool right) override;
   void Exit() override;
   
   void UpdateLights();
   
   void DrawRowLabel(const char* label, int row, int x, int y);
   void SetPreset(int preset);
   int GetNumSteps(NoteInterval interval) const;
   Vec2i ControllerToGrid(const Vec2i& controller);
   int GetNumControllerChunks(); //how many vertical chunks of the sequence are there to fit multi-rowed on the controller?
   
   Grid* mGrid;
   float mStrength;
   FloatSlider* mStrengthSlider;
   bool mUseStrengthSlider;
   Checkbox* mUseStrengthSliderCheckbox;
   bool mStochasticMode;
   Checkbox* mStochasticCheckbox;
   int mLpYOff;
   DropdownList* mPresetDropdown;
   int mPreset;
   int mColorOffset;
   DropdownList* mLpYOffDropdown;
   StepSequencerRow* mRows[NUM_STEPSEQ_ROWS];
   bool mAdjustOffsets;
   Checkbox* mAdjustOffsetsCheckbox;
   float mOffsets[NUM_STEPSEQ_ROWS];
   FloatSlider* mOffsetSlider[NUM_STEPSEQ_ROWS];
   std::map<int,int> mPadPressures;
   NoteInterval mRepeatRate;
   DropdownList* mRepeatRateDropdown;
   NoteRepeat* mNoteRepeats[NUM_STEPSEQ_ROWS];
   int mHeldRow;
   int mHeldCol;
   int mNumRows;
   int mNumMeasures;
   NoteInterval mStepInterval;
   DropdownList* mStepIntervalDropdown;
   IGridController* mGridController;
   int mCurrentColumn;
   IntSlider* mCurrentColumnSlider;
   StepSequencerNoteFlusher mFlusher;
   ClickButton* mShiftLeftButton;
   ClickButton* mShiftRightButton;
};


#endif /* defined(__modularSynth__StepSequencer__) */

