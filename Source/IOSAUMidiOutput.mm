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
//  IOSAUMidiOutput.mm
//  Bespoke
//

#include "IOSAUMidiOutput.h"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreMIDI/CoreMIDI.h>
#import <os/lock.h>

#include <algorithm>

namespace IOSAUMidiOutput
{
   namespace
   {
      struct Event
      {
         unsigned char mData[3];
         int mLength;
         int mSampleOffset;
      };

      constexpr int kMaxEvents = 1024; //per block. fixed, so that nothing is allocated on the audio thread
   }

   struct Receiver
   {
      os_unfair_lock mLock = OS_UNFAIR_LOCK_INIT; //in case a plugin sends from outside of the render call
      Event mEvents[kMaxEvents];
      int mNumEvents{ 0 };
      Event mCollected[kMaxEvents];
      int mBufferSize{ 0 };
   };

   namespace
   {
      int GetSampleOffset(const Receiver* receiver, const AudioTimeStamp* blockTime, MIDITimeStamp packetTime)
      {
         //documented as an offset into the block, but allow for an absolute sample time too
         if (packetTime < (MIDITimeStamp)receiver->mBufferSize)
            return (int)packetTime;
         if (blockTime != nullptr && (blockTime->mFlags & kAudioTimeStampSampleTimeValid) && blockTime->mSampleTime >= 0)
         {
            const MIDITimeStamp blockStart = (MIDITimeStamp)blockTime->mSampleTime;
            if (packetTime >= blockStart && packetTime - blockStart < (MIDITimeStamp)receiver->mBufferSize)
               return (int)(packetTime - blockStart);
         }
         return 0;
      }

      OSStatus OnMidiOutput(void* userData, const AudioTimeStamp* timeStamp, UInt32 midiOutNum, const MIDIPacketList* packets)
      {
         if (packets == nullptr)
            return noErr;

         Receiver* receiver = static_cast<Receiver*>(userData);
         os_unfair_lock_lock(&receiver->mLock);
         const MIDIPacket* packet = &packets->packet[0];
         for (UInt32 i = 0; i < packets->numPackets; ++i)
         {
            const int sampleOffset = GetSampleOffset(receiver, timeStamp, packet->timeStamp);
            int pos = 0;
            while (pos < packet->length)
            {
               const unsigned char status = packet->data[pos];
               if (status < 0x80 || status >= 0xf0) //system message or the inside of a sysex, skip to the next status byte
               {
                  ++pos;
                  continue;
               }
               const unsigned char type = status & 0xf0;
               const int length = (type == 0xc0 || type == 0xd0) ? 2 : 3; //program change and channel pressure are short
               if (pos + length > packet->length)
                  break;
               if (receiver->mNumEvents < kMaxEvents)
               {
                  Event& event = receiver->mEvents[receiver->mNumEvents++];
                  std::copy(packet->data + pos, packet->data + pos + length, event.mData);
                  event.mLength = length;
                  event.mSampleOffset = sampleOffset;
               }
               pos += length;
            }
            packet = MIDIPacketNext(packet);
         }
         os_unfair_lock_unlock(&receiver->mLock);
         return noErr;
      }
   }

   void Deleter::operator()(Receiver* receiver) const
   {
      delete receiver;
   }

   Receiver* Create(void* audioUnit, int bufferSize)
   {
      if (audioUnit == nullptr)
         return nullptr;

      CFArrayRef outputNames = nullptr;
      UInt32 size = sizeof(outputNames);
      if (AudioUnitGetProperty((AudioUnit)audioUnit, kAudioUnitProperty_MIDIOutputCallbackInfo, kAudioUnitScope_Global, 0, &outputNames, &size) != noErr || outputNames == nullptr)
         return nullptr;
      const CFIndex numOutputs = CFArrayGetCount(outputNames);
      CFRelease(outputNames);
      if (numOutputs == 0)
         return nullptr;

      Receiver* receiver = new Receiver();
      receiver->mBufferSize = bufferSize;

      AUMIDIOutputCallbackStruct callback{};
      callback.midiOutputCallback = OnMidiOutput;
      callback.userData = receiver;
      if (AudioUnitSetProperty((AudioUnit)audioUnit, kAudioUnitProperty_MIDIOutputCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback)) != noErr)
      {
         delete receiver;
         return nullptr;
      }
      return receiver;
   }

   void Collect(Receiver* receiver, const std::function<void(const unsigned char* data, int length, int sampleOffset)>& handler)
   {
      os_unfair_lock_lock(&receiver->mLock);
      const int numEvents = receiver->mNumEvents;
      std::copy(receiver->mEvents, receiver->mEvents + numEvents, receiver->mCollected);
      receiver->mNumEvents = 0;
      os_unfair_lock_unlock(&receiver->mLock);

      for (int i = 0; i < numEvents; ++i)
         handler(receiver->mCollected[i].mData, receiver->mCollected[i].mLength, receiver->mCollected[i].mSampleOffset);
   }
}
