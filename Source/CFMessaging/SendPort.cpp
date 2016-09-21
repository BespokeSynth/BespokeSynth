//
//  SendPort.cpp
//  NI_SoftwareSide
//
//  Created by Ryan Challinor on 11/6/14.
//
//

#include "SendPort.h"

SendPort::~SendPort()
{
   if (mSendPort)
      CFRelease(mSendPort);
}

void SendPort::CreateSender(const char* portName)
{
   mSendPort = CFMessagePortCreateRemote(kCFAllocatorDefault, CFStringCreateWithCString(kCFAllocatorDefault, portName, kCFStringEncodingASCII));
   assert(mSendPort != NULL);
}

void SendPort::SendData(uint32_t messageId, CFDataRef data, CFDataRef& replyData)
{
   CFMessagePortSendRequest(mSendPort,
                            messageId,
                            data,
                            1,
                            1,
                            kCFRunLoopDefaultMode,
                            &replyData);
}

void SendPort::Close()
{
}
