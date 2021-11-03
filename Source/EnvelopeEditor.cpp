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

    EnvelopeEditor.cpp
    Created: 9 Nov 2017 5:20:48pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "EnvelopeEditor.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Checkbox.h"

namespace
{
   float pointClickRadius = 4;
}

EnvelopeControl::EnvelopeControl(ofVec2f position, ofVec2f dimensions)
: mPosition(position)
, mDimensions(dimensions)
, mAdsr(nullptr)
, mClick(false)
, mViewLength(2000)
, mHighlightPoint(-1)
, mHighlightCurve(-1)
, mLastClickTime(0)
, mFixedLengthMode(false)
{
   
}

void EnvelopeControl::Draw()
{
   ofPushStyle();
   
   ofSetColor(100,100,.8f*gModuleDrawAlpha);
   
   ofSetLineWidth(.5f);
   ofRect(mPosition.x, mPosition.y, mDimensions.x, mDimensions.y, 0);
   
   ofSetColor(245, 58, 0, gModuleDrawAlpha);
   
   mViewAdsr.Set(*mAdsr);
   mViewAdsr.Clear();
   mViewAdsr.Start(0,1);
   mViewAdsr.Stop(GetReleaseTime());
   
   ofSetLineWidth(1);
   ofBeginShape();
   for (float i=0; i<mDimensions.x; i+=(.25f/gDrawScale))
   {
      float time = i/mDimensions.x * mViewLength;
      
      if (time < GetPreSustainTime())
      {
         float value = mViewAdsr.Value(time);
         AddVertex(i+mPosition.x, GetYForValue(value));
      }
      else
      {
         break;
      }
   }
   AddVertex(GetXForTime(GetPreSustainTime()),
             GetYForValue(mViewAdsr.Value(GetPreSustainTime())));
   ofEndShape(false);
   
   ofPushStyle();
   ofSetColor(0, 58, 245, gModuleDrawAlpha);
   ofLine(ofClamp(GetXForTime(GetPreSustainTime()), mPosition.x, mPosition.x + mDimensions.x),
          GetYForValue(mViewAdsr.Value(GetPreSustainTime())),
          ofClamp(GetXForTime(GetReleaseTime()), mPosition.x, mPosition.x + mDimensions.x),
          GetYForValue(mViewAdsr.Value(GetReleaseTime())));
   ofPopStyle();
   
   ofBeginShape();
   AddVertex(GetXForTime(GetReleaseTime()),
             GetYForValue(mViewAdsr.Value(GetReleaseTime())));
   for (float i=0; i<mDimensions.x; i+=(.25f/gDrawScale))
   {
      float time = i/mDimensions.x * mViewLength;
      
      if (time >= GetReleaseTime())
      {
         float value = mViewAdsr.Value(time);
         AddVertex(i+mPosition.x, GetYForValue(value));
      }
   }
   ofEndShape(false);
   
   ofSetLineWidth(3);
   ofBeginShape();
   bool started = false;
   for (float i=0; i<mDimensions.x; i+=(.25f/gDrawScale))
   {
      float time = i/mDimensions.x * mViewLength;
      if (mViewAdsr.GetStageForTime(time) == mHighlightCurve)
      {
         if (!started)
         {
            started = true;
            double stageStartTime;
            mViewAdsr.GetStage(time, stageStartTime);
            AddVertex(GetXForTime(stageStartTime), GetYForValue(mViewAdsr.Value(stageStartTime)));
         }
         
         if (mViewAdsr.GetHasSustainStage() && mHighlightCurve == mViewAdsr.GetSustainStage() && time > GetPreSustainTime())
            break;
         
         float value = mViewAdsr.Value(time);
         AddVertex(i+mPosition.x, GetYForValue(value));
      }
   }
   ofEndShape(false);
   
   float time = 0;
   ofSetLineWidth(.5f);
   for (int i=0; i<mViewAdsr.GetNumStages(); ++i)
   {
      if (mViewAdsr.GetHasSustainStage() && i == mViewAdsr.GetSustainStage() + 1)
         time = GetReleaseTime();
      
      time += mViewAdsr.GetStageData(i).time;
      
      if (i == mViewAdsr.GetNumStages()-1)
         time += 0;
      
      float value = mViewAdsr.Value(time);
      if (i == mHighlightPoint)
         ofFill();
      else
         ofNoFill();
      
      float x = GetXForTime(time);
      if (x < mPosition.x + mDimensions.x)
         ofCircle(x, GetYForValue(value), i == mHighlightPoint ? 8 : pointClickRadius);
   }
   
   ofSetLineWidth(1);
   ofSetColor(0,255,0,gModuleDrawAlpha * .5f);
   float drawTime = 0;
   if (mAdsr->GetStartTime(gTime) > 0 && mAdsr->GetStartTime(gTime) >= mAdsr->GetStopTime(gTime))
      drawTime = ofClamp(gTime - mAdsr->GetStartTime(gTime), 0, GetReleaseTime());
   if (mAdsr->GetStopTime(gTime) > mAdsr->GetStartTime(gTime))
      drawTime = GetReleaseTime() + (gTime - mAdsr->GetStopTime(gTime));
   if (drawTime > 0 && drawTime < mViewLength)
      ofLine(GetXForTime(drawTime), mPosition.y, GetXForTime(drawTime), mPosition.y + mDimensions.y);
   
   ofPopStyle();
}

void EnvelopeControl::OnClicked(int x, int y, bool right)
{
   if (x > mPosition.x-pointClickRadius &&
       x < mPosition.x + mDimensions.x + pointClickRadius &&
       y > mPosition.y - pointClickRadius &&
       y < mPosition.y + mDimensions.y + pointClickRadius)
   {
      mClick = true;
      
      if (right)
      {
         if (mHighlightCurve != -1)
         {
            mAdsr->GetStageData(mHighlightCurve).curve = 0;
            mViewAdsr.Set(*mAdsr);
         }
         if (mHighlightPoint != -1)
         {
            if (mHighlightPoint < mAdsr->GetNumStages()-1)
            {
               mAdsr->GetStageData(mHighlightPoint+1).time += mAdsr->GetStageData(mHighlightPoint).time;
               mAdsr->GetStageData(mHighlightPoint+1).curve = 0;
               
               for (int i=mHighlightPoint; i<mAdsr->GetNumStages(); ++i)
               {
                  mAdsr->GetStageData(i).time = mAdsr->GetStageData(i+1).time;
                  mAdsr->GetStageData(i).target = mAdsr->GetStageData(i+1).target;
                  mAdsr->GetStageData(i).curve = mAdsr->GetStageData(i+1).curve;
               }
               
               mAdsr->SetNumStages(mAdsr->GetNumStages() - 1);
               if (mAdsr->GetHasSustainStage() &&
                   mHighlightPoint <= mAdsr->GetSustainStage())
                  mAdsr->SetSustainStage(mAdsr->GetSustainStage()-1);
               mViewAdsr.Set(*mAdsr);
               
               mHighlightPoint = -1;
            }
         }
      }
      else if (gTime < mLastClickTime + 500 &&
               mHighlightCurve != -1 &&
               (mClickStart - ofVec2f(x,y)).lengthSquared() < pointClickRadius*pointClickRadius)
      {
         float clickTime = GetTimeForX(x);
         if (clickTime > GetPreSustainTime())
            clickTime -= GetReleaseTime() - GetPreSustainTime();
         
         for (int i=mAdsr->GetNumStages(); i>mHighlightCurve; --i)
         {
            mAdsr->GetStageData(i).time = mAdsr->GetStageData(i-1).time;
            mAdsr->GetStageData(i).target = mAdsr->GetStageData(i-1).target;
            mAdsr->GetStageData(i).curve = mAdsr->GetStageData(i-1).curve;
         }
         float priorStageTimes = 0;
         for (int i=0; i<mHighlightCurve; ++i)
            priorStageTimes += mAdsr->GetStageData(i).time;
         mAdsr->GetStageData(mHighlightCurve).time = clickTime - priorStageTimes;
         mAdsr->GetStageData(mHighlightCurve).target = GetValueForY(y);
         mAdsr->GetStageData(mHighlightCurve+1).time -= mAdsr->GetStageData(mHighlightCurve).time;
         mAdsr->SetNumStages(mAdsr->GetNumStages() + 1);
         if (mAdsr->GetHasSustainStage() &&
             mHighlightCurve <= mAdsr->GetSustainStage())
            mAdsr->SetSustainStage(mAdsr->GetSustainStage()+1);
         mViewAdsr.Set(*mAdsr);
         
         mHighlightPoint = mHighlightCurve;
         mHighlightCurve = -1;
      }
      
      mLastClickTime = gTime;
      mClickStart.set(x,y);
      mClickAdsr.Set(mViewAdsr);
   }
}

void EnvelopeControl::MouseReleased()
{
   mClick = false;
}

void EnvelopeControl::MouseMoved(float x, float y)
{
   if (!mClick)
   {
      mViewAdsr.Set(*mAdsr);
      
      mHighlightPoint = -1;
      float time = 0;
      for (int i=0; i<mViewAdsr.GetNumStages(); ++i)
      {
         if (mViewAdsr.GetHasSustainStage() && i == mViewAdsr.GetSustainStage() + 1)
            time = GetReleaseTime();
         
         time += mViewAdsr.GetStageData(i).time;
         
         if (i == mViewAdsr.GetNumStages()-1)
            time += 0;
         
         float value = mViewAdsr.Value(time);
         
         float pointX = GetXForTime(time);
         float pointY = GetYForValue(value);
         
         if (fabsf(x-pointX) < 4 && fabsf(y-pointY) < 4)
         {
            mHighlightPoint = i;
            mHighlightCurve = -1;
         }
      }
      
      if (mHighlightPoint == -1)
      {
         float time = GetTimeForX(x);
         float valueForY = GetValueForY(y);
         if (abs(mViewAdsr.Value(time) - valueForY) < .1f)
         {
            mHighlightCurve = mViewAdsr.GetStageForTime(time);
            if (mViewAdsr.GetHasSustainStage() && mHighlightCurve == mViewAdsr.GetSustainStage() && time > GetPreSustainTime())
               mHighlightCurve = -1;
         }
         else
         {
            mHighlightCurve = -1;
         }
      }
   }
   else
   {
      if (mHighlightPoint != -1)
      {
         ::ADSR::Stage& stage = mAdsr->GetStageData(mHighlightPoint);
         ::ADSR::Stage& originalStage = mClickAdsr.GetStageData(mHighlightPoint);
         
         float maxLength = 10000;
         if (mFixedLengthMode)
         {
            if (mHighlightPoint < mAdsr->GetNumStages() - 1)
               maxLength = stage.time + mAdsr->GetStageData(mHighlightPoint+1).time - 1;
            else if (mHighlightPoint == mAdsr->GetNumStages() - 1)
            {
               float time = 0;
               for (int i=0; i < mViewAdsr.GetNumStages()-1; ++i)
                  time += mViewAdsr.GetStageData(i).time;
               maxLength = mViewLength - time - 1;
            }
         }
         
         stage.time = ofClamp(originalStage.time + (x - mClickStart.x)/mDimensions.x * mViewLength, 0.001f, maxLength);
         
         if (mFixedLengthMode && mHighlightPoint < mAdsr->GetNumStages() - 1)
         {
            float timeAdjustment = stage.time - originalStage.time;
            mAdsr->GetStageData(mHighlightPoint+1).time = mClickAdsr.GetStageData(mHighlightPoint+1).time - timeAdjustment;
         }
         
         if (mHighlightPoint < mAdsr->GetNumStages()-1 || mAdsr->GetFreeReleaseLevel())
            stage.target = ofClamp(originalStage.target + ((mClickStart.y - y) / mDimensions.y), 0, 1);
         else
            stage.target = 0;
      }
      
      if (mHighlightCurve != -1)
      {
         ::ADSR::Stage& stage = mAdsr->GetStageData(mHighlightCurve);
         ::ADSR::Stage& originalStage = mClickAdsr.GetStageData(mHighlightCurve);
         
         stage.curve = ofClamp(originalStage.curve + ((mClickStart.y - y) / mDimensions.y), -1, 1);
      }
   }
}

void EnvelopeControl::AddVertex(float x, float y)
{
   if (x >= mPosition.x && x <= mPosition.x + mDimensions.x)
      ofVertex(x, y);
}

float EnvelopeControl::GetPreSustainTime()
{
   float time = 0;
   for (int i=0; i<=mViewAdsr.GetSustainStage() || (!mViewAdsr.GetHasSustainStage() && i < mViewAdsr.GetNumStages()); ++i)
      time += mViewAdsr.GetStageData(i).time;
   
   return time;
}

float EnvelopeControl::GetReleaseTime()
{
   if (!mViewAdsr.GetHasSustainStage())
      return GetPreSustainTime();
   if (mViewAdsr.GetMaxSustain() > 0)
      return GetPreSustainTime() + mViewAdsr.GetMaxSustain();
   else
      return GetPreSustainTime() + mViewLength * .2f;
}

float EnvelopeControl::GetTimeForX(float x)
{
   return ofMap(x,mPosition.x,mPosition.x+mDimensions.x,0,mViewLength);
}

float EnvelopeControl::GetValueForY(float y)
{
   return ofMap(y,mPosition.y,mPosition.y+mDimensions.y,1,0);
}

float EnvelopeControl::GetXForTime(float time)
{
   return time/mViewLength*mDimensions.x + mPosition.x;
}

float EnvelopeControl::GetYForValue(float value)
{
   return mDimensions.y * (1-value) + mPosition.y;
}

EnvelopeEditor::EnvelopeEditor()
: mEnvelopeControl(ofVec2f(10,40), ofVec2f(380,200))
, mADSRDisplay(nullptr)
, mPinned(false)
, mADSRViewLength(2000)
, mADSRViewLengthSlider(nullptr)
, mHasSustainStageCheckbox(nullptr)
, mSustainStageSlider(nullptr)
, mMaxSustainSlider(nullptr)
, mFreeReleaseLevelCheckbox(nullptr)
, mTargetCable(nullptr)
{
   mEnvelopeControl.SetViewLength(mADSRViewLength);
}

void EnvelopeEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mADSRViewLengthSlider = new FloatSlider(this,"length",2,20,120,15,&mADSRViewLength,10,10000);
   mPinButton = new ClickButton(this,"pin",3,2);
   
   static bool dummyBool;
   static int dummyInt;
   static float dummyFloat;
   
   mHasSustainStageCheckbox = new Checkbox(this, "has sustain", 2, mEnvelopeControl.GetPosition().y + mEnvelopeControl.GetDimensions().y + 6, &dummyBool);
   mSustainStageSlider = new IntSlider(this, "sustain stage", mHasSustainStageCheckbox, kAnchor_Right, 100, 15, &dummyInt, 1, MAX_ADSR_STAGES-1);
   mMaxSustainSlider = new FloatSlider(this, "max sustain", mSustainStageSlider, kAnchor_Right, 100, 15, &dummyFloat, -1, 5000);
   mFreeReleaseLevelCheckbox = new Checkbox(this, "free release", mHasSustainStageCheckbox, kAnchor_Below, &dummyBool);
   
   mADSRViewLengthSlider->SetMode(FloatSlider::kSquare);
   mMaxSustainSlider->SetMode(FloatSlider::kSquare);
}

EnvelopeEditor::~EnvelopeEditor()
{
}

void EnvelopeEditor::SetADSRDisplay(ADSRDisplay* adsrDisplay)
{
   mEnvelopeControl.SetADSR(adsrDisplay->GetADSR());
   
   mADSRDisplay = adsrDisplay;
   mADSRViewLength = adsrDisplay->GetMaxTime()+10;
   mEnvelopeControl.SetViewLength(mADSRViewLength);
   mHasSustainStageCheckbox->SetVar(&adsrDisplay->GetADSR()->GetHasSustainStage());
   mSustainStageSlider->SetVar(&adsrDisplay->GetADSR()->GetSustainStage());
   mMaxSustainSlider->SetVar(&adsrDisplay->GetADSR()->GetMaxSustain());
   mFreeReleaseLevelCheckbox->SetVar(&adsrDisplay->GetADSR()->GetFreeReleaseLevel());
   
   mSustainStageSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
   mMaxSustainSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
}

void EnvelopeEditor::DoSpecialDelete()
{
   mEnabled = false;
   mPinned = false;
}

void EnvelopeEditor::DrawModule()
{
   if (!mPinned)
   {
      float w,h;
      GetDimensions(w,h);
      
      ofPushStyle();
      ofSetColor(0,0,0);
      ofFill();
      ofSetLineWidth(.5f);
      ofRect(0,0,w,h);
      ofNoFill();
      ofSetColor(255,255,255);
      ofRect(0,0,w,h);
      ofPopStyle();
   }
   
   if (Minimized())
      return;
   
   mSustainStageSlider->SetExtents(1, mADSRDisplay->GetADSR()->GetNumStages() - 2);
   
   mADSRViewLengthSlider->Draw();
   mHasSustainStageCheckbox->Draw();
   mSustainStageSlider->Draw();
   mMaxSustainSlider->Draw();
   mFreeReleaseLevelCheckbox->Draw();
   if (!mPinned)
      mPinButton->Draw();
   
   mEnvelopeControl.Draw();
}

void EnvelopeEditor::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   mEnvelopeControl.OnClicked(x,y,right);
}

void EnvelopeEditor::MouseReleased()
{
   IDrawableModule::MouseReleased();
   
   mEnvelopeControl.MouseReleased();
}

bool EnvelopeEditor::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   
   mEnvelopeControl.MouseMoved(x, y);
   
   return false;
}

void EnvelopeEditor::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mHasSustainStageCheckbox)
   {
      mSustainStageSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
      mMaxSustainSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
   }
}

void EnvelopeEditor::ButtonClicked(ClickButton* button)
{
   if (button == mPinButton)
   {
      if (!mPinned)
      {
         TheSynth->AddDynamicModule(this);
         TheSynth->PopModalFocusItem();
         
         SetName(GetUniqueName("envelopeeditor", TheSynth->GetModuleNames<EnvelopeEditor*>()).c_str());
         
         Pin();
      }
   }
}

void EnvelopeEditor::Pin()
{
   mPinned = true;
   if (mTargetCable == nullptr)
   {
      mTargetCable = new PatchCableSource(this, kConnectionType_Modulator);
      AddPatchCableSource(mTargetCable);
      mTargetCable->SetTarget(mADSRDisplay);
      mTargetCable->SetClickable(false);
   }
}

void EnvelopeEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mADSRViewLengthSlider)
   {
      mEnvelopeControl.SetViewLength(mADSRViewLength);
      if (mADSRDisplay != nullptr)
         mADSRDisplay->SetMaxTime(mADSRViewLength);
   }
}

void EnvelopeEditor::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   if (mADSRDisplay != nullptr)
      moduleInfo["target"] = mADSRDisplay->Path();
   
   moduleInfo["onthefly"] = false;
}

void EnvelopeEditor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   if (moduleInfo["onthefly"] == false)
   {
      SetUpFromSaveData();
      Pin();
   }
}

void EnvelopeEditor::SetUpFromSaveData()
{
   SetADSRDisplay(dynamic_cast<ADSRDisplay*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target"))));
}
