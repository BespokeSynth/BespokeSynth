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
: mSlider(nullptr)
, mValue(0)
{
}

MacroSlider::~MacroSlider()
{
   for (auto mapping : mMappings)
      delete mapping;
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
   
   for (auto mapping : mMappings)
      mapping->Draw();
}

void MacroSlider::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (auto mapping : mMappings)
   {
      if (mapping->GetCableSource() == cableSource)
         mapping->UpdateControl();
   }
}

void MacroSlider::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void MacroSlider::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["num_mappings"] = (int)mMappings.size();
   for (int i=0; i<mMappings.size(); ++i)
   {
      string targetPath = "";
      if (mMappings[i]->GetCableSource()->GetTarget())
         targetPath = mMappings[i]->GetCableSource()->GetTarget()->Path();
      
      moduleInfo["mappings"][i]["target"] = targetPath;
   }
}

void MacroSlider::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadInt("num_mappings", moduleInfo, 3, 1, 100, K(isTextField));
   
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
      mapping->GetCableSource()->SetTarget(slider);
      mapping->UpdateControl();
      mMappings.push_back(mapping);
   }
   
   SetUpFromSaveData();
}

void MacroSlider::SetUpFromSaveData()
{
   int newNumMappings = mModuleSaveData.GetInt("num_mappings");
   if (mMappings.size() > newNumMappings)
   {
      for (int i=newNumMappings; i<mMappings.size(); ++i)
         delete mMappings[i];
   }
   mMappings.resize(newNumMappings);
   
   for (int i=0; i<mMappings.size(); ++i)
   {
      if (mMappings[i] == nullptr)
      {
         Mapping* mapping = new Mapping(this, i);
         mapping->CreateUIControls();
         mMappings[i] = mapping;
      }
   }
}

MacroSlider::Mapping::Mapping(MacroSlider* owner, int index)
: mOwner(owner)
, mIndex(index)
{
}

MacroSlider::Mapping::~Mapping()
{
   mOwner->RemovePatchCableSource(mTargetCable);
}

void MacroSlider::Mapping::CreateUIControls()
{
   mMinSlider = new FloatSlider(mOwner,("start"+ofToString(mIndex+1)).c_str(), 5, 25+mIndex*kMappingSpacing, 100, 15, &mDummyMin, 0, 1);
   mMaxSlider = new FloatSlider(mOwner,("end"+ofToString(mIndex+1)).c_str(), 5, 39+mIndex*kMappingSpacing, 100, 15, &mDummyMax, 0, 1);
   mTargetCable = new PatchCableSource(mOwner, kConnectionType_UIControl);
   mTargetCable->SetManualPosition(110, 39+mIndex*kMappingSpacing);
   mOwner->AddPatchCableSource(mTargetCable);
}

void MacroSlider::Mapping::UpdateControl()
{
   OnModulatorRepatch();
}

float MacroSlider::Mapping::Value(int samplesIn)
{
   mOwner->ComputeSliders(samplesIn);
   return ofMap(mOwner->GetValue(), 0, 1, GetMin(), GetMax(), K(clamp));
}

void MacroSlider::Mapping::Draw()
{
   mMinSlider->Draw();
   mMaxSlider->Draw();
   
   if (mTarget)
   {
      float x,y,w,h;
      mMinSlider->GetPosition(x, y, K(local));
      mMinSlider->GetDimensions(w, h);
      
      int lineX = ofMap(mTarget->GetValue(), mTarget->GetMin(), mTarget->GetMax(), x, x + w);
      int lineY1 = y;
      int lineY2 = y + h * 2;
      ofPushStyle();
      ofSetColor(ofColor::green);
      ofLine(lineX,lineY1,lineX,lineY2);
      ofPopStyle();
   }
}
