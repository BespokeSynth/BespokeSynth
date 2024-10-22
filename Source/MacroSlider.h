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
//  MacroSlider.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/15.
//
//

#pragma once

#include "IDrawableModule.h"
#include "Slider.h"
#include "IModulator.h"

class PatchCableSource;
class IUIControl;

class MacroSlider : public IDrawableModule, public IFloatSliderListener
{
public:
   MacroSlider();
   virtual ~MacroSlider();
   static IDrawableModule* Create() { return new MacroSlider(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   float GetValue() const { return mValue; }
   FloatSlider* GetSlider() { return mSlider; }
   void SetOutputTarget(int index, IUIControl* target);

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   const static int kMappingSpacing = 32;

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 110;
      height = 25 + (int)mMappings.size() * kMappingSpacing;
   }

   struct Mapping : public IModulator
   {
      Mapping(MacroSlider* owner, int index);
      ~Mapping();
      void CreateUIControls();
      void UpdateControl();
      void Draw();
      PatchCableSource* GetCableSource() const { return mTargetCable; }

      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->IsEnabled(); }

      MacroSlider* mOwner{ nullptr };
      int mIndex{ 0 };
   };

   FloatSlider* mSlider{ nullptr };
   float mValue{ 0 };
   std::vector<Mapping*> mMappings;
};
