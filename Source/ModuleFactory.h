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

#pragma once

#include "IDrawableModule.h"

#include "juce_core/juce_core.h"
#include "juce_audio_processors/juce_audio_processors.h"

typedef IDrawableModule* (*CreateModuleFn)(void);
typedef bool (*CanCreateModuleFn)(void);

class ModuleFactory
{
public:
   ModuleFactory();

   enum class SpawnMethod
   {
      Module,
      EffectChain,
      Prefab,
      Plugin,
      MidiController,
      Preset
   };

   struct Spawnable
   {
      SpawnMethod mSpawnMethod{ SpawnMethod::Module };
      std::string mLabel{ "" };
      std::string mDecorator{ "" };
      std::string mPresetModuleType{ "" };
      juce::PluginDescription mPluginDesc{};

      static std::string GetPluginLabel(juce::PluginDescription pluginDesc)
      {
         std::string pluginType = pluginDesc.pluginFormatName.toLowerCase().toStdString();
         if (pluginType == "audiounit")
            pluginType = "au"; //"audiounit" is too long, shorten it
         return pluginType;
      }

      static bool CompareAlphabetical(Spawnable a, Spawnable b)
      {
         juce::String aLower = juce::String(a.mLabel).toLowerCase();
         juce::String bLower = juce::String(b.mLabel).toLowerCase();
         if (aLower == bLower)
            return a.mDecorator < b.mDecorator;
         return aLower < bLower;
      }
   };

   struct ModuleInfo
   {
      CreateModuleFn mCreatorFn{ nullptr };
      CanCreateModuleFn mCanCreateFn{ nullptr };
      ModuleCategory mCategory{ ModuleCategory::kModuleCategory_Unknown };
      bool mIsHidden{ false };
      bool mIsExperimental{ false };
      bool mCanReceiveAudio{ false };
      bool mCanReceiveNotes{ false };
      bool mCanReceivePulses{ false };
   };

   IDrawableModule* MakeModule(std::string type);
   std::vector<Spawnable> GetSpawnableModules(ModuleCategory moduleCategory);
   std::vector<Spawnable> GetSpawnableModules(std::string keys, bool continuousString);
   ModuleCategory GetModuleCategory(std::string typeName);
   ModuleCategory GetModuleCategory(Spawnable spawnable);
   ModuleInfo GetModuleInfo(std::string typeName);
   bool IsExperimental(std::string typeName);
   static void GetPrefabs(std::vector<Spawnable>& prefabs);
   static void GetPresets(std::vector<Spawnable>& presets);
   static std::string FixUpTypeName(std::string typeName);

   static constexpr const char* kPluginSuffix = "[plugin]";
   static constexpr const char* kPrefabSuffix = "[prefab]";
   static constexpr const char* kMidiControllerSuffix = "[midicontroller]";
   static constexpr const char* kEffectChainSuffix = "[effectchain]";

private:
   void Register(std::string type, CreateModuleFn creator, CanCreateModuleFn canCreate, ModuleCategory moduleCategory, bool hidden, bool experimental, bool canReceiveAudio, bool canReceiveNotes, bool canReceivePulses);

   std::map<std::string, ModuleInfo> mFactoryMap;
};
