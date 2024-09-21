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
//  VocoderCarrierInput.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/18/13.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"

class VocoderBase
{
public:
   virtual ~VocoderBase() {}
   virtual void SetCarrierBuffer(float* buffer, int bufferSize) = 0;
};

class VocoderCarrierInput : public IAudioProcessor, public IDrawableModule
{
public:
   VocoderCarrierInput();
   virtual ~VocoderCarrierInput();
   static IDrawableModule* Create() { return new VocoderCarrierInput(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& w, float& h) override
   {
      w = 60;
      h = 0;
   }

   VocoderBase* mVocoder{ nullptr };
   IAudioReceiver* mVocoderTarget{ nullptr };
};
