/*
  ==============================================================================

    IAudioPoller.h
    Created: 12 Apr 2018 10:30:11pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

class IAudioPoller
{
public:
   virtual ~IAudioPoller() {}
   virtual void OnTransportAdvanced(float amount) = 0;
};
