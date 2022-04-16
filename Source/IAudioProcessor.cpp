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

    IAudioProcessor.cpp
    Created: 15 Oct 2017 10:24:40am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IAudioProcessor.h"

void IAudioProcessor::SyncBuffers(int overrideNumOutputChannels)
{
   SyncInputBuffer();

   int numOutputChannels = GetBuffer()->NumActiveChannels();
   if (overrideNumOutputChannels != -1)
      numOutputChannels = overrideNumOutputChannels;

   SyncOutputBuffer(numOutputChannels);
}
