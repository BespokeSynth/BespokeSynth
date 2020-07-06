//
//  RadioButton.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#ifndef __modularSynth__RadioButton__
#define __modularSynth__RadioButton__

#include "IUIControl.h"

struct RadioButtonElement
{
   string mLabel;
   int mValue;
};

enum RadioDirection
{
   kRadioVertical,
   kRadioHorizontal
};

class RadioButton;

class IRadioButtonListener
{
public:
   virtual ~IRadioButtonListener() {}
   virtual void RadioButtonUpdated(RadioButton* radio, int oldVal) = 0;
};

class RadioButton : public IUIControl
{
public:
   RadioButton(IRadioButtonListener* owner, const char* name, int x, int y, int* var, RadioDirection direction = kRadioVertical);
   RadioButton(IRadioButtonListener* owner, const char* name, IUIControl* anchor, AnchorDirection anchorDirection, int* var, RadioDirection direction = kRadioVertical);
   void AddLabel(const char* label, int value);
   void SetLabel(const char* label, int value);
   void Render() override;
   void SetMultiSelect(bool on) { mMultiSelect = on; }
   void Clear();
   EnumMap GetEnumMap();
   
   bool MouseMoved(float x, float y) override;
   
   static int GetSpacing();

   //IUIControl
   void SetFromMidiCC(float slider) override;
   float GetValueForMidiCC(float slider) const override;
   void SetValue(float value) override;
   void SetValueDirect(float value) override;
   float GetValue() const override;
   float GetMidiValue() override;
   int GetNumValues() override { return (int)mElements.size(); }
   string GetDisplayValue(float val) const override;
   bool IsBitmask() override { return mMultiSelect; }
   bool InvertScrollDirection() override { return mDirection == kRadioVertical; }
   void Increment(float amount) override;
   void Poll() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;

   void GetDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   
   ofVec2f GetOptionPosition(int optionIndex);
   
protected:
   ~RadioButton();   //protected so that it can't be created on the stack

private:
   void SetIndex(int i);
   void CalcSliderVal();
   void UpdateDimensions();

   void OnClicked(int x, int y, bool right) override;
   
   int mWidth;
   int mHeight;
   float mElementWidth;
   vector<RadioButtonElement> mElements;
   int* mVar;
   IRadioButtonListener* mOwner;
   bool mMultiSelect; //makes this... not a radio button. mVar becomes a bitmask
   RadioDirection mDirection;
   float mSliderVal;
   int mLastSetValue;
};

#endif /* defined(__modularSynth__RadioButton__) */
