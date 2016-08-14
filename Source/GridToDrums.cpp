//
//  GridToDrums.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/3/16.
//
//

#include "GridToDrums.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

GridToDrums::GridToDrums()
{
   SetIsNoteOrigin(true);
}

GridToDrums::~GridToDrums()
{
}

void GridToDrums::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void GridToDrums::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void GridToDrums::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   mNoteOutput.PlayNote(gTime, x+(grid->NumRows()-1-y)*grid->NumCols(), velocity*127);
}

void GridToDrums::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush();
}

void GridToDrums::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void GridToDrums::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
