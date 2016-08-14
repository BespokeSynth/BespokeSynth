//
//  MacroSlider.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/15.
//
//

#include "MacroSlider.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Transport.h"

MacroSlider::MacroSlider()
: mSlider(NULL)
, mValue(0)
{
   TheTransport->AddAudioPoller(this);
}

MacroSlider::~MacroSlider()
{
   mMappingMutex.lock();
   for (auto mapping : mMappings)
      delete mapping;
   mMappingMutex.unlock();
   TheTransport->RemoveAudioPoller(this);
}

void MacroSlider::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mSlider = new FloatSlider(this, "input", 5, 4, 100, 15, &mValue, 0, 1);
}

void MacroSlider::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mSlider->Draw();
   
   mMappingMutex.lock();
   for (auto mapping : mMappings)
      mapping->Draw();
   mMappingMutex.unlock();
}

void MacroSlider::OnTransportAdvanced(float amount)
{
   mSlider->Compute();
}

void MacroSlider::PostRepatch(PatchCableSource* cableSource)
{
   mMappingMutex.lock();
   for (auto mapping : mMappings)
      mapping->UpdateControl();
   mMappingMutex.unlock();
}

void MacroSlider::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mSlider)
   {
      UpdateValues();
   }
}

void MacroSlider::UpdateValues()
{
   mMappingMutex.lock();
   for (auto mapping : mMappings)
      mapping->UpdateValue(mValue);
   mMappingMutex.unlock();
}

void MacroSlider::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   mMappingMutex.lock();
   moduleInfo["num_mappings"] = (int)mMappings.size();
   for (int i=0; i<mMappings.size(); ++i)
   {
      string targetPath = "";
      if (mMappings[i]->mControl)
         targetPath = mMappings[i]->mControl->Path();
      
      moduleInfo["mappings"][i]["target"] = targetPath;
   }
   mMappingMutex.unlock();
}

void MacroSlider::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_mappings", moduleInfo, 3, 1, 100, K(isTextField));
   
   mMappingMutex.lock();
   for (auto mapping : mMappings)
      delete mapping;
   mMappings.clear();
   const Json::Value& mappings = moduleInfo["mappings"];
   for (int i=0; i<mappings.size(); ++i)
   {
      string target = mappings[i]["target"].asString();
      Mapping* mapping = new Mapping(this, i);
      mapping->CreateUIControls();
      FloatSlider* slider = dynamic_cast<FloatSlider*>(TheSynth->FindUIControl(target));
      mapping->mCableSource->SetTarget(slider);
      mapping->UpdateControl();
      mMappings.push_back(mapping);
   }
   mMappingMutex.unlock();
   
   SetUpFromSaveData();
}

void MacroSlider::SetUpFromSaveData()
{
   mMappingMutex.lock();
   int newNumMappings = mModuleSaveData.GetInt("num_mappings");
   if (mMappings.size() > newNumMappings)
   {
      for (int i=newNumMappings; i<mMappings.size(); ++i)
         delete mMappings[i];
   }
   mMappings.resize(newNumMappings);
   
   for (int i=0; i<mMappings.size(); ++i)
   {
      if (mMappings[i] == NULL)
      {
         Mapping* mapping = new Mapping(this, i);
         mapping->CreateUIControls();
         mMappings[i] = mapping;
      }
   }
   mMappingMutex.unlock();
}

void MacroSlider::PostLoadState()
{
   IDrawableModule::PostLoadState();
   UpdateValues();
}

MacroSlider::Mapping::Mapping(MacroSlider* owner, int index)
: mStart(0)
, mEnd(1)
, mCableSource(NULL)
, mControl(NULL)
, mOwner(owner)
, mIndex(index)
{
}

MacroSlider::Mapping::~Mapping()
{
   mOwner->RemovePatchCableSource(mCableSource);
}

void MacroSlider::Mapping::CreateUIControls()
{
   mStartSlider = new FloatSlider(mOwner,"start", 5, 25+mIndex*kMappingSpacing, 100, 15, &mStart, 0, 1);
   mEndSlider = new FloatSlider(mOwner,"end", 5, 39+mIndex*kMappingSpacing, 100, 15, &mEnd, 0, 1);
   mCableSource = new PatchCableSource(mOwner, kConnectionType_UIControl);
   mCableSource->SetManualPosition(110, 39+mIndex*kMappingSpacing);
   mOwner->AddPatchCableSource(mCableSource);
}

void MacroSlider::Mapping::UpdateControl()
{
   FloatSlider* control = dynamic_cast<FloatSlider*>(mCableSource->GetTarget());
   if (control && control != mControl)
   {
      mControl = control;
      mStartSlider->MatchExtents(mControl);
      mEndSlider->MatchExtents(mControl);
      mStart = mControl->GetMin();
      mEnd = mControl->GetMax();
   }
}

void MacroSlider::Mapping::UpdateValue(float value)
{
   if (mControl)
      mControl->SetValue(ofMap(value,0,1,mStart,mEnd));
}

void MacroSlider::Mapping::Draw()
{
   mStartSlider->Draw();
   mEndSlider->Draw();
   
   if (mControl)
   {
      int x,y,w,h;
      mStartSlider->GetPosition(x, y, K(local));
      mStartSlider->GetDimensions(w, h);
      
      int lineX = ofMap(mControl->GetValue(), mControl->GetMin(), mControl->GetMax(), x, x + w);
      int lineY1 = y;
      int lineY2 = y + h * 2;
      ofPushStyle();
      ofSetColor(ofColor::green);
      ofLine(lineX,lineY1,lineX,lineY2);
      ofPopStyle();
   }
}