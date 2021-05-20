//
//  ADSRDisplay.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/28/13.
//
//

#include "ADSRDisplay.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "ModularSynth.h"
#include "EnvelopeEditor.h"

ADSRDisplay::DisplayMode ADSRDisplay::sDisplayMode = ADSRDisplay::kDisplayEnvelope;

ADSRDisplay::ADSRDisplay(IDrawableModule* owner, const char* name, int x, int y, int w, int h, ::ADSR* adsr)
: mClick(false)
, mWidth(w)
, mHeight(h)
, mAdsr(adsr)
, mVol(1)
, mMaxTime(1000)
, mAdjustMode(kAdjustNone)
, mHighlighted(false)
, mEditor(nullptr)
, mOverrideDrawTime(-1)
{
   assert(owner);
   SetName(name);
   SetPosition(x,y);
   owner->AddUIControl(this);
   SetParent(owner);
   
   IFloatSliderListener* floatListener = dynamic_cast<IFloatSliderListener*>(owner);
   assert(floatListener);  //make anything that uses an ADSRDisplay a FloatSliderListener for these sliders
   if (floatListener)
   {
      int sliderHeight = h/4;
      mASlider = new FloatSlider(floatListener,(string(name)+"A").c_str(),x,y,w,sliderHeight,&(mAdsr->GetA()),1,1000);
      mDSlider = new FloatSlider(floatListener,(string(name)+"D").c_str(),x,y+sliderHeight,w,sliderHeight,&(mAdsr->GetD()),0,1000);
      mSSlider = new FloatSlider(floatListener,(string(name)+"S").c_str(),x,y+sliderHeight*2,w,sliderHeight,&(mAdsr->GetS()),0,1);
      mRSlider = new FloatSlider(floatListener,(string(name)+"R").c_str(),x,y+sliderHeight*3,w,sliderHeight,&(mAdsr->GetR()),1,1000);
      
      mASlider->SetMode(FloatSlider::kSquare);
      mDSlider->SetMode(FloatSlider::kSquare);
      mRSlider->SetMode(FloatSlider::kSquare);
      
      mASlider->SetShowName(false);
      mDSlider->SetShowName(false);
      mSSlider->SetShowName(false);
      mRSlider->SetShowName(false);
      
      UpdateSliderVisibility();
   }
}

ADSRDisplay::~ADSRDisplay()
{
}

void ADSRDisplay::Render()
{
   static bool sSkipDraw = false;
   if (sSkipDraw)
      return;
   
   UpdateSliderVisibility();

   ofPushStyle();
   ofPushMatrix();

   ofTranslate(mX,mY);

   ofSetColor(100,100,.8f*gModuleDrawAlpha);

   ofSetLineWidth(.5f);
   ofRect(0, 0, mWidth, mHeight, 0);

   if (mAdsr && sDisplayMode == kDisplayEnvelope)
   {
      ofSetColor(245, 58, 0, gModuleDrawAlpha);
      ofSetLineWidth(1);

      ofBeginShape();

      mViewAdsr.Set(*mAdsr);
      mViewAdsr.Clear();
      mViewAdsr.Start(0,1);
      float releaseTime = mMaxTime;
      if (mViewAdsr.GetMaxSustain() == -1 && mViewAdsr.GetHasSustainStage())
      {
         releaseTime = mMaxTime * .2f;
         for (int i=0; i<mViewAdsr.GetNumStages(); ++i)
         {
            releaseTime += mViewAdsr.GetStageData(i).time;
            if (i == mViewAdsr.GetSustainStage())
               break;
         }
         mViewAdsr.Stop(releaseTime);
      }
      ofVertex(0,mHeight);
      for (float i=0; i<mWidth; i+=(.25f/gDrawScale))
      {
         float time = i/mWidth * mMaxTime;
         float value = mViewAdsr.Value(time)*mVol;
         ofVertex(i, mHeight * (1 - value));
      }
      ofEndShape(false);
      
      ofSetLineWidth(1);
      ofSetColor(0,255,0,gModuleDrawAlpha * .5f);
      float drawTime = 0;
      if (mOverrideDrawTime != -1)
      {
         drawTime = mOverrideDrawTime;
      }
      else
      {
         if (mAdsr->GetStartTime(gTime) > 0 && mAdsr->GetStartTime(gTime) >= mAdsr->GetStopTime(gTime))
            drawTime = ofClamp(gTime - mAdsr->GetStartTime(gTime), 0, releaseTime * mAdsr->GetTimeScale()) / mAdsr->GetTimeScale();
         if (mAdsr->GetStopTime(gTime) > mAdsr->GetStartTime(gTime))
            drawTime = releaseTime * mAdsr->GetTimeScale() + (gTime - mAdsr->GetStopTime(gTime)) / mAdsr->GetTimeScale();
      }
      if (drawTime > 0 && drawTime < mMaxTime)
         ofLine(drawTime/mMaxTime*mWidth, 0, drawTime/mMaxTime*mWidth, mHeight);
   }
   
   ofFill();
   
   if (mHighlighted)
   {
      ofSetColor(255,255,0,.2f*gModuleDrawAlpha);
      ofRect(0,0,mWidth,mHeight, 0);
   }

   if (sDisplayMode == kDisplayEnvelope)
   {
      ofSetColor(0,255,255,.2f*gModuleDrawAlpha);
      switch (mAdjustMode)
      {
         case kAdjustAttack:
            ofRect(0,0,20,mHeight);
            break;
         case kAdjustDecaySustain:
            ofRect(20,0,mWidth-40,mHeight);
            break;
         case kAdjustRelease:
            ofRect(mWidth-20,0,20,mHeight);
            break;
         case kAdjustEnvelopeEditor:
            ofSetColor(255,255,255,.2f*gModuleDrawAlpha);
            ofRect(mWidth-10,0,10,10);
            break;
         case kAdjustAttackAR:
            ofRect(0, 0, mWidth*.5f, mHeight);
            break;
         case kAdjustReleaseAR:
            ofRect(mWidth*.5f, 0, mWidth*.5f, mHeight);
            break;
         default:
            break;
      }
   }

   ofPopMatrix();
   ofPopStyle();
   
   if (mASlider)
   {
      int sliderHeight = mHeight/4;
      mASlider->SetPosition(mX,mY);
      mDSlider->SetPosition(mX,mY+sliderHeight);
      mSSlider->SetPosition(mX,mY+sliderHeight*2);
      mRSlider->SetPosition(mX,mY+sliderHeight*3);
      
      mASlider->Draw();
      mDSlider->Draw();
      mSSlider->Draw();
      mRSlider->Draw();
   }
}

void ADSRDisplay::SetMaxTime(float maxTime)
{
   mMaxTime = maxTime;
   if (mASlider)
   {
      mASlider->SetExtents(mASlider->GetMin(), maxTime);
      mDSlider->SetExtents(mDSlider->GetMin(), maxTime);
      mRSlider->SetExtents(mRSlider->GetMin(), maxTime);
   }
}

void ADSRDisplay::SetADSR(::ADSR* adsr)
{
   mAdsr = adsr;
   if (mASlider)
   {
      mASlider->SetVar(&(mAdsr->GetA()));
      mDSlider->SetVar(&(mAdsr->GetD()));
      mSSlider->SetVar(&(mAdsr->GetS()));
      mRSlider->SetVar(&(mAdsr->GetR()));
   }
}

void ADSRDisplay::UpdateSliderVisibility()
{
   bool slidersActive = (sDisplayMode == kDisplaySliders) && mAdsr != nullptr && mAdsr->IsStandardADSR() && IsShowing();
   if (mASlider)
   {
      mASlider->SetShowing(slidersActive);
      mDSlider->SetShowing(slidersActive);
      mSSlider->SetShowing(slidersActive);
      mRSlider->SetShowing(slidersActive);
   }
}

//static
void ADSRDisplay::ToggleDisplayMode()
{
   sDisplayMode = (sDisplayMode == kDisplayEnvelope) ? kDisplaySliders : kDisplayEnvelope;
}

void ADSRDisplay::SpawnEnvelopeEditor()
{
   if (mEditor == nullptr)
   {
      mEditor = dynamic_cast<EnvelopeEditor*>(TheSynth->SpawnModuleOnTheFly("envelopeeditor", -1, -1, false));
      mEditor->SetADSRDisplay(this);
   }
   if (!mEditor->IsPinned())
   {
      mEditor->SetPosition(GetPosition().x+mWidth, GetPosition().y);
      TheSynth->PushModalFocusItem(mEditor);
   }
}

void ADSRDisplay::OnClicked(int x, int y, bool right)
{
   if (sDisplayMode == kDisplaySliders)
   {
      if (gHoveredUIControl == mASlider) mASlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mDSlider) mDSlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mSSlider) mSSlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mRSlider) mRSlider->TestClick(x + mX, y + mY, right, false);
      return;
   }

   if (!mShowing)
      return;
   
   if (right)
   {
      //randomize
      for (int i = 0; i < mAdsr->GetNumStages(); ++i)
      {
         if (i == 0)
            mAdsr->GetStageData(i).target = 1;
         else if (i == mAdsr->GetNumStages() - 1)
            mAdsr->GetStageData(i).target = 0;
         else
            mAdsr->GetStageData(i).target = ofRandom(0, 1);

         mAdsr->GetStageData(i).time = ofMap(pow(ofRandom(1), 2), 0, 1, 1, 200);
      }
      return;
   }
   
   if (mAdjustMode == kAdjustEnvelopeEditor)
   {
      TheSynth->ScheduleEnvelopeEditorSpawn(this);
   }
   else if (mAdsr->IsStandardADSR() || mAdsr->GetNumStages() == 2)
   {
      mClick = true;
      mClickStart.set(x,y);
      mClickAdsr.Set(mViewAdsr);
   }
}

void ADSRDisplay::MouseReleased()
{
   mClick = false;
}

bool ADSRDisplay::MouseMoved(float x, float y)
{
   if (!mClick)
   {
      if (x<0 || y<0 || x>mWidth || y>mHeight)
      {
         mAdjustMode = kAdjustNone;
      }
      else if (x >= mWidth-10 && x <= mWidth && y >= 0 && y <=10)
      {
         mAdjustMode = kAdjustEnvelopeEditor;
      }
      else if (mAdsr->GetNumStages() == 2)   //2-stage AR envelope
      {
         if (x < mWidth/2)
         {
            mAdjustMode = kAdjustAttackAR;
         }
         else
         {
            mAdjustMode = kAdjustReleaseAR;
         }
      }
      else if (!mAdsr->IsStandardADSR())
      {
         mAdjustMode = kAdjustNone;
      }      
      else if (x<20)
      {
         mAdjustMode = kAdjustAttack;
      }
      else if (x > mWidth-20)
      {
         mAdjustMode = kAdjustRelease;
      }
      else
      {
         mAdjustMode = kAdjustDecaySustain;
      }
   }
   if (mClick)
   {
      if (mAdsr == nullptr)
         return false;
      float mousePosSq = (x-mClickStart.x)/mWidth;
      if (mousePosSq > 0)
         mousePosSq *= mousePosSq;
      switch (mAdjustMode)
      {
         case kAdjustAttack:
         case kAdjustAttackAR:
         {
            float a = ofClamp(mClickAdsr.GetA() + mousePosSq * mMaxTime * .1f,1,mMaxTime);
            mViewAdsr.GetA() = a;
            mAdsr->GetA() = a;
            break;
         }
         case kAdjustDecaySustain:
         {
            float d = ofClamp(mClickAdsr.GetD() + mousePosSq * mMaxTime,1,mMaxTime);
            mViewAdsr.GetD() = d;
            mAdsr->GetD() = d;
            float s = ofClamp(mClickAdsr.GetS() + (mClickStart.y-y)/mHeight,0,1);
            mViewAdsr.GetS() = s;
            mAdsr->GetS() = s;
            break;
         }
         case kAdjustRelease:
         {
            float r = ofClamp(mClickAdsr.GetR() + mousePosSq * mMaxTime,1,mMaxTime);
            mViewAdsr.GetR() = r;
            mAdsr->GetR() = r;
            break;
         }
         case kAdjustReleaseAR:
         {
            float r = ofClamp(mClickAdsr.GetD() + mousePosSq * mMaxTime, 1, mMaxTime);
            mViewAdsr.GetD() = r;
            mAdsr->GetD() = r;
            break;
         }
         default:
            break;
      }
   }
   return false;
}

namespace
{
   const int kSaveStateRev = 1;
}

void ADSRDisplay::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
   
   mAdsr->SaveState(out);
}

void ADSRDisplay::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   mAdsr->LoadState(in);
}
