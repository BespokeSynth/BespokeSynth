/*
  ==============================================================================

    CodeEntry.cpp
    Created: 19 Apr 2020 9:26:55am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IDrawableModule.h"
#include "CodeEntry.h"

CodeEntry::CodeEntry(IDrawableModule* owner, const char* name, int x, int y, float w, float h)
: mCharWidth(5.85f)
, mCharHeight(15)
, mLastPublishTime(-999)
{
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
   mHovered = false;
   
   SetName(name);
   SetPosition(x,y);
   mWidth = w;
   mHeight = h;
   assert(owner);
   owner->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

CodeEntry::~CodeEntry()
{
}

void CodeEntry::Render()
{
   ofPushStyle();
   
   ofSetLineWidth(.5f);
   
   int w,h;
   GetDimensions(w,h);
   
   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;
   
   ofColor color(255,255,255);
   if (mString != mPublishedString)
      color.set(200, 255, 200);
   
   if (gTime - mLastPublishTime < 200)
   {
      ofPushStyle();
      ofSetColor(0,255,0,100*(1-(gTime - mLastPublishTime)/200));
      ofFill();
      ofRect(mX,mY,w,h);
      ofPopStyle();
   }
   
   if (isCurrent)
   {
      ofSetColor(color,gModuleDrawAlpha * .1f);
      ofFill();
      ofRect(mX,mY,w,h);
   }
   ofSetColor(color,gModuleDrawAlpha);
   ofNoFill();
   ofRect(mX,mY,w,h);
   const float kFontSize = 14;
   mCharWidth = gFontFixedWidth.GetStringWidth("x", kFontSize, K(isRenderThread));
   gFontFixedWidth.DrawString(mString, kFontSize, mX+2, mY + mCharHeight);
   
   /*for (int i = 0; i<60; ++i)
   {
      for (int j=0; j<15; ++j)
         ofRect(mX + 2 + mCharWidth * i, mY + 2 + mCharHeight * j, mCharWidth, mCharHeight, L(corner,2));
   }*/
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
   {
      if (mCaretBlink)
      {
         ofVec2f coords = GetCaretCoords();
         
         ofFill();
         ofRect(coords.x * mCharWidth + mX + 1.5f, coords.y * mCharHeight + mY + 2, 1, mCharHeight, L(corner,1));
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
      ofRect(mX,mY-12,GetStringWidth(Name()),12);
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      DrawTextNormal(Name(), mX, mY);
   }*/

   ofPopStyle();
}

void CodeEntry::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   float col = GetColForX(x);
   float row = GetRowForY(y);
   mCaretPosition = GetCaretPosition(col, row);
   
   MakeActive();
}

void CodeEntry::MakeActive()
{
   SetActiveKeyboardFocus(this);
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void CodeEntry::OnKeyPressed(int key, bool isRepeat)
{
   if (key == OF_KEY_BACKSPACE)
   {
      if (mCaretPosition > 0)
      {
         --mCaretPosition;
         if (mCaretPosition < mString.length() - 1)
            mString = mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition+1);
         else
            mString = mString.substr(0, mCaretPosition);
      }
   }
   else if (key == OF_KEY_TAB)
   {
      ofVec2f coords = GetCaretCoords();
      const int kTabSize = 3;
      int spacesNeeded = kTabSize - (int)coords.x % kTabSize;
      for (int i=0; i<spacesNeeded; ++i)
         AddCharacter(' ');
   }
   else if (key == OF_KEY_ESC)
   {
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(acceptEntry));
      mString = mPublishedString;   //revert
   }
   else if (key == OF_KEY_LEFT)
   {
      if (GetKeyModifiers() == kModifier_Command)
      {
         ofVec2f coords = GetCaretCoords();
         mCaretPosition = GetCaretPosition(0, coords.y);
      }
      else
      {
         if (mCaretPosition > 0)
            --mCaretPosition;
      }
   }
   else if (key == OF_KEY_RIGHT)
   {
      if (GetKeyModifiers() == kModifier_Command)
      {
         ofVec2f coords = GetCaretCoords();
         mCaretPosition = GetCaretPosition(9999, coords.y);
      }
      else
      {
         if (mCaretPosition < mString.length())
            ++mCaretPosition;
      }
   }
   else if (key == OF_KEY_UP)
   {
      ofVec2f coords = GetCaretCoords();
      if (coords.y > 0)
         --coords.y;
      mCaretPosition = GetCaretPosition(coords.x, coords.y);
   }
   else if (key == OF_KEY_DOWN)
   {
      ofVec2f coords = GetCaretCoords();
      ++coords.y;
      mCaretPosition = GetCaretPosition(coords.x, coords.y);
   }
   else if (key == OF_KEY_RETURN)
   {
      ofVec2f coords = GetCaretCoords();
      int lineNum = (int)round(coords.y);
      vector<string> lines = ofSplitString(mString, "\n");
      int numSpaces = 0;
      if (lineNum < (int)lines.size())
      {
         for (int i=0; i<lines[lineNum].length(); ++i)
         {
            if (lines[lineNum][i] == ' ')
               ++numSpaces;
            else
               break;
         }
      }
      AddCharacter('\n');
      for (int i=0; i<numSpaces; ++i)
         AddCharacter(' ');
   }
   else if (key == 'V' && GetKeyModifiers() == kModifier_Command)
   {
      juce::String clipboard = SystemClipboard::getTextFromClipboard();
      for (int i=0; i<clipboard.length(); ++i)
         AddCharacter(clipboard[i]);
   }
   else if (key == 'R' && GetKeyModifiers() == kModifier_Command)
   {
      Publish();
   }
   else
   {
      AddCharacter((char)key);
   }
}

void CodeEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      mString = mString.substr(0, mCaretPosition) + c + mString.substr(mCaretPosition);
      ++mCaretPosition;
   }
}

bool CodeEntry::AllowCharacter(char c)
{
   if (c == '\n')
      return true;
   return c >= ' ' && c <= '~';  //these encompass the ASCII range of printable characters
}

bool CodeEntry::MouseMoved(float x, float y)
{
   mHovered = TestHover(x, y);
   return false;
}

int CodeEntry::GetCaretPosition(int col, int row)
{
   vector<string> lines = ofSplitString(mString, "\n");
   int caretPos = 0;
   for (size_t i=0; i<row && i<lines.size(); ++i)
      caretPos += lines[i].length() + 1;
   
   if (row < (int)lines.size())
      caretPos += MIN(col, lines[row].length());
   
   return MIN(caretPos, (int)mString.length());
}

int CodeEntry::GetColForX(float x)
{
   x -= 2;
   
   return round(x / mCharWidth);
}

int CodeEntry::GetRowForY(float y)
{
   y -= 2;
   
   return int(y / mCharHeight);
}

ofVec2f CodeEntry::GetCaretCoords()
{
   ofVec2f coords;
   int caretRemaining = mCaretPosition;
   vector<string> lines = ofSplitString(mString, "\n");
   for (size_t i=0; i<lines.size(); ++i)
   {
      if (caretRemaining >= lines[i].length() + 1)
      {
         caretRemaining -= lines[i].length() + 1;
         ++coords.y;
      }
      else
      {
         coords.x = caretRemaining;
         break;
      }
   }
   return coords;
}

namespace
{
   const int kSaveStateRev = 0;
}

void CodeEntry::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   out << mString;
}

void CodeEntry::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mString;
   Publish();
}
