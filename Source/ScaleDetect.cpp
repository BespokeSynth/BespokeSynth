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
, mLastNote(0)
, mDoDetect(true)
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
   mListMutex.lock();
   mMatchesDropdown->Draw();
   mListMutex.unlock();

   DrawTextNormal(NoteName(mLastNote), 5, 12);
   
   if (mNotes.size() <= 7)
   {
      string pitchString;
      vector<int> rootRelative;
      for (int i=0; i<mNotes.size(); ++i)
         rootRelative.push_back((mNotes[i]-mLastNote+120)%TheScale->GetTet());
      sort(rootRelative.begin(), rootRelative.end());
      for (int i=0; i<rootRelative.size(); ++i)
         pitchString += ofToString(rootRelative[i]) + " ";
      DrawTextNormal(pitchString, 40, 30);
   }
}

void ScaleDetect::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   //TODO_PORT(Ryan) threads get fucked up here, adding a label causes text width to be calculated, which locks a mutex
   /*if (velocity > 0)
   {
      mListMutex.lock();
      mMatchesDropdown->Clear();
      int numMatches = 0;
      mSelectedMatch = 0;
      
      if (!VectorContains(pitch%TheScale->GetTet(),mNotes))
         mNotes.push_back(pitch % TheScale->GetTet());
      mLastNote = pitch;

      if (mDoDetect)
      {
         int numScaleTypes = TheScale->GetNumScaleTypes();
         for (int j=0; j<numScaleTypes-1; ++j)
         {
            if (ScaleSatisfied(pitch%TheScale->GetTet(), TheScale->GetScaleName(j)))
            {
               mMatchesDropdown->AddLabel(TheScale->GetScaleName(j).c_str(), numMatches++);
            }
         }
      }
      mListMutex.unlock();
   }*/
}

bool ScaleDetect::ScaleSatisfied(int root, string type)
{
   ScalePitches scale;
   scale.SetRoot(root);
   scale.SetScaleType(type);
   
   for (auto i = mNotes.begin(); i != mNotes.end(); ++i)
   {
      if (!scale.IsInScale(*i))
         return false;
   }
   return true;
}

void ScaleDetect::ButtonClicked(ClickButton *button)
{
   if (button == mResetButton)
   {
      mNotes.clear();
      mListMutex.lock();
      mMatchesDropdown->Clear();
      mListMutex.unlock();
   }
}

void ScaleDetect::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mMatchesDropdown)
   {
      mListMutex.lock();
      TheScale->SetScale(mLastNote, mMatchesDropdown->GetLabel(mSelectedMatch));
      mListMutex.unlock();
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


