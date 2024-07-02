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
//  ModulationChain.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/26/15.
//
//

#pragma once

#include "Ramp.h"
#include "LFO.h"

class ModulationChain
{
public:
   ModulationChain(float initialValue);
   float GetValue(int samplesIn) const;
   float GetIndividualValue(int samplesIn) const;
   void SetValue(float value);
   void RampValue(double time, float from, float to, double length);
   void SetLFO(NoteInterval interval, float amount);
   void AppendTo(ModulationChain* chain);
   void SetSidechain(ModulationChain* chain);
   void MultiplyIn(ModulationChain* chain);
   void CreateBuffer();
   void FillBuffer(float* buffer);
   float GetBufferValue(int sampleIdx);

private:
   Ramp mRamp;
   LFO mLFO;
   float mLFOAmount{ 0 };
   float* mBuffer{ nullptr };
   ModulationChain* mPrev{ nullptr };
   ModulationChain* mSidechain{ nullptr };
   ModulationChain* mMultiplyIn{ nullptr };
};

struct ModulationParameters
{
   ModulationParameters() {}
   ModulationParameters(ModulationChain* _pitchBend,
                        ModulationChain* _modWheel,
                        ModulationChain* _pressure,
                        float _pan)
   : pitchBend(_pitchBend)
   , modWheel(_modWheel)
   , pressure(_pressure)
   , pan(_pan)
   {}
   ModulationChain* pitchBend{ nullptr };
   ModulationChain* modWheel{ nullptr };
   ModulationChain* pressure{ nullptr };
   float pan{ 0 };

   static constexpr float kDefaultPitchBend{ 0 };
   static constexpr float kDefaultModWheel{ 0 };
   static constexpr float kDefaultPressure{ .5f };
};

struct ModulationCollection
{
   ModulationChain mPitchBend{ ModulationParameters::kDefaultPitchBend };
   ModulationChain mModWheel{ ModulationParameters::kDefaultModWheel };
   ModulationChain mPressure{ ModulationParameters::kDefaultPressure };
};

class Modulations
{
public:
   Modulations(bool isGlobalEffect); //isGlobalEffect: is the effect that we're using this on a global effect that affects all voices (pitch bend all voices that come through here the same way) or a voice effect (affect individual voices that come through here individually)?
   ModulationChain* GetPitchBend(int voiceIdx);
   ModulationChain* GetModWheel(int voiceIdx);
   ModulationChain* GetPressure(int voiceIdx);

private:
   ModulationCollection mGlobalModulation;
   std::vector<ModulationCollection> mVoiceModulations;
};
