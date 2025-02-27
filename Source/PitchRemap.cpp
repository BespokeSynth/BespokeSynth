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

    PitchRemap.cpp
    Created: 7 May 2021 10:17:55pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PitchRemap.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

PitchRemap::PitchRemap()
{
   for (size_t i = 0; i < mRemaps.size(); ++i)
   {
      mRemaps[i].mFromPitch = (int)i;
      mRemaps[i].mToPitch = (int)i;
   }
}

void PitchRemap::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   for (size_t i = 0; i < mRemaps.size(); ++i)
   {
      TEXTENTRY_NUM(mRemaps[i].mFromPitchEntry, ("from" + ofToString(i)).c_str(), 4, &mRemaps[i].mFromPitch, 0, 127);
      UIBLOCK_SHIFTX(62);
      TEXTENTRY_NUM(mRemaps[i].mToPitchEntry, ("to" + ofToString(i)).c_str(), 4, &mRemaps[i].mToPitch, 0, 127);
      UIBLOCK_NEWLINE();
   }
   ENDUIBLOCK(mWidth, mHeight);
}

void PitchRemap::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (size_t i = 0; i < mRemaps.size(); ++i)
   {
      mRemaps[i].mFromPitchEntry->Draw();
      ofRectangle rect = mRemaps[i].mFromPitchEntry->GetRect(true);
      ofLine(rect.getMaxX() + 5, rect.getCenter().y, rect.getMaxX() + 20, rect.getCenter().y);
      ofLine(rect.getMaxX() + 15, rect.getCenter().y - 4, rect.getMaxX() + 20, rect.getCenter().y);
      ofLine(rect.getMaxX() + 15, rect.getCenter().y + 4, rect.getMaxX() + 20, rect.getCenter().y);
      mRemaps[i].mToPitchEntry->Draw();
   }
}

void PitchRemap::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(time);
}

void PitchRemap::PlayNote(NoteMessage note)
{
   if (!mEnabled)
   {
      PlayNoteOutput(note);
      return;
   }

   if (note.pitch >= 0 && note.pitch < 128)
   {
      if (note.velocity > 0)
      {
         mInputNotes[note.pitch].mOn = true;
         mInputNotes[note.pitch].mVelocity = note.velocity;
         mInputNotes[note.pitch].mVoiceIdx = note.voiceIdx;
      }
      else
      {
         mInputNotes[note.pitch].mOn = false;
      }
   }

   bool remapped = false;
   for (size_t i = 0; i < mRemaps.size(); ++i)
   {
      if (note.pitch == mRemaps[i].mFromPitch)
      {
         note.pitch = mRemaps[i].mToPitch;
         PlayNoteOutput(note);
         remapped = true;
         break;
      }
   }

   if (!remapped)
      PlayNoteOutput(note);
}

void PitchRemap::TextEntryComplete(TextEntry* entry)
{
   //TODO(Ryan) make this handle mappings changing while notes are input
   mNoteOutput.Flush(NextBufferTime(false));
}

void PitchRemap::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void PitchRemap::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
