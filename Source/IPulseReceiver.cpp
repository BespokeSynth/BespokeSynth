/*
  ==============================================================================

    IPulseReceiver.cpp
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IPulseReceiver.h"

void IPulseSource::DispatchPulse(const vector<IPulseReceiver*>& receivers, int samplesTo, int flags)
{
   mHistory.AddEvent(gTime, true);
   mHistory.AddEvent(gTime+15, false);
   for (auto* receiver : receivers)
      receiver->OnPulse(samplesTo, flags);
}
