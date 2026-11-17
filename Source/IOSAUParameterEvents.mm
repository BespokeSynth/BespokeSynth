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
//  IOSAUParameterEvents.mm
//  Bespoke
//

#include "IOSAUParameterEvents.h"

#import <AudioToolbox/AudioToolbox.h>
#import <dispatch/dispatch.h>

namespace IOSAUParameterEvents
{
   struct Listener
   {
      AUEventListenerRef mRef{ nullptr };
      Handler mHandler;
      bool mAlive{ true };
   };

   Listener* Create(void* audioUnit, const std::vector<unsigned int>& paramIDs, Handler handler)
   {
      if (audioUnit == nullptr)
         return nullptr;

      Listener* listener = new Listener();
      listener->mHandler = std::move(handler);

      //delivered on the main queue, which runs in every run loop mode, so events still arrive while a touch is being tracked
      OSStatus status = AUEventListenerCreateWithDispatchQueue(&listener->mRef, 0, 0, dispatch_get_main_queue(), ^(void*, const AudioUnitEvent* event, UInt64, AudioUnitParameterValue) {
         if (!listener->mAlive || event == nullptr)
            return;
         EventType type;
         switch (event->mEventType)
         {
            case kAudioUnitEvent_BeginParameterChangeGesture: type = EventType::BeginGesture; break;
            case kAudioUnitEvent_EndParameterChangeGesture: type = EventType::EndGesture; break;
            default: return;
         }
         listener->mHandler(event->mArgument.mParameter.mParameterID, type);
      });
      if (status != noErr || listener->mRef == nullptr)
      {
         delete listener;
         return nullptr;
      }

      AudioUnitEvent event{};
      event.mArgument.mParameter.mAudioUnit = (AudioUnit)audioUnit;
      event.mArgument.mParameter.mScope = kAudioUnitScope_Global;
      event.mArgument.mParameter.mElement = 0;
      for (unsigned int paramID : paramIDs)
      {
         event.mArgument.mParameter.mParameterID = paramID;
         for (AudioUnitEventType type : { kAudioUnitEvent_BeginParameterChangeGesture, kAudioUnitEvent_EndParameterChangeGesture })
         {
            event.mEventType = type;
            AUEventListenerAddEventType(listener->mRef, nullptr, &event);
         }
      }
      return listener;
   }

   void Destroy(Listener* listener)
   {
      if (listener == nullptr)
         return;
      listener->mAlive = false;
      AUListenerDispose(listener->mRef);
      //events already queued on the main queue still hold this pointer. they run first, see mAlive is false, and do nothing
      dispatch_async(dispatch_get_main_queue(), ^{
         delete listener;
      });
   }
}
