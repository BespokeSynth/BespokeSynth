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

using namespace std;

class KDataArray
{
public:
   KDataArray() {}
   KDataArray(uint8_t val) { mData.assign(&val, &val+1); }
   KDataArray(uint16_t val) { mData.assign(((uint8_t*)&val), ((uint8_t*)&val)+2); }
   KDataArray(uint32_t messageId) { mData.assign(((uint8_t*)&messageId), ((uint8_t*)&messageId)+4); }
   KDataArray(string input) { mData.assign(input.c_str(), input.c_str()+input.length()+1); }
   uint8_t* data() { return mData.data(); }
   void resize(size_t size) { mData.resize(size); }
   size_t size() const { return mData.size(); }
   bool empty() const { return mData.empty(); }
   const uint8_t& operator[](size_t i) const { return mData[i]; }
   uint8_t& operator[](size_t i) { return mData[i]; }
   KDataArray operator+(const KDataArray& b) { mData.insert(mData.end(), b.mData.begin(), b.mData.end()); return *this; }
   void operator+=(const KDataArray& b) { mData.insert(mData.end(), b.mData.begin(), b.mData.end()); }
private:
   vector<uint8_t> mData;
};

struct ReplyEntry
{
   string mPort;
   KDataArray mInput;
   KDataArray mReply;
};

struct QueuedMessage
{
   string mPort;
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
   CFDataRef OnMessageReceived(string portName, SInt32 msgid, CFDataRef data) override;
   void AddReply(string portName, string input, string reply);
   void QueueMessage(string portName, KDataArray message);
   void Update();
   static KDataArray StringToData(string input);
   static KDataArray LettersToData(string letters);
   static uint16_t CharToSegments(char input);
   KDataArray CreateMessage(string type);
   
   bool IsReady() const { return mState == kState_Focused; }
   void SetListener(IKontrolListener* listener) { mListener = listener; }
   void SetDisplay(const uint16_t sliders[72], string display);
   void SetKeyLights(ofColor keys[61]);
   
private:
   void SendMessage(string portName, KDataArray data);
   void CreateSender(const char* portName);
   CFDataRef ProcessIncomingMessage(string portName, char* data, size_t length);
   static void Output(string str);
   string FormatString(string format, int number);
   KDataArray RespondToMessage(string portName, KDataArray input);
   void FollowUpToReply(string messageType, uint8_t* reply);
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
   }mState;
   
   string mSerialNumber;
   string mRequestPort;
   string mNotificationPort;
   string mRequestSerialPort;
   string mNotificationSerialPort;
   vector<ReplyEntry> mReplies;
   list<QueuedMessage> mMessageQueue;
   map<string,ListenPort> mListenPorts;
   map<string,SendPort> mSendPorts;
   ofMutex mMessageQueueMutex;
   IKontrolListener* mListener;
};

#endif /* defined(__NI_FakeDaemon__KontrolKommunicator__) */
