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

    NoteHumanizer.h
    Created: 2 Nov 2016 7:56:20pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifndef NOTEHUMANIZER_H_INCLUDED
#define NOTEHUMANIZER_H_INCLUDED


#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "INoteSource.h"
#include "Slider.h"
#include "Transport.h"

class NoteHumanizer : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   NoteHumanizer();
   ~NoteHumanizer();
   static IDrawableModule* Create() { return new NoteHumanizer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

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
      height = 40;
   }

   float mTime{ 33 };
   FloatSlider* mTimeSlider{ nullptr };
   float mVelocity{ .1 };
   FloatSlider* mVelocitySlider{ nullptr };

   std::array<float, 128> mLastDelayMs{};
};


#endif // NOTEHUMANIZER_H_INCLUDED
