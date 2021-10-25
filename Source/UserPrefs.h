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

    UserPrefs.h
    Created: 24 Oct 2021 10:29:53pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <string>
#include <vector>
#include "ofxJSONElement.h"

class IDrawableModule;
class IUIControl;
class DropdownList;
class TextEntry;
class Checkbox;
class FloatSlider;

class UserPref
{
public:
   virtual void Init() = 0;
   virtual IUIControl* GetControl() = 0;
   virtual void SetUpControl(IDrawableModule* owner) = 0;
   virtual void Save(int index) = 0;
};

void RegisterUserPref(UserPref* pref);

class UserPrefString : public UserPref
{
public:
   UserPrefString(std::string name, std::string defaultValue, int charWidth) : mName(name), mValue(defaultValue), mDefault(defaultValue), mCharWidth(charWidth) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   std::string& Get() { return mValue; }
   std::string GetDefault() { return mDefault; }
   void Save(int index) override;
private:
   std::string mName;
   std::string mValue;
   std::string mDefault;
   TextEntry* mTextEntry;
   int mCharWidth;
};

class UserPrefDropdownInt : public UserPref
{
public:
   UserPrefDropdownInt(std::string name, int defaultValue, int width) : mName(name), mValue(defaultValue), mWidth(width) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   DropdownList* GetDropdown() { return mDropdown; }
   int& Get() { return mValue; }
   int& GetIndex() { return mIndex; }
   void Save(int index) override;
private:
   std::string mName;
   int mValue;
   int mIndex;
   DropdownList* mDropdown;
   float mWidth;
};

class UserPrefDropdownString : public UserPref
{
public:
   UserPrefDropdownString(std::string name, std::string defaultValue, int width) : mName(name), mValue(defaultValue), mIndex(-1), mWidth(width) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   DropdownList* GetDropdown() { return mDropdown; }
   std::string& Get() { return mValue; }
   int& GetIndex() { return mIndex; }
   void Save(int index) override;
private:
   std::string mName;
   std::string mValue;
   int mIndex;
   DropdownList* mDropdown;
   float mWidth;
};

class UserPrefTextEntryInt : public UserPref
{
public:
   UserPrefTextEntryInt(std::string name, int defaultValue, int min, int max, int digits) : mName(name), mValue(defaultValue), mMin(min), mMax(max), mDigits(digits) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   int& Get() { return mValue; }
   void Save(int index) override;
private:
   std::string mName;
   int mValue;
   TextEntry* mTextEntry;
   int mMin;
   int mMax;
   int mDigits;
};

class UserPrefTextEntryFloat : public UserPref
{
public:
   UserPrefTextEntryFloat(std::string name, float defaultValue, float min, float max, int digits) : mName(name), mValue(defaultValue), mMin(min), mMax(max), mDigits(digits) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   float& Get() { return mValue; }
   void Save(int index) override;
private:
   std::string mName;
   float mValue;
   TextEntry* mTextEntry;
   float mMin;
   float mMax;
   int mDigits;
};

class UserPrefBool : public UserPref
{
public:
   UserPrefBool(std::string name, bool defaultValue) : mName(name), mValue(defaultValue) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   Checkbox* GetCheckbox() { return mCheckbox; }
   bool& Get() { return mValue; }
   void Save(int index) override;
private:
   std::string mName;
   bool mValue;
   Checkbox* mCheckbox;
};

class UserPrefFloat : public UserPref
{
public:
   UserPrefFloat(std::string name, float defaultValue, float min, float max) : mName(name), mValue(defaultValue), mMin(min), mMax(max) { RegisterUserPref(this); }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   FloatSlider* GetSlider() { return mSlider; }
   float& Get() { return mValue; }
   void Save(int index) override;
private:
   std::string mName;
   float mValue;
   FloatSlider* mSlider;
   float mMin;
   float mMax;
};

namespace
{
#if BESPOKE_MAC
   const char* kDefaultYoutubeDlPath = "/opt/local/bin/youtube-dl";
   const char* kDefaultFfmpegPath = "/opt/local/bin/ffmpeg";
#elif BESPOKE_LINUX
   const char* kDefaultYoutubeDlPath = "/usr/bin/youtube-dl";
   const char* kDefaultFfmpegPath = "/usr/bin/ffmpeg";
#else
   const char* kDefaultYoutubeDlPath = "c:/youtube-dl/bin/youtube-dl.exe";
   const char* kDefaultFfmpegPath = "c:/ffmpeg/bin/ffmpeg.exe";
#endif
}

class UserPrefsHolder
{
public:
   void Init();

   static std::string ToStringLeadingZeroes(int number);

   ofxJSONElement mUserPrefsFile;
   std::vector<UserPref*> mUserPrefs;

   UserPrefDropdownString devicetype{ "devicetype", "auto", 200 };
   UserPrefDropdownString audio_output_device{ "audio_output_device", "auto", 350 };
   UserPrefDropdownString audio_input_device{ "audio_input_device", "none", 350 };
   UserPrefDropdownInt samplerate{ "samplerate", 48000, 100 };
   UserPrefDropdownInt buffersize{ "buffersize", 256, 100 };
   UserPrefTextEntryInt width{ "width", 1700, 100, 10000, 5 };
   UserPrefTextEntryInt height{ "height", 1100, 100, 10000, 5 };
   UserPrefBool set_manual_window_position{ "set_manual_window_position", false };
   UserPrefTextEntryInt position_x{ "position_x", 100, -10000, 10000, 5 };
   UserPrefTextEntryInt position_y{ "position_y", 100, -10000, 10000, 5 };
   UserPrefFloat zoom{ "zoom", 1, .25f, 2 };
   UserPrefFloat ui_scale{ "ui_scale", 1, .25f, 2 };
   UserPrefFloat scroll_multiplier_vertical{ "scroll_multiplier_vertical", 1, -2, 2 };
   UserPrefFloat scroll_multiplier_horizontal{ "scroll_multiplier_horizontal", 1, -2, 2 };
   UserPrefBool autosave{ "autosave", false };
   UserPrefBool show_tooltips_on_load{ "show_tooltips_on_load", true };
   UserPrefBool show_minimap{ "show_minimap", false };
   UserPrefString recordings_path{ "recordings_path", "recordings/", 70 };
   UserPrefTextEntryFloat record_buffer_length_minutes{ "record_buffer_length_minutes", 30, 1, 120, 5 };
   UserPrefString tooltips{ "tooltips", "tooltips_eng.txt", 70 };
   UserPrefString layout{ "layout", "layouts/blank.json", 70 };
   UserPrefString youtube_dl_path{ "youtube_dl_path", kDefaultYoutubeDlPath, 70 };
   UserPrefString ffmpeg_path{ "ffmpeg_path", kDefaultFfmpegPath, 70 };
#if !BESPOKE_LINUX
   UserPrefBool vst_always_on_top{ "vst_always_on_top", true };
#endif
};

extern UserPrefsHolder UserPrefs;
