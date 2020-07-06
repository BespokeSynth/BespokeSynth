//
//  ValueSetter.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#ifndef __Bespoke__ValueSetter__
#define __Bespoke__ValueSetter__

#include "IDrawableModule.h"
#include "IPulseReceiver.h"
#include "TextEntry.h"
#include "ClickButton.h"

class PatchCableSource;
class IUIControl;

class ValueSetter : public IDrawableModule, public IPulseReceiver, public ITextEntryListener, public IButtonListener
{
public:
   ValueSetter();
   virtual ~ValueSetter();
   static IDrawableModule* Create() { return new ValueSetter(); }
   
   string GetTitleLabel() override { return "value setter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void OnPulse(double time, float velocity, int flags) override;
   
   void ButtonClicked(ClickButton* button) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void TextEntryComplete(TextEntry* entry) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   
   void Go();
   
   PatchCableSource* mControlCable;
   IUIControl* mTarget;
   float mValue;
   TextEntry* mValueEntry;
   ClickButton* mButton;
   
   float mWidth;
   float mHeight;
};

#endif /* defined(__Bespoke__ValueSetter__) */
