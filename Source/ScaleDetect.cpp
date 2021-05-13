//
//  ScaleDetect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 2/10/13.
//
//

#include "ScaleDetect.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"

ScaleDetect::ScaleDetect()
: mResetButton(nullptr)
, mLastPitch(0)
, mDoDetect(true)
, mNeedsUpdate(false)
, mSelectedMatch(0)
, mMatchesDropdown(nullptr)
{
}

void ScaleDetect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mResetButton = new ClickButton(this,"reset",4,18);
   mMatchesDropdown = new DropdownList(this,"matches",25,2,&mSelectedMatch);
}

void ScaleDetect::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mResetButton->Draw();
   mMatchesDropdown->Draw();

   DrawTextNormal(NoteName(mLastPitch), 5, 12);
   
   if (mNeedsUpdate)
   {
      mMatchesDropdown->Clear();
      int numMatches = 0;
      mSelectedMatch = 0;

      if (mDoDetect)
      {
         int numScaleTypes = TheScale->GetNumScaleTypes();
         for (int j=0; j<numScaleTypes-1; ++j)
         {
            if (ScaleSatisfied(mLastPitch%TheScale->GetTet(), TheScale->GetScaleName(j)))
               mMatchesDropdown->AddLabel(TheScale->GetScaleName(j).c_str(), numMatches++);
         }
      }
      
      mNeedsUpdate = false;
   }
   
   {
      string pitchString;
      vector<int> rootRelative;
      for (int i=0; i<128; ++i)
      {
         if (mPitchOn[i])
         {
            int entry = (i-mLastPitch+TheScale->GetTet()*10)%TheScale->GetTet();
            if (!VectorContains(entry, rootRelative))
               rootRelative.push_back(entry);
         }
      }
      sort(rootRelative.begin(), rootRelative.end());
      for (int i=0; i<rootRelative.size(); ++i)
         pitchString += ofToString(rootRelative[i]) + " ";
      DrawTextNormal(pitchString, 40, 30);
   }
}

void ScaleDetect::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (velocity > 0 && pitch >= 0 && pitch < 128)
   {
      mPitchOn[pitch] = true;
      mLastPitch = pitch;
      mNeedsUpdate = true;
   }
}

bool ScaleDetect::ScaleSatisfied(int root, string type)
{
   ScalePitches scale;
   scale.SetRoot(root);
   scale.SetScaleType(type);
   
   for (int i=0; i<128; ++i)
   {
      if (mPitchOn[i] && !scale.IsInScale(i))
         return false;
   }
   return true;
}

void ScaleDetect::ButtonClicked(ClickButton *button)
{
   if (button == mResetButton)
   {
      for (int i=0; i<128; ++i)
         mPitchOn[i] = false;
      mMatchesDropdown->Clear();
      mNeedsUpdate = true;
   }
}

void ScaleDetect::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mMatchesDropdown)
   {
      TheScale->SetScale(mLastPitch, mMatchesDropdown->GetLabel(mSelectedMatch));
   }
}

void ScaleDetect::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void ScaleDetect::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


