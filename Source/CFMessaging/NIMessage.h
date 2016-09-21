//
//  NIMessage.h
//  NI_FakeDaemon
//
//  Created by Ryan Challinor on 11/23/14.
//
//

#ifndef CFMessaging_NIMessage_h
#define CFMessaging_NIMessage_h

#include <iostream>

using namespace std;

typedef struct
{
   uint32_t   messageId;
   string className;
} MessageIdToString;

#define NUM_NI_MESSAGE_IDS 27

extern MessageIdToString MessageIdToStringTable[NUM_NI_MESSAGE_IDS];

string TypeForMessageID(uint32_t messageId);
uint32_t MessageIDForType(string type);

#endif
