//
//  IAudioSource.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/16/15.
//
//

#include "IAudioSource.h"
#include "PatchCableSource.h"

IAudioReceiver* IAudioSource::GetTarget(int index)
{
   assert(index < GetNumTargets());
   return GetPatchCableSource(index)->GetAudioReceiver();
}
