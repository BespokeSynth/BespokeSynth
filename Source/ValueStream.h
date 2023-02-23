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

    ValueStream.h
    Created: 25 Oct 2020 7:09:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Transport.h"
#include "Slider.h"
#include <array>

class PatchCableSource;

class ValueStream : public IDrawableModule, public IAudioPoller, public IFloatSliderListener
{
public:
   ValueStream();
   ~ValueStream();
   static IDrawableModule* Create() { return new ValueStream(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   IUIControl* GetUIControl() const { return mUIControl; }

   void OnTransportAdvanced(float amount) override;

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   IUIControl* mUIControl{ nullptr };
   FloatSlider* mFloatSlider{ nullptr };
   PatchCableSource* mControlCable{ nullptr };
   float mWidth{ 200 };
   float mHeight{ 120 };
   float mSpeed{ 1 };
   FloatSlider* mSpeedSlider{ nullptr };
   std::array<float, 100000> mValues{};
   int mValueDisplayPointer{ 0 };
};
