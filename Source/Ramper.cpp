//
//  Ramper.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#include "Ramper.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

Ramper::Ramper()
: mUIControl(nullptr)
, mLength(kInterval_1n)
, mLengthSelector(nullptr)
, mControlCable(nullptr)
, mTriggerButton(nullptr)
, mStartMeasure(0)
, mStartValue(0)
, mRamping(false)
, mTargetValue(0)
, mTargetValueSlider(nullptr)
{
   TheTransport->AddAudioPoller(this);
}

Ramper::~Ramper()
{
   TheTransport->RemoveAudioPoller(this);
}

void Ramper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mLengthSelector = new DropdownList(this,"length",3,3,(int*)(&mLength));
   mTriggerButton = new ClickButton(this,"start",67,3);
   mTargetValueSlider = new FloatSlider(this,"target",3,20,94,15,&mTargetValue,0,1);
   
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);
   
   mLengthSelector->AddLabel("64", kInterval_64);
   mLengthSelector->AddLabel("32", kInterval_32);
   mLengthSelector->AddLabel("16", kInterval_16);
   mLengthSelector->AddLabel("8", kInterval_8);
   mLengthSelector->AddLabel("4", kInterval_4);
   mLengthSelector->AddLabel("2", kInterval_2);
   mLengthSelector->AddLabel("1n", kInterval_1n);
   mLengthSelector->AddLabel("2n", kInterval_2n);
   mLengthSelector->AddLabel("4n", kInterval_4n);
   mLengthSelector->AddLabel("4nt", kInterval_4nt);
   mLengthSelector->AddLabel("8n", kInterval_8n);
   mLengthSelector->AddLabel("8nt", kInterval_8nt);
   mLengthSelector->AddLabel("16n", kInterval_16n);
   mLengthSelector->AddLabel("16nt", kInterval_16nt);
   mLengthSelector->AddLabel("32n", kInterval_32n);
   mLengthSelector->AddLabel("64n", kInterval_64n);
}

void Ramper::OnTransportAdvanced(float amount)
{
   if (mUIControl && mRamping)
   {
      float curMeasure = TheTransport->GetMeasure(gTime) + TheTransport->GetMeasurePos(gTime);
      float measureProgress = curMeasure - mStartMeasure;
      float length = TheTransport->GetDuration(mLength) / TheTransport->MsPerBar();
      float progress = measureProgress / length;
      if (progress >= 0 && progress < 1)
      {
         
         mUIControl->SetValue(ofLerp(mStartValue, mTargetValue, progress));
      }
      else
      {
         mRamping = false;
      }
   }
}

void Ramper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mLengthSelector->Draw();
   mTriggerButton->Draw();
   mTargetValueSlider->Draw();
}

void Ramper::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
}

void Ramper::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool Ramper::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   return false;
}

void Ramper::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (mControlCable->GetPatchCables().empty() == false)
   {
      mUIControl = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[0]->GetTarget());
      FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(mUIControl);
      if (floatSlider)
         mTargetValueSlider->MatchExtents(floatSlider);
   }
   else
   {
      mUIControl = nullptr;
   }
}

void Ramper::ButtonClicked(ClickButton* button)
{
   if (button == mTriggerButton)
   {
      if (mUIControl)
      {
         mStartValue = mUIControl->GetValue();
         mStartMeasure = TheTransport->GetMeasureTime(gTime);
         mRamping = true;
      }
   }
}

void Ramper::GetModuleDimensions(float& width, float& height)
{
   width = 100;
   height = 38;
}

void Ramper::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["uicontrol"] = mUIControl ? mUIControl->Path() : "";
}

void Ramper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol", moduleInfo);
   
   SetUpFromSaveData();
}

void Ramper::SetUpFromSaveData()
{
   string controlPath = mModuleSaveData.GetString("uicontrol");
   if (!controlPath.empty())
   {
      mUIControl = TheSynth->FindUIControl(controlPath);
      if (mUIControl)
         mControlCable->SetTarget(mUIControl);
   }
   else
   {
      mUIControl = nullptr;
   }
}
