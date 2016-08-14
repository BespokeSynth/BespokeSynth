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

#define HIDDEN_UICONTROL 9999

enum AnchorDirection
{
   kAnchorDirection_Below,
   kAnchorDirection_Right
};

class IUIControl : public IClickable
{
public:
   IUIControl() : mRemoteControlCount(0), mNoHover(false) {}
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
   virtual void KeyPressed(int key) {}
   void StartBeacon() override;
   bool IsPreset();
   virtual void GetRange(float& min, float& max) { min = 0; max = 1; }
   virtual bool IsBitmask() { return false; }
   void CheckHover(int x, int y);
   void DrawHover();
   virtual bool InvertScrollDirection() { return false; }
   virtual void Double() {}
   virtual void Halve() {}
   virtual void ResetToOriginal() {}
   virtual void Increment(float amount) {}
   void SetNoHover(bool noHover) { mNoHover = noHover; }
   virtual bool AttemptTextInput() { return false; }
   void PositionTo(IUIControl* anchor, AnchorDirection direction);
   
   virtual void SaveState(FileStreamOut& out) = 0;
   virtual void LoadState(FileStreamIn& in, bool shouldSetValue = true) = 0;
protected:
   virtual ~IUIControl();
   
   int mRemoteControlCount;
   bool mNoHover;
};

#endif
