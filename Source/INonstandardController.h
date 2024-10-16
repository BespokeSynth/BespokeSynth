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
//  INonstandardController.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/14.
//
//

#pragma once

#include "MidiController.h"

class ofxJSONElement;

class INonstandardController
{
public:
   virtual ~INonstandardController() {}
   virtual void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) = 0;
   virtual bool IsInputConnected() { return true; }
   virtual bool Reconnect() { return true; }
   virtual void Poll() {}
   virtual std::string GetControlTooltip(MidiMessageType type, int control) { return MidiController::GetDefaultTooltip(type, control); }
   virtual void SetLayoutData(ofxJSONElement& layout) {}
   virtual void SaveState(FileStreamOut& out) = 0;
   virtual void LoadState(FileStreamIn& in) = 0;
};
