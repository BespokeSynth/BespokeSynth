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
//  Arpeggiator.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "UIGrid.h"
#include "Scale.h"
#include "ModulationChain.h"

class Arpeggiator : public NoteEffectBase, public IDrawableModule, public ITimeListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IScaleListener
{
public:
   Arpeggiator();
   ~Arpeggiator();
   static IDrawableModule* Create() { return new Arpeggiator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IScaleListener
   void OnScaleChanged() override;

   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;

   std::string GetArpNoteDisplay(int pitch);
   void UpdateInterval();

   struct ArpNote
   {
      ArpNote(int _pitch, int _vel, int _voiceIdx, ModulationParameters _modulation)
      : pitch(_pitch)
      , vel(_vel)
      , voiceIdx(_voiceIdx)
      , modulation(_modulation)
      {}
      int pitch;
      int vel;
      int voiceIdx;
      ModulationParameters modulation;
   };
   std::vector<ArpNote> mChord;

   float mWidth;
   float mHeight;

   NoteInterval mInterval{ NoteInterval::kInterval_16n };
   int mLastPitch{ -1 };
   int mArpIndex{ -1 };
   char mArpString[MAX_TEXTENTRY_LENGTH]{};

   DropdownList* mIntervalSelector{ nullptr };
   int mArpStep{ 1 };
   int mArpPingPongDirection{ 1 };
   IntSlider* mArpStepSlider{ nullptr };

   int mCurrentOctaveOffset{ 0 };
   int mOctaveRepeats{ 1 };
   IntSlider* mOctaveRepeatsSlider{ nullptr };

   std::array<bool, 128> mInputNotes{ false };
   ofMutex mChordMutex;

   TransportListenerInfo* mTransportListenerInfo{ nullptr };
};
