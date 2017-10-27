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
   IDrawableModule* MakeModule(string type);
   vector<string> GetSpawnableModules(ModuleType moduleType);
   vector<string> GetSpawnableModules(char c);
   ModuleType GetModuleType(string typeName);
   bool IsExperimental(string typeName);
private:
   void Register(string type, CreateModuleFn creator, CanCreateModuleFn canCreate, ModuleType moduleType, bool hidden, bool experimental);
   map<string, CreateModuleFn> mFactoryMap;
   map<string, CanCreateModuleFn> mCanCreateMap;
   map<string, ModuleType> mModuleTypeMap;
   map<string, bool> mIsHiddenModuleMap;
   map<string, bool> mIsExperimentalModuleMap;
};

#endif /* defined(__modularSynth__ModuleFactory__) */
