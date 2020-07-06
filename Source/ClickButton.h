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

class ClickButton : public IUIControl
{
public:
   ClickButton(IButtonListener* owner, const char* label, int x, int y);
   ClickButton(IButtonListener* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection);
   void SetLabel(const char* label);
   void Render() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;

   //IUIControl
   void SetFromMidiCC(float slider) override;
   void SetValue(float value) override;
   float GetMidiValue() override;
   string GetDisplayValue(float val) const override { return val > 0 ? "click" : "nothing"; }
   int GetNumValues() override { return 2; }
   void GetDimensions(float& width, float& height) override { width = mWidth; height = 15; }
   void SaveState(FileStreamOut& out) override {}
   void LoadState(FileStreamIn& in, bool shouldSetValue) override {}
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }
   
protected:
   ~ClickButton();   //protected so that it can't be created on the stack

private:
   bool ButtonLit();

   void OnClicked(int x, int y, bool right) override;
   int mWidth;
   double mClickTime;
   IButtonListener* mOwner;
};

#endif /* defined(__modularSynth__ClickButton__) */
