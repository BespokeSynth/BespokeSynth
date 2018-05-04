//
//  NoteStepSequencer.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/3/13.
//
//

#ifndef __modularSynth__NoteStepSequencer__
#define __modularSynth__NoteStepSequencer__

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
#include "MidiController.h"
#include "Scale.h"

#define NSS_MAX_STEPS 16

class NoteStepSequencer : public IDrawableModule, public ITimeListener, public INoteSource, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public MidiDeviceListener, public UIGridListener, public IAudioPoller, public IScaleListener, public INoteReceiver
{
public:
   NoteStepSequencer();
   virtual ~NoteStepSequencer();
   static IDrawableModule* Create() { return new NoteStepSequencer(); }
   
   string GetTitleLabel() override { return "note sequencer"; }
   void CreateUIControls() override;
   
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void SetMidiController(string name);
   
   UIGrid* GetGrid() const { return mGrid; }
   
   int RowToPitch(int row);
   int PitchToRow(int pitch);
   
   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //ITimeListener
   void OnTimeEvent(int samplesTo) override;
   
   //IScaleListener
   void OnScaleChanged() override;
   
   //MidiDeviceListener
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   int ButtonToStep(int button);
   int StepToButton(int step);
   void SyncGridToSeq();
   void UpdateLights();
   void ShiftSteps(int amount);
   void UpdateVelocityGridPos();
   void SetUpStepControls();
   float ExtraWidth() const;
   float ExtraHeight() const;
   
   enum NoteMode
   {
      kNoteMode_Scale,
      kNoteMode_Chromatic,
      kNoteMode_Fifths
   };
   
   enum TimeMode
   {
      kTimeMode_Synced,
      kTimeMode_Downbeat,
      kTimeMode_Downbeat2,
      kTimeMode_Downbeat4,
      kTimeMode_Step,
      kTimeMode_Free
   };
   
   int mTones[NSS_MAX_STEPS];
   int mVels[NSS_MAX_STEPS];
   float mNoteLengths[NSS_MAX_STEPS];
   
   NoteInterval mInterval;
   int mArpIndex;
   
   DropdownList* mIntervalSelector;
   Checkbox* mRepeatIsHoldCheckbox;
   int mArpStep;
   int mArpPingPongDirection;
   IntSlider* mArpStepSlider;
   TimeMode mTimeMode;
   DropdownList* mTimeModeSelector;
   UIGrid* mGrid;
   UIGrid* mVelocityGrid;
   int mLastPitch;
   int mLastVel;
   float mLastNoteLength;
   double mLastNoteStartTime;
   bool mAlreadyDidNoteOff;
   int mOctave;
   IntSlider* mOctaveSlider;
   bool mHold;
   Checkbox* mHoldCheckbox;
   NoteMode mNoteMode;
   DropdownList* mNoteModeSelector;
   bool mWaitingForDownbeat;
   float mOffset;
   FloatSlider* mOffsetSlider;
   IntSlider* mLoopResetPointSlider;
   int mLoopResetPoint;
   
   int mLength;
   IntSlider* mLengthSlider;
   bool mSetLength;
   int mNoteRange;
   int mNumSteps;
   FloatSlider* mFreeTimeSlider;
   float mFreeTimeStep;
   float mFreeTimeCounter;
   bool mShowStepControls;
   int mRowOffset;
   
   MidiController* mController;

   ClickButton* mShiftBackButton;
   ClickButton* mShiftForwardButton;
   
   ClickButton* mRandomizePitchButton;
   ClickButton* mRandomizeLengthButton;
   ClickButton* mRandomizeVelocityButton;
   
   DropdownList* mToneDropdowns[NSS_MAX_STEPS];
   IntSlider* mVelocitySliders[NSS_MAX_STEPS];
   FloatSlider* mLengthSliders[NSS_MAX_STEPS];
};


#endif /* defined(__modularSynth__NoteStepSequencer__) */

