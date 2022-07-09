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
#include "IPulseReceiver.h"
#include "GridController.h"
#include "IDrivableSequencer.h"

#define NSS_MAX_STEPS 32

class PatchCableSource;

class NoteStepSequencer : public IDrawableModule, public ITimeListener, public INoteSource, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public MidiDeviceListener, public UIGridListener, public IAudioPoller, public IScaleListener, public INoteReceiver, public IPulseReceiver, public IGridControllerListener, public IDrivableSequencer
{
public:
   NoteStepSequencer();
   virtual ~NoteStepSequencer();
   static IDrawableModule* Create() { return new NoteStepSequencer(); }


   void CreateUIControls() override;

   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void SetMidiController(std::string name);

   UIGrid* GetGrid() const { return mGrid; }

   int RowToPitch(int row);
   int PitchToRow(int pitch);
   void SetStep(int index, int step, int velocity, float length);
   void SetPitch(int index, int pitch, int velocity, float length);

   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void Poll() override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

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

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(float x, float y, bool right) override;
   void UpdateGridControllerLights(bool force);

   int ButtonToStep(int button);
   int StepToButton(int step);
   void SyncGridToSeq();
   void UpdateLights();
   void ShiftSteps(int amount);
   void UpdateVelocityGridPos();
   void SetUpStepControls();
   float ExtraWidth() const;
   float ExtraHeight() const;
   void RandomizePitches(bool fifths);
   void Step(double time, float velocity, int pulseFlags);
   void SendNoteToCable(int index, double time, int pitch, int velocity);

   enum NoteMode
   {
      kNoteMode_Scale,
      kNoteMode_Chromatic,
      kNoteMode_Pentatonic,
      kNoteMode_Fifths
   };

   int mTones[NSS_MAX_STEPS];
   int mVels[NSS_MAX_STEPS];
   float mNoteLengths[NSS_MAX_STEPS];

   NoteInterval mInterval;
   int mArpIndex;

   DropdownList* mIntervalSelector;
   Checkbox* mRepeatIsHoldCheckbox;
   UIGrid* mGrid;
   UIGrid* mVelocityGrid;
   int mLastPitch;
   int mLastVel;
   int mLastStepIndex;
   float mLastNoteLength;
   double mLastNoteStartTime;
   double mLastNoteEndTime;
   bool mAlreadyDidNoteOff;
   int mOctave;
   IntSlider* mOctaveSlider;
   NoteMode mNoteMode;
   DropdownList* mNoteModeSelector;
   IntSlider* mLoopResetPointSlider;
   int mLoopResetPoint;

   int mLength;
   IntSlider* mLengthSlider;
   bool mSetLength;
   int mNoteRange;
   bool mShowStepControls;
   int mRowOffset;

   MidiController* mController;

   ClickButton* mShiftBackButton;
   ClickButton* mShiftForwardButton;
   ClickButton* mClearButton;

   ClickButton* mRandomizePitchButton;
   ClickButton* mRandomizeLengthButton;
   ClickButton* mRandomizeVelocityButton;
   float mRandomizePitchChance;
   int mRandomizePitchVariety;
   float mRandomizeLengthChance;
   float mRandomizeLengthRange;
   float mRandomizeVelocityChance;
   float mRandomizeVelocityDensity;
   FloatSlider* mRandomizePitchChanceSlider;
   IntSlider* mRandomizePitchVarietySlider;
   FloatSlider* mRandomizeLengthChanceSlider;
   FloatSlider* mRandomizeLengthRangeSlider;
   FloatSlider* mRandomizeVelocityChanceSlider;
   FloatSlider* mRandomizeVelocityDensitySlider;

   std::array<double, NSS_MAX_STEPS> mLastStepPlayTime{ -1 };
   std::array<DropdownList*, NSS_MAX_STEPS> mToneDropdowns;
   std::array<IntSlider*, NSS_MAX_STEPS> mVelocitySliders;
   std::array<FloatSlider*, NSS_MAX_STEPS> mLengthSliders;

   bool mHasExternalPulseSource;

   std::array<AdditionalNoteCable*, NSS_MAX_STEPS> mStepCables;

   TransportListenerInfo* mTransportListenerInfo;
   GridControlTarget* mGridControlTarget;
   int mGridControlOffsetX;
   int mGridControlOffsetY;
   IntSlider* mGridControlOffsetXSlider;
   IntSlider* mGridControlOffsetYSlider;
};


#endif /* defined(__modularSynth__NoteStepSequencer__) */
