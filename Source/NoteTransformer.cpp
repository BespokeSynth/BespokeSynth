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
   for (int i = 0; i < 7; ++i)
   {
      mToneModSlider[i] = new IntSlider(this, ("tone " + ofToString(i)).c_str(), 17, 118 - i * 17, 100, 15, &mToneMod[i], -7, 7);
   }

   for (int i = 0; i < 127; ++i)
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
   for (int i = 0; i < 7; ++i)
   {
      mToneModSlider[i]->Draw();

      if (gTime - mLastTimeTonePlayed[i] > 0 && gTime - mLastTimeTonePlayed[i] < 200)
      {
         float alpha = 1 - (gTime - mLastTimeTonePlayed[i]) / 200;
         ofSetColor(0, 255, 0, alpha * 255);
         ofRect(2, 118 - i * 17, 10, 10);
      }
   }
}

void NoteTransformer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void NoteTransformer::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.velocity == 0) //note off the one we played for this pitch, in case the transformer changed it while the note was held
   {
      if (mLastNoteOnForPitch[note.pitch] != -1)
      {
         NoteMessage noteOff = note.MakeNoteOff();
         note.pitch = mLastNoteOnForPitch[note.pitch];
         PlayNoteOutput(noteOff);
      }
      return;
   }

   int tone = TheScale->GetToneFromPitch(note.pitch);
   if (note.velocity > 0)
      mLastTimeTonePlayed[tone % 7] = note.time;
   int pitchOffset = note.pitch - TheScale->GetPitchFromTone(tone);

   tone += mToneMod[tone % TheScale->NumTonesInScale()];

   int outPitch = TheScale->GetPitchFromTone(tone) + pitchOffset;
   note.pitch = outPitch;
   PlayNoteOutput(note);
   mLastNoteOnForPitch[note.pitch] = outPitch;
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
