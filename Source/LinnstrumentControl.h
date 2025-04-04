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

    LinnstrumentControl.h
    Created: 28 Oct 2018 2:17:18pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "DropdownList.h"
#include "Scale.h"
#include "ModulationChain.h"
#include "Slider.h"

class IAudioSource;

class LinnstrumentControl : public IDrawableModule, public INoteReceiver, public IDropdownListener, public IScaleListener, public IFloatSliderListener, public MidiDeviceListener
{
public:
   LinnstrumentControl();
   virtual ~LinnstrumentControl();
   static IDrawableModule* Create() { return new LinnstrumentControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Init() override;
   void Poll() override;

   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   enum LinnstrumentColor
   {
      kLinnColor_Off,
      kLinnColor_Red,
      kLinnColor_Yellow,
      kLinnColor_Green,
      kLinnColor_Cyan,
      kLinnColor_Blue,
      kLinnColor_Magenta,
      kLinnColor_Black,
      kLinnColor_White,
      kLinnColor_Orange,
      kLinnColor_Lime,
      kLinnColor_Pink,
      kLinnColor_Invalid
   };

   void SetGridColor(int x, int y, LinnstrumentColor color, bool ignoreRow = false);

   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;

   void OnScaleChanged() override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   void InitController();
   void BuildControllerList();

   void UpdateScaleDisplay();
   void SendScaleInfo();
   LinnstrumentColor GetDesiredGridColor(int x, int y);
   int GridToPitch(int x, int y);
   void SetPitchColor(int pitch, LinnstrumentColor color);
   void SendNRPN(int param, int value);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 190;
      h = 7 + 17 * 4;
   }

   int mControllerIndex{ -1 };
   DropdownList* mControllerList{ nullptr };

   struct NoteAge
   {
      double mTime{ 0 };
      int mColor{ 0 };
      int mVoiceIndex{ -1 };
      int mOutputPitch{ 0 };
      void Update(int pitch, LinnstrumentControl* linnstrument);
   };

   static const int kRows = 8;
   static const int kCols = 25;
   std::array<LinnstrumentColor, kRows * kCols> mGridColorState;
   std::array<NoteAge, 128> mNoteAge;
   float mDecayMs{ 500 };
   FloatSlider* mDecaySlider{ nullptr };
   bool mBlackout{ false };
   Checkbox* mBlackoutCheckbox{ nullptr };
   bool mLightOctaves{ false };
   Checkbox* mLightOctavesCheckbox{ nullptr };
   int mLinnstrumentOctave{ 5 };
   bool mGuitarLines{ false };
   Checkbox* mGuitarLinesCheckbox{ nullptr };
   bool mControlPlayedLights{ true };

   int mLastReceivedNRPNParamMSB{ 0 };
   int mLastReceivedNRPNParamLSB{ 0 };
   int mLastReceivedNRPNValueMSB{ 0 };
   int mLastReceivedNRPNValueLSB{ 0 };

   std::array<ModulationParameters, kNumVoices> mModulators;

   double mRequestedOctaveTime{ 0 };

   MidiDevice mDevice;
};
