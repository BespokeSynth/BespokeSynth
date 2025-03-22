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

    ControlRecorder.h
    Created: 7 Apr 2024
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Curve.h"
#include "IModulator.h"
#include "Slider.h"

class PatchCableSource;

class ControlRecorder : public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener, public IModulator
{
public:
   ControlRecorder();
   ~ControlRecorder();
   static IDrawableModule* Create() { return new ControlRecorder(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetRecording(bool on);
   float GetLength() const { return mLength; }

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return IsEnabled() && mHasRecorded && !mRecord; }
   bool CanAdjustRange() const override { return false; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   float GetPlaybackTime(double time);
   void RecordPoint();
   void Clear();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   Curve mCurve{ 0 };
   bool mHasRecorded{ false };
   float mLength{ 0 };
   bool mQuantizeLength{ true };
   Checkbox* mQuantizeLengthCheckbox{ nullptr };
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_1n };
   DropdownList* mQuantizeLengthSelector{ nullptr };
   float mSpeed{ 1 };
   FloatSlider* mSpeedSlider{ nullptr };
   ClickButton* mClearButton{ nullptr };
   float mDisplayStartY{ 0 };
   float mWidth{ 220 };
   float mHeight{ 100 };
   double mRecordStartOffset{ 0 };
   Checkbox* mRecordCheckbox{ nullptr };
   bool mRecord{ false };
   IUIControl* mConnectedControl{ nullptr };
};
