/****************************************************************************
*
*  Copyright (c) 2010 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or
*  its licensors, and may only be used, duplicated, modified or distributed
*  pursuant to the terms and conditions of a separate, written license
*  agreement executed between you and Broadcom (an "Authorized License").
*  Except as set forth in an Authorized License, Broadcom grants no license
*  (express or implied), right to use, or waiver of any kind with respect to
*  the Software, and Broadcom expressly reserves all rights in and to the
*  Software and all intellectual property rights therein.  IF YOU HAVE NO
*  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
*  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
*  SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************
*
*  Filename: bcm7xxxPciEnet.c
*
****************************************************************************
*  Description:	This is Linux network driver for the BCM
* 				BroadBand Modems that interface with 7xxx via PCI or SCB
*               For example, 3368/7038, 3255/7401 and 7118.
*
*  Updates: 10/10/2005  wfeng.  Created.
*           09/18/2006  wfeng.  Added 7118 support.
****************************************************************************/
//
// bcmtypes.h - misc useful typedefs
//
#ifndef BCMTYPES_H
#define BCMTYPES_H

// These are also defined in typedefs.h in the application area, so I need to
// protect against re-definition.

#ifndef _TYPEDEFS_H_
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
typedef signed char     int8;
typedef signed short    int16;
typedef signed long     int32;
#endif

typedef unsigned char   byte;
typedef unsigned long   sem_t;

typedef unsigned long   HANDLE,*PULONG,DWORD,*PDWORD;
typedef signed long     LONG,*PLONG;

typedef unsigned int    *PUINT;
typedef signed int      INT;

typedef unsigned short  *PUSHORT;
typedef signed short    SHORT,*PSHORT,WORD,*PWORD;

typedef unsigned char   *PUCHAR;
typedef signed char     *PCHAR;

typedef void            *PVOID;

typedef unsigned char   BOOLEAN, *PBOOL, *PBOOLEAN;

typedef unsigned char   BYTE,*PBYTE;

//#ifndef __GNUC__
//The following has been defined in Vxworks internally: vxTypesOld.h
//redefine under vxworks will cause error
typedef signed int      *PINT;

typedef signed char     INT8;
typedef signed short    INT16;
typedef signed long     INT32;

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;

typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;

typedef void            VOID;
typedef unsigned char   BOOL;

/* typedef unsigned long  bool; */

//#endif  /* __GNUC__ */


// These are also defined in typedefs.h in the application area, so I need to
// protect against re-definition.
#ifndef TYPEDEFS_H

#define MAX_INT16 32767
#define MIN_INT16 -32768

// Useful for true/false return values.  This uses the
// Taligent notation (k for constant).
typedef enum
{
    kFalse = 0,
    kTrue = 1
} Bool;

#endif

/* macros to protect against unaligned accesses */

#if 0
/* first arg is an address, second is a value */
#define PUT16( a, d ) { 		\
  *((byte *)a) = (byte)((d)>>8); 	\
  *(((byte *)a)+1) = (byte)(d); 	\
}

#define PUT32( a, d ) { 		\
  *((byte *)a) = (byte)((d)>>24); 	\
  *(((byte *)a)+1) = (byte)((d)>>16); 	\
  *(((byte *)a)+2) = (byte)((d)>>8); 	\
  *(((byte *)a)+3) = (byte)(d); 	\
}

/* first arg is an address, returns a value */
#define GET16( a ) ( 			\
  (*((byte *)a) << 8) |			\
  (*(((byte *)a)+1))	 		\
)

#define GET32( a ) ( 			\
  (*((byte *)a) << 24)     |		\
  (*(((byte *)a)+1) << 16) | 		\
  (*(((byte *)a)+2) << 8)  | 		\
  (*(((byte *)a)+3))	 		\
)
#endif

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO  0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#endif
