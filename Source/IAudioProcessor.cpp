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
