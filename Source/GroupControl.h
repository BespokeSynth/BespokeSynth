//
//  GroupControl.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/7/16.
//
//

#ifndef __Bespoke__GroupControl__
#define __Bespoke__GroupControl__

#include "IDrawableModule.h"
#include "Checkbox.h"

class PatchCableSource;

class GroupControl : public IDrawableModule
{
public:
   GroupControl();
   ~GroupControl();
   static IDrawableModule* Create() { return new GroupControl(); }
   
   string GetTitleLabel() override { return "group control"; }
   void CreateUIControls() override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
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
   
   Checkbox* mGroupCheckbox;
   bool mGroupEnabled;
   
   vector<PatchCableSource*> mControlCables;
};

#endif /* defined(__Bespoke__GroupControl__) */
