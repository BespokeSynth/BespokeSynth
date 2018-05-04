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
      SaveVal(string prop) : mProperty(prop), mMin(0), mMax(10), mIsTextField(false), mFillDropdownFn(nullptr) { bzero(mString,MAX_TEXTENTRY_LENGTH); }
      
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
