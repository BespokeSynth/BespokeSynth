//
//  ClickButton.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#ifndef __modularSynth__ClickButton__
#define __modularSynth__ClickButton__

#include "IUIControl.h"

class ClickButton;

class IButtonListener
{
public:
   virtual ~IButtonListener() {}
   virtual void ButtonClicked(ClickButton* button) = 0;
};

enum class ButtonDisplayStyle
{
   kText,
   kNoLabel,
   kPlay,
   kPause,
   kStop,
   kGrabSample
};

class ClickButton : public IUIControl
{
public:
   ClickButton(IButtonListener* owner, const char* label, int x, int y, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   ClickButton(IButtonListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, ButtonDisplayStyle displayStyle = ButtonDisplayStyle::kText);
   void SetLabel(const char* label);
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   void SetDisplayText(bool display) { mDisplayStyle = ButtonDisplayStyle::kNoLabel; }
   void SetDimensions(float width, float height) { mWidth = width; mHeight = height; }

   //IUIControl
   void SetFromMidiCC(float slider, bool setViaModulator = false) override;
   void SetValue(float value) override;
   float GetValue() const override { return GetMidiValue(); }
   float GetMidiValue() const override;
   string GetDisplayValue(float val) const override;
   int GetNumValues() override { return 2; }
   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void SaveState(FileStreamOut& out) override {}
   void LoadState(FileStreamIn& in, bool shouldSetValue) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }
   
protected:
   ~ClickButton();   //protected so that it can't be created on the stack

private:
   bool ButtonLit() const;

   void OnClicked(int x, int y, bool right) override;
   float mWidth;
   float mHeight;
   double mClickTime;
   IButtonListener* mOwner;
   ButtonDisplayStyle mDisplayStyle;
};

#endif /* defined(__modularSynth__ClickButton__) */
