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
/*
  ==============================================================================

    GridSliders.cpp
    Created: 2 Aug 2021 10:32:05pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "GridSliders.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

GridSliders::GridSliders()
{
}

void GridSliders::Init()
{
   IDrawableModule::Init();
}

GridSliders::~GridSliders()
{
}

void GridSliders::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDirectionSelector = new DropdownList(this, "direction", 40, 3, (int*)(&mDirection));
   mGridControlTarget = new GridControlTarget(this, "grid", 3, 3);

   mDirectionSelector->AddLabel("vertical", (int)Direction::kVertical);
   mDirectionSelector->AddLabel("horizontal", (int)Direction::kHorizontal);

   for (int i = 0; i < (int)mControlCables.size(); ++i)
   {
      mControlCables[i] = new PatchCableSource(this, kConnectionType_ValueSetter);
      mControlCables[i]->SetManualPosition(i * 12 + 8, 28);
      mControlCables[i]->SetManualSide(PatchCableSource::Side::kBottom);
      AddPatchCableSource(mControlCables[i]);
   }
}

void GridSliders::Poll()
{
   if (mGridControlTarget->GetGridController())
   {
      int length;
      if (mDirection == Direction::kVertical)
         length = mGridControlTarget->GetGridController()->NumRows();
      else
         length = mGridControlTarget->GetGridController()->NumCols();

      for (int i = 0; i < (int)mControlCables.size(); ++i)
      {
         GridColor color = GridColor::kGridColorOff;

         float value = 0;
         if (mControlCables[i]->GetTarget())
            value = dynamic_cast<IUIControl*>(mControlCables[i]->GetTarget())->GetMidiValue();

         for (int j = 0; j < length; ++j)
         {
            float squareValue = j / float(length - 1);
            if (squareValue <= value + .01f)
               color = GridColor::kGridColor1Bright;
            else
               color = GridColor::kGridColorOff;

            int row, col;
            if (mDirection == Direction::kVertical)
            {
               row = length - j - 1;
               col = i;
            }
            else
            {
               row = i;
               col = j;
            }

            mGridControlTarget->GetGridController()->SetLight(col, row, color);
         }
      }
   }
}

void GridSliders::OnControllerPageSelected()
{
}

void GridSliders::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (velocity > 0)
   {
      int sliderIndex, squareIndex, length;
      if (mDirection == Direction::kVertical)
      {
         sliderIndex = x;
         squareIndex = grid->NumRows() - y - 1;
         length = mGridControlTarget->GetGridController()->NumRows();
      }
      else
      {
         sliderIndex = y;
         squareIndex = x;
         length = mGridControlTarget->GetGridController()->NumCols();
      }

      if (sliderIndex < mControlCables.size() && mControlCables[sliderIndex]->GetTarget())
      {
         float value = squareIndex / float(length - 1);
         for (auto& cable : mControlCables[sliderIndex]->GetPatchCables())
            dynamic_cast<IUIControl*>(cable->GetTarget())->SetFromMidiCC(value, NextBufferTime(false), false);
      }
   }
}

void GridSliders::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mGridControlTarget->Draw();
   mDirectionSelector->Draw();

   for (size_t i = 0; i < mControlCables.size(); ++i)
   {
      bool drawCable = false;
      int numCables = 0;
      if (mGridControlTarget->GetGridController())
      {
         if (mDirection == Direction::kVertical)
            numCables = mGridControlTarget->GetGridController()->NumCols();
         else
            numCables = mGridControlTarget->GetGridController()->NumRows();
         if (mGridControlTarget->GetGridController() && (int)i < numCables)
            drawCable = true;
      }

      mControlCables[i]->SetShowing(drawCable);
   }
}

void GridSliders::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void GridSliders::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool GridSliders::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   return false;
}

void GridSliders::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void GridSliders::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mDirectionSelector)
   {
   }
}

void GridSliders::GetModuleDimensions(float& width, float& height)
{
   float cablesWidth = 0;
   for (size_t i = 0; i < mControlCables.size(); ++i)
   {
      if (mControlCables[i]->IsShowing())
         cablesWidth = i * 12 + 8 + 8;
   }

   width = MAX(120, cablesWidth);
   height = 28;
}

void GridSliders::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void GridSliders::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void GridSliders::SetUpFromSaveData()
{
}

void GridSliders::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << (int)mControlCables.size();
   for (auto cable : mControlCables)
   {
      std::string path = "";
      if (cable->GetTarget())
         path = cable->GetTarget()->Path();
      out << path;
   }
}

void GridSliders::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int size;
   in >> size;
   for (int i = 0; i < size; ++i)
   {
      std::string path;
      in >> path;
      if (i < (int)mControlCables.size())
         mControlCables[i]->SetTarget(TheSynth->FindUIControl(path));
   }
}
