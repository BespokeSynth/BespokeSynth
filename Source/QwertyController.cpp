/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2024 Ryan Challinor (contact: awwbees@gmail.com)

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
//  QwertyController.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/15/24.
//
//

#include "QwertyController.h"
#include "SynthGlobals.h"

QwertyController::QwertyController(MidiDeviceListener* listener)
: mListener(listener)
{
}

QwertyController::~QwertyController()
{
}

std::string QwertyController::GetControlTooltip(MidiMessageType type, int control)
{
   if (type == kMidiMessage_Note)
      return std::string(1, char(control));
   return "[unmapped]";
}

void QwertyController::OnKeyPressed(int key)
{
   if (key >= 0 && key < (int)mPressedKeys.size())
   {
      mPressedKeys[key] = true;

      MidiNote note;
      note.mPitch = key;
      note.mVelocity = 127.0f;
      note.mChannel = 0;
      note.mDeviceName = "keyboard";
      mListener->OnMidiNote(note);
   }
}

void QwertyController::OnKeyReleased(int key)
{
   if (key >= 0 && key < (int)mPressedKeys.size() && mPressedKeys[key])
   {
      mPressedKeys[key] = false;

      MidiNote note;
      note.mPitch = key;
      note.mVelocity = 0.0f;
      note.mChannel = 0;
      note.mDeviceName = "keyboard";
      mListener->OnMidiNote(note);
   }
}

namespace
{
   const int kSaveStateRev = 1;
}

void QwertyController::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;
}

void QwertyController::LoadState(FileStreamIn& in)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
}
