//
//  GridController.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/9/15.
//
//

#ifndef __Bespoke__GridController__
#define __Bespoke__GridController__

#include "IDrawableModule.h"
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
   virtual void ConnectGridController(IGridController* grid) = 0;
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
   
   virtual void SetTarget(IClickable* target) = 0;
   
   NoteHistory& GetNoteHistory() { return mHistory; }
protected:
   NoteHistory mHistory;
};

class GridController : public IDrawableModule, public MidiDeviceListener, public IGridController
{
public:
   GridController();
   ~GridController() {}
   static IDrawableModule* Create() { return new GridController(); }
   
   string GetTitleLabel() override;
   void CreateUIControls() override;
   
   void DrawGrid();
   
   void SetTarget(IClickable* target) override;
   
   void SetLight(int x, int y, GridColor color, bool force = false) override;
   void SetLightDirect(int x, int y, int color, bool force = false) override;
   void ResetLights() override;
   int NumCols() override { return mCols; }
   int NumRows() override { return mRows; }
   bool HasInput() const override;
   bool IsMultisliderGrid() const override { return mColors.empty(); }

   void ControllerPageSelected() override;
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   void PostRepatch(PatchCableSource* cable) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   void OnInput(int control, float velocity);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& x, int&y) override;
   void OnClicked(int x, int y, bool right) override;
   void MouseReleased() override;

   unsigned int mRows;
   unsigned int mCols;
   int mControls[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   float mInput[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   int mLights[MAX_GRIDCONTROLLER_COLS][MAX_GRIDCONTROLLER_ROWS];
   vector<int> mColors;
   MidiMessageType mMessageType;
   MidiController* mController;
   int mControllerPage;
   UIGrid* mGrid;
   bool mClicked;
   GridCell mClickedCell;
};

#endif /* defined(__Bespoke__GridController__) */
