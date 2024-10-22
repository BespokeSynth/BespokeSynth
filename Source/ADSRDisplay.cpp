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
#include "PatchCable.h"
#include "IModulator.h"

ADSRDisplay::DisplayMode ADSRDisplay::sDisplayMode = ADSRDisplay::kDisplayEnvelope;

ADSRDisplay::ADSRDisplay(IDrawableModule* owner, const char* name, int x, int y, int w, int h, ::ADSR* adsr)
: mWidth(w)
, mHeight(h)
, mAdsr(adsr)
{
   assert(owner);
   SetName(name);
   SetPosition(x, y);
   owner->AddUIControl(this);
   SetParent(owner);

   IFloatSliderListener* floatListener = dynamic_cast<IFloatSliderListener*>(owner);
   assert(floatListener); //make anything that uses an ADSRDisplay a FloatSliderListener for these sliders
   if (floatListener)
   {
      int sliderHeight = h / 4;
      mASlider = new FloatSlider(floatListener, (std::string(name) + "A").c_str(), x, y, w, sliderHeight, &(mAdsr->GetA()), 0.001f, 1000);
      mDSlider = new FloatSlider(floatListener, (std::string(name) + "D").c_str(), x, y + sliderHeight, w, sliderHeight, &(mAdsr->GetD()), 0.001f, 1000);
      mSSlider = new FloatSlider(floatListener, (std::string(name) + "S").c_str(), x, y + sliderHeight * 2, w, sliderHeight, &(mAdsr->GetS()), 0, 1);
      mRSlider = new FloatSlider(floatListener, (std::string(name) + "R").c_str(), x, y + sliderHeight * 3, w, sliderHeight, &(mAdsr->GetR()), 0.001f, 1000);

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

   ofTranslate(mX, mY);

   ofSetColor(100, 100, 100, .8f * gModuleDrawAlpha);

   ofSetLineWidth(.5f);
   ofRect(0, 0, mWidth, mHeight, 0);

   if (mAdsr && sDisplayMode == kDisplayEnvelope)
   {
      ofSetColor(245, 58, 0, gModuleDrawAlpha);
      ofSetLineWidth(1);

      float timeBeforeSustain = mMaxTime;
      float releaseTime = mMaxTime;
      if (mAdsr->GetMaxSustain() == -1 && mAdsr->GetHasSustainStage())
      {
         timeBeforeSustain = 0;
         for (int i = 0; i < mAdsr->GetNumStages(); ++i)
         {
            timeBeforeSustain += mAdsr->GetStageData(i).time;
            if (i == mAdsr->GetSustainStage())
               break;
         }
         releaseTime = timeBeforeSustain + mMaxTime * .2f;
      }
      ADSR::EventInfo adsrEvent(0, releaseTime);

      ofBeginShape();
      ofVertex(0, mHeight);
      for (float i = 0; i < mWidth; i += (.25f / gDrawScale))
      {
         float time = i / mWidth * mMaxTime;
         ofVertex(GetDrawPoint(time, adsrEvent));
      }
      ofEndShape(false);

      if (mAdjustMode == kAdjustAttack ||
          mAdjustMode == kAdjustAttackAR ||
          mAdjustMode == kAdjustDecaySustain ||
          mAdjustMode == kAdjustRelease ||
          mAdjustMode == kAdjustReleaseAR)
      {
         float startTime = 0;
         float endTime = 999;

         if (mAdjustMode == kAdjustAttack || mAdjustMode == kAdjustAttackAR)
         {
            startTime = 0;
            endTime = mAdsr->GetA();
         }
         if (mAdjustMode == kAdjustDecaySustain)
         {
            startTime = mAdsr->GetA();
            endTime = releaseTime;
         }
         if (mAdjustMode == kAdjustRelease)
         {
            startTime = releaseTime;
            endTime = releaseTime + mAdsr->GetR();
         }
         if (mAdjustMode == kAdjustReleaseAR)
         {
            startTime = mAdsr->GetA();
            endTime = mAdsr->GetA() + mAdsr->GetD();
         }

         ofSetColor(0, 255, 255);
         ofBeginShape();
         ofVertex(GetDrawPoint(startTime, adsrEvent));
         for (float i = 0; i < mWidth; i += (.25f / gDrawScale))
         {
            float time = i / mWidth * mMaxTime;
            if (time >= startTime && time <= endTime)
               ofVertex(GetDrawPoint(time, adsrEvent));
         }
         ofVertex(GetDrawPoint(MIN(endTime, mMaxTime), adsrEvent));
         ofEndShape(false);
      }

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
            drawTime = releaseTime + (gTime - mAdsr->GetStopTime(gTime));
      }

      ofPushStyle();
      ofSetColor(0, 255, 0, gModuleDrawAlpha * .5f);
      float x = drawTime / mMaxTime * mWidth;
      float y = (1 - mAdsr->Value(drawTime, &adsrEvent) * mVol) * mHeight;
      if (drawTime >= timeBeforeSustain && drawTime <= releaseTime)
      {
         ofSetLineWidth(1.5f);
         ofLine(drawTime / mMaxTime * mWidth, y, releaseTime / mMaxTime * mWidth, y);
      }

      if (drawTime > 0 && drawTime < mMaxTime)
      {
         ofPushMatrix();
         ofClipWindow(0, 0, mWidth, mHeight, true);
         ofCircle(x, y, 1);
         ofLine(x, y, x, mHeight);
         ofPopMatrix();
      }

      mDrawTimeHistory[mDrawTimeHistoryIndex] = drawTime;

      for (size_t i = 0; i < mDrawTimeHistory.size() - 1; ++i)
      {
         ofFill();
         ofSetColor(0, 255, 0, gModuleDrawAlpha * ofLerp(.3f, 0, float(i) / mDrawTimeHistory.size()));
         int indexLeading = (mDrawTimeHistoryIndex - i + (int)mDrawTimeHistory.size()) % (int)mDrawTimeHistory.size();
         //int indexPast = (indexLeading - 1 + (int)mDrawTimeHistory.size()) % (int)mDrawTimeHistory.size();
         double timeLeading = mDrawTimeHistory[indexLeading];
         //double timePast = mDrawTimeHistory[indexPast];
         float xLeading = timeLeading / mMaxTime * mWidth;
         float yLeading = (1 - mAdsr->Value(timeLeading, &adsrEvent) * mVol) * mHeight;
         //float xPast = timePast / mMaxTime * mWidth;
         //float yPast = (1 - mAdsr->Value(timePast, &adsrEvent) * mVol) * mHeight;

         if (xLeading <= mWidth)
            ofLine(xLeading, yLeading, xLeading, mHeight);

         /*bool discontinuity = false;
         if (timeLeading < timePast)
            discontinuity = true;
         if (timeLeading >= releaseTime - gBufferSizeMs && timePast <= releaseTime)
            discontinuity = true;

         if (!discontinuity)
         {
            ofBeginShape();
            ofVertex(xLeading, yLeading);
            ofVertex(xLeading, mHeight);
            ofVertex(xPast, mHeight);
            ofVertex(xPast, yPast);
            ofEndShape();
         }*/
      }

      mDrawTimeHistoryIndex = (mDrawTimeHistoryIndex + 1) % mDrawTimeHistory.size();

      ofPopStyle();
   }

   ofFill();

   if (mHighlighted)
   {
      ofSetColor(255, 255, 0, .2f * gModuleDrawAlpha);
      ofRect(0, 0, mWidth, mHeight, 0);
   }

   ofSetColor(245, 58, 0, gModuleDrawAlpha);
   ofCircle(mWidth - 5, 5, 2);

   if (sDisplayMode == kDisplayEnvelope)
   {
      ofSetColor(0, 255, 255, .2f * gModuleDrawAlpha);
      switch (mAdjustMode)
      {
         case kAdjustAttack:
            ofRect(0, 0, 20, mHeight);
            break;
         case kAdjustDecaySustain:
            ofRect(20, 0, mWidth - 40, mHeight);
            break;
         case kAdjustRelease:
            ofRect(mWidth - 20, 0, 20, mHeight);
            break;
         case kAdjustEnvelopeEditor:
            ofSetColor(255, 255, 255, .2f * gModuleDrawAlpha);
            ofRect(mWidth - 10, 0, 10, 10);
            break;
         case kAdjustViewLength:
            ofSetColor(255, 255, 255, .2f * gModuleDrawAlpha);
            ofRect(0, 0, mWidth, 10);
            ofRect(ofMap(mMaxTime, 10, 10000, 0, mWidth - 3, K(clamp)), 0, 3, 10);
            ofSetColor(255, 255, 255, .8f * gModuleDrawAlpha);
            DrawTextNormal(ofToString(mMaxTime, 0) + " ms", 3, 8, 8);
            break;
         case kAdjustAttackAR:
            ofRect(0, 0, mWidth * .5f, mHeight);
            break;
         case kAdjustReleaseAR:
            ofRect(mWidth * .5f, 0, mWidth * .5f, mHeight);
            break;
         default:
            break;
      }
   }

   ofPopMatrix();
   ofPopStyle();

   if (mASlider)
   {
      int sliderHeight = mHeight / 4;
      mASlider->SetPosition(mX, mY);
      mDSlider->SetPosition(mX, mY + sliderHeight);
      mSSlider->SetPosition(mX, mY + sliderHeight * 2);
      mRSlider->SetPosition(mX, mY + sliderHeight * 3);

      mASlider->Draw();
      mDSlider->Draw();
      mSSlider->Draw();
      mRSlider->Draw();
   }
}

ofVec2f ADSRDisplay::GetDrawPoint(float time, const ADSR::EventInfo& adsrEvent)
{
   float value = mAdsr->Value(time, &adsrEvent) * mVol;
   float x = time / mMaxTime * mWidth;
   float y = mHeight * (1 - value);
   return ofVec2f(x, y);
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
   bool slidersActive = false;
   if (mAdsr != nullptr && IsShowing())
   {
      if (sDisplayMode == kDisplaySliders)
         slidersActive = true;
      if (PatchCable::sActivePatchCable != nullptr &&
          (PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_Modulator || PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_ValueSetter || PatchCable::sActivePatchCable->GetConnectionType() == kConnectionType_UIControl))
         slidersActive = true;
   }
   if (mASlider && mASlider->GetModulator() != nullptr && mASlider->GetModulator()->Active())
      slidersActive = true;
   if (mDSlider && mDSlider->GetModulator() != nullptr && mDSlider->GetModulator()->Active())
      slidersActive = true;
   if (mSSlider && mSSlider->GetModulator() != nullptr && mSSlider->GetModulator()->Active())
      slidersActive = true;
   if (mRSlider && mRSlider->GetModulator() != nullptr && mRSlider->GetModulator()->Active())
      slidersActive = true;
   if (mAdsr != nullptr && mASlider)
   {
      if (mAdsr->IsStandardADSR())
      {
         mASlider->SetShowing(slidersActive);
         mDSlider->SetShowing(slidersActive);
         mSSlider->SetShowing(slidersActive);
         mRSlider->SetShowing(slidersActive);
      }
      if (mAdsr->GetNumStages() == 3 && !mAdsr->GetHasSustainStage())
      {
         mASlider->SetShowing(slidersActive);
         mDSlider->SetShowing(slidersActive);
         mSSlider->SetShowing(false);
         mRSlider->SetShowing(slidersActive);
      }
      if (mAdsr->GetNumStages() == 2 && !mAdsr->GetHasSustainStage())
      {
         mASlider->SetShowing(slidersActive);
         mDSlider->SetShowing(slidersActive);
         mSSlider->SetShowing(false);
         mRSlider->SetShowing(false);
      }
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
      ModuleFactory::Spawnable spawnable;
      spawnable.mLabel = "envelopeeditor";
      mEditor = dynamic_cast<EnvelopeEditor*>(TheSynth->SpawnModuleOnTheFly(spawnable, -1, -1, false, "envelopepopup"));
      mEditor->SetADSRDisplay(this);
   }
   if (!mEditor->IsPinned())
   {
      mEditor->SetPosition(GetPosition().x + mWidth, GetPosition().y);
      mEditor->SetOwningContainer(GetModuleParent()->GetOwningContainer());
      TheSynth->PushModalFocusItem(mEditor);
   }
}

void ADSRDisplay::OnClicked(float x, float y, bool right)
{
   if (mASlider != nullptr && mASlider->IsShowing())
   {
      if (gHoveredUIControl == mASlider)
         mASlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mDSlider)
         mDSlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mSSlider)
         mSSlider->TestClick(x + mX, y + mY, right, false);
      if (gHoveredUIControl == mRSlider)
         mRSlider->TestClick(x + mX, y + mY, right, false);
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
      mClickStart.set(x, y);
      mClickAdsr.Set(*mAdsr);
      mClickLength = mMaxTime;
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
      if (x < 0 || y < 0 || x > mWidth || y > mHeight)
      {
         mAdjustMode = kAdjustNone;
      }
      else if (GetKeyModifiers() == kModifier_Shift)
      {
         mAdjustMode = kAdjustViewLength;
      }
      else if (x >= mWidth - 10 && x <= mWidth && y >= 0 && y <= 10)
      {
         mAdjustMode = kAdjustEnvelopeEditor;
      }
      else if (mAdsr->GetNumStages() == 2) //2-stage AR envelope
      {
         if (x < mWidth / 2)
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
      else if (x < 20)
      {
         mAdjustMode = kAdjustAttack;
      }
      else if (x > mWidth - 20)
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
      float mousePosSq = (x - mClickStart.x) / mWidth;
      if (mousePosSq > 0)
         mousePosSq *= mousePosSq;
      switch (mAdjustMode)
      {
         case kAdjustViewLength:
         {
            mMaxTime = std::clamp(mClickLength + mousePosSq * 500, 10.0f, 10000.0f);
            break;
         }
         case kAdjustAttack:
         case kAdjustAttackAR:
         {
            float a = ofClamp(mClickAdsr.GetA() + mousePosSq * mMaxTime * .1f, 1, mMaxTime);
            mAdsr->GetA() = a;
            break;
         }
         case kAdjustDecaySustain:
         {
            float d = ofClamp(mClickAdsr.GetD() + mousePosSq * mMaxTime, 1, mMaxTime);
            mAdsr->GetD() = d;
            float s = ofClamp(mClickAdsr.GetS() + (mClickStart.y - y) / mHeight, 0, 1);
            mAdsr->GetS() = s;
            break;
         }
         case kAdjustRelease:
         {
            float r = ofClamp(mClickAdsr.GetR() + mousePosSq * mMaxTime, 1, mMaxTime);
            mAdsr->GetR() = r;
            break;
         }
         case kAdjustReleaseAR:
         {
            float r = ofClamp(mClickAdsr.GetD() + mousePosSq * mMaxTime, 1, mMaxTime);
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
