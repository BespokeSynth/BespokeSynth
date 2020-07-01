//
//  CanvasElement.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/15.
//
//

#include "CanvasElement.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "Slider.h"
#include "CanvasControls.h"
#include "TextEntry.h"
#include "PatchCableSource.h"
#include "ModularSynth.h"
#include "EventCanvas.h"

CanvasElement::CanvasElement(Canvas* canvas, int col, int row, float offset, float length)
: mCanvas(canvas)
, mOffset(offset)
, mLength(length)
, mCol(col)
, mRow(row)
, mHighlighted(false)
{
}

void CanvasElement::Draw()
{
   DrawElement(false);
   if (mCanvas->ShouldWrap() && GetEnd() > 1)
      DrawElement(true);
}

void CanvasElement::DrawElement(bool wrapped)
{
   ofRectangle rect = GetRect(K(clamp), wrapped);
   ofRectangle fullRect = GetRect(!K(clamp), wrapped);
   
   ofPushStyle();
   
   ofSetColor(GetColor());
   ofFill();
   ofRect(rect);
   
   ofNoFill();
   if (mHighlighted)
   {
      ofSetColor(255,255,255);
      ofSetLineWidth(1.5f * gDrawScale);
   }
   else
   {
      ofSetColor(0,0,0);
   }
   ofLine(rect.x,rect.y,rect.x+rect.width,rect.y);
   ofLine(rect.x,rect.y+rect.height,rect.x+rect.width,rect.y+rect.height);
   if (fullRect.x >= 0)
   {
      ofPushStyle();
      if (mHighlighted && mCanvas->GetHighlightEnd() == Canvas::kHighlightEnd_Start)
      {
         ofSetLineWidth(2 * gDrawScale);
         ofSetColor(255,100,100);
      }
      ofLine(rect.x,rect.y,rect.x,rect.y+rect.height);
      ofPopStyle();
   }
   if (fullRect.x+fullRect.width <= mCanvas->GetLength() * mCanvas->GetGridWidth())
   {
      ofPushStyle();
      if (mHighlighted && mCanvas->GetHighlightEnd() == Canvas::kHighlightEnd_End && IsResizable())
      {
         ofSetLineWidth(2 * gDrawScale);
         ofSetColor(255,100,100);
      }
      ofLine(rect.x+rect.width,rect.y,rect.x+rect.width,rect.y+rect.height);
      ofPopStyle();
   }
   
   ofPopStyle();
   
   ofPushStyle();
   DrawContents();
   ofPopStyle();
}

float CanvasElement::GetStart() const
{
   return (mCol + mOffset) / mCanvas->GetNumCols();
}

void CanvasElement::SetStart(float start)
{
   FloatWrap(start, 1);
   start *= mCanvas->GetNumCols();
   mCol = int(start + .5f);
   mOffset = start - mCol;
}

float CanvasElement::GetEnd() const
{
   return (mCol + mOffset + mLength) / mCanvas->GetNumCols();
}

void CanvasElement::SetEnd(float end)
{
   mLength = end * mCanvas->GetNumCols() - mCol - mOffset;
}

ofRectangle CanvasElement::GetRect(bool clamp, bool wrapped) const
{
   float offset = wrapped ? -1 : 0;
   float start = ofMap(GetStart() + offset,mCanvas->mStart/mCanvas->GetLength(),mCanvas->mEnd/mCanvas->GetLength(),0,1,clamp) * mCanvas->GetGridWidth();
   float end = ofMap(GetEnd() + offset,mCanvas->mStart/mCanvas->GetLength(),mCanvas->mEnd/mCanvas->GetLength(),0,1,clamp) * mCanvas->GetGridWidth();
   float y = (float(mRow-mCanvas->GetRowOffset()) / mCanvas->GetNumVisibleRows()) * mCanvas->GetGridHeight();
   float height = float(mCanvas->GetGridHeight()) / mCanvas->GetNumVisibleRows();
   
   return ofRectangle(start, y, end-start, height);
}

void CanvasElement::AddElementUIControl(IUIControl* control)
{
   mUIControls.push_back(control);
   control->SetShowing(false);
}

void CanvasElement::CheckboxUpdated(string label, bool value)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(value);
   }
}

void CanvasElement::FloatSliderUpdated(string label, float oldVal, float newVal)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(newVal);
   }
}

void CanvasElement::IntSliderUpdated(string label, int oldVal, float newVal)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(newVal);
   }
}

namespace
{
   const int kCESaveStateRev = 1;
}

void CanvasElement::SaveState(FileStreamOut& out)
{
   out << kCESaveStateRev;
   
   out << mOffset;
   out << mLength;
}

void CanvasElement::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kCESaveStateRev);
   
   if (rev < 1)
   {
      in >> mRow;
      in >> mCol;
   }
   in >> mOffset;
   in >> mLength;
}

////////////////////

NoteCanvasElement::NoteCanvasElement(Canvas* canvas, int col, int row, float offset, float length)
: CanvasElement(canvas,col,row,offset,length)
, mVelocity(.5f)
, mVoiceIdx(-1)
, mPan(0)
{
   mElementOffsetSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()),"offset",0,0,100,15,&mOffset,-1,1);
   AddElementUIControl(mElementOffsetSlider);
   mElementLengthSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()),"length",0,0,100,15,&mLength,0,mCanvas->GetNumCols());
   AddElementUIControl(mElementLengthSlider);
   mElementRowSlider = new IntSlider(dynamic_cast<IIntSliderListener*>(canvas->GetControls()),"row",0,0,100,15,&mRow,0,mCanvas->GetNumRows()-1);
   AddElementUIControl(mElementRowSlider);
   mElementColSlider = new IntSlider(dynamic_cast<IIntSliderListener*>(canvas->GetControls()),"step",0,0,100,15,&mCol,0,mCanvas->GetNumCols()-1);
   AddElementUIControl(mElementColSlider);
   mVelocitySlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()),"velocity",0,0,100,15,&mVelocity,0,1);
   AddElementUIControl(mVelocitySlider);
}

CanvasElement* NoteCanvasElement::CreateDuplicate() const
{
   NoteCanvasElement* element = new NoteCanvasElement(mCanvas, mCol, mRow, mOffset, mLength);
   element->mVelocity = mVelocity;
   element->mVoiceIdx = mVoiceIdx;
   return element;
}

void NoteCanvasElement::DrawContents()
{
   ofPushStyle();
   ofSetColor(255,255,255);
   ofFill();
   //DrawTextNormal(ofToString(mVelocity), GetRect(true, false).x, GetRect(true, false).y);
   for (int i=0; i<2; ++i)
   {
      ofRectangle rect = GetRect(true, i==0 ? false : true);
      float oldHeight = rect.height;
      rect.height *= mVelocity;
      rect.y += oldHeight - rect.height;
      ofRect(rect);
      
      /*ofPushStyle();
      ofSetLineWidth(1.5f * gDrawScale);
      
      rect = GetRect(true, i==0 ? false : true);
      ofRectangle fullRect = GetRect(false, false);
      if (i == 1)
         fullRect.x -= mCanvas->GetGridWidth();
      float length = (GetEnd()-GetStart()) * mCanvas->GetLength();
      
      float start = (float(rect.x-fullRect.x)/fullRect.width);
      float end = ((float(rect.x+rect.width)-(fullRect.x))/fullRect.width);
      
      mPitchBendCurve.SetPosition(rect.x, rect.y - rect.height/2);
      mPitchBendCurve.SetDimensions(rect.width, rect.height);
      mPitchBendCurve.SetExtents(start*length, end*length);
      mPitchBendCurve.SetColor(ofColor::red);
      mPitchBendCurve.Render();
      
      mModWheelCurve.SetPosition(rect.x, rect.y);
      mModWheelCurve.SetDimensions(rect.width, rect.height);
      mModWheelCurve.SetExtents(start*length, end*length);
      mModWheelCurve.SetColor(ofColor::green);
      mModWheelCurve.Render();
      
      mPressureCurve.SetPosition(rect.x, rect.y);
      mPressureCurve.SetDimensions(rect.width, rect.height);
      mPressureCurve.SetExtents(start*length, end*length);
      mPressureCurve.SetColor(ofColor::blue);
      mPressureCurve.Render();
      
      ofPopStyle();*/
   }
   
   ofPopStyle();
}

void NoteCanvasElement::UpdateModulation(float pos)
{
   float curveTime = (pos - GetStart()) * mCanvas->GetLength();
   FloatWrap(curveTime, mCanvas->GetLength());
   mPitchBend.SetValue(mPitchBendCurve.Evaluate(curveTime));
   mModWheel.SetValue(mModWheelCurve.Evaluate(curveTime));
   mPressure.SetValue(mPressureCurve.Evaluate(curveTime));
   mPan = mPanCurve.Evaluate(curveTime);
}

void NoteCanvasElement::WriteModulation(float pos, float pitchBend, float modWheel, float pressure, float pan)
{
   float curveTime = (pos - GetStart()) * mCanvas->GetLength();
   FloatWrap(curveTime, mCanvas->GetLength());
   mPitchBendCurve.AddPoint(CurvePoint(curveTime, pitchBend));
   mModWheelCurve.AddPoint(CurvePoint(curveTime, modWheel));
   mPressureCurve.AddPoint(CurvePoint(curveTime, pressure));
   mPanCurve.AddPoint(CurvePoint(curveTime, pan));
}

namespace
{
   const int kNCESaveStateRev = 0;
}

void NoteCanvasElement::SaveState(FileStreamOut& out)
{
   CanvasElement::SaveState(out);
   
   out << kNCESaveStateRev;
   
   out << mVelocity;
}

void NoteCanvasElement::LoadState(FileStreamIn& in)
{
   CanvasElement::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kNCESaveStateRev);
   
   in >> mVelocity;
}

/////////////////////

SampleCanvasElement::SampleCanvasElement(Canvas* canvas, int col, int row, float offset, float length)
: CanvasElement(canvas,col,row,offset,length)
, mSample(nullptr)
, mNumLoops(1)
, mNumBars(1)
, mVolume(1)
, mMeasureSync(true)
{
   mNumBarsEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(canvas->GetControls()),"num bars",60,2,3,&mNumBars,1,128);
   AddElementUIControl(mNumBarsEntry);
   mElementOffsetSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()),"offset",0,0,100,15,&mOffset,-1,1);
   AddElementUIControl(mElementOffsetSlider);
   mNumLoopsSlider = new IntSlider(dynamic_cast<IIntSliderListener*>(canvas->GetControls()),"loops",0,0,100,15,&mNumLoops,1,16);
   AddElementUIControl(mNumLoopsSlider);
   mVolumeSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()),"volume",0,0,100,15,&mVolume,0,2);
   AddElementUIControl(mVolumeSlider);
   mMeasureSyncCheckbox = new Checkbox(dynamic_cast<IDrawableModule*>(canvas->GetControls()),"measure sync",0,0,&mMeasureSync);
   AddElementUIControl(mMeasureSyncCheckbox);
}

SampleCanvasElement::~SampleCanvasElement()
{
   delete mSample;
}

CanvasElement* SampleCanvasElement::CreateDuplicate() const
{
   SampleCanvasElement* element = new SampleCanvasElement(mCanvas, mCol, mRow, mOffset, mLength);
   element->mSample = new Sample();
   element->mSample->Create(mSample->Data());
   element->mSample->SetNumBars(mSample->GetNumBars());
   element->mNumLoops = mNumLoops;
   element->mNumBars = mNumBars;
   element->mVolume = mVolume;
   element->mMeasureSync = mMeasureSync;
   return element;
}

void SampleCanvasElement::SetSample(Sample* sample)
{
   mSample = sample;
   if (sample->GetNumBars() > -1)
      mNumBars = sample->GetNumBars();
}

void SampleCanvasElement::CheckboxUpdated(string label, bool value)
{
   CanvasElement::CheckboxUpdated(label, value);
   if (label == mMeasureSyncCheckbox->Name())
   {
      mNumBarsEntry->SetShowing(value);
      mNumLoopsSlider->SetShowing(value);
   }
}

void SampleCanvasElement::DrawContents()
{
   mLength = mNumBars * mNumLoops;
   
   ofRectangle fullRect = GetRect(false, false);
   ofRectangle clampedRect = GetRect(true, false);
   float clampedRectEndX = clampedRect.x + clampedRect.width;
   float sampleRectLoopWidth = fullRect.width / mNumLoops;
   for (int i=0; i<mNumLoops; ++i)
   {
      float sampleRectX = fullRect.x + i*sampleRectLoopWidth;
      float sampleRectEnd = sampleRectX + sampleRectLoopWidth;
      float samplePosStart = ofMap(clampedRect.x, sampleRectX, sampleRectEnd, 0, 1, K(clamp));
      float samplePosEnd = ofMap(clampedRectEndX, sampleRectX, sampleRectEnd, 0, 1, K(clamp));
      if ((sampleRectX >= clampedRect.x && sampleRectX <= clampedRectEndX) ||
          (sampleRectEnd >= clampedRect.x && sampleRectEnd <= clampedRectEndX) ||
          (sampleRectX < clampedRect.x && sampleRectEnd > clampedRectEndX))
      {
         ofPushMatrix();
         ofTranslate(MAX(clampedRect.x, sampleRectX),clampedRect.y);
         if (mSample)
         {
            float width = MIN(sampleRectEnd, clampedRectEndX) - MAX(sampleRectX, clampedRect.x);
            int length = mSample->LengthInSamples();
            DrawAudioBuffer(width, clampedRect.height, mSample->Data(), samplePosStart * length, samplePosEnd * length, -1);
            if (i > 0 && sampleRectX > clampedRect.x)
            {
               ofSetColor(255,255,0);
               ofLine(0,0,0,clampedRect.height);
            }
         }
         else
         {
            ofSetColor(0,0,0);
            ofRect(0,0,clampedRect.width,clampedRect.height);
         }
         ofPopMatrix();
      }
   }
}

namespace
{
   const int kSCESaveStateRev = 0;
}

void SampleCanvasElement::SaveState(FileStreamOut& out)
{
   CanvasElement::SaveState(out);
   
   out << kSCESaveStateRev;
   
   bool hasSample = mSample != nullptr;
   out << hasSample;
   mSample->SaveState(out);
   out << mNumLoops;
   out << mNumBars;
   out << mVolume;
   out << mMeasureSync;
}

void SampleCanvasElement::LoadState(FileStreamIn& in)
{
   CanvasElement::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSCESaveStateRev);
   
   bool hasSample;
   in >> hasSample;
   if (hasSample)
   {
      mSample = new Sample();
      mSample->LoadState(in);
   }
   in >> mNumLoops;
   in >> mNumBars;
   in >> mVolume;
   in >> mMeasureSync;
}

/////////////////////

EventCanvasElement::EventCanvasElement(Canvas* canvas, int col, int row, float offset)
: CanvasElement(canvas,col,row,offset,.5f)
, mValue(0)
{
   mValueEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(canvas->GetControls()),"value",60,2,7,&mValue,-99999,99999);
   AddElementUIControl(mValueEntry);
   
   mEventCanvas = dynamic_cast<EventCanvas*>(canvas->GetControls()->GetParent());
   assert(mEventCanvas);
   mUIControl = mEventCanvas->GetUIControlForRow(row);
   mIsCheckbox = dynamic_cast<Checkbox*>(mUIControl) != nullptr;
   
   if (mUIControl)
      mValue = mUIControl->GetValue();
}

EventCanvasElement::~EventCanvasElement()
{
}

CanvasElement* EventCanvasElement::CreateDuplicate() const
{
   EventCanvasElement* element = new EventCanvasElement(mCanvas, mCol, mRow, mOffset);
   element->mUIControl = mUIControl;
   element->mIsCheckbox = mIsCheckbox;
   element->mValue = mValue;
   return element;
}

void EventCanvasElement::DrawContents()
{
   if (GetRect(false,false).width != GetRect(true,false).width)
      return;  //only draw text for fully visible elements
   
   string text;
   if (mIsCheckbox)
   {
      text = mUIControl->Name();
      ofRectangle rect = GetRect(true, false);
      ofSetColor(0,0,0);
      DrawTextNormal(text, rect.x+4, rect.y+11);
      ofSetColor(255,255,255);
      DrawTextNormal(text, rect.x+3, rect.y+10);
   }
   else if (mUIControl)
   {
      text += mUIControl->Name();
      text += ":";
      text += mUIControl->GetDisplayValue(mValue);
      
      ofRectangle rect = GetRect(true, false);
      ofSetColor(0,0,0);
      DrawTextNormal(text, rect.x+rect.width+1, rect.y+11);
      ofSetColor(255,255,255);
      DrawTextNormal(text, rect.x+rect.width, rect.y+10);
   }
}

void EventCanvasElement::SetUIControl(IUIControl* control)
{
   bool hadUIControl = mUIControl != nullptr;
   mUIControl = dynamic_cast<IUIControl*>(control);
   mIsCheckbox = dynamic_cast<Checkbox*>(control) != nullptr;
   if (mUIControl && !hadUIControl)
      mValue = mUIControl->GetValue();
}

void EventCanvasElement::Trigger()
{
   if (mUIControl)
   {
      if (mIsCheckbox)
         mUIControl->SetValue(1);
      else
         mUIControl->SetValue(mValue);
   }
}

void EventCanvasElement::TriggerEnd()
{
   if (mUIControl && mIsCheckbox)
       mUIControl->SetValue(0);
}

ofColor EventCanvasElement::GetColor() const
{
   return mEventCanvas->GetRowColor(mRow);
}

float EventCanvasElement::GetEnd() const
{
   if (mIsCheckbox)  //normal resizable element
      return CanvasElement::GetEnd();
   
   float size = 4;
   float span = mCanvas->mEnd - mCanvas->mStart;
   return GetStart() + size * span / mCanvas->GetGridWidth();
}

namespace
{
   const int kECESaveStateRev = 1;
}

void EventCanvasElement::SaveState(FileStreamOut& out)
{
   CanvasElement::SaveState(out);
   
   out << kECESaveStateRev;
   
   out << mValue;
}

void EventCanvasElement::LoadState(FileStreamIn& in)
{
   CanvasElement::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kECESaveStateRev);
   
   in >> mValue;
   if (rev < 1)
   {
      string dummy;
      in >> dummy;
   }
}
