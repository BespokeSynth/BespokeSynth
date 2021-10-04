/*----------------------------------------------------------------------
 *  V3DKey.h -- Virtual 3D Keys
 *
 *  enums for all current V3DKeys
 *
 * Written:     January 2013
 * Original author:      Jim Wick
 *
 *----------------------------------------------------------------------
 *
 * Copyright notice:
 * Copyright (c) 2013-2014 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 *
 */
#ifndef _V3DKey_H_
#define _V3DKey_H_

static char v3DKeyCvsId[]="(C) 2013-2015 3Dconnexion: $Id: V3DKey.h 11091 2015-01-09 11:02:45Z jwick $";

/*
 * Virtual 3D Keys
 *
 * Functions that refer to hardware keys use these constants to identify the keys.
 * These represent hardware buttons on devices that have them.
 * If a hardware device doesn't have a key, it won't produce the event.
 * Not all hardware devices have all keys but any key that does exist is
 * in this enum.  If new hardware keys are added in the future, a new constant will
 * be created for it.
 * 
 * SI_BUTTON_PRESS_EVENT and SI_BUTTON_RELEASE_EVENT events identify the hardware
 * keys that generated the events through V3DKeys (Virtual 3D Mouse Keys).
 * These will represent the actual hardware key pressed unless the user did 
 * something clever in his xml file.
 */
typedef enum
{
   V3DK_MENU		= 1,
   V3DK_FIT			= 2,
   V3DK_TOP			= 3,
   V3DK_LEFT		= 4,
   V3DK_RIGHT		= 5,
   V3DK_FRONT		= 6,
   V3DK_BOTTOM		= 7,
   V3DK_BACK		= 8,
   V3DK_ROLL_CW		= 9,
   V3DK_ROLL_CCW	= 10,
   V3DK_ISO1		= 11,
   V3DK_ISO2		= 12,
   V3DK_1			= 13,
   V3DK_2			= 14,
   V3DK_3			= 15,
   V3DK_4			= 16,
   V3DK_5			= 17,
   V3DK_6			= 18,
   V3DK_7			= 19,
   V3DK_8			= 20,
   V3DK_9			= 21,
   V3DK_10			= 22,
   V3DK_ESC			= 23,
   V3DK_ALT			= 24,
   V3DK_SHIFT		= 25,
   V3DK_CTRL		= 26,
   V3DK_ROTATE		= 27,
   V3DK_PANZOOM		= 28,
   V3DK_DOMINANT	= 29,
   V3DK_PLUS		= 30,
   V3DK_MINUS		= 31,
   V3DK_SPIN_CW		= 32,
   V3DK_SPIN_CCW	= 33,
   V3DK_TILT_CW		= 34,
   V3DK_TILT_CCW	= 35,
   V3DK_ENTER		= 36,
   V3DK_DEL		= 37,
   V3DK_RESERVED0		= 38,
   V3DK_RESERVED1		= 39,
   V3DK_RESERVED2		= 40,
   V3DK_F1		= 41,
   V3DK_F2		= 42,
   V3DK_F3		= 43,
   V3DK_F4		= 44,
   V3DK_F5		= 45,
   V3DK_F6		= 46,
   V3DK_F7		= 47,
   V3DK_F8		= 48,
   V3DK_F9		= 49,
   V3DK_F10		= 50,
   V3DK_F11		= 51,
   V3DK_F12		= 52,
   V3DK_F13		= 53,
   V3DK_F14		= 54,
   V3DK_F15		= 55,
   V3DK_F16		= 56,
   V3DK_F17		= 57,
   V3DK_F18		= 58,
   V3DK_F19		= 59,
   V3DK_F20		= 60,
   V3DK_F21		= 61,
   V3DK_F22		= 62,
   V3DK_F23		= 63,
   V3DK_F24		= 64
} V3DKey;


#endif   /* _V3DKey_H_ */
