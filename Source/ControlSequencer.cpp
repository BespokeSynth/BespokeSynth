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
//  ControlSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 8/27/15.
//
//

#include "ControlSequencer.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"
#include "MathUtils.h"

std::list<ControlSequencer*> ControlSequencer::sControlSequencers;

ControlSequencer::ControlSequencer()
{
   sControlSequencers.push_back(this);
}

void ControlSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), false);
}

ControlSequencer::~ControlSequencer()
{
   TheTransport->RemoveListener(this);
   
   sControlSequencers.remove(this);
}

void ControlSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   int width, height;
   UIBLOCK(3, 3, 200);
   INTSLIDER(mLengthSlider, "length", &mLength, 1, 64); UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40); UIBLOCK_SHIFTRIGHT();
   BUTTON(mRandomize, "random");
   ENDUIBLOCK(width, height);

   mGrid = new UIGrid(5, 25, mRandomize->GetRect().getMaxX() - 6, 40, 16, 1, this);
   
   mControlCable = new PatchCableSource(this, kConnectionType_Modulator);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);
   
   mGrid->SetGridMode(UIGrid::kMultislider);
   mGrid->SetHighlightCol(gTime, -1);
   mGrid->SetMajorColSize(4);
   mGrid->SetListener(this);
   
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("2", kInterval_2);
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
}

void ControlSequencer::Poll()
{
}

void ControlSequencer::Step(double time, int pulseFlags)
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
   
   mGrid->SetHighlightCol(time, mStep);
   
   if (mUIControl && mEnabled)
   {
      mUIControl->SetFromMidiCC(mGrid->GetVal(mStep, 0), true);
      mControlCable->AddHistoryEvent(gTime, true);
      mControlCable->AddHistoryEvent(gTime + 15, false);
   }
}

void ControlSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;

   Step(time, flags);
}

void ControlSequencer::OnTimeEvent(double time)
{
   if (!mHasExternalPulseSource)
      Step(time, 0);
}

void ControlSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
   {
      mHasExternalPulseSource = true;
      mStep = pitch % std::max(1, mLength);
      Step(time, kPulseFlag_Repeat);
   }
}

void ControlSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGrid->Draw();
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mRandomize->Draw();
   
   int currentHover = mGrid->CurrentHover();
   if (currentHover != -1 && mUIControl)
   {
      ofPushStyle();
      ofSetColor(ofColor::grey);
      float val = mGrid->GetVal(currentHover % mGrid->GetCols(), currentHover / mGrid->GetCols());
      DrawTextNormal(mUIControl->GetDisplayValue(mUIControl->GetValueForMidiCC(val)), mGrid->GetPosition(true).x, mGrid->GetPosition(true).y+12);
      ofPopStyle();
   }
}

void ControlSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x, y, right);
}

void ControlSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool ControlSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
}

void ControlSequencer::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      int numValues = mUIControl ? mUIControl->GetNumValues() : 0;
      if (numValues > 1)
      {
         for (int i=0; i<mGrid->GetCols(); ++i)
         {
            float val = mGrid->GetVal(i, 0);
            val = int((val * (numValues-1)) + .5f) / float(numValues-1);   //quantize to match the number of allowed values
            mGrid->SetVal(i, 0, val);
         }
      }
   }
}

void ControlSequencer::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (mControlCable->GetPatchCables().empty() == false)
      mUIControl = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[0]->GetTarget());
   else
      mUIControl = nullptr;
   if (mUIControl)
   {
      for (int i=0; i<mGrid->GetCols(); ++i)
         mGrid->SetVal(i, 0, mUIControl->GetMidiValue());
   }
}

void ControlSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mLengthSlider)
   {
      mGrid->SetGrid(mLength, 1);
      if (mLength > oldVal)
      {
         //slice the loop into the nearest power of 2 and loop new steps from there
         int oldLengthPow2 = std::max(1, MathUtils::HighestPow2(oldVal));
         for (int i = oldVal; i < mLength; ++i)
         {
            int loopedFrom = i % oldLengthPow2;
            mGrid->SetVal(i, 0, mGrid->GetVal(i % oldLengthPow2, 0));
         }
      }
   }
}

void ControlSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval =  mInterval;
   }
}

void ControlSequencer::ButtonClicked(ClickButton* button)
{
   if (button == mRandomize)
   {
      for (int i=0; i<mGrid->GetCols(); ++i)
         mGrid->SetVal(i, 0, ofRandom(1));
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 30;
}

void ControlSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void ControlSequencer::Resize(float w, float h)
{
   w = MAX(w - extraW, 130);
   h = MAX(h - extraH, 40);
   SetGridSize(w,h);
}

void ControlSequencer::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
}

void ControlSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["uicontrol"] = mUIControl ? mUIControl->Path() : "";
}

void ControlSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol", moduleInfo);
   
   SetUpFromSaveData();
}

void ControlSequencer::SetUpFromSaveData()
{
   std::string controlPath = mModuleSaveData.GetString("uicontrol");
   if (!controlPath.empty())
   {
      mUIControl = TheSynth->FindUIControl(controlPath);
      if (mUIControl)
         mControlCable->SetTarget(mUIControl);
   }
   else
   {
      mUIControl = nullptr;
   }
}

namespace
{
   const int kSaveStateRev = 1;
}

void ControlSequencer::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   IDrawableModule::SaveState(out);
   
   mGrid->SaveState(out);
   out << mGrid->GetWidth();
   out << mGrid->GetHeight();
}

void ControlSequencer::LoadState(FileStreamIn& in)
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
      if (mOldLengthStr == "4n")  len = .25f;
      if (mOldLengthStr == "2n")  len = .5f;
      if (mOldLengthStr == "1")   len = 1;
      if (mOldLengthStr == "2")   len = 2;
      if (mOldLengthStr == "3")   len = 3;
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

bool ControlSequencer::LoadOldControl(FileStreamIn& in, std::string& oldName)
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
