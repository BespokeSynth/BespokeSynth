//
//  NamedMutex.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#include "NamedMutex.h"

void NamedMutex::Lock(string locker)
{
   if (mLocker == locker)
   {
      ++mExtraLockCount;
      return;
   }
   mMutex.lock();
   mLocker = locker;
}

void NamedMutex::Unlock()
{
   if (mExtraLockCount == 0)
   {
      mLocker = "<none>";
      mMutex.unlock();
   }
   else
   {
      --mExtraLockCount;
   }
}

ScopedMutex::ScopedMutex(NamedMutex* mutex, string locker)
: mMutex(mutex)
{
   mMutex->Lock(locker);
}

ScopedMutex::~ScopedMutex()
{
   mMutex->Unlock();
}