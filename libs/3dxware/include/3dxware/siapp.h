/*-----------------------------------------------------------------------------
 *
 * siapp.h -- Si static library interface header file
 *
 * Contains function headers and type definitions for siapp.c.
 *
 *-----------------------------------------------------------------------------
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
#ifndef SIAPP_H
#define SIAPP_H

static char SiAppCvsId[]="(C) 1998-2015 3Dconnexion: $Id: siapp.h 11091 2015-01-09 11:02:45Z jwick $";

#ifdef __cplusplus
extern "C" {
#endif



/* externally used functions */

enum SpwRetVal SiInitialize(void);
SPWbool        SiIsInitialized(void);
void           SiTerminate(void);
int            SiGetNumDevices (void);
SiDevID        SiDeviceIndex (int idx);
int            SiDispatch (SiHdl hdl, const SiGetEventData *pData, const SiSpwEvent *pEvent, const SiSpwHandlers *pDHandlers);
void           SiOpenWinInit (SiOpenData *pData, HWND hWnd);
SiHdl          SiOpen (const char *pAppName, SiDevID devID, const SiTypeMask *pTMask, int mode, const SiOpenData *pData);
SiHdl          SiOpenPort (const char *pAppName, const SiDevPort *pPort, int mode, const SiOpenData *pData);
enum SpwRetVal SiClose (SiHdl hdl);
void           SiGetEventWinInit (SiGetEventData *pData, UINT msg, WPARAM wParam, LPARAM lParam);
enum SpwRetVal SiGetEvent (SiHdl hdl, int flags, const SiGetEventData *pData, SiSpwEvent *pEvent);
enum SpwRetVal SiPeekEvent (SiHdl hdl, int flags, const SiGetEventData *pData, SiSpwEvent *pEvent);
enum SpwRetVal SiBeep (SiHdl hdl, const char *string);
enum SpwRetVal SiSetLEDs (SiHdl hdl, SPWuint32 mask);
enum SpwRetVal SiRezero (SiHdl hdl);
enum SpwRetVal SiGrabDevice (SiHdl hdl, SPWbool exclusive);
enum SpwRetVal SiReleaseDevice (SiHdl hdl);
int            SiButtonPressed (const SiSpwEvent *pEvent);
int            SiButtonReleased (const SiSpwEvent *pEvent);
enum SpwRetVal SiSetUiMode (SiHdl hdl, SPWuint32 mode);
enum SpwRetVal SiSetTypeMask (SiTypeMask *pTMask, int type1, ...);
enum           SpwRetVal SiGetDevicePort (SiHdl hdl, SiDevPort *pPort);
enum           SpwRetVal SiGetDriverInfo (SiVerInfo *pInfo);
void           SiGetLibraryInfo (SiVerInfo *pInfo);
enum           SpwRetVal SiGetDeviceInfo (SiHdl hdl, SiDevInfo *pInfo);
char * SpwErrorString (enum SpwRetVal val);
enum SpwRetVal SiSyncSendQuery(SiHdl hdl);
enum SpwRetVal SiSyncGetVersion(SiHdl hdl, SPWuint32 *pmajor, SPWuint32 *pminor);
enum SpwRetVal SiSyncGetNumberOfFunctions(SiHdl hdl, SPWuint32 *pnumberOfFunctions);
enum SpwRetVal SiSyncGetFunction(SiHdl hdl, SPWuint32 index, SPWint32 *pabsoluteFunctionNumber, WCHAR name[], SPWuint32 *pmaxNameLen);
enum SpwRetVal SiSyncGetButtonAssignment(SiHdl hdl, SPWuint32 buttonNumber, SPWint32 *passignedFunctionIndex);
enum SpwRetVal SiSyncSetButtonAssignment(SiHdl hdl, SPWuint32 buttonNumber, SPWint32 functionIndex);
enum SpwRetVal SiSyncSetButtonAssignmentAbsolute(SiHdl hdl, SPWuint32 buttonNumber, SPWint32 absoluteFunctionNumber );
enum SpwRetVal SiSyncSetButtonName(SiHdl hdl, SPWuint32 buttonNumber, const WCHAR name[]);
enum SpwRetVal SiSyncGetAxisLabel (SiHdl hdl, SPWuint32 axisNumber, WCHAR name[], SPWuint32 *pmaxNameLen );
enum SpwRetVal SiSyncSetAxisLabel (SiHdl hdl, SPWuint32 axisNumber, const WCHAR name[] );
enum SpwRetVal SiSyncGetOrientation (SiHdl hdl, SPWint32 axes[6] );
enum SpwRetVal SiSyncSetOrientation (SiHdl hdl, const SPWint32 axes[6] );
enum SpwRetVal SiSyncGetFilter (SiHdl hdl, SiSyncFilter i, SiSyncFilterValue *pv );
enum SpwRetVal SiSyncSetFilter (SiHdl hdl, SiSyncFilter i, SiSyncFilterValue v );
enum SpwRetVal SiSyncGetAxesState (SiHdl hdl, SiSyncAxesState *pa );
enum SpwRetVal SiSyncSetAxesState (SiHdl hdl, SiSyncAxesState a );
enum SpwRetVal SiSyncSetInfoLine (SiHdl hdl, SPWint32 duration, const WCHAR text[] );
enum SpwRetVal SiSyncGetScaleOverall (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleOverall (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleTx (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleTx (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleTy (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleTy (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleTz (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleTz (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleRx (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleRx (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleRy (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleRy (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncGetScaleRz (SiHdl hdl, SPWfloat32 *pv );
enum SpwRetVal SiSyncSetScaleRz (SiHdl hdl, SPWfloat32 v );
enum SpwRetVal SiSyncInvokeAbsoluteFunction (SiHdl hdl, SiSyncAbsFunctionNumber i );
enum SpwRetVal SiSyncSetButtonState (SiHdl hdl, SPWuint32 buttonNumber, SiSyncButtonState state );
enum SpwRetVal SiGetButtonName (SiHdl hdl, SPWuint32 buttonNumber, SiButtonName *pname);
enum SpwRetVal SiGetButtonV3DK (SiHdl hdl, SPWuint32 buttonNumber, SPWuint32 *pV3DK);
enum SpwRetVal SiGetButtonNameV3DK (SiHdl hdl, SPWuint32 V3DK, SiButtonName *pname);
enum SpwRetVal SiGetDeviceName (SiHdl hdl, SiDeviceName *pname);
enum SpwRetVal SiGetDeviceImageFileName (SiHdl hdl, char name[], SPWuint32 *pmaxNameLen);
HICON SiGetCompanyIcon(void);
enum SpwRetVal SiGetCompanyLogoFileName (char name[], SPWuint32 *pmaxNameLen);
enum SpwRetVal SiSyncSuspendFileWriting (SiHdl hdl);
enum SpwRetVal SiSyncResumeFileWriting (SiHdl hdl);
void *SiGetConnectionID(SiHdl hdl);
enum SpwRetVal SiSyncInvokeActionID (SPWuint32 hashCode, SPWbool press, const WCHAR actionID[]);
enum SpwRetVal SiSyncCreateButtonBank (SiHdl hdl, const WCHAR bankName[]);
enum SpwRetVal SiSyncDeleteButtonBank (SiHdl hdl, const WCHAR bankName[]);
enum SpwRetVal SiSyncSetCurrentButtonBank (SiHdl hdl, const WCHAR bankName[]);
enum SpwRetVal SiSyncPreviousButtonBank (SiHdl hdl);
enum SpwRetVal SiSyncNextButtonBank (SiHdl hdl);
enum SpwRetVal SiSyncGetCurrentButtonBank (SiHdl hdl, WCHAR bankName[], SPWuint32 *pmaxBankNameLen);
enum SpwRetVal SiSyncSetGrabSyncID (SiHdl hdl, SPWuint32 syncID );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SIAPP_H */
