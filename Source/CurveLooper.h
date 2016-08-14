//
//  CurveLooper.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#ifndef __Bespoke__CurveLooper__
#define __Bespoke__CurveLooper__

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Curve.h"
#include "Checkbox.h"

class PatchCableSource;

class CurveLooper : public IDrawableModule, public IDropdownListener, public IAudioPoller
{
public:
   CurveLooper();
   ~CurveLooper();
   static IDrawableModule* Create() { return new CurveLooper(); }
   
   string GetTitleLabel() override { return "curve looper"; }
   void CreateUIControls() override;
   
   IUIControl* GetUIControl() const { return mUIControl; }
   
   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
private:
   void SetNumSteps(int numSteps, bool stretch);
   float GetPlaybackPosition();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& width, int& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   Curve mCurve;
   IUIControl* mUIControl;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   int mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   int mWidth;
   int mHeight;
   bool mRecord;
   Checkbox* mRecordCheckbox;
   float mLastRecordPos;
};

#endif /* defined(__Bespoke__CurveLooper__) */
