//
//  IAudioSource.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/22/12.
//
//

#ifndef modularSynth_IAudioSource_h
#define modularSynth_IAudioSource_h

#include "RollingBuffer.h"
#include "SynthGlobals.h"
#include "IPatchable.h"

class IAudioReceiver;

#define VIZ_BUFFER_SECONDS .1f

class IAudioSource : public virtual IPatchable
{
public:
   IAudioSource() : mVizBuffer(VIZ_BUFFER_SECONDS*gSampleRate) {}
   virtual ~IAudioSource() {}
   virtual void Process(double time) = 0;
   //virtual void SetTarget(IAudioReceiver* target) { mTarget = target; }
   IAudioReceiver* GetTarget();
   virtual IAudioReceiver* GetTarget2() { return NULL; }
   RollingBuffer* GetVizBuffer() { return &mVizBuffer; }
private:
   RollingBuffer mVizBuffer;
};

#endif
