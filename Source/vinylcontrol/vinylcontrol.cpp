//
//  vinylcontrol.h
//  modularSynth
//
//  Created by Ryan Challinor on 4/7/13.
//
//

#include "vinylcontrol.h"

vinylcontrol::vinylcontrol(int sampleRate)
:pitchfactor(1.0), pitchAVG(0.0), smoothingIterator(0), hasBeenAlive(0), mSampleRate(sampleRate) {

   const char* vinylType;

   vinylType = "serato_2a";
   smoothingIterations = SMOOTHING;

   timecoder_build_lookup((char*)vinylType);

   timecoder_init(&tc);

   lastTimecode = (int)timecoder_get_safe(&tc);
   firstTimecode = 0;
   currentTimecode = 0;
}

inline float vinylcontrol::m_checkSamplerateFactor() {
   return mSampleRate / float(DEVICE_RATE);
}


void vinylcontrol::m_resetFirstTimecode() {
   firstTimecode = 0;
}

void vinylcontrol::m_output_values() {
   if(timecoder_get_alive(&tc)) {
      if(hasBeenAlive == 0 && (smoothingIterator%smoothingIterations == 0)) {
         hasBeenAlive = 1;
      }
      timecoder_get_pitch(&tc, &pitch);
      //   ToOutFloat(1, pitch*m_checkSamplerateFactor());

      if(smoothingIterator % smoothingIterations != 0) {
         pitchAVG += pitch;
      } else {
         pitch = pitchAVG/smoothingIterations;
         mPitch = pitch*m_checkSamplerateFactor();
         pitchAVG = pitch;
      }

      if((currentTimecode = timecoder_get_position(&tc, 0)) != -1) {
         if(firstTimecode == 0) {
            firstTimecode = currentTimecode;
         }
         mPosition =  (((float)(currentTimecode-firstTimecode))/float(lastTimecode-firstTimecode));
      }

   } else if (hasBeenAlive == 1 && (smoothingIterator%smoothingIterations == 0)) {
      hasBeenAlive = 0;
   }

   smoothingIterator++;
}

void vinylcontrol::Process(float* left, float* right, int numSamples)
{
   float* in[2];
   in[0] = left;
   in[1] = right;

   int i;
   int n;
   // convert to pcm
   for(n = 0; n < numSamples; n++)
   {
      for(i = 0; i < 2; i++)
      {
         pcm[n * DEVICE_CHANNELS + i] = (signed short)(SCALE*(float)in[i][n]);
      }
   }

	timecoder_submit(&tc,pcm,n);

   m_output_values();

}  // end m_signal
