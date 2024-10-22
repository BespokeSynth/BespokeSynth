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
//  IMidiVoice.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/20/12.
//
//

#pragma once

#include "ModulationChain.h"
#include "Profiler.h"

class IVoiceParams;

class IMidiVoice
{
public:
   IMidiVoice() {}
   virtual ~IMidiVoice() {}
   virtual void ClearVoice() = 0;
   void SetPitch(float pitch) { mPitch = ofClamp(pitch, 0, 127); }
   void SetModulators(ModulationParameters modulators) { mModulators = modulators; }
   virtual void Start(double time, float amount) = 0;
   virtual void Stop(double time) = 0;
   virtual bool Process(double time, ChannelBuffer* out, int oversampling) = 0;
   virtual bool IsDone(double time) = 0;
   virtual void SetVoiceParams(IVoiceParams* params) = 0;
   void SetPan(float pan)
   {
      assert(pan >= -1 && pan <= 1);
      mPan = pan;
   }
   float GetPan() const
   {
      assert(mPan >= -1 && mPan <= 1);
      return mPan;
   }

   float GetPitch(int samplesIn) { return mPitch + (mModulators.pitchBend ? mModulators.pitchBend->GetValue(samplesIn) : ModulationParameters::kDefaultPitchBend); }
   float GetModWheel(int samplesIn) { return mModulators.modWheel ? mModulators.modWheel->GetValue(samplesIn) : ModulationParameters::kDefaultModWheel; }
   float GetPressure(int samplesIn) { return mModulators.pressure ? mModulators.pressure->GetValue(samplesIn) : ModulationParameters::kDefaultPressure; }

private:
   float mPitch{ 0 };
   float mPan{ 0 };
   ModulationParameters mModulators;
};
