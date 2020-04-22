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

class CodeEntry : public IUIControl, public IKeyboardFocusListener
{
public:
   CodeEntry(IDrawableModule* owner, const char* name, int x, int y, float w, float h);
   void OnKeyPressed(int key, bool isRepeat) override;
   void Render() override;
   
   void MakeActive();
   void Publish() { mPublishedString = mString; }
   
   void ClearInput() { mString = ""; mCaretPosition = 0; }
   const string GetText() const { return mPublishedString; }
   
   void GetDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }

   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   
protected:
   ~CodeEntry();   //protected so that it can't be created on the stack
   
private:
   void AddCharacter(char c);
   bool AllowCharacter(char c);
   int GetCaretPosition(int col, int row);
   int GetColForX(float x);
   int GetRowForY(float y);
   ofVec2f GetCaretCoords();
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   
   float mWidth;
   float mHeight;
   float mCharWidth;
   float mCharHeight;
   string mString;
   string mPublishedString;
   int mCaretPosition;
   float mCaretBlinkTimer;
   bool mCaretBlink;
   bool mHovered;
   double mLastPublishTime;
};
