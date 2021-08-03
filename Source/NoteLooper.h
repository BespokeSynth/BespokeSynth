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

#include <iostream>
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
   
   string GetTitleLabel() override { return "note looper"; }
   void CreateUIControls() override;
   void Init() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   double GetCurPos(double time) const;
   NoteCanvasElement* AddNote(double measurePos, int pitch, int velocity, double length, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters());
   void SetNumMeasures(int numMeasures);
   int GetNewVoice(int voiceIdx);
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=mWidth; height=mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   struct NoteEvent
   {
      bool mValid;
      float mPos;
      int mPitch;
      int mVelocity;
      int mJustPlaced;
      int mAssociatedEvent;   //associated note on/off
   };
   
   struct CurrentNote
   {
      int mPitch;
      int mVelocity;
   };

   struct SavedPattern
   {
      ClickButton* mStoreButton;
      ClickButton* mLoadButton;
      vector<CanvasElement*> mNotes;
   };

   float mWidth;
   float mHeight;
   int mMinRow;
   int mMaxRow;
   bool mWrite;
   Checkbox* mWriteCheckbox;
   bool mDeleteOrMute;
   Checkbox* mDeleteOrMuteCheckbox;
   IntSlider* mNumMeasuresSlider;
   int mNumMeasures;
   vector<CanvasElement*> mNoteChecker {128};
   std::array<NoteCanvasElement*, 128> mInputNotes {};
   std::array<NoteCanvasElement*, 128> mCurrentNotes {};
   Canvas* mCanvas;
   ClickButton* mClearButton;
   int mVoiceRoundRobin;

   std::array<ModulationParameters, kNumVoices+1> mVoiceModulations {};
   std::array<int, kNumVoices> mVoiceMap {};

   std::array<SavedPattern, 4> mSavedPatterns;
};

