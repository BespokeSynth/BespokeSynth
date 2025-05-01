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
//  ModuleSaveData.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/19/14.
//
//

#pragma once

#include "TextEntry.h"

class ofxJSONElement;
class IUIControl;
class IntSlider;
class FloatSlider;
class DropdownList;

typedef void (*FillDropdownFn)(DropdownList*);

class ModuleSaveData
{
public:
   ModuleSaveData();
   ~ModuleSaveData();

   void Save(ofxJSONElement& moduleInfo);

   void SetInt(std::string prop, int val);
   void SetInt(std::string prop, int val, int min, int max, bool isTextField);
   void SetFloat(std::string prop, float val);
   void SetFloat(std::string prop, float val, float min, float max, bool isTextField);
   void SetBool(std::string prop, bool val);
   void SetString(std::string prop, std::string val);
   template <class T>
   void SetEnum(std::string prop, T val) { SetInt(prop, (int)val); }

   void SetExtents(std::string prop, float min, float max);

   bool HasProperty(std::string prop);
   int GetInt(std::string prop);
   float GetFloat(std::string prop);
   bool GetBool(std::string prop);
   std::string GetString(std::string prop);
   template <class T>
   T GetEnum(std::string prop) { return (T)GetInt(prop); }

   int LoadInt(std::string prop, const ofxJSONElement& moduleInfo, int defaultValue = 0, int min = 0, int max = 10, bool isTextField = false);
   int LoadInt(std::string prop, const ofxJSONElement& moduleInfo, int defaultValue, IntSlider* slider, bool isTextField = false);
   float LoadFloat(std::string prop, const ofxJSONElement& moduleInfo, float defaultValue = 0, float min = 0, float max = 1, bool isTextField = false);
   float LoadFloat(std::string prop, const ofxJSONElement& moduleInfo, float defaultValue, FloatSlider* slider, bool isTextField = false);
   bool LoadBool(std::string prop, const ofxJSONElement& moduleInfo, bool defaultValue = false);
   std::string LoadString(std::string prop, const ofxJSONElement& moduleInfo, std::string defaultValue = "", FillDropdownFn fillFn = nullptr);
   template <class T>
   T LoadEnum(std::string prop, const ofxJSONElement& moduleInfo, int defaultValue, IUIControl* list = nullptr, EnumMap* map = nullptr)
   {
      if (list)
         SetEnumMapFromList(prop, list);
      if (map)
         SetEnumMap(prop, map);
      return (T)LoadInt(prop, moduleInfo, defaultValue);
   }

   void UpdatePropertyMax(std::string prop, float max);

   enum Type
   {
      kInt,
      kFloat,
      kBool,
      kString
   };

   struct SaveVal
   {
      explicit SaveVal(std::string prop)
      : mProperty(std::move(prop))
      {}

      std::string mProperty;
      Type mType{ kInt };
      int mInt{ 0 };
      float mFloat{ 0 };
      bool mBool{ false };
      char mString[MAX_TEXTENTRY_LENGTH]{};
      float mMin{ 0 };
      float mMax{ 10 };
      bool mIsTextField{ false };
      EnumMap mEnumValues;
      FillDropdownFn mFillDropdownFn{ nullptr };
   };

   std::list<SaveVal*>& GetSavedValues() { return mValues; }

   //generally these are only used internally, needed to expose them for a special case
   void SetEnumMapFromList(std::string prop, IUIControl* list);
   void SetEnumMap(std::string prop, EnumMap* map);

private:
   SaveVal* GetVal(std::string prop);

   std::list<SaveVal*> mValues;
};
