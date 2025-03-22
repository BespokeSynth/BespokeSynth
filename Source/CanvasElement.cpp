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
#include "ModularSynth.h"
#include "EventCanvas.h"
#include "SampleCanvas.h"

CanvasElement::CanvasElement(Canvas* canvas, int col, int row, float offset, float length)
: mCanvas(canvas)
, mOffset(offset)
, mLength(length)
, mCol(col)
, mRow(row)
{
}

void CanvasElement::Draw(ofVec2f offset)
{
   if (offset.x == 0 && offset.y == 0)
   {
      DrawElement(K(clamp), !K(wrapped), offset);
      if (mCanvas->ShouldWrap() && GetEnd() > 1)
         DrawElement(K(clamp), K(wrapped), offset);
   }
   else
   {
      ofPushStyle();
      ofSetLineWidth(3.0f);
      ofNoFill();
      ofSetColor(255, 255, 255, ofMap(sin(gTime / 500 * PI * 2), -1, 1, 40, 120));
      ofRect(GetRectAtDestination(K(clamp), !K(wrapped), offset), 0);
      if (mCanvas->ShouldWrap() && GetEnd() > 1)
         ofRect(GetRectAtDestination(K(clamp), K(wrapped), offset), 0);
      ofPopStyle();

      DrawElement(!K(clamp), !K(wrapped), offset);
   }
}

void CanvasElement::DrawElement(bool clamp, bool wrapped, ofVec2f offset)
{
   ofRectangle rect = GetRect(clamp, wrapped, offset);
   ofRectangle fullRect = GetRect(false, wrapped, offset);

   if (rect.width < 0)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0);
      ofRect(rect.x + rect.width, rect.y, -rect.width, rect.height, 0);
      ofPopStyle();
   }

   ofPushStyle();
   DrawContents(clamp, wrapped, offset);
   ofPopStyle();

   if (mHighlighted && mCanvas == gHoveredUIControl)
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(255, 200, 0);
      ofSetLineWidth(.75f);

      ofLine(rect.x, rect.y, rect.x + rect.width, rect.y);
      ofLine(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height);

      if (fullRect.x >= 0)
      {
         ofPushStyle();
         if (mCanvas->GetHighlightEnd() == Canvas::kHighlightEnd_Start && IsResizable())
         {
            ofSetLineWidth(1.5f);
            ofSetColor(255, 100, 100);
         }
         ofLine(rect.x, rect.y, rect.x, rect.y + rect.height);
         ofPopStyle();
      }
      if (fullRect.x + fullRect.width <= mCanvas->GetLength() * mCanvas->GetWidth())
      {
         ofPushStyle();
         if (mCanvas->GetHighlightEnd() == Canvas::kHighlightEnd_End && IsResizable())
         {
            ofSetLineWidth(1.5f);
            ofSetColor(255, 100, 100);
         }
         ofLine(rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height);
         ofPopStyle();
      }

      ofPopStyle();
   }
}

void CanvasElement::DrawOffscreen()
{
   ofPushStyle();
   ofSetLineWidth(1);
   ofSetColor(255, 255, 255);
   {
      ofRectangle rect = GetRect(K(clamp), !K(wrapped));
      if (rect.y < 0)
      {
         rect.y = 0;
         rect.height = 1;
      }
      if (rect.y + rect.height > mCanvas->GetHeight())
      {
         rect.y = mCanvas->GetHeight() - 1;
         rect.height = 1;
      }
      rect.width = MAX(rect.width, 1);
      ofRect(rect);
   }
   if (mCanvas->ShouldWrap() && GetEnd() > 1)
   {
      ofRectangle rect = GetRect(K(clamp), K(wrapped));
      if (rect.y < 0)
      {
         rect.y = 0;
         rect.height = 1;
      }
      if (rect.y > mCanvas->GetHeight())
      {
         rect.y = mCanvas->GetHeight() - 1;
         rect.height = 1;
      }
      rect.width = MAX(rect.width, 1);
      ofRect(rect);
   }
   ofPopStyle();
}

float CanvasElement::GetStart(int col, float offset) const
{
   return (col + offset) / mCanvas->GetNumCols();
}

float CanvasElement::GetEnd(int col, float offset, float length) const
{
   return (col + offset + length) / mCanvas->GetNumCols();
}

float CanvasElement::GetStart() const
{
   return GetStart(mCol, mOffset);
}

void CanvasElement::SetStart(float start, bool preserveLength)
{
   float end = 0;
   if (!preserveLength)
      end = GetEnd();
   start *= mCanvas->GetNumCols();
   mCol = int(start + .5f);
   mOffset = start - mCol;
   if (!preserveLength)
      SetEnd(end);
}

float CanvasElement::GetEnd() const
{
   return GetEnd(mCol, mOffset, mLength);
}

void CanvasElement::SetEnd(float end)
{
   mLength = end * mCanvas->GetNumCols() - mCol - mOffset;
}

ofRectangle CanvasElement::GetRect(bool clamp, bool wrapped, ofVec2f offset) const
{
   float wrapOffset = wrapped ? -1 : 0;
   float start = ofMap(GetStart() + wrapOffset, mCanvas->mViewStart / mCanvas->GetLength(), mCanvas->mViewEnd / mCanvas->GetLength(), 0, 1, clamp) * mCanvas->GetWidth();
   float end = ofMap(GetEnd() + wrapOffset, mCanvas->mViewStart / mCanvas->GetLength(), mCanvas->mViewEnd / mCanvas->GetLength(), 0, 1, clamp) * mCanvas->GetWidth();
   float y = (float(mRow - mCanvas->GetRowOffset()) / mCanvas->GetNumVisibleRows()) * mCanvas->GetHeight();
   float height = float(mCanvas->GetHeight()) / mCanvas->GetNumVisibleRows();

   return ofRectangle(start + offset.x, y + offset.y, end - start, height);
}

ofRectangle CanvasElement::GetRectAtDestination(bool clamp, bool wrapped, ofVec2f dragOffset) const
{
   int newRow;
   int newCol;
   float newOffset;
   GetDragDestinationData(dragOffset, newRow, newCol, newOffset);

   float wrapOffset = wrapped ? -1 : 0;
   float start = ofMap(GetStart(newCol, newOffset) + wrapOffset, mCanvas->mViewStart / mCanvas->GetLength(), mCanvas->mViewEnd / mCanvas->GetLength(), 0, 1, clamp) * mCanvas->GetWidth();
   float end = ofMap(GetEnd(newCol, newOffset, mLength) + wrapOffset, mCanvas->mViewStart / mCanvas->GetLength(), mCanvas->mViewEnd / mCanvas->GetLength(), 0, 1, clamp) * mCanvas->GetWidth();
   float y = (float(newRow - mCanvas->GetRowOffset()) / mCanvas->GetNumVisibleRows()) * mCanvas->GetHeight();
   float height = float(mCanvas->GetHeight()) / mCanvas->GetNumVisibleRows();

   return ofRectangle(start, y, end - start, height);
}

void CanvasElement::GetDragDestinationData(ofVec2f dragOffset, int& newRow, int& newCol, float& newOffset) const
{
   dragOffset.x *= mCanvas->mViewEnd - mCanvas->mViewStart;

   float colDrag = (dragOffset.x / mCanvas->GetWidth()) * mCanvas->GetNumCols() / mCanvas->GetLength();
   float rowDrag = (dragOffset.y / mCanvas->GetHeight()) * mCanvas->GetNumVisibleRows();

   newCol = mCol;
   if (mCanvas->GetDragMode() & Canvas::kDragHorizontal)
      newCol = ofClamp(int(mCol + colDrag + .5f), 0, mCanvas->GetNumCols() - 1);

   newRow = mRow;
   if (mCanvas->GetDragMode() & Canvas::kDragVertical)
      newRow = ofClamp(int(mRow + rowDrag + .5f), 0, mCanvas->GetNumRows() - 1);

   newOffset = mOffset;
   if (GetKeyModifiers() & kModifier_Alt) //non-snapped drag
      newOffset = mOffset + colDrag - (newCol - mCol);
   if (GetKeyModifiers() & kModifier_Command) //quantize
      newOffset = 0;
}

void CanvasElement::MoveElementByDrag(ofVec2f dragOffset)
{
   int newRow;
   int newCol;
   float newOffset;
   GetDragDestinationData(dragOffset, newRow, newCol, newOffset);

   mRow = newRow;
   mCol = newCol;
   mOffset = newOffset;
}

void CanvasElement::AddElementUIControl(IUIControl* control)
{
   mUIControls.push_back(control);
   // Block modulation cables from targeting these controls.
   control->SetCableTargetable(false);
   control->SetShowing(false);
}

void CanvasElement::CheckboxUpdated(std::string label, bool value, double time)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(value, time);
   }
}

void CanvasElement::FloatSliderUpdated(std::string label, float oldVal, float newVal, double time)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(newVal, time);
   }
}

void CanvasElement::IntSliderUpdated(std::string label, int oldVal, float newVal, double time)
{
   for (auto* control : mUIControls)
   {
      if (control->Name() == label)
         control->SetValue(newVal, time);
   }
}

void CanvasElement::ButtonClicked(std::string label, double time)
{
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
: CanvasElement(canvas, col, row, offset, length)
{
   if (canvas != nullptr && canvas->GetControls())
   {
      mElementOffsetSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()), "offset", 0, 0, 100, 15, &mOffset, -1, 1);
      AddElementUIControl(mElementOffsetSlider);
      mElementLengthSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()), "length", 0, 0, 100, 15, &mLength, 0, mCanvas->GetNumCols());
      AddElementUIControl(mElementLengthSlider);
      mElementRowSlider = new IntSlider(dynamic_cast<IIntSliderListener*>(canvas->GetControls()), "row", 0, 0, 100, 15, &mRow, 0, mCanvas->GetNumRows() - 1);
      AddElementUIControl(mElementRowSlider);
      mElementColSlider = new IntSlider(dynamic_cast<IIntSliderListener*>(canvas->GetControls()), "step", 0, 0, 100, 15, &mCol, 0, mCanvas->GetNumCols() - 1);
      AddElementUIControl(mElementColSlider);
      mVelocitySlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()), "velocity", 0, 0, 100, 15, &mVelocity, 0, 1);
      AddElementUIControl(mVelocitySlider);
   }
}

CanvasElement* NoteCanvasElement::CreateDuplicate() const
{
   NoteCanvasElement* element = new NoteCanvasElement(mCanvas, mCol, mRow, mOffset, mLength);
   element->mVelocity = mVelocity;
   element->mVoiceIdx = mVoiceIdx;
   return element;
}

void NoteCanvasElement::DrawContents(bool clamp, bool wrapped, ofVec2f offset)
{
   ofPushStyle();
   ofFill();
   //DrawTextNormal(ofToString(mVelocity), GetRect(true, false).x, GetRect(true, false).y);

   ofRectangle rect = GetRect(clamp, wrapped, offset);
   float fullHeight = rect.height;
   rect.height *= mVelocity;
   rect.y += (fullHeight - rect.height) * .5f;
   if (rect.width > 0)
   {
      ofSetColorGradient(ofColor::white, ofColor(210, 210, 210), ofVec2f(ofLerp(rect.getMinX(), rect.getMaxX(), .5f), rect.y), ofVec2f(rect.getMaxX(), rect.y));
      ofRect(rect, 0);
   }

   /*ofSetLineWidth(1.5f * gDrawScale);
      
   rect = GetRect(clamp, wrapped);
   ofRectangle fullRect = GetRect(false, false);
   if (wrapped)
      fullRect.x -= mCanvas->GetWidth();
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
   mPressureCurve.Render();*/

   ofPopStyle();
}

void NoteCanvasElement::UpdateModulation(float pos)
{
   float curveTime = (pos - GetStart()) * mCanvas->GetLength();
   curveTime = FloatWrap(curveTime, mCanvas->GetLength());
   mPitchBend.SetValue(mPitchBendCurve.Evaluate(curveTime));
   mModWheel.SetValue(mModWheelCurve.Evaluate(curveTime));
   mPressure.SetValue(mPressureCurve.Evaluate(curveTime));
   mPan = mPanCurve.Evaluate(curveTime);
}

void NoteCanvasElement::WriteModulation(float pos, float pitchBend, float modWheel, float pressure, float pan)
{
   float curveTime = (pos - GetStart()) * mCanvas->GetLength();
   curveTime = FloatWrap(curveTime, mCanvas->GetLength());
   mPitchBendCurve.AddPoint(CurvePoint(curveTime, pitchBend));
   mModWheelCurve.AddPoint(CurvePoint(curveTime, modWheel));
   mPressureCurve.AddPoint(CurvePoint(curveTime, pressure));
   mPanCurve.AddPoint(CurvePoint(curveTime, pan));
}

namespace
{
   const int kNCESaveStateRev = 1;
}

void NoteCanvasElement::SaveState(FileStreamOut& out)
{
   CanvasElement::SaveState(out);

   out << kNCESaveStateRev;

   out << mVelocity;

   mPitchBendCurve.SaveState(out);
   mModWheelCurve.SaveState(out);
   mPressureCurve.SaveState(out);
   mPanCurve.SaveState(out);
}

void NoteCanvasElement::LoadState(FileStreamIn& in)
{
   CanvasElement::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kNCESaveStateRev);

   in >> mVelocity;

   if (rev > 0)
   {
      mPitchBendCurve.LoadState(in);
      mModWheelCurve.LoadState(in);
      mPressureCurve.LoadState(in);
      mPanCurve.LoadState(in);
   }
}

/////////////////////

SampleCanvasElement::SampleCanvasElement(Canvas* canvas, int col, int row, float offset, float length)
: CanvasElement(canvas, col, row, offset, length)
{
   mElementOffsetSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()), "offset", 0, 0, 100, 15, &mOffset, -1, 1);
   AddElementUIControl(mElementOffsetSlider);
   mVolumeSlider = new FloatSlider(dynamic_cast<IFloatSliderListener*>(canvas->GetControls()), "volume", 0, 0, 100, 15, &mVolume, 0, 2);
   AddElementUIControl(mVolumeSlider);
   mMuteCheckbox = new Checkbox(dynamic_cast<IDrawableModule*>(canvas->GetControls()), "mute", 0, 0, &mMute);
   AddElementUIControl(mMuteCheckbox);
   mSplitSampleButton = new ClickButton(dynamic_cast<IButtonListener*>(canvas->GetControls()), "split", 0, 0);
   AddElementUIControl(mSplitSampleButton);
   mResetSpeedButton = new ClickButton(dynamic_cast<IButtonListener*>(canvas->GetControls()), "reset speed", 0, 0);
   AddElementUIControl(mResetSpeedButton);
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
   element->mVolume = mVolume;
   element->mMute = mMute;
   return element;
}

void SampleCanvasElement::SetSample(Sample* sample)
{
   mSample = sample;
}

void SampleCanvasElement::CheckboxUpdated(std::string label, bool value, double time)
{
   CanvasElement::CheckboxUpdated(label, value, time);
}

void SampleCanvasElement::ButtonClicked(std::string label, double time)
{
   CanvasElement::ButtonClicked(label, time);
   if (label == "split")
   {
      ChannelBuffer* firstHalf = new ChannelBuffer(mSample->Data()->BufferSize() / 2);
      ChannelBuffer* secondHalf = new ChannelBuffer(mSample->Data()->BufferSize() / 2);
      firstHalf->CopyFrom(mSample->Data(), firstHalf->BufferSize(), 0);
      secondHalf->CopyFrom(mSample->Data(), secondHalf->BufferSize(), mSample->Data()->BufferSize() / 2);

      SampleCanvasElement* element = new SampleCanvasElement(mCanvas, mCol, mRow, mOffset, mLength);
      element->mSample = new Sample();
      element->mSample->Create(secondHalf);
      element->mVolume = mVolume;
      element->mMute = mMute;
      element->SetStart((GetStart() + GetEnd()) * .5f, false);
      mCanvas->AddElement(element);

      mSample->Create(firstHalf);
      mLength /= 2;
   }
   if (label == "reset speed")
   {
      SampleCanvas* sampleCanvas = dynamic_cast<SampleCanvas*>(mCanvas->GetParent());
      if (sampleCanvas != nullptr)
      {
         float lengthMs = mSample->LengthInSamples() / mSample->GetSampleRateRatio() / gSampleRateMs;
         float lengthOriginalSpeed = lengthMs / TheTransport->GetDuration(sampleCanvas->GetInterval());
         mLength = lengthOriginalSpeed;
      }
   }
}

void SampleCanvasElement::DrawContents(bool clamp, bool wrapped, ofVec2f offset)
{
   if (wrapped)
      return;

   ofRectangle rect = GetRect(false, false, offset);

   ofPushMatrix();
   ofTranslate(rect.x, rect.y);
   if (mSample)
   {
      float width = rect.width;
      DrawAudioBuffer(width, rect.height, mSample->Data(), 0, mSample->LengthInSamples(), -1);
   }
   else
   {
      ofSetColor(0, 0, 0);
      ofRect(0, 0, rect.width, rect.height);
   }
   ofPopMatrix();

   if (rect.width < 0)
      rect.set(rect.x + rect.width, rect.y, -rect.width, rect.height);

   SampleCanvas* sampleCanvas = dynamic_cast<SampleCanvas*>(mCanvas->GetParent());
   if (sampleCanvas != nullptr && mSample != nullptr)
   {
      float lengthMs = mSample->LengthInSamples() / mSample->GetSampleRateRatio() / gSampleRateMs;
      float lengthOriginalSpeed = lengthMs / TheTransport->GetDuration(sampleCanvas->GetInterval());
      float speed = lengthOriginalSpeed / mLength;
      ofSetColor(255, 255, 255);
      DrawTextBold(ofToString(speed, 2), rect.x + 3, rect.y + 10, 10);
   }

   if (mMute)
   {
      ofSetColor(0, 0, 0, 140);
      ofFill();
      ofRect(rect);
   }

   ofSetColor(255, 255, 255);
   ofNoFill();
   ofRect(rect);
}

namespace
{
   const int kSCESaveStateRev = 2;
}

void SampleCanvasElement::SaveState(FileStreamOut& out)
{
   CanvasElement::SaveState(out);

   out << kSCESaveStateRev;

   bool hasSample = mSample != nullptr;
   out << hasSample;
   if (mSample != nullptr)
      mSample->SaveState(out);
   out << mVolume;
   out << mMute;
}

void SampleCanvasElement::LoadState(FileStreamIn& in)
{
   CanvasElement::LoadState(in);

   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSCESaveStateRev);

   bool hasSample;
   in >> hasSample;
   if (hasSample)
   {
      mSample = new Sample();
      mSample->LoadState(in);
   }
   if (rev < 2)
   {
      int dummy;
      in >> dummy;
      in >> dummy;
   }
   in >> mVolume;
   if (rev == 0)
   {
      bool dummy;
      in >> dummy;
   }
   if (rev >= 1)
      in >> mMute;
}

/////////////////////

EventCanvasElement::EventCanvasElement(Canvas* canvas, int col, int row, float offset)
: CanvasElement(canvas, col, row, offset, .5f)
{
   mValueEntry = new TextEntry(dynamic_cast<ITextEntryListener*>(canvas->GetControls()), "value", 60, 2, 7, &mValue, -99999, 99999);
   AddElementUIControl(mValueEntry);

   mEventCanvas = dynamic_cast<EventCanvas*>(canvas->GetControls()->GetParent());
   assert(mEventCanvas);
   mUIControl = mEventCanvas->GetUIControlForRow(row);
   mIsCheckbox = dynamic_cast<Checkbox*>(mUIControl) != nullptr;
   mIsButton = dynamic_cast<ClickButton*>(mUIControl) != nullptr;

   if (mUIControl)
      mValue = mUIControl->GetValue();
   if (mIsButton)
      mValue = 1;
}

EventCanvasElement::~EventCanvasElement()
{
}

CanvasElement* EventCanvasElement::CreateDuplicate() const
{
   EventCanvasElement* element = new EventCanvasElement(mCanvas, mCol, mRow, mOffset);
   element->mUIControl = mUIControl;
   element->mIsCheckbox = mIsCheckbox;
   element->mIsButton = mIsButton;
   element->mValue = mValue;
   return element;
}

void EventCanvasElement::DrawContents(bool clamp, bool wrapped, ofVec2f offset)
{
   if (wrapped)
      return;

   if (GetRect(false, false, offset).width != GetRect(clamp, false, offset).width)
      return; //only draw text for fully visible elements

   ofSetColor(mEventCanvas->GetRowColor(mRow));
   ofFill();
   ofRect(GetRect(clamp, wrapped, offset), 0);

   std::string text;
   if (mIsCheckbox)
   {
      text = mUIControl->Name();
      ofRectangle rect = GetRect(clamp, false, offset);
      ofSetColor(0, 0, 0);
      DrawTextNormal(text, rect.x + 4, rect.y + 11);
      ofSetColor(255, 255, 255);
      DrawTextNormal(text, rect.x + 3, rect.y + 10);
   }
   else if (mUIControl)
   {
      text += mUIControl->Name();
      text += ":";
      text += mUIControl->GetDisplayValue(mValue);

      ofRectangle rect = GetRect(clamp, false, offset);
      ofSetColor(0, 0, 0);
      DrawTextNormal(text, rect.x + rect.width + 1, rect.y + 11);
      ofSetColor(255, 255, 255);
      DrawTextNormal(text, rect.x + rect.width, rect.y + 10);
   }
}

void EventCanvasElement::SetUIControl(IUIControl* control)
{
   bool hadUIControl = mUIControl != nullptr;
   mUIControl = dynamic_cast<IUIControl*>(control);
   mIsCheckbox = dynamic_cast<Checkbox*>(control) != nullptr;
   mIsButton = dynamic_cast<ClickButton*>(control) != nullptr;
   if (mUIControl && !hadUIControl)
      mValue = mUIControl->GetValue();
   if (mIsButton)
      mValue = 1;
}

void EventCanvasElement::Trigger(double time)
{
   if (mUIControl)
   {
      if (mIsCheckbox)
         mUIControl->SetValue(1, time);
      else
         mUIControl->SetValue(mValue, time);
   }
}

void EventCanvasElement::TriggerEnd(double time)
{
   if (mUIControl && mIsCheckbox)
      mUIControl->SetValue(0, time);
}

float EventCanvasElement::GetEnd() const
{
   if (mIsCheckbox) //normal resizable element
      return CanvasElement::GetEnd();

   float size = 4;
   float span = mCanvas->mViewEnd - mCanvas->mViewStart;
   return GetStart() + size * span / mCanvas->GetWidth();
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
      std::string dummy;
      in >> dummy;
   }
}
