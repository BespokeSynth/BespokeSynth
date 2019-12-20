//
//  NoteEffectBase.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#ifndef Bespoke_NoteEffectBase_h
#define Bespoke_NoteEffectBase_h

#include "INoteReceiver.h"
#include "INoteSource.h"

class NoteEffectBase : public INoteReceiver, public INoteSource
{
public:
   void PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation) override
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   }
   void SendCC(int control, int value, int voiceIdx = -1) override
   {
      SendCCOutput(control, value, voiceIdx);
   }
};

#endif
