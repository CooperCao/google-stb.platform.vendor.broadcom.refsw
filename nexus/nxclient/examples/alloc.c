/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_memory.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(alloc);

int main(int argc, char **argv)
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_ClientConfiguration clientConfig;
    int rc;
    NEXUS_HeapHandle heap;
    int curarg = 1;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    while (curarg < argc) {
        int size = atoi(argv[curarg++]);
        heap = clientConfig.heap[NXCLIENT_DYNAMIC_HEAP] ? clientConfig.heap[NXCLIENT_DYNAMIC_HEAP] : NULL;
        block = NEXUS_MemoryBlock_Allocate(heap, size * 1024 * 1024, 0, NULL);
        if (!block && heap) {
            rc = NxClient_GrowHeap(NXCLIENT_DYNAMIC_HEAP);
            if (rc) return BERR_TRACE(rc);
            block = NEXUS_MemoryBlock_Allocate(heap, size * 1024 * 1024, 0, NULL);
        }

        if (curarg > 2) printf(", ");
        if (block) {
            NEXUS_Error rc;
            void *buffer;

            rc = NEXUS_MemoryBlock_Lock(block, &buffer);
            BDBG_ASSERT(rc==NEXUS_SUCCESS);
            printf("alloc %d MB at %p\n", size, buffer);
            BKNI_Memset(buffer, 0, size*1024*1024);
            NEXUS_MemoryBlock_Unlock(block);
        }
        else {
            printf("FAIL %d MB\n", size);
            rc = -1;
        }
    }

    if (!rc) {
        /* keep the allocation until ctrl-c */
        while (1) BKNI_Sleep(1000);
    }
    BERR_TRACE(rc);
    return rc;
}
