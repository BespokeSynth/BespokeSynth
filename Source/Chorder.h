//
//  Chorder.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef __modularSynth__Chorder__
#define __modularSynth__Chorder__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "DropdownList.h"

#define TOTAL_NUM_NOTES 128

class Chorder : public NoteEffectBase, public IDrawableModule, public UIGridListener, public IDropdownListener
{
public:
   Chorder();
   static IDrawableModule* Create() { return new Chorder(); }
   
   string GetTitleLabel() override { return "chorder"; }
   void CreateUIControls() override;
   
   void AddTone(int tone, float velocity=1);
   void RemoveTone(int tone);
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* dropdown, int oldVal) override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   virtual bool Enabled() const override { return mEnabled; }
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 135; height = 75; }
   void OnClicked(int x, int y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void PlayChorderNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation);
   void CheckLeftovers();
   void SyncChord();
   
   UIGrid* mChordGrid;
   int mVelocity;
   bool mInputNotes[TOTAL_NUM_NOTES];
   int mHeldCount[TOTAL_NUM_NOTES];
   
   bool mDiatonic;
   int mChordIndex;
   int mInversion;
   Checkbox* mDiatonicCheckbox;
   DropdownList* mChordDropdown;
   DropdownList* mInversionDropdown;
};

#endif /* defined(__modularSynth__Chorder__) */

