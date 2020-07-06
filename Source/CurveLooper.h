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
#include "EnvelopeEditor.h"

class PatchCableSource;

class CurveLooper : public IDrawableModule, public IDropdownListener, public IButtonListener, public IAudioPoller
{
public:
   CurveLooper();
   ~CurveLooper();
   static IDrawableModule* Create() { return new CurveLooper(); }
   
   string GetTitleLabel() override { return "curve looper"; }
   void CreateUIControls() override;
   
   IUIControl* GetUIControl() const { return mUIControl; }
   
   void OnTransportAdvanced(float amount) override;
   
   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   float GetPlaybackPosition();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   IUIControl* mUIControl;
   int mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   int mWidth;
   int mHeight;
   EnvelopeControl mEnvelopeControl;
   ::ADSR mAdsr;
   ClickButton* mRandomizeButton;
};

#endif /* defined(__Bespoke__CurveLooper__) */
