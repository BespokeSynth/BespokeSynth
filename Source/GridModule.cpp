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

    GridModule.cpp
    Created: 19 Jul 2020 10:36:16pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "GridModule.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "ScriptModule.h"
#include "PatchCableSource.h"

GridModule::GridModule()
{
   for (size_t i = 0; i < mHighlightCells.size(); ++i)
      mHighlightCells[i].time = -1;
   mColors.push_back(ofColor::white);
   mColors.push_back(ofColor::magenta);
   mColors.push_back(ofColor::lime);
   mColors.push_back(ofColor::red);
   mColors.push_back(ofColor::yellow);
   mColors.push_back(ofColor::blue);
   for (size_t i = 0; i < mGridOverlay.size(); ++i)
      mGridOverlay[i] = -1;
}

void GridModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mMomentaryCheckbox = new Checkbox(this, "momentary", 40, 3, &mMomentary);

   mGrid = new UIGrid(this, "uigrid", 40, 22, 90, 90, 8, 8);
   mGrid->SetListener(this);
   mGridControlTarget = new GridControlTarget(this, "grid", 4, 4);

   GetPatchCableSource()->SetEnabled(false);

   mGridOutputCable = new PatchCableSource(this, kConnectionType_Grid);
   mGridOutputCable->SetManualPosition(10, 30);
   mGridOutputCable->AddTypeFilter("gridcontroller");
   AddPatchCableSource(mGridOutputCable);
}

GridModule::~GridModule()
{
}

void GridModule::Init()
{
   IDrawableModule::Init();

   UpdateLights();
}

void GridModule::OnControllerPageSelected()
{
   if (mGridControlTarget->GetGridController())
      mGridControlTarget->GetGridController()->ResetLights();

   UpdateLights();
}

void GridModule::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   OnGridButton(col, row, value, nullptr);
}

void GridModule::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (y < GetRows() && x < GetCols())
   {
      for (auto listener : mScriptListeners)
         listener->RunCode(gTime, "on_grid_button(" + ofToString(x) + ", " + ofToString(y) + ", " + ofToString(velocity) + ")");

      if (mGridControllerOwner)
         mGridControllerOwner->OnGridButton(x, y, velocity, this);
   }

   UpdateLights();
}

void GridModule::PlayNote(NoteMessage note)
{
   OnGridButton(note.pitch % GetCols(), ((note.pitch / GetCols()) % GetRows()), note.velocity / 127.0f, nullptr);
}

void GridModule::UpdateLights()
{
   if (!mGridControlTarget)
      return;

   for (int x = 0; x < GetCols(); ++x)
   {
      for (int y = 0; y < GetRows(); ++y)
      {
         if (mGridControlTarget->GetGridController())
         {
            if (mDirectColorMode)
               mGridControlTarget->GetGridController()->SetLightDirect(x, y, Get(x, y) * 127);
            else
               mGridControlTarget->GetGridController()->SetLight(x, y, Get(x, y) > 0 ? kGridColor1Bright : kGridColorOff);
         }
      }
   }
}

void GridModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMomentaryCheckbox->Draw();

   ofSetColor(150, 150, 150, 255);
   mGrid->Draw();
   mGridControlTarget->Draw();

   ofPushStyle();
   ofSetColor(128, 128, 128, gModuleDrawAlpha * .8f);
   for (int i = 0; i < mGrid->GetRows() && i < (int)mLabels.size(); ++i)
   {
      ofVec2f pos = mGrid->GetCellPosition(0, i) + mGrid->GetPosition(true);
      float scale = MIN(mGrid->IClickable::GetDimensions().y / mGrid->GetRows() - 2, 10);
      DrawTextNormal(mLabels[i], 2, pos.y - (scale / 8), scale);
   }
   ofPopStyle();

   ofPushStyle();
   ofFill();
   for (int i = 0; i < (int)mGridOverlay.size(); ++i)
   {
      if (mGridOverlay[i] > 0)
      {
         Vec2i cell(i % kGridOverlayMaxDim, i / kGridOverlayMaxDim);
         if (cell.x < GetCols() && cell.y < GetRows())
         {
            ofVec2f pos = mGrid->GetCellPosition(cell.x, cell.y) + mGrid->GetPosition(true);
            float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
            float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();
            ofSetColor(GetColor(mGridOverlay[i]));
            ofRect(pos.x + 3, pos.y + 3, xsize - 6, ysize - 6);
         }
      }
   }
   ofPopStyle();

   ofPushStyle();
   ofSetLineWidth(3);
   for (size_t i = 0; i < mHighlightCells.size(); ++i)
   {
      if (mHighlightCells[i].time != -1)
      {
         if (gTime - mHighlightCells[i].time < mHighlightCells[i].duration)
         {
            if (gTime - mHighlightCells[i].time > 0)
            {
               ofVec2f pos = mGrid->GetCellPosition(mHighlightCells[i].position.x, mHighlightCells[i].position.y) + mGrid->GetPosition(true);
               float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
               float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();
               ofSetColor(mHighlightCells[i].color, (1 - (gTime - mHighlightCells[i].time) / mHighlightCells[i].duration) * 255);
               ofRect(pos.x, pos.y, xsize, ysize);
            }
         }
         else
         {
            mHighlightCells[i].time = -1;
         }
      }
   }
   ofPopStyle();
}

void GridModule::HighlightCell(int col, int row, double time, double duration, int colorIndex)
{
   for (size_t i = 0; i < mHighlightCells.size(); ++i)
   {
      if (mHighlightCells[i].time == -1)
      {
         mHighlightCells[i].time = time;
         mHighlightCells[i].position = Vec2i(col, row);
         mHighlightCells[i].duration = duration;
         mHighlightCells[i].color = GetColor(colorIndex);
         break;
      }
   }
}

void GridModule::SetCellColor(int col, int row, int colorIndex)
{
   mGridOverlay[col + row * kGridOverlayMaxDim] = colorIndex;
}

void GridModule::SetLabel(int row, std::string label)
{
   if (row >= (int)mLabels.size())
      mLabels.resize(row + 1);
   mLabels[row] = label;
}

void GridModule::SetColor(int colorIndex, ofColor color)
{
   if (colorIndex >= (int)mColors.size())
      mColors.resize(colorIndex + 1);
   mColors[colorIndex] = color;
}

ofColor GridModule::GetColor(int colorIndex) const
{
   if (colorIndex < (int)mColors.size())
      return mColors[colorIndex];
   return ofColor::magenta;
}

void GridModule::AddListener(ScriptModule* listener)
{
   if (!ListContains(listener, mScriptListeners))
      mScriptListeners.push_back(listener);
}

void GridModule::Clear()
{
   mGrid->Clear();
   for (size_t i = 0; i < mGridOverlay.size(); ++i)
      mGridOverlay[i] = -1;
   UpdateLights();
}

void GridModule::GetModuleDimensions(float& width, float& height)
{
   ofRectangle rect = mGrid->GetRect(true);
   width = rect.x + rect.width + 2;
   height = rect.y + rect.height + 6;
}

void GridModule::Resize(float w, float h)
{
   float curW, curH;
   GetModuleDimensions(curW, curH);
   mGrid->SetDimensions(mGrid->GetWidth() + w - curW, mGrid->GetHeight() + h - curH);
}

void GridModule::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mGrid->TestClick(x, y, right);
}

void GridModule::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mGrid->MouseReleased();
}

void GridModule::SetLight(int x, int y, GridColor color, bool force)
{
   if (x >= GetCols() || y >= GetRows())
      return;

   int colorIdx = (int)color;

   SetLightDirect(x, y, colorIdx, force);
}

void GridModule::SetLightDirect(int x, int y, int color, bool force)
{
   if (mGridControlTarget->GetGridController())
      mGridControlTarget->GetGridController()->SetLightDirect(x, y, color, force);
   SetCellColor(x, y, color);
   mGrid->SetVal(x, y, color > 0 ? 1 : 0, false);
}

void GridModule::ResetLights()
{
   if (mGridControlTarget->GetGridController())
      mGridControlTarget->GetGridController()->ResetLights();
   Clear();
}

bool GridModule::HasInput() const
{
   return false;
}

void GridModule::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mGridOutputCable)
   {
      auto* target = dynamic_cast<GridControlTarget*>(cableSource->GetTarget());
      if (target == mGridControlTarget) //patched into ourself
         cableSource->Clear();
      else if (target)
         target->SetGridController(this);
   }
}

void GridModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("direct_color_mode", moduleInfo, true);

   SetUpFromSaveData();
}

void GridModule::SetUpFromSaveData()
{
   mDirectColorMode = mModuleSaveData.GetBool("direct_color_mode");
}

void GridModule::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mMomentaryCheckbox)
      mGrid->SetMomentary(mMomentary);
}

void GridModule::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mGrid->SaveState(out);
   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
   out << mGrid->GetCols();
   out << mGrid->GetRows();
   out << mGrid->GetMajorColSize();
   out << (int)mLabels.size();
   for (auto label : mLabels)
      out << label;
   out << (int)mColors.size();
   for (auto color : mColors)
      out << color.r << color.g << color.b;
   for (auto overlay : mGridOverlay)
      out << overlay;
}

void GridModule::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 1)
   {
      mGrid->LoadState(in);
      float w, h;
      in >> w;
      in >> h;
      mGrid->SetDimensions(w, h);
   }

   if (rev >= 2)
   {
      int cols, rows, divisions, numLabels;
      in >> cols;
      in >> rows;
      in >> divisions;
      mGrid->SetGrid(cols, rows);
      mGrid->SetMajorColSize(divisions);
      in >> numLabels;
      mLabels.resize(numLabels);
      for (int i = 0; i < numLabels; ++i)
         in >> mLabels[i];
   }

   if (rev >= 3)
   {
      int numColors;
      in >> numColors;
      mColors.resize(numColors);
      for (int i = 0; i < numColors; ++i)
         in >> mColors[i].r >> mColors[i].g >> mColors[i].b;
   }

   if (rev >= 4)
   {
      for (size_t i = 0; i < mGridOverlay.size(); ++i)
         in >> mGridOverlay[i];
   }
}
