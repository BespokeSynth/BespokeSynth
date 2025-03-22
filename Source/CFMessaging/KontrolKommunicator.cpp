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
//  KontrolKommunicator.cpp
//
//  Created by Ryan Challinor on 11/22/14.
//
//

#include "KontrolKommunicator.h"
#include <mach/mach.h>
#include "NIMessage.h"
#include <iostream>

#define KEYS_MESSAGE "75 67 56 02  73 79 65 4b"
#define RKEY_MESSAGE "75 67 56 02  79 65 4b 52"

KontrolKommunicator::KontrolKommunicator()
: mState(kState_Initialization)
, mListener(NULL)
{
}

void KontrolKommunicator::Init()
{
}

void KontrolKommunicator::Exit()
{
   mSendPorts.clear();
   mListenPorts.clear();
}

void KontrolKommunicator::CreateSender(const char* portName)
{
   mSendPorts[portName].CreateSender(portName);
}

void KontrolKommunicator::CreateListener(const char* portName)
{
   mListenPorts[portName].CreateListener(portName, this);
}

void KontrolKommunicator::OutputRawData2(const uint8_t* data, size_t length)
{
   for (int i = 0; i < length; ++i)
   {
      std::cout << ofToString(data[i]);
      if (i % 4 == 3)
         std::cout << " ";
   }
   std::cout << "\n";
   for (int i = 0; i < length; ++i)
   {
      std::cout << FormatString("%02x ", data[i]);
      if (i % 4 == 3)
         std::cout << " ";
   }
   std::cout << "\n";
}

CFDataRef KontrolKommunicator::OnMessageReceived(std::string portname, SInt32 msgid, CFDataRef data)
{
   vm_address_t vmData;
   vm_allocate(mach_task_self(), &vmData, (vm_size_t)vm_page_size, TRUE);
   char* dataBuffer = (char*)vmData;
   size_t length = CFDataGetLength(data);
   CFDataGetBytes(data, CFRangeMake(0, length), (uint8_t*)dataBuffer);

   if (mListener)
   {
      switch (msgid)
      {
         case 0x03734e00: //NIButtonPressedMessage
         {
            mListener->OnKontrolButton((int)(((uint8_t*)dataBuffer)[16]), (bool)(((uint8_t*)dataBuffer)[20]));
         }
         break;
         case 0x03654e00: //NIWheelsChangedMessage
         {
            mListener->OnKontrolEncoder((int)(((uint8_t*)dataBuffer)[16]), (float)((((int32_t*)dataBuffer)[5]) / 1000000000.0f));
         }
         break;
         case 0x03774e00: //NIBrowseWheelMessage
         {
            mListener->OnKontrolEncoder(8, (float)(((int8_t*)dataBuffer)[20]));
         }
         break;
         case 0x03564e66: //NIOctaveChangedMessage
         {
            mListener->OnKontrolOctave((int)(((uint8_t*)dataBuffer)[8]));
         }
         break;
      }
   }

   mMessageQueueMutex.lock();

   Output(portname + " received CF message with length " + ofToString(length) + ":\n");
   uint32_t messageID = *((uint32_t*)dataBuffer);
   Output(FormatString("0x%08x", messageID) + " (" + TypeForMessageID(messageID) + ")\n");
   OutputRawData((uint8_t*)dataBuffer, length);

   CFDataRef reply = ProcessIncomingMessage(portname, dataBuffer, length);

   mMessageQueueMutex.unlock();

   vm_deallocate(mach_task_self(), vmData, (vm_size_t)vm_page_size);

   return reply;
}

CFDataRef KontrolKommunicator::ProcessIncomingMessage(std::string portName, char* data, size_t length)
{
   KDataArray input;
   input.resize(length);
   for (int i = 0; i < length; ++i)
      input[i] = data[i];
   KDataArray reply = RespondToMessage(portName, input);
   if (!reply.empty())
   {
      vm_address_t vmaddress;
      vm_allocate(mach_task_self(), &vmaddress, (vm_size_t)vm_page_size, TRUE);
      memcpy((void*)vmaddress, reply.data(), reply.size());

      Output("Replying to " + portName + " with length " + ofToString(reply.size()) + ":\n");
      OutputData(reply);

      return CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (uint8_t*)vmaddress, reply.size(), kCFAllocatorNull);
   }
   else
   {
      Output("Replying to " + portName + " with:\n[empty]\n");
   }

   return NULL;
}

//static
uint16_t KontrolKommunicator::CharToSegments(char input)
{
   switch (input)
   {
      case ' ': return 0x0000;
      case 'A': return 0xcf18;
      case 'B': return 0x3f52;
      case 'C': return 0xf300;
      case 'D': return 0x3f42;
      case 'E': return 0xf318;
      case 'F': return 0xc318;
      case 'G': return 0xfb10;
      case 'H': return 0xcc18;
      case 'I': return 0x3342;
      case 'J': return 0x7c00;
      case 'K': return 0xc08c;
      case 'L': return 0xf000;
      case 'M': return 0xcc05;
      case 'N': return 0xcc81;
      case 'O': return 0xff00;
      case 'P': return 0xc718;
      case 'Q': return 0xff80;
      case 'R': return 0xc798;
      case 'S': return 0x3381;
      case 'T': return 0x0342;
      case 'U': return 0xfc00;
      case 'V': return 0xc024;
      case 'W': return 0xcca0;
      case 'X': return 0x00a5;
      case 'Y': return 0x0045;
      case 'Z': return 0x3324;
      case '0': return 0xff24;
      case '1': return 0x3142;
      case '2': return 0x7718;
      case '3': return 0x3f18;
      case '4': return 0x8c18;
      case '5': return 0xbb18;
      case '6': return 0xfb18;
      case '7': return 0x0324;
      case '8': return 0xff18;
      case '9': return 0xbf18;
      case '|': return 0x0042;
      case '/': return 0x0024;
      case '-': return 0x0018;
      case '\\': return 0x0081;
      case '\'': return 0x0002;
      case '?': return 0x0750;
      case '*': return 0x00ff;
      case '+': return 0x005a;
      case '#': return 0xc05a;
      case '.': return 0;
   }
   return 0xffff;
}

//static
KDataArray KontrolKommunicator::LettersToData(std::string input)
{
   KDataArray data;
   data.resize(input.length() * 2);
   for (int i = 0; i < input.length(); ++i)
   {
      uint16_t segments = CharToSegments(input[i]);
      data[i * 2] = *(((uint8_t*)&segments) + 1);
      data[i * 2 + 1] = segments;
   }
   return data;
}

void KontrolKommunicator::Update()
{
   /*static float timer = 0;
   const char rotate[8] = { '|', '/', '-', '\\', '|', '/', '-', '\\'};
   static uint8_t cycle = 0;
   static uint8_t color = 0;
   if (mState == kState_Focused)
   {
      timer += ofGetLastFrameTime();
      const float timeStep = 1.0f/12.0f;
      while (timer > timeStep)
      {
         timer -= timeStep;
         cycle = (cycle+1) % 8;
         ++color;
         
         KDataArray text(MessageIDForType("NIDisplayDrawMessage"));
         text += StringToData("00 00 00 00  00 00 00 00  03 00 48 00  b0 01 00 00");   //some sort of header
         int idLength = 4;
         int headerLength = 16;
         int rowLength = 144;
         uint16_t segments = CharToSegments(rotate[cycle]);
         
         string row = "        ";
         for (int i=0; i<18; ++i)
            row += rotate[cycle];
         row += " WHO'S   IN   KONTROL NOW?  ";
         for (int i=0; i<18; ++i)
            row += rotate[cycle];
         KDataArray centerRow = LettersToData(row);
         row = "        ";
         for (int i=0; i<8*8; ++i)
            row += rotate[cycle];
         KDataArray bottomRow = LettersToData(row);
         
         text += (uint8_t)0;
         for (int i=0; i<rowLength-1; ++i)
            text += (uint8_t)4;
         text += centerRow;
         text += bottomRow;
         
         QueueMessage(mRequestSerialPort, text);
         
         KDataArray lights(MessageIDForType("NISetLedStateMessage"));
         
         lights += StringToData("d0 00 00 00  13 00 00 00  00 00 00 00  00 00 00 13  00 13 00 00  00 00 00 00  00 13 00 13  00"); //some sort of header
         for (int i=0; i<61*3; ++i)
            lights += (uint8_t)(powf(ofRandom(1),5)*255);
         
         QueueMessage(mRequestSerialPort, lights);
      }
   }*/

   mMessageQueueMutex.lock();
   if (!mMessageQueue.empty())
   {
      QueuedMessage message = *mMessageQueue.begin();
      mMessageQueue.pop_front();
      SendMessage(message.mPort, message.mMessage);
   }
   mMessageQueueMutex.unlock();
}

void KontrolKommunicator::SendMessage(std::string portName, KDataArray data)
{
   if (mSendPorts.find(portName) == mSendPorts.end())
      CreateSender(portName.c_str());

   vm_address_t vmaddress;
   vm_allocate(mach_task_self(), &vmaddress, (vm_size_t)vm_page_size, TRUE);
   memcpy((void*)vmaddress, data.data(), data.size());
   uint32_t messageID = *((uint32_t*)data.data());

   Output("Sending message (" + TypeForMessageID(messageID) + ") to " + portName + " with length " + ofToString(data.size()) + " and contents:\n");
   OutputRawData((uint8_t*)vmaddress, data.size());

   CFDataRef reply = NULL;
   mSendPorts[portName].SendData(*((uint32_t*)vmaddress), CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (uint8_t*)vmaddress, data.size(), kCFAllocatorNull), reply);

   if (reply)
   {
      vm_deallocate(mach_task_self(), vmaddress, (vm_size_t)vm_page_size);

      Output("Received reply on " + portName + " with length " + ofToString(CFDataGetLength(reply)) + " and contents:\n");

      vm_address_t vmData;
      vm_allocate(mach_task_self(), &vmData, (vm_size_t)vm_page_size, TRUE);
      char* dataBuffer = (char*)vmData;
      int replyLength = (int)CFDataGetLength(reply);
      CFDataGetBytes(reply, CFRangeMake(0, replyLength), (uint8_t*)dataBuffer);

      OutputRawData((uint8_t*)vmData, replyLength);

      if (DataEquals(data, StringToData(RKEY_MESSAGE)))
      {
         if (mListener)
            mListener->OnKontrolOctave((uint8_t)dataBuffer[0]);
      }

      FollowUpToReply(TypeForMessageID(messageID), (uint8_t*)vmData);

      vm_deallocate(mach_task_self(), vmData, (vm_size_t)vm_page_size);
      CFRelease(reply);
   }
}

void KontrolKommunicator::AddReply(std::string portName, std::string input, std::string reply)
{
   ReplyEntry entry;
   entry.mPort = portName;
   entry.mInput = StringToData(input);
   entry.mReply = StringToData(reply);
   mReplies.push_back(entry);
}

void KontrolKommunicator::QueueMessage(std::string portName, KDataArray message)
{
   QueuedMessage entry;
   entry.mPort = portName;
   entry.mMessage = message;
   mMessageQueueMutex.lock();
   mMessageQueue.push_back(entry);
   mMessageQueueMutex.unlock();
}

KDataArray KontrolKommunicator::RespondToMessage(std::string portName, KDataArray input)
{
   uint32_t messageID = *((uint32_t*)input.data());
   std::string type = TypeForMessageID(messageID);

   //fake software responses
   if (portName == mNotificationPort)
   {
      if (type == "NISetSerialNumberMessage")
      {
         mState = kState_InitializeSerial;
         mSerialNumber = "";
         for (int i = 16; i < input.size(); i += 2)
            mSerialNumber += (char)input[i];
         Output("SERIAL NUMBER:" + mSerialNumber + "\n");
         QueueMessage("NIHWMainHandler", CreateMessage("NIGetServiceVersionMessage"));
         return KDataArray();
      }
   }
   if (portName == mNotificationSerialPort)
   {
      if (type == "NIDeviceStateChangeMessage")
      {
         mState = kState_Initialized;
         QueueMessage(mRequestSerialPort, CreateMessage("NIDisplayDrawMessage"));
         //mystery message
         QueueMessage(mRequestSerialPort, StringToData("00 43 43 02  72 65 73 75 "));
         return KDataArray();
      }
      if (type == "NISetFocusMessage")
      {
         mState = kState_Focused;
         QueueMessage(mRequestSerialPort, StringToData("54 73 49 02  00 00 00 00")); //unknown
         QueueMessage(mRequestSerialPort, StringToData("74 73 49 02  65 75 72 74  ")); //unknown
         QueueMessage(mRequestSerialPort, StringToData("56 73 6b 02  03 00 00 00  ")); //unknown
         QueueMessage(mRequestSerialPort, StringToData("43 73 74 02  00 00 00 00  00 00 00 00  28 00 00 00  06 00 00 00  00 00 ff 3f  00 00 01 00  03 00 00 02  00 00 00 00  03 01 00 00  00 00 7f 00  00 00 00 01  00 00 00 00  00 00 00 00  ")); //unknown
         QueueMessage(mRequestSerialPort, StringToData("43 73 70 02  00 00 00 00  00 00 00 00  18 00 00 00  03 40 00 06  00 00 7f 00  03 41 00 06  00 00 7f 00  03 0b 00 00  00 00 7f 00  ")); //unknown
         return KDataArray();
      }
   }

   Output("&&& No appropriate reply found.\n");
   return KDataArray();
}

int WordAlign(int input)
{
   return (input + 3) / 4 * 4;
}

void KontrolKommunicator::FollowUpToReply(std::string messageType, uint8_t* reply)
{
   //fake software followups
   if (messageType == "NIGetServiceVersionMessage")
   {
      if (mState == kState_Initialization)
         QueueMessage("NIHWMainHandler", CreateMessage("NIHWSDeviceConnectMessage"));
      else if (mState == kState_InitializeSerial)
         QueueMessage("NIHWMainHandler", CreateMessage("NIHWSDeviceConnectSerialMessage"));
   }
   if (messageType == "NIHWSDeviceConnectMessage")
   {
      mRequestPort = (char*)(reply + 8);
      mNotificationPort = (char*)(reply + 8 + WordAlign((int)mRequestPort.length()) + /*4*/ 5); // TODO(Ryan) plus 5??? it used to be plus 4.
      Output("REQUEST PORT:" + mRequestPort + " NOTIFICATION PORT:" + mNotificationPort + "\n");

      CreateListener(mNotificationPort.c_str());
      QueueMessage(mRequestPort, CreateMessage("NISetAsciiStringMessage"));
   }
   if (messageType == "NISetAsciiStringMessage")
   {
      if (mState == kState_Initialization)
         QueueMessage(mRequestPort, CreateMessage("NIRequestSerialNumberMessage"));
      if (mState == kState_InitializeSerial)
      {
         QueueMessage(mRequestSerialPort, StringToData(KEYS_MESSAGE));
         QueueMessage(mRequestSerialPort, StringToData(RKEY_MESSAGE));
      }
   }
   if (messageType == "NIHWSDeviceConnectSerialMessage")
   {
      mRequestSerialPort = (char*)(reply + 8);
      mNotificationSerialPort = (char*)(reply + 8 + WordAlign((int)mRequestSerialPort.length()) + 4);
      Output("REQUEST SERIAL PORT:" + mRequestSerialPort + " NOTIFICATION SERIAL PORT:" + mNotificationSerialPort + "\n");

      CreateListener(mNotificationSerialPort.c_str());
      QueueMessage(mRequestSerialPort, CreateMessage("NISetAsciiStringMessage"));
   }
}

KDataArray KontrolKommunicator::CreateMessage(std::string type)
{
   uint32_t messageID = MessageIDForType(type);
   KDataArray ret(messageID);

   //fake software messages
   if (type == "NIGetServiceVersionMessage")
   {
   }
   if (type == "NIHWSDeviceConnectMessage")
   {
      //connects a Komplete Kontrol
      ret += StringToData("50 13 00 00  4b 4b 69 4e  79 6d 72 70  11 00 00 00  4b 00 6f 00  6d 00 70 00  6c 00 65 00  74 00 65 00  20 00 4b 00  6f 00 6e 00  74 00 72 00  6f 00 6c 00  00 00");
   }
   if (type == "NISetAsciiStringMessage")
   {
      if (mState == kState_Initialization)
      {
         //sends a message with notification port in it
         ret += StringToData("80 bc 97 53  ff 7f 00 00  25 00 00 00");
         ret += KDataArray(mNotificationPort);
      }
      if (mState == kState_InitializeSerial)
      {
         //sends a message with notification serial port in it
         ret += StringToData("00 00 00 00  00 00 00 00  2d 00 00 00");
         ret += KDataArray(mNotificationSerialPort);
      }
   }
   if (type == "NIHWSDeviceConnectSerialMessage")
   {
      ret += StringToData("50 13 00 00  4b 4b 69 4e  79 6d 72 70  09 00 00 00");
      KDataArray zero = KDataArray((uint8_t)0);
      for (int i = 0; i < mSerialNumber.length(); ++i)
         ret += KDataArray((uint8_t)mSerialNumber[i]) + zero;
      ret += zero + zero;
   }
   if (type == "NIDeviceStateChangeMessage")
   {
      ret += StringToData("65 75 72 74");
   }
   if (type == "NIDisplayDrawMessage")
   {
      ret += StringToData("00 00 00 00  00 00 00 00  03 00 48 00  b0 01 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 c7 18  c7 98 f3 18  33 81 33 81  3f 52 c7 98  ff 00 cc a0  33 81 f3 18  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00");
   }
   if (type == "NISetFocusMessage")
   {
      ret += StringToData("65 75 72 74");
   }
   return ret;
}

void KontrolKommunicator::Output(std::string str)
{
   return;

   std::cout << str;
}

std::string KontrolKommunicator::FormatString(std::string format, int number)
{
   char buffer[100];
   snprintf(buffer, sizeof(buffer), format.c_str(), number);
   return (std::string)buffer;
}

KDataArray KontrolKommunicator::StringToData(std::string input)
{
   std::vector<std::string> tokens = ofSplitString(input, " ", true);
   size_t length = tokens.size();
   KDataArray data;
   data.resize(length);
   for (int i = 0; i < length; ++i)
      data[i] = ofHexToInt(tokens[i]);
   return data;
}

bool KontrolKommunicator::DataEquals(const KDataArray& a, const KDataArray& b)
{
   if (a.size() != b.size())
      return false;
   for (int i = 0; i < a.size(); ++i)
   {
      if (a[i] != b[i])
         return false;
   }
   return true;
}

void KontrolKommunicator::OutputData(const KDataArray& a)
{
   for (int i = 0; i < a.size(); ++i)
   {
      Output(ofToString(a[i]));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
   for (int i = 0; i < a.size(); ++i)
   {
      Output(FormatString("%02x ", a[i]));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
   for (int i = 0; i < a.size(); ++i)
   {
      char c;
      if (a[i] > 32 && a[i] < 127)
         c = a[i];
      else
         c = '#';
      Output(FormatString("%c ", c));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
}

void KontrolKommunicator::OutputRawData(const uint8_t* data, size_t length)
{
   for (int i = 0; i < length; ++i)
   {
      Output(ofToString(data[i]));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
   for (int i = 0; i < length; ++i)
   {
      Output(FormatString("%02x ", data[i]));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
   for (int i = 0; i < length; ++i)
   {
      char c;
      if (data[i] > 32 && data[i] < 127)
         c = data[i];
      else
         c = '#';
      Output(FormatString("%c ", c));
      if (i % 4 == 3)
         Output(" ");
   }
   Output("\n");
}

void KontrolKommunicator::SetDisplay(const uint16_t sliders[72], std::string display)
{
   KDataArray text(MessageIDForType("NIDisplayDrawMessage"));
   text += StringToData("00 00 00 00  00 00 00 00  03 00 48 00  b0 01 00 00"); //some sort of header
   int rowLength = 72;
   assert(display.length() == rowLength * 2);

   for (int i = 0; i < rowLength; ++i)
      text += (uint16_t)sliders[i];

   KDataArray output = LettersToData(display);
   text += output;

   SendMessage(mRequestSerialPort, text);
}

void KontrolKommunicator::SetKeyLights(ofColor keys[61])
{
   KDataArray lights(MessageIDForType("NISetLedStateMessage"));

   lights += StringToData("d0 00 00 00  13 00 00 00  00 00 00 00  00 00 00 13  00 13 00 00  00 00 00 00  00 13 00 13  00"); //some sort of header

   for (int i = 0; i < 61; ++i)
   {
      lights += (uint8_t)keys[i].r;
      lights += (uint8_t)keys[i].g;
      lights += (uint8_t)keys[i].b;
   }

   SendMessage(mRequestSerialPort, lights);
}
