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
//  TableModulator.cpp
//  Bespoke
//

#include "TableModulator.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "FileStream.h"

namespace
{
   const float kColX = 4;
   const float kColW = 100;
   const float kGridX = 118;
   const float kGridY = 40;
   const float kGridH = 120;
}

TableModulator::TableModulator()
{
   for (int i = 0; i < kMaxSteps; ++i)
      mTable[i] = (i % 8) / 7.0f; //a gentle repeating ramp so it visibly steps out of the box
}

TableModulator::~TableModulator()
{
   TheTransport->RemoveListener(this);
}

void TableModulator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 3;
   mIntervalSelector = new DropdownList(this, "rate", kColX + 28, y, (int*)(&mInterval), 60);
   y += 17;
   mNumStepsSlider = new IntSlider(this, "steps", kColX, y, (int)kColW, 14, &mNumSteps, 1, kMaxSteps);
   y += 16;
   mMultiplySlider = new IntSlider(this, "multiply", kColX, y, (int)kColW, 14, &mMultiply, 1, 24);
   y += 16;
   mAddSlider = new IntSlider(this, "add", kColX, y, (int)kColW, 14, &mAdd, 0, 100);
   y += 16;
   mSubtractSlider = new IntSlider(this, "subtract", kColX, y, (int)kColW, 14, &mSubtract, 0, 100);
   y += 16;
   mDivideSlider = new IntSlider(this, "divide", kColX, y, (int)kColW, 14, &mDivide, 1, 24);
   y += 16;
   mLowSlider = new IntSlider(this, "low", kColX, y, (int)kColW, 14, &mOutLow, 0, 127);
   y += 16;
   mHighSlider = new IntSlider(this, "high", kColX, y, (int)kColW, 14, &mOutHigh, 0, 127);
   y += 16;
   mRandomizeButton = new ClickButton(this, "randomize", kColX, y);

   //longer-than-a-bar divisions first, then the note values
   mIntervalSelector->AddLabel("16", kInterval_16);
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

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);

   UpdateTransportListener();
}

void TableModulator::UpdateTransportListener()
{
   TransportListenerInfo* info = TheTransport->GetListenerInfo(this);
   if (info != nullptr)
      info->mInterval = mInterval;
   else
      TheTransport->AddListener(this, mInterval, OffsetInfo(0, false), false);
}

int TableModulator::ComputeOutput(int step) const
{
   int lo = MIN(mOutLow, mOutHigh);
   int hi = MAX(mOutLow, mOutHigh);
   int s = ofClamp(step, 0, kMaxSteps - 1);
   //the drawn bar height (0..1) maps across the [low,high] range - so the range actually rescales
   //the output rather than just clamping it
   float frac = ofClamp(mTable[s], 0.0f, 1.0f);
   int mapped = lo + (int)roundf(frac * (hi - lo));
   int div = MAX(1, mDivide);
   int out = (mapped * mMultiply + mAdd - mSubtract) / div;
   return ofClamp(out, lo, hi);
}

void TableModulator::OnTimeEvent(double time)
{
   if (!mEnabled || mNumSteps <= 0)
      return;
   mCurStep = (mCurStep + 1) % mNumSteps;
   mCurrentOutput = ComputeOutput(mCurStep);
}

float TableModulator::Value(int samplesIn)
{
   //recompute live so table / range / math edits show up immediately (not only on the next step)
   mCurrentOutput = ComputeOutput(mCurStep);
   return ofClamp((float)mCurrentOutput, GetMin(), GetMax());
}

void TableModulator::Randomize()
{
   for (int i = 0; i < kMaxSteps; ++i)
      mTable[i] = ofRandom(0.0f, 1.0f);
}

void TableModulator::EditCellAt(float localX, float localY)
{
   float gridW = mWidth - kGridX - 6;
   if (gridW <= 0 || mNumSteps <= 0)
      return;
   float colW = gridW / mNumSteps;
   int col = (int)((localX - kGridX) / colW);
   col = ofClamp(col, 0, mNumSteps - 1);
   float frac = 1.0f - (localY - kGridY) / kGridH; //top = 1.0, bottom = 0.0
   mTable[col] = ofClamp(frac, 0.0f, 1.0f);
}

void TableModulator::OnClicked(float x, float y, bool right)
{
   float gridW = mWidth - kGridX - 6;
   if (!right && x >= kGridX && x <= kGridX + gridW && y >= kGridY && y <= kGridY + kGridH)
   {
      EditCellAt(x, y);
      mEditingGrid = true;
      mDragOffX = TheSynth->GetMouseX(GetOwningContainer()) - x;
      mDragOffY = TheSynth->GetMouseY(GetOwningContainer()) - y;
      return;
   }
   IDrawableModule::OnClicked(x, y, right);
}

void TableModulator::Poll()
{
   IModulator::Poll();

   if (mEditingGrid)
   {
      if (TheSynth->IsMouseButtonHeld(1))
      {
         float localX = TheSynth->GetMouseX(GetOwningContainer()) - mDragOffX;
         float localY = TheSynth->GetMouseY(GetOwningContainer()) - mDragOffY;
         EditCellAt(localX, localY);
      }
      else
      {
         mEditingGrid = false;
      }
   }
}

void TableModulator::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();
   if (GetSliderTarget() && fromUserClick)
      InitializeRange((float)mCurrentOutput, (float)MIN(mOutLow, mOutHigh), (float)MAX(mOutLow, mOutHigh), GetSliderTarget()->GetMode());
}

void TableModulator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("rate", kColX, 14);
   mIntervalSelector->Draw();
   mNumStepsSlider->Draw();
   mMultiplySlider->Draw();
   mAddSlider->Draw();
   mSubtractSlider->Draw();
   mDivideSlider->Draw();
   mLowSlider->Draw();
   mHighSlider->Draw();
   mRandomizeButton->Draw();

   //big output display
   ofPushStyle();
   ofSetColor(160, 220, 255);
   DrawTextNormal("out: " + ofToString(mCurrentOutput), kGridX, 24, 18);
   ofPopStyle();

   //the value table, drawn as bars you can click/drag to set (1-100)
   float gridW = mWidth - kGridX - 6;
   ofPushStyle();
   ofFill();
   ofSetColor(12, 14, 20, 220);
   ofRect(kGridX, kGridY, gridW, kGridH);

   if (mNumSteps > 0)
   {
      float colW = gridW / mNumSteps;
      for (int i = 0; i < mNumSteps; ++i)
      {
         float v = ofClamp(mTable[i], 0.0f, 1.0f);
         float bh = v * kGridH;
         float bx = kGridX + i * colW;
         bool cur = (i == mCurStep);
         ofFill();
         if (cur)
            ofSetColor(120, 230, 150, 230);
         else
            ofSetColor(70, 130, 200, 200);
         ofRect(bx + 1, kGridY + kGridH - bh, MAX(1.0f, colW - 2), bh);
         //value label if columns are wide enough
         if (colW > 16)
         {
            ofSetColor(230, 235, 245, 200);
            DrawTextNormal(ofToString(ComputeOutput(i)), (int)(bx + 2), (int)(kGridY + kGridH - 3), 9);
         }
      }
   }
   ofNoFill();
   ofSetColor(180, 185, 200, 120);
   ofRect(kGridX, kGridY, gridW, kGridH);
   ofPopStyle();
}

void TableModulator::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumStepsSlider && mCurStep >= mNumSteps)
      mCurStep = -1;
}

void TableModulator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
      UpdateTransportListener();
}

void TableModulator::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizeButton)
      Randomize();
}

void TableModulator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void TableModulator::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void TableModulator::SetUpFromSaveData()
{
}

void TableModulator::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);
   for (int i = 0; i < kMaxSteps; ++i)
      out << mTable[i];
}

void TableModulator::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);
   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());
   for (int i = 0; i < kMaxSteps; ++i)
      in >> mTable[i];
}
