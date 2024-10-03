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

    VolcaBeatsControl.h
    Created: 28 Jan 2017 10:48:13pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Slider.h"

class VolcaBeatsControl : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   VolcaBeatsControl();
   virtual ~VolcaBeatsControl();
   static IDrawableModule* Create() { return new VolcaBeatsControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void FloatSliderUpdated(FloatSlider* slider, double oldVal, double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 263;
      height = 170;
   }

   double mClapSpeed{ .5 };
   double mClaveSpeed{ .5 };
   double mAgogoSpeed{ .5 };
   double mCrashSpeed{ .5 };
   double mStutterTime{ .5 };
   double mStutterDepth{ 0 };
   double mTomDecay{ .5 };
   double mClosedHatDecay{ .5 };
   double mOpenHatDecay{ .5 };
   double mHatGrain{ .5 };

   FloatSlider* mClapSpeedSlider{ nullptr };
   FloatSlider* mClaveSpeedSlider{ nullptr };
   FloatSlider* mAgogoSpeedSlider{ nullptr };
   FloatSlider* mCrashSpeedSlider{ nullptr };
   FloatSlider* mStutterTimeSlider{ nullptr };
   FloatSlider* mStutterDepthSlider{ nullptr };
   FloatSlider* mTomDecaySlider{ nullptr };
   FloatSlider* mClosedHatDecaySlider{ nullptr };
   FloatSlider* mOpenHatDecaySlider{ nullptr };
   FloatSlider* mHatGrainSlider{ nullptr };

   double mLevels[10]{};
   FloatSlider* mLevelSliders[10]{ nullptr };
};
