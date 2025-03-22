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

    EnvelopeModulator.h
    Created: 16 Nov 2017 10:28:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "ADSR.h"
#include "ADSRDisplay.h"
#include "NoteEffectBase.h"
#include "IModulator.h"
#include "IPulseReceiver.h"

class EnvelopeModulator : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public NoteEffectBase, public IModulator, public IPulseReceiver
{
public:
   EnvelopeModulator();
   virtual ~EnvelopeModulator();
   static IDrawableModule* Create() { return new EnvelopeModulator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return true; }
   bool ShouldSuppressAutomaticOutputCable() override { return true; }
   void Delete() { delete this; }
   void DrawModule() override;

   void Start(double time, const ::ADSR& adsr);
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   void CreateUIControls() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool IsResizable() const override { return false; }
   void Resize(float w, float h) override;

   void PlayNote(NoteMessage note) override;
   void OnPulse(double time, float velocity, int flags) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void GetModuleDimensions(float& width, float& height) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

private:
   void OnClicked(float x, float y, bool right) override;

   float mWidth{ 250 };
   float mHeight{ 122 };

   ADSRDisplay* mAdsrDisplay{ nullptr };
   ::ADSR mAdsr{ 10, 100, .5, 100 };

   bool mUseVelocity{ false };
   Checkbox* mUseVelocityCheckbox{ nullptr };
};
