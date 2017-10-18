/*
  ==============================================================================

    IAudioProcessor.h
    Created: 15 Oct 2017 10:24:40am
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IAudioReceiver.h"
#include "IAudioSource.h"

class IAudioProcessor : public IAudioReceiver, public IAudioSource
{
public:
   IAudioProcessor(int bufferSize) : IAudioReceiver(bufferSize) {}
protected:
   void SyncBuffers(int overrideNumOutputChannels = -1);
};
