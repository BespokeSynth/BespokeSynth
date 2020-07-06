//
//  ComboGridController.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/10/15.
//
//

#ifndef __Bespoke__ComboGridController__
#define __Bespoke__ComboGridController__

#include "GridController.h"

class ComboGridController : public IDrawableModule, public IGridController, public IGridControllerListener
{
public:
   ComboGridController();
   ~ComboGridController() {}
   static IDrawableModule* Create() { return new ComboGridController(); }
   
   string GetTitleLabel() override;
   void CreateUIControls() override;
   
   void Init() override;
   
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return mCols; }
   int NumRows() override { return mRows; }
   bool HasInput() const override;
   
   void SetTarget(IClickable* target);

   void OnControllerPageSelected() override {}
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   enum Arrangements
   {
      kHorizontal,
      kVertical,
      kSquare
   };

   void InitializeCombo();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override;

   unsigned int mRows;
   unsigned int mCols;
   vector<IGridController*> mGrids;
   Arrangements mArrangement;
};

#endif /* defined(__Bespoke__ComboGridController__) */
