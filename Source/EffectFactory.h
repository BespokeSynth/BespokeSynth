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
//  EffectFactory.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/1/14.
//
//

#pragma once

#include "IAudioEffect.h"

typedef IAudioEffect* (*CreateEffectFn)(void);

class EffectFactory
{
public:
   EffectFactory();
   IAudioEffect* MakeEffect(std::string type);
   std::vector<std::string> GetSpawnableEffects();

private:
   void Register(std::string type, CreateEffectFn creator);
   std::map<std::string, CreateEffectFn> mFactoryMap;
};
