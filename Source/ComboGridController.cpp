//
//  ComboGridController.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 2/10/15.
//
//

#include "ComboGridController.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

ComboGridController::ComboGridController()
: mRows(0)
, mCols(0)
{
   assert(false);
}

void ComboGridController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

string ComboGridController::GetTitleLabel()
{
   string title = "combo:";
   for (int i=0; i<mGrids.size(); ++i)
   {
      title += dynamic_cast<IClickable*>(mGrids[i])->Name();
      if (i < mGrids.size()-1)
         title += "+";
   }
   return title;
}

void ComboGridController::Init()
{
   IDrawableModule::Init();
   
   InitializeCombo();
}

void ComboGridController::InitializeCombo()
{
   //for (int i=0; i<mGrids.size(); ++i)
   //   mGrids[i]->SetTarget(this);
   
   mCols = 0;
   mRows = 0;
   mArrangement = mModuleSaveData.GetEnum<Arrangements>("arrangement");
   if (mArrangement == kHorizontal)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         mCols += mGrids[i]->NumCols();
         mRows = MAX(mRows, mGrids[i]->NumRows());
      }
   }
   else if (mArrangement == kVertical)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         mCols = MAX(mCols, mGrids[i]->NumCols());
         mRows += mGrids[i]->NumRows();
      }
   }
   else if (mArrangement == kSquare)
   {
      assert(false); //TODO(Ryan) implement
   }
}

void ComboGridController::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void ComboGridController::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (mArrangement == kHorizontal)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         if (mGrids[i] == grid)
            break;
         x += mGrids[i]->NumCols();
      }
   }
   else if (mArrangement == kVertical)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         if (mGrids[i] == grid)
            break;
         y += mGrids[i]->NumRows();
      }
   }
   else if (mArrangement == kSquare)
   {
      assert(false); //TODO(Ryan) implement
   }
   
   for (auto cable : GetPatchCableSource()->GetPatchCables())
   {
      auto* listener = dynamic_cast<IGridControllerListener*>(cable->GetTarget());
      if (listener)
         listener->OnGridButton(x, y, velocity, this);
   }

   GetPatchCableSource()->AddHistoryEvent(gTime, HasInput());
}

bool ComboGridController::HasInput() const
{
   for (int i=0; i<mGrids.size(); ++i)
   {
      if (mGrids[i]->HasInput())
         return true;
   }
   return false;
}

void ComboGridController::SetLight(int x, int y, GridColor color, bool force)
{
   if (mArrangement == kHorizontal)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         if (x < mGrids[i]->NumCols())
         {
            mGrids[i]->SetLight(x, y, color, force);
            break;
         }
         x -= mGrids[i]->NumCols();
      }
   }
   else if (mArrangement == kVertical)
   {
      for (int i=0; i<mGrids.size(); ++i)
      {
         if (y < mGrids[i]->NumRows())
         {
            mGrids[i]->SetLight(x, y, color, force);
            break;
         }
         y -= mGrids[i]->NumRows();
      }
   }
   else if (mArrangement == kSquare)
   {
      assert(false); //TODO(Ryan) implement
   }
}

void ComboGridController::SetLightDirect(int x, int y, int color, bool force)
{
   assert(false); //TODO(Ryan) implement, maybe
}

void ComboGridController::ResetLights()
{
   for (int i=0; i<mCols; ++i)
   {
      for (int j=0; j<mRows; ++j)
      {
         SetLight(i,j,kGridColorOff);
      }
   }
}

void ComboGridController::GetModuleDimensions(float& w, float& h)
{
   w = 0;
   h = 0;
}

void ComboGridController::SetTarget(IClickable* target)
{
   GetPatchCableSource()->SetTarget(target);
}

void ComboGridController::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("grids", moduleInfo);
   EnumMap map;
   map["horizontal"] = kHorizontal;
   map["vertical"] = kVertical;
   map["square"] = kSquare;
   mModuleSaveData.LoadEnum<Arrangements>("arrangement", moduleInfo, kHorizontal, nullptr, &map);
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ComboGridController::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   string grids = "";
   for (int i=0; i<mGrids.size(); ++i)
   {
      IDrawableModule* grid = dynamic_cast<IDrawableModule*>(mGrids[i]);
      if (grid)
      {
         grids += grid->Name();
         if (i < mGrids.size()-1)
            grids += ",";
      }
   }
   moduleInfo["grids"] = grids;
}

void ComboGridController::SetUpFromSaveData()
{
   string grids = mModuleSaveData.GetString("grids");
   vector<string> gridVec = ofSplitString(grids, ",");
   mGrids.clear();
   if (grids != "")
   {
      for (int i=0; i<gridVec.size(); ++i)
         mGrids.push_back(dynamic_cast<IGridController*>(TheSynth->FindModule(gridVec[i])));
   }
   
   //for (int i=0; i<mGrids.size(); ++i)
   //   mGrids[i]->SetTarget(this);
   
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   
   InitializeCombo();
}
