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
//  KontrolKommunicator.h
//
//  Created by Ryan Challinor on 11/22/14.
//
//

#ifndef __NI_FakeDaemon__KontrolKommunicator__
#define __NI_FakeDaemon__KontrolKommunicator__

#include <string>
#include <map>
#include "ListenPort.h"
#include "SendPort.h"
#include <CoreFoundation/CoreFoundation.h>
#include "../OpenFrameworksPort.h"

class KDataArray
{
public:
   KDataArray() {}
   KDataArray(uint8_t val) { mData.assign(&val, &val + 1); }
   KDataArray(uint16_t val) { mData.assign(((uint8_t*)&val), ((uint8_t*)&val) + 2); }
   KDataArray(uint32_t messageId) { mData.assign(((uint8_t*)&messageId), ((uint8_t*)&messageId) + 4); }
   KDataArray(std::string input) { mData.assign(input.c_str(), input.c_str() + input.length() + 1); }
   uint8_t* data() { return mData.data(); }
   void resize(size_t size) { mData.resize(size); }
   size_t size() const { return mData.size(); }
   bool empty() const { return mData.empty(); }
   const uint8_t& operator[](size_t i) const { return mData[i]; }
   uint8_t& operator[](size_t i) { return mData[i]; }
   KDataArray operator+(const KDataArray& b)
   {
      mData.insert(mData.end(), b.mData.begin(), b.mData.end());
      return *this;
   }
   void operator+=(const KDataArray& b) { mData.insert(mData.end(), b.mData.begin(), b.mData.end()); }

private:
   std::vector<uint8_t> mData;
};

struct ReplyEntry
{
   std::string mPort;
   KDataArray mInput;
   KDataArray mReply;
};

struct QueuedMessage
{
   std::string mPort;
   KDataArray mMessage;
};

class IKontrolListener
{
public:
   virtual ~IKontrolListener() {}
   virtual void OnKontrolButton(int control, bool on) = 0;
   virtual void OnKontrolEncoder(int control, float change) = 0;
   virtual void OnKontrolOctave(int octave) = 0;
};

class KontrolKommunicator : public ListenPortCallback
{
public:
   KontrolKommunicator();
   virtual ~KontrolKommunicator() {}
   void Init();
   void Exit();
   void CreateListener(const char* portName);
   CFDataRef OnMessageReceived(std::string portName, SInt32 msgid, CFDataRef data) override;
   void AddReply(std::string portName, std::string input, std::string reply);
   void QueueMessage(std::string portName, KDataArray message);
   void Update();
   static KDataArray StringToData(std::string input);
   static KDataArray LettersToData(std::string letters);
   static uint16_t CharToSegments(char input);
   KDataArray CreateMessage(std::string type);

   bool IsReady() const { return mState == kState_Focused; }
   void SetListener(IKontrolListener* listener) { mListener = listener; }
   void SetDisplay(const uint16_t sliders[72], std::string display);
   void SetKeyLights(ofColor keys[61]);

private:
   void SendMessage(std::string portName, KDataArray data);
   void CreateSender(const char* portName);
   CFDataRef ProcessIncomingMessage(std::string portName, char* data, size_t length);
   static void Output(std::string str);
   std::string FormatString(std::string format, int number);
   KDataArray RespondToMessage(std::string portName, KDataArray input);
   void FollowUpToReply(std::string messageType, uint8_t* reply);
   bool DataEquals(const KDataArray& a, const KDataArray& b);
   void OutputData(const KDataArray& a);
   void OutputRawData(const uint8_t* data, size_t length);
   void OutputRawData2(const uint8_t* data, size_t length); //circumvents Output() being disabled

   enum State
   {
      kState_Initialization,
      kState_InitializeSerial,
      kState_Initialized,
      kState_Focused
   } mState;

   std::string mSerialNumber;
   std::string mRequestPort;
   std::string mNotificationPort;
   std::string mRequestSerialPort;
   std::string mNotificationSerialPort;
   std::vector<ReplyEntry> mReplies;
   std::list<QueuedMessage> mMessageQueue;
   std::map<std::string, ListenPort> mListenPorts;
   std::map<std::string, SendPort> mSendPorts;
   ofMutex mMessageQueueMutex;
   IKontrolListener* mListener;
};

#endif /* defined(__NI_FakeDaemon__KontrolKommunicator__) */
