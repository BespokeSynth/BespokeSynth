//
//  IAudioReceiver.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/11/12.
//
//

#ifndef modularSynth_IAudioReceiver_h
#define modularSynth_IAudioReceiver_h

class IAudioReceiver
{
public:
   virtual ~IAudioReceiver() {}
   virtual float* GetBuffer(int& bufferSize) = 0;
};

#endif
