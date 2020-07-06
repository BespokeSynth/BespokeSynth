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

#define NUM_STEPSEQ_ROWS 16
#define META_STEP_MAX 64

class StepSequencer;

class StepSequencerRow : public ITimeListener
{
public:
   StepSequencerRow(StepSequencer* seq, UIGrid* grid, int row);
   ~StepSequencerRow();
   void OnTimeEvent(double time) override;
   void SetOffset(float offset);
   void UpdateTimeListener();
private:
   UIGrid* mGrid;
   int mRow;
   StepSequencer* mSeq;
   float mOffset;
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

class StepSequencer : public IDrawableModule, public INoteSource, public ITimeListener, public IFloatSliderListener, public IGridControllerListener, public IButtonListener, public IDropdownListener, public INoteReceiver, public IRadioButtonListener, public IIntSliderListener, public IPush2GridController
{
public:
   StepSequencer();
   ~StepSequencer();
   static IDrawableModule* Create() { return new StepSequencer(); }
   
   string GetTitleLabel() override { return "drum sequencer"; }
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
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendPressure(int pitch, int pressure) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
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
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   void Exit() override;
   void KeyPressed(int key, bool isRepeat) override;
   
   void UpdateLights();
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
   
   struct HeldButton
   {
      HeldButton(int col, int row) { mCol = col; mRow = row; mTime = gTime; }
      int mCol;
      int mRow;
      double mTime;
   };
   
   UIGrid* mGrid;
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
   int mNumRows;
   int mNumMeasures;
   NoteInterval mStepInterval;
   DropdownList* mStepIntervalDropdown;
   GridController* mGridController;
   GridController* mVelocityGridController;
   GridController* mMetaStepGridController;
   int mCurrentColumn;
   IntSlider* mCurrentColumnSlider;
   StepSequencerNoteFlusher mFlusher;
   ClickButton* mShiftLeftButton;
   ClickButton* mShiftRightButton;
   std::list<HeldButton> mHeldButtons;
   uint32* mMetaStepMasks;
};


#endif /* defined(__modularSynth__StepSequencer__) */

