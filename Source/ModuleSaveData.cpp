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
//  ModuleSaveData.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/19/14.
//
//

#include "ModuleSaveData.h"
#include "ModularSynth.h"
#include "ofxJSONElement.h"
#include "SynthGlobals.h"
#include "DropdownList.h"
#include "RadioButton.h"
#include "Slider.h"

ModuleSaveData::ModuleSaveData()
{

}

ModuleSaveData::~ModuleSaveData()
{
   for (auto iter = mValues.begin(); iter != mValues.end(); ++iter)
      delete *iter;
}

ModuleSaveData::SaveVal* ModuleSaveData::GetVal(std::string prop)
{
   for (auto iter = mValues.begin(); iter != mValues.end(); ++iter)
   {
      if ((*iter)->mProperty == prop)
         return *iter;
   }
   
   //didn't find it, add
   ModuleSaveData::SaveVal* newVal = new ModuleSaveData::SaveVal(prop);
   mValues.push_back(newVal);
   return newVal;
}

void ModuleSaveData::Save(ofxJSONElement& moduleInfo)
{
   for (auto i=mValues.begin(); i!=mValues.end(); ++i)
   {
      const SaveVal* save = *i;
      assert(save);
      switch (save->mType)
      {
         case kInt:
            moduleInfo[save->mProperty] = save->mInt;
            break;
         case kFloat:
            moduleInfo[save->mProperty] = save->mFloat;
            break;
         case kBool:
            moduleInfo[save->mProperty] = save->mBool;
            break;
         case kString:
            moduleInfo[save->mProperty] = save->mString;
            break;
      }
   }
}

void ModuleSaveData::SetInt(std::string prop, int val)
{
   SaveVal* save = GetVal(prop);
   assert(save && save->mType == kInt);
   save->mInt = val;
}

void ModuleSaveData::SetInt(std::string prop, int val, int min, int max, bool isTextField)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mType = kInt;
   save->mInt = val;
   save->mMin = min;
   save->mMax = max;
   save->mIsTextField = isTextField;
}

void ModuleSaveData::SetFloat(std::string prop, float val)
{
   SaveVal* save = GetVal(prop);
   assert(save && save->mType == kFloat);
   save->mFloat = val;
}

void ModuleSaveData::SetFloat(std::string prop, float val, float min, float max, bool isTextField)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mType = kFloat;
   save->mFloat = val;
   save->mMin = min;
   save->mMax = max;
   save->mIsTextField = isTextField;
}

void ModuleSaveData::SetBool(std::string prop, bool val)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mType = kBool;
   save->mBool = val;
}

void ModuleSaveData::SetString(std::string prop, std::string val)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mType = kString;
   StringCopy(save->mString, val.c_str(), MAX_TEXTENTRY_LENGTH);
}

void ModuleSaveData::SetExtents(std::string prop, float min, float max)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mMin = min;
   save->mMax = max;
}

bool ModuleSaveData::HasProperty(std::string prop)
{
   for (auto i=mValues.begin(); i!=mValues.end(); ++i)
   {
      if ((*i)->mProperty == prop)
         return true;
   }
   return false;
}

int ModuleSaveData::GetInt(std::string prop)
{
   const SaveVal* save = GetVal(prop);
   assert(save);
   assert(save->mType == kInt);
   return save->mInt;
}

float ModuleSaveData::GetFloat(std::string prop)
{
   const SaveVal* save = GetVal(prop);
   assert(save);
   assert(save->mType == kFloat);
   return save->mFloat;
}

bool ModuleSaveData::GetBool(std::string prop)
{
   const SaveVal* save = GetVal(prop);
   assert(save);
   assert(save->mType == kBool);
   return save->mBool;
}

std::string ModuleSaveData::GetString(std::string prop)
{
   const SaveVal* save = GetVal(prop);
   assert(save);
   assert(save->mType == kString);
   return save->mString;
}

int ModuleSaveData::LoadInt(std::string prop, const ofxJSONElement& moduleInfo, int defaultValue, int min, int max, bool isTextField)
{
   int val = defaultValue;
   try
   {
      if (!moduleInfo[prop].isNull())
         val = moduleInfo[prop].asInt();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
   SetInt(prop, val, min, max, isTextField);
   return val;
}

int ModuleSaveData::LoadInt(std::string prop, const ofxJSONElement& moduleInfo, int defaultValue, IntSlider* slider, bool isTextField)
{
   int min=0;
   int max=10;
   if (slider)
      slider->GetRange(min, max);
   return LoadInt(prop, moduleInfo, defaultValue, min, max, isTextField);
}

float ModuleSaveData::LoadFloat(std::string prop, const ofxJSONElement& moduleInfo, float defaultValue, float min, float max, bool isTextField)
{
   float val = defaultValue;
   try
   {
      if (!moduleInfo[prop].isNull())
         val = moduleInfo[prop].asDouble();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
   SetFloat(prop, val, min, max, isTextField);
   return val;
}

float ModuleSaveData::LoadFloat(std::string prop, const ofxJSONElement& moduleInfo, float defaultValue, FloatSlider* slider, bool isTextField)
{
   float min=0;
   float max=1;
   if (slider)
      slider->GetRange(min, max);
   return LoadFloat(prop, moduleInfo, defaultValue, min, max, isTextField);
}

bool ModuleSaveData::LoadBool(std::string prop, const ofxJSONElement& moduleInfo, bool defaultValue)
{
   bool val = defaultValue;
   try
   {
      if (!moduleInfo[prop].isNull())
         val = moduleInfo[prop].asBool();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
   SetBool(prop, val);
   return val;
}

std::string ModuleSaveData::LoadString(std::string prop, const ofxJSONElement& moduleInfo, std::string defaultValue, FillDropdownFn fillFn)
{
   std::string val = defaultValue;
   try
   {
      if (!moduleInfo[prop].isNull())
         val = moduleInfo[prop].asString();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }
   SetString(prop, val);
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mFillDropdownFn = fillFn;
   return val;
}

void ModuleSaveData::UpdatePropertyMax(std::string prop, float max)
{
   if (HasProperty(prop))
   {
      SaveVal* save = GetVal(prop);
      assert(save);
      save->mMax = max;
   }
}

void ModuleSaveData::SetEnumMapFromList(std::string prop, IUIControl* list)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   DropdownList* dropdownList = dynamic_cast<DropdownList*>(list);
   RadioButton* radioButton = dynamic_cast<RadioButton*>(list);
   if (dropdownList)
      save->mEnumValues = dropdownList->GetEnumMap();
   if (radioButton)
      save->mEnumValues = radioButton->GetEnumMap();
}

void ModuleSaveData::SetEnumMap(std::string prop, EnumMap* map)
{
   SaveVal* save = GetVal(prop);
   assert(save);
   save->mEnumValues = *map;
}
