/*
  ==============================================================================

    IPulseReceiver.cpp
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IPulseReceiver.h"
#include "PatchCableSource.h"

void IPulseSource::DispatchPulse(PatchCableSource* destination, float velocity, int samplesTo, int flags)
{
   const vector<IPulseReceiver*>& receivers = destination->GetPulseReceivers();
   destination->AddHistoryEvent(gTime, true);
   destination->AddHistoryEvent(gTime + 15, false);
   for (auto* receiver : receivers)
      receiver->OnPulse(velocity, samplesTo, flags);
}
