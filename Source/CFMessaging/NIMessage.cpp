//
//  NIMessage.cpp
//  NI_FakeDaemon
//
//  Created by Ryan Challinor on 11/24/14.
//
//

#include "NIMessage.h"

MessageIdToString MessageIdToStringTable[NUM_NI_MESSAGE_IDS] = {
   { 0x03536756, "NIGetServiceVersionMessage"   },
   { 0x03444300, "NIDeviceConnectMessage"       },
   { 0x03404300, "NISetAsciiStringMessage"      },
   { 0x03446724, "NIGetDeviceEnabledMessage"    },
   { 0x03444e00, "NIDeviceStateChangeMessage"   },
   { 0x03446743, "NIGetDeviceAvailableMessage"  },
   { 0x03434e00, "NISetFocusMessage"            },
   { 0x03446744, "NIGetDriverVersionMessage"    },
   { 0x03436746, "NIGetFirmwareVersionMessage"  },
   { 0x03436753, "NIGetSerialNumberMessage"     },
   { 0x03646749, "NIGetDisplayInvertedMessage"  },
   { 0x03646743, "NIGetDisplayContrastMessage"  },
   { 0x03646742, "NIGetDisplayBacklightMessage" },
   { 0x03566766, "NIGetFloatPropertyMessage"    },
   { 0x03647344, "NIDisplayDrawMessage"         },
   { 0x03654e00, "NIWheelsChangedMessage"       },
   { 0x03504e00, "NIPadsChangedMessage"         },
   { 0x036c7500, "NISetLedStateMessage"         },
   { 0x02447500, "NIHWSDeviceConnectMessage"    },
   { 0x03447143, "NIRequestSerialNumberMessage" },
   { 0x02444e2b, "NISetSerialNumberMessage"     },
   { 0x03442d00, "NISoftwareShuttingDownMessage"    },
   { 0x02444900, "NIHWSDeviceConnectSerialMessage"  },
   { 0x03434300, "NIMysteryMessage" },
   { 0x03734e00, "NIButtonPressedMessage" },
   { 0x03774e00, "NIBrowseWheelMessage" },
   { 0x03564e66, "NIOctaveChangedMessage" },
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

