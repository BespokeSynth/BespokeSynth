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
//  NoteLooper.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/31/13.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "MidiDevice.h"
#include "Scale.h"
#include "Canvas.h"
#include "CanvasElement.h"
#include "ClickButton.h"

class NoteLooper : public IDrawableModule, public NoteEffectBase, public IAudioPoller, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener
{
public:
   NoteLooper();
   ~NoteLooper();
   static IDrawableModule* Create() { return new NoteLooper(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   bool DrawToPush2Screen() override;

   int GetNumMeasures() const { return mNumMeasures; }
   void SetNumMeasures(int numMeasures);

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   double GetCurPos(double time) const;
   NoteCanvasElement* AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());
   int GetNewVoice(int voiceIdx);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   struct SavedPattern
   {
      ClickButton* mStoreButton{ nullptr };
      ClickButton* mLoadButton{ nullptr };
      std::vector<CanvasElement*> mNotes;
   };

   float mWidth{ 370 };
   float mHeight{ 140 };
   int mMinRow{ 127 };
   int mMaxRow{ 0 };
   bool mWrite{ false };
   Checkbox* mWriteCheckbox{ nullptr };
   bool mDeleteOrMute{ false };
   Checkbox* mDeleteOrMuteCheckbox{ nullptr };
   IntSlider* mNumMeasuresSlider{ nullptr };
   int mNumMeasures{ 1 };
   std::vector<CanvasElement*> mNoteChecker{ 128 };
   std::array<NoteCanvasElement*, 128> mInputNotes{};
   std::array<NoteCanvasElement*, 128> mCurrentNotes{};
   Canvas* mCanvas{ nullptr };
   ClickButton* mClearButton{ nullptr };
   int mVoiceRoundRobin{ kNumVoices - 1 };
   bool mAllowLookahead{ true };

   std::array<ModulationParameters, kNumVoices + 1> mVoiceModulations{};
   std::array<int, kNumVoices> mVoiceMap{};

   std::array<SavedPattern, 4> mSavedPatterns;
};
