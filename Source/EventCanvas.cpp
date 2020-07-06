//
//  EventCanvas.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/28/15.
//
//

#include "EventCanvas.h"
#include "IAudioSource.h"
#include "SynthGlobals.h"
#include "DrumPlayer.h"
#include "ModularSynth.h"
#include "CanvasControls.h"
#include "Scale.h"
#include "CanvasElement.h"
#include "Profiler.h"
#include "PatchCableSource.h"

EventCanvas::EventCanvas()
: mCanvas(nullptr)
, mCanvasControls(nullptr)
, mNumMeasuresEntry(nullptr)
, mNumMeasures(1)
, mQuantizeButton(nullptr)
, mInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mScrollPartial(0)
, mPosition(0)
, mPositionSlider(nullptr)
, mRecord(false)
, mRecordCheckbox(nullptr)
, mPreviousPosition(0)
{
   TheTransport->AddAudioPoller(this);
   SetEnabled(true);
   
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

void EventCanvas::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mQuantizeButton = new ClickButton(this,"quantize",160,5);
   mNumMeasuresEntry = new TextEntry(this,"measures",63,5,3,&mNumMeasures,1,999);
   mIntervalSelector = new DropdownList(this,"interval",110,5,(int*)(&mInterval));
   mPositionSlider = new FloatSlider(this,"position",5,31,390,15,&mPosition,0,1);
   mRecordCheckbox = new Checkbox(this,"record",220,5,&mRecord);
   
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
   
   mCanvas = new Canvas(this, 5, 45, 390, 100, L(length,1), L(rows,8), L(cols,16), &(EventCanvasElement::Create));
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
   
   //look ahead one buffer so that we set things slightly early, so we'll do things like catch the downbeat right after enabling a sequencer, etc.
   float posOffset = gBufferSize * gInvSampleRateMs / TheTransport->MsPerBar() / mNumMeasures;
   
   float curPos = ((TheTransport->GetMeasure(gTime) % mNumMeasures) + TheTransport->GetMeasurePos(gTime)) / mNumMeasures + posOffset;
   FloatWrap(curPos, 1);
   
   mCanvas->SetCursorPos(curPos);
   mPosition = curPos;
   
   if (!mEnabled)
      return;
   
   for (auto* canvasElement : mCanvas->GetElements())
   {
      float elementStart = canvasElement->GetStart();
      bool startPassed = (elementStart > mPreviousPosition && elementStart <= curPos) ||
                         (curPos < mPreviousPosition && (elementStart > mPreviousPosition || elementStart <= curPos));
      float elementEnd = canvasElement->GetEnd();
      FloatWrap(elementEnd, mCanvas->GetLength());
      bool endPassed = (elementEnd > mPreviousPosition && elementEnd <= curPos) ||
                       (curPos < mPreviousPosition && (elementEnd > mPreviousPosition || elementEnd <= curPos));
      if (startPassed || endPassed)
      {
         EventCanvasElement* element = static_cast<EventCanvasElement*>(canvasElement);
         if (curPos > elementEnd)
         {
            if (startPassed)
               element->Trigger();
            if (endPassed)
               element->TriggerEnd();
         }
         else
         {
            if (endPassed)
               element->TriggerEnd();
            if (startPassed)
               element->Trigger();
         }
         
         IUIControl* control = mRowConnections[element->mRow].mUIControl;
         if (control)
            mRowConnections[element->mRow].mLastValue = control->GetValue();
      }
   }
   
   for (int i=0; i<mControlCables.size(); ++i)
   {
      if (mRowConnections[i].mUIControl)
      {
         float value = mRowConnections[i].mUIControl->GetValue();
         
         if (mRecord && mRowConnections[i].mLastValue != value)
         {
            float colPos = curPos * mCanvas->GetNumCols();
            int col = int(colPos+.5f);
            EventCanvasElement* element = new EventCanvasElement(mCanvas, col, i, colPos - col);
            element->SetUIControl(mRowConnections[i].mUIControl);
            element->SetValue(value);
            mCanvas->AddElement(element);
         }
      
         mRowConnections[i].mLastValue = value;
      }
   }
   
   mPreviousPosition = curPos;
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
      mCanvas->RescaleNumCols(TheTransport->GetDuration(kInterval_1n)/TheTransport->GetDuration(mInterval) * mNumMeasures);
      mCanvas->SetMajorColumnInterval(-1);
   }
}

IUIControl* EventCanvas::GetUIControlForRow(int row)
{
   return mRowConnections[row].mUIControl;
}

ofColor EventCanvas::GetRowColor(int row) const
{
   return mRowColors[row%mRowColors.size()];
}

void EventCanvas::CanvasUpdated(Canvas* canvas)
{
   if (canvas == mCanvas)
   {
      
   }
}
void EventCanvas::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (int i=0; i<mControlCables.size(); ++i)
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
   for (int i=0;i<mCanvas->GetNumVisibleRows();++i)
   {
      ofColor color = GetRowColor(i+mCanvas->GetRowOffset());
      color.a = 50;
      ofSetColor(color);
      
      float boxHeight = (float(mCanvas->GetGridHeight())/mCanvas->GetNumVisibleRows());
      float y = mCanvas->GetPosition(true).y + i*boxHeight;
      ofRect(mCanvas->GetPosition(true).x,y,mCanvas->GetGridWidth(),boxHeight);
   }
   ofPopStyle();
   
   ofPushStyle();
   ofSetColor(128,128,128);
   mCanvas->Draw();
   ofPopStyle();
   
   mPositionSlider->SetExtents(mCanvas->mStart, mCanvas->mEnd);
   
   mCanvasControls->Draw();
   mQuantizeButton->Draw();
   mNumMeasuresEntry->Draw();
   mIntervalSelector->Draw();
   mPositionSlider->Draw();
   mRecordCheckbox->Draw();
   
   ofRectangle canvasRect = mCanvas->GetRect(true);
   for (int i=0; i<mControlCables.size(); ++i)
   {
      if (mCanvas->IsRowVisible(i))
      {
         mControlCables[i]->SetManualPosition(GetRect().width, canvasRect.y + (canvasRect.height/mCanvas->GetNumVisibleRows()) * (i-mCanvas->GetRowOffset()+.5f));
         mControlCables[i]->SetEnabled(true);
      }
      else
      {
         mControlCables[i]->SetEnabled(false);
      }
   }
}

bool EventCanvas::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   /*if (x >= mCanvas->GetPosition(true).x && y >= mCanvas->GetPosition(true).y &&
       x < mCanvas->GetPosition(true).x + mCanvas->GetWidth() && y < mCanvas->GetPosition(true).y + mCanvas->GetHeight())
   {
      mScrollPartial += -scrollY;
      int scrollWhole = int(mScrollPartial);
      mScrollPartial -= scrollWhole;
      mCanvas->SetRowOffset(mCanvas->GetRowOffset()+scrollWhole);
   }*/
   
   float canvasX,canvasY;
   mCanvas->GetPosition(canvasX, canvasY, true);
   ofVec2f canvasPos = ofVec2f(ofMap(x, canvasX, canvasX+mCanvas->GetWidth(), 0, 1),
                               ofMap(y, canvasY, canvasY+mCanvas->GetHeight(), 0, 1));
   if (IsInUnitBox(canvasPos))
   {
      float zoomCenter = ofLerp(mCanvas->mStart, mCanvas->mEnd, canvasPos.x);
      float distFromStart = zoomCenter - mCanvas->mStart;
      float distFromEnd = zoomCenter - mCanvas->mEnd;
      
      distFromStart *= 1 - scrollY/100;
      distFromEnd *= 1 - scrollY/100;
      
      float slideX = (mCanvas->mEnd - mCanvas->mStart) * -scrollX/300;
      
      mCanvas->mStart = ofClamp(zoomCenter - distFromStart + slideX, 0, 1);
      mCanvas->mEnd = ofClamp(zoomCenter - distFromEnd + slideX, 0, 1);
      return true;
   }
   return false;
}

void EventCanvas::SyncControlCablesToCanvas()
{
   if (mCanvas->GetNumRows() == mControlCables.size())
      return;  //nothing to do
   
   if (mCanvas->GetNumRows() > mControlCables.size())
   {
      int oldSize = (int)mControlCables.size();
      mControlCables.resize(mCanvas->GetNumRows());
      for (int i=oldSize; i<mControlCables.size(); ++i)
      {
         mControlCables[i] = new PatchCableSource(this, kConnectionType_UIControl);
         mControlCables[i]->SetOverrideCableDir(ofVec2f(1,0));
         mControlCables[i]->SetColor(GetRowColor(i));
         AddPatchCableSource(mControlCables[i]);
      }
   }
   else
   {
      for (int i=mCanvas->GetNumRows(); i<mControlCables.size(); ++i)
         RemovePatchCableSource(mControlCables[i]);
      mControlCables.resize(mCanvas->GetNumRows());
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 140;
}

void EventCanvas::Resize(float w, float h)
{
   w = MAX(w - extraW, 390);
   h = MAX(h - extraH, 100);
   mCanvas->SetDimensions(w, h);
   mPositionSlider->SetDimensions(w, 14);
}

void EventCanvas::GetModuleDimensions(float& width, float& height)
{
   width = mCanvas->GetWidth() + extraW;
   height = mCanvas->GetHeight() + extraH;
}

namespace
{
   bool ElementSort(EventCanvasElement* a, EventCanvasElement* b)
   {
      return a->GetStart() < b->GetStart();
   }
}

void EventCanvas::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mPreviousPosition = mPosition + .001f;
   }
}

void EventCanvas::ButtonClicked(ClickButton* button)
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

void EventCanvas::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mPositionSlider)
   {
      float measure = mNumMeasures * mPosition;
      TheTransport->SetMeasure((int)measure);
      TheTransport->SetMeasurePos(measure - (int)measure);
   }
}

void EventCanvas::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void EventCanvas::TextEntryComplete(TextEntry* entry)
{
   if (entry == mNumMeasuresEntry)
   {
      mCanvas->SetNumCols(TheTransport->CountInStandardMeasure(mInterval) * mNumMeasures);
   }
}

void EventCanvas::DropdownUpdated(DropdownList* list, int oldVal)
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
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["canvaswidth"] = mCanvas->GetWidth();
   moduleInfo["canvasheight"] = mCanvas->GetHeight();
}

namespace
{
   const int kSaveStateRev = 0;
}

void EventCanvas::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << (int)mControlCables.size();
   for (auto cable : mControlCables)
   {
      string path = "";
      if (cable->GetTarget())
         path = cable->GetTarget()->Path();
      out << path;
   }
   
   mCanvas->SaveState(out);
}

void EventCanvas::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   int size;
   in >> size;
   mControlCables.resize(size);
   for (auto cable : mControlCables)
   {
      string path;
      in >> path;
      cable->SetTarget(TheSynth->FindUIControl(path));
   }
   
   mCanvas->LoadState(in);
   
   mPositionSlider->SetDimensions(mCanvas->GetWidth(), 14);
}

vector<IUIControl*> EventCanvas::ControlsToIgnoreInSaveState() const
{
   vector<IUIControl*> ignore;
   ignore.push_back(mCanvas);
   return ignore;
}
