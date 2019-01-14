//
//  IAudioReceiver.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/11/12.
//
//

#ifndef modularSynth_IAudioReceiver_h
#define modularSynth_IAudioReceiver_h

#include "ChannelBuffer.h"

class IAudioReceiver
{
public:
   enum InputMode
   {
      kInputMode_Mono,
      kInputMode_Multichannel
   };
   
   IAudioReceiver(int bufferSize) : mInputBuffer(bufferSize) {}
   virtual ~IAudioReceiver() {}
   virtual ChannelBuffer* GetBuffer() { return &mInputBuffer; }
   virtual InputMode GetInputMode() { return kInputMode_Multichannel; }
protected:
   void SyncInputBuffer();
private:
   ChannelBuffer mInputBuffer;
};

#endif
