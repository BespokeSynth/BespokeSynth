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
//  LaunchpadInterpreter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#include "LaunchpadInterpreter.h"
#include "MidiController.h"

LaunchpadInterpreter::LaunchpadInterpreter(ILaunchpadListener* listener)
: mListener(listener)
{
}

void LaunchpadInterpreter::SetController(MidiController* controller, int page)
{
   mController = controller;
   mControllerPage = page;
}

void LaunchpadInterpreter::OnMidiNote(MidiNote& note)
{
   int x = note.mPitch % 16;
   int y = 7 - note.mPitch / 16;
   if (x == 8)
      x = -1;

   if (IsMonome())
   {
      x = note.mPitch % 8;
      y = 7 - note.mPitch / 8;
   }

   mListener->OnButtonPress(x, y, note.mVelocity > 0);
}

void LaunchpadInterpreter::OnMidiControl(MidiControl& control)
{
   if (control.mControl >= 104 && control.mControl <= 111)
   {
      mListener->OnButtonPress(control.mControl - 104, -1, control.mValue > 0);
   }
}

bool LaunchpadInterpreter::IsMonome() const
{
   if (mController == nullptr)
      return false;
   return strcmp(mController->Name(), "monome") == 0;
}

void LaunchpadInterpreter::UpdateLights(std::vector<LightUpdate> lightUpdates, bool force /*=false*/)
{
   if (mController == nullptr)
      return;

   for (int i = 0; i < lightUpdates.size(); ++i)
   {
      int x = lightUpdates[i].mX;
      int y = lightUpdates[i].mY;
      int color = LaunchpadColor(lightUpdates[i].mR, lightUpdates[i].mG);
      int lookup;
      if (x == -1 && y == -1)
         continue;
      else if (x >= 8 || y >= 8)
         continue;
      else if (x == -1)
         lookup = 64 + y;
      else if (y == -1)
         lookup = 64 + 8 + x;
      else
         lookup = x + y * 8;

      int val;
      if (IsMonome())
         val = lightUpdates[i].mIntensity * 127;
      else
         val = color;

      if (mLights[lookup] != val || force)
      {
         if (IsMonome())
         {
            if (x >= 0 && y >= 0)
               mController->SendNote(mControllerPage, x + (7 - y) * 8, val);
         }
         else
         {
            if (x == -1)
               mController->SendNote(mControllerPage, 8 + (7 - y) * 16, val);
            else if (y == -1)
               mController->SendData(mControllerPage, 176, x + 104, val);
            else
               mController->SendNote(mControllerPage, x + (7 - y) * 16, val);
         }
         mLights[lookup] = val;
      }
   }
}

void LaunchpadInterpreter::Draw(ofVec2f vPos)
{
}

void LaunchpadInterpreter::ResetLaunchpad()
{
   if (mController)
      mController->SendData(mControllerPage, 176, 0, 0); //reset code

   if (IsMonome())
   {
      for (int i = 0; i < 64; ++i)
         mController->SendNote(mControllerPage, i, 0);
   }
}

int LaunchpadInterpreter::LaunchpadColor(int r, int g)
{
   return r + (g << 4);
}
