//
//  NoteFilter.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/28/15.
//
//

#include "NoteFilter.h"
#include "SynthGlobals.h"

NoteFilter::NoteFilter()
: mMinPitch(0)
, mMaxPitch(7)
{
   for (int i=0; i<128; ++i)
   {
      mGate[i] = true;
      mLastPlayTime[i] = -999;
   }
}

NoteFilter::~NoteFilter()
{
}

void NoteFilter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteFilter::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   int pitch = mMinPitch;
   for (auto* checkbox : mGateCheckboxes)
   {
      checkbox->Draw();
      ofPushStyle();
      ofFill();
      ofSetColor(0,255,0,(1 - ofClamp((gTime - mLastPlayTime[pitch])/250,0,1))*255);
      ofRect(75,checkbox->GetPosition(true).y+4,8,8);
      ofPopStyle();
      ++pitch;
   }
}

void NoteFilter::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mEnabled)
   {
      if (pitch >= 0 && pitch < 128)
      {
         mLastPlayTime[pitch] = time;
         if (mGate[pitch])
            PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
}

void NoteFilter::GetModuleDimensions(float& width, float& height)
{
   width = 80;
   height = 3 + (mMaxPitch - mMinPitch + 1) * 18;
}

void NoteFilter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("min pitch", moduleInfo, 0, 0, 127, K(isTextField));
   mModuleSaveData.LoadInt("max pitch", moduleInfo, 7, 0, 127, K(isTextField));
   
   SetUpFromSaveData();
}

void NoteFilter::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   
   mMinPitch = mModuleSaveData.GetInt("min pitch");
   mMaxPitch = mModuleSaveData.GetInt("max pitch");
   
   for (auto* checkbox : mGateCheckboxes)
      RemoveUIControl(checkbox);
   mGateCheckboxes.clear();
   
   int numCheckboxes = (mMaxPitch - mMinPitch + 1);
   for (int i=0; i<numCheckboxes; ++i)
   {
      int pitch = i+mMinPitch;
      Checkbox* checkbox = new Checkbox(this,(NoteName(pitch) + ofToString(pitch/12 - 2) + " (" + ofToString(pitch) + ")").c_str(),3,3+i*18,&mGate[pitch]);
      mGateCheckboxes.push_back(checkbox);
   }
}
