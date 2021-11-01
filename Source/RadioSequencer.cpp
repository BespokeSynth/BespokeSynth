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

    RadioSequencer.cpp
    Created: 10 Jun 2017 4:53:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "RadioSequencer.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"
#include "MathUtils.h"

namespace
{
   const float kEarlyOffsetMs = 10;
}

RadioSequencer::RadioSequencer()
{
}

void RadioSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(kEarlyOffsetMs, true), false);
}

RadioSequencer::~RadioSequencer()
{
   TheTransport->RemoveListener(this);
}

void RadioSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   int width, height;
   UIBLOCK(3, 3, 200);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, 16); UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40); UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mGridControlTarget, new GridControlTarget(UICONTROL_BASICS("grid")));
   ENDUIBLOCK(width, height);

   mGrid = new UIGrid(5, 25, mGridControlTarget->GetRect().getMaxX() - 6, 170, mLength, 8, this);
   mGrid->SetHighlightCol(gTime, -1);
   mGrid->SetSingleColumnMode(true);
   mGrid->SetMajorColSize(4);
   mGrid->SetListener(this);
   
   /*mIntervalSelector->AddLabel("8", kInterval_8);
    mIntervalSelector->AddLabel("4", kInterval_4);
    mIntervalSelector->AddLabel("2", kInterval_2);*/
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   
   SyncControlCablesToGrid();
}

void RadioSequencer::Poll()
{
}

void RadioSequencer::OnControllerPageSelected()
{
   UpdateGridLights();
}

void RadioSequencer::OnGridButton(int x, int y, float velocity, IGridController* grid)
{
   if (velocity > 0)
      mGrid->SetVal(x, y, 1);
   UpdateGridLights();
}

void RadioSequencer::UpdateGridLights()
{
   if (mGridControlTarget->GetGridController())
   {
      for (int row=0; row<mGrid->GetRows(); ++row)
      {
         for (int col=0; col<mGrid->GetCols(); ++col)
         {
            if (mGrid->GetVal(col, row) == 1)
               mGridControlTarget->GetGridController()->SetLight(col, row, GridColor::kGridColor1Bright);
            else if (col == mGrid->GetHighlightCol(gTime+gBufferSizeMs+TheTransport->GetEventLookaheadMs()))
               mGridControlTarget->GetGridController()->SetLight(col, row, GridColor::kGridColor1Dim);
            else
               mGridControlTarget->GetGridController()->SetLight(col, row, GridColor::kGridColorOff);
         }
      }
   }
}

void RadioSequencer::Step(double time, int pulseFlags)
{
   int length = mLength;
   if (length <= 0)
      length = 1;

   int direction = 1;
   if (pulseFlags & kPulseFlag_Backward)
      direction = -1;
   if (pulseFlags & kPulseFlag_Repeat)
      direction = 0;

   mStep = (mStep + direction + length) % length;

   if (pulseFlags & kPulseFlag_Reset)
      mStep = 0;
   else if (pulseFlags & kPulseFlag_Random)
      mStep = gRandom() % length;

   if (!mHasExternalPulseSource || (pulseFlags & kPulseFlag_SyncToTransport))
   {
      mStep = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, length);
   }

   if (pulseFlags & kPulseFlag_Align)
   {
      int stepsPerMeasure = TheTransport->GetStepsPerMeasure(this);
      int numMeasures = ceil(float(length) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure(time) % numMeasures;
      int step = ((TheTransport->GetQuantized(time, mTransportListenerInfo) % stepsPerMeasure) + measure * stepsPerMeasure) % length;
      mStep = step;
   }

   if (!mEnabled)
      return;

   mGrid->SetHighlightCol(time, mStep);

   std::vector<IUIControl*> controlsToEnable;
   for (int i = 0; i < mControlCables.size(); ++i)
   {
      IUIControl* uicontrol = nullptr;
      if (mControlCables[i]->GetTarget())
         uicontrol = dynamic_cast<IUIControl*>(mControlCables[i]->GetTarget());
      if (uicontrol)
      {
         if (mGrid->GetVal(mStep, i) > 0)
            controlsToEnable.push_back(uicontrol);
         else
            uicontrol->SetValue(0);
      }
   }

   for (auto* control : controlsToEnable)
      control->SetValue(1);

   UpdateGridLights();
}

void RadioSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;

   Step(time, flags);
}

void RadioSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 0);
}

void RadioSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mHasExternalPulseSource = true;
      mStep = pitch % std::max(1, mLength);
      Step(time, kPulseFlag_Repeat);
   }
}

void RadioSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGrid->Draw();
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mGridControlTarget->Draw();
   
   for (int i=0; i<mControlCables.size(); ++i)
   {
      mControlCables[i]->SetManualPosition(GetRect(true).width, mGrid->GetPosition(true).y+(mGrid->GetHeight()/mGrid->GetRows())*(i+.5f));
   }
}

void RadioSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x, y, right);
}

void RadioSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool RadioSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
}

void RadioSequencer::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
   }
}

void RadioSequencer::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
}

void RadioSequencer::SyncControlCablesToGrid()
{
   if (mGrid->GetRows() == mControlCables.size())
      return;  //nothing to do
   
   if (mGrid->GetRows() > mControlCables.size())
   {
      int oldSize = (int)mControlCables.size();
      mControlCables.resize(mGrid->GetRows());
      for (int i=oldSize; i<mControlCables.size(); ++i)
      {
         mControlCables[i] = new PatchCableSource(this, kConnectionType_Modulator);
         mControlCables[i]->SetOverrideCableDir(ofVec2f(1,0));
         //mControlCables[i]->SetColor(GetRowColor(i));
         AddPatchCableSource(mControlCables[i]);
      }
   }
   else
   {
      for (int i=mGrid->GetRows(); i<mControlCables.size(); ++i)
         RemovePatchCableSource(mControlCables[i]);
      mControlCables.resize(mGrid->GetRows());
   }
}

void RadioSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
}

void RadioSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLengthSlider)
   {
      mGrid->SetGrid(mLength, mGrid->GetRows());
      if (mLength > oldVal)
      {
         //slice the loop into the nearest power of 2 and loop new steps from there
         int oldLengthPow2 = std::max(1, MathUtils::HighestPow2(oldVal));
         for (int i = oldVal; i < mLength; ++i)
         {
            int loopedFrom = i % oldLengthPow2;
            for (int row = 0; row < mGrid->GetRows(); ++row)
               mGrid->SetVal(i, row, mGrid->GetVal(i % oldLengthPow2, row));
         }
      }
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 30;
}

void RadioSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void RadioSequencer::Resize(float w, float h)
{
   w = MAX(w - extraW, 200);
   h = MAX(h - extraH, 170);
   SetGridSize(w,h);
}

void RadioSequencer::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
}

void RadioSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}

void RadioSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{  
   mModuleSaveData.LoadBool("one_per_column_mode", moduleInfo, true);

   SetUpFromSaveData();
}

void RadioSequencer::SetUpFromSaveData()
{
   mGrid->SetSingleColumnMode(mModuleSaveData.GetBool("one_per_column_mode"));
}

namespace
{
   const int kSaveStateRev = 1;
}

void RadioSequencer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   IDrawableModule::SaveState(out);
   
   out << (int)mControlCables.size();
   for (auto cable : mControlCables)
   {
      std::string path = "";
      if (cable->GetTarget())
         path = cable->GetTarget()->Path();
      out << path;
   }
   
   mGrid->SaveState(out);
   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
}

void RadioSequencer::LoadState(FileStreamIn& in)
{
   mLoadRev = -1;

   if (ModuleContainer::sFileSaveStateRev >= 422)
   {
      in >> mLoadRev;
      LoadStateValidate(mLoadRev <= kSaveStateRev);
   }

   IDrawableModule::LoadState(in);

   if (ModuleContainer::sFileSaveStateRev <= 421)
   {
      in >> mLoadRev;
      LoadStateValidate(mLoadRev <= kSaveStateRev);
   }
   
   int size;
   in >> size;
   mControlCables.resize(size);
   for (auto cable : mControlCables)
   {
      std::string path;
      in >> path;
      cable->SetTarget(TheSynth->FindUIControl(path));
   }
   
   mGrid->LoadState(in);

   if (mLoadRev >= 1)
   {
      float width, height;
      in >> width;
      in >> height;
      mGrid->SetDimensions(width, height);
   }

   if (mLoadRev == 0)
   {
      //port old data
      float len = 0;
      if (mOldLengthStr == "4")   len = 4;
      if (mOldLengthStr == "6")   len = 6;
      if (mOldLengthStr == "8")   len = 8;
      if (mOldLengthStr == "16")  len = 16;
      if (mOldLengthStr == "32")  len = 32;
      if (mOldLengthStr == "64")  len = 64;
      if (mOldLengthStr == "128") len = 128;

      mLength = int(len * TheTransport->CountInStandardMeasure(mInterval));
      int min, max;
      mLengthSlider->GetRange(min, max);
      if (mLength > max)
         mLengthSlider->SetExtents(min, mLength);
   }
}

bool RadioSequencer::LoadOldControl(FileStreamIn& in, std::string& oldName)
{
   if (mLoadRev < 1)
   {
      if (oldName == "length")
      {
         //load dropdown string
         int dropdownRev;
         in >> dropdownRev;
         in >> mOldLengthStr;
         return true;
      }
   }
   return false;
}
