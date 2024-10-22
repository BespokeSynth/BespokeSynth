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

    GlobalControls.h
    Created: 26 Sep 2020 11:34:18am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "OpenFrameworksPort.h"
#include "Slider.h"

class GlobalControls : public IDrawableModule, public IFloatSliderListener
{
public:
   GlobalControls();
   virtual ~GlobalControls();
   static IDrawableModule* Create() { return new GlobalControls(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }
   std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   FloatSlider* mZoomSlider{ nullptr };
   FloatSlider* mXSlider{ nullptr };
   FloatSlider* mYSlider{ nullptr };
   FloatSlider* mMouseScrollXSlider{ nullptr };
   FloatSlider* mMouseScrollYSlider{ nullptr };
   FloatSlider* mBackgroundLissajousRSlider{ nullptr };
   FloatSlider* mBackgroundLissajousGSlider{ nullptr };
   FloatSlider* mBackgroundLissajousBSlider{ nullptr };
   FloatSlider* mBackgroundRSlider{ nullptr };
   FloatSlider* mBackgroundGSlider{ nullptr };
   FloatSlider* mBackgroundBSlider{ nullptr };
   FloatSlider* mCornerRadiusSlider{ nullptr };
   FloatSlider* mCableAlphaSlider{ nullptr };

   float mWidth{ 200 };
   float mHeight{ 20 };
   float mMouseScrollX{ 0 };
   float mMouseScrollY{ 0 };
};