//
//  IUIControl.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/13/13.
//
//

#ifndef modularSynth_IUIControl_h
#define modularSynth_IUIControl_h

#include "IClickable.h"
#include "SynthGlobals.h"

class FileStreamIn;
class FileStreamOut;
class PatchCableSource;

#define HIDDEN_UICONTROL 9999

enum AnchorDirection
{
   kAnchor_Below,
   kAnchor_Right,
   kAnchor_Right_Padded
};

class IUIControl : public IClickable
{
public:
   IUIControl() : mRemoteControlCount(0), mNoHover(false), mShouldSaveState(true) {}
   void Delete() { delete this; }
   void AddRemoteController() { ++mRemoteControlCount; }
   void RemoveRemoteController() { ++mRemoteControlCount; }
   virtual void SetFromMidiCC(float slider) = 0;
   virtual float GetValueForMidiCC(float slider) const { return 0; }
   virtual void SetValue(float value) = 0;
   virtual void SetValueDirect(float value) { SetValue(value); } //override if you need special control here
   virtual float GetValue() const { return 0; }
   virtual float GetMidiValue() { return 0; }
   virtual int GetNumValues() { return 0; } //the number of distinct values that you can have for this control, zero indicates infinite (like a float slider)
   virtual string GetDisplayValue(float val) const { return "unimplemented"; }
   virtual void Init() {}
   virtual void Poll() {}
   virtual void KeyPressed(int key, bool isRepeat) {}
   void StartBeacon() override;
   bool IsPreset();
   virtual void GetRange(float& min, float& max) { min = 0; max = 1; }
   virtual bool IsBitmask() { return false; }
   bool TestHover(int x, int y);
   void CheckHover(int x, int y);
   void DrawHover();
   void DrawPatchCableHover();
   virtual bool CanBeTargetedBy(PatchCableSource* source) const;
   virtual bool InvertScrollDirection() { return false; }
   virtual void Double() {}
   virtual void Halve() {}
   virtual void ResetToOriginal() {}
   virtual void Increment(float amount) {}
   void SetNoHover(bool noHover) { mNoHover = noHover; }
   virtual bool AttemptTextInput() { return false; }
   void PositionTo(IUIControl* anchor, AnchorDirection direction);
   void GetColors(ofColor& color, ofColor& textColor);
   bool GetShouldSaveState() const { return mShouldSaveState; }
   void SetShouldSaveState(bool save) { mShouldSaveState = save; }
   virtual bool IsSliderControl() { return true; }
   virtual bool IsButtonControl() { return false; }
   
   virtual void SaveState(FileStreamOut& out) = 0;
   virtual void LoadState(FileStreamIn& in, bool shouldSetValue = true) = 0;
protected:
   virtual ~IUIControl();
   
   int mRemoteControlCount;
   bool mNoHover;
   bool mShouldSaveState;
};

#endif
