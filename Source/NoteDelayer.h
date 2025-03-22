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
//  NoteDelayer.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/3/16.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Transport.h"

class NoteDelayer : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   NoteDelayer();
   ~NoteDelayer();
   static IDrawableModule* Create() { return new NoteDelayer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void OnTransportAdvanced(float amount) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 108;
      height = 22;
   }

   float mDelay{ .25 };
   FloatSlider* mDelaySlider{ nullptr };

   float mLastNoteOnTime{ 0 };

   static const int kQueueSize = 500;
   NoteMessage mInputNotes[kQueueSize]{};
   int mConsumeIndex{ 0 };
   int mAppendIndex{ 0 };
};
