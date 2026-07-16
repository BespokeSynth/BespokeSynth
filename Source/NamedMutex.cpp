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
//  NamedMutex.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#include "NamedMutex.h"

void NamedMutex::Lock(std::string locker)
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

ScopedMutex::ScopedMutex(NamedMutex* mutex, std::string locker)
: mMutex(mutex)
{
   mMutex->Lock(locker);
}

ScopedMutex::~ScopedMutex()
{
   mMutex->Unlock();
}