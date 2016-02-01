/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  This module implements a generic priority queue
 * based on the implementation described in section 4.3 of:
 * "The Algorithm Design Manual", Second Edition, Steven S. Skiena
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BMUXLIB_PQUEUE_H_
#define BMUXLIB_PQUEUE_H_

#include "bstd.h" /* also includes berr, bdbg, etc */

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef struct BMUXlib_P_PriorityQueue *BMUXlib_PriorityQueue_Handle;

/* If A < B, returns -1.
 * If A==B, returns 0
 * If A > B, returns 1 */
typedef int (*BMUXlib_PriorityQueue_Cmp)(
   void *entryA,
   void *entryB
   );

typedef struct BMUXlib_PriorityQueue_CreateSettings
{
   unsigned uiSignature; /* [DO NOT MODIFY] */

   unsigned uiSize;
   unsigned uiEntrySize;
   BMUXlib_PriorityQueue_Cmp fCmp;
} BMUXlib_PriorityQueue_CreateSettings;

void
BMUXlib_PriorityQueue_GetDefaultCreateSettings(
   BMUXlib_PriorityQueue_CreateSettings *pstSettings
   );

BERR_Code
BMUXlib_PriorityQueue_Create(
   BMUXlib_PriorityQueue_Handle *phQueue,
   const BMUXlib_PriorityQueue_CreateSettings *pstSettings
   );

void
BMUXlib_PriorityQueue_Destroy(
   BMUXlib_PriorityQueue_Handle hQueue
   );

BERR_Code
BMUXlib_PriorityQueue_Insert(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   );

bool
BMUXlib_PriorityQueue_PeekMin(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   );

BERR_Code
BMUXlib_PriorityQueue_ExtractMin(
   BMUXlib_PriorityQueue_Handle hQueue,
   void *pEntry
   );

unsigned
BMUXlib_PriorityQueue_GetCount(
   BMUXlib_PriorityQueue_Handle hQueue
   );

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_PQUEUE_H_ */
