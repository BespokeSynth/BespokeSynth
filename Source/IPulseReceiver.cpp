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
   const vector<IPulseReceiver*>& receivers = destination->GetPulseReceivers();
   destination->AddHistoryEvent(time, true);
   destination->AddHistoryEvent(time + 15, false);
   for (auto* receiver : receivers)
      receiver->OnPulse(time, velocity, flags);
}
