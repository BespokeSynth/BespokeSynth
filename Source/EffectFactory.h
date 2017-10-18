//
//  EffectFactory.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/1/14.
//
//

#ifndef __Bespoke__EffectFactory__
#define __Bespoke__EffectFactory__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IAudioEffect.h"

typedef IAudioEffect* (*CreateEffectFn)(void);

class EffectFactory
{
public:
   EffectFactory();
   IAudioEffect* MakeEffect(string type);
   vector<string> GetSpawnableEffects();
private:
   void Register(string type, CreateEffectFn creator);
   map<string, CreateEffectFn> mFactoryMap;
};

#endif /* defined(__Bespoke__EffectFactory__) */
