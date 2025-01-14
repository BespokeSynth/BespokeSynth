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

    NotePanAlternator.h
    Created: 25 Mar 2018 9:27:25pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Slider.h"

class NotePanAlternator : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   NotePanAlternator();
   static IDrawableModule* Create() { return new NotePanAlternator(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

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

   bool mFlip{ false };
   float mPanOne{ -1 };
   FloatSlider* mPanOneSlider{ nullptr };
   float mPanTwo{ 1 };
   FloatSlider* mPanTwoSlider{ nullptr };
};
