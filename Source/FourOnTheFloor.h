//
//  FourOnTheFloor.h
//  modularSynth
//
//  Created by Ryan Challinor on 6/23/13.
//
//

#ifndef __modularSynth__FourOnTheFloor__
#define __modularSynth__FourOnTheFloor__

#include <iostream>
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "Transport.h"
#include "Checkbox.h"

class FourOnTheFloor : public IDrawableModule, public INoteSource, public ITimeListener
{
public:
   FourOnTheFloor();
   ~FourOnTheFloor();
   static IDrawableModule* Create() { return new FourOnTheFloor(); }
   
   string GetTitleLabel() override { return "four on the floor"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=120; height=22; }
   bool Enabled() const override { return mEnabled; }
   
   
   bool mTwoOnTheFloor;
   Checkbox* mTwoOnTheFloorCheckbox;
};

#endif /* defined(__modularSynth__FourOnTheFloor__) */

