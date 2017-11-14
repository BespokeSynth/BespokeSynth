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

namespace
{
   float envelopeX = 10;
   float envelopeY = 40;
   float envelopeHeight = 200;
   float pointClickRadius = 4;
}

EnvelopeEditor::EnvelopeEditor()
:mADSRDisplay(nullptr)
, mPinned(false)
, mADSRViewLength(2000)
, mADSRViewLengthSlider(nullptr)
, mHighlightPoint(-1)
, mHighlightCurve(-1)
, mLastClickTime(0)
, mHasSustainStageCheckbox(nullptr)
, mSustainStageSlider(nullptr)
, mMaxSustainSlider(nullptr)
, mFreeReleaseLevelCheckbox(nullptr)
, mTargetCable(nullptr)
{
}

void EnvelopeEditor::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mADSRViewLengthSlider = new FloatSlider(this,"length",2,20,120,15,&mADSRViewLength,100,10000);
   mPinButton = new ClickButton(this,"pin",3,2);
   
   static bool dummyBool;
   static int dummyInt;
   static float dummyFloat;
   
   mHasSustainStageCheckbox = new Checkbox(this, "has sustain", 2, envelopeY + envelopeHeight + 2, &dummyBool);
   mSustainStageSlider = new IntSlider(this, "sustain stage", mHasSustainStageCheckbox, kAnchor_Right, 100, 15, &dummyInt, 1, MAX_ADSR_STAGES-1);
   mMaxSustainSlider = new FloatSlider(this, "max sustain", mSustainStageSlider, kAnchor_Right, 100, 15, &dummyFloat, -1, 5000);
   mFreeReleaseLevelCheckbox = new Checkbox(this, "free release", mHasSustainStageCheckbox, kAnchor_Below, &dummyBool);
}

EnvelopeEditor::~EnvelopeEditor()
{
}

void EnvelopeEditor::SetADSRDisplay(ADSRDisplay* adsrDisplay)
{
   mADSRDisplay = adsrDisplay;
   mHasSustainStageCheckbox->SetVar(&adsrDisplay->GetADSR()->GetHasSustainStage());
   mSustainStageSlider->SetVar(&adsrDisplay->GetADSR()->GetSustainStage());
   mMaxSustainSlider->SetVar(&adsrDisplay->GetADSR()->GetMaxSustain());
   mFreeReleaseLevelCheckbox->SetVar(&adsrDisplay->GetADSR()->GetFreeReleaseLevel());
   
   mSustainStageSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
   mMaxSustainSlider->SetShowing(mADSRDisplay->GetADSR()->GetHasSustainStage());
}

void EnvelopeEditor::DrawModule()
{
   if (!mPinned)
   {
      int w,h;
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
   
   mADSRViewLengthSlider->Draw();
   mHasSustainStageCheckbox->Draw();
   mSustainStageSlider->Draw();
   mMaxSustainSlider->Draw();
   mFreeReleaseLevelCheckbox->Draw();
   if (!mPinned)
      mPinButton->Draw();
   
   ofPushStyle();
   
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   
   ofSetColor(100,100,.8f*gModuleDrawAlpha);
   
   ofSetLineWidth(.5f);
   ofRect(envelopeX, envelopeY, envelopeWidth, envelopeHeight, 0);
   
   ofSetColor(245, 58, 0, gModuleDrawAlpha);
   
   mViewAdsr.Set(*mADSRDisplay->GetADSR());
   mViewAdsr.Clear();
   mViewAdsr.Start(0,1);
   mViewAdsr.Stop(GetReleaseTime());
   
   ofSetLineWidth(1);
   ofBeginShape();
   AddVertex(GetXForTime(0),GetYForValue(0));
   for (float i=0; i<envelopeWidth; i+=(.25f/gDrawScale))
   {
      float time = i/envelopeWidth * mADSRViewLength;
      
      if (time < GetPreSustainTime())
      {
         float value = mViewAdsr.Value(time);
         AddVertex(i+envelopeX, GetYForValue(value));
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
   ofLine(ofClamp(GetXForTime(GetPreSustainTime()), envelopeX, envelopeX + envelopeWidth),
          GetYForValue(mViewAdsr.Value(GetPreSustainTime())),
          ofClamp(GetXForTime(GetReleaseTime()), envelopeX, envelopeX + envelopeWidth),
          GetYForValue(mViewAdsr.Value(GetReleaseTime())));
   ofPopStyle();
   
   ofBeginShape();
   AddVertex(GetXForTime(GetReleaseTime()),
            GetYForValue(mViewAdsr.Value(GetReleaseTime())));
   for (float i=0; i<envelopeWidth; i+=(.25f/gDrawScale))
   {
      float time = i/envelopeWidth * mADSRViewLength;
      
      if (time >= GetReleaseTime())
      {
         float value = mViewAdsr.Value(time);
         AddVertex(i+envelopeX, GetYForValue(value));
      }
   }
   ofEndShape(false);
   
   ofSetLineWidth(3);
   ofBeginShape();
   bool started = false;
   for (float i=0; i<envelopeWidth; i+=(.25f/gDrawScale))
   {
      float time = i/envelopeWidth * mADSRViewLength;
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
         AddVertex(i+envelopeX, GetYForValue(value));
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
      if (x < envelopeX + envelopeWidth)
         ofCircle(x, GetYForValue(value), i == mHighlightPoint ? 8 : pointClickRadius);
   }
   
   ofPopStyle();
}

void EnvelopeEditor::AddVertex(float x, float y)
{
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   if (x >= envelopeX && x <= envelopeX + envelopeWidth)
      ofVertex(x, y);
}

void EnvelopeEditor::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
   
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   
   if (x > envelopeX-pointClickRadius &&
       x < envelopeX + envelopeWidth + pointClickRadius &&
       y > envelopeY - pointClickRadius &&
       y < envelopeY+envelopeHeight+pointClickRadius)
   {
      mClick = true;
      
      if (right)
      {
         if (mHighlightCurve != -1)
         {
            ADSR* adsr = mADSRDisplay->GetADSR();
            adsr->GetStageData(mHighlightCurve).curve = 0;
            mViewAdsr.Set(*adsr);
         }
         if (mHighlightPoint != -1)
         {
            ADSR* adsr = mADSRDisplay->GetADSR();
            if (mHighlightPoint < adsr->GetNumStages()-1)
            {
               adsr->GetStageData(mHighlightPoint+1).time += adsr->GetStageData(mHighlightPoint).time;
               adsr->GetStageData(mHighlightPoint+1).curve = 0;
               
               for (int i=mHighlightPoint; i<adsr->GetNumStages(); ++i)
               {
                  adsr->GetStageData(i).time = adsr->GetStageData(i+1).time;
                  adsr->GetStageData(i).target = adsr->GetStageData(i+1).target;
                  adsr->GetStageData(i).curve = adsr->GetStageData(i+1).curve;
               }
               
               adsr->SetNumStages(adsr->GetNumStages() - 1);
               if (adsr->GetHasSustainStage() &&
                   mHighlightPoint <= adsr->GetSustainStage())
                  adsr->SetSustainStage(adsr->GetSustainStage()-1);
               mViewAdsr.Set(*adsr);
               
               mHighlightPoint = -1;
            }
         }
      }
      else if (gTime < mLastClickTime + 1000 &&
          mHighlightCurve != -1 &&
          (mClickStart - ofVec2f(x,y)).lengthSquared() < pointClickRadius*pointClickRadius)
      {
         float clickTime = GetTimeForX(x);
         if (clickTime > GetPreSustainTime())
            clickTime -= GetReleaseTime() - GetPreSustainTime();
         
         ADSR* adsr = mADSRDisplay->GetADSR();
         for (int i=adsr->GetNumStages(); i>mHighlightCurve; --i)
         {
            adsr->GetStageData(i).time = adsr->GetStageData(i-1).time;
            adsr->GetStageData(i).target = adsr->GetStageData(i-1).target;
            adsr->GetStageData(i).curve = adsr->GetStageData(i-1).curve;
         }
         float priorStageTimes = 0;
         for (int i=0; i<mHighlightCurve; ++i)
            priorStageTimes += adsr->GetStageData(i).time;
         adsr->GetStageData(mHighlightCurve).time = clickTime - priorStageTimes;
         adsr->GetStageData(mHighlightCurve).target = GetValueForY(y);
         adsr->GetStageData(mHighlightCurve+1).time -= adsr->GetStageData(mHighlightCurve).time;
         adsr->SetNumStages(adsr->GetNumStages() + 1);
         if (adsr->GetHasSustainStage() &&
             mHighlightCurve <= adsr->GetSustainStage())
            adsr->SetSustainStage(adsr->GetSustainStage()+1);
         mViewAdsr.Set(*adsr);
         
         mHighlightPoint = mHighlightCurve;
         mHighlightCurve = -1;
      }
      
      mLastClickTime = gTime;
      mClickStart.set(x,y);
      mClickAdsr.Set(mViewAdsr);
   }
}

void EnvelopeEditor::MouseReleased()
{
   IDrawableModule::MouseReleased();
   
   mClick = false;
}

bool EnvelopeEditor::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   
   if (!mClick)
   {
      mViewAdsr.Set(*mADSRDisplay->GetADSR());
      
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
            if (mHighlightCurve == mViewAdsr.GetNumStages())
               mHighlightCurve = -1;
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
         ADSR::Stage& stage = mADSRDisplay->GetADSR()->GetStageData(mHighlightPoint);
         ADSR::Stage& originalStage = mClickAdsr.GetStageData(mHighlightPoint);
         
         stage.time = ofClamp(originalStage.time + (x - mClickStart.x)/envelopeWidth * mADSRViewLength, 1, 10000);
         if (mHighlightPoint < mADSRDisplay->GetADSR()->GetNumStages()-1 || mADSRDisplay->GetADSR()->GetFreeReleaseLevel())
            stage.target = ofClamp(originalStage.target + ((mClickStart.y - y) / envelopeHeight), 0, 1);
         else
            stage.target = 0;
      }
      
      if (mHighlightCurve != -1)
      {
         ADSR::Stage& stage = mADSRDisplay->GetADSR()->GetStageData(mHighlightCurve);
         ADSR::Stage& originalStage = mClickAdsr.GetStageData(mHighlightCurve);
         
         stage.curve = ofClamp(originalStage.curve + ((mClickStart.y - y) / envelopeHeight), -1, 1);
      }
   }
   return false;
}

float EnvelopeEditor::GetPreSustainTime()
{
   float time = 0;
   for (int i=0; i<=mViewAdsr.GetSustainStage() || (!mViewAdsr.GetHasSustainStage() && i < mViewAdsr.GetNumStages()); ++i)
      time += mViewAdsr.GetStageData(i).time;

   return time;
}

float EnvelopeEditor::GetReleaseTime()
{
   if (!mViewAdsr.GetHasSustainStage())
      return GetPreSustainTime();
   if (mViewAdsr.GetMaxSustain() > 0)
      return GetPreSustainTime() + mViewAdsr.GetMaxSustain();
   else
      return GetPreSustainTime() + mADSRViewLength * .2f;
}

float EnvelopeEditor::GetTimeForX(float x)
{
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   return ofMap(x,envelopeX,envelopeX+envelopeWidth,0,mADSRViewLength);
}

float EnvelopeEditor::GetValueForY(float y)
{
   return ofMap(y,envelopeY,envelopeY+envelopeHeight,1,0);
}

float EnvelopeEditor::GetXForTime(float time)
{
   float envelopeWidth = IClickable::GetDimensions().x - envelopeX*2;
   return time/mADSRViewLength*envelopeWidth + envelopeX;
}

float EnvelopeEditor::GetYForValue(float value)
{
   return envelopeHeight * (1-value) + envelopeY;
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
         mPinned = true;
         TheSynth->AddDynamicModule(this);
         TheSynth->PopModalFocusItem();
         
         SetName(GetUniqueName("envelopeeditor", TheSynth->GetModuleNames<EnvelopeEditor*>()).c_str());
         
         if (mTargetCable == nullptr)
         {
            mTargetCable = new PatchCableSource(this, kConnectionType_UIControl);
            AddPatchCableSource(mTargetCable);
            mTargetCable->SetTarget(mADSRDisplay);
            mTargetCable->SetClickable(false);
         }
      }
   }
}

void EnvelopeEditor::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   if (mADSRDisplay != nullptr)
      moduleInfo["target"] = mADSRDisplay->Path();
}

void EnvelopeEditor::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void EnvelopeEditor::SetUpFromSaveData()
{
   mADSRDisplay = dynamic_cast<ADSRDisplay*>(TheSynth->FindUIControl(mModuleSaveData.GetString("target")));
}
