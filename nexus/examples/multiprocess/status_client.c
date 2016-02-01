/***************************************************************************
 *     (c)2011-2012 Broadcom Corporation
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

#include "nexus_platform.h"
#include "nexus_platform_common.h"
#include "nexus_playpump.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

BDBG_MODULE(status_client);

int main(void)
{
    int rc;
    unsigned i;
    NEXUS_InterfaceName interfaceName;
#define MAX_OBJECTS 16
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
    unsigned num;
    
    rc = NEXUS_Platform_Join();
    if (rc) return -1;

    /* this assumes client is eUnprotected or eVerified */
    strcpy(interfaceName.name, "NEXUS_Playpump");
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    
    if (!num) {
        printf("no playpumps found\n");
    }
    else {
        for (i=0;i<num;i++) {
            NEXUS_PlaypumpHandle playpump = objects[i].object;
            NEXUS_PlaypumpStatus status;
            
            rc = NEXUS_Playpump_GetStatus(playpump, &status);
            BDBG_ASSERT(!rc);
            
            printf(
                "playpump %p\n"
                "started %u\n"
                "fifoDepth %u\n"
                "fifoSize %u\n"
                "descFifoDepth %u\n"
                "descFifoSize %u\n"
                "bufferBase %p\n"
                "bytesPlayed %u\n"
                "index %u\n"
                "pacingTsRangeError %u\n"
                "syncErrors %u\n"
                "resyncEvents %u\n"
                "streamErrors %u\n"
                "mediaPtsType %u\n"
                "mediaPts %#x\n",
                (void*)playpump,
                status.started,
                status.fifoDepth,
                status.fifoSize,
                status.descFifoDepth,
                status.descFifoSize,
                status.bufferBase,
                (unsigned)status.bytesPlayed,
                status.index,
                status.pacingTsRangeError,
                status.syncErrors,
                status.resyncEvents,
                status.streamErrors,
                status.mediaPtsType,
                status.mediaPts);
        }
    }
        
    NEXUS_Platform_Uninit();
    
    return 0;
}
