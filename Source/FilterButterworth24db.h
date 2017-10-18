//
//  FilterButterworth24db.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/19/16.
//
//

#ifndef __Bespoke__FilterButterworth24db__
#define __Bespoke__FilterButterworth24db__

class CFilterButterworth24db
{
public:
   CFilterButterworth24db(void);
   ~CFilterButterworth24db(void);
   void SetSampleRate(float fs);
   void Set(float cutoff, float q);
   void CopyCoeffFrom(CFilterButterworth24db& other);
   float Run(float input);
   void Clear();
   
private:
   float t0, t1, t2, t3;
   float coef0, coef1, coef2, coef3;
   float history1, history2, history3, history4;
   float gain;
   float min_cutoff, max_cutoff;
};

#endif /* defined(__Bespoke__FilterButterworth24db__) */
