/*----------------------------------------------------------------------
*  spwerror.h -- 3DxWare function return values
*
*  This file contains all the 3Dconnexion standard error return
*  return values for functions
*
*----------------------------------------------------------------------
*
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
#ifndef _SPWERROR_H_
#define _SPWERROR_H_

static char spwerrorCvsId[]="(C) 1996-2015 3Dconnexion: $Id: spwerror.h 11091 2015-01-09 11:02:45Z jwick $";

enum SpwRetVal            /* Error return values.                           */
{
  SPW_NO_ERROR,           /* No error.                                      */
  SPW_ERROR,              /* Error -- function failed.                      */
  SI_BAD_HANDLE,          /* Invalid 3DxWare handle.                        */
  SI_BAD_ID,              /* Invalid device ID.                             */
  SI_BAD_VALUE,           /* Invalid argument value.                        */
  SI_IS_EVENT,            /* Event is a 3DxWare event.                      */
  SI_SKIP_EVENT,          /* Skip this 3DxWare event.                       */
  SI_NOT_EVENT,           /* Event is not a 3DxWare event.                  */
  SI_NO_DRIVER,           /* 3DxWare driver is not running.                 */
  SI_NO_RESPONSE,         /* 3DxWare driver is not responding.              */
  SI_UNSUPPORTED,         /* The function is unsupported by this version.   */
  SI_UNINITIALIZED,       /* 3DxWare input library is uninitialized.        */
  SI_WRONG_DRIVER,        /* Driver is incorrect for this 3DxWare version.  */
  SI_INTERNAL_ERROR,      /* Internal 3DxWare error.                        */
  SI_BAD_PROTOCOL,        /* The transport protocol is unknown.             */
  SI_OUT_OF_MEMORY,       /* Unable to malloc space required.               */
  SPW_DLL_LOAD_ERROR,     /* Could not load siapp dlls                      */
  SI_NOT_OPEN,            /* 3D mouse device not open                       */
  SI_ITEM_NOT_FOUND,      /* Item not found                                 */
  SI_UNSUPPORTED_DEVICE,  /* The device is not supported                    */
  SI_NOT_ENOUGH_MEMORY,   /* Not enough memory (but not a malloc problem)   */
  SI_SYNC_WRONG_HASHCODE, /* Wrong hash code sent to a Sync function        */
  SI_INCOMPATIBLE_PROTOCOL_MIX  /* Attempt to mix MWM and S80 protocol in invalid way */
};

typedef enum SpwRetVal SpwReturnValue;

#endif   /* _SPWERROR_H_ */
