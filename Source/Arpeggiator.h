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

#ifndef __modularSynth__Arpeggiator__
#define __modularSynth__Arpeggiator__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
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


   void CreateUIControls() override;

   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //IScaleListener
   void OnScaleChanged() override;

   //IButtonListener
   void ButtonClicked(ClickButton* button) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   //IDropdownListener
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   //IIntSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   bool Enabled() const override { return mEnabled; }
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

   NoteInterval mInterval;
   int mLastPitch;
   int mArpIndex;
   char mArpString[MAX_TEXTENTRY_LENGTH];

   DropdownList* mIntervalSelector;
   int mArpStep;
   int mArpPingPongDirection;
   IntSlider* mArpStepSlider;

   int mCurrentOctaveOffset;
   int mOctaveRepeats;
   IntSlider* mOctaveRepeatsSlider;

   std::array<bool, 128> mInputNotes{ false };
   ofMutex mChordMutex;

   TransportListenerInfo* mTransportListenerInfo;
};

#endif /* defined(__modularSynth__Arpeggiator__) */
