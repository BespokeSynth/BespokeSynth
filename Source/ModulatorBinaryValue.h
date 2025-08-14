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

    ModulatorBinaryValue.h
    Created: 13 Aug 2025 8:00:00am
    Author:  Andrius Merkys

  ==============================================================================
*/

#pragma once
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "Slider.h"
#include "IModulator.h"

enum BinaryValueCode
{
   kCodeByte,
   kCodeGray
};

class PatchCableSource;

class ModulatorBinaryValue : public IDrawableModule, public IDropdownListener, public IFloatSliderListener
{
public:
   ModulatorBinaryValue();
   virtual ~ModulatorBinaryValue();
   static IDrawableModule* Create() { return new ModulatorBinaryValue(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {};
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 140;
      h = 17 * 2 + 4 + 20;
   }
   int GetBitValue(int);

   float mInput{ 0 };
   int mCode{ kCodeByte };

   struct BitModulator : public IModulator
   {
      BitModulator(ModulatorBinaryValue* owner, int index)
      : mOwner(owner)
      , mIndex(index)
      {
      }
      void UpdateControl() { OnModulatorRepatch(); }
      void SetCableSource(PatchCableSource* cableSource) { mTargetCableSource = cableSource; }
      PatchCableSource* GetCableSource() const { return mTargetCableSource; }

      //IModulator
      virtual float Value(int samplesIn = 0) override;
      virtual bool Active() const override { return mOwner->IsEnabled(); }

      ModulatorBinaryValue* mOwner{ nullptr };
      int mIndex{ 0 };
   };

   BitModulator mBit0;
   BitModulator mBit1;
   BitModulator mBit2;
   BitModulator mBit3;
   BitModulator mBit4;
   BitModulator mBit5;
   BitModulator mBit6;
   BitModulator mBit7;

   std::array<BitModulator, 8> mBits{ mBit7, mBit6, mBit5, mBit4, mBit3, mBit2, mBit1, mBit0 };

   FloatSlider* mInputSlider{ nullptr };
   DropdownList* mCodeSelector{ nullptr };
};
