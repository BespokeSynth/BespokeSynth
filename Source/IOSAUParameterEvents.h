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
//
//  IOSAUParameterEvents.h
//  Bespoke
//
//  Hears an AudioUnit's parameter gesture events on iOS.
//  JUCE only listens for these on macOS (its AUEventListener code is all inside #if JUCE_MAC), so on iOS a hosted
//  plugin's gestures never reach AudioProcessorListener::audioProcessorParameterChangeGestureBegin().
//

#pragma once

#include <functional>
#include <vector>

namespace IOSAUParameterEvents
{
   struct Listener;

   enum class EventType
   {
      BeginGesture,
      EndGesture
   };

   //called on the main thread. paramID is the AU's own parameter ID (what juce's getParameterID() holds, as a string)
   using Handler = std::function<void(unsigned int paramID, EventType type)>;

   //listens to the given parameters of an AudioUnit (an AudioUnit handle, from AudioPluginInstance::getPlatformSpecificData())
   Listener* Create(void* audioUnit, const std::vector<unsigned int>& paramIDs, Handler handler);

   //stops listening. call on the main thread, before the AudioUnit is disposed. the handler isn't called after this
   void Destroy(Listener* listener);
}
