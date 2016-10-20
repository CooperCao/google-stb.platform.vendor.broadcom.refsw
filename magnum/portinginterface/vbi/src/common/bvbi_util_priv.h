/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/

#ifndef BVBI_UTIL_PRIV_H__
#define BVBI_UTIL_PRIV_H__

#include "bstd.h"
#include "bmma.h"
#include "berr.h"

/******************************************************************************
 * BVBI interface to BMMA. Usage of BMMA is very limited, so required features
 * are encapsulated into this little toolkit. It prevents repetition.
 *****************************************************************************/

typedef struct BVBI_P_MmaData
{
    BMMA_Block_Handle handle;
    uint8_t* pSwAccess;
    BMMA_DeviceOffset pHwAccess;
    /* Programming note: This data structure should have only two states:
     * completely valid, and completely unused. In the latter case, all three
     * fields should be NULL.
     *
     * Therefore, it is sufficient to test the first field against NULL in any
     * software logic. This is the convention that will be followed.
     */
}
BVBI_P_MmaData;

BERR_Code BVBI_P_MmaAlloc
    (BMMA_Heap_Handle hMmaHeap, size_t size, unsigned alignment,
     BVBI_P_MmaData* pMmaData);
void BVBI_P_MmaFree (BVBI_P_MmaData* pMmaData);
void BVBI_P_MmaFlush_isrsafe (BVBI_P_MmaData* pMmaData, size_t size);

/*
 * A software object for accumulating sections of data into a line. Provides
 * double-buffering.
 */

typedef struct BVBI_P_LineBuilder_Handle* BVBI_LineBuilder_Handle;

BERR_Code BVBI_LineBuilder_Open (
    BVBI_LineBuilder_Handle* pHandle,
    BMMA_Heap_Handle hMmaHeap, size_t lineCount, size_t lineSize);

void BVBI_LineBuilder_Close (BVBI_LineBuilder_Handle handle);

BERR_Code BVBI_LineBuilder_Put_isr (
    BVBI_LineBuilder_Handle handle, uint8_t* sectionData, size_t sectionSize,
    size_t sectionOffset, int sequenceNumber, int lineNumber);

BERR_Code BVBI_LineBuilder_Get_isr (
    BVBI_LineBuilder_Handle handle,
    BMMA_DeviceOffset* pLineData, int* pSequenceNumber,
    int* pLineNumber);

#endif /* BVBI_UTIL_PRIV_H__ */
