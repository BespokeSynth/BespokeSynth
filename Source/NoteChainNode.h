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
//  NoteChainNode.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/16.
//
//

#pragma once

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "Slider.h"
#include "Transport.h"
#include "DropdownList.h"
#include "IPulseReceiver.h"

class NoteChainNode : public IDrawableModule, public INoteSource, public IButtonListener, public ITextEntryListener, public IFloatSliderListener, public IAudioPoller, public IDropdownListener, public ITimeListener, public IPulseSource, public IPulseReceiver
{
public:
   NoteChainNode();
   virtual ~NoteChainNode();
   static IDrawableModule* Create() { return new NoteChainNode(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void OnTimeEvent(double time) override;
   void OnTransportAdvanced(float amount) override;

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void TextEntryComplete(TextEntry* entry) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 110;
      h = 76;
   }

   void TriggerNote(double time);

   ClickButton* mTriggerButton{ nullptr };
   TextEntry* mPitchEntry{ nullptr };
   FloatSlider* mVelocitySlider{ nullptr };
   FloatSlider* mDurationSlider{ nullptr };
   DropdownList* mNextSelector{ nullptr };
   int mPitch{ 48 };
   float mVelocity{ 1 };
   float mDuration{ .25 };
   float mDurationMs{ 50 };
   NoteInterval mNextInterval{ NoteInterval::kInterval_8n };
   float mNext{ 0 };
   double mStartTime{ 0 };
   bool mNoteOn{ false };
   bool mWaitingToTrigger{ false };
   bool mQueueTrigger{ false };
   PatchCableSource* mNextNodeCable{ nullptr };
};
