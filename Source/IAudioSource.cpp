//
//  IAudioSource.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/16/15.
//
//

#include "IAudioSource.h"
#include "IAudioReceiver.h"
#include "PatchCableSource.h"

IAudioReceiver* IAudioSource::GetTarget(int index)
{
   assert(index < GetNumTargets());
   return GetPatchCableSource(index)->GetAudioReceiver();
}

void IAudioSource::SyncOutputBuffer(int numChannels)
{
   for (int i=0; i<GetNumTargets(); ++i)
   {
      if (GetTarget(i))
      {
         ChannelBuffer* out = GetTarget(i)->GetBuffer();
         out->SetNumActiveChannels(numChannels);
      }
   }
   GetVizBuffer()->SetNumChannels(numChannels);
}

