//
//  TextEntry.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#ifndef __modularSynth__TextEntry__
#define __modularSynth__TextEntry__

#include <iostream>
#include "IUIControl.h"
#include "SynthGlobals.h"

class TextEntry;

class ITextEntryListener
{
public:
   virtual ~ITextEntryListener() {}
   virtual void TextEntryComplete(TextEntry* entry) = 0;
   virtual void TextEntryActivated(TextEntry* entry) {}
};

enum TextEntryType
{
   kTextEntry_Text,
   kTextEntry_Int,
   kTextEntry_Float
};

class IKeyboardFocusListener
{
public:
   virtual ~IKeyboardFocusListener() {}
   static void SetActiveKeyboardFocus(IKeyboardFocusListener* focus) { sCurrentKeyboardFocus = focus; }
   static IKeyboardFocusListener* GetActiveKeyboardFocus() { return sCurrentKeyboardFocus; }
   static void ClearActiveKeyboardFocus(bool acceptEntry);
   
   virtual void OnKeyPressed(int key, bool isRepeat) = 0;
private:
   virtual void AcceptEntry(bool pressedEnter) {}
   static IKeyboardFocusListener* sCurrentKeyboardFocus;
};

class TextEntry : public IUIControl, public IKeyboardFocusListener
{
public:
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max);
   void OnKeyPressed(int key, bool isRepeat) override;
   void Render() override;
   
   void MakeActiveTextEntry(bool setCaretToEnd);
   
   void SetNextTextEntry(TextEntry* entry);
   void UpdateDisplayString();
   void SetInErrorMode(bool error) { mInErrorMode = error; }
   void DrawLabel(bool draw) { mDrawLabel = draw; }
   void SetRequireEnter(bool require) { mRequireEnterToAccept = require; }
   void SetFlexibleWidth(bool flex) { mFlexibleWidth = flex; }
   void ClearInput() { bzero(mString, MAX_TEXTENTRY_LENGTH); mCaretPosition = 0; }
   const char* GetText() const { return mString; }
   
   void GetDimensions(float& width, float& height) override;

   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }
   
protected:
   ~TextEntry();   //protected so that it can't be created on the stack
   
private:
   void Construct(ITextEntryListener* owner, const char* name, int x, int y, int charWidth);  //shared constructor
   
   void AddCharacter(char c);
   bool AllowCharacter(char c);
   void AcceptEntry(bool pressedEnter) override;
   
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   
   int mCharWidth;
   ITextEntryListener* mListener;
   char mString[MAX_TEXTENTRY_LENGTH];
   char* mVarString;
   int* mVarInt;
   float* mVarFloat;
   int mIntMin;
   int mIntMax;
   float mFloatMin;
   float mFloatMax;
   int mCaretPosition;
   float mCaretBlinkTimer;
   bool mCaretBlink;
   TextEntryType mType;
   TextEntry* mNextTextEntry;
   TextEntry* mPreviousTextEntry;
   bool mInErrorMode;
   bool mDrawLabel;
   float mLabelSize;
   bool mFlexibleWidth;
   bool mHovered;
   bool mRequireEnterToAccept;
};

#endif /* defined(__modularSynth__TextEntry__) */
