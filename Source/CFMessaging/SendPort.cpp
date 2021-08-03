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
