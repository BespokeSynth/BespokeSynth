
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

TextEntry* TextEntry::sCurrentTextEntry = NULL;

//static
void TextEntry::ClearActiveTextEntry(bool acceptEntry)
{
   if (sCurrentTextEntry && acceptEntry)
      sCurrentTextEntry->AcceptEntry();
   sCurrentTextEntry = NULL;
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var)
: mVarString(var)
, mVarInt(NULL)
, mVarFloat(NULL)
, mType(kTextEntry_Text)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max)
: mVarString(NULL)
, mVarInt(var)
, mVarFloat(NULL)
, mType(kTextEntry_Int)
, mIntMin(min)
, mIntMax(max)
{
   Construct(owner, name, x, y, charWidth);
}

TextEntry::TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max)
: mVarString(NULL)
, mVarInt(NULL)
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
   mNextTextEntry = NULL;
   mPreviousTextEntry = NULL;
   mInErrorMode = false;
   mFlexibleWidth = false;
   
   UpdateDisplayString();
   
   SetName(name);
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
   
   if (mDescription != "")
      DrawText(mDescription, mX - 3 - GetStringWidth(mDescription), mY+12);
   
   bool isCurrent = sCurrentTextEntry == this;
   
   ofColor color(255,255,255);
   if (!isCurrent && mInErrorMode)
      color.set(200,100,100);

   if (!isCurrent)
      UpdateDisplayString();
   
   int w,h;
   GetDimensions(w,h);
   if (isCurrent)
   {
      ofSetColor(color,gModuleDrawAlpha * .1f);
      ofFill();
      ofRect(mX,mY,w,h);
   }
   ofSetColor(color,gModuleDrawAlpha);
   ofNoFill();
   ofRect(mX,mY,w,h);
   DrawText(mString, mX+2, mY+12);
   
   if (sCurrentTextEntry == this)
   {
      if (mCaretBlink)
      {
         int caretX = mX+2;
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

   ofPopStyle();
}

void TextEntry::GetDimensions(int& width, int& height)
{
   if (mFlexibleWidth)
      width = MAX(30,GetStringWidth(mString) + 4);
   else
      width = mCharWidth * 9;
   height = 15;
}

void TextEntry::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   MakeActiveTextEntry();
}

void TextEntry::MakeActiveTextEntry()
{
   sCurrentTextEntry = this;
   if (mListener)
      mListener->TextEntryActivated(this);
   mCaretPosition = strlen(mString);
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void TextEntry::OnKeyPressed(int key, bool isRepeat)
{
   if (key == OF_KEY_RETURN)
   {
      ClearActiveTextEntry(K(acceptEntry));
   }
   if (key == OF_KEY_TAB)
   {
      AcceptEntry();
      if (GetKeyModifiers() == kModifier_Shift)
      {
         if (mPreviousTextEntry)
            mPreviousTextEntry->MakeActiveTextEntry();
      }
      else
      {
         if (mNextTextEntry)
            mNextTextEntry->MakeActiveTextEntry();
      }
   }
   else if (key == OF_KEY_BACKSPACE)
   {
      int len = strlen(mString);
      if (mCaretPosition > 0)
      {
         for (int i=mCaretPosition-1; i<len; ++i)
            mString[i] = mString[i+1];
         --mCaretPosition;
      }
   }
   else if (key == OF_KEY_ESC)
   {
      ClearActiveTextEntry(!K(acceptEntry));
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
   else if (AllowCharacter(key))
   {
      int len = strlen(mString);
      if (/*len < mCharWidth && */len < MAX_TEXTENTRY_LENGTH-1)
      {
         for (int i=len; i>mCaretPosition; --i)
            mString[i] = mString[i-1];
         mString[mCaretPosition] = (char)key;
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

void TextEntry::AcceptEntry()
{
   if (mVarString)
      StringCopy(mVarString, mString, MAX_TEXTENTRY_LENGTH);
   if (mVarInt)
   {
      *mVarInt = ofClamp(ofToInt(mString), mIntMin, mIntMax);
      StringCopy(mString, ofToString(*mVarInt).c_str(), MAX_TEXTENTRY_LENGTH);
   }
   if (mVarFloat)
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
      return isdigit(c) || c == '-';
   if (mType == kTextEntry_Float)
      return isdigit(c) || c == '.' || c == '-';
   return false;
}

void TextEntry::SetNextTextEntry(TextEntry* entry)
{
   mNextTextEntry = entry;
   if (entry)
      entry->mPreviousTextEntry = this;
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
   AcceptEntry();
}
