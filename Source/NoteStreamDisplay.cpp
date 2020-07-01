/*
  ==============================================================================

    NoteStreamDisplay.cpp
    Created: 21 May 2020 11:13:12pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteStreamDisplay.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"

NoteStreamDisplay::NoteStreamDisplay()
: mWidth(400)
, mHeight(200)
, mDurationMs(2000)
, mPitchMin(127)
, mPitchMax(0)
{
   for (int i=0; i<kNoteStreamCapacity; ++i)
   {
      mNoteStream[i].timeOn = -1;
   }
}

void NoteStreamDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   UIBLOCK0();
   BUTTON(mResetButton, "reset");
   ENDUIBLOCK0();
}

void NoteStreamDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   int barLines = int(ceil(mDurationMs / TheTransport->MsPerBar()));
   ofSetColor(150, 150, 150);
   for (int i=0; i<barLines; ++i)
   {
      double measureStartTime = gTime - TheTransport->MsPerBar() * (TheTransport->GetMeasurePos(gTime) + i);
      float x = ofMap(gTime - measureStartTime, mDurationMs, 0, 0, mWidth);
      ofLine(x, 0, x, mHeight);
   }
   
   ofFill();
   if (mPitchMin <= mPitchMax)
   {
      float noteHeight = mHeight / (mPitchMax - mPitchMin + 1);
      
      for (int i=0; i<kNoteStreamCapacity; ++i)
      {
         if (IsElementActive(i))
         {
            float xStart = ofMap(gTime - mNoteStream[i].timeOn, mDurationMs, 0, 0, mWidth);
            float xEnd;
            if (mNoteStream[i].timeOff == -1)
               xEnd = mWidth;
            else
               xEnd = ofMap(gTime - mNoteStream[i].timeOff, mDurationMs, 0, 0, mWidth);
            float yStart = GetYPos(mNoteStream[i].pitch, noteHeight);
            
            ofSetColor(0,ofMap(mNoteStream[i].velocity, 0, 127.0f, 50, 200),0);
            ofRect(xStart, yStart, xEnd - xStart, noteHeight, L(cornerRadius, 2));
            
            ofSetColor(mNoteStream[i].velocity / 127.0f * 255, mNoteStream[i].velocity / 127.0f * 255, mNoteStream[i].velocity / 127.0f * 255);
            ofRect(xStart, yStart, 3, noteHeight, L(cornerRadius, 2));
            
            ofSetColor(0,0,0);
            ofRect(xEnd-3, yStart, 3, noteHeight, L(cornerRadius, 2));
         }
      }
      
      ofSetColor(100,100,255);
      bool* notes = mNoteOutput.GetNotes();
      for (int i=mPitchMin; i<=mPitchMax; ++i)
      {
         if (notes[i])
            ofRect(mWidth-3, GetYPos(i, noteHeight), 3, noteHeight, L(cornerRadius, 2));
      }
   }
   
   mResetButton->Draw();
}

float NoteStreamDisplay::GetYPos(int pitch, float noteHeight) const
{
   return ofMap(pitch, mPitchMin, mPitchMax+1, mHeight - noteHeight, 0);
}

void NoteStreamDisplay::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (velocity > 0)
   {
      bool inserted = false;
      double oldest = -1;
      int oldestIndex = -1;
      for (int i=0; i<kNoteStreamCapacity; ++i)
      {
         if (!IsElementActive(i))
         {
            mNoteStream[i].pitch = pitch;
            mNoteStream[i].velocity = velocity;
            mNoteStream[i].timeOn = time;
            mNoteStream[i].timeOff = -1;
            
            inserted = true;
            break;
         }
         else
         {
            if (mNoteStream[i].timeOn < oldest)
            {
               oldest = mNoteStream[i].timeOn;
               oldestIndex = i;
            }
         }
      }
      
      if (!inserted && oldestIndex != -1)
      {
         mNoteStream[oldestIndex].pitch = pitch;
         mNoteStream[oldestIndex].velocity = velocity;
         mNoteStream[oldestIndex].timeOn = time;
         mNoteStream[oldestIndex].timeOff = -1;
      }
      
      if (pitch < mPitchMin)
         mPitchMin = pitch;
      if (pitch > mPitchMax)
         mPitchMax = pitch;
   }
   else
   {
      for (int i=0; i<kNoteStreamCapacity; ++i)
      {
         if (mNoteStream[i].pitch == pitch &&
             mNoteStream[i].timeOff == -1 &&
             mNoteStream[i].timeOn < time)
            mNoteStream[i].timeOff = time;
      }
   }
}

bool NoteStreamDisplay::IsElementActive(int index) const
{
   return mNoteStream[index].timeOn != -1 && (mNoteStream[index].timeOff == -1 || gTime - mNoteStream[index].timeOff < mDurationMs);
}

void NoteStreamDisplay::ButtonClicked(ClickButton* button)
{
   if (button == mResetButton)
   {
      mPitchMin = 127;
      mPitchMax = 0;
      for (int i=0; i<kNoteStreamCapacity; ++i)
      {
         if (IsElementActive(i))
         {
            if (mNoteStream[i].pitch < mPitchMin)
               mPitchMin = mNoteStream[i].pitch;
            if (mNoteStream[i].pitch > mPitchMax)
               mPitchMax = mNoteStream[i].pitch;
         }
      }
   }
}

void NoteStreamDisplay::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;
}

void NoteStreamDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("duration_ms", moduleInfo, 2000, 0, 999999, K(isTextField));
   mModuleSaveData.LoadInt("width", moduleInfo, 400, 50, 999999, K(isTextField));
   mModuleSaveData.LoadInt("height", moduleInfo, 200, 50, 999999, K(isTextField));
   
   SetUpFromSaveData();
}

void NoteStreamDisplay::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void NoteStreamDisplay::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mDurationMs = mModuleSaveData.GetFloat("duration_ms");
   mWidth = mModuleSaveData.GetInt("width");
   mHeight = mModuleSaveData.GetInt("height");
}
