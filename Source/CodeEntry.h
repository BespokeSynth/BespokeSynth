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
   virtual pair<int,int> ExecuteBlock(int lineStart, int lineEnd) = 0;  //return start/end lines that actually ran
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
   const string GetText() const { return mPublishedString; }
   const vector<string> GetLines() const { return ofSplitString(mString, "\n"); }
   void SetText(string text) { UpdateString(text); }
   void SetError(bool error, int errorLine = -1);
   void SetDoSyntaxHighlighting(bool highlight) { mDoSyntaxHighlighting = highlight; }
   
   static void OnPythonInit();
   
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void SetDimensions(float width, float height);

   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   bool IsTextEntry() const override { return true; }
   
   ofVec2f GetLinePos(int lineNum, bool end, bool published = true);
   float GetCharHeight() const { return mCharHeight; }
   float GetCharWidth() const { return mCharWidth; }
   
protected:
   ~CodeEntry();   //protected so that it can't be created on the stack
   
private:
   void AddCharacter(char c);
   void AddString(string s);
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
   void UpdateString(string newString);
   void DrawSyntaxHighlight(string input, ofColor color, std::vector<int> mapping, int filter1, int filter2);
   string FilterText(string input, std::vector<int> mapping, int filter1, int filter2);
   void OnCodeUpdated();
   string GetVisibleCode();
   bool IsAutocompleteShowing();
   void AcceptAutocompletion();
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   struct UndoBufferEntry
   {
      UndoBufferEntry() : mCaretPos(0) {}
      string mString;
      int mCaretPos;
   };

   struct AutocompleteSignatureInfo
   {
      bool valid;
      int entryIndex;
      vector<string> params;
      int caretPos;
   };

   struct AutocompleteInfo
   {
      bool valid;
      string autocompleteFull;
      string autocompleteRest;
   };
   
   ICodeEntryListener* mListener;
   float mWidth;
   float mHeight;
   float mCharWidth;
   float mCharHeight;
   string mString;
   string mPublishedString;
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
};
