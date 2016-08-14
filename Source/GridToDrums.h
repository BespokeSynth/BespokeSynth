//
//  GridToDrums.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/3/16.
//
//

#ifndef __Bespoke__GridToDrums__
#define __Bespoke__GridToDrums__

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "GridController.h"

class GridToDrums : public IDrawableModule, public INoteSource, public IGridControllerListener
{
public:
   GridToDrums();
   virtual ~GridToDrums();
   static IDrawableModule* Create() { return new GridToDrums(); }
   
   string GetTitleLabel() override { return "grid to drums"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void ConnectGridController(IGridController* grid) override {}
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
protected:
   void TriggerNote();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(int& w, int& h) override { w=80; h=10; }
};

#endif /* defined(__Bespoke__GridToDrums__) */
