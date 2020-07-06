//
//  Selector.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#ifndef __Bespoke__Selector__
#define __Bespoke__Selector__

#include "IDrawableModule.h"
#include "RadioButton.h"

class PatchCableSource;

class Selector : public IDrawableModule, public IRadioButtonListener
{
public:
   Selector();
   ~Selector();
   static IDrawableModule* Create() { return new Selector(); }
   
   string GetTitleLabel() override { return "selector"; }
   void CreateUIControls() override;
   
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   void SyncList();
   
   RadioButton* mSelector;
   int mCurrentValue;
   
   vector<PatchCableSource*> mControlCables;
};


#endif /* defined(__Bespoke__Selector__) */
