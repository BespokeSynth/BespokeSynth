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

#pragma once

#include "MidiDevice.h"
#include "INonstandardController.h"

class QwertyController : public INonstandardController
{
public:
   QwertyController(MidiDeviceListener* listener);
   ~QwertyController();

   void OnKeyPressed(int key);
   void OnKeyReleased(int key);

   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) override {}
   bool IsInputConnected() override { return true; }
   bool Reconnect() override { return true; }
   std::string GetControlTooltip(MidiMessageType type, int control) override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;

private:
   MidiDeviceListener* mListener{ nullptr };
   std::array<bool, 128> mPressedKeys{};
};
