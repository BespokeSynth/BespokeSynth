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
/*
  ==============================================================================

    IPulseReceiver.h
    Created: 20 Sep 2018 9:24:33pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

class PatchCableSource;

enum PulseFlags
{
   kPulseFlag_None = 0x0,
   kPulseFlag_Reset = 0x1,
   kPulseFlag_Random = 0x2,
   kPulseFlag_SyncToTransport = 0x4,
   kPulseFlag_Backward = 0x8,
   kPulseFlag_Align = 0x10,
   kPulseFlag_Repeat = 0x20
};

class IPulseReceiver
{
public:
   virtual ~IPulseReceiver() {}
   virtual void OnPulse(double time, float velocity, int flags) = 0;
};

class IPulseSource
{
public:
   IPulseSource() {}
   virtual ~IPulseSource() {}
   void DispatchPulse(PatchCableSource* destination, double time, float velocity, int flags);
};
