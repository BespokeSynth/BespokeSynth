//
//  Chorder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#include "Chorder.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PolyphonyMgr.h"

Chorder::Chorder()
: mVelocity(0)
{
   bzero(mHeldCount, TOTAL_NUM_NOTES*sizeof(int));
   bzero(mInputNotes, TOTAL_NUM_NOTES*sizeof(bool));
}

void Chorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mChordGrid = new UIGrid(2,2,130,50,7,3);
   mChordGrid->SetVal(0, 1, 1);
   mChordGrid->SetListener(this);
}

void Chorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mChordGrid->Draw();
}

void Chorder::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   int tone = col + (mChordGrid->GetRows()/2-row)*mChordGrid->GetCols();
   if (value > 0 && oldValue == 0)
   {
      AddTone(tone, value*value);
   }
   else if (value == 0 && oldValue > 0)
   {
      RemoveTone(tone);
   }
}

void Chorder::AddTone(int tone, float velocity)
{
   mChordGrid->SetVal((tone+mChordGrid->GetCols()*10) % mChordGrid->GetCols(),
                      mChordGrid->GetRows() - 1 - (tone + (mChordGrid->GetRows()/2 * mChordGrid->GetCols())) / mChordGrid->GetCols(),
                      sqrtf(velocity), !K(notifyListeners));
   for (int i=0; i<TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         int chordtone = tone + TheScale->GetToneFromPitch(i);
         int outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(chordtone));
         PlayChorderNote(gTime, outPitch, mVelocity * velocity);
      }
   }
}

void Chorder::RemoveTone(int tone)
{
   mChordGrid->SetVal((tone+mChordGrid->GetCols()*10) % mChordGrid->GetCols(),
                      mChordGrid->GetRows() - 1 - (tone + (mChordGrid->GetRows()/2 * mChordGrid->GetCols())) / mChordGrid->GetCols(),
                      0, !K(notifyListeners));
   for (int i=0; i<TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         int chordtone = tone + TheScale->GetToneFromPitch(i);
         int outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(chordtone));
         PlayChorderNote(gTime, outPitch, 0);
      }
   }
}

void Chorder::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   mChordGrid->TestClick(x, y, right);
}

void Chorder::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mChordGrid->MouseReleased();
}

bool Chorder::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mChordGrid->NotifyMouseMoved(x,y);
   return false;
}

void Chorder::CheckboxUpdated(Checkbox *checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush();
      bzero(mHeldCount,TOTAL_NUM_NOTES*sizeof(int));
      bzero(mInputNotes, TOTAL_NUM_NOTES*sizeof(bool));
   }
}

void Chorder::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      return;
   }

   mInputNotes[pitch] = velocity > 0;
   
   if (velocity > 0)
      mVelocity = velocity;

   int idx = 0;
   for (int row=0; row<mChordGrid->GetRows(); ++row)
   {
      for (int col=0; col<mChordGrid->GetCols(); ++col)
      {
         float val = mChordGrid->GetVal(col, row);
         if (val > 0)
         {
            int chordTone = col + (mChordGrid->GetRows()/2-row)*mChordGrid->GetCols();
            int voice = (voiceIdx == -1) ? -1 : (voiceIdx + idx) % 16;
            int outPitch;
            
            if (chordTone%TheScale->NumPitchesInScale() == 0) //if this is the pressed note or an octave of it
            {
               //play the pressed note (might not be in scale, so play it directly)
               int octave = chordTone/TheScale->NumPitchesInScale();
               outPitch = pitch+TheScale->GetTet()*octave;
            }
            else
            {
               int tone = chordTone + TheScale->GetToneFromPitch(pitch);
               outPitch = TheScale->MakeDiatonic(TheScale->GetPitchFromTone(tone));
            }
            
            PlayChorderNote(time, outPitch, velocity*val*val, voice, pitchBend, modWheel, pressure);
            
            ++idx;
         }
      }
   }
   CheckLeftovers();
}

void Chorder::PlayChorderNote(double time, int pitch, int velocity, int voice /*=-1*/, ModulationChain* pitchBend /*= nullptr*/, ModulationChain* modWheel /*= nullptr*/, ModulationChain* pressure /*= nullptr*/)
{
   assert(velocity >= 0);
   
   if (pitch < 0 || pitch >= TOTAL_NUM_NOTES)
      return;
   
   bool wasOn = mHeldCount[pitch] > 0;
   
   if (velocity > 0)
      ++mHeldCount[pitch];
   
   if (velocity == 0 && mHeldCount[pitch] > 0)
      --mHeldCount[pitch];
   
   if (mHeldCount[pitch] > 0 && !wasOn)
      PlayNoteOutput(time, pitch, velocity, voice, pitchBend, modWheel, pressure);
   if (mHeldCount[pitch] == 0 && wasOn)
      PlayNoteOutput(time, pitch, 0, voice, pitchBend, modWheel, pressure);
   
   //ofLog() << ofToString(pitch) + " " + ofToString(velocity) + ": " + ofToString(mHeldCount[pitch]) + " " + ofToString(voice);
}

void Chorder::CheckLeftovers()
{
   bool anyHeldNotes = false;
   for (int i=0; i<TOTAL_NUM_NOTES; ++i)
   {
      if (mInputNotes[i])
      {
         anyHeldNotes = true;
         break;
      }
   }
   
   if (!anyHeldNotes)
   {
      for (int i=0; i<TOTAL_NUM_NOTES; ++i)
      {
         if (mHeldCount[i] > 0)
         {
            ofLog() << "Somehow there are still notes in the count! Clearing";
            PlayNoteOutput(gTime, i, 0, -1);
            mHeldCount[i] = 0;
         }
      }
   }
}

void Chorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("multislider_mode", moduleInfo, true);

   SetUpFromSaveData();
}

void Chorder::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   
   bool multisliderMode = mModuleSaveData.GetBool("multislider_mode");
   mChordGrid->SetGridMode(multisliderMode ? UIGrid::kMultislider : UIGrid::kNormal);
   mChordGrid->SetRestrictDragToRow(multisliderMode);
   mChordGrid->SetClickClearsToZero(true);//!multisliderMode);
}

namespace
{
   const int kSaveStateRev = 0;
}

void Chorder::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mChordGrid->SaveState(out);
}

void Chorder::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mChordGrid->LoadState(in);
}


