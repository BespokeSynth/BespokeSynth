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

    IAudioReceiver.cpp
    Created: 15 Oct 2017 10:23:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IAudioReceiver.h"

void IAudioReceiver::SyncInputBuffer()
{
   if (GetInputMode() == kInputMode_Mono && GetBuffer()->NumActiveChannels() > 1)
   { //sum to mono
      for (int i = 1; i < GetBuffer()->NumActiveChannels(); ++i)
         Add(GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(i), GetBuffer()->BufferSize());
      //Mult(GetBuffer()->GetChannel(0), 1.0f / GetBuffer()->NumActiveChannels(), GetBuffer()->BufferSize());
      GetBuffer()->SetNumActiveChannels(1);
   }
}
