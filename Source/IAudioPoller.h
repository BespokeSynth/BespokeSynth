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

    IAudioPoller.h
    Created: 12 Apr 2018 10:30:11pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

class IAudioPoller
{
public:
   virtual ~IAudioPoller() {}
   virtual void OnTransportAdvanced(float amount) = 0;
   
   float getPollPriority() { return mPollPriority; };
   void setPollPriority(float inPri) { mPollPriority = CLAMP(inPri, 0, 1); }
   
   bool operator <(const IAudioPoller & right) const { return mPollPriority < right.mPollPriority; }
   
private:
   float mPollPriority = 0.5; //on a scale from 0-1, with a default in the middle
};
