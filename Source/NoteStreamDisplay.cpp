/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
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
{
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
   for (int i = 0; i < barLines; ++i)
   {
      double measureStartTime = gTime - TheTransport->MsPerBar() * (TheTransport->GetMeasurePos(gTime) + i);
      float x = ofMap(gTime - measureStartTime, mDurationMs, 0, 0, mWidth);
      ofLine(x, 0, x, mHeight);
   }

   ofFill();
   if (mPitchMin <= mPitchMax)
   {
      float noteHeight = mHeight / (mPitchMax - mPitchMin + 1);

      for (int i = 0; i < kNoteStreamCapacity; ++i)
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

            ofSetColor(0, ofMap(mNoteStream[i].velocity, 0, 127.0f, 50, 200), 0);
            ofRect(xStart, yStart, xEnd - xStart, noteHeight, L(cornerRadius, 2));

            ofSetColor(mNoteStream[i].velocity / 127.0f * 255, mNoteStream[i].velocity / 127.0f * 255, mNoteStream[i].velocity / 127.0f * 255);
            ofRect(xStart, yStart, 3, noteHeight, L(cornerRadius, 2));

            ofSetColor(0, 0, 0);
            ofRect(xEnd - 3, yStart, 3, noteHeight, L(cornerRadius, 2));
         }
      }

      ofSetColor(100, 100, 255);
      bool* notes = mNoteOutput.GetNotes();
      for (int i = mPitchMin; i <= mPitchMax; ++i)
      {
         if (notes[i])
            ofRect(mWidth - 3, GetYPos(i, noteHeight), 3, noteHeight, L(cornerRadius, 2));
      }
   }

   mResetButton->Draw();
}

void NoteStreamDisplay::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      DrawTextNormal(mDebugDisplayText, mWidth + 10, 0);
   }
}

float NoteStreamDisplay::GetYPos(int pitch, float noteHeight) const
{
   return ofMap(pitch, mPitchMin, mPitchMax + 1, mHeight - noteHeight, -noteHeight);
}

void NoteStreamDisplay::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (note.velocity > 0)
   {
      bool inserted = false;
      double oldest = -1;
      int oldestIndex = -1;
      for (int i = 0; i < kNoteStreamCapacity; ++i)
      {
         if (!IsElementActive(i))
         {
            mNoteStream[i].pitch = note.pitch;
            mNoteStream[i].velocity = note.velocity;
            mNoteStream[i].timeOn = note.time;
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
         mNoteStream[oldestIndex].pitch = note.pitch;
         mNoteStream[oldestIndex].velocity = note.velocity;
         mNoteStream[oldestIndex].timeOn = note.time;
         mNoteStream[oldestIndex].timeOff = -1;
      }

      if (note.pitch < mPitchMin)
         mPitchMin = note.pitch;
      if (note.pitch > mPitchMax)
         mPitchMax = note.pitch;
   }
   else
   {
      for (int i = 0; i < kNoteStreamCapacity; ++i)
      {
         if (mNoteStream[i].pitch == note.pitch &&
             mNoteStream[i].timeOff == -1 &&
             mNoteStream[i].timeOn < note.time)
            mNoteStream[i].timeOff = note.time;
      }
   }

   if (mDrawDebug)
      AddDebugLine("PlayNote(" + ofToString(note.time / 1000) + ", " + ofToString(note.pitch) + ", " + ofToString(note.velocity) + ", " + ofToString(note.voiceIdx) + ")", 35);
}

bool NoteStreamDisplay::IsElementActive(int index) const
{
   return mNoteStream[index].timeOn != -1 && (mNoteStream[index].timeOff == -1 || gTime - mNoteStream[index].timeOff < mDurationMs);
}

void NoteStreamDisplay::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
   {
      mPitchMin = 127;
      mPitchMax = 0;
      for (int i = 0; i < kNoteStreamCapacity; ++i)
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
