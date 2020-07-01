/*
  ==============================================================================

    INoteReceiver.cpp
    Created: 17 May 2020 10:59:27pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "INoteReceiver.h"
#include "Profiler.h"

NoteInputBuffer::NoteInputBuffer(INoteReceiver* receiver)
: mReceiver(receiver)
{
   for (int i=0; i<kBufferSize; ++i)
      mBuffer[i].time = -1;
}

void NoteInputBuffer::Process(double time)
{
   PROFILER(NoteInputBuffer);
   
   //process note offs first
   for (int i=0; i<kBufferSize; ++i)
   {
      if (mBuffer[i].time != -1 && mBuffer[i].velocity == 0 &&
          IsTimeWithinFrame(mBuffer[i].time))
      {
         NoteInputElement& element = mBuffer[i];
         mReceiver->PlayNote(element.time, element.pitch, element.velocity, element.voiceIdx, element.modulation);
         mBuffer[i].time = -1;
      }
   }
   
   //now process note ons
   for (int i=0; i<kBufferSize; ++i)
   {
      if (mBuffer[i].time != -1 && mBuffer[i].velocity != 0 &&
          IsTimeWithinFrame(mBuffer[i].time))
      {
         NoteInputElement& element = mBuffer[i];
         mReceiver->PlayNote(element.time, element.pitch, element.velocity, element.voiceIdx, element.modulation);
         mBuffer[i].time = -1;
      }
   }
}

void NoteInputBuffer::QueueNote(double time, int pitch, float velocity, int voiceIdx, ModulationParameters modulation)
{
   for (int i=0; i<kBufferSize; ++i)
   {
      if (mBuffer[i].time == -1)
      {
         mBuffer[i].time = time;
         NoteInputElement& element = mBuffer[i];
         element.pitch = pitch;
         element.velocity = velocity;
         element.voiceIdx = voiceIdx;
         element.modulation = modulation;
         break;
      }
   }
}

//static
bool NoteInputBuffer::IsTimeWithinFrame(double time)
{
   return time <= gTime + gBufferSize * gInvSampleRateMs;
}
