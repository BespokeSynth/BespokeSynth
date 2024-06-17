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
//  CurveLooper.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#include "CurveLooper.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "SynthGlobals.h"
#include <algorithm>

namespace
{
   const int kAdsrTime = 10000;
}

CurveLooper::CurveLooper()
{
   mEnvelopeControl.SetADSR(&mAdsr);
   mEnvelopeControl.SetViewLength(kAdsrTime);
   mEnvelopeControl.SetFixedLengthMode(true);
   mAdsr.GetFreeReleaseLevel() = true;
   mAdsr.SetNumStages(2);
   mAdsr.GetHasSustainStage() = false;
   mAdsr.GetStageData(0).target = .5f;
   mAdsr.GetStageData(0).time = kAdsrTime * .1f;
   mAdsr.GetStageData(1).target = .5f;
   mAdsr.GetStageData(1).time = kAdsrTime * .8f;
}

void CurveLooper::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

CurveLooper::~CurveLooper()
{
   TheTransport->RemoveAudioPoller(this);
}

void CurveLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLengthSelector = new DropdownList(this, "length", 5, 3, (int*)(&mLength));
   mRandomizeButton = new ClickButton(this, "randomize", -1, -1);

   mControlCable = new PatchCableSource(this, kConnectionType_ValueSetter);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);

   mLengthSelector->AddLabel("4n", -4);
   mLengthSelector->AddLabel("2n", -2);
   mLengthSelector->AddLabel("1", 1);
   mLengthSelector->AddLabel("2", 2);
   mLengthSelector->AddLabel("3", 3);
   mLengthSelector->AddLabel("4", 4);
   mLengthSelector->AddLabel("6", 6);
   mLengthSelector->AddLabel("8", 8);
   mLengthSelector->AddLabel("16", 16);
   mLengthSelector->AddLabel("32", 32);
   mLengthSelector->AddLabel("64", 64);
   mLengthSelector->AddLabel("128", 128);

   mRandomizeButton->PositionTo(mLengthSelector, kAnchor_Right);
}

void CurveLooper::Poll()
{
}

void CurveLooper::OnTransportAdvanced(float amount)
{
   if (mEnabled)
   {
      ADSR::EventInfo adsrEvent(0, kAdsrTime);

      for (auto* control : mUIControls)
      {
         if (control != nullptr)
            control->SetFromMidiCC(mAdsr.Value(GetPlaybackPosition() * kAdsrTime, &adsrEvent), gTime, true);
      }
   }
}

float CurveLooper::GetPlaybackPosition()
{
   if (mLength < 0)
   {
      float ret = TheTransport->GetMeasurePos(gTime) * (-mLength);
      return FloatWrap(ret, 1);
   }
   return (TheTransport->GetMeasurePos(gTime) + TheTransport->GetMeasure(gTime) % mLength) / mLength;
}

void CurveLooper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mLengthSelector->Draw();
   mRandomizeButton->Draw();

   mEnvelopeControl.Draw();

   ofPushStyle();
   ofSetColor(ofColor::lime);
   float x = ofLerp(mEnvelopeControl.GetPosition().x, mEnvelopeControl.GetPosition().x + mEnvelopeControl.GetDimensions().x, GetPlaybackPosition());
   ofLine(x, mEnvelopeControl.GetPosition().y, x, mEnvelopeControl.GetPosition().y + mEnvelopeControl.GetDimensions().y);
   ofPopStyle();
}

void CurveLooper::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   mEnvelopeControl.OnClicked(x, y, right);
}

void CurveLooper::MouseReleased()
{
   IDrawableModule::MouseReleased();

   mEnvelopeControl.MouseReleased();
}

bool CurveLooper::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mEnvelopeControl.MouseMoved(x, y);

   return false;
}

void CurveLooper::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (size_t i = 0; i < mUIControls.size(); ++i)
   {
      if (i < mControlCable->GetPatchCables().size())
         mUIControls[i] = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[i]->GetTarget());
      else
         mUIControls[i] = nullptr;
   }
}

void CurveLooper::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void CurveLooper::DropdownUpdated(DropdownList* list, int oldVal, double time)
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

void CurveLooper::ButtonClicked(ClickButton* button, double time)
{
   if (button == mRandomizeButton)
   {
      mAdsr.SetNumStages(gRandom() % 6 + 2);
      std::vector<float> times;
      for (int i = 0; i < mAdsr.GetNumStages(); ++i)
         times.push_back(ofRandom(1, kAdsrTime - 1));
      std::sort(times.begin(), times.end());
      float timeElapsed = 0;
      for (int i = 0; i < mAdsr.GetNumStages(); ++i)
      {
         mAdsr.GetStageData(i).time = times[i] - timeElapsed;
         mAdsr.GetStageData(i).target = ofRandom(0, 1);
         float val = ofRandom(-1, 1);
         mAdsr.GetStageData(i).curve = val * val * (val > 0 ? 1 : -1);
         timeElapsed += mAdsr.GetStageData(i).time;
      }
   }
}

void CurveLooper::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void CurveLooper::Resize(float w, float h)
{
   mWidth = MAX(w, 200);
   mHeight = MAX(h, 120);
   mEnvelopeControl.SetDimensions(ofVec2f(mWidth - 10, mHeight - 30));
}

void CurveLooper::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void CurveLooper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("width", moduleInfo, 200, 120, 1000);
   mModuleSaveData.LoadInt("height", moduleInfo, 120, 15, 1000);

   SetUpFromSaveData();
}

void CurveLooper::SetUpFromSaveData()
{
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
   Resize(mWidth, mHeight);
}

void CurveLooper::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   mAdsr.SaveState(out);
}

void CurveLooper::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 1)
      mAdsr.LoadState(in);
}
