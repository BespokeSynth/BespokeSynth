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
//  TextEntry.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#include "TextEntry.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "FileStream.h"
#include "UserPrefs.h"

#include "juce_gui_basics/juce_gui_basics.h"

IKeyboardFocusListener* IKeyboardFocusListener::sCurrentKeyboardFocus = nullptr;
IKeyboardFocusListener* IKeyboardFocusListener::sKeyboardFocusBeforeClick = nullptr;

//static
void IKeyboardFocusListener::ClearActiveKeyboardFocus(bool notifyListeners)
{
   if (sCurrentKeyboardFocus && notifyListeners)
      sCurrentKeyboardFocus->AcceptEntry(false);
   sCurrentKeyboardFocus = nullptr;
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var)
: mVarCString(var)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, std::string* var)
: mVarString(var)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max)
: mVarInt(var)
, mType(kTextEntry_Int)
, mIntMin(min)
, mIntMax(max)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max)
: mVarFloat(var)
, mType(kTextEntry_Float)
, mFloatMin(min)
, mFloatMax(max)
{
   Construct(owner, name, x, y, charWidth);
}

void TextEntry::Construct(ITextEntryListener* owner, const char* name, int x, int y, int charWidth)
{
   mCharWidth = charWidth;
   mListener = owner;

   UpdateDisplayString();

   SetName(name);
   mLabelSize = gFont.GetStringWidth(name, 13) + 3;
   SetPosition(x, y);
   assert(owner);
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(owner);
   if (module)
      module->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

TextEntry::~TextEntry()
{
}

void TextEntry::Delete()
{
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
      IKeyboardFocusListener::ClearActiveKeyboardFocus(false);
   delete this;
}

void TextEntry::Render()
{
   ofPushStyle();

   ofSetLineWidth(.5f);

   float xOffset = 0;
   if (mDrawLabel)
   {
      DrawTextNormal(Name(), mX, mY + 12);
      xOffset = mLabelSize;
   }

   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;

   ofColor color(255, 255, 255);
   if (!isCurrent && mInErrorMode)
      color.set(200, 100, 100);

   if (!isCurrent)
      UpdateDisplayString();

   float w, h;
   GetDimensions(w, h);
   if (isCurrent)
   {
      ofSetColor(color, gModuleDrawAlpha * .1f);
      ofFill();
      ofRect(mX + xOffset, mY, w - xOffset, h);
   }
   ofSetColor(color, gModuleDrawAlpha);
   ofNoFill();
   ofRect(mX + xOffset, mY, w - xOffset, h);

   gFontFixedWidth.DrawString(mString, 12, mX + 2 + xOffset, mY + 12);

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
   {
      if (mCaretBlink)
      {
         int caretX = mX + 2 + xOffset;
         int caretY = mY + 1;
         if (mCaretPosition > 0)
         {
            char beforeCaret[MAX_TEXTENTRY_LENGTH];
            strncpy(beforeCaret, mString, mCaretPosition);
            beforeCaret[mCaretPosition] = 0;
            caretX += gFontFixedWidth.GetStringWidth(beforeCaret, 12);
         }
         ofFill();
         ofRect(caretX, caretY, 1, 12, L(corner, 1));
      }
      mCaretBlinkTimer += ofGetLastFrameTime();
      if (mCaretBlinkTimer > .3f)
      {
         mCaretBlinkTimer -= .3f;
         mCaretBlink = !mCaretBlink;
      }
   }

   if (mCaretPosition != mCaretPosition2 && isCurrent)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 255, 255, 90);

      int selStartX = mX + 2 + xOffset;
      int selEndX = mX + 2 + xOffset;
      int selY = mY + 1;

      //
      int start = MIN(mCaretPosition, mCaretPosition2);
      char selectionTmp[MAX_TEXTENTRY_LENGTH];
      strncpy(selectionTmp, mString, start);
      selectionTmp[start] = 0;
      selStartX += gFontFixedWidth.GetStringWidth(selectionTmp, 12);

      //
      int end = MAX(mCaretPosition, mCaretPosition2);
      strncpy(selectionTmp, mString, end);
      selectionTmp[end] = 0;
      selEndX += gFontFixedWidth.GetStringWidth(selectionTmp, 12);

      ofRect(selStartX, selY, selEndX - selStartX, 12, 0);

      ofPopStyle();
   }

   /*if (mHovered)
   {
      ofSetColor(100, 100, 100, .8f*gModuleDrawAlpha);
      ofFill();
      ofRect(mX+xOffset,mY-12,GetStringWidth(Name()),10);
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      DrawTextNormal(Name(), mX+xOffset, mY);
   }*/

   ofPopStyle();

   DrawHover(mX + xOffset, mY, w - xOffset, h);
}

void TextEntry::GetDimensions(float& width, float& height)
{
   if (mFlexibleWidth)
      width = MAX(30.0f, gFontFixedWidth.GetStringWidth(mString, 12) + 4);
   else
      width = mCharWidth * 9;

   if (mDrawLabel)
      width += mLabelSize;

   height = 15;
}

void TextEntry::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   float xOffset = 2;
   if (mDrawLabel)
      xOffset += mLabelSize;

   if (sKeyboardFocusBeforeClick != this)
   {
      SelectAll();
   }
   else
   {
      mCaretPosition = 0;

      char caretCheck[MAX_TEXTENTRY_LENGTH];
      size_t checkLength = strnlen(mString, MAX_TEXTENTRY_LENGTH);
      strncpy(caretCheck, mString, checkLength);
      int lastSubstrWidth = gFontFixedWidth.GetStringWidth(caretCheck, 12);
      for (int i = (int)checkLength - 1; i >= 0; --i)
      {
         caretCheck[i] = 0; //shorten string by one

         int substrWidth = gFontFixedWidth.GetStringWidth(caretCheck, 12);
         //ofLog() << x << " " << i << " " << (xOffset + substrWidth);
         if (x > xOffset + ((substrWidth + lastSubstrWidth) * .5f))
         {
            mCaretPosition = i + 1;
            break;
         }

         lastSubstrWidth = substrWidth;
      }

      mCaretPosition2 = mCaretPosition;
   }

   MakeActiveTextEntry(false);
}

void TextEntry::MakeActiveTextEntry(bool setCaretToEnd)
{
   SetActiveKeyboardFocus(this);
   if (mListener)
      mListener->TextEntryActivated(this);
   if (setCaretToEnd)
   {
      mCaretPosition = (int)strlen(mString);
      mCaretPosition2 = mCaretPosition;
   }
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void TextEntry::RemoveSelectedText()
{
   int caretStart = MAX(0, MIN(mCaretPosition, mCaretPosition2));
   int caretEnd = MIN((int)strlen(mString), MAX(mCaretPosition, mCaretPosition2));
   std::string newString = mString;
   strcpy(mString, (newString.substr(0, caretStart) + newString.substr(caretEnd)).c_str());
   MoveCaret(caretStart, false);
}

void TextEntry::SelectAll()
{
   mCaretPosition = 0;
   mCaretPosition2 = (int)strnlen(mString, MAX_TEXTENTRY_LENGTH);
}

void TextEntry::OnKeyPressed(int key, bool isRepeat)
{
   if (key == OF_KEY_RETURN)
   {
      AcceptEntry(true);
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
   }
   else if (key == OF_KEY_TAB)
   {
      TextEntry* pendingNewEntry = nullptr;
      if (GetKeyModifiers() == kModifier_Shift)
         pendingNewEntry = mPreviousTextEntry;
      else
         pendingNewEntry = mNextTextEntry;

      AcceptEntry(false);
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));

      if (pendingNewEntry)
      {
         pendingNewEntry->MakeActiveTextEntry(true);
         pendingNewEntry->SelectAll();
      }
   }
   else if (key == juce::KeyPress::backspaceKey)
   {
      int len = (int)strlen(mString);
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else if (mCaretPosition > 0)
      {
         for (int i = mCaretPosition - 1; i < len; ++i)
            mString[i] = mString[i + 1];
         --mCaretPosition;
         --mCaretPosition2;
      }
   }
   else if (key == juce::KeyPress::deleteKey)
   {
      int len = (int)strlen(mString);
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else
      {
         for (int i = mCaretPosition; i < len; ++i)
            mString[i] = mString[i + 1];
      }
   }
   else if (key == OF_KEY_ESC || key == '`')
   {
      IKeyboardFocusListener::ClearActiveKeyboardFocus(K(notifyListeners));
      mCaretPosition2 = mCaretPosition;
   }
   else if (key == OF_KEY_LEFT)
   {
      if (GetKeyModifiers() & kModifier_Command)
         MoveCaret(0);
      else if (!(GetKeyModifiers() & kModifier_Shift) && mCaretPosition != mCaretPosition2)
         MoveCaret(MIN(mCaretPosition, mCaretPosition2));
      else if (mCaretPosition > 0)
         MoveCaret(mCaretPosition - 1);
   }
   else if (key == OF_KEY_RIGHT)
   {
      if (GetKeyModifiers() & kModifier_Command)
         MoveCaret((int)strlen(mString));
      else if (!(GetKeyModifiers() & kModifier_Shift) && mCaretPosition != mCaretPosition2)
         MoveCaret(MAX(mCaretPosition, mCaretPosition2));
      else if (mCaretPosition < (int)strlen(mString))
         MoveCaret(mCaretPosition + 1);
   }
   else if (key == OF_KEY_UP)
   {
      if (mType == kTextEntry_Float)
      {
         if (*mVarFloat + 1 <= mFloatMax)
         {
            *mVarFloat += 1;
            UpdateDisplayString();
            AcceptEntry(false);
         }
      }
      else if (mType == kTextEntry_Int)
      {
         if (*mVarInt + 1 <= mIntMax)
         {
            *mVarInt += 1;
            UpdateDisplayString();
            AcceptEntry(false);
         }
      }
   }
   else if (key == OF_KEY_DOWN)
   {
      if (mType == kTextEntry_Float)
      {
         if (*mVarFloat - 1 >= mFloatMin)
         {
            *mVarFloat -= 1;
            UpdateDisplayString();
            AcceptEntry(false);
         }
      }
      else if (mType == kTextEntry_Int)
      {
         if (*mVarInt - 1 >= mIntMin)
         {
            *mVarInt -= 1;
            UpdateDisplayString();
            AcceptEntry(false);
         }
      }
   }
   else if (toupper(key) == 'V' && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      juce::String clipboard = TheSynth->GetTextFromClipboard();

      std::string newString = mString;
      strcpy(mString, (newString.substr(0, mCaretPosition) + clipboard.toStdString() + newString.substr(mCaretPosition)).c_str());
      if (UserPrefs.immediate_paste.Get())
         AcceptEntry(true);
      else
         MoveCaret(mCaretPosition + clipboard.length());
   }
   else if ((toupper(key) == 'C' || toupper(key) == 'X') && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         int caretStart = MIN(mCaretPosition, mCaretPosition2);
         int caretEnd = MAX(mCaretPosition, mCaretPosition2);
         std::string tmpString(mString);
         TheSynth->CopyTextToClipboard(tmpString.substr(caretStart, caretEnd - caretStart));

         if (toupper(key) == 'X')
            RemoveSelectedText();
      }
   }
   else if (toupper(key) == 'A' && GetKeyModifiers() == kModifier_Command)
   {
      SelectAll();
   }
   else if (key == juce::KeyPress::homeKey)
   {
      MoveCaret(0);
   }
   else if (key == juce::KeyPress::endKey)
   {
      MoveCaret((int)strlen(mString));
   }
   else if (key < CHAR_MAX && juce::CharacterFunctions::isPrintable((char)key))
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      AddCharacter((char)key);
   }
}

void TextEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      int len = (int)strlen(mString);
      if (/*len < mCharWidth && */ len < MAX_TEXTENTRY_LENGTH - 1)
      {
         for (int i = len; i > mCaretPosition; --i)
            mString[i] = mString[i - 1];
         mString[mCaretPosition] = c;
         mString[len + 1] = '\0';
         ++mCaretPosition;
         ++mCaretPosition2;
      }
   }
}

void TextEntry::UpdateDisplayString()
{
   if (mVarCString)
      StringCopy(mString, mVarCString, MAX_TEXTENTRY_LENGTH);
   if (mVarString)
      StringCopy(mString, mVarString->c_str(), MAX_TEXTENTRY_LENGTH);
   if (mVarInt)
      StringCopy(mString, ofToString(*mVarInt).c_str(), MAX_TEXTENTRY_LENGTH);
   if (mVarFloat)
      StringCopy(mString, ofToString(*mVarFloat).c_str(), MAX_TEXTENTRY_LENGTH);
}

void TextEntry::ClearInput()
{
   std::memset(mString, 0, MAX_TEXTENTRY_LENGTH);
   mCaretPosition = 0;
   mCaretPosition2 = 0;
}

void TextEntry::SetText(std::string text)
{
   if (mVarCString)
      StringCopy(mVarCString, text.c_str(), MAX_TEXTENTRY_LENGTH);
   if (mVarString)
      *mVarString = text;
   mCaretPosition = 0;
   mCaretPosition2 = 0;
}

void TextEntry::SetFromMidiCC(float slider, double time, bool setViaModulator)
{
   if (mType == kTextEntry_Int)
   {
      *mVarInt = GetValueForMidiCC(slider);
      UpdateDisplayString();
   }

   if (mType == kTextEntry_Float)
   {
      *mVarFloat = GetValueForMidiCC(slider);
      UpdateDisplayString();
   }
}

float TextEntry::GetValueForMidiCC(float slider) const
{
   if (mType == kTextEntry_Int)
   {
      slider = ofClamp(slider, 0, 1);
      return (int)round(ofMap(slider, 0, 1, mIntMin, mIntMax));
   }

   if (mType == kTextEntry_Float)
   {
      slider = ofClamp(slider, 0, 1);
      return ofMap(slider, 0, 1, mFloatMin, mFloatMax);
   }

   return 0;
}

float TextEntry::GetMidiValue() const
{
   if (mType == kTextEntry_Int)
      return ofMap(*mVarInt, mIntMin, mIntMax, 0, 1);

   if (mType == kTextEntry_Float)
      return ofMap(*mVarFloat, mFloatMin, mFloatMax, 0, 1);

   return 0;
}

void TextEntry::GetRange(float& min, float& max)
{
   if (mType == kTextEntry_Int)
   {
      min = mIntMin;
      max = mIntMax;
   }
   else if (mType == kTextEntry_Float)
   {
      min = mFloatMin;
      max = mFloatMax;
   }
   else
   {
      min = 0;
      max = 1;
   }
}

void TextEntry::SetValue(float value, double time, bool forceUpdate /*= false*/)
{
   if (mType == kTextEntry_Int)
   {
      *mVarInt = std::round(value);
      UpdateDisplayString();
   }

   if (mType == kTextEntry_Float)
   {
      *mVarFloat = value;
      UpdateDisplayString();
   }
}

float TextEntry::GetValue() const
{
   if (mType == kTextEntry_Int)
      return *mVarInt;

   if (mType == kTextEntry_Float)
      return *mVarFloat;

   return 0;
}

int TextEntry::GetNumValues()
{
   if (mType == kTextEntry_Int)
      return mIntMax - mIntMin + 1;
   return 0;
}

std::string TextEntry::GetDisplayValue(float val) const
{
   if (mType == kTextEntry_Int || mType == kTextEntry_Float)
      return ofToString(val);
   return mString;
}

void TextEntry::AcceptEntry(bool pressedEnter)
{
   if (!pressedEnter && mRequireEnterToAccept)
   {
      CancelEntry();
      return;
   }

   if (mVarCString)
      StringCopy(mVarCString, mString, MAX_TEXTENTRY_LENGTH);
   if (mVarString)
      *mVarString = mString;
   if (mVarInt && mString[0] != 0)
   {
      *mVarInt = ofClamp(ofToInt(mString), mIntMin, mIntMax);
      StringCopy(mString, ofToString(*mVarInt).c_str(), MAX_TEXTENTRY_LENGTH);
   }
   if (mVarFloat && mString[0] != 0)
   {
      *mVarFloat = ofClamp(ofToFloat(mString), mFloatMin, mFloatMax);
      StringCopy(mString, ofToString(*mVarFloat).c_str(), MAX_TEXTENTRY_LENGTH);
   }

   if (mListener)
      mListener->TextEntryComplete(this);
}

void TextEntry::CancelEntry()
{
   if (mListener)
      mListener->TextEntryCancelled(this);
}

void TextEntry::MoveCaret(int pos, bool allowSelection)
{
   mCaretPosition = pos;
   if (!allowSelection || !(GetKeyModifiers() & kModifier_Shift))
      mCaretPosition2 = mCaretPosition;
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

bool TextEntry::AllowCharacter(char c)
{
   if (mType == kTextEntry_Text)
      return juce::CharacterFunctions::isPrintable(c);
   if (mType == kTextEntry_Int)
      return juce::CharacterFunctions::isDigit((char)c) || c == '-';
   if (mType == kTextEntry_Float)
      return juce::CharacterFunctions::isDigit((char)c) || c == '.' || c == '-';
   return false;
}

void TextEntry::Increment(float amount)
{
   if (mType == kTextEntry_Float)
   {
      float newVal = *mVarFloat + amount;
      if (newVal >= mFloatMin && newVal <= mFloatMax)
      {
         *mVarFloat = newVal;
         UpdateDisplayString();
         AcceptEntry(false);
      }
   }
   else if (mType == kTextEntry_Int)
   {
      int newVal = *mVarInt + (int)amount;
      if (newVal >= mIntMin && newVal <= mIntMax)
      {
         *mVarInt = newVal;
         UpdateDisplayString();
         AcceptEntry(false);
      }
   }
}

void TextEntry::SetNextTextEntry(TextEntry* entry)
{
   mNextTextEntry = entry;
   if (entry)
      entry->mPreviousTextEntry = this;
}

bool TextEntry::MouseMoved(float x, float y)
{
   mHovered = TestHover(x, y);
   CheckHover(x, y);
   return false;
}

namespace
{
   const int kSaveStateRev = 0;
}

void TextEntry::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << std::string(mString);
}

void TextEntry::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   std::string var;
   in >> var;
   if (shouldSetValue)
   {
      StringCopy(mString, var.c_str(), MAX_TEXTENTRY_LENGTH);
      AcceptEntry(false);
   }
}