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
//  NoteCanvas.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/30/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Canvas.h"
#include "Slider.h"
#include "INoteReceiver.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "AbletonDeviceShared.h"
#include "LaunchpadKeyboard.h"
#include "IInputRecordable.h"

class CanvasControls;
class CanvasTimeline;
class CanvasScrollbar;

class NoteCanvas : public IDrawableModule, public INoteSource, public ICanvasListener, public IFloatSliderListener, public IAudioPoller, public IIntSliderListener, public INoteReceiver, public IButtonListener, public IDropdownListener, public ITextEntryListener, public IAbletonGridController, public IInputRecordable
{
public:
   NoteCanvas();
   ~NoteCanvas();
   static IDrawableModule* Create() { return new NoteCanvas(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void KeyPressed(int key, bool isRepeat) override;

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void Clear(double time);
   NoteCanvasElement* AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());

   /**
    * @brief FitNotes adapt measures and pitch range to fit all added notes
    */
   void FitNotes(bool length = true, bool pitchRange = false);

   void OnTransportAdvanced(float amount) override;

   void CanvasUpdated(Canvas* canvas) override;

   //IAbletonGridController
   bool OnAbletonGridControl(IAbletonGridDevice* abletonGrid, int controlIndex, float midiValue) override;
   void UpdateAbletonGridLeds(IAbletonGridDevice* abletonGrid) override;
   bool UpdateAbletonMoveScreen(IAbletonGridDevice* abletonGrid, AbletonMoveLCD* lcd) override;
   bool HasHighPriorityAbletonMoveScreenUpdate(IAbletonGridDevice* abletonGrid) override;

   //IInputRecordable
   void SetRecording(bool record) override;
   bool IsRecording() const override { return mRecord; }
   void ClearRecording() override { Clear(NextBufferTime(false)); }
   void CancelRecording() override { SetRecording(false); }
   float GetRecordingLengthMeasures() const override { return mNumMeasures; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;

   double GetCurPos(double time) const;
   void UpdateNumColumns();
   void SetNumMeasures(int numMeasures);
   bool FreeRecordParityMatched();
   void ClipNotes();
   void QuantizeNotes();
   void LoadMidi();
   void SaveMidi();
   bool RemoveEditPitch(int pitch);
   bool AddEditPitch(int pitch, bool atEveryMeasure);
   bool ToggleEditPitch(int pitch, bool atEveryMeasure);
   void DoubleLoop();
   void CopyNotesToClipboard(int stepIndex);
   std::string GetCurrentEditMeasureString() const;

   Canvas* mCanvas{ nullptr };
   CanvasControls* mCanvasControls{ nullptr };
   CanvasTimeline* mCanvasTimeline{ nullptr };
   CanvasScrollbar* mCanvasScrollbarHorizontal{ nullptr };
   CanvasScrollbar* mCanvasScrollbarVertical{ nullptr };
   std::vector<CanvasElement*> mNoteChecker{ 128 };
   std::vector<NoteCanvasElement*> mInputNotes{ 128 };
   std::vector<NoteCanvasElement*> mCurrentNotes{ 128 };
   IntSlider* mNumMeasuresSlider{ nullptr };
   int mNumMeasures{ 1 };
   ClickButton* mQuantizeButton{ nullptr };
   ClickButton* mSaveMidiButton{ nullptr };
   ClickButton* mLoadMidiButton{ nullptr };
   TextEntry* mLoadMidiTrackEntry{ nullptr };
   int mLoadMidiTrack{ 1 };
   ClickButton* mClipButton{ nullptr };
   bool mPlay{ true };
   Checkbox* mPlayCheckbox{ nullptr };
   bool mRecord{ false };
   Checkbox* mRecordCheckbox{ nullptr };
   bool mStopQueued{ false };
   NoteInterval mInterval{ NoteInterval::kInterval_8n };
   DropdownList* mIntervalSelector{ nullptr };
   bool mFreeRecord{ false };
   Checkbox* mFreeRecordCheckbox{ nullptr };
   int mFreeRecordStartMeasure{ 0 };
   bool mShowIntervals{ false };
   Checkbox* mShowIntervalsCheckbox{ nullptr };
   LaunchpadKeyboard* mGridKeyboardInterface{ nullptr };

   int mEditMeasureOffset{ 0 };
   float mEditMeasureOffsetSlider{ 0.0f };
   double mEditHoldTime{ 0.0 };
   double mCopyHoldTime{ 0.0 };
   int mEditHoldStep{ -1 };
   int mEditCurrentPitchContext{ -1 };
   bool mHasMadeStepEdit{ false };
   std::vector<NoteCanvasElement*> mCurrentEditElements{};
   std::vector<NoteCanvasElement*> mClipboardElements{};
   int mClipboardCopyFromStep{ 0 };
   std::array<bool, 128> mPitchMuted{};
   float mPlaceNoteVelocity{ kVelocityNormal };

   std::vector<ModulationParameters> mVoiceModulations;
};
