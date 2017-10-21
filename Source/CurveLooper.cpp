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

CurveLooper::CurveLooper()
: mUIControl(nullptr)
, mInterval(kInterval_16n)
, mIntervalSelector(nullptr)
, mLength(4)
, mLengthSelector(nullptr)
, mControlCable(nullptr)
, mWidth(200)
, mHeight(100)
, mRecord(false)
{
   TheTransport->AddAudioPoller(this);
}

CurveLooper::~CurveLooper()
{
   TheTransport->RemoveAudioPoller(this);
}

void CurveLooper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",5,3,(int*)(&mInterval));
   mLengthSelector = new DropdownList(this,"length",45,3,(int*)(&mLength));
   mRecordCheckbox = new Checkbox(this,"rec",90,3,&mRecord);
   
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);
   
   /*mIntervalSelector->AddLabel("8", kInterval_8);
    mIntervalSelector->AddLabel("4", kInterval_4);
    mIntervalSelector->AddLabel("2", kInterval_2);*/
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   
   mLengthSelector->AddLabel("4n", 1);
   mLengthSelector->AddLabel("2n", 2);
   mLengthSelector->AddLabel("1", 4);
   mLengthSelector->AddLabel("2", 8);
   mLengthSelector->AddLabel("3", 12);
   mLengthSelector->AddLabel("4", 16);
   mLengthSelector->AddLabel("6", 24);
   mLengthSelector->AddLabel("8", 32);
   mLengthSelector->AddLabel("16", 64);
   mLengthSelector->AddLabel("32", 128);
   mLengthSelector->AddLabel("64", 256);
   mLengthSelector->AddLabel("128", 512);
}

void CurveLooper::Init()
{
   IDrawableModule::Init();
   
   mCurve.AddPoint(CurvePoint(.2f, .4f));
   mCurve.AddPoint(CurvePoint(.6f, .9f));
   mCurve.AddPoint(CurvePoint(.7f, .2f));
}

void CurveLooper::Poll()
{
}

void CurveLooper::OnTransportAdvanced(float amount)
{
   if (mUIControl)
   {
      if (mRecord)
      {
         float pos = GetPlaybackPosition();
         mCurve.DeleteBetween(mLastRecordPos, pos+.01f);
         mCurve.AddPoint(CurvePoint(pos, mUIControl->GetMidiValue()));
         mLastRecordPos = pos;
      }
      else
      {
         mUIControl->SetFromMidiCC(mCurve.Evaluate(GetPlaybackPosition()));
      }
   }
}

float CurveLooper::GetPlaybackPosition()
{
   return TheTransport->GetMeasurePos();
}

void CurveLooper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mCurve.SetPosition(5,5);
   mCurve.SetDimensions(mWidth-10, mHeight-10);
   mCurve.Render();
   mIntervalSelector->Draw();
   mLengthSelector->Draw();
   mRecordCheckbox->Draw();
   
   ofPushStyle();
   ofSetColor(ofColor::lime);
   float x = ofLerp(5, mWidth-10, GetPlaybackPosition());
   ofLine(x, 5, x, mHeight - 10);
   ofPopStyle();
}

void CurveLooper::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
}

void CurveLooper::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool CurveLooper::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   return false;
}

void CurveLooper::SetNumSteps(int numSteps, bool stretch)
{
}

void CurveLooper::PostRepatch(PatchCableSource* cableSource)
{
   if (mControlCable->GetPatchCables().empty() == false)
      mUIControl = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[0]->GetTarget());
   else
      mUIControl = nullptr;
}

void CurveLooper::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mRecordCheckbox)
   {
      if (mRecord)
      {
         mLastRecordPos = GetPlaybackPosition();
      }
   }
}

void CurveLooper::DropdownUpdated(DropdownList* list, int oldVal)
{
   int newSteps = int(mLength/4.0f * TheTransport->CountInStandardMeasure(mInterval));
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
   }
}

void CurveLooper::GetModuleDimensions(int &width, int &height)
{
   width = mWidth;
   height = mHeight;
}

void CurveLooper::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}

void CurveLooper::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["uicontrol"] = mUIControl ? mUIControl->Path() : "";
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void CurveLooper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol", moduleInfo);
   mModuleSaveData.LoadInt("width", moduleInfo, 200, 120, 1000);
   mModuleSaveData.LoadInt("height", moduleInfo, 100, 15, 1000);
   
   SetUpFromSaveData();
}

void CurveLooper::SetUpFromSaveData()
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
   
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
}

namespace
{
   const int kSaveStateRev = 0;
}

void CurveLooper::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
//   mGrid->SaveState(out);
}

void CurveLooper::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
//   mGrid->LoadState(in);
}
