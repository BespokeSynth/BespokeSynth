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
//  EventCanvas.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#include "EventCanvas.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "CanvasControls.h"
#include "Scale.h"
#include "CanvasElement.h"
#include "Profiler.h"
#include "PatchCableSource.h"
#include "CanvasScrollbar.h"

EventCanvas::EventCanvas()
{
   mRowColors.push_back(ofColor::red);
   mRowColors.push_back(ofColor::green);
   mRowColors.push_back(ofColor::blue);
   mRowColors.push_back(ofColor::orange);
   mRowColors.push_back(ofColor::purple);
   mRowColors.push_back(ofColor::yellow);

   for (auto& color : mRowColors)
   {
      color.setBrightness(color.getBrightness() * .8f);
      color.setSaturation(color.getSaturation() * .7f);
   }

   mRowConnections.resize(kMaxEventRows);
}

void EventCanvas::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

void EventCanvas::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mQuantizeButton = new ClickButton(this, "quantize", 160, 5);
   mNumMeasuresEntry = new TextEntry(this, "measures", 5, 5, 3, &mNumMeasures, 1, 999);
   mIntervalSelector = new DropdownList(this, "interval", 110, 5, (int*)(&mInterval));
   mRecordCheckbox = new Checkbox(this, "record", 220, 5, &mRecord);

   mNumMeasuresEntry->DrawLabel(true);

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);

   mCanvas = new Canvas(this, 5, 45, 390, 100, L(length, 1), L(rows, 8), L(cols, 16), &(EventCanvasElement::Create));
   AddUIControl(mCanvas);
   mCanvasControls = new CanvasControls();
   mCanvasControls->SetCanvas(mCanvas);
   mCanvasControls->CreateUIControls();
   mCanvasControls->AllowDragModeSelection(false);
   AddChild(mCanvasControls);
   UpdateNumColumns();

   mCanvas->SetListener(this);
   mCanvas->SetDragMode(Canvas::kDragHorizontal);
   mCanvas->SetNumVisibleRows(8);

   mCanvasScrollbarHorizontal = new CanvasScrollbar(mCanvas, "scrollh", CanvasScrollbar::Style::kHorizontal);
   AddUIControl(mCanvasScrollbarHorizontal);

   SyncControlCablesToCanvas();
}

EventCanvas::~EventCanvas()
{
   mCanvas->SetListener(nullptr);
   TheTransport->RemoveAudioPoller(this);
}

void EventCanvas::OnTransportAdvanced(float amount)
{
   PROFILER(EventCanvas);

   if (mCanvas == nullptr)
      return;

   //look ahead two buffers so that we set things slightly early, so we'll do things like catch the downbeat right after enabling a sequencer, etc.
   double lookaheadMsAmount = gBufferSizeMs * 2;
   double lookaheadTime = gTime + lookaheadMsAmount;
   double lookaheadPos = DoubleWrap(((TheTransport->GetMeasure(lookaheadTime) % mNumMeasures) + TheTransport->GetMeasurePos(lookaheadTime)) / mNumMeasures, 1);

   mCanvas->SetCursorPos(lookaheadPos);
   mPosition = lookaheadPos;
   double bufferOffsetAmount = gBufferSizeMs / TheTransport->MsPerBar() / mNumMeasures;
   mPreviousPosition = std::min(mPreviousPosition, lookaheadPos - bufferOffsetAmount);

   if (!mEnabled)
      return;

   for (auto* canvasElement : mCanvas->GetElements())
   {
      float elementStart = canvasElement->GetStart();
      bool startPassed = (lookaheadPos >= elementStart && mPreviousPosition < elementStart);
      float elementEnd = canvasElement->GetEnd();
      if (elementEnd > mCanvas->GetLength())
         elementEnd = FloatWrap(elementEnd, mCanvas->GetLength());
      bool endPassed = (lookaheadPos >= elementEnd && mPreviousPosition < elementEnd);
      if (startPassed || endPassed)
      {
         EventCanvasElement* element = static_cast<EventCanvasElement*>(canvasElement);
         if (lookaheadPos > elementEnd)
         {
            if (startPassed)
               element->Trigger(GetTriggerTime(lookaheadTime, lookaheadPos, elementStart));
            if (endPassed)
               element->TriggerEnd(GetTriggerTime(lookaheadTime, lookaheadPos, elementEnd));
         }
         else
         {
            if (endPassed)
               element->TriggerEnd(GetTriggerTime(lookaheadTime, lookaheadPos, elementEnd));
            if (startPassed)
               element->Trigger(GetTriggerTime(lookaheadTime, lookaheadPos, elementStart));
         }

         IUIControl* control = mRowConnections[element->mRow].mUIControl;
         if (control)
            mRowConnections[element->mRow].mLastValue = control->GetValue();
      }
   }

   for (int i = 0; i < mControlCables.size(); ++i)
   {
      if (mRowConnections[i].mUIControl)
      {
         float value = mRowConnections[i].mUIControl->GetValue();

         if (mRecord && mRowConnections[i].mLastValue != value)
         {
            float colPos = lookaheadPos * mCanvas->GetNumCols();
            int col = int(colPos + .5f);
            EventCanvasElement* element = new EventCanvasElement(mCanvas, col, i, colPos - col);
            element->SetUIControl(mRowConnections[i].mUIControl);
            element->SetValue(value);
            mCanvas->AddElement(element);
         }

         mRowConnections[i].mLastValue = value;
      }
   }

   mPreviousPosition = lookaheadPos;
}

double EventCanvas::GetTriggerTime(double lookaheadTime, double lookaheadPos, float eventPos)
{
   double cursorAdvanceSinceEvent = lookaheadPos - eventPos;
   if (cursorAdvanceSinceEvent < 0)
      cursorAdvanceSinceEvent += 1;
   double time = lookaheadTime - cursorAdvanceSinceEvent * TheTransport->MsPerBar() * mNumMeasures;
   if (time < gTime)
      time = gTime;
   return time;
}

void EventCanvas::UpdateNumColumns()
{
   if (TheTransport->GetDuration(mInterval) < TheTransport->GetDuration(kInterval_1n))
   {
      mCanvas->RescaleNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
      mCanvas->SetMajorColumnInterval(TheTransport->CountInStandardMeasure(mInterval) / 4);
   }
   else
   {
      mCanvas->RescaleNumCols(TheTransport->GetDuration(kInterval_1n) / TheTransport->GetDuration(mInterval) * mNumMeasures);
      mCanvas->SetMajorColumnInterval(-1);
   }
}

IUIControl* EventCanvas::GetUIControlForRow(int row)
{
   return mRowConnections[row].mUIControl;
}

ofColor EventCanvas::GetRowColor(int row) const
{
   return mRowColors[row % mRowColors.size()];
}

void EventCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
   }
}
void EventCanvas::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (int i = 0; i < mControlCables.size(); ++i)
   {
      mRowConnections[i].mUIControl = dynamic_cast<IUIControl*>(mControlCables[i]->GetTarget());
      if (mRowConnections[i].mUIControl)
         mRowConnections[i].mLastValue = mRowConnections[i].mUIControl->GetValue();
   }

   for (auto canvasElement : mCanvas->GetElements())
   {
      EventCanvasElement* element = static_cast<EventCanvasElement*>(canvasElement);
      int row = element->mRow;
      element->SetUIControl(GetUIControlForRow(row));
   }
}

void EventCanvas::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofFill();
   for (int i = 0; i < mCanvas->GetNumVisibleRows(); ++i)
   {
      ofColor color = GetRowColor(i + mCanvas->GetRowOffset());
      color.a = 50;
      ofSetColor(color);

      float boxHeight = (float(mCanvas->GetHeight()) / mCanvas->GetNumVisibleRows());
      float y = mCanvas->GetPosition(true).y + i * boxHeight;
      ofRect(mCanvas->GetPosition(true).x, y, mCanvas->GetWidth(), boxHeight);
   }
   ofPopStyle();

   ofPushStyle();
   ofSetColor(128, 128, 128);
   mCanvas->Draw();
   ofPopStyle();

   mCanvasScrollbarHorizontal->Draw();

   mCanvasControls->Draw();
   mQuantizeButton->Draw();
   mNumMeasuresEntry->Draw();
   mIntervalSelector->Draw();
   mRecordCheckbox->Draw();

   ofRectangle canvasRect = mCanvas->GetRect(true);
   for (int i = 0; i < mControlCables.size(); ++i)
   {
      if (mCanvas->IsRowVisible(i))
      {
         mControlCables[i]->SetManualPosition(GetRect().width, canvasRect.y + (canvasRect.height / mCanvas->GetNumVisibleRows()) * (i - mCanvas->GetRowOffset() + .5f));
         mControlCables[i]->SetEnabled(true);
      }
      else
      {
         mControlCables[i]->SetEnabled(false);
      }
   }
}

void EventCanvas::SyncControlCablesToCanvas()
{
   if (mCanvas->GetNumRows() == mControlCables.size())
      return; //nothing to do

   if (mCanvas->GetNumRows() > mControlCables.size())
   {
      int oldSize = (int)mControlCables.size();
      mControlCables.resize(mCanvas->GetNumRows());
      for (int i = oldSize; i < mControlCables.size(); ++i)
      {
         mControlCables[i] = new PatchCableSource(this, kConnectionType_UIControl);
         mControlCables[i]->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
         mControlCables[i]->SetColor(GetRowColor(i));
         AddPatchCableSource(mControlCables[i]);
      }
   }
   else
   {
      for (int i = mCanvas->GetNumRows(); i < mControlCables.size(); ++i)
         RemovePatchCableSource(mControlCables[i]);
      mControlCables.resize(mCanvas->GetNumRows());
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 150;
}

void EventCanvas::Resize(float w, float h)
{
   w = MAX(w - extraW, 390);
   h = MAX(h - extraH, 100);
   mCanvas->SetDimensions(w, h);
}

void EventCanvas::GetModuleDimensions(float& width, float& height)
{
   width = mCanvas->GetWidth() + extraW;
   height = mCanvas->GetHeight() + extraH;
}

void EventCanvas::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      mPreviousPosition = mPosition + .001f;
   }
}

void EventCanvas::ButtonClicked(ClickButton* button, double time)
{
   if (button == mQuantizeButton)
   {
      bool anyHighlighted = false;
      for (auto* element : mCanvas->GetElements())
      {
         if (element->GetHighlighted())
         {
            anyHighlighted = true;
            break;
         }
      }
      for (auto* element : mCanvas->GetElements())
      {
         if (anyHighlighted == false || element->GetHighlighted())
         {
            element->mCol = int(element->mCol + element->mOffset + .5f) % mCanvas->GetNumCols();
            element->mOffset = 0;
         }
      }
   }
}

void EventCanvas::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void EventCanvas::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void EventCanvas::TextEntryComplete(TextEntry* entry)
{
   if (entry == mNumMeasuresEntry)
   {
      mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   }
}

void EventCanvas::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      UpdateNumColumns();
   }
}

void EventCanvas::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadFloat("canvaswidth", moduleInfo, 390, 390, 99999, K(isTextField));
   mModuleSaveData.LoadFloat("canvasheight", moduleInfo, 100, 40, 99999, K(isTextField));
   mModuleSaveData.LoadInt("num_rows", moduleInfo, 8, 1, 999, K(isTextField));

   SetUpFromSaveData();
}

void EventCanvas::SetUpFromSaveData()
{
   mCanvas->SetDimensions(mModuleSaveData.GetFloat("canvaswidth"), mModuleSaveData.GetFloat("canvasheight"));
   assert(mModuleSaveData.GetInt("num_rows") <= kMaxEventRows);
   mCanvas->SetNumRows(mModuleSaveData.GetInt("num_rows"));
   SyncControlCablesToCanvas();
}

void EventCanvas::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["canvaswidth"] = mCanvas->GetWidth();
   moduleInfo["canvasheight"] = mCanvas->GetHeight();
}

void EventCanvas::SaveState(FileStreamOut& out)
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

   mCanvas->SaveState(out);
}

void EventCanvas::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   int size;
   in >> size;
   mControlCables.resize(size);
   for (auto cable : mControlCables)
   {
      std::string path;
      in >> path;
      cable->SetTarget(TheSynth->FindUIControl(path));
   }

   mCanvas->LoadState(in);
}

std::vector<IUIControl*> EventCanvas::ControlsToIgnoreInSaveState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mCanvas);
   return ignore;
}
