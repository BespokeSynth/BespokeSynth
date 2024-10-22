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
//  EffectFactory.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/1/14.
//
//

#include "EffectFactory.h"

#include "BitcrushEffect.h"
#include "DelayEffect.h"
#include "BiquadFilterEffect.h"
#include "DistortionEffect.h"
//#include "Stutter.h"
#include "TremoloEffect.h"
#include "Compressor.h"
#include "NoiseEffect.h"
#include "GateEffect.h"
//#include "ReverbEffect.h"
#include "Muter.h"
#include "Pumper.h"
#include "LiveGranulator.h"
#include "DCRemoverEffect.h"
#include "FreeverbEffect.h"
#include "EQEffect.h"
//#include "AudioUnitEffect.h"
#include "PitchShiftEffect.h"
#include "ButterworthFilterEffect.h"
#include "GainStageEffect.h"

EffectFactory::EffectFactory()
{
   Register("bitcrush", &(BitcrushEffect::Create));
   Register("delay", &(DelayEffect::Create));
   Register("biquad", &(BiquadFilterEffect::Create));
   Register("distortion", &(DistortionEffect::Create));
   //Register("stutter", &(Stutter::Create));   stutter is now a standalone module as StutterControl
   Register("tremolo", &(TremoloEffect::Create));
   Register("compressor", &(Compressor::Create));
   Register("noisify", &(NoiseEffect::Create));
   Register("gate", &(GateEffect::Create));
   //Register("reverb", &(ReverbEffect::Create));
   Register("muter", &(Muter::Create));
   Register("pumper", &(Pumper::Create));
   Register("granulator", &(LiveGranulator::Create));
   Register("dcremover", &(DCRemoverEffect::Create));
   Register("freeverb", &(FreeverbEffect::Create));
   Register("basiceq", &(EQEffect::Create));
   //Register("audiounit", &(AudioUnitEffect::Create));
   Register("pitchshift", &(PitchShiftEffect::Create));
   //Register("formant", &(FormantFilterEffect::Create));
   Register("butterworth", &(ButterworthFilterEffect::Create));
   Register("gainstage", &(GainStageEffect::Create));
}

void EffectFactory::Register(std::string type, CreateEffectFn creator)
{
   mFactoryMap[type] = creator;
}

IAudioEffect* EffectFactory::MakeEffect(std::string type)
{
   if (type == "eq") //fix up old save data
      type = "basiceq";

   auto iter = mFactoryMap.find(type);
   if (iter != mFactoryMap.end())
      return iter->second();
   return nullptr;
}

std::vector<std::string> EffectFactory::GetSpawnableEffects()
{
   std::vector<std::string> effects;
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
      effects.push_back(iter->first);
   sort(effects.begin(), effects.end());
   return effects;
}
