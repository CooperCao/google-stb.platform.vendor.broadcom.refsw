/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BMUXLIB_LIST_H_
#define BMUXLIB_LIST_H_

#include "bstd.h" /* also includes berr, bdbg, etc */

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef struct BMUXlib_List_Context *BMUXlib_List_Handle;

typedef enum BMUXlib_List_Type
{
      BMUXlib_List_Type_eFIFO,
      BMUXlib_List_Type_eStack,

      BMUXlib_List_Type_eMax
} BMUXlib_List_Type;

typedef struct BMUXlib_List_CreateSettings
{
      size_t uiCount;
      BMUXlib_List_Type eType;
} BMUXlib_List_CreateSettings;

BERR_Code
BMUXlib_List_GetDefaultCreateSettings(
         BMUXlib_List_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_List_Create(
         BMUXlib_List_Handle *phList,
         const BMUXlib_List_CreateSettings *pstSettings
         );

BERR_Code
BMUXlib_List_Destroy(
         BMUXlib_List_Handle hList
         );

BERR_Code
BMUXlib_List_Reset(
         BMUXlib_List_Handle hList
         );

BERR_Code
BMUXlib_List_Push(
         BMUXlib_List_Handle hList,
         void *pEntry
         );

BERR_Code
BMUXlib_List_Pop(
         BMUXlib_List_Handle hList,
         void **pEntry
         );

BERR_Code
BMUXlib_List_Add(
         BMUXlib_List_Handle hList,
         void *pEntry
         );

BERR_Code
BMUXlib_List_Remove(
         BMUXlib_List_Handle hList,
         void **pEntry
         );

BERR_Code
BMUXlib_List_GetHead(
         BMUXlib_List_Handle hList,
         void **pEntry
         );

BERR_Code
BMUXlib_List_GetNumEntries(
         BMUXlib_List_Handle hList,
         size_t *puiCount
         );

BERR_Code
BMUXlib_List_GetNumFree(
         BMUXlib_List_Handle hList,
         size_t *puiCount
         );

bool BMUXlib_List_IsEmpty(
         BMUXlib_List_Handle hList
         );

bool BMUXlib_List_IsFull(
         BMUXlib_List_Handle hList
         );

BERR_Code
BMUXlib_List_GetEntries(
         BMUXlib_List_Handle hList,
         const void *astEntries0[], /* Can be NULL if uiNumEntries0=0. */
         size_t *puiNumEntries0,
         const void *astEntries1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumEntries1=0. */
         size_t *puiNumEntries1
         );

/* MUXlib Lightweight List */
#include "blst_squeue.h"
#define BMUXLIB_LIST_DEFINE_ENTRY( _type ) \
   typedef struct _type##Entry \
   { \
      BLST_SQ_ENTRY(_type##Entry) link; \
      _type stData; \
   } _type##Entry;
#define BMUXLIB_LIST_DEFINE_HEAD( _type ) BLST_SQ_HEAD(_type##Head, _type##Entry );
#define BMUXLIB_LIST_DEFINE_LIST( _type ) \
   typedef struct _type##List \
   { \
      struct _type##Head stHead; \
      size_t uiCount; \
   } _type##List;

#define BMUXLIB_LIST_DEFINE( _type ) \
      BMUXLIB_LIST_DEFINE_ENTRY( _type ) \
      BMUXLIB_LIST_DEFINE_HEAD( _type ) \
      BMUXLIB_LIST_DEFINE_LIST( _type )

#define BMUXLIB_LIST_TYPE( _type ) _type##List
#define BMUXLIB_LIST_ENTRY_TYPE( _type ) _type##Entry

#define BMUXLIB_LIST_INIT( _plist ) { BLST_SQ_INIT( &(_plist)->stHead ); (_plist)->uiCount = 0; }
#define BMUXLIB_LIST_FIRST( _plist, _ppentry ) { *_ppentry = BLST_SQ_FIRST( &(_plist)->stHead ); }
#define BMUXLIB_LIST_LAST( _plist, _ppentry ) { *_ppentry = BLST_SQ_LAST( &(_plist)->stHead ); }
#define BMUXLIB_LIST_ADD( _plist, _pentry ) { BLST_SQ_INSERT_TAIL( &(_plist)->stHead, _pentry, link ); (_plist)->uiCount++; }
#define BMUXLIB_LIST_REMOVE( _plist, _ppentry ) { BMUXLIB_LIST_FIRST( _plist, _ppentry ); BLST_SQ_REMOVE_HEAD( &(_plist)->stHead, link ); (_plist)->uiCount--; }
#define BMUXLIB_LIST_PUSH( _plist, _pentry ) { BLST_SQ_INSERT_HEAD( &(_plist)->stHead, _pentry, link ); (_plist)->uiCount++; }
#define BMUXLIB_LIST_POP( _plist, _ppentry ) BMUXLIB_LIST_REMOVE( _plist, _ppentry )
#define BMUXLIB_LIST_COUNT( _plist ) ( (_plist)->uiCount )
#define BMUXLIB_LIST_ISEMPTY( _plist )  ( 0 == BMUXLIB_LIST_COUNT( _plist ) )
#define BMUXLIB_LIST_GETNUMENTRIES( _plist, _pcount ) { *(_pcount) = BMUXLIB_LIST_COUNT(_plist); }
#define BMUXLIB_LIST_GETUSAGE( _plist, _ppMinUsage, _ppMaxUsage, _ppSize ) { *(_ppMinUsage) = *(_ppMaxUsage) = 1; *(_ppSize) = 1; }

#define BMUXLIB_LIST_ENTRY_NEXT( _pentry ) BLST_SQ_NEXT( _pentry, link )
#define BMUXLIB_LIST_ENTRY_DATA( _pentry ) (&(_pentry)->stData)
#define BMUXLIB_LIST_ENTRY_METADATA( _pentry ) (&(_pentry)->stMetaData)

#if BDBG_DEBUG_BUILD
/* Max usage can be used to find out how much of the list was used for lists that start out EMPTY
   Min usage can be used to find out how much of the list was used for lists that start out FULL
   (for example use for free lists - i.e. if nothing is used, this will return a min of the count of items in the list)
   Obviously, for lists that start out empty min usage will return 0, and for lists that start out
   full max usage will return the count of items
   uiSize indicates the count of items in the list for use in calculating percentage usage
*/
BERR_Code
BMUXlib_List_GetUsage(
         BMUXlib_List_Handle hList,
         size_t *puiMinUsage,
         size_t *puiMaxUsage,
         size_t *puiSize
         );
#endif
#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_LIST_H_ */
