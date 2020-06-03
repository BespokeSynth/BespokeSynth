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
   virtual void ExecuteCode(string code) = 0;
};

class CodeEntry : public IUIControl, public IKeyboardFocusListener
{
public:
   CodeEntry(ICodeEntryListener* owner, const char* name, int x, int y, float w, float h);
   void OnKeyPressed(int key, bool isRepeat) override;
   void Render() override;
   
   void MakeActive();
   void Publish();
   
   void ClearInput() { mString = ""; mCaretPosition = 0; }
   const string GetText() const { return mPublishedString; }
   const vector<string> GetLines() const { return ofSplitString(mString, "\n"); }
   void SetText(string text) { mString = text; }
   void SetError(bool error, int errorLine = -1);
   
   void GetDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }

   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   
   ofVec2f GetLinePos(int lineNum, bool end);
   float GetCharHeight() const { return mCharHeight; }
   float GetCharWidth() const { return mCharWidth; }
   
protected:
   ~CodeEntry();   //protected so that it can't be created on the stack
   
private:
   void AddCharacter(char c);
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
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   bool MouseScrolled(int x, int y, float scrollX, float scrollY) override;
   
   ICodeEntryListener* mListener;
   float mWidth;
   float mHeight;
   float mCharWidth;
   float mCharHeight;
   string mString;
   string mPublishedString;
   int mCaretPosition;
   int mCaretPosition2;
   float mCaretBlinkTimer;
   bool mCaretBlink;
   bool mHovered;
   double mLastPublishTime;
   bool mHasError;
   int mErrorLine;
   ofVec2f mScroll;
};
