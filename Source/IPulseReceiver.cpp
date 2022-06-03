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

    IPulseReceiver.cpp
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IPulseReceiver.h"
#include "PatchCableSource.h"

void IPulseSource::DispatchPulse(PatchCableSource* destination, double time, float velocity, int flags)
{
   if (time == destination->GetLastOnEventTime()) //avoid stack overflow
      return;

   const std::vector<IPulseReceiver*>& receivers = destination->GetPulseReceivers();
   destination->AddHistoryEvent(time, true, flags);
   destination->AddHistoryEvent(time + 15, false);
   for (auto* receiver : receivers)
      receiver->OnPulse(time, velocity, flags);
}
