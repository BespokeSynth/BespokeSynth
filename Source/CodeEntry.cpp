/*
  ==============================================================================

    CodeEntry.cpp
    Created: 19 Apr 2020 9:26:55am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IDrawableModule.h"
#include "CodeEntry.h"
#include "Transport.h"

CodeEntry::CodeEntry(ICodeEntryListener* owner, const char* name, int x, int y, float w, float h)
: mListener(owner)
, mCharWidth(5.85f)
, mCharHeight(15)
, mCaretPosition(0)
, mCaretPosition2(0)
, mLastPublishTime(-999)
, mHasError(false)
, mErrorLine(-1)
{
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
   mHovered = false;
   
   SetName(name);
   SetPosition(x,y);
   mWidth = w;
   mHeight = h;
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(owner);
   assert(module);
   module->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

CodeEntry::~CodeEntry()
{
}

void CodeEntry::Render()
{
   ofPushStyle();
   ofPushMatrix();
   
   ofSetLineWidth(.5f);
   
   int w,h;
   GetDimensions(w,h);
   
   ofClipWindow(mX, mY, w, h);
   
   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;
   
   ofColor color(255,255,255);
   if (mString != mPublishedString)
      color.set(200, 255, 200);
   
   ofFill();
   if (gTime - mLastPublishTime < 200)
   {
      ofSetColor(0,255,0,100*(1-(gTime - mLastPublishTime)/200));
      ofRect(mX,mY,w,h);
   }
   
   ofNoFill();
   if (isCurrent)
   {
      ofPushStyle();
      int lum = 255 * ofMap(sinf(TheTransport->GetMeasurePos() * TWO_PI * 4), -1, 1, .4f, 1);
      if (mHasError)
         ofSetColor(255, 0, 0, gModuleDrawAlpha);
      else
         ofSetColor(lum, lum, lum, gModuleDrawAlpha);
      ofSetLineWidth(4);
      ofRect(mX,mY,w,h);
      ofPopStyle();
   }
   else
   {
      if (mHasError)
         ofSetColor(255, 0, 0, gModuleDrawAlpha);
      else
         ofSetColor(color,gModuleDrawAlpha);
      ofRect(mX,mY,w,h);
   }
   
   if (mHasError && mErrorLine >= 0)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0, gModuleDrawAlpha * .5f);
      ofRect(mX, mErrorLine * mCharHeight + mY + 3 - mScroll.y, mWidth, mCharHeight, L(corner,2));
      ofFill();
   }
   
   ofSetColor(color,gModuleDrawAlpha);
   const float kFontSize = 14;
   mCharWidth = gFontFixedWidth.GetStringWidth("x", kFontSize, K(isRenderThread));
   mCharHeight = gFontFixedWidth.GetStringHeight("x", kFontSize, K(isRenderThread));
   gFontFixedWidth.DrawString(mString, kFontSize, mX+2 - mScroll.x, mY + mCharHeight - mScroll.y);
   
   /*for (int i = 0; i<60; ++i)
   {
      for (int j=0; j<15; ++j)
         ofRect(mX + 2 + mCharWidth * i, mY + 2 + mCharHeight * j, mCharWidth, mCharHeight, L(corner,2));
   }*/
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
   {
      if (mCaretBlink)
      {
         ofVec2f coords = GetCaretCoords(mCaretPosition);
         
         ofFill();
         ofRect(coords.x * mCharWidth + mX + 1.5f - mScroll.x, coords.y * mCharHeight + mY + 2 - mScroll.y, 1, mCharHeight, L(corner,1));
      }
      mCaretBlinkTimer += ofGetLastFrameTime();
      if (mCaretBlinkTimer > .3f)
      {
         mCaretBlinkTimer -= .3f;
         mCaretBlink = !mCaretBlink;
      }
      
      if (mCaretPosition != mCaretPosition2)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(255, 255, 255, 50);
         int caretStart = MIN(mCaretPosition, mCaretPosition2);
         int caretEnd = MAX(mCaretPosition, mCaretPosition2);
         ofVec2f coordsStart = GetCaretCoords(caretStart);
         ofVec2f coordsEnd = GetCaretCoords(caretEnd);
         
         int startLineNum = (int)round(coordsStart.y);
         int endLineNum = (int)round(coordsEnd.y);
         int startCol = (int)round(coordsStart.x);
         int endCol = (int)round(coordsEnd.x);
         vector<string> lines = ofSplitString(mString, "\n");
         for (int i=startLineNum; i<=endLineNum; ++i)
         {
            int begin = 0;
            int end = (int)lines[i].length();
            if (i == startLineNum)
               begin = startCol;
            if (i == endLineNum)
               end = endCol;
            ofRect(begin * mCharWidth + mX + 1.5f - mScroll.x, i * mCharHeight + mY + 3 - mScroll.y, (end - begin) * mCharWidth, mCharHeight, L(corner,2));
         }
         
         ofPopStyle();
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

   ofPopMatrix();
   ofPopStyle();
}

void CodeEntry::SetError(bool error, int errorLine)
{
   mHasError = error;
   mErrorLine = errorLine;
}

void CodeEntry::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   float col = GetColForX(x);
   float row = GetRowForY(y);
   MoveCaret(GetCaretPosition(col, row));
   
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
   const int kTabSize = 3;
   
   if (key == OF_KEY_BACKSPACE)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else if (mCaretPosition > 0)
      {
         MoveCaret(mCaretPosition - 1, false);
         if (mCaretPosition < mString.length() - 1)
            mString = mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition+1);
         else
            mString = mString.substr(0, mCaretPosition);
      }
   }
   else if (key == KeyPress::deleteKey)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else if (mCaretPosition > 0)
      {
         if (mCaretPosition == 0)
            mString = mString.substr(1);
         if (mCaretPosition < mString.length() - 1)
            mString = mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition + 1);
      }
   }
   else if (key == OF_KEY_TAB)
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      int spacesNeeded = kTabSize - (int)coords.x % kTabSize;
      for (int i=0; i<spacesNeeded; ++i)
         AddCharacter(' ');
   }
   else if (key == OF_KEY_ESC)
   {
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(acceptEntry));
      mString = mPublishedString;   //revert
      mCaretPosition2 = mCaretPosition;
   }
   else if (key == OF_KEY_LEFT)
   {
      if (GetKeyModifiers() & kModifier_Command)
      {
         MoveCaretToStart();
      }
      else
      {
         if (mCaretPosition > 0)
            MoveCaret(mCaretPosition - 1);
      }
   }
   else if (key == OF_KEY_RIGHT)
   {
      if (GetKeyModifiers() & kModifier_Command)
      {
         MoveCaretToEnd();
      }
      else
      {
         if (mCaretPosition < mString.length())
            MoveCaret(mCaretPosition + 1);
      }
   }
   else if (key == OF_KEY_UP)
   {
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      if (coords.y > 0)
         --coords.y;
      MoveCaret(GetCaretPosition(coords.x, coords.y));
   }
   else if (key == OF_KEY_DOWN)
   {
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      ++coords.y;
      MoveCaret(GetCaretPosition(coords.x, coords.y));
   }
   else if (key == OF_KEY_RETURN)
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      int lineNum = (int)round(coords.y);
      vector<string> lines = ofSplitString(mString, "\n");
      int numSpaces = 0;
      if (mCaretPosition > 0 && mString[mCaretPosition-1] == ':') //auto-indent
         numSpaces += kTabSize;
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
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      
      juce::String clipboard = SystemClipboard::getTextFromClipboard();
      for (int i=0; i<clipboard.length(); ++i)
         AddCharacter(clipboard[i]);
   }
   else if (key == 'V' && (GetKeyModifiers() == (kModifier_Command | kModifier_Shift)))
   {
      if (gHoveredUIControl != nullptr)
      {
         if (mCaretPosition != mCaretPosition2)
            RemoveSelectedText();
         
         string insert = gHoveredUIControl->Path();
         for (int i=0; i<insert.length(); ++i)
            AddCharacter(insert[i]);
      }
   }
   else if ((key == 'C' || key == 'X') && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         int caretStart = MIN(mCaretPosition, mCaretPosition2);
         int caretEnd = MAX(mCaretPosition, mCaretPosition2);
         SystemClipboard::copyTextToClipboard(mString.substr(caretStart,caretEnd-caretStart));
         
         if (key == 'X')
            RemoveSelectedText();
      }
   }
   else if (key == KeyPress::endKey)
   {
      MoveCaretToEnd();
   }
   else if (key == KeyPress::homeKey)
   {
      MoveCaretToStart();
   }
   else if (key == 'R' && GetKeyModifiers() == kModifier_Command)
   {
      Publish();
      mListener->ExecuteCode(mString);
   }
   else
   {
      AddCharacter((char)key);
   }
}

void CodeEntry::Publish()
 {
    mPublishedString = mString;
    mLastPublishTime = gTime;
 }

void CodeEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      
      mString = mString.substr(0, mCaretPosition) + c + mString.substr(mCaretPosition);
      MoveCaret(mCaretPosition + 1, false);
   }
}

bool CodeEntry::AllowCharacter(char c)
{
   if (c == '\n')
      return true;
   return c >= ' ' && c <= '~';  //these encompass the ASCII range of printable characters
}

void CodeEntry::RemoveSelectedText()
{
   int caretStart = MIN(mCaretPosition, mCaretPosition2);
   int caretEnd = MAX(mCaretPosition, mCaretPosition2);
   mString = mString.substr(0, caretStart) + mString.substr(caretEnd);
   MoveCaret(caretStart, false);
}

void CodeEntry::MoveCaret(int pos, bool allowSelection /*=true*/)
{
   mCaretPosition = pos;
   if (!allowSelection || !(GetKeyModifiers() & kModifier_Shift))
      mCaretPosition2 = mCaretPosition;
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void CodeEntry::MoveCaretToStart()
{
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   MoveCaret(GetCaretPosition(0, coords.y));
}

void CodeEntry::MoveCaretToEnd()
{
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   MoveCaret(GetCaretPosition(9999, coords.y));
}

bool CodeEntry::MouseMoved(float x, float y)
{
   mHovered = TestHover(x, y);
   return false;
}

bool CodeEntry::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   if (fabs(mScroll.x) > fabsf(mScroll.y))
      mScroll.y = 0;
   else
      mScroll.x = 0;
   
   mScroll.x = MAX(mScroll.x + scrollX * -10, 0);
   mScroll.y = MAX(mScroll.y + scrollY * -10, 0);
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
   
   x -= mScroll.x;
   
   return round(x / mCharWidth);
}

int CodeEntry::GetRowForY(float y)
{
   y -= 2;
   
   y -= mScroll.y;
   
   return int(y / mCharHeight);
}

ofVec2f CodeEntry::GetLinePos(int lineNum, bool end /*= false*/)
{
   float x = mX - mScroll.x;
   float y = lineNum * mCharHeight + mY - mScroll.y;
   
   if (end)
   {
      vector<string> lines = ofSplitString(mString, "\n");
      if (lineNum < (int)lines.size())
         x += lines[lineNum].length() * mCharWidth;
   }
   
   return ofVec2f(x, y);
}

ofVec2f CodeEntry::GetCaretCoords(int caret)
{
   ofVec2f coords;
   int caretRemaining = caret;
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
   //if (mListener != nullptr)
   //   mListener->ExecuteCode(mString);
}
