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
//  TextEntry.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#pragma once

#include <climits>
#include "IUIControl.h"
#include "SynthGlobals.h"

class TextEntry;

class ITextEntryListener
{
public:
   virtual ~ITextEntryListener() {}
   virtual void TextEntryComplete(TextEntry* entry) = 0;
   virtual void TextEntryCancelled(TextEntry* entry) {}
   virtual void TextEntryActivated(TextEntry* entry) {}
};

enum TextEntryType
{
   kTextEntry_Text,
   kTextEntry_Int,
   kTextEntry_Float
};

class TextEntry : public IUIControl, public IKeyboardFocusListener
{
public:
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, std::string* var);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max);
   void OnKeyPressed(int key, bool isRepeat) override;
   void Render() override;
   void Delete() override;

   void MakeActiveTextEntry(bool setCaretToEnd);
   void RemoveSelectedText();
   void SetNextTextEntry(TextEntry* entry);
   void UpdateDisplayString();
   void SetInErrorMode(bool error) { mInErrorMode = error; }
   void DrawLabel(bool draw) { mDrawLabel = draw; }
   void SetRequireEnter(bool require) { mRequireEnterToAccept = require; }
   void SetFlexibleWidth(bool flex) { mFlexibleWidth = flex; }
   void ClearInput();
   const char* GetText() const { return mString; }
   TextEntryType GetTextEntryType() const { return mType; }
   void SetText(std::string text);
   void SelectAll();

   void GetDimensions(float& width, float& height) override;

   //IUIControl
   void SetFromMidiCC(float slider, double time, bool setViaModulator) override;
   float GetValueForMidiCC(float slider) const override;
   float GetMidiValue() const override;
   void GetRange(float& min, float& max) override;
   void SetValue(float value, double time, bool forceUpdate = false) override;
   float GetValue() const override;
   int GetNumValues() override;
   std::string GetDisplayValue(float val) const override;
   void Increment(float amount) override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool IsTextEntry() const override { return true; }
   bool ModulatorUsesLiteralValue() const override { return true; }

protected:
   ~TextEntry(); //protected so that it can't be created on the stack

private:
   void Construct(ITextEntryListener* owner, const char* name, int x, int y, int charWidth); //shared constructor

   void AddCharacter(char c);
   bool AllowCharacter(char c);
   void AcceptEntry(bool pressedEnter) override;
   void CancelEntry() override;
   void MoveCaret(int pos, bool allowSelection = true);
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   int mCharWidth{ 3 };
   ITextEntryListener* mListener{ nullptr };
   char mString[MAX_TEXTENTRY_LENGTH]{};
   char* mVarCString{ nullptr };
   std::string* mVarString{ nullptr };
   int* mVarInt{ nullptr };
   float* mVarFloat{ nullptr };
   int mIntMin{ 0 };
   int mIntMax{ 0 };
   float mFloatMin{ 0 };
   float mFloatMax{ 0 };
   int mCaretPosition{ 0 };
   int mCaretPosition2{ 0 };
   float mCaretBlinkTimer{ 0 };
   bool mCaretBlink{ true };
   TextEntryType mType{ TextEntryType::kTextEntry_Text };
   TextEntry* mNextTextEntry{ nullptr };
   TextEntry* mPreviousTextEntry{ nullptr };
   bool mInErrorMode{ false };
   bool mDrawLabel{ false };
   float mLabelSize{ 0 };
   bool mFlexibleWidth{ false };
   bool mHovered{ false };
   bool mRequireEnterToAccept{ false };
};
