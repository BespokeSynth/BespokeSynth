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
//  NIMessage.h
//  NI_FakeDaemon
//
//  Created by Ryan Challinor on 11/23/14.
//
//

#ifndef CFMessaging_NIMessage_h
#define CFMessaging_NIMessage_h

#include <cstdint>
#include <string>

typedef struct
{
   uint32_t messageId;
   std::string className;
} MessageIdToString;

#define NUM_NI_MESSAGE_IDS 27

extern MessageIdToString MessageIdToStringTable[NUM_NI_MESSAGE_IDS];

std::string TypeForMessageID(uint32_t messageId);
uint32_t MessageIDForType(std::string type);

#endif
