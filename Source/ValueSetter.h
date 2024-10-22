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
//  ValueSetter.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#pragma once

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Slider.h"

class PatchCableSource;
class IUIControl;

class ValueSetter : public IDrawableModule, public IPulseReceiver, public ITextEntryListener, public IButtonListener, public IFloatSliderListener
{
public:
   ValueSetter();
   virtual ~ValueSetter();
   static IDrawableModule* Create() { return new ValueSetter(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void ButtonClicked(ClickButton* button, double time) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void TextEntryComplete(TextEntry* entry) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void Go(double time);

   PatchCableSource* mControlCable{ nullptr };
   std::array<IUIControl*, IDrawableModule::kMaxOutputsPerPatchCableSource> mTargets{};
   float mValue{ 0 };
   TextEntry* mValueEntry{ nullptr };
   FloatSlider* mValueSlider{ nullptr };
   ClickButton* mButton{ nullptr };
   double mLastClickTime{ 0 };

   float mWidth{ 200 };
   float mHeight{ 20 };
};
