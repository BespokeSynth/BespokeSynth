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

class ListenPortCallback
{
public:
   virtual ~ListenPortCallback() {}
   virtual CFDataRef OnMessageReceived(std::string portName, SInt32 msgid, CFDataRef data) = 0;
};

class ListenPort
{
public:
   ListenPort()
   : mReceivePort(NULL)
   {}
   ~ListenPort();
   void CreateListener(const char* portName, ListenPortCallback* callback);
   CFDataRef OnMessageReceived(SInt32 msgid, CFDataRef data);
   void Close();

private:
   std::string mPortName;
   CFMessagePortRef mReceivePort;
   CFRunLoopSourceRef mLoopSource;
   ListenPortCallback* mCallback;
};

class ListenPortReceiver
{
public:
   static CFDataRef OnMessageReceived(CFMessagePortRef local, SInt32 msgid, CFDataRef data, void* info);
   static void RegisterPort(ListenPort* port, CFMessagePortRef portRef);

private:
   static ListenPort* LookupPort(CFMessagePortRef portRef);

   static std::map<CFMessagePortRef, ListenPort*> mPortMap;
};

#endif /* defined(__NI_SoftwareSide__ListenPort__) */
