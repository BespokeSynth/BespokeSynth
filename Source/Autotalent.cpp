/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  Autotalent.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/27/13.
//
//

#include "Autotalent.h"
#include "SynthGlobals.h"
#include "FFT.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "Profiler.h"

#define L2SC (float)3.32192809488736218171

Autotalent::Autotalent()
: IAudioProcessor(gBufferSize)
{
   mWorkingBuffer = new float[GetBuffer()->BufferSize()];
   Clear(mWorkingBuffer, GetBuffer()->BufferSize());

   mfs = gSampleRate;

   mcbsize = 2048;
   mcorrsize = mcbsize / 2 + 1;

   mpmax = 1 / (float)70; // max and min periods (ms)
   mpmin = 1 / (float)700; // eventually may want to bring these out as sliders

   mnmax = (unsigned long)(gSampleRate * mpmax);
   if (mnmax > mcorrsize)
   {
      mnmax = mcorrsize;
   }
   mnmin = (unsigned long)(gSampleRate * mpmin);

   mcbi = (float*)calloc(mcbsize, sizeof(float));
   mcbf = (float*)calloc(mcbsize, sizeof(float));
   mcbo = (float*)calloc(mcbsize, sizeof(float));

   mcbiwr = 0;
   mcbord = 0;

   mlfophase = 0;

   // Initialize formant corrector
   mford = 7; // should be sufficient to capture formants
   mfalph = pow(0.001f, (float)80 / (gSampleRate));
   mflamb = -(0.8517 * sqrt(atan(0.06583 * gSampleRate)) - 0.1916); // or about -0.88 @ 44.1kHz
   mfk = (float*)calloc(mford, sizeof(float));
   mfb = (float*)calloc(mford, sizeof(float));
   mfc = (float*)calloc(mford, sizeof(float));
   mfrb = (float*)calloc(mford, sizeof(float));
   mfrc = (float*)calloc(mford, sizeof(float));
   mfsig = (float*)calloc(mford, sizeof(float));
   mfsmooth = (float*)calloc(mford, sizeof(float));
   mfhp = 0;
   mflp = 0;
   mflpa = pow(0.001f, (float)10 / (gSampleRate));
   mfbuff = (float**)malloc((mford) * sizeof(float*));
   for (int ti = 0; ti < mford; ti++)
   {
      mfbuff[ti] = (float*)calloc(mcbsize, sizeof(float));
   }
   mftvec = (float*)calloc(mford, sizeof(float));
   mfmute = 1;
   mfmutealph = pow(0.001f, (float)1 / (gSampleRate));

   // Standard raised cosine window, max height at N/2
   mhannwindow = (float*)calloc(mcbsize, sizeof(float));
   for (int ti = 0; ti < mcbsize; ti++)
   {
      mhannwindow[ti] = -0.5 * cos(2 * PI * ti / mcbsize) + 0.5;
   }

   // Generate a window with a single raised cosine from N/4 to 3N/4
   mcbwindow = (float*)calloc(mcbsize, sizeof(float));
   for (int ti = 0; ti < (mcbsize / 2); ti++)
   {
      mcbwindow[ti + mcbsize / 4] = -0.5 * cos(4 * PI * ti / (mcbsize - 1)) + 0.5;
   }

   mnoverlap = 4;

   mFFT = new ::FFT((int)mcbsize);

   mffttime = (float*)calloc(mcbsize, sizeof(float));
   mfftfreqre = (float*)calloc(mcorrsize, sizeof(float));
   mfftfreqim = (float*)calloc(mcorrsize, sizeof(float));


   // ---- Calculate autocorrelation of window ----
   macwinv = (float*)calloc(mcbsize, sizeof(float));
   for (int ti = 0; ti < mcbsize; ti++)
   {
      mffttime[ti] = mcbwindow[ti];
   }
   mFFT->Forward(mcbwindow, mfftfreqre, mfftfreqim);
   for (int ti = 0; ti < mcorrsize; ti++)
   {
      mfftfreqre[ti] = (mfftfreqre[ti]) * (mfftfreqre[ti]) + (mfftfreqim[ti]) * (mfftfreqim[ti]);
      mfftfreqim[ti] = 0;
   }
   mFFT->Inverse(mfftfreqre, mfftfreqim, mffttime);
   for (long ti = 1; ti < mcbsize; ti++)
   {
      macwinv[ti] = mffttime[ti] / mffttime[0];
      if (macwinv[ti] > 0.000001)
      {
         macwinv[ti] = (float)1 / macwinv[ti];
      }
      else
      {
         macwinv[ti] = 0;
      }
   }
   macwinv[0] = 1;
   // ---- END Calculate autocorrelation of window ----


   mlrshift = 0;
   mptarget = 0;
   msptarget = 0;

   mvthresh = 0.7; //  The voiced confidence (unbiased peak) threshold level

   // Pitch shifter initialization
   mphprdd = 0.01; // Default period
   minphinc = (float)1 / (mphprdd * gSampleRate);
   mphincfact = 1;
   mphasein = 0;
   mphaseout = 0;
   mfrag = (float*)calloc(mcbsize, sizeof(float));
   mfragsize = 0;
}

void Autotalent::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mASelector = new RadioButton(this, "A", 4, 40, &mA);
   mBbSelector = new RadioButton(this, "Bb", 24, 40, &mBb);
   mBSelector = new RadioButton(this, "B", 44, 40, &mB);
   mCSelector = new RadioButton(this, "C", 64, 40, &mC);
   mDbSelector = new RadioButton(this, "Db", 84, 40, &mDb);
   mDSelector = new RadioButton(this, "D", 104, 40, &mD);
   mEbSelector = new RadioButton(this, "Eb", 124, 40, &mEb);
   mESelector = new RadioButton(this, "E", 144, 40, &mE);
   mFSelector = new RadioButton(this, "F", 164, 40, &mF);
   mGbSelector = new RadioButton(this, "Gb", 184, 40, &mGb);
   mGSelector = new RadioButton(this, "G", 204, 40, &mG);
   mAbSelector = new RadioButton(this, "Ab", 224, 40, &mAb);
   mAmountSlider = new FloatSlider(this, "amount", 4, 100, 150, 15, &mAmount, 0, 1);
   mSmoothSlider = new FloatSlider(this, "smooth", 4, 120, 150, 15, &mSmooth, 0, .8f);
   mShiftSlider = new IntSlider(this, "shift", 4, 140, 150, 15, &mShift, -6, 6);
   mScwarpSlider = new IntSlider(this, "scwarp", 4, 160, 150, 15, &mScwarp, -3, 3);
   mLfoampSlider = new FloatSlider(this, "lfoamp", 4, 180, 150, 15, &mLfoamp, 0, 1);
   mLforateSlider = new FloatSlider(this, "lforate", 4, 200, 150, 15, &mLforate, 0, 20);
   mLfoshapeSlider = new IntSlider(this, "lfoshape", 4, 220, 150, 15, &mLfoshape, -1, 1);
   mLfosymmSlider = new FloatSlider(this, "lfosymm", 4, 240, 150, 15, &mLfosymm, 0, 1);
   mLfoquantCheckbox = new Checkbox(this, "lfoquant", 4, 260, &mLfoquant);
   mFcorrCheckbox = new Checkbox(this, "formant correct", 4, 280, &mFcorr);
   mFwarpSlider = new FloatSlider(this, "fwarp", 4, 300, 150, 15, &mFwarp, -5, 5);
   mMixSlider = new FloatSlider(this, "mix", 4, 320, 150, 15, &mMix, 0, 1);
   mSetFromScaleButton = new ClickButton(this, "set from scale", 4, 340);

   mASelector->AddLabel("A ", 1);
   mASelector->AddLabel(" ", 0);
   mASelector->AddLabel("-", -1);

   mBbSelector->AddLabel("Bb", 1);
   mBbSelector->AddLabel(" ", 0);
   mBbSelector->AddLabel("-", -1);

   mBSelector->AddLabel("B ", 1);
   mBSelector->AddLabel(" ", 0);
   mBSelector->AddLabel("-", -1);

   mCSelector->AddLabel("C ", 1);
   mCSelector->AddLabel(" ", 0);
   mCSelector->AddLabel("-", -1);

   mDbSelector->AddLabel("Db", 1);
   mDbSelector->AddLabel(" ", 0);
   mDbSelector->AddLabel("-", -1);

   mDSelector->AddLabel("D ", 1);
   mDSelector->AddLabel(" ", 0);
   mDSelector->AddLabel("-", -1);

   mEbSelector->AddLabel("Eb", 1);
   mEbSelector->AddLabel(" ", 0);
   mEbSelector->AddLabel("-", -1);

   mESelector->AddLabel("E ", 1);
   mESelector->AddLabel(" ", 0);
   mESelector->AddLabel("-", -1);

   mFSelector->AddLabel("F ", 1);
   mFSelector->AddLabel(" ", 0);
   mFSelector->AddLabel("-", -1);

   mGbSelector->AddLabel("Gb", 1);
   mGbSelector->AddLabel(" ", 0);
   mGbSelector->AddLabel("-", -1);

   mGSelector->AddLabel("G ", 1);
   mGSelector->AddLabel(" ", 0);
   mGSelector->AddLabel("-", -1);

   mAbSelector->AddLabel("Ab", 1);
   mAbSelector->AddLabel(" ", 0);
   mAbSelector->AddLabel("-", -1);
}

Autotalent::~Autotalent()
{
   delete mFFT;
   free(mcbi);
   free(mcbf);
   free(mcbo);
   free(mcbwindow);
   free(mhannwindow);
   free(macwinv);
   free(mfrag);
   free(mffttime);
   free(mfftfreqre);
   free(mfftfreqim);
   free(mfk);
   free(mfb);
   free(mfc);
   free(mfrb);
   free(mfrc);
   free(mfsmooth);
   free(mfsig);
   for (int ti = 0; ti < mford; ti++)
   {
      free(mfbuff[ti]);
   }
   free(mfbuff);
   free(mftvec);
}


void Autotalent::Process(double time)
{
   PROFILER(Autotalent);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);
   SyncBuffers();

   int iNotes[12];
   int iPitch2Note[12];
   int iNote2Pitch[12];
   int numNotes;
   int iScwarp;

   long int N;
   long int Nf;
   long int fs;

   long int ti;
   long int ti2;
   long int ti3;
   long int ti4;
   float tf;
   float tf2;

   // Variables for cubic spline interpolator
   float indd;
   int ind0;
   int ind1;
   int ind2;
   int ind3;
   float vald;
   float val0;
   float val1;
   float val2;
   float val3;

   int lowersnap;
   int uppersnap;
   float lfoval;

   float pperiod;
   float fa;
   float fb;
   float fc;
   float fk;
   float flamb;
   float frlamb;
   float falph;
   float foma;
   float f1resp;
   float f0resp;
   float flpa;
   int ford;

   float* pfInput = GetBuffer()->GetChannel(0);

   int bufferSize = GetBuffer()->BufferSize();

   Clear(mWorkingBuffer, GetBuffer()->BufferSize());
   float* pfOutput = mWorkingBuffer;

   iNotes[0] = mA;
   iNotes[1] = mBb;
   iNotes[2] = mB;
   iNotes[3] = mC;
   iNotes[4] = mDb;
   iNotes[5] = mD;
   iNotes[6] = mEb;
   iNotes[7] = mE;
   iNotes[8] = mF;
   iNotes[9] = mGb;
   iNotes[10] = mG;
   iNotes[11] = mAb;
   iScwarp = mScwarp;

   // Some logic for the semitone->scale and scale->semitone conversion
   // If no notes are selected as being in the scale, instead snap to all notes
   ti2 = 0;
   for (ti = 0; ti < 12; ti++)
   {
      if (iNotes[ti] >= 0)
      {
         iPitch2Note[ti] = (int)ti2;
         iNote2Pitch[ti2] = (int)ti;
         ti2 = ti2 + 1;
      }
      else
      {
         iPitch2Note[ti] = -1;
      }
   }
   numNotes = (int)ti2;
   while (ti2 < 12)
   {
      iNote2Pitch[ti2] = -1;
      ti2 = ti2 + 1;
   }
   if (numNotes == 0)
   {
      for (ti = 0; ti < 12; ti++)
      {
         iNotes[ti] = 1;
         iPitch2Note[ti] = (int)ti;
         iNote2Pitch[ti] = (int)ti;
      }
      numNotes = 12;
   }
   iScwarp = (iScwarp + numNotes * 5) % numNotes;

   ford = mford;
   falph = mfalph;
   foma = (float)1 - falph;
   flpa = mflpa;
   flamb = mflamb;
   tf = pow((float)2, mFwarp / 2) * (1 + flamb) / (1 - flamb);
   frlamb = (tf - 1) / (tf + 1);

   maref = (float)mTune;

   N = mcbsize;
   Nf = mcorrsize;
   fs = mfs;

   pperiod = mpmax;
   float inpitch = minpitch;
   float conf = mconf;
   float outpitch = moutpitch;


   /*******************
    *  MAIN DSP LOOP  *
    *******************/
   for (int lSampleIndex = 0; lSampleIndex < bufferSize; lSampleIndex++)
   {
      // load data into circular buffer
      tf = (float)*(pfInput++);
      ti4 = mcbiwr;
      mcbi[ti4] = tf;

      if (mFcorr)
      {
         // Somewhat experimental formant corrector
         //  formants are removed using an adaptive pre-filter and
         //  re-introduced after pitch manipulation using post-filter
         // tf is signal input
         fa = tf - mfhp; // highpass pre-emphasis filter
         mfhp = tf;
         fb = fa;
         for (ti = 0; ti < ford; ti++)
         {
            mfsig[ti] = fa * fa * foma + mfsig[ti] * falph;
            fc = (fb - mfc[ti]) * flamb + mfb[ti];
            mfc[ti] = fc;
            mfb[ti] = fb;
            fk = fa * fc * foma + mfk[ti] * falph;
            mfk[ti] = fk;
            tf = fk / (mfsig[ti] + 0.000001);
            tf = tf * foma + mfsmooth[ti] * falph;
            mfsmooth[ti] = tf;
            mfbuff[ti][ti4] = tf;
            fb = fc - tf * fa;
            fa = fa - tf * fc;
         }
         mcbf[ti4] = fa;
         // Now hopefully the formants are reduced
         // More formant correction code at the end of the DSP loop
      }
      else
      {
         mcbf[ti4] = tf;
      }


      // Input write pointer logic
      mcbiwr++;
      if (mcbiwr >= N)
      {
         mcbiwr = 0;
      }


      // ********************
      // * Low-rate section *
      // ********************

      // Every N/noverlap samples, run pitch estimation / manipulation code
      if ((mcbiwr) % (N / mnoverlap) == 0)
      {
         // ---- Obtain autocovariance ----

         // Window and fill FFT buffer
         ti2 = mcbiwr;
         for (ti = 0; ti < N; ti++)
         {
            mffttime[ti] = (float)(mcbi[(ti2 - ti + N) % N] * mcbwindow[ti]);
         }

         // Calculate FFT
         mFFT->Forward(mffttime, mfftfreqre, mfftfreqim);

         // Remove DC
         mfftfreqre[0] = 0;
         mfftfreqim[0] = 0;

         // Take magnitude squared
         for (ti = 1; ti < Nf; ti++)
         {
            mfftfreqre[ti] = (mfftfreqre[ti]) * (mfftfreqre[ti]) + (mfftfreqim[ti]) * (mfftfreqim[ti]);
            mfftfreqim[ti] = 0;
         }

         // Calculate IFFT
         mFFT->Inverse(mfftfreqre, mfftfreqim, mffttime);

         // Normalize
         tf = (float)1 / mffttime[0];
         for (ti = 1; ti < N; ti++)
         {
            mffttime[ti] = mffttime[ti] * tf;
         }
         mffttime[0] = 1;

         //  ---- END Obtain autocovariance ----


         //  ---- Calculate pitch and confidence ----

         // Calculate pitch period
         //   Pitch period is determined by the location of the max (biased)
         //     peak within a given range
         //   Confidence is determined by the corresponding unbiased height
         tf2 = 0;
         pperiod = mpmin;
         for (ti = mnmin; ti < mnmax; ti++)
         {
            ti2 = ti - 1;
            ti3 = ti + 1;
            if (ti2 < 0)
            {
               ti2 = 0;
            }
            if (ti3 > Nf)
            {
               ti3 = Nf;
            }
            tf = mffttime[ti];

            if (tf > mffttime[ti2] && tf >= mffttime[ti3] && tf > tf2)
            {
               tf2 = tf;
               ti4 = ti;
            }
         }
         if (tf2 > 0)
         {
            conf = tf2 * macwinv[ti4];
            if (ti4 > 0 && ti4 < Nf)
            {
               // Find the center of mass in the vicinity of the detected peak
               tf = mffttime[ti4 - 1] * (ti4 - 1);
               tf = tf + mffttime[ti4] * (ti4);
               tf = tf + mffttime[ti4 + 1] * (ti4 + 1);
               tf = tf / (mffttime[ti4 - 1] + mffttime[ti4] + mffttime[ti4 + 1]);
               pperiod = tf / fs;
            }
            else
            {
               pperiod = (float)ti4 / fs;
            }
         }

         // Convert to semitones
         tf = (float)-12 * log10((float)maref * pperiod) * L2SC;
         if (conf >= mvthresh)
         {
            inpitch = tf;
            minpitch = tf; // update pitch only if voiced
         }
         mconf = conf;

         mPitch = inpitch + 69;
         mConfidence = conf;

         //  ---- END Calculate pitch and confidence ----


         //  ---- Modify pitch in all kinds of ways! ----

         outpitch = inpitch;

         // Pull to fixed pitch
         outpitch = (1 - mPull) * outpitch + mPull * mFixed;

         // -- Convert from semitones to scale notes --
         ti = (int)(outpitch / 12 + 32) - 32; // octave
         tf = outpitch - ti * 12; // semitone in octave
         ti2 = (int)tf;
         ti3 = ti2 + 1;
         // a little bit of pitch correction logic, since it's a convenient place for it
         if (iNotes[ti2 % 12] < 0 || iNotes[ti3 % 12] < 0) // if between 2 notes that are more than a semitone apart
         {
            lowersnap = 1;
            uppersnap = 1;
         }
         else
         {
            lowersnap = 0;
            uppersnap = 0;
            if (iNotes[ti2 % 12] == 1) // if specified by user
            {
               lowersnap = 1;
            }
            if (iNotes[ti3 % 12] == 1) // if specified by user
            {
               uppersnap = 1;
            }
         }
         // (back to the semitone->scale conversion)
         // finding next lower pitch in scale
         while (iNotes[(ti2 + 12) % 12] < 0)
         {
            ti2 = ti2 - 1;
         }
         // finding next higher pitch in scale
         while (iNotes[ti3 % 12] < 0)
         {
            ti3 = ti3 + 1;
         }
         tf = (tf - ti2) / (ti3 - ti2) + iPitch2Note[(ti2 + 12) % 12];
         if (ti2 < 0)
         {
            tf = tf - numNotes;
         }
         outpitch = tf + numNotes * ti;
         // -- Done converting to scale notes --

         // The actual pitch correction
         ti = (int)(outpitch + 128) - 128;
         tf = outpitch - ti - 0.5;
         ti2 = ti3 - ti2;
         if (ti2 > 2)
         { // if more than 2 semitones apart, put a 2-semitone-like transition halfway between
            tf2 = (float)ti2 / 2;
         }
         else
         {
            tf2 = (float)1;
         }
         if (mSmooth < 0.001)
         {
            tf2 = tf * tf2 / 0.001;
         }
         else
         {
            tf2 = tf * tf2 / mSmooth;
         }
         if (tf2 < -0.5)
            tf2 = -0.5;
         if (tf2 > 0.5)
            tf2 = 0.5;
         tf2 = 0.5 * sin(PI * tf2) + 0.5; // jumping between notes using horizontally-scaled sine segment
         tf2 = tf2 + ti;
         if ((tf < 0.5 && lowersnap) || (tf >= 0.5 && uppersnap))
         {
            outpitch = mAmount * tf2 + ((float)1 - mAmount) * outpitch;
         }

         // Add in pitch shift
         outpitch = outpitch + mShift;

         // LFO logic
         tf = mLforate * N / (mnoverlap * fs);
         if (tf > 1)
            tf = 1;
         mlfophase = mlfophase + tf;
         if (mlfophase > 1)
            mlfophase = mlfophase - 1;
         lfoval = mlfophase;
         tf = (mLfosymm + 1) / 2;
         if (tf <= 0 || tf >= 1)
         {
            if (tf <= 0)
               lfoval = 1 - lfoval;
         }
         else
         {
            if (lfoval <= tf)
            {
               lfoval = lfoval / tf;
            }
            else
            {
               lfoval = 1 - (lfoval - tf) / (1 - tf);
            }
         }
         if (mLfoshape >= 0)
         {
            // linear combination of cos and line
            lfoval = (0.5 - 0.5 * cos(lfoval * PI)) * mLfoshape + lfoval * (1 - mLfoshape);
            lfoval = mLfoamp * (lfoval * 2 - 1);
         }
         else
         {
            // smoosh the sine horizontally until it's squarish
            tf = 1 + mLfoshape;
            if (tf < 0.001)
            {
               lfoval = (lfoval - 0.5) * 2 / 0.001;
            }
            else
            {
               lfoval = (lfoval - 0.5) * 2 / tf;
            }
            if (lfoval > 1)
               lfoval = 1;
            if (lfoval < -1)
               lfoval = -1;
            lfoval = mLfoamp * sin(lfoval * PI * 0.5);
         }
         // add in quantized LFO
         if (mLfoquant)
         {
            outpitch = outpitch + (int)(numNotes * lfoval + numNotes + 0.5) - numNotes;
         }


         // Convert back from scale notes to semitones
         outpitch = outpitch + iScwarp; // output scale rotate implemented here
         ti = (int)(outpitch / numNotes + 32) - 32;
         tf = outpitch - ti * numNotes;
         ti2 = (int)tf;
         ti3 = ti2 + 1;
         outpitch = iNote2Pitch[ti3 % numNotes] - iNote2Pitch[ti2];
         if (ti3 >= numNotes)
         {
            outpitch = outpitch + 12;
         }
         outpitch = outpitch * (tf - ti2) + iNote2Pitch[ti2];
         outpitch = outpitch + 12 * ti;
         outpitch = outpitch - (iNote2Pitch[iScwarp] - iNote2Pitch[0]); //more scale rotation here

         // add in unquantized LFO
         if (!mLfoquant)
         {
            outpitch = outpitch + lfoval * 2;
         }


         if (outpitch < -36)
            outpitch = -48;
         if (outpitch > 24)
            outpitch = 24;

         moutpitch = outpitch;

         //  ---- END Modify pitch in all kinds of ways! ----

         // Compute variables for pitch shifter that depend on pitch
         minphinc = maref * Pow2(inpitch / 12) / fs;
         moutphinc = maref * Pow2(outpitch / 12) / fs;
         mphincfact = moutphinc / minphinc;
      }
      // ************************
      // * END Low-Rate Section *
      // ************************


      // *****************
      // * Pitch Shifter *
      // *****************

      // Pitch shifter (kind of like a pitch-synchronous version of Fairbanks' technique)
      //   Note: pitch estimate is naturally N/2 samples old
      mphasein = mphasein + minphinc;
      mphaseout = mphaseout + moutphinc;

      //   When input phase resets, take a snippet from N/2 samples in the past
      if (mphasein >= 1)
      {
         mphasein = mphasein - 1;
         ti2 = mcbiwr - N / 2;
         for (ti = -N / 2; ti < N / 2; ti++)
         {
            mfrag[(ti + N) % N] = mcbf[(ti + ti2 + N) % N];
         }
      }

      //   When output phase resets, put a snippet N/2 samples in the future
      if (mphaseout >= 1)
      {
         mfragsize = mfragsize * 2;
         if (mfragsize > N)
         {
            mfragsize = N;
         }
         mphaseout = mphaseout - 1;
         ti2 = mcbord + N / 2;
         ti3 = (long int)(((float)mfragsize) / mphincfact);
         if (ti3 >= N / 2)
         {
            ti3 = N / 2 - 1;
         }
         for (ti = -ti3 / 2; ti < (ti3 / 2); ti++)
         {
            tf = mhannwindow[(long int)N / 2 + ti * (long int)N / ti3];
            // 3rd degree polynomial interpolator - based on eqns from Hal Chamberlin's book
            indd = mphincfact * ti;
            ind1 = (int)indd;
            ind2 = ind1 + 1;
            ind3 = ind1 + 2;
            ind0 = ind1 - 1;
            val0 = mfrag[(ind0 + N) % N];
            val1 = mfrag[(ind1 + N) % N];
            val2 = mfrag[(ind2 + N) % N];
            val3 = mfrag[(ind3 + N) % N];
            vald = 0;
            vald = vald - (float)0.166666666667 * val0 * (indd - ind1) * (indd - ind2) * (indd - ind3);
            vald = vald + (float)0.5 * val1 * (indd - ind0) * (indd - ind2) * (indd - ind3);
            vald = vald - (float)0.5 * val2 * (indd - ind0) * (indd - ind1) * (indd - ind3);
            vald = vald + (float)0.166666666667 * val3 * (indd - ind0) * (indd - ind1) * (indd - ind2);
            mcbo[(ti + ti2 + N) % N] = mcbo[(ti + ti2 + N) % N] + vald * tf;
         }
         mfragsize = 0;
      }
      mfragsize++;

      //   Get output signal from buffer
      tf = mcbo[mcbord]; // read buffer

      mcbo[mcbord] = 0; // erase for next cycle
      mcbord++; // increment read pointer
      if (mcbord >= N)
      {
         mcbord = 0;
      }

      // *********************
      // * END Pitch Shifter *
      // *********************

      ti4 = (mcbiwr + 2) % N;
      if (mFcorr)
      {
         // The second part of the formant corrector
         // This is a post-filter that re-applies the formants, designed
         //   to result in the exact original signal when no pitch
         //   manipulation is performed.
         // tf is signal input
         // gotta run it 3 times because of a pesky delay free loop
         //  first time: compute 0-response
         tf2 = tf;
         fa = 0;
         fb = fa;
         for (ti = 0; ti < ford; ti++)
         {
            fc = (fb - mfrc[ti]) * frlamb + mfrb[ti];
            tf = mfbuff[ti][ti4];
            fb = fc - tf * fa;
            mftvec[ti] = tf * fc;
            fa = fa - mftvec[ti];
         }
         tf = -fa;
         for (ti = ford - 1; ti >= 0; ti--)
         {
            tf = tf + mftvec[ti];
         }
         f0resp = tf;
         //  second time: compute 1-response
         fa = 1;
         fb = fa;
         for (ti = 0; ti < ford; ti++)
         {
            fc = (fb - mfrc[ti]) * frlamb + mfrb[ti];
            tf = mfbuff[ti][ti4];
            fb = fc - tf * fa;
            mftvec[ti] = tf * fc;
            fa = fa - mftvec[ti];
         }
         tf = -fa;
         for (ti = ford - 1; ti >= 0; ti--)
         {
            tf = tf + mftvec[ti];
         }
         f1resp = tf;
         //  now solve equations for output, based on 0-response and 1-response
         tf = (float)2 * tf2;
         tf2 = tf;
         tf = ((float)1 - f1resp + f0resp);
         if (tf != 0)
         {
            tf2 = (tf2 + f0resp) / tf;
         }
         else
         {
            tf2 = 0;
         }
         //  third time: update delay registers
         fa = tf2;
         fb = fa;
         for (ti = 0; ti < ford; ti++)
         {
            fc = (fb - mfrc[ti]) * frlamb + mfrb[ti];
            mfrc[ti] = fc;
            mfrb[ti] = fb;
            tf = mfbuff[ti][ti4];
            fb = fc - tf * fa;
            fa = fa - tf * fc;
         }
         tf = tf2;
         tf = tf + flpa * mflp; // lowpass post-emphasis filter
         mflp = tf;
         // Bring up the gain slowly when formant correction goes from disabled
         // to enabled, while things stabilize.
         if (mfmute > 0.5)
         {
            tf = tf * (mfmute - 0.5) * 2;
         }
         else
         {
            tf = 0;
         }
         tf2 = mfmutealph;
         mfmute = (1 - tf2) + tf2 * mfmute;
         // now tf is signal output
         // ...and we're done messing with formants
      }
      else
      {
         mfmute = 0;
      }

      // Write audio to output of plugin
      // Mix (blend between original (delayed) =0 and processed =1)
      *(pfOutput++) = mMix * tf + (1 - mMix) * mcbi[ti4];
   }

   Add(target->GetBuffer()->GetChannel(0), mWorkingBuffer, bufferSize);

   GetVizBuffer()->WriteChunk(mWorkingBuffer, bufferSize, 0);

   GetBuffer()->Reset();

   // Tell the host the algorithm latency
   mLatency = (N - 1);
}

void Autotalent::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mASelector->Draw();
   mBbSelector->Draw();
   mBSelector->Draw();
   mCSelector->Draw();
   mDbSelector->Draw();
   mDSelector->Draw();
   mEbSelector->Draw();
   mESelector->Draw();
   mFSelector->Draw();
   mGbSelector->Draw();
   mGSelector->Draw();
   mAbSelector->Draw();
   mAmountSlider->Draw();
   mSmoothSlider->Draw();
   mShiftSlider->Draw();
   mScwarpSlider->Draw();
   mLfoampSlider->Draw();
   mLforateSlider->Draw();
   mLfoshapeSlider->Draw();
   mLfosymmSlider->Draw();
   mLfoquantCheckbox->Draw();
   mFcorrCheckbox->Draw();
   mFwarpSlider->Draw();
   mMixSlider->Draw();
   mSetFromScaleButton->Draw();

   float pitch = mPitch;
   while (pitch > 12)
      pitch -= 12;
   while (pitch < 0)
      pitch += 12;
   float x = ofMap(pitch, 0, 12, 4, 244);
   ofSetColor(255, 0, 255);
   ofLine(x, 90, x, 90 - ofMap(mConfidence, 0, 1, 0, 50));
}

void Autotalent::ButtonClicked(ClickButton* button, double time)
{
   if (button == mSetFromScaleButton)
   {
      mA = TheScale->MakeDiatonic(69) == 69 ? 1 : -1;
      mBb = TheScale->MakeDiatonic(70) == 70 ? 1 : -1;
      mB = TheScale->MakeDiatonic(71) == 71 ? 1 : -1;
      mC = TheScale->MakeDiatonic(72) == 72 ? 1 : -1;
      mDb = TheScale->MakeDiatonic(73) == 73 ? 1 : -1;
      mD = TheScale->MakeDiatonic(74) == 74 ? 1 : -1;
      mEb = TheScale->MakeDiatonic(75) == 75 ? 1 : -1;
      mE = TheScale->MakeDiatonic(76) == 76 ? 1 : -1;
      mF = TheScale->MakeDiatonic(77) == 77 ? 1 : -1;
      mGb = TheScale->MakeDiatonic(78) == 78 ? 1 : -1;
      mG = TheScale->MakeDiatonic(79) == 79 ? 1 : -1;
      mAb = TheScale->MakeDiatonic(80) == 80 ? 1 : -1;
      UpdateShiftSlider();
   }
}

void Autotalent::PlayNote(NoteMessage note)
{
   if (note.velocity > 0)
   {
      mC = (note.pitch % 12) == 0 ? 1 : -1;
      mDb = (note.pitch % 12) == 1 ? 1 : -1;
      mD = (note.pitch % 12) == 2 ? 1 : -1;
      mEb = (note.pitch % 12) == 3 ? 1 : -1;
      mE = (note.pitch % 12) == 4 ? 1 : -1;
      mF = (note.pitch % 12) == 5 ? 1 : -1;
      mGb = (note.pitch % 12) == 6 ? 1 : -1;
      mG = (note.pitch % 12) == 7 ? 1 : -1;
      mAb = (note.pitch % 12) == 8 ? 1 : -1;
      mA = (note.pitch % 12) == 9 ? 1 : -1;
      mBb = (note.pitch % 12) == 10 ? 1 : -1;
      mB = (note.pitch % 12) == 11 ? 1 : -1;
      UpdateShiftSlider();
   }
}

void Autotalent::UpdateShiftSlider()
{
   int numTones = 1;
   if (mA != -1)
      ++numTones;
   if (mBb != -1)
      ++numTones;
   if (mB != -1)
      ++numTones;
   if (mC != -1)
      ++numTones;
   if (mDb != -1)
      ++numTones;
   if (mD != -1)
      ++numTones;
   if (mEb != -1)
      ++numTones;
   if (mE != -1)
      ++numTones;
   if (mF != -1)
      ++numTones;
   if (mGb != -1)
      ++numTones;
   if (mG != -1)
      ++numTones;
   if (mAb != -1)
      ++numTones;

   mShiftSlider->SetExtents(-numTones, numTones);
}

void Autotalent::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void Autotalent::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}
