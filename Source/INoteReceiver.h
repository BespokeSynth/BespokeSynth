//
//  INoteReceiver.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef modularSynth_INoteReceiver_h
#define modularSynth_INoteReceiver_h

#include "OpenFrameworksPort.h"
#include "ModulationChain.h"

class INoteReceiver
{
public:
   virtual ~INoteReceiver() {}
   virtual void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) = 0;
   virtual void SendPressure(int pitch, int pressure) {}
   virtual void SendCC(int control, int value, int voiceIdx = -1) = 0;
};

#endif
