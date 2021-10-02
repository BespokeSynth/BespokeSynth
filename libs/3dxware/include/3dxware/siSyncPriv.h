/*----------------------------------------------------------------------
 *  siSyncPriv.h -- 3DxWare GUI Synchronization Private header
 *
 * Written:     June 2005-2013
 * Author:      Jim Wick
 *
 *----------------------------------------------------------------------
 *
 * (c) Copyright 1998-2015 3Dconnexion. All rights reserved. 
 * Permission to use, copy, modify, and distribute this software for all
 * purposes and without fees is hereby granted provided that this copyright
 * notice appears in all copies.  Permission to modify this software is granted
 * and 3Dconnexion will support such modifications only is said modifications are
 * approved by 3Dconnexion.
 *
 */


#ifndef _SISYNCPRIV_H_
#define _SISYNCPRIV_H_


/*
 *  All packets start with the same fields.
 *  Many packets have data following the itemCode.
 */
typedef struct                /* Sync Packet */
   {
   SPWuint32		size;		    /* total packet size */
   SPWuint32		hashCode;		/* Hash code that syncs a question with an answer */
   SiSyncOpCode		opCode;         /* OpCode */
   SiSyncItemCode	itemCode;		/* itemCode */
   /* There will, generally, be more data starting here.
    * There will not be any pointers, the data will be in here.
    */
   } SiSyncPacketHeader;

/*
 * I've enumerated all the possible packets here, not because they are all different,
 * but mostly just for documentation.  So the developer knows what parameters are
 * expected with which packet type.
 */
typedef struct { SiSyncPacketHeader h;									} SiSyncGetVersionPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 major; SPWint32 minor;	} SiSyncSetVersionPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncCommandQueryPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncCommandSaveConfigPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetNumberOfFunctionsPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 n;						} SiSyncSetNumberOfFunctionsPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i;						} SiSyncGetFunctionPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; SPWint32 n; WCHAR name[1];} SiSyncSetFunctionPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i;						} SiSyncGetButtonAssignmentPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; SPWint32 n;			} SiSyncSetButtonAssignmentPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; SPWint32 n;			} SiSyncSetButtonAssignmentAbsolutePacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; WCHAR name[1];		} SiSyncSetButtonNamePacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i;						} SiSyncGetAxisLabelPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; WCHAR name[1];		} SiSyncSetAxisLabelPacket;
typedef struct { SiSyncPacketHeader h; 									} SiSyncGetOrientationPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 a[6];					} SiSyncSetOrientationPacket;
typedef struct { SiSyncPacketHeader h; SiSyncFilter i;					} SiSyncGetFilterPacket;
typedef struct { SiSyncPacketHeader h; SiSyncFilter i; SiSyncFilterValue v;	} SiSyncSetFilterPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetAxesStatePacket;
typedef struct { SiSyncPacketHeader h; SiSyncAxesState a;				} SiSyncSetAxesStatePacket;
typedef struct { SiSyncPacketHeader h; SPWint32 duration; WCHAR s[1];	} SiSyncSetInfoLinePacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleOverallPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleOverallPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleTxPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleTxPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleTyPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleTyPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleTzPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleTzPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleRxPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleRxPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleRyPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleRyPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncGetScaleRzPacket;
typedef struct { SiSyncPacketHeader h; SPWfloat32 v;					} SiSyncSetScaleRzPacket;
typedef struct { SiSyncPacketHeader h; SiSyncAbsFunctionNumber i;		} SiSyncAbsFunctionPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 i; SPWbool state;		} SiSyncSetButtonStatePacket;
typedef struct { SiSyncPacketHeader h; SPWbool bSuspendOrResume;		} SiSyncSuspendFileWritingPacket;
typedef struct { SiSyncPacketHeader h; SPWint32 buttonNumber; SPWbool press; } SiSyncInjectButtonEventPacket;
typedef struct { SiSyncPacketHeader h; SPWbool press; WCHAR actionID[1]; } SiSyncInvokeActionIDPacket;
typedef struct { SiSyncPacketHeader h; WCHAR bankName[1];				} SiSyncCreateButtonBankPacket;
typedef struct { SiSyncPacketHeader h; WCHAR bankName[1];				} SiSyncDeleteButtonBankPacket;
typedef struct { SiSyncPacketHeader h; WCHAR bankName[1];				} SiSyncSetCurrentButtonBankPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncPreviousButtonBankPacket;
typedef struct { SiSyncPacketHeader h;									} SiSyncNextButtonBankPacket;
typedef struct { SiSyncPacketHeader h; WCHAR bankName[1]; SPWuint32 maxNameLen; } SiSyncGetCurrentButtonBankPacket;
typedef struct { SiSyncPacketHeader h; SPWuint32 syncID;				} SiSyncSetGrabSyncIDPacket;

// Turn off "nonstandard extension used : nameless struct/union" warning
#pragma warning ( disable : 4201 )
typedef struct
{
	union 
	{
		SiSyncPacketHeader h;
		SiSyncGetVersionPacket gv;
		SiSyncSetVersionPacket sv;
		SiSyncCommandQueryPacket cq;
		SiSyncCommandSaveConfigPacket cs;
		SiSyncGetNumberOfFunctionsPacket gnf;
		SiSyncSetNumberOfFunctionsPacket snf;
		SiSyncGetFunctionPacket gf;
		SiSyncSetFunctionPacket sf;
		SiSyncGetButtonAssignmentPacket gba;
		SiSyncSetButtonAssignmentPacket sba;
		SiSyncSetButtonAssignmentAbsolutePacket sbaa;
		SiSyncSetButtonNamePacket sbn;
		SiSyncGetAxisLabelPacket ga;
		SiSyncSetAxisLabelPacket sa;
		SiSyncGetOrientationPacket go;
		SiSyncSetOrientationPacket so;
		SiSyncGetFilterPacket gfi;
		SiSyncSetFilterPacket sfi;
		SiSyncGetAxesStatePacket gas;
		SiSyncSetAxesStatePacket sas;
		SiSyncSetInfoLinePacket si;
		SiSyncGetScaleOverallPacket gso;
		SiSyncSetScaleOverallPacket sso;
		SiSyncGetScaleTxPacket gtx;
		SiSyncSetScaleTxPacket stx;
		SiSyncGetScaleTyPacket gty;
		SiSyncSetScaleTyPacket sty;
		SiSyncGetScaleTzPacket gtz;
		SiSyncSetScaleTzPacket stz;
		SiSyncGetScaleRxPacket grx;
		SiSyncSetScaleRxPacket srx;
		SiSyncGetScaleRyPacket gry;
		SiSyncSetScaleRyPacket sry;
		SiSyncGetScaleRzPacket grz;
		SiSyncSetScaleRzPacket srz;
		SiSyncAbsFunctionPacket	absf;
		SiSyncSetButtonStatePacket sbs;
		SiSyncSuspendFileWritingPacket sfw;
		SiSyncInjectButtonEventPacket ibe;
		SiSyncInvokeActionIDPacket ia;
		SiSyncCreateButtonBankPacket cbb;
		SiSyncDeleteButtonBankPacket dbb;
		SiSyncSetCurrentButtonBankPacket scbb;
		SiSyncPreviousButtonBankPacket pbb;
		SiSyncNextButtonBankPacket nbb;
		SiSyncGetCurrentButtonBankPacket gcbb;
		SiSyncSetGrabSyncIDPacket sgsid;
	};
} SiSyncPacket;
// Turn warning back on
#pragma warning ( default : 4201 )


#endif   /* _SI_SYNCPRIV_H_ */
