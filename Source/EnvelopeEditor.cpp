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
#include "UIControlMacros.h"

namespace
{
   float pointClickRadius = 4;
}

EnvelopeControl::EnvelopeControl(ofVec2f position, ofVec2f dimensions, EnvelopeEditor* editor)
: mPosition(position)
, mDimensions(dimensions)
, mEditor(editor)
{
}

void EnvelopeControl::Draw()
{
   ofPushStyle();

   ofSetColor(100, 100, .8f * gModuleDrawAlpha);

   ofSetLineWidth(.5f);
   ofRect(mPosition.x, mPosition.y, mDimensions.x, mDimensions.y, 0);

   ofSetColor(245, 58, 0, gModuleDrawAlpha);

   if (mAdsr != nullptr)
   {
      ADSR::EventInfo adsrEvent(0, GetReleaseTime());

      ofSetLineWidth(1);
      ofBeginShape();
      for (float i = 0; i < mDimensions.x; i += (.25f / gDrawScale))
      {
         float time = i / mDimensions.x * mViewLength;

         if (time < GetPreSustainTime())
         {
            float value = mAdsr->Value(time, &adsrEvent);
            AddVertex(i + mPosition.x, GetYForValue(value));
         }
         else
         {
            break;
         }
      }
      AddVertex(GetXForTime(GetPreSustainTime()),
                GetYForValue(mAdsr->Value(GetPreSustainTime(), &adsrEvent)));
      ofEndShape(false);

      ofPushStyle();
      ofSetColor(0, 58, 245, gModuleDrawAlpha);
      ofLine(ofClamp(GetXForTime(GetPreSustainTime()), mPosition.x, mPosition.x + mDimensions.x),
             GetYForValue(mAdsr->Value(GetPreSustainTime(), &adsrEvent)),
             ofClamp(GetXForTime(GetReleaseTime()), mPosition.x, mPosition.x + mDimensions.x),
             GetYForValue(mAdsr->Value(GetReleaseTime(), &adsrEvent)));
      ofPopStyle();

      ofBeginShape();
      AddVertex(GetXForTime(GetReleaseTime()),
                GetYForValue(mAdsr->Value(GetReleaseTime(), &adsrEvent)));
      for (float i = 0; i < mDimensions.x; i += (.25f / gDrawScale))
      {
         float time = i / mDimensions.x * mViewLength;

         if (time >= GetReleaseTime())
         {
            float value = mAdsr->Value(time, &adsrEvent);
            AddVertex(i + mPosition.x, GetYForValue(value));
         }
      }
      ofEndShape(false);

      ofSetLineWidth(3);
      ofBeginShape();
      bool started = false;
      for (float i = 0; i < mDimensions.x; i += (.25f / gDrawScale))
      {
         float time = i / mDimensions.x * mViewLength;
         double stageStartTime;
         if (mAdsr->GetStage(time, stageStartTime, &adsrEvent) == mHighlightCurve)
         {
            if (!started)
            {
               started = true;
               AddVertex(GetXForTime(stageStartTime), GetYForValue(mAdsr->Value(stageStartTime, &adsrEvent)));
            }

            if (mAdsr->GetHasSustainStage() && mHighlightCurve == mAdsr->GetSustainStage() && time > GetPreSustainTime())
               break;

            float value = mAdsr->Value(time, &adsrEvent);
            AddVertex(i + mPosition.x, GetYForValue(value));
         }
      }
      ofEndShape(false);

      float time = 0;
      ofSetLineWidth(.5f);
      for (int i = 0; i < mAdsr->GetNumStages(); ++i)
      {
         if (mAdsr->GetHasSustainStage() && i == mAdsr->GetSustainStage() + 1)
            time = GetReleaseTime();

         time += mAdsr->GetStageData(i).time;

         if (i == mAdsr->GetNumStages() - 1)
            time += 0;

         float value = mAdsr->Value(time, &adsrEvent);
         if (i == mHighlightPoint)
            ofFill();
         else
            ofNoFill();

         float x = GetXForTime(time);
         if (x < mPosition.x + mDimensions.x)
            ofCircle(x, GetYForValue(value), i == mHighlightPoint ? 8 : pointClickRadius);
      }

      ofSetLineWidth(1);
      ofSetColor(0, 255, 0, gModuleDrawAlpha * .5f);
      float drawTime = 0;
      if (mAdsr->GetStartTime(gTime) > 0 && mAdsr->GetStartTime(gTime) >= mAdsr->GetStopTime(gTime))
         drawTime = ofClamp(gTime - mAdsr->GetStartTime(gTime), 0, GetReleaseTime());
      if (mAdsr->GetStopTime(gTime) > mAdsr->GetStartTime(gTime))
         drawTime = GetReleaseTime() + (gTime - mAdsr->GetStopTime(gTime));
      if (drawTime > 0 && drawTime < mViewLength)
         ofLine(GetXForTime(drawTime), mPosition.y, GetXForTime(drawTime), mPosition.y + mDimensions.y);
   }

   ofPopStyle();
}

void EnvelopeControl::OnClicked(float x, float y, bool right)
{
   if (x > mPosition.x - pointClickRadius &&
       x < mPosition.x + mDimensions.x + pointClickRadius &&
       y > mPosition.y - pointClickRadius &&
       y < mPosition.y + mDimensions.y + pointClickRadius &&
       mAdsr != nullptr)
   {
      mClick = true;

      if (right)
      {
         if (mHighlightCurve != -1)
         {
            mAdsr->GetStageData(mHighlightCurve).curve = 0;
         }
         if (mHighlightPoint != -1)
         {
            if (mHighlightPoint < mAdsr->GetNumStages() - 1)
            {
               mAdsr->GetStageData(mHighlightPoint + 1).time += mAdsr->GetStageData(mHighlightPoint).time;
               mAdsr->GetStageData(mHighlightPoint + 1).curve = mAdsr->GetStageData(mHighlightPoint).curve;

               for (int i = mHighlightPoint; i < mAdsr->GetNumStages(); ++i)
               {
                  mAdsr->GetStageData(i).time = mAdsr->GetStageData(i + 1).time;
                  mAdsr->GetStageData(i).target = mAdsr->GetStageData(i + 1).target;
                  mAdsr->GetStageData(i).curve = mAdsr->GetStageData(i + 1).curve;
               }

               mAdsr->SetNumStages(mAdsr->GetNumStages() - 1);
               if (mAdsr->GetHasSustainStage() &&
                   mHighlightPoint <= mAdsr->GetSustainStage())
                  mAdsr->SetSustainStage(mAdsr->GetSustainStage() - 1);

               mHighlightPoint = -1;
            }
         }
      }
      else if (gTime < mLastClickTime + 500 &&
               mHighlightCurve != -1 &&
               (mClickStart - ofVec2f(x, y)).lengthSquared() < pointClickRadius * pointClickRadius)
      {
         float clickTime = GetTimeForX(x);
         if (clickTime > GetPreSustainTime())
            clickTime -= GetReleaseTime() - GetPreSustainTime();

         for (int i = mAdsr->GetNumStages(); i > mHighlightCurve; --i)
         {
            mAdsr->GetStageData(i).time = mAdsr->GetStageData(i - 1).time;
            mAdsr->GetStageData(i).target = mAdsr->GetStageData(i - 1).target;
            mAdsr->GetStageData(i).curve = mAdsr->GetStageData(i - 1).curve;
         }
         float priorStageTimes = 0;
         for (int i = 0; i < mHighlightCurve; ++i)
            priorStageTimes += mAdsr->GetStageData(i).time;
         mAdsr->GetStageData(mHighlightCurve).time = clickTime - priorStageTimes;
         mAdsr->GetStageData(mHighlightCurve).target = GetValueForY(y);
         mAdsr->GetStageData(mHighlightCurve + 1).time -= mAdsr->GetStageData(mHighlightCurve).time;
         mAdsr->SetNumStages(mAdsr->GetNumStages() + 1);
         if (mAdsr->GetHasSustainStage() &&
             mHighlightCurve <= mAdsr->GetSustainStage())
            mAdsr->SetSustainStage(mAdsr->GetSustainStage() + 1);

         mHighlightPoint = mHighlightCurve;
         mHighlightCurve = -1;
      }

      mLastClickTime = gTime;
      mClickStart.set(x, y);
      mClickAdsr.Set(*mAdsr);
   }
}

void EnvelopeControl::MouseReleased()
{
   mClick = false;
}

void EnvelopeControl::MouseMoved(float x, float y)
{
   if (mAdsr == nullptr)
      return;

   if (!mClick)
   {
      ADSR::EventInfo adsrEvent(0, GetReleaseTime());

      mHighlightPoint = -1;
      float time = 0;
      for (int i = 0; i < mAdsr->GetNumStages(); ++i)
      {
         if (mAdsr->GetHasSustainStage() && i == mAdsr->GetSustainStage() + 1)
            time = GetReleaseTime();

         time += mAdsr->GetStageData(i).time;

         if (i == mAdsr->GetNumStages() - 1)
            time += 0;

         float value = mAdsr->Value(time, &adsrEvent);

         float pointX = GetXForTime(time);
         float pointY = GetYForValue(value);

         if (fabsf(x - pointX) < 4 && fabsf(y - pointY) < 4)
         {
            mHighlightPoint = i;
            mHighlightCurve = -1;
         }
      }

      if (mHighlightPoint == -1)
      {
         float highlightTime = GetTimeForX(x);
         float valueForY = GetValueForY(y);
         double stageStartTime;
         if (abs(mAdsr->Value(highlightTime, &adsrEvent) - valueForY) < .1f)
         {
            mHighlightCurve = mAdsr->GetStage(highlightTime, stageStartTime, &adsrEvent);
            if (mAdsr->GetHasSustainStage() && mHighlightCurve == mAdsr->GetSustainStage() && highlightTime > GetPreSustainTime())
               mHighlightCurve = -1;
            if (mHighlightCurve >= mAdsr->GetNumStages())
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
               maxLength = stage.time + mAdsr->GetStageData(mHighlightPoint + 1).time - 1;
            else if (mHighlightPoint == mAdsr->GetNumStages() - 1)
            {
               float time = 0;
               for (int i = 0; i < mAdsr->GetNumStages() - 1; ++i)
                  time += mAdsr->GetStageData(i).time;
               maxLength = mViewLength - time - 1;
            }
         }

         stage.time = ofClamp(originalStage.time + (x - mClickStart.x) / mDimensions.x * mViewLength, 0.001f, maxLength);

         if (mFixedLengthMode && mHighlightPoint < mAdsr->GetNumStages() - 1)
         {
            float timeAdjustment = stage.time - originalStage.time;
            mAdsr->GetStageData(mHighlightPoint + 1).time = mClickAdsr.GetStageData(mHighlightPoint + 1).time - timeAdjustment;
         }

         if (mHighlightPoint < mAdsr->GetNumStages() - 1 || mAdsr->GetFreeReleaseLevel())
            stage.target = ofClamp(originalStage.target + ((mClickStart.y - y) / mDimensions.y), 0, 1);
         else
            stage.target = 0;
      }

      if (mHighlightCurve != -1)
      {
         ::ADSR::Stage& stage = mAdsr->GetStageData(mHighlightCurve);
         ::ADSR::Stage& originalStage = mClickAdsr.GetStageData(mHighlightCurve);
         ofVec2f sliderLimits{ -2, 2 };
         if (mEditor)
            sliderLimits = mEditor->GetCurveSliderExtentsForStage(mHighlightCurve);
         stage.curve = ofClamp(originalStage.curve + ((mClickStart.y - y) / mDimensions.y), sliderLimits.x, sliderLimits.y);
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
   if (mAdsr != nullptr)
   {
      for (int i = 0; i <= mAdsr->GetSustainStage() || (!mAdsr->GetHasSustainStage() && i < mAdsr->GetNumStages()); ++i)
         time += mAdsr->GetStageData(i).time;
   }

   return time;
}

float EnvelopeControl::GetReleaseTime()
{
   if (mAdsr != nullptr && !mAdsr->GetHasSustainStage())
      return GetPreSustainTime();
   if (mAdsr != nullptr && mAdsr->GetMaxSustain() > 0)
      return GetPreSustainTime() + mAdsr->GetMaxSustain();
   else
      return GetPreSustainTime() + mViewLength * .2f;
}

float EnvelopeControl::GetTimeForX(float x)
{
   return ofMap(x, mPosition.x, mPosition.x + mDimensions.x, 0, mViewLength);
}

float EnvelopeControl::GetValueForY(float y)
{
   return ofMap(y, mPosition.y, mPosition.y + mDimensions.y, 1, 0);
}

float EnvelopeControl::GetXForTime(float time)
{
   return time / mViewLength * mDimensions.x + mPosition.x;
}

float EnvelopeControl::GetYForValue(float value)
{
   return mDimensions.y * (1 - value) + mPosition.y;
}

EnvelopeEditor::EnvelopeEditor()
: mEnvelopeControl(ofVec2f(5, 25), ofVec2f(380, 200), this)
{
}

void EnvelopeEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   static bool dummyBool;
   static float dummyFloat;

   UIBLOCK(3, 3, 130);
   FLOATSLIDER(mADSRViewLengthSlider, "view length", &dummyFloat, 10, 10000);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mMaxSustainSlider, "max sustain", &dummyFloat, -1, 5000);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mPinButton, "pin");
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mFreeReleaseLevelCheckbox, "free release", &dummyBool);
   ENDUIBLOCK0();

   UIBLOCK(3, mHeight - 70);
   for (size_t i = 0; i < mStageControls.size(); ++i)
   {
      FLOATSLIDER(mStageControls[i].mTargetSlider, ("target" + ofToString(i)).c_str(), &dummyFloat, 0, 1);
      FLOATSLIDER(mStageControls[i].mTimeSlider, ("time" + ofToString(i)).c_str(), &dummyFloat, 1, 1000);
      FLOATSLIDER(mStageControls[i].mCurveSlider, ("curve" + ofToString(i)).c_str(), &dummyFloat, -2, 2);
      CHECKBOX(mStageControls[i].mSustainCheckbox, ("sustain" + ofToString(i)).c_str(), &mStageControls[i].mIsSustainStage);
      UIBLOCK_NEWCOLUMN();

      mStageControls[i].mTimeSlider->SetMode(FloatSlider::kSquare);
   }
   ENDUIBLOCK0();

   mADSRViewLengthSlider->SetMode(FloatSlider::kSquare);
   mMaxSustainSlider->SetMode(FloatSlider::kSquare);

   Resize(mWidth, mHeight);
}

EnvelopeEditor::~EnvelopeEditor()
{
}

void EnvelopeEditor::SetADSRDisplay(ADSRDisplay* adsrDisplay)
{
   mEnvelopeControl.SetADSR(adsrDisplay->GetADSR());

   mADSRDisplay = adsrDisplay;
   mADSRViewLengthSlider->SetVar(&adsrDisplay->GetMaxTime());
   mMaxSustainSlider->SetVar(&adsrDisplay->GetADSR()->GetMaxSustain());
   mFreeReleaseLevelCheckbox->SetVar(&adsrDisplay->GetADSR()->GetFreeReleaseLevel());

   for (int i = 0; i < (int)mStageControls.size(); ++i)
   {
      mStageControls[i].mTargetSlider->SetVar(&adsrDisplay->GetADSR()->GetStageData(i).target);
      mStageControls[i].mTimeSlider->SetVar(&adsrDisplay->GetADSR()->GetStageData(i).time);
      mStageControls[i].mCurveSlider->SetVar(&adsrDisplay->GetADSR()->GetStageData(i).curve);
      mStageControls[i].mIsSustainStage = adsrDisplay->GetADSR()->GetHasSustainStage() && (adsrDisplay->GetADSR()->GetSustainStage() == i);
   }
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
      float w, h;
      GetDimensions(w, h);

      ofPushStyle();
      ofSetColor(0, 0, 0);
      ofFill();
      ofSetLineWidth(.5f);
      ofRect(0, 0, w, h);
      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(0, 0, w, h);
      ofPopStyle();
   }

   mMaxSustainSlider->SetShowing(mADSRDisplay != nullptr && mADSRDisplay->GetADSR()->GetHasSustainStage());

   mADSRViewLengthSlider->Draw();
   mMaxSustainSlider->Draw();
   mFreeReleaseLevelCheckbox->Draw();
   if (!mPinned)
      mPinButton->Draw();

   if (mADSRDisplay != nullptr)
      mEnvelopeControl.SetViewLength(mADSRDisplay->GetMaxTime());
   mEnvelopeControl.Draw();

   if (mADSRDisplay != nullptr)
   {
      int numStages = mADSRDisplay->GetADSR()->GetNumStages();
      for (int i = 0; i < (int)mStageControls.size(); ++i)
      {
         mStageControls[i].mIsSustainStage = mADSRDisplay->GetADSR()->GetHasSustainStage() && (mADSRDisplay->GetADSR()->GetSustainStage() == i);

         mStageControls[i].mTargetSlider->SetShowing(i < numStages);
         mStageControls[i].mTimeSlider->SetShowing(i < numStages);
         mStageControls[i].mCurveSlider->SetShowing(i < numStages);
         mStageControls[i].mSustainCheckbox->SetShowing(i > 0 && i < numStages - 1);

         mStageControls[i].mTargetSlider->Draw();
         mStageControls[i].mTimeSlider->Draw();
         mStageControls[i].mCurveSlider->Draw();
         mStageControls[i].mSustainCheckbox->Draw();
      }

      //move release level checkbox below final stage control
      ofVec2f pos = mStageControls[numStages - 1].mSustainCheckbox->GetPosition(K(local));
      mFreeReleaseLevelCheckbox->SetPosition(pos.x, pos.y);
   }
}

void EnvelopeEditor::Resize(float w, float h)
{
   w = MAX(w, 250);
   h = MAX(h, 150);
   mEnvelopeControl.SetDimensions(ofVec2f(w - 10, h - 105));

   for (int i = 0; i < (int)mStageControls.size(); ++i)
   {
      mStageControls[i].mTargetSlider->Move(0, h - mHeight);
      mStageControls[i].mTimeSlider->Move(0, h - mHeight);
      mStageControls[i].mCurveSlider->Move(0, h - mHeight);
      mStageControls[i].mSustainCheckbox->Move(0, h - mHeight);
   }

   mWidth = w;
   mHeight = h;
}

void EnvelopeEditor::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mEnvelopeControl.OnClicked(x, y, right);
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

void EnvelopeEditor::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (int i = 0; i < (int)mStageControls.size(); ++i)
   {
      if (checkbox == mStageControls[i].mSustainCheckbox && mADSRDisplay != nullptr)
      {
         if (mStageControls[i].mIsSustainStage)
         {
            mADSRDisplay->GetADSR()->GetHasSustainStage() = true;
            mADSRDisplay->GetADSR()->SetSustainStage(i);
         }
         else
         {
            mADSRDisplay->GetADSR()->GetHasSustainStage() = false;
         }
      }
   }
}

void EnvelopeEditor::ButtonClicked(ClickButton* button, double time)
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

ofVec2f EnvelopeEditor::GetCurveSliderExtentsForStage(int stage) const
{
   if (stage < 0 || stage >= static_cast<int>(mStageControls.size()))
      return { -2, 2 };
   return {
      mStageControls[stage].mCurveSlider->GetMin(),
      mStageControls[stage].mCurveSlider->GetMax()
   };
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

void EnvelopeEditor::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mADSRViewLengthSlider)
   {
      for (int i = 0; i < (int)mStageControls.size(); ++i)
      {
         if (mADSRDisplay != nullptr)
            mStageControls[i].mTimeSlider->SetExtents(1, mADSRDisplay->GetMaxTime());
      }
   }
}

void EnvelopeEditor::SaveLayout(ofxJSONElement& moduleInfo)
{
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
      // Since `mPinned` is set after `IDrawableModule::LoadBasics` we need to manually set minimized
      // after setting `mPinned` which is used to check if this module has a titlebar.
      SetMinimized(moduleInfo["start_minimized"].asBool(), false);
   }
}

void EnvelopeEditor::SetUpFromSaveData()
{
   ADSRDisplay* display = dynamic_cast<ADSRDisplay*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
   if (display != nullptr)
      SetADSRDisplay(display);
}
