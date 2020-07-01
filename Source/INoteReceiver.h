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
   virtual void SendMidi(const MidiMessage& message) { }
};

struct NoteInputElement
{
   double time;
   int pitch;
   float velocity;
   int voiceIdx;
   ModulationParameters modulation;
};

class NoteInputBuffer
{
public:
   NoteInputBuffer(INoteReceiver* receiver);
   void Process(double time);
   void QueueNote(double time, int pitch, float velocity, int voiceIdx, ModulationParameters modulation);
   static bool IsTimeWithinFrame(double time);
private:
   static const int kBufferSize = 50;
   NoteInputElement mBuffer[kBufferSize];
   INoteReceiver* mReceiver;
};

#endif
