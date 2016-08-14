//
//  ListenPort.h
//  NI_SoftwareSide
//
//  Created by Ryan Challinor on 11/5/14.
//  Copyright (c) 2014 Ryan Challinor. All rights reserved.
//

#ifndef __NI_SoftwareSide__ListenPort__
#define __NI_SoftwareSide__ListenPort__

#include <CoreFoundation/CoreFoundation.h>
#include <string>
#include <map>

using namespace std;

class ListenPortCallback
{
public:
   virtual ~ListenPortCallback() {}
   virtual CFDataRef OnMessageReceived(string portName, SInt32 msgid, CFDataRef data) = 0;
};

class ListenPort
{
public:
   ListenPort() : mReceivePort(NULL) {}
   ~ListenPort();
   void CreateListener(const char* portName, ListenPortCallback* callback);
   CFDataRef OnMessageReceived(SInt32 msgid, CFDataRef data);
   void Close();
private:
   string mPortName;
   CFMessagePortRef mReceivePort;
   CFRunLoopSourceRef mLoopSource;
   ListenPortCallback* mCallback;
};

class ListenPortReceiver
{
public:
   static CFDataRef OnMessageReceived(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info);
   static void RegisterPort(ListenPort* port, CFMessagePortRef portRef);
private:
   static ListenPort* LookupPort(CFMessagePortRef portRef);
   
   static map<CFMessagePortRef, ListenPort*> mPortMap;
};

#endif /* defined(__NI_SoftwareSide__ListenPort__) */
