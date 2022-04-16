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
//  ModuleFactory.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#ifndef __modularSynth__ModuleFactory__
#define __modularSynth__ModuleFactory__

#include <iostream>
#include "IDrawableModule.h"

typedef IDrawableModule* (*CreateModuleFn)(void);
typedef bool (*CanCreateModuleFn)(void);

class ModuleFactory
{
public:
   ModuleFactory();
   IDrawableModule* MakeModule(std::string type);
   std::vector<std::string> GetSpawnableModules(ModuleType moduleType);
   std::vector<std::string> GetSpawnableModules(std::string keys);
   ModuleType GetModuleType(std::string typeName);
   bool IsExperimental(std::string typeName);
   static void GetPrefabs(std::vector<std::string>& prefabs);
   static std::string FixUpTypeName(std::string typeName);

   static constexpr const char* kVSTSuffix = "[vst]";
   static constexpr const char* kPrefabSuffix = "[prefab]";
   static constexpr const char* kMidiControllerSuffix = "[midicontroller]";
   static constexpr const char* kEffectChainSuffix = "[effectchain]";

private:
   void Register(std::string type, CreateModuleFn creator, CanCreateModuleFn canCreate, ModuleType moduleType, bool hidden, bool experimental);
   std::map<std::string, CreateModuleFn> mFactoryMap;
   std::map<std::string, CanCreateModuleFn> mCanCreateMap;
   std::map<std::string, ModuleType> mModuleTypeMap;
   std::map<std::string, bool> mIsHiddenModuleMap;
   std::map<std::string, bool> mIsExperimentalModuleMap;
};

#endif /* defined(__modularSynth__ModuleFactory__) */
