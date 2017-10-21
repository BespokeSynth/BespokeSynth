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
   , mController(nullptr)
{
   for (int i= 0; i<64+8+8; ++i)
      mLights[i] = 0;
}

void LaunchpadInterpreter::SetController(MidiController* controller, int page)
{
   mController = controller;
   mControllerPage = page;
}

void LaunchpadInterpreter::OnMidiNote(MidiNote& note)
{
   int x = note.mPitch % 16;
   int y = 7 - note.mPitch/16;
   if (x == 8)
      x = -1;
   
   if (IsMonome())
   {
      x = note.mPitch % 8;
      y = 7 - note.mPitch / 8;
   }
   
   mListener->OnButtonPress(x,y, note.mVelocity>0);
}

void LaunchpadInterpreter::OnMidiControl(MidiControl& control)
{
   if (control.mControl >= 104 && control.mControl <= 111)
   {
      mListener->OnButtonPress(control.mControl-104, -1, control.mValue>0);
   }
}

bool LaunchpadInterpreter::IsMonome() const
{
   if (mController == nullptr)
      return false;
   return strcmp(mController->Name(),"monome") == 0;
}

void LaunchpadInterpreter::UpdateLights(vector<LightUpdate> lightUpdates, bool force /*=false*/)
{
   if (mController == nullptr)
      return;
   
   for (int i=0; i<lightUpdates.size(); ++i)
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
         lookup = x + y*8;
      
      int val;
      if (IsMonome())
         val = lightUpdates[i].mIntensity*127;
      else
         val = color;
      
      if (mLights[lookup] != val || force)
      {
         if (IsMonome())
         {
            if (x >= 0 && y >= 0)
               mController->SendNote(mControllerPage, x+(7-y)*8, val);
         }
         else
         {
            if (x == -1)
               mController->SendNote(mControllerPage, 8 + (7-y)*16, val);
            else if (y == -1)
               mController->SendData(mControllerPage, 176, x+104, val);
            else
               mController->SendNote(mControllerPage, x + (7-y)*16, val);
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
      mController->SendData(mControllerPage, 176,0,0); //reset code
   
   if (IsMonome())
   {
      for (int i=0; i<64; ++i)
         mController->SendNote(mControllerPage, i, 0);
   }
}

int LaunchpadInterpreter::LaunchpadColor(int r, int g)
{
   return r + (g<<4);
}
