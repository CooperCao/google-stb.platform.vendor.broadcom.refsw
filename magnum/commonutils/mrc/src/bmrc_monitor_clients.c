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
 * Implementation of the Realtime Memory Monitor for 7038
 *
 ***************************************************************************/

#include "bstd.h"
#include "bmrc.h"
#include "bkni.h"
#include "bmrc_monitor.h"
#include "bmrc_monitor_priv.h"
#include "bmrc_clienttable_priv.h"

BDBG_MODULE(BMRC_monitor_clients);
BDBG_FILE_MODULE(BMRC_MONITOR);
BDBG_FILE_MODULE(BMRC_MONITOR_PRINT);

#define BDBG_MSG_TRACE(x) /* BDBG_LOG(x) */

static const char hwBlockNames[BMRC_Monitor_HwBlock_eMax+1][12]={
#define BCHP_P_MEMC_DEFINE_HWBLOCK(block) #block,
#include "memc/bchp_memc_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_HWBLOCK
    "Invalid"
};


/*
 * HW blocks internally used by system (for example used by OS like USB, or internal to CPU) and could access _any_ OS or device memory location
 * Any other blocks could only access memory that was explicitly allocated (for example other blocks wouldn't be able to access memory from OS heap)
 */
static const uint8_t privilegedClients[] = {
    /* Internal */
    BMRC_Monitor_HwBlock_eBSP,
    BMRC_Monitor_HwBlock_eCPU,
    BMRC_Monitor_HwBlock_eMEMC,

    /* OS */
    BMRC_Monitor_HwBlock_eEBI,
    BMRC_Monitor_HwBlock_eETHERNET,
    BMRC_Monitor_HwBlock_eFLASH,
    BMRC_Monitor_HwBlock_eIEEE_1394,
    BMRC_Monitor_HwBlock_eMOCA,
    BMRC_Monitor_HwBlock_ePCI,
    BMRC_Monitor_HwBlock_eSATA,
    BMRC_Monitor_HwBlock_eSDIO,
    BMRC_Monitor_HwBlock_eUART,
    BMRC_Monitor_HwBlock_eUBUS,
    BMRC_Monitor_HwBlock_eUSB,
    BMRC_Monitor_HwBlock_eWIFI,
#if BMRC_ALLOW_GFX_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eM2MC,
#endif
#if BMRC_ALLOW_XPT_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eXPT,
#endif
#if BMRC_ALLOW_M2M_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eM2M,
#endif

    /* Decoder/Encoder Predicted Fetch Request - reading outside of programmed range */
    BMRC_Monitor_HwBlock_ePREFETCH,

    /* other */
    BMRC_Monitor_HwBlock_eTRACE,
    BMRC_Monitor_HwBlock_eMSA,
    BMRC_Monitor_HwBlock_eSCPU,

    /* unknown */
    BMRC_Monitor_HwBlock_eGFAP,
    BMRC_Monitor_HwBlock_eMCARD,
    BMRC_Monitor_HwBlock_eMCP,
    BMRC_Monitor_HwBlock_eFRONTEND,
    BMRC_Monitor_HwBlock_eDOCSIS,
    BMRC_Monitor_HwBlock_eT2_TDI,
    BMRC_Monitor_HwBlock_eRESERVED,
    /* last */
    BMRC_Monitor_HwBlock_eInvalid
};

/*
 * HW blocks, that could access _any_ allocated device memory, but not OS memory
 */
static const uint8_t controlledClients[] = {
    BMRC_Monitor_HwBlock_eBVN,
#if !BMRC_ALLOW_GFX_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eM2MC,
#endif
#if !BMRC_ALLOW_M2M_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eM2M,
#endif
    BMRC_Monitor_HwBlock_e3D,
    BMRC_Monitor_HwBlock_eSID,
    BMRC_Monitor_HwBlock_eTPCAP,
#if !BMRC_ALLOW_XPT_TO_ACCESS_KERNEL
    BMRC_Monitor_HwBlock_eXPT,
#endif
    /* last */
    BMRC_Monitor_HwBlock_eInvalid
};

/*
 * other clients, could access only memory that is allocated inside particular SW module.
 * Below is a map that translates SW module to a list of clients.
 */

typedef struct BMRC_P_Monitor_FileInfo  {
    char swModule[8];
    uint8_t hwBlocks[8];
} BMRC_P_Monitor_FileInfo;


static const BMRC_P_Monitor_FileInfo fileMap[] = {
    {"baud", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bxpt_ra", {
        BMRC_Monitor_HwBlock_eAVD,
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eXPT,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bxvd", {
        BMRC_Monitor_HwBlock_eAVD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"brap", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bvdc", {
        BMRC_Monitor_HwBlock_eBVN,
        BMRC_Monitor_HwBlock_eVEC,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bvbi", {
        BMRC_Monitor_HwBlock_eVEC_VBI,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bxpt", {
        BMRC_Monitor_HwBlock_eXPT,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bvce", {
        BMRC_Monitor_HwBlock_eVICE,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bsid", {
        BMRC_Monitor_HwBlock_eSID,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bape", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bdsp", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bvde", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }},
    {"bvee", {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eInvalid }}
};



void  BMRC_Monitor_P_MapInit(struct BMRC_P_MonitorClientMap *map)
{
    bool clients[BMRC_Monitor_HwBlock_eMax];
    bool fileBlocks[BMRC_Monitor_HwBlock_eMax];
    unsigned i;
    unsigned unmapped;
    unsigned length;
    static const struct BMRC_P_ClientMapEntry rawMap[] = {
#define BCHP_P_MEMC_DEFINE_CLIENT_MAP(client,block,svp_block) {BCHP_MemcClient_e##client, BMRC_Monitor_HwBlock_e##block},
#include "memc/bchp_memc_clients_chip_map.h"
#undef BCHP_P_MEMC_DEFINE_CLIENT_MAP
        {BCHP_MemcClient_eMax, BMRC_Monitor_HwBlock_eInvalid}
    }; /* this map is not sorted by either client or block */


    /* verify that all HW blocks are accunted for */
    /* 1 , clear allocation array */
    for(i=0;i<BMRC_Monitor_HwBlock_eMax;i++) {
        clients[i]=false;
        fileBlocks[i]=false;
    }

    /* 2. Tag clients from the privilegedClients list */
    for(i=0;i<sizeof(privilegedClients)/sizeof(*privilegedClients);i++) {
        if(privilegedClients[i]==BMRC_Monitor_HwBlock_eInvalid) {
            break;
        }
        clients[privilegedClients[i]]=true;
    }

    /* 3. Tag clients from the controlledClients list */
    for(i=0;i<sizeof(controlledClients)/sizeof(*controlledClients);i++) {
        if(controlledClients[i]==BMRC_Monitor_HwBlock_eInvalid) {
            break;
        }
        clients[controlledClients[i]]=true;
    }
    /* 3. Tag clients from the fileMap */
    for(i=0;i<sizeof(fileMap)/sizeof(*fileMap);i++) {
        const BMRC_P_Monitor_FileInfo *info = &fileMap[i];
        unsigned j;
        for(j=0;j<sizeof(info->hwBlocks)/sizeof(info->hwBlocks[0]);j++) {
            if(info->hwBlocks[j]==BMRC_Monitor_HwBlock_eInvalid) {
                break;
            }
            clients[info->hwBlocks[j]]=true;
            fileBlocks[info->hwBlocks[j]]=true;
        }
    }
    /* 4. Confirm that all HW blocks were allocated */
    for(unmapped=0,i=0;i<sizeof(clients)/sizeof(clients[0]);i++) {
        if(!clients[i]) {
            unmapped++;
            BDBG_ERR(("unknown HW block %s(%u) %u", hwBlockNames[i], i, BMRC_Monitor_HwBlock_eMax));
        }
    }
    if(unmapped) {
        BDBG_ASSERT(unmapped==0);
    }

    for(length=0,i=0;i<sizeof(fileBlocks)/sizeof(fileBlocks[0]);i++) {
        if(fileBlocks[i]) {
            map->fileBlocks[length] = i;
            length++;
        }
    }
    map->fileBlocks[length] = BMRC_Monitor_HwBlock_eInvalid;

    BDBG_CASSERT(sizeof(rawMap)/sizeof(rawMap[0])==sizeof(map->clientMap)/sizeof(map->clientMap[0]));
    for(i=0;i<sizeof(rawMap)/sizeof(rawMap[0]);i++) {
        unsigned j;
        struct BMRC_P_ClientMapEntry next = rawMap[i];
        for(j=0;j<i;j++) {
            if(next.block < map->clientMap[j].block) {
                /* found insertion point */
                break;
            }
        }
        /* insert and shift old data down */
        for(;j<=i;j++) {
            struct BMRC_P_ClientMapEntry tmp = map->clientMap[j];
            map->clientMap[j] = next;
            next = tmp;
        }
    }
    for(i=0;i<sizeof(map->clientMap)/sizeof(map->clientMap[0]);i++) {
        BDBG_MSG_TRACE(("clientMap:[%u] %u(%s):%u(%s)", i, map->clientMap[i].block, hwBlockNames[map->clientMap[i].block], map->clientMap[i].client, BMRC_P_GET_CLIENT_NAME(map->clientMap[i].client)));
    }
    /* build array of entries */
    for(i=0;i<sizeof(map->clientMapEntry)/sizeof(map->clientMapEntry[0]);i++) {
        unsigned j;
        unsigned entry = sizeof(rawMap)/sizeof(rawMap[0]) - 1; /* point to last element in map, with Invalid HW block */
        for(j=0;j<sizeof(map->clientMap)/sizeof(map->clientMap[0]);j++) {
            if(map->clientMap[j].block == i) { /* bingo */
                entry = j;
                break;
            }
        }
        map->clientMapEntry[i] = entry;
    }
    for(i=0;i<sizeof(map->clientMapEntry)/sizeof(map->clientMapEntry[0]);i++) {
        unsigned entry = map->clientMapEntry[i];
        BSTD_UNUSED(entry);
        BDBG_MSG_TRACE(("clientMapEntry:[%u(%s)] %d(%s:%s)", i, hwBlockNames[i], entry, hwBlockNames[map->clientMap[entry].block], BMRC_P_GET_CLIENT_NAME(map->clientMap[entry].client)));
    }
    for(i=0;i<sizeof(map->clientToBlock)/sizeof(map->clientToBlock[0]);i++) {
        map->clientToBlock[i]=BMRC_Monitor_HwBlock_eInvalid;
    }
    for(i=0;i<sizeof(rawMap)/sizeof(rawMap[0]);i++) {
        BCHP_MemcClient client = rawMap[i].client;
        if(client==BCHP_MemcClient_eMax) {
            break;
        }
        map->clientToBlock[client] = rawMap[i].block;
    }
    return;
}

/* cheap version of toupper */
#define B_TOUPPER(x) ((x)&(~0x20))

static const struct BMRC_P_Monitor_FileInfo *
BMRC_Monitor_P_GetFileInfo(const char *fname)
{
    unsigned i;
    const char *ptr;
    if (fname==NULL) {
        return NULL;
    }

    for(ptr=fname;*ptr!='\0';ptr++) { } /* scan to the end of string */

    for(;;) { /* back track to find first '\' or '/' */
        if(ptr==fname) { /* no directory */
            break;
        }
        if(*ptr=='\\' || *ptr=='/') {
            ptr++; /* drop it */
            break;
        }
        ptr--;
    }
    fname = ptr;

    for(i=0;i<sizeof(fileMap)/sizeof(fileMap);i++) {
        unsigned j;
        for(j=0;B_TOUPPER(fileMap[i].swModule[j]) == B_TOUPPER(fname[j]);) { /* do strncmp type of stuff */
            j++;
            if(fileMap[i].swModule[j]=='\0') {
                /* bingo!!! found match */
                BDBG_MSG_TRACE(("allocation from %s matched to %s clients", fname, fileMap[i].swModule));
                return &fileMap[i];
            }
        }
    }
    return NULL;
}

void
BMRC_Monitor_P_SetHwBlocks(const struct BMRC_P_MonitorClientMap *map, const uint8_t *hwBlocks, struct BMRC_Monitor_P_ClientList *clientList, bool add)
{
    unsigned i;
    for(i=0;;i++) {
        unsigned index;
        BMRC_Monitor_HwBlock  hwBlock = hwBlocks[i];
        if(hwBlock==BMRC_Monitor_HwBlock_eInvalid) {
            break;
        }
        for(index = map->clientMapEntry[hwBlock]; map->clientMap[index].block == hwBlock;index++) {
            clientList->clients[map->clientMap[index].client]=add;
        }
    }
    return;
}

void BMRC_Monitor_P_MapGetClientsByFileName(const struct BMRC_P_MonitorClientMap *map, const char *fname, struct BMRC_Monitor_P_ClientList *clientList)
{
    const struct BMRC_P_Monitor_FileInfo *fileInfo = BMRC_Monitor_P_GetFileInfo(fname);
    BKNI_Memset(clientList,0,sizeof(*clientList));
    if(fileInfo) {
        BMRC_Monitor_P_SetHwBlocks(map, fileInfo->hwBlocks, clientList, true);
    } else {
        BMRC_Monitor_P_SetHwBlocks(map, map->fileBlocks, clientList, true);
    }
    BMRC_Monitor_P_SetHwBlocks(map, controlledClients, clientList, true);
    BMRC_Monitor_P_SetHwBlocks(map, privilegedClients, clientList, false);
    return;
}

void BMRC_Monitor_P_MapGetHwClients(struct BMRC_P_MonitorClientMap *map, struct BMRC_Monitor_P_ClientList *clientList)
{
    BKNI_Memset(clientList,0,sizeof(*clientList));
    BMRC_Monitor_P_SetHwBlocks(map, map->fileBlocks, clientList, true);
    BMRC_Monitor_P_SetHwBlocks(map, controlledClients, clientList, true);
    BMRC_Monitor_P_SetHwBlocks(map, privilegedClients, clientList, false);
    return;
}

void BMRC_Monitor_P_PrintBitmap(const struct BMRC_P_MonitorClientMap *map, const uint32_t *bitmap, size_t bitmapSize, const char *blockedType)
{
    int i;
    char buf[80];
    size_t buf_off=0;
    uint8_t hwBlocks[1+BMRC_Monitor_HwBlock_eMax/8]; /* bitmap for HW groups */
#ifdef BDBG_DEBUG_BUILD
#else
    BSTD_UNUSED (blockedType);
#endif

    BKNI_Memset(hwBlocks, 0, sizeof(hwBlocks));
    for(i=bitmapSize-1;i>=0;i--) {
        int bit;
        for(bit=31;bit>=0;bit--) {
            if(bitmap[i] & 1<<bit) {
                int left = sizeof(buf) - buf_off;
                unsigned client = i*32 + bit;
                const char *name = BMRC_P_GET_CLIENT_NAME(client);
                int rc;
                BMRC_Monitor_HwBlock block = map->clientToBlock[client];
                hwBlocks[block/8] |= 1<<(block%8);
                if(left<=20) {
                    BDBG_MODULE_MSG(BMRC_MONITOR_PRINT,("clients: %s",buf));
                    left = sizeof(buf);
                    buf_off = 0;
                }
                rc = BKNI_Snprintf(buf+buf_off,left, "%s%s(%u)",buf_off==0?"":",",name,client);
                if(rc<0 || rc>left) {
                    BDBG_MODULE_MSG(BMRC_MONITOR,("clients: %s..",buf));
                    left = sizeof(buf);
                    buf_off = 0;
                } else {
                    buf_off+=rc;
                }
            }
        }
    }
    if(buf_off) {
        BDBG_MODULE_MSG(BMRC_MONITOR,("clients: %s",buf));
        buf_off = 0;
    }
    for(i=sizeof(hwBlocks)-1;i>=0;i--) {
        int bit;
        for(bit=7;bit>=0;bit--) {
            if(hwBlocks[i] & 1<<bit) {
                int left = sizeof(buf) - buf_off;
                unsigned hwBlock = i*8 + bit;
                const char *name;
                int rc;

                if(hwBlock >= sizeof(hwBlockNames)/sizeof(hwBlockNames[0])) {
                    continue;
                }
                name = hwBlockNames[hwBlock];
                rc = BKNI_Snprintf(buf+buf_off,left, "%s%s",buf_off==0?"":",",name);
                if(rc<0 || rc>left) {
                    BDBG_MODULE_LOG(BMRC_MONITOR,("blocked[%s] HW: %s..", blockedType, buf));
                    left = sizeof(buf);
                    buf_off = 0;
                } else {
                    buf_off+=rc;
                }
            }
        }
    }
    if(buf_off) {
        BDBG_MODULE_LOG(BMRC_MONITOR,("blocked[%s] HW: %s", blockedType, buf));
        buf_off = 0;
    }

    return;
}
