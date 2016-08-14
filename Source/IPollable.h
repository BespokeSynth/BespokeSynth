//
//  IPollable.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/2/13.
//
//

#ifndef modularSynth_IPollable_h
#define modularSynth_IPollable_h

class IPollable
{
public:
   virtual ~IPollable() {}
   virtual void Poll() {}
};

#endif
