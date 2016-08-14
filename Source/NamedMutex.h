//
//  NamedMutex.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#ifndef __modularSynth__NamedMutex__
#define __modularSynth__NamedMutex__

#include "OpenFrameworksPort.h"

class NamedMutex
{
public:
   NamedMutex() : mLocker("<none>"), mExtraLockCount(0) {}
   void Lock(string locker);
   void Unlock();
private:
   ofMutex mMutex;
   string mLocker;
   int mExtraLockCount;
};

class ScopedMutex
{
public:
   ScopedMutex(NamedMutex* mutex, string locker);
   ~ScopedMutex();
private:
   NamedMutex* mMutex;
};

#endif /* defined(__modularSynth__NamedMutex__) */
