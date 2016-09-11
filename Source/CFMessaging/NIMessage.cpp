//
//  NIMessage.cpp
//  NI_FakeDaemon
//
//  Created by Ryan Challinor on 11/24/14.
//
//

#ifdef BESPOKE_MAC

#include "NIMessage.h"

MessageIdToString MessageIdToStringTable[NUM_NI_MESSAGE_IDS] = {
   { 0x02536756, "NIGetServiceVersionMessage"   },
   { 0x02444300, "NIDeviceConnectMessage"       },
   { 0x02404300, "NISetAsciiStringMessage"      },
   { 0x02446724, "NIGetDeviceEnabledMessage"    },
   { 0x02444e00, "NIDeviceStateChangeMessage"   },
   { 0x02446743, "NIGetDeviceAvailableMessage"  },
   { 0x02434e00, "NISetFocusMessage"            },
   { 0x02446744, "NIGetDriverVersionMessage"    },
   { 0x02436746, "NIGetFirmwareVersionMessage"  },
   { 0x02436753, "NIGetSerialNumberMessage"     },
   { 0x02646749, "NIGetDisplayInvertedMessage"  },
   { 0x02646743, "NIGetDisplayContrastMessage"  },
   { 0x02646742, "NIGetDisplayBacklightMessage" },
   { 0x02566766, "NIGetFloatPropertyMessage"    },
   { 0x02647344, "NIDisplayDrawMessage"         },
   { 0x02654e00, "NIWheelsChangedMessage"       },
   { 0x02504e00, "NIPadsChangedMessage"         },
   { 0x026c7500, "NISetLedStateMessage"         },
   { 0x02447500, "NIHWSDeviceConnectMessage"    },
   { 0x02447143, "NIRequestSerialNumberMessage" },
   { 0x02444e2b, "NISetSerialNumberMessage"     },
   { 0x02442d00, "NISoftwareShuttingDownMessage"    },
   { 0x02444900, "NIHWSDeviceConnectSerialMessage"  },
   { 0x02434300, "NIMysteryMessage" },
   { 0x02734e00, "NIButtonPressedMessage" },
   { 0x02774e00, "NIBrowseWheelMessage" },
   { 0x02564e66, "NIOctaveChangedMessage" },
};

string TypeForMessageID(uint32_t messageId)
{
   for (int i=0; i<NUM_NI_MESSAGE_IDS; ++i)
   {
      if (MessageIdToStringTable[i].messageId == messageId)
         return MessageIdToStringTable[i].className;
   }
   char messageIdStr[16];
   sprintf(messageIdStr, "0x%08x", messageId);
   return string("Unknown message type ")+messageIdStr;
}

uint32_t MessageIDForType(string type)
{
   for (int i=0; i<NUM_NI_MESSAGE_IDS; ++i)
   {
      if (MessageIdToStringTable[i].className == type)
         return MessageIdToStringTable[i].messageId;
   }
   return 0;
}

#endif
