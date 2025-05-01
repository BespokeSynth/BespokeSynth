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

#pragma once

#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteSource.h"
#include "Slider.h"
#include "UIGrid.h"
#include "MidiController.h"
#include "Scale.h"
#include "IPulseReceiver.h"
#include "GridController.h"
#include "IDrivableSequencer.h"
#include "Push2Control.h"

#define NSS_MAX_STEPS 32

class PatchCableSource;

class NoteStepSequencer : public IDrawableModule, public ITimeListener, public INoteSource, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public MidiDeviceListener, public UIGridListener, public IAudioPoller, public IScaleListener, public INoteReceiver, public IPulseReceiver, public IGridControllerListener, public IDrivableSequencer, public IPush2GridController
{
public:
   NoteStepSequencer();
   virtual ~NoteStepSequencer();
   static IDrawableModule* Create() { return new NoteStepSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void Init() override;
   void SetEnabled(bool enabled) override;

   void SetMidiController(std::string name);

   UIGrid* GetGrid() const { return mGrid; }

   enum NoteMode
   {
      kNoteMode_Scale,
      kNoteMode_Chromatic,
      kNoteMode_Pentatonic,
      kNoteMode_Fifths
   };

   static int RowToPitch(NoteMode noteMode, int row, int octave, int rowOffset);

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
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

   //IPush2GridController
   bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) override;
   void UpdatePush2Leds(Push2Control* push2) override;

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
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

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
   void KeyPressed(int key, bool isRepeat) override;

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
   void RandomizeVelocities();
   void RandomizeLengths();
   void Step(double time, float velocity, int pulseFlags);
   void SendNoteToCable(int index, double time, int pitch, int velocity);
   void GetPush2Layout(int& sequenceRows, int& pitchCols, int& pitchRows);

   int mTones[NSS_MAX_STEPS]{};
   int mVels[NSS_MAX_STEPS]{};
   float mNoteLengths[NSS_MAX_STEPS]{};

   NoteInterval mInterval{ NoteInterval::kInterval_8n };
   int mArpIndex{ -1 };

   DropdownList* mIntervalSelector{ nullptr };
   UIGrid* mGrid{ nullptr };
   UIGrid* mVelocityGrid{ nullptr };
   int mLastPitch{ -1 };
   int mLastStepIndex{ -1 };
   float mLastNoteLength{ 1 };
   double mLastNoteEndTime{ 0 };
   bool mAlreadyDidNoteOff{ false };
   int mOctave{ 3 };
   IntSlider* mOctaveSlider{ nullptr };
   NoteMode mNoteMode{ NoteMode::kNoteMode_Scale };
   DropdownList* mNoteModeSelector{ nullptr };
   IntSlider* mLoopResetPointSlider{ nullptr };
   int mLoopResetPoint{ 0 };
   int mStepLengthSubdivisions{ 2 };

   int mLength{ 8 };
   IntSlider* mLengthSlider{ nullptr };
   bool mSetLength{ false };
   int mNoteRange{ 15 };
   bool mShowStepControls{ false };
   int mRowOffset{ 0 };

   MidiController* mController{ nullptr };

   ClickButton* mShiftBackButton{ nullptr };
   ClickButton* mShiftForwardButton{ nullptr };
   ClickButton* mClearButton{ nullptr };

   ClickButton* mRandomizeAllButton{ nullptr };
   ClickButton* mRandomizePitchButton{ nullptr };
   ClickButton* mRandomizeLengthButton{ nullptr };
   ClickButton* mRandomizeVelocityButton{ nullptr };
   float mRandomizePitchChance{ 1 };
   int mRandomizePitchVariety{ 4 };
   float mRandomizeLengthChance{ 1 };
   float mRandomizeLengthRange{ 1 };
   float mRandomizeVelocityChance{ 1 };
   float mRandomizeVelocityDensity{ .6 };
   FloatSlider* mRandomizePitchChanceSlider{ nullptr };
   IntSlider* mRandomizePitchVarietySlider{ nullptr };
   FloatSlider* mRandomizeLengthChanceSlider{ nullptr };
   FloatSlider* mRandomizeLengthRangeSlider{ nullptr };
   FloatSlider* mRandomizeVelocityChanceSlider{ nullptr };
   FloatSlider* mRandomizeVelocityDensitySlider{ nullptr };

   std::array<double, NSS_MAX_STEPS> mLastStepPlayTime{ -1 };
   std::array<DropdownList*, NSS_MAX_STEPS> mToneDropdowns;
   std::array<IntSlider*, NSS_MAX_STEPS> mVelocitySliders;
   std::array<FloatSlider*, NSS_MAX_STEPS> mLengthSliders;

   bool mHasExternalPulseSource{ false };

   std::array<AdditionalNoteCable*, NSS_MAX_STEPS> mStepCables;

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
   GridControlTarget* mGridControlTarget{ nullptr };
   int mGridControlOffsetX{ 0 };
   int mGridControlOffsetY{ 0 };
   IntSlider* mGridControlOffsetXSlider{ nullptr };
   IntSlider* mGridControlOffsetYSlider{ nullptr };
   int mPush2HeldStep{ -1 };
   bool mPush2HeldStepWasEdited{ false };
   double mPush2ButtonPressTime{ -1 };
   int mQueuedPush2Tone{ -1 };
   int mQueuedPush2Vel{ 127 };
   float mQueuedPush2Length{ 1 };
   bool mGridSyncQueued{ false };
   bool mPush2VelocityHeld{ false };
   bool mPush2LengthHeld{ false };

   enum class Push2GridDisplayMode
   {
      PerStep,
      GridView
   };
   Push2GridDisplayMode mPush2GridDisplayMode{ Push2GridDisplayMode::PerStep };
};
