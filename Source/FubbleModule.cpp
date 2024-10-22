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

    FubbleModule.cpp
    Created: 8 Aug 2020 10:03:56am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "FubbleModule.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

#include "juce_core/juce_core.h"

FubbleModule::FubbleModule()
: mAxisH(this, true)
, mAxisV(this, false)
{
   UpdatePerlinSeed();
}

FubbleModule::~FubbleModule()
{
}

void FubbleModule::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   CHECKBOX(mQuantizeLengthCheckbox, "quantize", &mQuantizeLength);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mQuantizeLengthSelector, "length", (int*)(&mQuantizeInterval), 60);
   UIBLOCK_SHIFTLEFT();
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, .1f, 10);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mClearButton, "clear");
   ENDUIBLOCK0();

   UIBLOCK(10, mHeight - kBottomControlHeight + 3, 135);
   FLOATSLIDER(mPerlinStrengthSlider, "mutate amount", &mPerlinStrength, 0, 2);
   FLOATSLIDER(mPerlinScaleSlider, "mutate warp", &mPerlinScale, 0, 5);
   FLOATSLIDER(mPerlinSpeedSlider, "mutate noise", &mPerlinSpeed, 0, 2);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mUpdatePerlinSeedButton, "reseed");
   ENDUIBLOCK0();

   mAxisH.SetCableSource(new PatchCableSource(this, kConnectionType_Modulator));
   AddPatchCableSource(mAxisH.GetCableSource());
   mAxisV.SetCableSource(new PatchCableSource(this, kConnectionType_Modulator));
   AddPatchCableSource(mAxisV.GetCableSource());

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

void FubbleModule::Init()
{
   IDrawableModule::Init();
}

void FubbleModule::Poll()
{
   if (mIsDrawing)
   {
      RecordPoint();
   }
   else
   {
      if (mPerlinStrength > 0)
      {
         double perlinTime = gTime;
         int numPoints = mAxisH.mCurve.GetNumPoints();
         for (int i = 0; i < numPoints; ++i)
         {
            CurvePoint* pointH = mAxisH.mCurve.GetPoint(i);
            CurvePoint* pointV = mAxisV.mCurve.GetPoint(i);
            float deltaH = ofMap(GetPerlinNoiseValue(perlinTime, pointH->mValue, pointV->mValue, true), 0, 1, -mPerlinStrength, mPerlinStrength) * .003f;
            float deltaV = ofMap(GetPerlinNoiseValue(perlinTime, pointH->mValue, pointV->mValue, false), 0, 1, -mPerlinStrength, mPerlinStrength) * .003f;

            //try to keep in bounds
            if (pointH->mValue < 0)
               deltaH -= pointH->mValue * .1f;
            if (pointH->mValue > 1)
               deltaH -= (pointH->mValue - 1) * .1f;
            if (pointV->mValue < 0)
               deltaV -= pointV->mValue * .1f;
            if (pointV->mValue > 1)
               deltaV -= (pointV->mValue - 1) * .1f;

            pointH->mValue += deltaH;
            pointV->mValue += deltaV;
         }
      }
   }
}

float FubbleModule::GetPlaybackTime(double time)
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

void FubbleModule::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mQuantizeLengthCheckbox->Draw();
   mQuantizeLengthSelector->SetShowing(mQuantizeLength);
   mQuantizeLengthSelector->Draw();
   mSpeedSlider->SetShowing(!mQuantizeLength);
   mSpeedSlider->Draw();
   mClearButton->Draw();
   mPerlinStrengthSlider->Draw();
   mPerlinScaleSlider->Draw();
   mPerlinSpeedSlider->Draw();
   mUpdatePerlinSeedButton->Draw();

   ofPushStyle();
   ofSetColor(100, 100, 100, 100);
   ofFill();

   ofPushMatrix();
   ofRectangle rect = GetFubbleRect();
   ofTranslate(rect.x, rect.y);
   ofRect(0, 0, rect.width, rect.height);

   if (mPerlinStrength > 0)
   {
      double perlinTime = gTime;
      const int kGridSize = 30;
      for (int col = 0; col < kGridSize; ++col)
      {
         for (int row = 0; row < kGridSize; ++row)
         {
            float x = col * (rect.width / kGridSize);
            float y = row * (rect.height / kGridSize);
            ofSetColor(GetPerlinNoiseValue(perlinTime, x / rect.width, y / rect.height, true) * 255, 0, GetPerlinNoiseValue(perlinTime, x / rect.width, y / rect.height, false) * 255, ofClamp(mPerlinStrength, 0, 1) * 255);
            ofRect(x, y, (rect.width / kGridSize) + .5f, (rect.height / kGridSize) + .5f, 0);
         }
      }
   }

   ofSetColor(GetColor(kModuleCategory_Modulator));
   ofPushMatrix();
   if (mAxisH.GetCableSource()->GetTarget())
      DrawTextNormal(mAxisH.GetCableSource()->GetTarget()->Name(), rect.width * .4f, rect.height - 2);
   ofRotate(-PI * .5f);
   if (mAxisV.GetCableSource()->GetTarget())
      DrawTextNormal(mAxisV.GetCableSource()->GetTarget()->Name(), -rect.height * .6f, rect.width - 2);
   ofPopMatrix();

   //draw curve
   ofPushStyle();
   ofSetColor(GetColor(kModuleCategory_Modulator));
   DrawTextNormal("length: " + ofToString(mLength, 2), 5, 15);
   ofSetColor(220, 220, 220);
   ofNoFill();
   ofBeginShape();
   for (float t = 0; t < mLength; t += .01f)
   {
      float x = mAxisH.mCurve.Evaluate(t) * rect.width;
      float y = (1 - mAxisV.mCurve.Evaluate(t)) * rect.height;

      ofVertex(x, y);
   }
   ofEndShape();

   //draw individual points
   /*ofFill();
   ofSetColor(220,220,220,100);
   int numPoints = mAxisH.mCurve.GetNumPoints();
   const int kMaxDrawPoints = 100;
   float step = MAX(1, numPoints/kMaxDrawPoints);
   for (float i=0; int(i)<numPoints; i+=step)
   {
      float x = mAxisH.mCurve.GetPoint(int(i))->mValue * rect.width;
      float y = (1 - mAxisV.mCurve.GetPoint(int(i))->mValue) * rect.height;
      //ofRect(x-3,y-3,6,6,0);
      ofCircle(x,y,2.5f);
   }*/

   ofFill();
   ofSetColor(255, 255, 255);
   if (mAxisH.mHasRecorded)
   {
      float time = mIsDrawing ? mRecordStartOffset : GetPlaybackTime(gTime);
      float currentX = mAxisH.mCurve.Evaluate(time, true) * rect.width;
      float currentY = (1 - mAxisV.mCurve.Evaluate(time, true)) * rect.height;
      ofCircle(currentX, currentY, 4);
   }

   if (mIsDrawing || IsHovered() || mIsRightClicking)
   {
      ofNoFill();
      ofSetColor(GetColor(kModuleCategory_Modulator), (mIsDrawing || mIsRightClicking) ? 255 : 50);
      ofCircle(GetFubbleMouseCoord().x * rect.width, (1 - GetFubbleMouseCoord().y) * rect.height, 5);
   }
   ofPopStyle();

   ofPopMatrix();

   ofPushMatrix();
   ofTranslate(10, mHeight - (kTimelineSectionHeight + kBottomControlHeight) + 3);

   ofSetColor(100, 100, 100, 100);
   ofRect(0, 0, mWidth - 20, 20);
   mAxisH.mCurve.SetDimensions(mWidth - 20, 20);
   mAxisH.mCurve.Render();
   ofSetColor(GetColor(kModuleCategory_Modulator));
   if (mAxisH.GetCableSource()->GetTarget())
      DrawTextNormal(mAxisH.GetCableSource()->GetTarget()->Name(), 5, 15);

   ofTranslate(0, 25);
   ofSetColor(100, 100, 100, 100);
   ofRect(0, 0, mWidth - 20, 20);
   mAxisV.mCurve.SetDimensions(mWidth - 20, 20);
   mAxisV.mCurve.Render();
   ofSetColor(GetColor(kModuleCategory_Modulator));
   if (mAxisV.GetCableSource()->GetTarget())
      DrawTextNormal(mAxisV.GetCableSource()->GetTarget()->Name(), 5, 15);

   ofPopMatrix();
   ofPopStyle();

   if (mLength > 0)
   {
      float playbackTime = GetPlaybackTime(gTime);
      float lineX = ofLerp(10, mWidth - 20, playbackTime / mLength);
      ofLine(lineX, mHeight - (kTimelineSectionHeight + kBottomControlHeight), lineX, mHeight - (kBottomControlHeight));
   }

   float rightAlign = mWidth - 5;
   float leftAlign = 5;
   if (mAxisH.GetCableSource()->GetTarget() && mAxisH.GetCableSource()->GetTarget()->GetRect().getCenter().x < GetRect().getCenter().x)
   {
      mAxisH.GetCableSource()->SetManualPosition(leftAlign, mHeight - (kBottomControlHeight + kTimelineSectionHeight) + 13);
      mAxisH.GetCableSource()->SetOverrideCableDir(ofVec2f(-1, 0), PatchCableSource::Side::kLeft);
   }
   else
   {
      mAxisH.GetCableSource()->SetManualPosition(rightAlign, mHeight - (kBottomControlHeight + kTimelineSectionHeight) + 13);
      mAxisH.GetCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   }

   if (mAxisV.GetCableSource()->GetTarget() && mAxisV.GetCableSource()->GetTarget()->GetRect().getCenter().x < GetRect().getCenter().x)
   {
      mAxisV.GetCableSource()->SetManualPosition(leftAlign, mHeight - (kBottomControlHeight + kTimelineSectionHeight) + 38);
      mAxisV.GetCableSource()->SetOverrideCableDir(ofVec2f(-1, 0), PatchCableSource::Side::kLeft);
   }
   else
   {
      mAxisV.GetCableSource()->SetManualPosition(rightAlign, mHeight - (kBottomControlHeight + kTimelineSectionHeight) + 38);
      mAxisV.GetCableSource()->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   }
}

float FubbleModule::GetPerlinNoiseValue(double time, float x, float y, bool horizontal)
{
   float perlinScale = mPerlinScale * 2.0f;
   float perlinSpeed = mPerlinSpeed * .002f;
   if (horizontal)
      return mNoise.noise(x * perlinScale, y * perlinScale, time * perlinSpeed + mPerlinSeed);
   else
      return mNoise.noise(y * perlinScale * .997f, x * perlinScale * .984f, time * 1.011f * perlinSpeed + 420 + mPerlinSeed);
}

void FubbleModule::DrawModuleUnclipped()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("(concept by @_ojack_)", 60, -3, 9);
}

ofRectangle FubbleModule::GetFubbleRect()
{
   return ofRectangle(10, kTopControlHeight, mWidth - 20, mHeight - (kTimelineSectionHeight + kBottomControlHeight + kTopControlHeight));
}

ofVec2f FubbleModule::GetFubbleMouseCoord()
{
   ofRectangle fubbleRect = GetFubbleRect();
   return ofVec2f(ofClamp((mMouseX - fubbleRect.x) / fubbleRect.width, 0, 1),
                  ofClamp(1 - ((mMouseY - fubbleRect.y) / fubbleRect.height), 0, 1));
}

void FubbleModule::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mMouseX = x;
   mMouseY = y;

   if (IsHovered())
   {
      if (right)
      {
         mIsRightClicking = true;
      }
      else
      {
         mIsDrawing = true;
         mRecordStartOffset = TheTransport->GetMeasureTime(gTime);
         Clear();
         RecordPoint();
      }
   }
}

void FubbleModule::Clear()
{
   mLength = 0;
   mSpeed = 1;
   mAxisH.mCurve.Clear();
   mAxisH.mHasRecorded = false;
   mAxisV.mCurve.Clear();
   mAxisV.mHasRecorded = false;
}

bool FubbleModule::IsHovered()
{
   return GetFubbleRect().contains(mMouseX, mMouseY);
}

void FubbleModule::RecordPoint()
{
   float time = TheTransport->GetMeasureTime(gTime) - mRecordStartOffset;
   auto coord = GetFubbleMouseCoord();
   mAxisH.mCurve.AddPointAtEnd(CurvePoint(time, coord.x));
   mAxisV.mCurve.AddPointAtEnd(CurvePoint(time, coord.y));
   mAxisH.mCurve.SetExtents(0, time);
   mAxisV.mCurve.SetExtents(0, time);
   mAxisH.mHasRecorded = true;
   mAxisV.mHasRecorded = true;
   mLength = time;
}

void FubbleModule::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mIsRightClicking = false;

   if (mIsDrawing)
   {
      mIsDrawing = false;
      if (mQuantizeLength)
      {
         float quantizeResolution = TheTransport->GetMeasureFraction(mQuantizeInterval);
         int quantizeIntervalSteps = juce::roundToInt(mLength / quantizeResolution);
         if (quantizeIntervalSteps <= 0)
            quantizeIntervalSteps = 1;
         float quantizedLength = quantizeResolution * quantizeIntervalSteps;
         mLength = quantizedLength;
         mAxisH.mCurve.SetExtents(0, mLength);
         mAxisV.mCurve.SetExtents(0, mLength);
      }
      mRecordStartOffset = fmod(mRecordStartOffset, mLength);
   }
}

bool FubbleModule::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mMouseX = x;
   mMouseY = y;

   return false;
}

void FubbleModule::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mAxisH.GetCableSource())
      mAxisH.UpdateControl();

   if (cableSource == mAxisV.GetCableSource())
      mAxisV.UpdateControl();
}

void FubbleModule::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void FubbleModule::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   /*int newSteps = int(mLength/4.0f * TheTransport->CountInStandardMeasure(mInterval));
   if (list == mIntervalSelector)
   {
      if (newSteps > 0)
      {
         SetNumSteps(newSteps, true);
      }
      else
      {
         mInterval = (NoteInterval)oldVal;
      }
   }
   if (list == mLengthSelector)
   {
      if (newSteps > 0)
         SetNumSteps(newSteps, false);
      else
         mLength = oldVal;
   }*/
}

void FubbleModule::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
      Clear();
   if (button == mUpdatePerlinSeedButton)
      UpdatePerlinSeed();
}

void FubbleModule::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void FubbleModule::Resize(float w, float h)
{
   w = MAX(w, 211);
   h = MAX(h, 180);
   mPerlinStrengthSlider->SetPosition(mPerlinStrengthSlider->GetPosition(true).x, mPerlinStrengthSlider->GetPosition(true).y + h - mHeight);
   mPerlinScaleSlider->SetPosition(mPerlinScaleSlider->GetPosition(true).x, mPerlinScaleSlider->GetPosition(true).y + h - mHeight);
   mPerlinSpeedSlider->SetPosition(mPerlinSpeedSlider->GetPosition(true).x, mPerlinSpeedSlider->GetPosition(true).y + h - mHeight);
   mUpdatePerlinSeedButton->SetPosition(mUpdatePerlinSeedButton->GetPosition(true).x, mUpdatePerlinSeedButton->GetPosition(true).y + h - mHeight);
   mWidth = w;
   mHeight = h;
}

void FubbleModule::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["uicontrol_h"] = mAxisH.GetCableSource()->GetTarget() ? mAxisH.GetCableSource()->GetTarget()->Path() : "";
   moduleInfo["uicontrol_v"] = mAxisV.GetCableSource()->GetTarget() ? mAxisV.GetCableSource()->GetTarget()->Path() : "";
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void FubbleModule::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol_h", moduleInfo);
   mModuleSaveData.LoadString("uicontrol_v", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, mWidth, 120, 1000);
   mModuleSaveData.LoadInt("height", moduleInfo, mHeight, 15, 1000);

   SetUpFromSaveData();
}

void FubbleModule::SetUpFromSaveData()
{
   {
      std::string controlPathH = mModuleSaveData.GetString("uicontrol_h");
      if (!controlPathH.empty())
      {
         auto uicontrol = TheSynth->FindUIControl(controlPathH);
         if (uicontrol)
         {
            mAxisH.GetCableSource()->SetTarget(uicontrol);
            mAxisH.UpdateControl();
         }
      }
   }

   {
      std::string controlPathV = mModuleSaveData.GetString("uicontrol_v");
      if (!controlPathV.empty())
      {
         auto uicontrol = TheSynth->FindUIControl(controlPathV);
         if (uicontrol)
         {
            mAxisV.GetCableSource()->SetTarget(uicontrol);
            mAxisV.UpdateControl();
         }
      }
   }

   Resize(mModuleSaveData.GetInt("width"), mModuleSaveData.GetInt("height"));
}

void FubbleModule::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mAxisH.mCurve.SaveState(out);
   mAxisV.mCurve.SaveState(out);
   out << mAxisH.mHasRecorded << mAxisV.mHasRecorded;
   out << mLength;
   out << mRecordStartOffset;
}

void FubbleModule::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   mAxisH.mCurve.LoadState(in);
   mAxisV.mCurve.LoadState(in);

   if (rev >= 2)
      in >> mAxisH.mHasRecorded >> mAxisV.mHasRecorded;

   if (rev >= 3)
   {
      in >> mLength;
      mAxisH.mCurve.SetExtents(0, mLength);
      mAxisV.mCurve.SetExtents(0, mLength);
      in >> mRecordStartOffset;
   }
}

float FubbleModule::FubbleAxis::Value(int samplesIn)
{
   float playbackTime = mOwner->GetPlaybackTime(gTime + samplesIn * gInvSampleRateMs);
   float val;
   if (mOwner->mIsDrawing || mOwner->mIsRightClicking)
      val = mIsHorizontal ? mOwner->GetFubbleMouseCoord().x : mOwner->GetFubbleMouseCoord().y;
   else
      val = mCurve.Evaluate(playbackTime, true);
   return ofMap(val, 0, 1, GetMin(), GetMax(), K(clamp));
}
