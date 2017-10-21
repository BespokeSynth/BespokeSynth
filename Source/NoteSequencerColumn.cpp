//
//  NoteSequencerColumn.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/12/16.
//
//

#include "NoteSequencerColumn.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Grid.h"
#include "NoteStepSequencer.h"

NoteSequencerColumn::NoteSequencerColumn()
: mGrid(nullptr)
, mColumnSlider(nullptr)
, mRowSlider(nullptr)
, mGridCable(nullptr)
, mRow(0)
, mColumn(0)
, mSequencer(nullptr)
{
}

NoteSequencerColumn::~NoteSequencerColumn()
{
}

void NoteSequencerColumn::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mColumnSlider = new IntSlider(this,"column",2,2,80,15,&mColumn,0,15);
   mRowSlider = new IntSlider(this,"row",2,20,80,15,&mRow,0,15);
   
   mGridCable = new PatchCableSource(this, kConnectionType_Special);
   mGridCable->AddTypeFilter("notesequencer");
   AddPatchCableSource(mGridCable);
}

void NoteSequencerColumn::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mColumnSlider->Draw();
   mRowSlider->Draw();
}

void NoteSequencerColumn::PostRepatch(PatchCableSource* cableSource)
{
   if (mGridCable == cableSource)
      SyncWithCable();
}

void NoteSequencerColumn::SyncWithCable()
{
   mSequencer = dynamic_cast<NoteStepSequencer*>(mGridCable->GetTarget());
   if (mSequencer)
   {
      mGrid = mSequencer->GetGrid();
      mColumnSlider->SetExtents(0, mGrid->GetCols()-1);
      mRowSlider->SetExtents(0, mGrid->GetRows()-1);
   }
}

void NoteSequencerColumn::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (velocity > 0 && mSequencer && mGrid)
   {
      int row = mSequencer->PitchToRow(pitch);
      if (row != -1)
      {
         mRow = row;
         
         float val = 1;
         for (int i=0; i<mGrid->GetRows(); ++i)
         {
            if (mGrid->GetVal(mColumn, i) != 0)
               val = mGrid->GetVal(mColumn, i);
         }
         
         mGrid->SetVal(mColumn, mRow, val);
      }
   }
}

void NoteSequencerColumn::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mRowSlider)
   {
      if (mGrid)
         mGrid->SetVal(mColumn, mRow, 1);
   }
}

void NoteSequencerColumn::GetModuleDimensions(int &width, int &height)
{
   width = 100;
   height = 38;
}

void NoteSequencerColumn::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["target"] = mSequencer ? mSequencer->Path() : "";
}

void NoteSequencerColumn::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteSequencerColumn::SetUpFromSaveData()
{
   mGridCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SyncWithCable();
}
