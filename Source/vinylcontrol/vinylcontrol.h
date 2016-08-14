// 
// Vinyl control external, using the timecoder from
// Mark Hill's xwax-src <http://www.xwax.co.uk>
//
// Niklas Klügel <kluegel@in.tum.de>

#ifndef VINYL_CONTROL_H
#define VINYL_CONTROL_H

extern "C"
{
   #include "timecoder.h"
};

#define SCALE ((float)(1<<15))
#define SMOOTHING 10

class vinylcontrol {

	public:
   vinylcontrol(int sampleRate);

   void Process(float* left, float* right, int numSamples);

   float GetPitch() { return mPitch; }
   float GetPosition() { return mPosition; }
   bool GetStopped() { return hasBeenAlive == false; }
	
	protected:

                float m_checkSamplerateFactor();
                void m_output_values();
                
                void m_resetFirstTimecode();

	private:	


                float pitch;
                float pitchfactor;

                float pitchAVG;

                unsigned int smoothingIterator;
                unsigned int smoothingIterations;


                int lastTimecode;
                int firstTimecode;
                int currentTimecode;

                int hasBeenAlive;
                struct timecoder_t tc;

                signed short pcm[8192];

   int mSampleRate;

   float mPitch;
   float mPosition;

};



#endif
