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

    CodeEntry.h
    Created: 19 Apr 2020 9:26:55am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IUIControl.h"
#include "SynthGlobals.h"
#include "TextEntry.h"

class ICodeEntryListener
{
public:
   virtual ~ICodeEntryListener() {}
   virtual void ExecuteCode() = 0;
   virtual std::pair<int,int> ExecuteBlock(int lineStart, int lineEnd) = 0;  //return start/end lines that actually ran
   virtual void OnCodeUpdated() {}
};

class CodeEntry : public IUIControl, public IKeyboardFocusListener
{
public:
   CodeEntry(ICodeEntryListener* owner, const char* name, int x, int y, float w, float h);
   void OnKeyPressed(int key, bool isRepeat) override;
   void Render() override;
   void Poll() override;
   
   void RenderOverlay();
   void MakeActive();
   void Publish();
   
   void ClearInput() { mString = ""; mCaretPosition = 0; }
   const std::string GetText(bool published) const { return published ? mPublishedString : mString; }
   const std::vector<std::string> GetLines(bool published) const { return ofSplitString(published ? mPublishedString : mString, "\n"); }
   void SetText(std::string text) { UpdateString(text); }
   void SetError(bool error, int errorLine = -1);
   void SetDoSyntaxHighlighting(bool highlight) { mDoSyntaxHighlighting = highlight; }
   static bool HasJediNotInstalledWarning() { return sWarnJediNotInstalled; }
   
   static void OnPythonInit();
   
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void SetDimensions(float width, float height);

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool IsTextEntry() const override { return true; }
   
   ofVec2f GetLinePos(int lineNum, bool end, bool published = true);
   float GetCharHeight() const { return mCharHeight; }
   float GetCharWidth() const { return mCharWidth; }

   void SetStyleFromJSON(const ofxJSONElement &vdict);
   
protected:
   ~CodeEntry();   //protected so that it can't be created on the stack
   
private:
   void AddCharacter(char c);
   void AddString(std::string s);
   bool AllowCharacter(char c);
   int GetCaretPosition(int col, int row);
   int GetColForX(float x);
   int GetRowForY(float y);
   ofVec2f GetCaretCoords(int caret);
   void RemoveSelectedText();
   void ShiftLines(bool backwards);
   void MoveCaret(int pos, bool allowSelection = true);
   void MoveCaretToStart();
   void MoveCaretToEnd();
   void MoveCaretToNextToken(bool backwards);
   void Undo();
   void Redo();
   void UpdateString(std::string newString);
   void DrawSyntaxHighlight(std::string input, ofColor color, std::vector<int> mapping, int filter1, int filter2);
   std::string FilterText(std::string input, std::vector<int> mapping, int filter1, int filter2);
   void OnCodeUpdated();
   std::string GetVisibleCode();
   bool IsAutocompleteShowing();
   void AcceptAutocompletion();
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   static bool sWarnJediNotInstalled;
   
   struct UndoBufferEntry
   {
      UndoBufferEntry() : mCaretPos(0) {}
      std::string mString;
      int mCaretPos;
   };

   struct AutocompleteSignatureInfo
   {
      bool valid;
      int entryIndex;
      std::vector<std::string> params;
      int caretPos;
   };

   struct AutocompleteInfo
   {
      bool valid;
      std::string autocompleteFull;
      std::string autocompleteRest;
   };
   
   ICodeEntryListener* mListener;
   float mWidth;
   float mHeight;
   float mCharWidth;
   float mCharHeight;
   std::string mString;
   std::string mPublishedString;
   std::array<UndoBufferEntry, 50> mUndoBuffer;
   int mUndoBufferPos;
   int mUndosLeft;
   int mRedosLeft;
   int mCaretPosition;
   int mCaretPosition2;
   float mCaretBlinkTimer;
   bool mCaretBlink;
   bool mHovered;
   double mLastPublishTime;
   int mLastPublishedLineStart;
   int mLastPublishedLineEnd;
   bool mHasError;
   int mErrorLine;
   ofVec2f mScroll;
   std::vector<int> mSyntaxHighlightMapping;
   bool mDoSyntaxHighlighting;
   bool mDoPythonAutocomplete;
   double mLastInputTime;
   std::array<AutocompleteSignatureInfo, 10> mAutocompleteSignatures;
   std::array<AutocompleteInfo, 10> mAutocompletes;
   float mAutocompleteUpdateTimer;
   ofVec2f mAutocompleteCaretCoords;
   bool mWantToShowAutocomplete;
   int mAutocompleteHighlightIndex;
   bool mCodeUpdated;


   // Style Sheet
   ofColor currentBg{60,60,60}, publishedBg{25,35,25}, unpublishedBg{35,35,35};
   ofColor stringColor{(int)(0.9*255), (int)(0.7*255), (int)(0.6*255), 255};
   ofColor numberColor{(int)(0.9*255), (int)(0.9*255), (int)(1.0*255), 255};
   ofColor name1Color{(int)(0.4*255), (int)(0.9*255), (int)(0.8*255), 255};
   ofColor name2Color{(int)(0.7*255), (int)(0.9*255), (int)(0.3*255), 255};
   ofColor name3Color{(int)(0.3*255), (int)(0.9*255), (int)(0.4*255), 255};
   ofColor definedColor{(int)(0.6*255), (int)(1.0*255), (int)(0.9*255), 255};
   ofColor equalsColor{(int)(0.9*255), (int)(0.7*255), (int)(0.6*255), 255};
   ofColor parenColor{(int)(0.6*255), (int)(0.5*255), (int)(0.9*255), 255};
   ofColor braceColor{(int)(0.4*255), (int)(0.5*255), (int)(0.7*255), 255};
   ofColor bracketColor{(int)(0.5*255), (int)(0.8*255), (int)(0.7*255), 255};
   ofColor opColor{(int)(0.9*255), (int)(0.3*255), (int)(0.6*255), 255};
   ofColor commaColor{(int)(0.5*255), (int)(0.6*255), (int)(0.5*255), 255};
   ofColor commentColor{(int)(0.5*255), (int)(0.5*255), (int)(0.5*255), 255};
   ofColor unknownColor = ofColor::white;

   float mFontSize = 14;
};
