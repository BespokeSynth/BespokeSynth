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

#ifndef __modularSynth__ModuleSaveData__
#define __modularSynth__ModuleSaveData__

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
   
   void SetInt(string prop, int val);
   void SetInt(string prop, int val, int min, int max, bool isTextField);
   void SetFloat(string prop, float val);
   void SetFloat(string prop, float val, float min, float max, bool isTextField);
   void SetBool(string prop, bool val);
   void SetString(string prop, string val);
   
   void SetExtents(string prop, float min, float max);
   
   bool HasProperty(string prop);
   int GetInt(string prop);
   float GetFloat(string prop);
   bool GetBool(string prop);
   string GetString(string prop);
   template <class T> T GetEnum(string prop) { return (T)GetInt(prop); }
   
   int LoadInt(string prop, const ofxJSONElement& moduleInfo, int defaultValue = 0, int min=0, int max=10, bool isTextField = false);
   int LoadInt(string prop, const ofxJSONElement& moduleInfo, int defaultValue, IntSlider* slider, bool isTextField = false);
   float LoadFloat(string prop, const ofxJSONElement& moduleInfo, float defaultValue = 0, float min=0, float max=1, bool isTextField = false);
   float LoadFloat(string prop, const ofxJSONElement& moduleInfo, float defaultValue , FloatSlider* slider, bool isTextField = false);
   bool LoadBool(string prop, const ofxJSONElement& moduleInfo, bool defaultValue = false);
   string LoadString(string prop, const ofxJSONElement& moduleInfo, string defaultValue = "", FillDropdownFn fillFn = nullptr);
   template <class T> T LoadEnum(string prop, const ofxJSONElement& moduleInfo, int defaultValue, IUIControl* list = nullptr, EnumMap* map = nullptr)
   {
      if (list)
         SetEnumMapFromList(prop, list);
      if (map)
         SetEnumMap(prop, map);
      return (T)LoadInt(prop, moduleInfo, defaultValue);
   }
   
   void UpdatePropertyMax(string prop, float max);
   
   enum Type
   {
      kInt,
      kFloat,
      kBool,
      kString
   };
   
   struct SaveVal
   {
      explicit SaveVal(string prop) : mProperty(std::move(prop)), mString(), mMin(0), mMax(10), mIsTextField(false), mFillDropdownFn(nullptr) {}
      
      string mProperty;
      Type mType;
      int mInt;
      float mFloat;
      bool mBool;
      char mString[MAX_TEXTENTRY_LENGTH];
      float mMin;
      float mMax;
      bool mIsTextField;
      EnumMap mEnumValues;
      FillDropdownFn mFillDropdownFn;
   };
   
   list<SaveVal*>& GetSavedValues() { return mValues; }

   //generally these are only used internally, needed to expose them for a special case
   void SetEnumMapFromList(string prop, IUIControl* list);
   void SetEnumMap(string prop, EnumMap* map);
   
private:
   SaveVal* GetVal(string prop);
   
   list<SaveVal*> mValues;
};

#endif /* defined(__modularSynth__ModuleSaveData__) */
