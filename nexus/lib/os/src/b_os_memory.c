/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: Event
*    Library routines for heap management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "bkni.h"
#include <stdlib.h>

BDBG_MODULE(b_os_memory);
/*****************************************************************************************/
/* By Default, these routines are a simple passthrough to standard malloc/free routines. */
/* However, guardbands and leak detection may be added here to facilitate debugging.     */
/*****************************************************************************************/

/***************************************************************************
Summary:
Allocate Memory (malloc equivalent)
***************************************************************************/
#ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    void *B_Os_Malloc_tagged(
        size_t numBytes,
        const char *pFunction,
        int line
        )
    {
        BSTD_UNUSED(pFunction);
        BSTD_UNUSED(line);
    return malloc(numBytes);
    }
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Free Memory (free equivalent)
***************************************************************************/
#ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    void B_Os_Free_tagged(
        void *pMemory,
        const char *pFunction,
        int line
        )
    {
        BSTD_UNUSED(pFunction);
        BSTD_UNUSED(line);
    free(pMemory);
    }
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Allocate and Clear memory (calloc equivalent)
***************************************************************************/
#ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    void *B_Os_Calloc_tagged(
        size_t numMembers,
        size_t memberSize,
        const char *pFunction,
        int line
        )
    {
        BSTD_UNUSED(pFunction);
        BSTD_UNUSED(line);
        return calloc(numMembers, memberSize);
    }
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */


/***************************************************************************
Summary:
Re-Allocate memory (realloc equivalent)

Note:   B_Os_Realloc_tagged() has been removed, since we don't have a
        way to implement it with the existing BKNI memory functions.
        We need a BKNI_Realloc() function.

***************************************************************************/
#ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI
    void *B_Os_Realloc_tagged(
        void *pMemory,
        size_t numBytes,
        const char *pFunction,
        int line
        )
    {
        BSTD_UNUSED(pFunction);
        BSTD_UNUSED(line);
        return realloc(pMemory, numBytes);
    }
#endif /* #ifdef B_OS_LIB_USE_NATIVE_MALLOC_CALLS_INSTEAD_OF_BKNI */

/***************************************************************************
Summary:
Set a block of memory to a specified value (memset equivalent).
Important: Make sure this can tolerate a null "mem" pointer. Because that
might happen when it gets called by the B_Os_Calloc() macro.
***************************************************************************/
void *B_Os_Memset(void *mem, int ch, size_t n)
{
    if (mem)
    {
        BKNI_Memset(mem, ch, n);
    }
    return mem;
}
