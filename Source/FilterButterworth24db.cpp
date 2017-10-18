//
//  FilterButterworth24db.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#include "FilterButterworth24db.h"
#include "SynthGlobals.h"
#include <math.h>

#define BUDDA_Q_SCALE 6.f

CFilterButterworth24db::CFilterButterworth24db(void)
{
   SetSampleRate(gSampleRate);
   Set(gSampleRate/2, 0.0);
   Clear();
}

CFilterButterworth24db::~CFilterButterworth24db(void)
{
}

void CFilterButterworth24db::Clear()
{
   history1 = 0.f;
   history2 = 0.f;
   history3 = 0.f;
   history4 = 0.f;
}

void CFilterButterworth24db::SetSampleRate(float fs)
{
   float pi = 4.f * atanf(1.f);
   
   t0 = 4.f * fs * fs;
   t1 = 8.f * fs * fs;
   t2 = 2.f * fs;
   t3 = pi / fs;
   
   min_cutoff = fs * 0.01f;
   max_cutoff = fs * 0.45f;
}

void CFilterButterworth24db::Set(float cutoff, float q)
{
   if (cutoff < min_cutoff)
      cutoff = min_cutoff;
   else if(cutoff > max_cutoff)
      cutoff = max_cutoff;
   
   if(q < 0.f)
      q = 0.f;
   else if(q > 1.f)
      q = 1.f;
   
   float wp = t2 * tanf(t3 * cutoff);
   float bd, bd_tmp, b1, b2;
   
   q *= BUDDA_Q_SCALE;
   q += 1.f;
   
   b1 = (0.765367f / q) / wp;
   b2 = 1.f / (wp * wp);
   
   bd_tmp = t0 * b2 + 1.f;
   
   bd = 1.f / (bd_tmp + t2 * b1);
   
   gain = bd * 0.5f;
   
   coef2 = (2.f - t1 * b2);
   
   coef0 = coef2 * bd;
   coef1 = (bd_tmp - t2 * b1) * bd;
   
   b1 = (1.847759f / q) / wp;
   
   bd = 1.f / (bd_tmp + t2 * b1);
   
   gain *= bd;
   coef2 *= bd;
   coef3 = (bd_tmp - t2 * b1) * bd;
}

float CFilterButterworth24db::Run(float input)
{
   float output = input * gain;
   float new_hist;
   
   output -= history1 * coef0;
   new_hist = output - history2 * coef1;
   
   output = new_hist + history1 * 2.f;
   output += history2;
   
   history2 = history1;
   history1 = new_hist;
   
   output -= history3 * coef2;
   new_hist = output - history4 * coef3;
   
   output = new_hist + history3 * 2.f;
   output += history4;
   
   history4 = history3;
   history3 = new_hist;
   
   return output;
}

void CFilterButterworth24db::CopyCoeffFrom(CFilterButterworth24db& other)
{
   coef0 = other.coef0;
   coef1 = other.coef1;
   coef2 = other.coef2;
   coef3 = other.coef3;
   gain = other.gain;
}

