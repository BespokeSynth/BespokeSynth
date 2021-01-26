/*----------------------------------------------------------------------
 *  siSync.h -- 3DxWare GUI Synchronization header
 *
 * Written:     September 2004
 * Author:      Jim Wick
 *
 *----------------------------------------------------------------------
 *
 * Copyright notice:
 * Copyright (c) 1998-2015 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 *
 */
#ifndef _SISYNC_H_
#define _SISYNC_H_

#include "siSyncDefines.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	int state; /* VURZYX (Tx = LSB (& 1<<0) */
} SiSyncAxesState;


/*
 * Private / implementation structures
 * 
 * We suggest you leave these hidden and use the accessor functions rather than
 * directly accessing the structures.
 */
#include "siSyncPriv.h"


/*
 * Accessor Function headers 
 */
SPWuint32 SiSyncGetSize(SiSyncPacket p);
void      SiSyncSetSize(SiSyncPacket *p, SPWuint32 size);

SPWuint32 SiSyncGetHashCode(SiSyncPacket p);
void      SiSyncSetHashCode(SiSyncPacket *p, SPWuint32 hashCode);

SiSyncOpCode SiSyncGetOpCode(SiSyncPacket p);
void         SiSyncSetOpCode(SiSyncPacket *p, SPWuint32 opCode);

SiSyncItemCode SiSyncGetItemCode(SiSyncPacket p);
void           SiSyncSetItemCode(SiSyncPacket *p, SPWuint32 itemCode);

#ifdef __cplusplus
}
#endif

#endif   /* _SI_SYNC_H_ */
