/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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
