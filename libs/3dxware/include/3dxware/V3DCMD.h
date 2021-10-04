/*----------------------------------------------------------------------
 *  V3DCMD.h -- Virtual 3D Commands
 *
 *  enums for all current V3DCMDs
 *  These are functions that all applications should respond to if they are 
 *  at all applicable.  These cmds are generated from any number of places
 *  including hard and soft buttons.  They don't necessarily represent a
 *  hardware button
 *
 * Written:     January 2013
 * Original author:      Jim Wick
 *
 *----------------------------------------------------------------------
 *
 * Copyright (c) 2013-2015 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 *
 */
#ifndef _V3DCMD_H_
#define _V3DCMD_H_

static char v3DCMDCvsId[]="(C) 2013-2015 3Dconnexion: $Id: V3DCMD.h 11091 2015-01-09 11:02:45Z jwick $";


/*
 * Constants
 */


/*
 * Virtual 3D Commands
 * 
 * These function numbers will never change, but the list will be amended as more
 * V3DCMDs are created.
 * For use with SI_CMD_EVENT.
 * Most of these don't have a separate press and release of these events.  
 * Some keys do have press and release (Esc, Shift, Ctrl) as expected of keyboard keys.
 */
typedef enum
{
   V3DCMD_NOOP			= 0,
   V3DCMD_MENU_OPTIONS	= 1,
   V3DCMD_VIEW_FIT		= 2,
   V3DCMD_VIEW_TOP		= 3,
   V3DCMD_VIEW_LEFT		= 4,
   V3DCMD_VIEW_RIGHT	= 5,
   V3DCMD_VIEW_FRONT	= 6,
   V3DCMD_VIEW_BOTTOM	= 7,
   V3DCMD_VIEW_BACK		= 8,
   V3DCMD_VIEW_ROLLCW	= 9,
   V3DCMD_VIEW_ROLLCCW	= 10,
   V3DCMD_VIEW_ISO1		= 11,
   V3DCMD_VIEW_ISO2		= 12,
   V3DCMD_KEY_F1		= 13,
   V3DCMD_KEY_F2		= 14,
   V3DCMD_KEY_F3		= 15,
   V3DCMD_KEY_F4		= 16,
   V3DCMD_KEY_F5		= 17,
   V3DCMD_KEY_F6		= 18,
   V3DCMD_KEY_F7		= 19,
   V3DCMD_KEY_F8		= 20,
   V3DCMD_KEY_F9		= 21,
   V3DCMD_KEY_F10		= 22,
   V3DCMD_KEY_F11		= 23,
   V3DCMD_KEY_F12		= 24,
   V3DCMD_KEY_ESC		= 25,
   V3DCMD_KEY_ALT		= 26,
   V3DCMD_KEY_SHIFT		= 27,
   V3DCMD_KEY_CTRL		= 28,
   V3DCMD_FILTER_ROTATE	= 29,
   V3DCMD_FILTER_PANZOOM= 30,
   V3DCMD_FILTER_DOMINANT=31,
   V3DCMD_SCALE_PLUS	= 32,
   V3DCMD_SCALE_MINUS	= 33,
   V3DCMD_VIEW_SPINCW	= 34,
   V3DCMD_VIEW_SPINCCW	= 35,
   V3DCMD_VIEW_TILTCW	= 36,
   V3DCMD_VIEW_TILTCCW	= 37,
   V3DCMD_MENU_POPUP	= 38,
   V3DCMD_MENU_BUTTONMAPPINGEDITOR	  = 39,
   V3DCMD_MENU_ADVANCEDSETTINGSEDITOR = 40,
   V3DCMD_MOTIONMACRO_ZOOM = 41,
   V3DCMD_MOTIONMACRO_ZOOMOUT_CURSORTOCENTER = 42,
   V3DCMD_MOTIONMACRO_ZOOMIN_CURSORTOCENTER = 43,
   V3DCMD_MOTIONMACRO_ZOOMOUT_CENTERTOCENTER = 44,
   V3DCMD_MOTIONMACRO_ZOOMIN_CENTERTOCENTER = 45,
   V3DCMD_MOTIONMACRO_ZOOMOUT_CURSORTOCURSOR = 46,
   V3DCMD_MOTIONMACRO_ZOOMIN_CURSORTOCURSOR = 47,
   V3DCMD_VIEW_QZ_IN = 48,
   V3DCMD_VIEW_QZ_OUT = 49,
} V3DCMD;

#endif   /* _V3DCMD_H_ */
