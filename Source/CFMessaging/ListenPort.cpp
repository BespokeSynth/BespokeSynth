//
//  ListenPort.cpp
//  NI_SoftwareSide
//
//  Created by Ryan Challinor on 11/5/14.
//  Copyright (c) 2014 Ryan Challinor. All rights reserved.
//


#include "ListenPort.h"
#include <iostream>

ListenPort::~ListenPort()
{
   Close();
}

void* SetupRunLoop(void* source)
{
   //add the source to the run loop
   CFRunLoopAddSource(CFRunLoopGetCurrent(), *((CFRunLoopSourceRef *)source), kCFRunLoopDefaultMode);
   
   // start the run loop
   CFRunLoopRun();
   
   // exit thread when the runloop is done
   CFShow(CFSTR("thread exiting"));
   pthread_exit(0);
}

void ListenPort::CreateListener(const char* portName, ListenPortCallback* callback)
{
   cout << "Listening on port " << portName << endl;
   
   mPortName = portName;
   mCallback = callback;
   
   Boolean shouldFree;
   mReceivePort = CFMessagePortCreateLocal(kCFAllocatorDefault, CFStringCreateWithCString(kCFAllocatorDefault, portName, kCFStringEncodingASCII), ListenPortReceiver::OnMessageReceived, NULL, &shouldFree);
   
   ListenPortReceiver::RegisterPort(this, mReceivePort);
   
   // create the runloop souce from the local message port passed in
   mLoopSource = CFMessagePortCreateRunLoopSource(kCFAllocatorDefault, mReceivePort, (CFIndex)0);
   
   pthread_t a;
   pthread_create(&a, 0, SetupRunLoop, (void *)&mLoopSource);
   pthread_detach(a);
}

CFDataRef ListenPort::OnMessageReceived(SInt32 msgid, CFDataRef data)
{
   return mCallback->OnMessageReceived(mPortName, msgid, data);
}

void ListenPort::Close()
{
   if (mReceivePort)
   {
      CFMessagePortInvalidate(mReceivePort);
      while (CFMessagePortIsValid(mReceivePort))
      {
         // sleep 0.1ms
         usleep(100);
      }
      CFRelease(mReceivePort);
   }
   mReceivePort = NULL;
}

//static
map<CFMessagePortRef, ListenPort*> ListenPortReceiver::mPortMap;

//static
void ListenPortReceiver::RegisterPort(ListenPort* port, CFMessagePortRef portRef)
{
   mPortMap[portRef] = port;
}

//static
ListenPort* ListenPortReceiver::LookupPort(CFMessagePortRef portRef)
{
   return mPortMap[portRef];
}

//static
CFDataRef ListenPortReceiver::OnMessageReceived(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info)
{
   ListenPort* port = LookupPort(local);
   
   return port->OnMessageReceived(msgid, data);
}
