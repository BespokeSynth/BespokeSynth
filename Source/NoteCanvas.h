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

class CanvasControls;
class CanvasTimeline;
class CanvasScrollbar;

class NoteCanvas : public IDrawableModule, public INoteSource, public ICanvasListener, public IFloatSliderListener, public IAudioPoller, public IIntSliderListener, public INoteReceiver, public IButtonListener, public IDropdownListener, public ITextEntryListener
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
    * @brief FitNotes adapt measures to fit all added notes
    */
   void FitNotes();

   void OnTransportAdvanced(float amount) override;

   void CanvasUpdated(Canvas* canvas) override;

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
   void GetModuleDimensions(float& width, float& height) override;

   double GetCurPos(double time) const;
   void UpdateNumColumns();
   void SetRecording(bool rec);
   void SetNumMeasures(int numMeasures);
   bool FreeRecordParityMatched();
   void ClipNotes();
   void QuantizeNotes();
   void LoadMidi();
   void SaveMidi();

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

   std::vector<ModulationParameters> mVoiceModulations;
};
