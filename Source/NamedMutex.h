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
//  NamedMutex.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/20/14.
//
//

#pragma once

#include "OpenFrameworksPort.h"

class NamedMutex
{
public:
   void Lock(std::string locker);
   void Unlock();

private:
   ofMutex mMutex;
   std::string mLocker{ "<none>" };
   int mExtraLockCount{ 0 };
};

class ScopedMutex
{
public:
   ScopedMutex(NamedMutex* mutex, std::string locker);
   ~ScopedMutex();

private:
   NamedMutex* mMutex;
};
