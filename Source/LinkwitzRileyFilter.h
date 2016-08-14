//
//  LinkwitzRileyFilter.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/30/14.
//
//

#ifndef __Bespoke__LinkwitzRileyFilter__
#define __Bespoke__LinkwitzRileyFilter__

#include <iostream>

#include "SynthGlobals.h"

class CLinkwitzRiley_4thOrder
{
public:
   CLinkwitzRiley_4thOrder(const double crossoverFreq=1000.0)
   :mCrossoverFreq(crossoverFreq),
   mXm1(0.0), mXm2(0.0), mXm3(0.0), mXm4(0.0),
   mLYm1(0.0), mLYm2(0.0), mLYm3(0.0), mLYm4(0.0),
   mHYm1(0.0), mHYm2(0.0), mHYm3(0.0), mHYm4(0.0)
   {
      CalculateCoefficients();
   }
   
   void SetCrossoverFreq(const double newCrossoverFreq)
   {
      mCrossoverFreq = newCrossoverFreq;
      CalculateCoefficients();
   }
   
   void ProcessSample(const float &sample, float &lowOut, float &highOut)
   {
      static double smp;	// done in case sample is coming in as a reused var in the outs
      smp = sample;
      lowOut = mL_A0 * smp + mL_A1 * mXm1 + mL_A2 * mXm2 + mL_A3 * mXm3 + mL_A4 * mXm4
      - mB1 * mLYm1 - mB2 * mLYm2 - mB3 * mLYm3 - mB4 * mLYm4;
      highOut = mH_A0 * smp + mH_A1 * mXm1 + mH_A2 * mXm2 + mH_A3 * mXm3 + mH_A4 * mXm4
      - mB1 * mHYm1 - mB2 * mHYm2 - mB3 * mHYm3 - mB4 * mHYm4;
      // Shuffle history
      mXm4 = mXm3;	mXm3 = mXm2;	mXm2 = mXm1;	mXm1 = smp;
      mLYm4 = mLYm3;	mLYm3 = mLYm2;	mLYm2 = mLYm1; mLYm1 = lowOut;	// low
      mHYm4 = mHYm3;	mHYm3 = mHYm2;	mHYm2 = mHYm1; mHYm1 = highOut;// high
   }
   
private:
   void CalculateCoefficients()
   {
      // shared for both lp, hp; optimizations here
      double wc = 2.0 * M_PI * mCrossoverFreq;
      double wc2 = wc * wc;
      double wc3 = wc2 * wc;
      double wc4 = wc2 * wc2;
      double k = wc / tan(M_PI * mCrossoverFreq / gSampleRate);
      double k2=k * k;
      double k3=k2 * k;
      double k4=k2 * k2;
      double sqrt2 = sqrt(2.0);
      double sq_tmp1 = sqrt2 * wc3 * k;
      double sq_tmp2 = sqrt2 * wc * k3;
      double a_tmp = 4.0 * wc2 * k2 + 2.0 * sq_tmp1 + k4 + 2.0 * sq_tmp2 + wc4;
      // Shared coeff
      mB1 = (4.0 * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / a_tmp;
      mB2 = (6.0 * wc4 - 8.0 * wc2 * k2 + 6.0 * k4) / a_tmp;
      mB3 = (4.0 * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / a_tmp;
      mB4 = (k4 - 2.0 * sq_tmp1 + wc4 - 2.0 * sq_tmp2 + 4.0 * wc2 * k2) / a_tmp;
      // low-pass coeff
      mL_A0 = wc4 / a_tmp;
      mL_A1 = 4.0 * wc4 / a_tmp;
      mL_A2 = 6.0 * wc4 / a_tmp;
      mL_A3 = mL_A1;
      mL_A4 = mL_A0;
      // high-pass coeff
      mH_A0 = k4 / a_tmp;
      mH_A1 = -4.0 * k4 / a_tmp;
      mH_A2 = 6.0 * k4 / a_tmp;
      mH_A3 = mH_A1;
      mH_A4 = mH_A0;
   }
   double mCrossoverFreq;
   double mB1, mB2, mB3, mB4;	// shared with lp & hp
   double mL_A0, mL_A1, mL_A2, mL_A3, mL_A4;	// lp coefficients
   double mH_A0, mH_A1, mH_A2, mH_A3, mH_A4;	// hp coefficients
   double mXm1, mXm2, mXm3, mXm4;	// incoming sample history
   double mLYm1, mLYm2, mLYm3, mLYm4;	// low output history
   double mHYm1, mHYm2, mHYm3, mHYm4;	// high output history
};

/*As for how to use it, pretty easy:
// I just allocate two of them with the desired crossover points
mLow = new CLinkwitzRiley_4thOrder(44000.0, 400.0);
mHigh = new CLinkwitzRiley_4thOrder(44000.0, 4000.0);
// Then take your sample, pass it into the low, then take the high from that and pass it through the high crossover
double low, mid, high;
mLow->ProcessSample(sampleIn, low, high);
mHigh->ProcessSample(high, mid, high);
When I sum the low mid and high back together, I can’t discern any signal loss once I bypass the splitter…
The only bad thing I’ve noticed is that changing the low freq real time is unstable. There are probably ways around this…
Good luck!*/

#endif /* defined(__Bespoke__LinkwitzRileyFilter__) */
