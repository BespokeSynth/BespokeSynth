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
//  LaunchpadInterpreter.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#pragma once

#include "OpenFrameworksPort.h"

class MidiController;
struct MidiNote;
struct MidiControl;

struct LightUpdate
{
   LightUpdate(int x, int y, char r, char g, float intensity = 1)
   : mX(x)
   , mY(y)
   , mR(r)
   , mG(g)
   , mIntensity(intensity)
   {}
   int mX{ 0 };
   int mY{ 0 };
   char mR{ 0 };
   char mG{ 0 };
   float mIntensity{ 1 };
};

class ILaunchpadListener
{
public:
   virtual ~ILaunchpadListener() {}
   virtual void OnButtonPress(int x, int y, bool bOn) = 0;
};

class LaunchpadInterpreter
{
public:
   LaunchpadInterpreter(ILaunchpadListener* listener);
   void SetController(MidiController* controller, int controllerPage);
   void OnMidiNote(MidiNote& note);
   void OnMidiControl(MidiControl& control);
   void UpdateLights(std::vector<LightUpdate> lightUpdates, bool force = false);
   void Draw(ofVec2f vPos);
   void ResetLaunchpad();
   bool HasLaunchpad() { return mController != nullptr; }

   static int LaunchpadColor(int r, int g);

private:
   bool IsMonome() const;

   ILaunchpadListener* mListener{ nullptr };
   int mLights[64 + 8 + 8]{}; //grid + side + top
   MidiController* mController{ nullptr };
   int mControllerPage{ 0 };
};
