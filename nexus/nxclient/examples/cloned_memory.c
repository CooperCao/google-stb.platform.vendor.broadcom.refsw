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
 **************************************************************************/
#include "nxclient.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(cloned_memory);

/**
Run client twice:

nexus nxserver &
cloned_memory &
cloned_memory <token printed from first run>

# kill apps in either order. memory is only freed when both exit.
**/
int main(int argc, char **argv)
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_MemoryBlockTokenHandle token;
    NEXUS_Addr addr;
    NEXUS_Error rc;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    if (argc > 1) {
        token = (NEXUS_MemoryBlockTokenHandle)strtoul(argv[1], NULL, 0);
        BDBG_ASSERT(token);
        block = NEXUS_MemoryBlock_Clone(token);
        if (!block) {
            BDBG_ERR(("unable to clone %p", (void*)token));
            goto done;
        }
        NEXUS_MemoryBlock_LockOffset(block, &addr);
        BDBG_WRN(("cloned block %p: offset " BDBG_UINT64_FMT, (void*)block, BDBG_UINT64_ARG(addr)));
        while (1) BKNI_Sleep(1000);
    }
    else {
        block = NEXUS_MemoryBlock_Allocate(NULL, 1024, 0, NULL);
        NEXUS_MemoryBlock_LockOffset(block, &addr);
        token = NEXUS_MemoryBlock_CreateToken(block);
        BDBG_WRN(("block %p: offset " BDBG_UINT64_FMT ", token %p", (void*)block, BDBG_UINT64_ARG(addr), (void*)token));
        while (1) BKNI_Sleep(1000);
    }

done:
    NxClient_Uninit();
    return 0;
}
