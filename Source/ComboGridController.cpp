/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
{
   assert(false);
}

void ComboGridController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
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
      for (int i = 0; i < mGrids.size(); ++i)
      {
         mCols += mGrids[i]->NumCols();
         mRows = MAX(mRows, mGrids[i]->NumRows());
      }
   }
   else if (mArrangement == kVertical)
   {
      for (int i = 0; i < mGrids.size(); ++i)
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
      for (int i = 0; i < mGrids.size(); ++i)
      {
         if (mGrids[i] == grid)
            break;
         x += mGrids[i]->NumCols();
      }
   }
   else if (mArrangement == kVertical)
   {
      for (int i = 0; i < mGrids.size(); ++i)
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
   for (int i = 0; i < mGrids.size(); ++i)
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
      for (int i = 0; i < mGrids.size(); ++i)
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
      for (int i = 0; i < mGrids.size(); ++i)
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
   for (int i = 0; i < mCols; ++i)
   {
      for (int j = 0; j < mRows; ++j)
      {
         SetLight(i, j, kGridColorOff);
      }
   }
}

void ComboGridController::GetModuleDimensions(float& w, float& h)
{
   w = 0;
   h = 0;
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
   std::string grids = "";
   for (int i = 0; i < mGrids.size(); ++i)
   {
      IDrawableModule* grid = dynamic_cast<IDrawableModule*>(mGrids[i]);
      if (grid)
      {
         grids += grid->Name();
         if (i < mGrids.size() - 1)
            grids += ",";
      }
   }
   moduleInfo["grids"] = grids;
}

void ComboGridController::SetUpFromSaveData()
{
   std::string grids = mModuleSaveData.GetString("grids");
   std::vector<std::string> gridVec = ofSplitString(grids, ",");
   mGrids.clear();
   if (grids != "")
   {
      for (int i = 0; i < gridVec.size(); ++i)
         mGrids.push_back(dynamic_cast<IGridController*>(TheSynth->FindModule(gridVec[i])));
   }

   //for (int i=0; i<mGrids.size(); ++i)
   //   mGrids[i]->SetTarget(this);

   SetUpPatchCables(mModuleSaveData.GetString("target"));

   InitializeCombo();
}
