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
#include "FormantFilterEffect.h"
#include "ButterworthFilterEffect.h"

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
   Register("eq", &(EQEffect::Create));
   //Register("audiounit", &(AudioUnitEffect::Create));
   Register("pitchshift", &(PitchShiftEffect::Create));
   Register("formant", &(FormantFilterEffect::Create));
   Register("butterworth", &(ButterworthFilterEffect::Create));
}

void EffectFactory::Register(string type, CreateEffectFn creator)
{
   mFactoryMap[type] = creator;
}

IAudioEffect* EffectFactory::MakeEffect(string type)
{
   auto iter = mFactoryMap.find(type);
   if (iter != mFactoryMap.end())
      return iter->second();
   return nullptr;
}

vector<string> EffectFactory::GetSpawnableEffects()
{
   vector<string> effects;
   for (auto iter = mFactoryMap.begin(); iter != mFactoryMap.end(); ++iter)
      effects.push_back(iter->first);
   sort(effects.begin(), effects.end());
   return effects;
}

