//
//  NoteTransformer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/13/14.
//
//

#include "NoteTransformer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

NoteTransformer::NoteTransformer()
{
   for (int i=0; i<7; ++i)
   {
      mToneMod[i] = 0;
      mToneModSlider[i] = new IntSlider(this,("tone "+ofToString(i)).c_str(),17,118-i*17,100,15,&mToneMod[i],-7,7);
      mLastTimeTonePlayed[i] = 0;
   }
   
   for (int i=0; i<127; ++i)
      mLastNoteOnForPitch[i] = -1;
}

NoteTransformer::~NoteTransformer()
{
}

void NoteTransformer::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   ofFill();
   for (int i=0; i<7; ++i)
   {
      mToneModSlider[i]->Draw();
      
      if (gTime - mLastTimeTonePlayed[i] > 0 && gTime - mLastTimeTonePlayed[i] < 200)
      {
         float alpha = 1 - (gTime - mLastTimeTonePlayed[i])/200;
         ofSetColor(0,255,0,alpha*255);
         ofRect(2,118-i*17,10,10);
      }
   }
}

void NoteTransformer::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void NoteTransformer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity == 0)   //note off the one we played for this pitch, in case the transformer changed it while the note was held
   {
      if (mLastNoteOnForPitch[pitch] != -1)
         PlayNoteOutput(time, mLastNoteOnForPitch[pitch], 0, voiceIdx);
      return;
   }
   
   int tone = TheScale->GetToneFromPitch(pitch);
   if (velocity > 0)
      mLastTimeTonePlayed[tone%7] = time;
   int pitchOffset = pitch - TheScale->GetPitchFromTone(tone);
   
   tone += mToneMod[tone % TheScale->NumPitchesInScale()];
   
   int outPitch = TheScale->GetPitchFromTone(tone) + pitchOffset;
   PlayNoteOutput(time, outPitch, velocity, voiceIdx, modulation);
   mLastNoteOnForPitch[pitch] = outPitch;
}

void NoteTransformer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteTransformer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

