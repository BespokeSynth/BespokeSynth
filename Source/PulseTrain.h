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
/*
  ==============================================================================

    PulseTrain.h
    Created: 10 Mar 2020 9:15:47pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Transport.h"
#include "Checkbox.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "Slider.h"
#include "IPulseReceiver.h"
#include "UIGrid.h"

class PatchCableSource;

class PulseTrain : public IDrawableModule, public ITimeListener, public IDropdownListener, public IIntSliderListener, public IFloatSliderListener, public IAudioPoller, public IPulseSource, public IPulseReceiver, public UIGridListener
{
public:
   PulseTrain();
   virtual ~PulseTrain();
   static IDrawableModule* Create() { return new PulseTrain(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;
   void Init() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   //IAudioPoller
   void OnTransportAdvanced(float amount) override;

   //ITimeListener
   void OnTimeEvent(double time) override;

   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;

   //IDrawableModule
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   void Step(double time, float velocity, int flags);

   static const int kMaxSteps = 128;
   float mWidth{ 254 };
   float mHeight{ 58 };
   float mVels[kMaxSteps]{};
   int mLength{ 8 };
   IntSlider* mLengthSlider{ nullptr };
   int mStep{ 9999 };
   NoteInterval mInterval{ NoteInterval::kInterval_8n };
   DropdownList* mIntervalSelector{ nullptr };
   bool mResetOnStart{ true };
   Checkbox* mResetOnStartCheckbox{ nullptr };

   PatchCableSource* mStepCables[kMaxSteps]{ nullptr };

   UIGrid* mVelocityGrid{ nullptr };
};
