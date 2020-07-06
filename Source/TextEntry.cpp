
//
//  TextEntry.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#include "TextEntry.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "FileStream.h"

IKeyboardFocusListener* IKeyboardFocusListener::sCurrentKeyboardFocus = nullptr;

//static
void IKeyboardFocusListener::ClearActiveKeyboardFocus(bool acceptEntry)
{
   if (sCurrentKeyboardFocus && acceptEntry)
      sCurrentKeyboardFocus->AcceptEntry(false);
   sCurrentKeyboardFocus = nullptr;
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var)
: mVarString(var)
, mVarInt(nullptr)
, mVarFloat(nullptr)
, mType(kTextEntry_Text)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max)
: mVarString(nullptr)
, mVarInt(var)
, mVarFloat(nullptr)
, mType(kTextEntry_Int)
, mIntMin(min)
, mIntMax(max)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max)
: mVarString(nullptr)
, mVarInt(nullptr)
, mVarFloat(var)
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
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
   mNextTextEntry = nullptr;
   mPreviousTextEntry = nullptr;
   mInErrorMode = false;
   mDrawLabel = false;
   mFlexibleWidth = false;
   mHovered = false;
   mRequireEnterToAccept = false;
   
   UpdateDisplayString();
   
   SetName(name);
   mLabelSize = GetStringWidth(name) + 3;
   SetPosition(x,y);
   assert(owner);
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(owner);
   if (module)
      module->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

TextEntry::~TextEntry()
{
}

void TextEntry::Render()
{
   ofPushStyle();
   
   ofSetLineWidth(.5f);
   
   float xOffset = 0;
   if (mDrawLabel)
   {
      DrawTextNormal(Name(), mX, mY+12);
      xOffset = mLabelSize;
   }
   
   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;
   
   ofColor color(255,255,255);
   if (!isCurrent && mInErrorMode)
      color.set(200,100,100);

   if (!isCurrent)
      UpdateDisplayString();
   
   float w,h;
   GetDimensions(w,h);
   if (isCurrent)
   {
      ofSetColor(color,gModuleDrawAlpha * .1f);
      ofFill();
      ofRect(mX + xOffset,mY,w - xOffset,h);
   }
   ofSetColor(color,gModuleDrawAlpha);
   ofNoFill();
   ofRect(mX + xOffset,mY,w - xOffset,h);
   DrawTextNormal(mString, mX+2+xOffset, mY+12);
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
   {
      if (mCaretBlink)
      {
         int caretX = mX+2+xOffset-1;
         int caretY = mY+1;
         if (mCaretPosition > 0)
         {
            char beforeCaret[MAX_TEXTENTRY_LENGTH];
            strncpy(beforeCaret, mString, mCaretPosition);
            beforeCaret[mCaretPosition] = 0;
            caretX += GetStringWidth(beforeCaret);
         }
         ofFill();
         ofRect(caretX,caretY,1,12);
      }
      mCaretBlinkTimer += ofGetLastFrameTime();
      if (mCaretBlinkTimer > .3f)
      {
         mCaretBlinkTimer -= .3f;
         mCaretBlink = !mCaretBlink;
      }
   }
   
   /*if (mHovered)
   {
      ofSetColor(100, 100, 100, .8f*gModuleDrawAlpha);
      ofFill();
      ofRect(mX+xOffset,mY-12,GetStringWidth(Name()),12);
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      DrawTextNormal(Name(), mX+xOffset, mY);
   }*/

   ofPopStyle();
}

void TextEntry::GetDimensions(float& width, float& height)
{
   if (mFlexibleWidth)
      width = MAX(30,GetStringWidth(mString) + 4);
   else
      width = mCharWidth * 9;
   
   if (mDrawLabel)
      width += mLabelSize;
   
   height = 15;
}

void TextEntry::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   float xOffset = 2;
   if (mDrawLabel)
      xOffset += mLabelSize;
   
   mCaretPosition = 0;
   
   char caretCheck[MAX_TEXTENTRY_LENGTH];
   size_t checkLength = strnlen(mString, MAX_TEXTENTRY_LENGTH);
   strncpy(caretCheck, mString, checkLength);
   int lastSubstrWidth = GetStringWidth(caretCheck);
   for (int i=(int)checkLength-1; i >= 0; --i)
   {
      caretCheck[i] = 0;   //shorten string by one
      
      int substrWidth = GetStringWidth(caretCheck);
      //ofLog() << x << " " << i << " " << (xOffset + substrWidth);
      if (x > xOffset + ((substrWidth + lastSubstrWidth) * .5f))
      {
         mCaretPosition = i + 1;
         break;
      }
      
      lastSubstrWidth = substrWidth;
   }
   
   MakeActiveTextEntry(false);
}

void TextEntry::MakeActiveTextEntry(bool setCaretToEnd)
{
   SetActiveKeyboardFocus(this);
   if (mListener)
      mListener->TextEntryActivated(this);
   if (setCaretToEnd)
      mCaretPosition = (int)strlen(mString);
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void TextEntry::OnKeyPressed(int key, bool isRepeat)
{
   if (key == OF_KEY_RETURN)
   {
      if (mRequireEnterToAccept)
         AcceptEntry(true);
      IKeyboardFocusListener::ClearActiveKeyboardFocus(K(acceptEntry));
   }
   if (key == OF_KEY_TAB)
   {
      AcceptEntry(false);
      if (GetKeyModifiers() == kModifier_Shift)
      {
         if (mPreviousTextEntry)
            mPreviousTextEntry->MakeActiveTextEntry(true);
      }
      else
      {
         if (mNextTextEntry)
            mNextTextEntry->MakeActiveTextEntry(true);
      }
   }
   else if (key == OF_KEY_BACKSPACE)
   {
      int len = (int)strlen(mString);
      if (mCaretPosition > 0)
      {
         for (int i=mCaretPosition-1; i<len; ++i)
            mString[i] = mString[i+1];
         --mCaretPosition;
      }
   }
   else if (key == OF_KEY_ESC)
   {
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(acceptEntry));
   }
   else if (key == OF_KEY_LEFT)
   {
      if (mCaretPosition > 0)
         --mCaretPosition;
   }
   else if (key == OF_KEY_RIGHT)
   {
      if (mCaretPosition < strlen(mString))
         ++mCaretPosition;
   }
   else if (key == 'V' && GetKeyModifiers() == kModifier_Command)
   {
      juce::String clipboard = SystemClipboard::getTextFromClipboard();
      for (int i=0; i<clipboard.length(); ++i)
         AddCharacter(clipboard[i]);
   }
   else
   {
      AddCharacter((char)key);
   }
}

void TextEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      int len = (int)strlen(mString);
      if (/*len < mCharWidth && */len < MAX_TEXTENTRY_LENGTH-1)
      {
         for (int i=len; i>mCaretPosition; --i)
            mString[i] = mString[i-1];
         mString[mCaretPosition] = c;
         ++mCaretPosition;
      }
   }
}

void TextEntry::UpdateDisplayString()
{
   if (mVarString)
      StringCopy(mString, mVarString, MAX_TEXTENTRY_LENGTH);
   if (mVarInt)
      StringCopy(mString, ofToString(*mVarInt).c_str(), MAX_TEXTENTRY_LENGTH);
   if (mVarFloat)
      StringCopy(mString, ofToString(*mVarFloat).c_str(), MAX_TEXTENTRY_LENGTH);
}

void TextEntry::AcceptEntry(bool pressedEnter)
{
   if (!pressedEnter && mRequireEnterToAccept)
      return;
   
   if (mVarString)
      StringCopy(mVarString, mString, MAX_TEXTENTRY_LENGTH);
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

bool TextEntry::AllowCharacter(char c)
{
   if (mType == kTextEntry_Text)
      return c >= ' ' && c <= '~';  //these encompass the ASCII range of printable characters
   if (mType == kTextEntry_Int)
      return CharacterFunctions::isDigit((char)c) || c == '-';
   if (mType == kTextEntry_Float)
      return CharacterFunctions::isDigit((char)c) || c == '.' || c == '-';
   return false;
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
   return false;
}

namespace
{
   const int kSaveStateRev = 0;
}

void TextEntry::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << string(mString);
}

void TextEntry::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   string var;
   in >> var;
   StringCopy(mString, var.c_str(), MAX_TEXTENTRY_LENGTH);
   AcceptEntry(false);
}
