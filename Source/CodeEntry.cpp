/*
  ==============================================================================

    CodeEntry.cpp
    Created: 19 Apr 2020 9:26:55am
    Author:  Ryan Challinor

  ==============================================================================
*/

#if BESPOKE_WINDOWS
#define ssize_t ssize_t_undef_hack  //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "IDrawableModule.h"
#include "CodeEntry.h"
#include "Transport.h"
#if BESPOKE_WINDOWS
#undef ssize_t
#endif

#include "pybind11/embed.h"
#include "pybind11/stl.h"

namespace py = pybind11;

CodeEntry::CodeEntry(ICodeEntryListener* owner, const char* name, int x, int y, float w, float h)
: mListener(owner)
, mCharWidth(5.85f)
, mCharHeight(15)
, mUndoBufferPos(0)
, mUndosLeft(0)
, mRedosLeft(0)
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

namespace
{
   const float kFontSize = 14;
}

void CodeEntry::Render()
{
   ofPushStyle();
   ofPushMatrix();
   
   ofSetLineWidth(.5f);
   
   float w,h;
   GetDimensions(w,h);
   
   ofClipWindow(mX, mY, w, h);
   
   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;
   
   ofColor color = ofColor::white;
   
   ofFill();
   
   if (isCurrent)
   {
      ofSetColor(60,60,60);
   }
   else
   {
      if (mString != mPublishedString)
         ofSetColor(25,35,25);
      else
         ofSetColor(35,35,35);
   }
   ofRect(mX, mY, w, h);
   
   if (gTime - mLastPublishTime < 200)
   {
      ofSetColor(0,255,0,100*(1-(gTime - mLastPublishTime)/200));
      ofRect(mX,mY,w,h);
   }
   
   ofPushStyle();
   ofNoFill();
   if (mHasError)
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      ofSetLineWidth(2);
   }
   else if (mString != mPublishedString)
   {
      ofSetColor(170, 255, 170, gModuleDrawAlpha);
      ofSetLineWidth(2);
   }
   else
   {
      ofSetColor(color,gModuleDrawAlpha);
   }
   ofRect(mX,mY,w,h);
   ofPopStyle();
   
   if (mHasError && mErrorLine >= 0)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0, gModuleDrawAlpha * .5f);
      ofRect(mX, mErrorLine * mCharHeight + mY + 3 - mScroll.y, mWidth, mCharHeight, L(corner,2));
      ofFill();
   }
   
   mCharWidth = gFontFixedWidth.GetStringWidth("x", kFontSize, K(isRenderThread));
   mCharHeight = gFontFixedWidth.GetStringHeight("x", kFontSize, K(isRenderThread));
   
   if (mString != mPublishedString)
   {
      ofSetColor(color, gModuleDrawAlpha * .05f);
      gFontFixedWidth.DrawString(mPublishedString, kFontSize, mX+2 - mScroll.x, mY + mCharHeight - mScroll.y);
   }
   
   ofSetColor(color, gModuleDrawAlpha);
   
   //syntax-highlighted text
   static ofColor stringColor(0.9*255, 0.7*255, 0.6*255, 255);
   static ofColor numberColor(0.9*255, 0.9*255, 1.0*255, 255);
   static ofColor name1Color(0.4*255, 0.9*255, 0.8*255, 255);
   static ofColor name2Color(0.7*255, 0.9*255, 0.3*255, 255);
   static ofColor name3Color(0.3*255, 0.9*255, 0.4*255, 255);
   static ofColor definedColor(0.6*255, 1.0*255, 0.9*255, 255);
   static ofColor equalsColor(0.9*255, 0.7*255, 0.6*255, 255);
   static ofColor parenColor(0.6*255, 0.5*255, 0.9*255, 255);
   static ofColor braceColor(0.4*255, 0.5*255, 0.7*255, 255);
   static ofColor bracketColor(0.5*255, 0.8*255, 0.7*255, 255);
   static ofColor opColor(0.9*255, 0.3*255, 0.6*255, 255);
   static ofColor commaColor(0.5*255, 0.6*255, 0.5*255, 255);
   static ofColor commentColor(0.5*255, 0.5*255, 0.5*255, 255);
   static ofColor unknownColor = ofColor::white;
   
   ofPushStyle();
   const float dim = .7f;
   DrawSyntaxHighlight(mString, stringColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 3, -1);
   DrawSyntaxHighlight(mString, numberColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 2, -1);
   DrawSyntaxHighlight(mString, name1Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 1, -1);
   DrawSyntaxHighlight(mString, name2Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 90, -1);
   DrawSyntaxHighlight(mString, name3Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 91, -1);
   DrawSyntaxHighlight(mString, definedColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 92, -1);
   DrawSyntaxHighlight(mString, equalsColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 22, -1);
   DrawSyntaxHighlight(mString, parenColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 7, 8);
   DrawSyntaxHighlight(mString, braceColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 25, 26);
   DrawSyntaxHighlight(mString, bracketColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 9, 10);
   DrawSyntaxHighlight(mString, opColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 51, -1);
   DrawSyntaxHighlight(mString, commaColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 12, -1);
   DrawSyntaxHighlight(mString, commentColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 53, -1);
   DrawSyntaxHighlight(mString, unknownColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 52, -1); //"error" token (like incomplete quotes)
   DrawSyntaxHighlight(mString, unknownColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, -1, -1);
   ofPopStyle();
   
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
         auto lines = GetLines();
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

void CodeEntry::DrawSyntaxHighlight(string input, ofColor color, std::vector<int> mapping, int filter1, int filter2)
{
   string filtered = FilterText(input, mapping, filter1, filter2);
   ofSetColor(color, gModuleDrawAlpha);
   gFontFixedWidth.DrawString(filtered, kFontSize, mX+2 - mScroll.x, mY + mCharHeight - mScroll.y);
}

string CodeEntry::FilterText(string input, std::vector<int> mapping, int filter1, int filter2)
{
   int nonSpaceCount = 0;
   bool inComment = false;
   for (size_t i=0; i<input.size(); ++i)
   {
      bool isSpace = (input[i] == ' ');
      
      if (nonSpaceCount < mapping.size() && mapping[nonSpaceCount] == 53 && input[i] == '#') //comment
         inComment = true;
      
      if (input[i] == '\n')
         inComment = false;
      
      if (input[i] != '\n')
      {
         if (nonSpaceCount < mapping.size())
         {
            if (mapping[nonSpaceCount] != filter1 && mapping[nonSpaceCount] != filter2)
               input[i] = ' ';
         }
         else
         {
            bool showLeftovers = (filter1 == -1 && filter2 == -1);
            if (!showLeftovers)
               input[i] = ' ';
         }
      }
      
      if (!isSpace || inComment)
         ++nonSpaceCount;
   }
   return input;
}

void CodeEntry::UpdateSyntaxHighlightMapping()
{
   string syntaxHighlightCode = R"(def syntax_highlight_basic():
    """
    this uses the built in lexer/tokenizer in python to identify part of code
    will return a meaningful lookuptable for index colours per character
    """
    import tokenize
    import io
    import token
   
    text = syntax_highlight_code

    #print(token.tok_name) #   <--- dict of token-kinds.
    
    output = []
    defined = []

    with io.StringIO(unicode(text)) as f:
        try:
           tokens = tokenize.generate_tokens(f.readline)

           for token in tokens:
               #print(token)
               if token[0] in (0, 5, 56, 256):
                   continue
               if not token[1] or (token[2] == token[3]):
                   continue

               token_type = token[0]
               
               if token_type == 1:
                   if token[1] in {'print', 'def', 'class', 'break', 'continue', 'return', 'while', 'or', 'and', 'dir', 'if', 'elif', 'else', 'is', 'in', 'as', 'out', 'with', 'from', 'import', 'with', 'for'}:
                       token_type = 90
                   elif token[1] in {'False', 'True', 'yield', 'repr', 'range', 'enumerate', 'len', 'type', 'list', 'tuple', 'int', 'str', 'float'}:
                       token_type = 91
                   elif token[1] in globals() or token[1] in defined:
                       token_type = 92
                   else:
                       defined.append(token[1])

               elif token_type == 51:
                   # OPS
                   # 7: 'LPAR', 8: 'RPAR
                   # 9: 'LSQB', 10: 'RSQB'
                   # 25: 'LBRACE', 26: 'RBRACE'
                   #if token.exact_type in {7, 8, 9, 10, 25, 26}:
                   #    token_type = token.exact_type
                   #elif token.exact_type == 22:
                   #    token_type = token.exact_type
                   if token[1] in {'(', ')'}:
                      token_type = 7
                   elif token[1] in {'[', ']'}:
                      token_type = 9
                   elif token[1] in {'{', '}'}:
                     token_type = 25
                   elif token[1] in {'='}:
                     token_type = 22
                   elif token[1] in {','}:
                     token_type = 12

               # print(token)
               #  start = (line number, 1 indexed) , (char index, 0 indexed)
               # print('|start:', token.start, '|end:', token.end, "[", token.exact_type, token.type, "]")
               row_start, char_start = token[2][0]-1, token[2][1]
               row_end, char_end = token[3][0]-1, token[3][1]
               #index1 = (row_start * textWidth) + char_start
               #index2 = (row_end * textWidth) + char_end

               output = output + [token_type]*(char_end - char_start)
               #print(char_end-char_start)
        except:
           print("exception when syntax highlighting")
            

    #print(output)
    return output)";
   
   try
   {
      py::exec(syntaxHighlightCode, py::globals());
      py::globals()["syntax_highlight_code"] = mString;
      py::object ret = py::eval("syntax_highlight_basic()", py::globals());
      mSyntaxHighlightMapping = ret.cast< std::vector<int> >();
   }
   catch (const std::exception &e)
   {
      ofLog() << "syntax highlight execution exception: " << e.what();
   }
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

namespace
{
   const int kTabSize = 3;
}

void CodeEntry::OnKeyPressed(int key, bool isRepeat)
{
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
            UpdateString(mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition+1));
         else
            UpdateString(mString.substr(0, mCaretPosition));
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
            UpdateString(mString.substr(1));
         if (mCaretPosition < mString.length() - 1)
            UpdateString(mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition + 1));
      }
   }
   else if (key == OF_KEY_TAB)
   {
      if (mCaretPosition != mCaretPosition2 || (GetKeyModifiers() & kModifier_Shift))
      {
         ShiftLines(GetKeyModifiers() & kModifier_Shift);
      }
      else
      {
         ofVec2f coords = GetCaretCoords(mCaretPosition);
         int spacesNeeded = kTabSize - (int)coords.x % kTabSize;
         string tab;
         for (int i=0; i<spacesNeeded; ++i)
            tab += " ";
         AddString(tab);
      }
   }
   else if (key == ']' && GetKeyModifiers() == kModifier_Command)
   {
      ShiftLines(false);
   }
   else if (key == '[' && GetKeyModifiers() == kModifier_Command)
   {
      ShiftLines(true);
   }
   else if (key == OF_KEY_ESC)
   {
      IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(acceptEntry));
      UpdateString(mPublishedString);   //revert
      mCaretPosition2 = mCaretPosition;
   }
   else if (key == OF_KEY_LEFT)
   {
      if (GetKeyModifiers() & kModifier_Command)
      {
         MoveCaretToStart();
      }
      else if (GetKeyModifiers() & kModifier_Alt)
      {
         MoveCaretToNextToken(true);
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
      else if (GetKeyModifiers() & kModifier_Alt)
      {
         MoveCaretToNextToken(false);
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
      auto lines = GetLines();
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
      string tab = "\n";
      for (int i=0; i<numSpaces; ++i)
         tab += ' ';
      AddString(tab);
   }
   else if (key == 'V' && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();
      
      juce::String clipboard = SystemClipboard::getTextFromClipboard();
      AddString(clipboard.toStdString());
   }
   else if (key == 'V' && (GetKeyModifiers() == (kModifier_Command | kModifier_Shift)))
   {
      IClickable* pasteName = nullptr;
      if (gHoveredUIControl != nullptr)
         pasteName = gHoveredUIControl;
      else if (gHoveredModule != nullptr)
         pasteName = gHoveredModule;
      if (pasteName != nullptr)
      {
         if (mCaretPosition != mCaretPosition2)
            RemoveSelectedText();
         
         string insert = pasteName->Path();
         AddString(insert);
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
   else if (key == 'Z' && GetKeyModifiers() == kModifier_Command)
   {
      Undo();
   }
   else if (key == 'Z' && (GetKeyModifiers() == (kModifier_Command | kModifier_Shift)))
   {
      Redo();
   }
   else if (key == 'A' && GetKeyModifiers() == kModifier_Command)
   {
      mCaretPosition = 0;
      mCaretPosition2 = (int)mString.size();
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
   UpdateSyntaxHighlightMapping();
}

void CodeEntry::Undo()
{
   if (mUndosLeft > 0)
   {
      int size = (int)mUndoBuffer.size();
      mUndoBufferPos = (mUndoBufferPos - 1 + size) % size;
      --mUndosLeft;
      mRedosLeft = MIN(mRedosLeft + 1, size);
      
      mString = mUndoBuffer[mUndoBufferPos].mString;
      mCaretPosition = mUndoBuffer[mUndoBufferPos].mCaretPos;
      mCaretPosition2 = mUndoBuffer[mUndoBufferPos].mCaretPos;
      
      UpdateSyntaxHighlightMapping();
   }
}

void CodeEntry::Redo()
{
   if (mRedosLeft > 0)
   {
      int size = (int)mUndoBuffer.size();
      mUndoBufferPos = (mUndoBufferPos + 1) % size;
      --mRedosLeft;
      mUndosLeft = MIN(mUndosLeft + 1, size);
      
      mString = mUndoBuffer[mUndoBufferPos].mString;
      mCaretPosition = mUndoBuffer[mUndoBufferPos].mCaretPos;
      mCaretPosition2 = mUndoBuffer[mUndoBufferPos].mCaretPos;
      
      UpdateSyntaxHighlightMapping();
   }
}

void CodeEntry::UpdateString(string newString)
{
   mUndoBuffer[mUndoBufferPos].mCaretPos = mCaretPosition;
   
   mString = newString;
   
   int size = (int)mUndoBuffer.size();
   mRedosLeft = 0;
   mUndosLeft = MIN(mUndosLeft + 1, size);
   mUndoBufferPos = (mUndoBufferPos + 1) % size;
   mUndoBuffer[mUndoBufferPos].mString = mString;
   
   UpdateSyntaxHighlightMapping();
}

void CodeEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      string s;
      s += c;
      AddString(s);
   }
}

void CodeEntry::AddString(string s)
{
   if (mCaretPosition != mCaretPosition2)
      RemoveSelectedText();
   
   string toAdd;
   for (int i=0; i<s.size(); ++i)
   {
      if (AllowCharacter(s[i]))
         toAdd += s[i];
   }
   
   UpdateString(mString.substr(0, mCaretPosition) + toAdd + mString.substr(mCaretPosition));
   MoveCaret(mCaretPosition + toAdd.size(), false);
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
   UpdateString(mString.substr(0, caretStart) + mString.substr(caretEnd));
   MoveCaret(caretStart, false);
}

void CodeEntry::ShiftLines(bool backwards)
{
   int caretStart = MIN(mCaretPosition, mCaretPosition2);
   int caretEnd = MAX(mCaretPosition, mCaretPosition2);
   ofVec2f coordsStart = GetCaretCoords(caretStart);
   ofVec2f coordsEnd = GetCaretCoords(caretEnd);
   
   auto lines = GetLines();
   string newString = "";
   for (size_t i=0; i<lines.size(); ++i)
   {
      if (i >= coordsStart.y && i <= coordsEnd.y)
      {
         int numSpaces = 0;
         for (size_t j=0; j<lines[i].size(); ++j)
         {
            if (lines[i][j] != ' ')
               break;
            ++numSpaces;
         }
         
         if (backwards)
         {
            int charsToRemove = numSpaces % kTabSize;
            if (charsToRemove == 0)
               charsToRemove = kTabSize;
            if (charsToRemove > numSpaces)
               charsToRemove = numSpaces;
            lines[i] = lines[i].substr(charsToRemove);
            if (i == coordsStart.y)
               caretStart = (int)newString.size() + numSpaces - charsToRemove;
            if (i == coordsEnd.y)
               caretEnd = (int)newString.size() + (int)lines[i].size();
         }
         else
         {
            int spacesNeeded = kTabSize - (int)numSpaces % kTabSize;
            for (int j=0; j<spacesNeeded; ++j)
               lines[i] = " " + lines[i];
            if (i == coordsStart.y)
               caretStart = (int)newString.size() + numSpaces + spacesNeeded;
            if (i == coordsEnd.y)
               caretEnd = (int)newString.size() + (int)lines[i].size();
         }
      }
      newString += lines[i] + "\n";
   }
   UpdateString(newString);
   
   mCaretPosition = caretStart;
   mCaretPosition2 = caretEnd;
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
   auto lines = GetLines();
   int x = 0;
   if (coords.y < lines.size())
   {
      for (size_t i=0; i<lines[coords.y].size(); ++i)
      {
         if (lines[coords.y][i] == ' ')
            ++x;
         else
            break;
      }
   }
   if (x == coords.x)
      x = 0;
   MoveCaret(GetCaretPosition(x, coords.y));
}

void CodeEntry::MoveCaretToEnd()
{
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   MoveCaret(GetCaretPosition(9999, coords.y));
}

void CodeEntry::MoveCaretToNextToken(bool backwards)
{
   //eh... just move it more for now
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   int amount = 5;
   if (backwards)
      amount *= -1;
   MoveCaret(GetCaretPosition(MAX(0, coords.x + amount), coords.y));
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
   auto lines = GetLines();
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
   
   x += mScroll.x;

   if (x < 0)
      x = 0;
   
   return round(x / mCharWidth);
}

int CodeEntry::GetRowForY(float y)
{
   y -= 2;
   
   y += mScroll.y;

   if (y < 0)
      y = 0;
   
   return int(y / mCharHeight);
}

ofVec2f CodeEntry::GetLinePos(int lineNum, bool end)
{
   float x = mX - mScroll.x;
   float y = lineNum * mCharHeight + mY - mScroll.y;
   
   if (end)
   {
      auto lines = ofSplitString(mPublishedString, "\n");
      if (lineNum < (int)lines.size())
         x += lines[lineNum].length() * mCharWidth;
   }
   
   return ofVec2f(x, y);
}

ofVec2f CodeEntry::GetCaretCoords(int caret)
{
   ofVec2f coords;
   int caretRemaining = caret;
   auto lines = GetLines();
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
   UpdateString(mString);
   //Publish();
   //if (mListener != nullptr)
   //   mListener->ExecuteCode(mString);
}
