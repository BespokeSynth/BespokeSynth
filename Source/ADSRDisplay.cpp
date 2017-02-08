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

ADSRDisplay::DisplayMode ADSRDisplay::sDisplayMode = ADSRDisplay::kDisplayEnvelope;

ADSRDisplay::ADSRDisplay(IDrawableModule* owner, const char* name, int x, int y, int w, int h, ADSR* adsr)
: mClick(false)
, mWidth(w)
, mHeight(h)
, mAdsr(adsr)
, mVol(1)
, mMaxTime(1000)
, mAdjustMode(kAdjustNone)
, mHighlighted(false)
, mActive(true)
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
      mASlider = new FloatSlider(floatListener,(string(name)+"A").c_str(),x,y,w,sliderHeight,&(mAdsr->mA),1,1000);
      mDSlider = new FloatSlider(floatListener,(string(name)+"D").c_str(),x,y+sliderHeight,w,sliderHeight,&(mAdsr->mD),0,1000);
      mSSlider = new FloatSlider(floatListener,(string(name)+"S").c_str(),x,y+sliderHeight*2,w,sliderHeight,&(mAdsr->mS),0,1);
      mRSlider = new FloatSlider(floatListener,(string(name)+"R").c_str(),x,y+sliderHeight*3,w,sliderHeight,&(mAdsr->mR),1,1000);
      
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
   UpdateSliderVisibility();

   ofPushStyle();
   ofPushMatrix();

   ofTranslate(mX,mY);

   ofSetColor(100,100,.8f*gModuleDrawAlpha);

   ofSetLineWidth(.5f);
   ofRect(0, 0, mWidth, mHeight, 0);

   if (mAdsr && (mActive || sDisplayMode == kDisplayEnvelope))
   {
      if (mActive)
         ofSetColor(245, 58, 0, gModuleDrawAlpha);
      else
         ofSetColor(0, 230, 245, .3f*gModuleDrawAlpha);
      ofSetLineWidth(1);

      ofBeginShape();

      mViewAdsr.Set(mAdsr->mA,mAdsr->mD,mAdsr->mS,mAdsr->mR,mAdsr->mMaxSustain);
      mViewAdsr.Clear();
      mViewAdsr.Start(0,1);
      float releaseTime = mAdsr->mA + mAdsr->mD + mMaxTime * .2f;
      ofVertex(0,mHeight);
      for (float i=0; i<mWidth; i+=(.25f/gDrawScale))
      {
         float time = i/mWidth * mMaxTime;
         if (time > releaseTime)
            mViewAdsr.Stop(releaseTime);
         float value = mViewAdsr.Value(time)*mVol;
         ofVertex(i, mHeight * (1 - value));
      }
      ofEndShape(false);
   }
   
   ofFill();
   
   if (mHighlighted)
   {
      ofSetColor(255,255,0,.2f*gModuleDrawAlpha);
      ofRect(0,0,mWidth,mHeight, 0);
   }

   if (mActive && sDisplayMode == kDisplayEnvelope)
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

void ADSRDisplay::SetADSR(ADSR* adsr)
{
   mAdsr = adsr;
   if (mASlider)
   {
      mASlider->SetVar(&(mAdsr->mA));
      mDSlider->SetVar(&(mAdsr->mD));
      mSSlider->SetVar(&(mAdsr->mS));
      mRSlider->SetVar(&(mAdsr->mR));
   }
}

void ADSRDisplay::SetActive(bool active)
{
   mActive = active;
}

void ADSRDisplay::UpdateSliderVisibility()
{
   bool slidersActive = mActive && (sDisplayMode == kDisplaySliders);
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

void ADSRDisplay::OnClicked(int x, int y, bool right)
{
   if (!mShowing || !mActive || sDisplayMode != kDisplayEnvelope)
      return;
   
   if (right)
   {
      mAdsr->mA = ofMap(pow(ofRandom(1),2),0,1,1,100);
      mAdsr->mD = ofMap(pow(ofRandom(1),2),0,1,1,100);
      mAdsr->mS = ofRandom(0,1);
      mAdsr->mR = ofMap(pow(ofRandom(1),2),0,1,1,100);
      return;
   }
   
   mClick = true;
   mClickStart.set(x,y);
   mClickAdsr.Set(mViewAdsr.mA, mViewAdsr.mD, mViewAdsr.mS, mViewAdsr.mR);
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
      if (mAdsr == NULL)
         return false;
      float mousePosSq = (x-mClickStart.x)/mWidth;
      if (mousePosSq > 0)
         mousePosSq *= mousePosSq;
      switch (mAdjustMode)
      {
         case kAdjustAttack:
         {
            float a = ofClamp(mClickAdsr.mA + mousePosSq * 100,1,mMaxTime);
            mViewAdsr.mA = a;
            mAdsr->mA = a;
            break;
         }
         case kAdjustDecaySustain:
         {
            float d = ofClamp(mClickAdsr.mD + mousePosSq * 1000,1,mMaxTime);
            mViewAdsr.mD = d;
            mAdsr->mD = d;
            float s = ofClamp(mClickAdsr.mS + (mClickStart.y-y)/mHeight,0,1);
            mViewAdsr.mS = s;
            mAdsr->mS = s;
            break;
         }
         case kAdjustRelease:
         {
            float r = ofClamp(mClickAdsr.mR + mousePosSq * 1000,1,mMaxTime);
            mViewAdsr.mR = r;
            mAdsr->mR = r;
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
   
   out << mAdsr->mA;
   out << mAdsr->mD;
   out << mAdsr->mS;
   out << mAdsr->mR;
}

void ADSRDisplay::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   in >> mAdsr->mA;
   in >> mAdsr->mD;
   in >> mAdsr->mS;
   in >> mAdsr->mR;
   if (rev == 0)
   {
      float dummy;
      in >> dummy;
   }
}
