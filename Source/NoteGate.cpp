//
//  NoteGate.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/22/16.
//
//

#include "NoteGate.h"
#include "SynthGlobals.h"

NoteGate::NoteGate()
: mMinPitch(0)
, mMaxPitch(7)
{
   for (int i=0; i<128; ++i)
   {
      mGate[i] = true;
      mLastPlayTime[i] = -999;
   }
}

NoteGate::~NoteGate()
{
}

void NoteGate::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void NoteGate::DrawModule()
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

void NoteGate::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (mEnabled)
   {
      if (pitch >= 0 && pitch < 128)
      {
         mLastPlayTime[pitch] = time;
         if (mGate[pitch])
            PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      }
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void NoteGate::GetModuleDimensions(int& width, int& height)
{
   width = 80;
   height = 3 + (mMaxPitch - mMinPitch + 1) * 18;
}

void NoteGate::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("min pitch", moduleInfo, 0, 0, 127, K(isTextField));
   mModuleSaveData.LoadInt("max pitch", moduleInfo, 7, 0, 127, K(isTextField));
   
   SetUpFromSaveData();
}

void NoteGate::SetUpFromSaveData()
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

