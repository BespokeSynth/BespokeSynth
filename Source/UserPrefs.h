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

enum class UserPrefCategory
{
   General,
   Graphics,
   Paths
};

class UserPref
{
public:
   virtual void Init() = 0;
   virtual IUIControl* GetControl() = 0;
   virtual void SetUpControl(IDrawableModule* owner) = 0;
   virtual void Save(int index, ofxJSONElement& prefsJson) = 0;
   virtual bool DiffersFromSavedValue() const = 0;
   UserPrefCategory mCategory{ UserPrefCategory::General };
   std::string mName;
};

void RegisterUserPref(UserPref* pref);

class UserPrefString : public UserPref
{
public:
   UserPrefString(std::string name, std::string defaultValue, int charWidth, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mCharWidth(charWidth)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   std::string& Get() { return mValue; }
   std::string GetDefault() { return mDefault; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   std::string mValue;
   std::string mDefault;
   TextEntry* mTextEntry{ nullptr };
   int mCharWidth{ 0 };
};

class UserPrefDropdownInt : public UserPref
{
public:
   UserPrefDropdownInt(std::string name, int defaultValue, int width, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mWidth(width)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   DropdownList* GetDropdown() { return mDropdown; }
   int& Get() { return mValue; }
   int GetDefault() { return mDefault; }
   int& GetIndex() { return mIndex; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   int mValue{ 0 };
   int mDefault{ 0 };
   int mIndex{ -1 };
   DropdownList* mDropdown{ nullptr };
   float mWidth{ 100 };
};

class UserPrefDropdownString : public UserPref
{
public:
   UserPrefDropdownString(std::string name, std::string defaultValue, int width, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mWidth(width)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   DropdownList* GetDropdown() { return mDropdown; }
   std::string& Get() { return mValue; }
   std::string GetDefault() { return mDefault; }
   int& GetIndex() { return mIndex; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   std::string mValue;
   std::string mDefault;
   int mIndex{ -1 };
   DropdownList* mDropdown{ nullptr };
   float mWidth{ 100 };
};

class UserPrefTextEntryInt : public UserPref
{
public:
   UserPrefTextEntryInt(std::string name, int defaultValue, int min, int max, int digits, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mMin(min)
   , mMax(max)
   , mDigits(digits)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   int& Get() { return mValue; }
   int GetDefault() { return mDefault; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   int mValue{ 0 };
   int mDefault{ 0 };
   TextEntry* mTextEntry{ nullptr };
   int mMin{ 0 };
   int mMax{ 1 };
   int mDigits{ 5 };
};

class UserPrefTextEntryFloat : public UserPref
{
public:
   UserPrefTextEntryFloat(std::string name, float defaultValue, float min, float max, int digits, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mMin(min)
   , mMax(max)
   , mDigits(digits)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   TextEntry* GetTextEntry() { return mTextEntry; }
   float& Get() { return mValue; }
   float GetDefault() { return mDefault; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   float mValue{ 0 };
   float mDefault{ 0 };
   TextEntry* mTextEntry{ nullptr };
   float mMin{ 0 };
   float mMax{ 1 };
   int mDigits{ 5 };
};

class UserPrefBool : public UserPref
{
public:
   UserPrefBool(std::string name, bool defaultValue, UserPrefCategory category)
   : mValue(defaultValue)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   Checkbox* GetCheckbox() { return mCheckbox; }
   bool& Get() { return mValue; }
   bool GetDefault() { return mDefault; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   bool mValue{ 0 };
   bool mDefault{ 0 };
   Checkbox* mCheckbox{ nullptr };
};

class UserPrefFloat : public UserPref
{
public:
   UserPrefFloat(std::string name, float defaultValue, float min, float max, UserPrefCategory category)
   : mValue(defaultValue)
   , mDefault(defaultValue)
   , mMin(min)
   , mMax(max)
   {
      RegisterUserPref(this);
      mName = name;
      mCategory = category;
   }
   void Init() override;
   void SetUpControl(IDrawableModule* owner) override;
   IUIControl* GetControl() override;
   FloatSlider* GetSlider() { return mSlider; }
   float& Get() { return mValue; }
   float GetDefault() { return mDefault; }
   void Save(int index, ofxJSONElement& prefsJson) override;
   bool DiffersFromSavedValue() const override;

private:
   float mValue{ 0 };
   float mDefault{ 0 };
   FloatSlider* mSlider{ nullptr };
   float mMin{ 0 };
   float mMax{ 1 };
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

   float LastTargetFramerate;
   ofxJSONElement mUserPrefsFile;
   std::vector<UserPref*> mUserPrefs;
   UserPrefDropdownString devicetype{ "devicetype", "auto", 200, UserPrefCategory::General };
   UserPrefDropdownString audio_output_device{ "audio_output_device", "auto", 350, UserPrefCategory::General };
   UserPrefDropdownString audio_input_device{ "audio_input_device", "none", 350, UserPrefCategory::General };
   UserPrefDropdownInt samplerate{ "samplerate", 48000, 100, UserPrefCategory::General };
   UserPrefDropdownInt buffersize{ "buffersize", 256, 100, UserPrefCategory::General };
   UserPrefDropdownInt oversampling{ "oversampling", 1, 100, UserPrefCategory::General };
   UserPrefTextEntryInt width{ "width", 1700, 100, 10000, 5, UserPrefCategory::General };
   UserPrefTextEntryInt height{ "height", 1100, 100, 10000, 5, UserPrefCategory::General };
   UserPrefBool set_manual_window_position{ "set_manual_window_position", false, UserPrefCategory::General };
   UserPrefTextEntryInt position_x{ "position_x", 200, -10000, 10000, 5, UserPrefCategory::General };
   UserPrefTextEntryInt position_y{ "position_y", 200, -10000, 10000, 5, UserPrefCategory::General };
   UserPrefFloat zoom{ "zoom", 1.3f, .25f, 2, UserPrefCategory::General };
   UserPrefFloat ui_scale{ "ui_scale", 1.3f, .25f, 2, UserPrefCategory::General };
   UserPrefDropdownString cable_drop_behavior{ "cable_drop_behavior", "show quickspawn", 150, UserPrefCategory::General };
   UserPrefDropdownString qwerty_to_pitch_mode{ "qwerty_to_pitch_mode", "Ableton", 150, UserPrefCategory::General };
   UserPrefFloat grid_snap_size{ "grid_snap_size", 30, 5, 150, UserPrefCategory::General };
   UserPrefFloat scroll_multiplier_vertical{ "scroll_multiplier_vertical", 1, -2, 2, UserPrefCategory::General };
   UserPrefFloat scroll_multiplier_horizontal{ "scroll_multiplier_horizontal", 1, -2, 2, UserPrefCategory::General };
   UserPrefBool wrap_mouse_on_pan{ "wrap_mouse_on_pan", true, UserPrefCategory::General };
   UserPrefBool autosave{ "autosave", false, UserPrefCategory::General };
   UserPrefBool show_tooltips_on_load{ "show_tooltips_on_load", true, UserPrefCategory::General };
   UserPrefBool show_minimap{ "show_minimap", false, UserPrefCategory::General };
   UserPrefFloat minimap_margin{ "minimap_margin", 10, 0, 50, UserPrefCategory::General };
   UserPrefDropdownString minimap_corner{ "minimap_corner", "Top right", 150, UserPrefCategory::General };
   UserPrefBool immediate_paste{ "immediate_paste", false, UserPrefCategory::General };
   UserPrefTextEntryFloat record_buffer_length_minutes{ "record_buffer_length_minutes", 30, 1, 120, 5, UserPrefCategory::General };
#if !BESPOKE_LINUX
   UserPrefBool vst_always_on_top{ "vst_always_on_top", true, UserPrefCategory::General };
#endif
   UserPrefTextEntryInt max_output_channels{ "max_output_channels", 16, 1, 1024, 5, UserPrefCategory::General };
   UserPrefTextEntryInt max_input_channels{ "max_input_channels", 16, 1, 1024, 5, UserPrefCategory::General };
   UserPrefString plugin_preference_order{ "plugin_preference_order", "VST3;VST;AudioUnit;LV2", 70, UserPrefCategory::General };

   UserPrefBool draw_background_lissajous{ "draw_background_lissajous", true, UserPrefCategory::Graphics };
   UserPrefBool background_lissajous_autocorrelate{ "background_lissajous_autocorrelate", true, UserPrefCategory::Graphics };
   UserPrefFloat cable_alpha{ "cable_alpha", 1, 0.05f, 1, UserPrefCategory::Graphics };
   UserPrefBool fade_cable_middle{ "fade_cable_middle", true, UserPrefCategory::Graphics };
   UserPrefFloat cable_quality{ "cable_quality", 1, .1f, 3, UserPrefCategory::Graphics };
   UserPrefFloat lissajous_r{ "lissajous_r", 0.408f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat lissajous_g{ "lissajous_g", 0.245f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat lissajous_b{ "lissajous_b", 0.418f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat background_r{ "background_r", 0.09f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat background_g{ "background_g", 0.09f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat background_b{ "background_b", 0.09f, 0, 1, UserPrefCategory::Graphics };
   UserPrefFloat target_framerate{ "target_framerate", 60, 30, 144, UserPrefCategory::Graphics };
   UserPrefFloat motion_trails{ "motion_trails", 1, 0, 2, UserPrefCategory::Graphics };
   UserPrefBool draw_module_highlights{ "draw_module_highlights", true, UserPrefCategory::Graphics };
   UserPrefTextEntryFloat mouse_offset_x{ "mouse_offset_x", 0, -100, 100, 5, UserPrefCategory::Graphics };
   UserPrefTextEntryFloat mouse_offset_y
   {
      "mouse_offset_y",
#if BESPOKE_MAC
      -4,
#else
      0,
#endif
      -100, 100, 5, UserPrefCategory::Graphics
   };

   UserPrefString recordings_path{ "recordings_path", "recordings/", 70, UserPrefCategory::Paths };
   UserPrefString samples_path{ "samples_path", "samples/", 70, UserPrefCategory::Paths };
   UserPrefString tooltips{ "tooltips", "tooltips_eng.txt", 70, UserPrefCategory::Paths };
   UserPrefString layout{ "layout", "layouts/blank.json", 70, UserPrefCategory::Paths };
   UserPrefString youtube_dl_path{ "youtube_dl_path", kDefaultYoutubeDlPath, 70, UserPrefCategory::Paths };
   UserPrefString ffmpeg_path{ "ffmpeg_path", kDefaultFfmpegPath, 70, UserPrefCategory::Paths };
};

extern UserPrefsHolder UserPrefs;
