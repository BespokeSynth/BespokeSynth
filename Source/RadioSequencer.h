/*
  ==============================================================================

    RadioSequencer.h
    Created: 10 Jun 2017 4:53:13pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "GridController.h"

class PatchCableSource;

class RadioSequencer : public IDrawableModule, public ITimeListener, public IDropdownListener, public UIGridListener, public IGridControllerListener
{
public:
   RadioSequencer();
   ~RadioSequencer();
   static IDrawableModule* Create() { return new RadioSequencer(); }
   
   string GetTitleLabel() override { return "radio sequencer"; }
   void CreateUIControls() override;
   
   //IGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //ITimeListener
   void OnTimeEvent(double time) override;
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   void SetGridSize(float w, float h);
   void SetNumSteps(int numSteps, bool stretch);
   void SyncControlCablesToGrid();
   void UpdateGridLights();
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   UIGrid* mGrid;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   int mLength;
   DropdownList* mLengthSelector;
   vector<PatchCableSource*> mControlCables;
   GridController* mGridController;
};
