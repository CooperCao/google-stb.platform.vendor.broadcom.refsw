/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(heaps);

static void print_usage(void)
{
    printf(
    "Usage: heaps OPTIONS\n"
    "\n"
    "Prints status of all Nexus heaps for client\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -server   print all heaps in the system (by server index)\n"
    );
}

static void print_heap(unsigned i, NEXUS_HeapHandle heap, unsigned otherIndex)
{
    NEXUS_MemoryStatus status;
    char buf[128];
    unsigned size_percentage;
    NEXUS_Error rc;
    char otherIndexStr[8];
    if (otherIndex == NEXUS_MAX_HEAPS) {
        otherIndexStr[0] = 0;
        /* avoid NEXUS_OffsetToCachedAddr on non-client heap, which prints an error. */
        rc = NEXUS_Heap_GetStatus_driver(heap, &status);
        if (rc) return;
    }
    else {
        snprintf(otherIndexStr, sizeof(otherIndexStr), "%u", otherIndex);
        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc) return;
    }
    size_percentage = status.size/100; /* avoid overflow by dividing size by 100 to get percentage */
    NEXUS_Heap_ToString(&status, buf, sizeof(buf));
    printf("%-2d " BDBG_UINT64_FMT " %d 0x%-8x %3d %08lx %3d%% %3d%% %-16s %s\n",
           i, BDBG_UINT64_ARG(status.offset), status.memcIndex, status.size, status.size/(1024*1024), (unsigned long)status.addr,
        size_percentage?(status.size-status.free)/size_percentage:0,
        size_percentage?status.highWatermark/size_percentage:0,
        buf, otherIndexStr);
}

static void print_heaps(bool server)
{
    unsigned i, otherIndex;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    printf("heap offset memc size        MB vaddr      used peak name        %s\n", server?"client":"server");
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (server) {
            for (otherIndex=0;otherIndex<NEXUS_MAX_HEAPS;otherIndex++) {
                if (clientConfig.heap[otherIndex] == platformConfig.heap[i]) break;
            }
            print_heap(i, platformConfig.heap[i], otherIndex);
        }
        else {
            if (clientConfig.heap[i]) {
                for (otherIndex=0;otherIndex<NEXUS_MAX_HEAPS;otherIndex++) {
                    if (platformConfig.heap[otherIndex] == clientConfig.heap[i]) break;
                }
                print_heap(i, clientConfig.heap[i], otherIndex);
            }
        }
    }
}


int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    int curarg = 1;
    bool server = false;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-server")) {
            server = true;
        }
        curarg++;
    }

    print_heaps(server);

    NxClient_Uninit();
    return 0;
}
