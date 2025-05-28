/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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
//  BassLineSequencer.h
//  BespokeSynth
//
//  Created by Ryan Challinor on 1/15/25.
//
//

#pragma once

#include "ClickButton.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "INoteSource.h"
#include "IPulseReceiver.h"
#include "Slider.h"
#include "IDrivableSequencer.h"
#include "UIGrid.h"
#include "Scale.h"
#include "NoteStepSequencer.h"

class BassLineSequencer : public IDrawableModule, public IButtonListener, public IDropdownListener, public IIntSliderListener, public ITimeListener, public IPulseReceiver, public INoteSource, public INoteReceiver, IDrivableSequencer, public UIGridListener, public IFloatSliderListener, public IScaleListener
{
public:
   BassLineSequencer();
   virtual ~BassLineSequencer();
   static IDrawableModule* Create() { return new BassLineSequencer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   //IDrawableModule
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IScaleListener
   void OnScaleChanged() override;

   //IPulseReceiver
   void OnPulse(double time, double velocity, int flags) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IDrivableSequencer
   bool HasExternalPulseSource() const override { return mHasExternalPulseSource; }
   void ResetExternalPulseSource() override { mHasExternalPulseSource = false; }

   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, double value, double oldValue) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(double& width, double& height) override;
   void OnClicked(double x, double y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(double x, double y) override;
   void KeyPressed(int key, bool isRepeat) override;
   bool ShouldSerializeForSnapshot() override { return true; }

   void StepBy(double time, double velocity, int flags);
   void ResetStep();
   void UpdateStepControls();
   void OnEditStepUpdated();
   ofVec2d GetNoteDrawPos(int stepIdx, double displayWidth, double displayHeight, double lineWidth, bool end);
   void UpdatePitchLabels();
   int GetPageCount() const { return (int)ceil(static_cast<double>(mLength) / mNumVisibleStepControls); }
   double GetDisplayWidth() const { return mWidth - 6; }

   static const int kMaxSteps = 128;
   static const int kMaxStepControls = 16;
   static const int kEditStepControlIndex = kMaxStepControls;
   static constexpr double kDisplayX = 3;
   static constexpr double kDisplayHeight = 50;

   struct Step
   {
      Step() {}

      int mTone{ 0 };
      double mVelocity{ 0 };
      bool mTie{ false };
   };

   struct StepControl
   {
      double xPos{ 0 };
      double yPos{ 0 };
      double xMax{ 0 };
      double yMax{ 0 };

      int mTone{ 8 };
      double mVelocity{ 0 };
      bool mTie{ false };

      UIGrid* mGridSquare{ nullptr };
      DropdownList* mToneDropdown{ nullptr };
      Checkbox* mTieCheckbox{ nullptr };
   };

   enum class GlideMode
   {
      PitchBend,
      Legato,
      SlideCC
   };

   std::array<Step, kMaxSteps> mSteps;
   std::array<StepControl, kMaxStepControls + 1> mStepControls;
   double mWidth{ 0 }, mHeight{ 0 };
   bool mHasExternalPulseSource{ false };
   TransportListenerInfo* mTransportListenerInfo{ nullptr };

   double mNoteDisplayY{ 0 };
   int mStepIdx{ 0 };
   int mPlayingPitch{ -1 };
   int mLastVelocity{ 0 };
   bool mLastWasTied{ false };
   int mNoteRange{ 22 };
   int mNumVisibleStepControls{ 16 };
   int mHighlightDisplayStepIdx{ -1 };
   int mHighlightStepControlIdx{ -1 };
   ofVec2d mMouseHoverPos{};
   int mHeldInputPitch{ -1 };
   int mWriteNewNotePitch{ -1 };
   double mLastInputVelocity{ 0.0 };

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   DropdownList* mIntervalSelector{ nullptr };
   int mLength{ 32 };
   IntSlider* mLengthSlider{ nullptr };
   int mOctave{ 1 };
   IntSlider* mOctaveSlider{ nullptr };
   NoteStepSequencer::NoteMode mNoteMode{ NoteStepSequencer::NoteMode::kNoteMode_Scale };
   DropdownList* mNoteModeSelector{ nullptr };
   double mGlideTime{ 35 };
   FloatSlider* mGlideSlider{ nullptr };
   ClickButton* mClearButton{ nullptr };
   ClickButton* mShiftLeftButton{ nullptr };
   ClickButton* mShiftRightButton{ nullptr };
   ClickButton* mRandomizeButton{ nullptr };
   double mRandomDensity{ 0.65 };
   FloatSlider* mRandomDensitySlider{ nullptr };
   int mRandomVariety{ 7 };
   IntSlider* mRandomVarietySlider{ nullptr };
   double mRandomAccents{ 0.25 };
   FloatSlider* mRandomAccentsSlider{ nullptr };
   double mRandomTies{ 0.3 };
   FloatSlider* mRandomTiesSlider{ nullptr };
   int mEditPage{ 0 };
   IntSlider* mEditPageSlider{ nullptr };
   int mEditStepControl{ -1 };
   IntSlider* mEditStepControlSlider{ nullptr };
   GlideMode mGlideMode{ GlideMode::PitchBend };
   DropdownList* mGlideModeSelector{ nullptr };

   ModulationParameters mModulation{};
   ModulationChain mPitchBend{ ModulationParameters::kDefaultPitchBend };
};
