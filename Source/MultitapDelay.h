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

    MultitapDelay.h
    Created: 25 Nov 2018 11:16:38am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioProcessor.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "INoteReceiver.h"
#include "Granulator.h"
#include "ADSR.h"

class Sample;

class MultitapDelay : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public INoteReceiver
{
public:
   MultitapDelay();
   ~MultitapDelay();
   static IDrawableModule* Create() { return new MultitapDelay(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   //IFloatSliderListener
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   //IDropdownListener
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   //IButtonListener
   void ButtonClicked(ClickButton* button, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   struct DelayTap
   {
      DelayTap();
      void Process(float* sampleOut, int offset, int ch);
      void Draw(float w, float h);

      float mDelayMs{ 100 };
      float mGain{ 0 };
      float mFeedback{ 0 };
      float mPan{ 0 };

      MultitapDelay* mOwner{ nullptr };

      FloatSlider* mDelayMsSlider{ nullptr };
      FloatSlider* mGainSlider{ nullptr };
      FloatSlider* mFeedbackSlider{ nullptr };
      FloatSlider* mPanSlider{ nullptr };

      ChannelBuffer mTapBuffer;
   };

   struct DelayMPETap
   {
      void Process(float* sampleOut, int offset, int ch);
      void Draw(float w, float h);

      float mPitch{ 0 };
      ModulationChain* mPitchBend{ nullptr };
      ModulationChain* mPressure{ nullptr };
      ModulationChain* mModWheel{ nullptr };

      ::ADSR mADSR{ 100, 0, 1, 100 };

      MultitapDelay* mOwner{ nullptr };
   };

   int mNumTaps{ 4 };
   std::vector<DelayTap> mTaps;
   static const int kNumMPETaps = 16;
   DelayMPETap mMPETaps[kNumMPETaps];

   ChannelBuffer mWriteBuffer;
   FloatSlider* mDryAmountSlider{ nullptr };
   float mDryAmount{ 1 };
   FloatSlider* mDisplayLengthSlider{ nullptr };
   float mDisplayLength{ 10 };
   RollingBuffer mDelayBuffer;
};
