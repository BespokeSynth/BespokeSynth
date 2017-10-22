/*
  ==============================================================================

    IAudioReceiver.cpp
    Created: 15 Oct 2017 10:23:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "IAudioReceiver.h"

void IAudioReceiver::SyncInputBuffer()
{
   if (GetInputMode() == kInputMode_Mono && GetBuffer()->NumActiveChannels() > 1)
   {  //sum to mono
      for (int i=1; i<GetBuffer()->NumActiveChannels(); ++i)
         Add(GetBuffer()->GetChannel(0), GetBuffer()->GetChannel(i), GetBuffer()->BufferSize());
      //Mult(GetBuffer()->GetChannel(0), 1.0f / GetBuffer()->NumActiveChannels(), GetBuffer()->BufferSize());
      GetBuffer()->SetNumActiveChannels(1);
   }
}
