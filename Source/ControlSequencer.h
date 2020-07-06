//
//  ControlSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/27/15.
//
//

#ifndef __Bespoke__ControlSequencer__
#define __Bespoke__ControlSequencer__

#include <iostream>
#include "IDrawableModule.h"
#include "UIGrid.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"

class PatchCableSource;

class ControlSequencer : public IDrawableModule, public ITimeListener, public IDropdownListener, public UIGridListener, public IButtonListener
{
public:
   ControlSequencer();
   ~ControlSequencer();
   static IDrawableModule* Create() { return new ControlSequencer(); }
   
   string GetTitleLabel() override { return "control sequencer"; }
   void CreateUIControls() override;
   
   IUIControl* GetUIControl() const { return mUIControl; }
   
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
   
   void CheckboxUpdated(Checkbox* checkbox) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
   static list<ControlSequencer*> sControlSequencers;
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
private:
   void SetGridSize(float w, float h);
   void SetNumSteps(int numSteps, bool stretch);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override;
   void OnClicked(int x, int y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   
   UIGrid* mGrid;
   IUIControl* mUIControl;
   NoteInterval mInterval;
   DropdownList* mIntervalSelector;
   int mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   ClickButton* mRandomize;
};

#endif /* defined(__Bespoke__ControlSequencer__) */
