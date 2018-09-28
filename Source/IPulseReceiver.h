/*
  ==============================================================================

    IPulseReceiver.h
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "INoteSource.h"

enum PulseFlags
{
   kPulseFlag_None = 0x0,
   kPulseFlag_Reset = 0x1
};

class IPulseReceiver
{
public:
   virtual ~IPulseReceiver() {}
   virtual void OnPulse(int samplesTo, int flags) = 0;
};

class IPulseSource
{
public:
   virtual ~IPulseSource() {}
   void DispatchPulse(const vector<IPulseReceiver*>& receivers, int samplesTo, int flags);
   
   NoteHistory& GetPulseHistory() { return mHistory; }
private:
   NoteHistory mHistory;
};
