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
//  ClipLauncher.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/17/15.
//
//

#pragma once

#include "IAudioSource.h"
#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "OpenFrameworksPort.h"
#include "JumpBlender.h"

class Sample;
class Looper;

class ClipLauncher : public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public ITimeListener, public IButtonListener
{
public:
   ClipLauncher();
   ~ClipLauncher();
   static IDrawableModule* Create() { return new ClipLauncher(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Init() override;

   int GetRowY(int idx);

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void OnTimeEvent(double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   void RecalcPos(double time, int idx);

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   class SampleData
   {
   public:
      SampleData() {}
      ~SampleData();

      void Init(ClipLauncher* launcher, int index);
      void Draw();

      Sample* mSample{ nullptr };
      int mNumBars{ 1 };
      float mVolume{ 1 };
      Checkbox* mGrabCheckbox{ nullptr };
      Checkbox* mPlayCheckbox{ nullptr };
      ClipLauncher* mClipLauncher{ nullptr };
      int mIndex{ 0 };
      bool mPlay{ false };
      bool mHasSample{ false };
   };

   Looper* mLooper{ nullptr };

   float mVolume{ 1 };
   FloatSlider* mVolumeSlider{ nullptr };

   std::vector<SampleData> mSamples;
   JumpBlender mJumpBlender;
   ofMutex mSampleMutex;
};
