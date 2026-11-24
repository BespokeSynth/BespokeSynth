/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

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

    ITouchListener.h
    Created: 23 Nov 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

//receives touchscreen input directly, with each finger tracked separately, rather than through the usual single-pointer mouse handling.
//register with ModularSynth::AddTouchListener().
//a touch that a listener claims belongs to it until it lifts, and doesn't also act as the mouse or navigate the canvas.
//positions are in raw screen space, use ModularSynth::GetMouseX()/GetMouseY() to convert them to a module container's space.
//everything is called from the message thread. mice and trackpads always use the usual mouse handling.
class ITouchListener
{
public:
   virtual ~ITouchListener() {}
   virtual bool WantsTouch(float screenX, float screenY) = 0; //return true to claim a new touch at this position
   virtual void TouchPressed(int touchId, float screenX, float screenY) = 0; //touchId is >= 0, and is unique among the touches that are currently down
   virtual void TouchMoved(int touchId, float screenX, float screenY) = 0;
   virtual void TouchReleased(int touchId) = 0; //also called if the system cancels the touch
};
