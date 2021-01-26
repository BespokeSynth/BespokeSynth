/*----------------------------------------------------------------------
 *  si.h -- 3DxWare input library header
 *
 *  3DxWare Input Library
 *
 *----------------------------------------------------------------------
 *
 * Copyright notice:
 * Copyright (c) 1996-2015 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 *
 */
#ifndef _SI_H_
#define _SI_H_

static char incFileNameCvsId[]="(C) 1996-2015 3Dconnexion: $Id: si.h 11391 2015-05-06 15:55:57Z ngomes $";

#ifndef __cplusplus
#ifdef _WIN32
#include <windows.h>
#endif
#endif

// This fixes up structure packing differences between x64 and x86
#pragma pack(push, 4)

#include "spwmacro.h"
#include "spwdata.h"
#include "siSync.h"
#include "V3DKey.h"
#include "V3DCMD.h"

#include "spwerror.h"

#include "siDefines.h"

typedef int SiDevID;          /* Device ID */
typedef void *SiHdl;          /* 3DxWare handle */
typedef void *SiTransCtl;     /* 3DxWare transport control handle */

#ifdef _WIN32
typedef struct                /* Open data. Drive use only.  */
{
  HWND hWnd;               /* Window handle for 3DxWare messages.        */
  SiTransCtl transCtl;     /* 3DxWare transport control handle. Reserved */
                           /* for the s80 transport mechanism.             */
  DWORD processID;         /* The process ID for this application.         */
  char exeFile[MAX_PATH];  /* The executable name of the process.          */
  SPWint32 libFlag;        /* Library version flag.                        */
} SiOpenData;
#endif

#ifdef _WIN32
typedef struct                /* Get event Data. Drive use only. */
{
  UINT msg;
  WPARAM wParam;
  LPARAM lParam;
} SiGetEventData;
#endif

typedef struct                /* Device type mask */
{
  unsigned char mask[8];
} SiTypeMask;

typedef struct                /* Device port information */
{
  SiDevID devID;             /* Device ID */
  int devType;               /* Device type */
  int devClass;              /* Device class */
  char devName[SI_STRSIZE];  /* Device name */
  char portName[SI_MAXPORTNAME]; /* Port name */
} SiDevPort;

typedef struct                /* Device information */
{
  char firmware[SI_STRSIZE]; /* Firmware version */
  int devType;               /* Device type */
  int numButtons;            /* Number of buttons */
  int numDegrees;            /* Number of degrees of freedom */
  SPWbool canBeep;           /* Device beeps */
  int majorVersion;          /* Major version number */
  int minorVersion;          /* Minor version number */
} SiDevInfo;

typedef struct                /* Button information */
{
  char name[SI_STRSIZE];	  /* Contains the name of a button for display in an app's GUI */
} SiButtonName;

typedef struct                /* Button information */
{
  char name[SI_STRSIZE];	  /* Contains the name of a device for display in an app's GUI */
} SiDeviceName;

typedef struct                /* Port information */
{
  char name[SI_MAXPATH];	  /* The name of a port the device is located on */
} SiPortName;


/* 
 * An SiAppCmdID (arbitrary ID of some application function) is used in SI_APP_EVENTs.  
 * It is just passed back and forth from/to apps.
 * We have no idea what it is.  Only the app knows.
 */
typedef struct
{
  char appCmdID[SI_MAXAPPCMDID];	  /* The AppCmdID from the AppCommand button action */
} SiAppCmdID;

typedef struct                /* Version information */
{
  int major;                 /* Major version number */
  int minor;                 /* Minor version number */
  int build;                 /* Build number */
  char version[SI_STRSIZE];  /* Version string */
  char date[SI_STRSIZE];     /* Date string */
} SiVerInfo;

typedef struct                /* Sensitivity parameters */
{
  char dummy;
} SiSensitivity;

typedef struct                /* Tuning parameters */
{
  char dummy;
} SiTuning;

// Turn off "nonstandard extension used : nameless struct/union" warning
#pragma warning ( disable : 4201 )

typedef struct
{
  SPWuint8 code;                 /* Out of band message code */
  union {
    SPWuint8 message[SI_MAXBUF-1];  /* The actual message       */
    SPWint32 messageAsLongs[SI_MAXBUF/4]; /* Access for longs/DWORDs  */
    void    *pvoid[SI_MAXBUF/8];    /* void ptrs.  Enough room for 64bit ptrs */
  };
} SiSpwOOB;

typedef struct
{
  union {
    SPWuint8 string[SI_KEY_MAXBUF];  /* No longer used, but it establishes the total size of SiSpwEvent, so keep it around in case anyone cares about the old size. */
    struct {
      SiKeyboardEventType type;
      union {
        struct keyData { int VirtualKeyCode; int ScanCode; } keyData; // Data for KeyPress and KeyRelease SiKeyboardEventType
      };
    } keyboardEvent;
  };
} SiKeyboardData;

// Turn warning back on
#pragma warning ( default : 4201 )

typedef struct                /* Bitmasks of button states */
{
  SPWuint32 last;            /* Buttons pressed as of last event */
  SPWuint32 current;         /* Buttons pressed as of this event */
  SPWuint32 pressed;         /* Buttons pressed this event */
  SPWuint32 released;        /* Buttons released this event */
} SiButtonData;

/*
 * SI_BUTTON_PRESS_EVENT & SI_BUTTON_RELEASE_EVENT are hardware button 
 * events.  Meaning that they are meant to be sent when a specific hardware
 * button is pressed.  The correlation between the actual hardware button
 * and the resulting button number could be broken by careful editing of
 * a config file, but it is intended that the correlation be intact.
 * This is basically the same as SI_BUTTON_EVENT, but allows
 * more than 29 buttons because it isn't limited to a 32-bit mask.
 * For buttons <= 31, both SI_BUTTON_EVENTs and SI_BUTTON_PRESS/RELEASE_EVENTs are sent.
 * In the future there may be a way to switch off one or the other.
 * This event was introduced in 3DxWare driver v. 5.2, but not implemented
 * until 3DxWare 10 (3DxWinCore 17 r8207).
 */
typedef struct				  /* Data for SI_BUTTON_PRESS/RELEASE_EVENT */
{
  V3DKey buttonNumber;	  /* The V3DKey that went down/up in a   *
                           * SI_BUTTON_PRESS/RELEASE_EVENT event */
} SiHWButtonData;

typedef struct				  /* Data for SI_APP_EVENT */
{
  SPWbool pressed; /* SPW_TRUE if the invoking button pressed, SPW_FALSE otherwise */
  SiAppCmdID id;   /* The Application-specific function identifier *
                    * invoked by the user in a SI_APP_EVENT.  
                    * The id is last so we can optimize the use of transport memory. */
} SiAppCommandData;

typedef struct				  /* Data for SI_CMD_EVENT */
{
  SPWbool pressed;			    /* SPW_TRUE if the invoking button pressed, SPW_FALSE otherwise */
  SPWuint32 functionNumber; /* The V3DCMD_ function number invoked by the *
                             * user in a SI_CMD_EVENT (see V3DCMD.h) */
  SPWint32 iArgs[16];		    /* Optional arguments on a V3DCMD_ basis */
  SPWfloat32 fArgs[16];
} SiCmdEventData;

typedef struct				  /* Data for SI_DEVICE_CHANGE_EVENT */
{
  SiDeviceChangeType type;  /* The type of event that happened */
  SiDevID devID;			      /* The device ID effected */
  SiPortName portName;      /* The device path that changed */
} SiDeviceChangeEventData;

typedef struct          /* 3DxWare data */
{
  SiButtonData bData;       /* Button data */
  long mData[6];            /* Motion data (index via SI_TX, etc) */
  long period;              /* Period (milliseconds) */
} SiSpwData;

typedef struct          /* 3DxWare event */
{
  int type;                 /* Event type */
  union
  {
    SiSpwData spwData;            /* Button, motion, or combo data        */
    SiSpwOOB spwOOB;              /* Out of band message                  */
    SiOrientation spwOrientation; /* Which hand orientation is the device */
    char exData[SI_MAXBUF];       /* Exception data. Driver use only      */
    SiKeyboardData spwKeyData;    /* String for keyboard data             */
    SiSyncPacket siSyncPacket;    /* GUI SyncPacket sent to applications  */
    SiHWButtonData hwButtonEvent; /* V3DKey that goes with          *
                                   * SI_BUTTON_PRESS/RELEASE_EVENT        */
    SiAppCommandData appCommandData;  /* Application command event function data that *
                                   * goes with an SI_APP_EVENT event      */
    SiDeviceChangeEventData deviceChangeEventData;    /* Data for connecting/disconnecting devices */
    SiCmdEventData cmdEventData;  /* V3DCMD_* function data that *
                                   * goes with an SI_CMD_EVENT event      */
  } u;
} SiSpwEvent;

#ifdef _WIN32
typedef struct                /* Event handler (for SiDispatch) */
{
  int (*func) (SiOpenData *, SiGetEventData *, SiSpwEvent *, void *);
  void *data;
} SiEventHandler;
#endif

#ifdef _WIN32
typedef struct                /* 3DxWare event handlers */
{
  SiEventHandler button;     /* Button event handler */
  SiEventHandler motion;     /* Motion event handler */
  SiEventHandler combo;      /* Combo event handler */
  SiEventHandler zero;       /* Zero event handler */
  SiEventHandler exception;  /* Exception event handler */
} SiSpwHandlers;
#endif

// Reset packing to default so don't effect including file
#pragma pack(pop)


#endif   /* _SI_H_ */
