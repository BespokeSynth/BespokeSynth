//
//  Checkbox.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#ifndef __modularSynth__Checkbox__
#define __modularSynth__Checkbox__

#include <iostream>
#include "IUIControl.h"

class Checkbox;

class Checkbox : public IUIControl
{
public:
   Checkbox(IDrawableModule* owner, const char* label, int x, int y, bool* var);
   Checkbox(IDrawableModule* owner, const char* label, IUIControl* anchor, AnchorDirection anchorDirection, bool* var);
   void SetLabel(const char* label);
   void SetVar(bool* var) { mVar = var; }
   void Render() override;
   void SetDisplayText(bool display);
   void UseCircleLook(ofColor color);
   void DisableCircleLook() { mUseCircleLook = false; }
   
   bool MouseMoved(float x, float y) override;

   //IUIControl
   void SetFromMidiCC(float slider) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   float GetValue() const override;
   float GetMidiValue() override;
   int GetNumValues() override { return 2; }
   string GetDisplayValue(float val) const override;
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return true; }
   
   bool CheckNeedsDraw() override;
   
protected:
   ~Checkbox();   //protected so that it can't be created on the stack

private:
   void OnClicked(int x, int y, bool right) override;
   void GetDimensions(float& width, float& height) override { width = mWidth; height = 15; }
   void CalcSliderVal();
   int mWidth;
   bool* mVar;
   IDrawableModule* mOwner;
   bool mLastDisplayedValue;
   bool mDisplayText;
   bool mUseCircleLook;
   ofColor mCustomColor;
   float mSliderVal;
   bool mLastSetValue;
};

#endif /* defined(__modularSynth__Checkbox__) */
