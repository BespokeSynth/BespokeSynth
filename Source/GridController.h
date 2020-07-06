//
//  GridController.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#ifndef __Bespoke__GridController__
#define __Bespoke__GridController__

#include "IUIControl.h"
#include "MidiController.h"
#include "UIGrid.h"
#include "INoteSource.h"

#define MAX_GRIDCONTROLLER_ROWS 512
#define MAX_GRIDCONTROLLER_COLS 512

class IGridController;
class PatchCableSource;

enum GridColor
{
   kGridColorOff,
   kGridColor1Dim,
   kGridColor1Bright,
   kGridColor2Dim,
   kGridColor2Bright,
   kGridColor3Dim,
   kGridColor3Bright
};

class IGridControllerListener
{
public:
   virtual ~IGridControllerListener() {}
   virtual void OnControllerPageSelected() = 0;
   virtual void OnGridButton(int x, int y, float velocity, IGridController* grid) = 0;
};

class IGridController
{
public:
   virtual ~IGridController() {}
   virtual void SetLight(int x, int y, GridColor color, bool force = false) = 0;
   virtual void SetLightDirect(int x, int y, int color, bool force = false) = 0;
   virtual void ResetLights() = 0;
   virtual int NumCols() = 0;
   virtual int NumRows() = 0;
   virtual bool HasInput() const = 0;
   virtual bool IsMultisliderGrid() const { return false; }
};

class GridController : public IUIControl, public IGridController
{
public:
   GridController(IGridControllerListener* owner, const char* name, int x, int y);
   ~GridController() {}
   
   void Render() override;
   
   void SetUp(GridLayout* layout, int page, MidiController* controller);
   void UnhookController();
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return mCols; }
   int NumRows() override { return mRows; }
   bool HasInput() const override;
   //bool IsMultisliderGrid() const override { return mColors.empty(); }   //commented out... don't remember what types of grids this is supposed to be for
   
   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   bool CanBeTargetedBy(PatchCableSource* source) const override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   bool IsSliderControl() override { return false; }
   bool IsButtonControl() override { return false; }

   void OnControllerPageSelected();
   void OnInput(int control, float velocity);
   
private:
   void GetDimensions(float& width, float& height) override { width = 30; height = 15; }
   
   unsigned int mRows;
   unsigned int mCols;
   int mControls[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   float mInput[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   int mLights[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   vector<int> mColors;
   MidiMessageType mMessageType;
   MidiController* mController;
   int mControllerPage;
   IGridControllerListener* mOwner;
};

#endif /* defined(__Bespoke__GridController__) */
