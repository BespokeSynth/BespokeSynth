/*
  ==============================================================================

    IPulseReceiver.h
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

class PatchCableSource;

enum PulseFlags
{
   kPulseFlag_None = 0x0,
   kPulseFlag_Reset = 0x1,
   kPulseFlag_Random = 0x2,
   kPulseFlag_SyncToTransport = 0x4,
   kPulseFlag_Backward = 0x8
};

class IPulseReceiver
{
public:
   virtual ~IPulseReceiver() {}
   virtual void OnPulse(double time, float velocity, int flags) = 0;
};

class IPulseSource
{
public:
   virtual ~IPulseSource() {}
   void DispatchPulse(PatchCableSource* destination, double time, float velocity, int flags);
};
