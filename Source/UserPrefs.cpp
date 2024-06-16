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
/*
  ==============================================================================

    UserPrefs.cpp
    Created: 24 Oct 2021 10:29:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "UserPrefs.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "DropdownList.h"
#include "TextEntry.h"
#include "Slider.h"
#include "Checkbox.h"

UserPrefsHolder UserPrefs;

void RegisterUserPref(UserPref* pref)
{
   UserPrefs.mUserPrefs.push_back(pref);
}

// Helper function to convert string to appropriate type and set the JSON value
void setValueFromString(Json::Value& element, const std::string& value)
{
   switch (element.type())
   {
      case Json::nullValue:
         ofLog() << "Cannot update null value.";
         break;
      case Json::intValue:
         element = std::stoi(value);
         break;
      case Json::uintValue:
         element = static_cast<unsigned int>(std::stoul(value));
         break;
      case Json::realValue:
         element = std::stod(value);
         break;
      case Json::stringValue:
         element = value;
         break;
      case Json::booleanValue:
         if (value == "true" || value == "1")
         {
            element = true;
         }
         else if (value == "false" || value == "0")
         {
            element = false;
         }
         else
         {
            ofLog() << "Invalid boolean value.";
         }
         break;
      case Json::arrayValue:
      case Json::objectValue:
         ofLog() << "Cannot update array or object value with a single command-line parameter.";
         break;
      default:
         ofLog() << "Unsupported JSON value type.";
         break;
   }
}

void UserPrefsHolder::Init()
{
   mUserPrefsFile.open(TheSynth->GetUserPrefsPath());

   // Look for overrides in command line options
   for (int i = 0; i < juce::JUCEApplication::getCommandLineParameterArray().size(); ++i)
   {
      juce::String argument = juce::JUCEApplication::getCommandLineParameterArray()[i];
      if (argument == "-o" || argument == "--option")
      {
         juce::String argJsonOption = juce::JUCEApplication::getCommandLineParameterArray()[i + 1];
         auto value = juce::JUCEApplication::getCommandLineParameterArray()[i + 2].toStdString();
         auto& element = mUserPrefsFile[argJsonOption.toStdString()];
         try
         {
            setValueFromString(element, value);
         }
         catch (std::invalid_argument)
         {
            ofLog() << "Failed to convert argument " << value << " to the desired type";
         }
      }
   }

   for (auto* pref : mUserPrefs)
      pref->Init();
}

std::string UserPrefsHolder::ToStringLeadingZeroes(int number)
{
   char buffer[9];
   snprintf(buffer, sizeof(buffer), "%08d", number);
   return buffer;
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefString::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asString();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}
void UserPrefString::SetUpControl(IDrawableModule* owner)
{
   mTextEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(owner), mName.c_str(), -1, -1, mCharWidth, &mValue);
}

IUIControl* UserPrefString::GetControl()
{
   return mTextEntry;
}

void UserPrefString::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mValue;
}

bool UserPrefString::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mValue != mDefault;
   return mValue != UserPrefs.mUserPrefsFile[mName].asString();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefDropdownInt::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asInt();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}

void UserPrefDropdownInt::SetUpControl(IDrawableModule* owner)
{
   mDropdown = new DropdownList(dynamic_cast<IDropdownListener*>(owner), mName.c_str(), -1, -1, &mIndex, mWidth);
}

IUIControl* UserPrefDropdownInt::GetControl()
{
   return mDropdown;
}

void UserPrefDropdownInt::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = ofToInt(mDropdown->GetLabel(mIndex));
}

bool UserPrefDropdownInt::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return ofToInt(mDropdown->GetLabel(mIndex)) != mDefault;
   return ofToInt(mDropdown->GetLabel(mIndex)) != UserPrefs.mUserPrefsFile[mName].asInt();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefDropdownString::Init()
{
   if (!UserPrefs.mUserPrefsFile[mName].isNull())
      mValue = UserPrefs.mUserPrefsFile[mName].asString();
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asString();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}

void UserPrefDropdownString::SetUpControl(IDrawableModule* owner)
{
   mDropdown = new DropdownList(dynamic_cast<IDropdownListener*>(owner), mName.c_str(), -1, -1, &mIndex, mWidth);
}

IUIControl* UserPrefDropdownString::GetControl()
{
   return mDropdown;
}

void UserPrefDropdownString::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mDropdown->GetLabel(mIndex);
}

bool UserPrefDropdownString::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mDropdown->GetLabel(mIndex) != mDefault;
   return mDropdown->GetLabel(mIndex) != UserPrefs.mUserPrefsFile[mName].asString();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefTextEntryInt::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asInt();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}

void UserPrefTextEntryInt::SetUpControl(IDrawableModule* owner)
{
   mTextEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(owner), mName.c_str(), -1, -1, mDigits, &mValue, mMin, mMax);
}

IUIControl* UserPrefTextEntryInt::GetControl()
{
   return mTextEntry;
}

void UserPrefTextEntryInt::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mValue;
}

bool UserPrefTextEntryInt::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mValue != mDefault;
   return mValue != UserPrefs.mUserPrefsFile[mName].asInt();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefTextEntryFloat::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asFloat();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}
void UserPrefTextEntryFloat::SetUpControl(IDrawableModule* owner)
{
   mTextEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(owner), mName.c_str(), -1, -1, mDigits, &mValue, mMin, mMax);
}

IUIControl* UserPrefTextEntryFloat::GetControl()
{
   return mTextEntry;
}

void UserPrefTextEntryFloat::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mValue;
}

bool UserPrefTextEntryFloat::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mValue != mDefault;
   return mValue != UserPrefs.mUserPrefsFile[mName].asFloat();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefBool::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asBool();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}

void UserPrefBool::SetUpControl(IDrawableModule* owner)
{
   mCheckbox = new Checkbox(owner, mName.c_str(), -1, -1, &mValue);
}

IUIControl* UserPrefBool::GetControl()
{
   return mCheckbox;
}

void UserPrefBool::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mValue;
}

bool UserPrefBool::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mValue != mDefault;
   return mValue != UserPrefs.mUserPrefsFile[mName].asBool();
}

////////////////////////////////////////////////////////////////////////////////

void UserPrefFloat::Init()
{
   try
   {
      if (!UserPrefs.mUserPrefsFile[mName].isNull())
         mValue = UserPrefs.mUserPrefsFile[mName].asFloat();
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent("json error loading userpref for " + mName + ": " + e.what(), kLogEventType_Error);
      UserPrefs.mUserPrefsFile[mName] = mDefault;
   }
}

void UserPrefFloat::SetUpControl(IDrawableModule* owner)
{
   mSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(owner), mName.c_str(), -1, -1, 200, 15, &mValue, mMin, mMax);
   mSlider->SetShowName(false);
}

IUIControl* UserPrefFloat::GetControl()
{
   return mSlider;
}

void UserPrefFloat::Save(int index, ofxJSONElement& prefsJson) //this numbering is a silly markup hack to get the json file to save ordered
{
   prefsJson.removeMember(mName);
   prefsJson["**" + UserPrefsHolder::ToStringLeadingZeroes(index) + "**" + mName] = mValue;
}

bool UserPrefFloat::DiffersFromSavedValue() const
{
   if (UserPrefs.mUserPrefsFile[mName].isNull())
      return mValue != mDefault;
   return mValue != UserPrefs.mUserPrefsFile[mName].asFloat();
}
