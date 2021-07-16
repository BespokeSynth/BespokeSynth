/*
  ==============================================================================

    NoteStepper.cpp
    Created: 15 Jul 2021 9:11:23pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteStepper.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

NoteStepper::NoteStepper()
: mCurrentDestinationIndex(-1)
, mLength(4)
, mLastNoteOnTime(-9999)
{
   for (int i=0; i<128; ++i)
      mLastNoteDestinations[i] = -1;
}

void NoteStepper::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK(3, 3, 15+(kMaxDestinations-1)*15);
   INTSLIDER(mLengthSlider,"length",&mLength,1,kMaxDestinations);
   ENDUIBLOCK(mWidth,mHeight);
   mHeight += 15;
   
   for (int i=0; i<kMaxDestinations; ++i)
   {
      mDestinationCables[i] = new PatchCableSource(this, kConnectionType_Note);
      mDestinationCables[i]->SetOverrideCableDir(ofVec2f(0,1));
      AddPatchCableSource(mDestinationCables[i]);
      mDestinationCables[i]->SetManualPosition(10+i*15, mHeight-8);
   }
   
   GetPatchCableSource()->SetEnabled(false);
}

void NoteStepper::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mLengthSlider->Draw();
   
   for (int i=0; i<kMaxDestinations; ++i)
   {
      mDestinationCables[i]->SetEnabled(i < mLength);
      if (i == mCurrentDestinationIndex)
      {
         ofPushStyle();
         ofSetColor(255,255,255);
         ofCircle(10+i*15, mHeight-8, 6);
         ofPopStyle();
      }
   }
}

void NoteStepper::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   int selectedDestination = 0;
   if (velocity > 0)
   {
      if (time > mLastNoteOnTime + 10) //slop, to make a chord count as a single step
         mCurrentDestinationIndex = (mCurrentDestinationIndex + 1) % mLength;
      
      selectedDestination = mCurrentDestinationIndex;
      mLastNoteOnTime = time;
      
      if (mLastNoteDestinations[pitch] != -1 && mLastNoteDestinations[pitch] != selectedDestination)
         SendNoteToIndex(mLastNoteDestinations[pitch], time, pitch, 0, voiceIdx, modulation);
      mLastNoteDestinations[pitch] = selectedDestination;
   }
   else
   {
      selectedDestination = mLastNoteDestinations[pitch];
      if (selectedDestination == -1)
         return;
      mLastNoteDestinations[pitch] = -1;
   }
   
   SendNoteToIndex(selectedDestination, time, pitch, velocity, voiceIdx, modulation);
}

void NoteStepper::SendNoteToIndex(int index, double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   const vector<INoteReceiver*>& receivers = mDestinationCables[index]->GetNoteReceivers();
   mDestinationCables[index]->AddHistoryEvent(gTime, velocity > 0);
   for (auto* receiver : receivers)
      receiver->PlayNote(time, pitch, velocity, voiceIdx, modulation);
}

void NoteStepper::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteStepper::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoteStepper::SetUpFromSaveData()
{
}

void NoteStepper::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
}
