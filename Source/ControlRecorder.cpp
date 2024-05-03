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

    ControlRecorder.cpp
    Created: 7 Apr 2024
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ControlRecorder.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

#include "juce_core/juce_core.h"

ControlRecorder::ControlRecorder()
{
}

ControlRecorder::~ControlRecorder()
{
}

void ControlRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   float controlH;
   UIBLOCK0();
   CHECKBOX(mRecordCheckbox, "record", &mRecord);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mQuantizeLengthCheckbox, "quantize", &mQuantizeLength);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mQuantizeLengthSelector, "length", (int*)(&mQuantizeInterval), 60);
   UIBLOCK_SHIFTLEFT();
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, .1f, 10);
   UIBLOCK_NEWLINE();
   BUTTON(mClearButton, "clear");
   ENDUIBLOCK(mWidth, controlH);

   mDisplayStartY = controlH + 3;

   mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCable->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCable);

   mQuantizeLengthSelector->AddLabel("8n", kInterval_8n);
   mQuantizeLengthSelector->AddLabel("4n", kInterval_4n);
   mQuantizeLengthSelector->AddLabel("2n", kInterval_2n);
   mQuantizeLengthSelector->AddLabel("1", kInterval_1n);
   mQuantizeLengthSelector->AddLabel("2", kInterval_2);
   mQuantizeLengthSelector->AddLabel("3", kInterval_3);
   mQuantizeLengthSelector->AddLabel("4", kInterval_4);
   mQuantizeLengthSelector->AddLabel("8", kInterval_8);
   mQuantizeLengthSelector->AddLabel("16", kInterval_16);
   mQuantizeLengthSelector->AddLabel("32", kInterval_32);
   mQuantizeLengthSelector->AddLabel("64", kInterval_64);
}

void ControlRecorder::Init()
{
   IDrawableModule::Init();
}

void ControlRecorder::Poll()
{
   IModulator::Poll();

   if (mRecord)
      RecordPoint();
}

float ControlRecorder::GetPlaybackTime(double time)
{
   float measureTime = TheTransport->GetMeasureTime(time) - mRecordStartOffset;
   if (!mQuantizeLength)
      measureTime *= mSpeed;

   if (mLength > 0)
   {
      while (measureTime < 0)
         measureTime += mLength;
      return fmod(measureTime, mLength);
   }
   return 0;
}

void ControlRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRecordCheckbox->Draw();
   mQuantizeLengthCheckbox->Draw();
   mQuantizeLengthSelector->SetShowing(mQuantizeLength);
   mQuantizeLengthSelector->Draw();
   mSpeedSlider->SetShowing(!mQuantizeLength);
   mSpeedSlider->Draw();
   mClearButton->Draw();

   DrawTextNormal("length: " + ofToString(mLength, 2) + " measures",
                  mClearButton->GetRect(K(local)).getMaxX() + 5, mClearButton->GetRect(K(local)).getMinY() + 12);

   ofPushStyle();
   ofSetColor(100, 100, 100, 100);
   ofFill();

   ofPushMatrix();
   ofTranslate(3, mDisplayStartY);

   ofSetColor(100, 100, 100, 100);
   ofRect(0, 0, mWidth - 6, mHeight - 5 - mDisplayStartY);
   mCurve.SetDimensions(mWidth - 6, mHeight - 5 - mDisplayStartY);
   mCurve.Render();
   ofSetColor(GetColor(kModuleCategory_Modulator));
   if (GetPatchCableSource()->GetTarget())
      DrawTextNormal(GetPatchCableSource()->GetTarget()->Name(), 5, 15);

   if (mLength > 0 && !mRecord)
   {
      float playbackTime = GetPlaybackTime(gTime);
      float lineX = ofLerp(3, mWidth - 6, playbackTime / mLength);
      ofLine(lineX, 0, lineX, mHeight - 5 - mDisplayStartY);
   }

   ofPopMatrix();
   ofPopStyle();
}


void ControlRecorder::Clear()
{
   mLength = 0;
   mSpeed = 1;
   mCurve.Clear();
   mHasRecorded = false;
}

void ControlRecorder::RecordPoint()
{
   IUIControl* target = dynamic_cast<IUIControl*>(GetPatchCableSource()->GetTarget());
   if (target)
   {
      float time = TheTransport->GetMeasureTime(gTime) - mRecordStartOffset;
      mCurve.AddPointAtEnd(CurvePoint(time, target->GetMidiValue()));
      mCurve.SetExtents(0, time);
      mHasRecorded = true;
      mLength = time;
   }
}

void ControlRecorder::SetRecording(bool record)
{
   mRecord = record;
   if (mRecord)
   {
      mRecordStartOffset = TheTransport->GetMeasureTime(gTime);
      Clear();
      RecordPoint();
   }
   else
   {
      if (mQuantizeLength)
      {
         float quantizeResolution = TheTransport->GetMeasureFraction(mQuantizeInterval);
         int quantizeIntervalSteps = juce::roundToInt(mLength / quantizeResolution);
         if (quantizeIntervalSteps <= 0)
            quantizeIntervalSteps = 1;
         float quantizedLength = quantizeResolution * quantizeIntervalSteps;
         mLength = quantizedLength;
         mCurve.SetExtents(0, mLength);
      }
      mRecordStartOffset = fmod(mRecordStartOffset, mLength);
   }
}

void ControlRecorder::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (fromUserClick && GetNumTargets() <= 1)
      Clear();

   mConnectedControl = dynamic_cast<IUIControl*>(GetPatchCableSource()->GetTarget());
}

void ControlRecorder::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRecordCheckbox)
      SetRecording(mRecord);
}

void ControlRecorder::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void ControlRecorder::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
      Clear();
}

void ControlRecorder::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void ControlRecorder::Resize(float w, float h)
{
   w = MAX(w, 220);
   h = MAX(h, 100);
   mWidth = w;
   mHeight = h;
}

void ControlRecorder::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ControlRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void ControlRecorder::SetUpFromSaveData()
{
}

void ControlRecorder::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;

   mCurve.SaveState(out);
   out << mHasRecorded;
   out << mLength;
   out << mRecordStartOffset;
}

void ControlRecorder::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;
   in >> mHeight;

   mCurve.LoadState(in);
   in >> mHasRecorded;

   in >> mLength;
   mCurve.SetExtents(0, mLength);
   in >> mRecordStartOffset;
}

float ControlRecorder::Value(int samplesIn)
{
   float playbackTime = GetPlaybackTime(gTime + samplesIn * gInvSampleRateMs);
   float val = mCurve.Evaluate(playbackTime, true);
   if (mConnectedControl != nullptr && mConnectedControl->ModulatorUsesLiteralValue())
      return mConnectedControl->GetValueForMidiCC(val);
   return val;
}
