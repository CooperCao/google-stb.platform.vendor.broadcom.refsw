/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bmrc_monitor.h"
#include "bmrc_monitor_priv.h"
#include "bkni.h"
#include "bkni_multi.h"          /* for semaphores */
#include "blst_aa_tree.h"
#include "bmma_pool.h"
#include "bmma_types.h"
#include "blst_queue.h"
#include "bmrc_clienttable_priv.h"

BDBG_MODULE(BMRC_MONITOR);
BDBG_FILE_MODULE(BMRC_MonitorRegion);


#define BMRC_P_MONITOR_MAX_RANGES 8
#define BMRC_P_MONITOR_CLIENT_MASK_ARRAY_ELEMENT_SIZE 32
#define BMRC_P_MONITOR_CLIENT_MASK_ARRAY_SIZE  (BMRC_Client_eMaxCount + (BMRC_P_MONITOR_CLIENT_MASK_ARRAY_ELEMENT_SIZE-1)) / BMRC_P_MONITOR_CLIENT_MASK_ARRAY_ELEMENT_SIZE

#define BMRC_P_MONITOR_DBG_TRACE 0

#if BMRC_P_MONITOR_DBG_TRACE
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif


/* Definitions for word sizes.

    Dword (or Word)  32 bits
    Oword  64 bits
    Gword  128 bits
    Jword  256 bits (J for Jumbo, if you like.)
*/
#define BMRC_P_MONITOR_DWORD_BYTES 4
#define BMRC_P_MONITOR_OWORD_BYTES 8
#define BMRC_P_MONITOR_GWORD_BYTES 16
#define BMRC_P_MONITOR_JWORD_BYTES 32

/* SCB Protocol Specifications the following table was derived from are available at
   SCB 4.x: http://www.blr.broadcom.com/projects/DVT_BLR/Memc_Arch
   SCB 5.0: http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Bussing/Released/28nm/SCB_Protocol.doc
   SCB 8.0: http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Bussing/Released/64bits/SCB_Spec_80.docx
   These specs are also used in BMRC_P_Monitor_Dump_isr below. */
#if BCHP_40NM
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x40
#elif BCHP_CHIP==7250 || BCHP_CHIP==7364 || BCHP_CHIP==7366 || BCHP_CHIP==7439 || BCHP_CHIP==7445 || BCHP_CHIP==74371 || BCHP_CHIP==7586
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x50
#else
/* new silicon */
#define BMRC_MONITOR_P_SCB_PROTOCOL_VER 0x80
#endif

typedef enum
{
    BMRC_P_Monitor_ScbCommand_eLR = 1,
    BMRC_P_Monitor_ScbCommand_eLW = 2,
    BMRC_P_Monitor_ScbCommand_eREF = 3,
    BMRC_P_Monitor_ScbCommand_eMRS = 4,
    BMRC_P_Monitor_ScbCommand_eEMRS = 5,
    BMRC_P_Monitor_ScbCommand_ePALL = 6,
    BMRC_P_Monitor_ScbCommand_eDR = 7,
    BMRC_P_Monitor_ScbCommand_eDW = 8,
    BMRC_P_Monitor_ScbCommand_eMR = 9,
    BMRC_P_Monitor_ScbCommand_eMW = 10,
    BMRC_P_Monitor_ScbCommand_eCR = 11,
#if BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50
    BMRC_P_Monitor_ScbCommand_eLWWR = 12,
#endif
    BMRC_P_Monitor_ScbCommand_eUnknown = 0
}BMRC_P_Monitor_ScbCommand;

/* some clients have J-word min access length */
#define BMRC_P_MONITOR_CHECKER_ADDR_ALIGN         ~0x000000FF

/* SCB Command Table */
static const struct BMRC_P_Monitor_ScbCommandInfo {
    BMRC_P_Monitor_ScbCommand eScbCommand;
    uint32_t ulCommand;
    uint32_t ulMask;
    const char *pName;
} g_ScbCommandInfoTbl[] =
{
    {BMRC_P_Monitor_ScbCommand_eLR,   0x000,   0x1E0, "Linear Read - LR"},
    {BMRC_P_Monitor_ScbCommand_eLW,   0x020,   0x1E0, "Linear Write - LW"},
#if BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50
    {BMRC_P_Monitor_ScbCommand_eLWWR, 0x060,   0x1E0, "Linear Write with Reply - LWWR"},
#endif
    {BMRC_P_Monitor_ScbCommand_eREF,  0x05C,   0x1FF, "Refresh - REF"},
    {BMRC_P_Monitor_ScbCommand_eMRS,  0x05D,   0x1FF, "Mode Register Set - MRS"},
    {BMRC_P_Monitor_ScbCommand_eEMRS, 0x05E,   0x1FF, "Extended Mode Reg Set - EMRS"},
    {BMRC_P_Monitor_ScbCommand_ePALL, 0x05F,   0x1FF, "Precharge All Banks - PALL"},
#if (BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50)
    {BMRC_P_Monitor_ScbCommand_eDR,   0x180,   0x1C0, "Video Raster Read - DR"},
    {BMRC_P_Monitor_ScbCommand_eDW,   0x1C0,   0x1C0, "Video Raster Write - DW"},
#else
    {BMRC_P_Monitor_ScbCommand_eDR,   0x180,   0x1E0, "Video Raster Read - DR"},
    {BMRC_P_Monitor_ScbCommand_eDW,   0x1A0,   0x1E0, "Video Raster Write - DW"},
#endif
    {BMRC_P_Monitor_ScbCommand_eMR,   0x080,   0x180, "MPEG Block Read - MR"},
    {BMRC_P_Monitor_ScbCommand_eMW,   0x100,   0x180, "MPEG Block Write - MW"},
    {BMRC_P_Monitor_ScbCommand_eCR,   0x040,   0x1E0, "Cache Read"}
};
#define BMRC_P_MONITOR_SCB_ACCESS_TABLE_SIZE (sizeof(g_ScbCommandInfoTbl)/sizeof(g_ScbCommandInfoTbl[0]))

#define BMRC_P_MONITOR_ALIGNED_RANGE_START(start_addr, exclusive) \
    ((start_addr + (exclusive ? 0 : ~(BMRC_P_MONITOR_CHECKER_ADDR_ALIGN))) & \
     BMRC_P_MONITOR_CHECKER_ADDR_ALIGN)

#define BMRC_P_MONITOR_ALIGNED_RANGE_END(end_addr, exclusive) \
    ((end_addr + (exclusive ? ~(BMRC_P_MONITOR_CHECKER_ADDR_ALIGN) : 0)) & \
     BMRC_P_MONITOR_CHECKER_ADDR_ALIGN)

#define BMRC_P_MONITOR_CLIENT_MASK(client_id)      (1 << ((client_id) % BMRC_P_MONITOR_CLIENT_MASK_ARRAY_ELEMENT_SIZE))
#define BMRC_P_MONITOR_CLIENT_MASK_IDX(client_id)  ((client_id) / BMRC_P_MONITOR_CLIENT_MASK_ARRAY_ELEMENT_SIZE)
#define BMRC_P_MONITOR_CLIENT_MASK_IS_SET(client_mask, client_id) \
    (client_mask[BMRC_P_MONITOR_CLIENT_MASK_IDX(client_id)] & BMRC_P_MONITOR_CLIENT_MASK(client_id))
#define BMRC_P_MONITOR_CLIENT_MASK_SET(client_mask, client_id) \
    client_mask[BMRC_P_MONITOR_CLIENT_MASK_IDX(client_id)] |= BMRC_P_MONITOR_CLIENT_MASK(client_id);
#define BMRC_P_MONITOR_CLIENT_MASK_UNSET(client_mask, client_id) \
    client_mask[BMRC_P_MONITOR_CLIENT_MASK_IDX(client_id)] &= ~BMRC_P_MONITOR_CLIENT_MASK(client_id);


/* default settings */
static const BMRC_Monitor_Settings s_stDefaultSettings = {
    BMRC_AccessType_eWrite,  /* kernel violation access block */
    BMRC_AccessType_eWrite,  /* regular violation access block */
    UINT32_C(-1),            /* maximum number of checkers to use. -1 means use all available. */
    false
};

static void BMRC_P_Monitor_UpdateFull(BMRC_Monitor_Handle hMonitor);
static void BMRC_P_Monitor_UpdateSingle(BMRC_Monitor_Handle hMonitor, unsigned range);
static void BMRC_P_Monitor_isr( void *cnxt, int no, BMRC_CheckerInfo *pInfo);

typedef struct BMRC_Monitor_P_Clients {
    uint32_t client_masks[BMRC_P_MONITOR_CLIENT_MASK_ARRAY_SIZE];
}BMRC_Monitor_P_Clients;


typedef struct BMRC_Monitor_P_AllocatedRegion {
    BLST_AA_TREE_ENTRY(BMRC_Monitor_P_AllocatedRegionTree) node; /* tree of regions sorted by address */
    BMMA_DeviceOffset addr;
    unsigned size;
} BMRC_Monitor_P_AllocatedRegion;

typedef struct BMRC_Monitor_CombinedRegion {
    BMMA_DeviceOffset addr;
    BMMA_DeviceOffset size; /* size of region could exceed 4GB */
} BMRC_Monitor_CombinedRegion;

static int BMRC_Monitor_P_AllocatedRegionTree_Compare_isrsafe(const struct BMRC_Monitor_P_AllocatedRegion* node, BMMA_DeviceOffset addr)
{
    if(addr > node->addr) {
        return 1;
    } else if(addr==node->addr) {
        return 0;
    } else {
        return -1;
    }
}


BLST_AA_TREE_HEAD(BMRC_Monitor_P_AllocatedRegionTree, BMRC_Monitor_P_AllocatedRegion);
BLST_AA_TREE_GENERATE_INSERT(BMRC_Monitor_P_AllocatedRegionTree, BMMA_DeviceOffset, BMRC_Monitor_P_AllocatedRegion, node, BMRC_Monitor_P_AllocatedRegionTree_Compare_isrsafe)
BLST_AA_TREE_GENERATE_FIND(BMRC_Monitor_P_AllocatedRegionTree, BMMA_DeviceOffset, BMRC_Monitor_P_AllocatedRegion, node, BMRC_Monitor_P_AllocatedRegionTree_Compare_isrsafe)
#if BDBG_DEBUG_BUILD
BLST_AA_TREE_GENERATE_FIND_SOME(BMRC_Monitor_P_AllocatedRegionTree, BMMA_DeviceOffset, BMRC_Monitor_P_AllocatedRegion, node, BMRC_Monitor_P_AllocatedRegionTree_Compare_isrsafe)
#endif
BLST_AA_TREE_GENERATE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, BMRC_Monitor_P_AllocatedRegion, node)
BLST_AA_TREE_GENERATE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, BMRC_Monitor_P_AllocatedRegion, node)
#if BDBG_DEBUG_BUILD
BLST_AA_TREE_GENERATE_PREV(BMRC_Monitor_P_AllocatedRegionTree, BMRC_Monitor_P_AllocatedRegion, node)
#endif
BLST_AA_TREE_GENERATE_REMOVE(BMRC_Monitor_P_AllocatedRegionTree, BMRC_Monitor_P_AllocatedRegion, node)

struct BMRC_Monitor_P_CheckerState {
    bool active;
    bool interruptDisabled;
    bool exclusive;
    BMRC_AccessType accessType;
    BMMA_DeviceOffset addr;
    BMMA_DeviceOffset size;
    BMRC_Monitor_P_Clients clients;
};

BDBG_OBJECT_ID(BMRC_MonitorRegion);
struct BMRC_MonitorRegion {
    BDBG_OBJECT(BMRC_MonitorRegion)
    BLST_Q_ENTRY(BMRC_MonitorRegion) link;
    BMRC_MonitorRegion_Settings settings;
    uint8_t hwBlocks[1]; /* variable length list, terminated by BMRC_Monitor_HwBlock_eInvalid */
};

BDBG_OBJECT_ID(BMRC_Monitor);
typedef struct BMRC_P_MonitorContext {
    BDBG_OBJECT(BMRC_Monitor)
    BMRC_Settings mrcSettings;
    BMRC_Monitor_Settings stSettings;
    BMRC_Handle mrc;
    BREG_Handle reg;
    BINT_Handle isr;
    BKNI_MutexHandle pMutex; /* Semaphore to lock this monitor */
    BMMA_DeviceOffset mem_low, mem_high; /* memory region what would be monitored */
    BMMA_PoolAllocator_Handle poolAllocator;
    unsigned numberOfUserRanges;
    BLST_Q_HEAD(BMRC_P_MonitorRegions, BMRC_MonitorRegion) userRegions;
    bool enabled;

    unsigned max_ranges; /* maximum number of checkers */
    unsigned num_combined_regions;
    struct BMRC_Monitor_P_AllocatedRegionTree regions; /* list of all known memory regions, sorted by their address, from low to high */
    struct BMRC_Monitor_P_AllocatedRegionTree custom_regions; /* list of exclusive regions */
    BMRC_Monitor_CombinedRegion combined_regions[BMRC_P_MONITOR_MAX_RANGES]; /* continuous holes between tracked allocations */
    BMRC_Monitor_P_Clients clients; /* current clients used to setup range checker */
    BMRC_ClientInfo client_infos[BMRC_Client_eMaxCount];
    BINT_CallbackHandle irq[BMRC_P_MONITOR_MAX_RANGES];
    BMRC_Checker_Handle ahCheckers[BMRC_P_MONITOR_MAX_RANGES];
    struct BMRC_Monitor_P_CheckerState checkerState[BMRC_P_MONITOR_MAX_RANGES]; /* since there is no way to get actual state of range checkers, remember them here */
    bool protect_mem_low;
    struct BMRC_P_MonitorClientMap map;
    unsigned last_threshold;
    size_t allocated; /* number of allocated bytes */
    unsigned partial_update_count;
    struct {
        struct {
            unsigned full, single, partial;
        } updates;
    } stats;

}BMRC_P_MonitorContext;


/* client mask manipulation functions */
static void
BMRC_P_Monitor_MasksClear(BMRC_Monitor_P_Clients *mask)
{
    BKNI_Memset(mask->client_masks, 0, sizeof(mask->client_masks));
}

static void
BMRC_P_Monitor_MaskAdd(BMRC_Monitor_P_Clients *mask, BMRC_Client client)
{
    BMRC_P_MONITOR_CLIENT_MASK_SET(mask->client_masks, client);
}

#ifdef BDBG_DEBUG_BUILD
#include "bmrc_priv.h"
/* this function builds the hardware client id bit string */
static const char *BMRC_P_Monitor_BuildClientIdString_isrsafe(const BMRC_Monitor_P_Clients *mask, char *buf, size_t buf_len)
{
    int i;
    size_t buf_off = 0;
    const unsigned clients_bitmap_size = (BMRC_P_CLIENTS_MAX+7)/8;
    uint8_t clients_bitmap[(BMRC_P_CLIENTS_MAX+7)/8];
    bool leading_zeros=true;

    for(i=0;i<(int)clients_bitmap_size;i++) {
        clients_bitmap[i]=0;
    }
    for (i = 0; i < BMRC_Client_eMaxCount; i++) {
        if(!BMRC_P_MONITOR_CLIENT_MASK_IS_SET(mask->client_masks, i)) {
            int clientId = BMRC_P_GET_CLIENT_ID(0 /* memc */, i);
            if(clientId>=0) {
                unsigned bit = clientId%8;
                unsigned byte = clientId/8;
                if(byte<clients_bitmap_size) {
                    clients_bitmap[byte] |=  1<<bit;
                }
            }
        }
    }

    *buf = 0;
    for(i=sizeof(clients_bitmap)/sizeof(clients_bitmap[0])-1;i>=0;i--) {
        int left = buf_len - buf_off;
        int rc;
        if(left<=0) {
            break;
        }
        if(leading_zeros && clients_bitmap[i]==0) {
            continue;
        }
        leading_zeros = false;
        rc = BKNI_Snprintf(buf+buf_off,left, "%s%02x",(i%4)==3?".":"",clients_bitmap[i]);
        if(rc<0 || rc>=left) {
            break;
        }
        buf_off += rc;
    }
    return buf;
}
#endif

static void BMRC_P_Monitor_BuildMaskFromList(const struct BMRC_Monitor_P_ClientList *clientList, BMRC_Monitor_P_Clients *clients)
{
    unsigned i;
    BMRC_P_Monitor_MasksClear(clients);
    for(i=0;i<sizeof(clientList->clients)/sizeof(clientList->clients[0]);i++) {
        if(clientList->clients[i]) {
            BMRC_P_Monitor_MaskAdd(clients, i);
        }
    }
    return;
}


static void BMRC_P_Monitor_MaskBuildHwClients(BMRC_Monitor_Handle hMonitor,BMRC_Monitor_P_Clients *mask)
{
    struct BMRC_Monitor_P_ClientList clientList;
    BMRC_Monitor_P_MapGetHwClients(&hMonitor->map, &clientList);
    BMRC_P_Monitor_BuildMaskFromList(&clientList, mask);
    return;
}


BERR_Code
BMRC_Monitor_Open(BMRC_Monitor_Handle *phMonitor, BREG_Handle hReg, BINT_Handle hIsr, BCHP_Handle hChp, BMRC_Handle hMrc, BMMA_DeviceOffset ulMemLow, BMMA_DeviceOffset ulMemHigh, BMRC_Monitor_Settings *pSettings)
{
    BMRC_Monitor_Handle hMonitor;
    BERR_Code rc;
    unsigned i;
    unsigned max_checkers;
    BMMA_PoolAllocator_CreateSettings poolSettings;

    BSTD_UNUSED(hChp);
    BDBG_ASSERT(hReg);
    BDBG_ASSERT(hIsr);
    BDBG_ASSERT(hChp);


    if (ulMemLow >= ulMemHigh)
    {
        BDBG_ERR(("High memory address must be greater than low memory address."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hMonitor = BKNI_Malloc(sizeof(*hMonitor));
    if (!hMonitor) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(hMonitor, 0, sizeof(*hMonitor));
    BDBG_OBJECT_SET(hMonitor, BMRC_Monitor);
    BMRC_Monitor_P_MapInit(&hMonitor->map);

    BMRC_GetMaxCheckers(hMrc, &max_checkers);
    if (max_checkers > BMRC_P_MONITOR_MAX_RANGES)
    {
        BDBG_WRN(("BMRC_P_MONITOR_MAX_RANGES less than actual checkers available. Max checkers: %d", max_checkers));
        BDBG_WRN(("Setting max checkers to BMRC_P_MONITOR_MAX_RANGES (%u/%u)", max_checkers, BMRC_P_MONITOR_MAX_RANGES));
        max_checkers = BMRC_P_MONITOR_MAX_RANGES;
    }

    if (pSettings)
    {
        if ((pSettings->ulNumCheckersToUse != UINT32_C(-1)) &&
            (pSettings->ulNumCheckersToUse > max_checkers))
        {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);

            BDBG_ERR(("Not enough checkers available. Num checkers: %d, Max checkers: %d",
                      pSettings->ulNumCheckersToUse, max_checkers));
            goto err_max_checkers;
        }

        hMonitor->stSettings = *pSettings;
    }
    else
    {
        hMonitor->stSettings = s_stDefaultSettings;
    }
    hMonitor->enabled = !hMonitor->stSettings.startDisabled;

    /* create mutex for re-entrant control */
    rc = BERR_TRACE(BKNI_CreateMutex(&(hMonitor->pMutex)));
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to create mutex."));
        (void)BERR_TRACE(rc);
        goto err_mutex;
    }

    hMonitor->mrc = hMrc;
    BMRC_GetSettings(hMonitor->mrc, &hMonitor->mrcSettings);
    hMonitor->reg = hReg;
    hMonitor->isr = hIsr;
    hMonitor->mem_high = ulMemHigh;
    hMonitor->mem_low = ulMemLow;
    hMonitor->max_ranges = (hMonitor->stSettings.ulNumCheckersToUse == UINT32_C(-1)) ?
        max_checkers : hMonitor->stSettings.ulNumCheckersToUse;
    hMonitor->num_combined_regions = 0;
    hMonitor->last_threshold = BMRC_P_MONITOR_JWORD_BYTES;
    hMonitor->partial_update_count = 0;
    hMonitor->allocated = 0;
    hMonitor->stats.updates.full = hMonitor->stats.updates.single = hMonitor->stats.updates.partial = 0;
    hMonitor->numberOfUserRanges = 0;
    BLST_Q_INIT(&hMonitor->userRegions);

    BDBG_MSG(("BMRC_Monitor_Open: MEMC%u " BDBG_UINT64_FMT "..." BDBG_UINT64_FMT "",  hMonitor->mrcSettings.usMemcId, BDBG_UINT64_ARG(hMonitor->mem_low), BDBG_UINT64_ARG(hMonitor->mem_high)));

    BMMA_PoolAllocator_GetDefaultCreateSettings(&poolSettings);
    poolSettings.allocationSize = sizeof(BMRC_Monitor_P_AllocatedRegion);
    rc = BMMA_PoolAllocator_Create(&hMonitor->poolAllocator, &poolSettings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_allocator; }

    /* populate clientinfo array */
    for (i = 0; i < BMRC_Client_eMaxCount; i++)
    {
        BMRC_Checker_GetClientInfo(hMonitor->mrc, i, &(hMonitor->client_infos[i]));
    }

    BLST_AA_TREE_INIT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions);
    BLST_AA_TREE_INIT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->custom_regions);

    BMRC_P_Monitor_MaskBuildHwClients(hMonitor,&hMonitor->clients);
    for (i = 0; i < BMRC_Client_eMaxCount; i++)
    {
        BMRC_ClientInfo *client_info = &hMonitor->client_infos[i];
        if (client_info->eClient == BMRC_Client_eInvalid)
        {
            BMRC_P_MONITOR_CLIENT_MASK_UNSET(hMonitor->clients.client_masks, i);
        }
    }

    BDBG_ASSERT(hMonitor->mem_high > hMonitor->mem_low);
    hMonitor->combined_regions[0].addr = hMonitor->mem_low;
    hMonitor->combined_regions[0].size = hMonitor->mem_high - hMonitor->mem_low;
    hMonitor->num_combined_regions = 1;

    for (i=0;i<hMonitor->max_ranges;i++) {
        BMRC_Checker_Handle hChecker = NULL;

        rc = BMRC_Checker_Create(hMrc, &hChecker);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("Out of checkers on checker create."));
            (void)BERR_TRACE(rc);
            goto err_checker_create;
        }

        rc = BMRC_Checker_SetCallback(hChecker, BMRC_P_Monitor_isr, hMonitor, i);
        if (rc!=BERR_SUCCESS) {
            (void)BERR_TRACE(rc);
            goto err_set_callback;
        }

        rc = BMRC_Checker_EnableCallback(hChecker);
        if (rc!=BERR_SUCCESS) {
            (void)BERR_TRACE(rc);
            goto err_enable_callback;
        }

        hMonitor->ahCheckers[i] = hChecker;
    }

    BMRC_P_Monitor_UpdateFull(hMonitor);

    *phMonitor = hMonitor;

    return BERR_SUCCESS;

err_enable_callback:
err_set_callback:
err_checker_create:
    for (i=0;i<hMonitor->max_ranges && NULL!=hMonitor->ahCheckers[i];i++) {
        BMRC_Checker_Destroy(hMonitor->ahCheckers[i]);
    }
    BMMA_PoolAllocator_Destroy(hMonitor->poolAllocator);
err_allocator:
    BKNI_DestroyMutex(hMonitor->pMutex);
err_max_checkers:
err_mutex:
    BKNI_Free(hMonitor);
    return rc;
}

static void BMRC_MonitorRegion_P_Remove(BMRC_Monitor_Handle hMonitor, BMRC_MonitorRegion_Handle hRegion)
{
    BLST_Q_REMOVE(&hMonitor->userRegions, hRegion, link);
    BDBG_ASSERT(hMonitor->numberOfUserRanges>0);
    hMonitor->numberOfUserRanges--;
    BDBG_OBJECT_DESTROY(hRegion, BMRC_MonitorRegion);
    BKNI_Free(hRegion);
    return;
}

void
BMRC_Monitor_Close(BMRC_Monitor_Handle hMonitor)
{
    unsigned i;
    BMRC_Monitor_P_AllocatedRegion *region;
    BMRC_MonitorRegion_Handle userRegion;

    BDBG_MSG(("stats: %p full:%u single:%u partial:%u", (void *)hMonitor, hMonitor->stats.updates.full, hMonitor->stats.updates.single, hMonitor->stats.updates.partial));
    for (i=0;i<hMonitor->max_ranges;i++) {
        BMRC_Checker_Destroy(hMonitor->ahCheckers[i]);
    }
    while( NULL !=(region=BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions))) {
        BLST_AA_TREE_REMOVE(BMRC_Monitor_P_AllocatedRegionTree,&hMonitor->regions, region);
        BMMA_PoolAllocator_Free(hMonitor->poolAllocator, region);
    }
    while( NULL != (userRegion = BLST_Q_FIRST(&hMonitor->userRegions))) {
        BMRC_MonitorRegion_P_Remove(hMonitor, userRegion);
    }

    BMMA_PoolAllocator_Destroy(hMonitor->poolAllocator);

    /* destroy mutex */
    BKNI_DestroyMutex(hMonitor->pMutex);
    BDBG_OBJECT_DESTROY(hMonitor, BMRC_Monitor);
    BKNI_Free(hMonitor);
}

void
BMRC_Monitor_GetDefaultSettings(BMRC_Monitor_Settings *pDefSettings)
{
    *pDefSettings = s_stDefaultSettings;
}

static void
BMRC_P_Monitor_GetClientsByFname(BMRC_Monitor_Handle hMonitor,const char *fname, BMRC_Monitor_P_Clients *clients)
{
    struct BMRC_Monitor_P_ClientList clientList;
    BMRC_Monitor_P_MapGetClientsByFileName(&hMonitor->map, fname, &clientList);
    BMRC_P_Monitor_BuildMaskFromList(&clientList, clients);
    return;
}

#if 0
static void
BMRC_P_Monitor_CombinedRegionVerify(BMRC_Monitor_Handle hMonitor)
{
    unsigned i;

    for(i=0;i<hMonitor->num_combined_regions;i++) {
        const BMRC_Monitor_CombinedRegion *region=&hMonitor->combined_regions[i];
        BDBG_MSG_TRACE(("%p: region [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - %u" , (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region->addr+region->size), (unsigned)region->size));
        BDBG_ASSERT(region->size > 0);
        if(i+1<hMonitor->num_combined_regions) {
            const BMRC_Monitor_CombinedRegion *next=&hMonitor->combined_regions[i+1];
            BDBG_MSG_TRACE(("%p: region[%u,%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , (void *)hMonitor, i, i+1, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region->addr+region->size), BDBG_UINT64_ARG(next->addr), BDBG_UINT64_ARG(next->addr+next->size)));
            BDBG_ASSERT(region->addr < next->addr);
            BDBG_ASSERT(region->addr + region->size <= next->addr);
        }
    }

    if(1) {
        BMRC_Monitor_P_AllocatedRegion *region;
        for(region=BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions);region;region=BLST_AA_TREE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions, region)) {
            BMMA_DeviceOffset region_start = region->addr;
            BMMA_DeviceOffset region_end = region->addr + region->size;
            for(i=0;i<hMonitor->num_combined_regions;i++) {
                BDBG_MSG_TRACE(("%p: verify [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " %s", (void *)hMonitor, i, BDBG_UINT64_ARG(region_start), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(hMonitor->combined_regions[i].addr), BDBG_UINT64_ARG(hMonitor->combined_regions[i].addr + hMonitor->combined_regions[i].size), (region_start >= hMonitor->combined_regions[i].addr  && region_end <= hMonitor->combined_regions[i].addr + hMonitor->combined_regions[i].size) ? "match" : "no-match"));
                if(region_start >= hMonitor->combined_regions[i].addr  && region_end <= hMonitor->combined_regions[i].addr + hMonitor->combined_regions[i].size) {
                    BDBG_WRN(("%p: found untracked range " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " ([%u]" BDBG_UINT64_FMT ".."  BDBG_UINT64_FMT ")", (void *)hMonitor, BDBG_UINT64_ARG(region_start), BDBG_UINT64_ARG(region_end), i, BDBG_UINT64_ARG(hMonitor->combined_regions[i].addr), BDBG_UINT64_ARG(hMonitor->combined_regions[i].addr + hMonitor->combined_regions[i].size)));
                    BDBG_ASSERT(0);
                }
            }
        }
    }
    return;
}
#else
#define BMRC_P_Monitor_CombinedRegionVerify(hMonitor)
#endif

static void
BMRC_P_Monitor_UpdatePartial(BMRC_Monitor_Handle hMonitor)
{
    hMonitor->partial_update_count ++;
    BDBG_MSG_TRACE(("PartialUpdate:%p %u", (void *)hMonitor, hMonitor->partial_update_count));
    hMonitor->stats.updates.partial++;
    if(hMonitor->partial_update_count >=100) {
        BMRC_P_Monitor_UpdateFull(hMonitor);
    }
    return;
}

static void
BMRC_P_Monitor_Alloc(void *cnxt, BSTD_DeviceOffset addr, size_t size, const char *fname, int line)
{
    BMRC_Monitor_Handle hMonitor = cnxt;
    BMRC_Monitor_P_AllocatedRegion *region;
    BMRC_Monitor_P_Clients clients;

    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);

    BDBG_MSG_TRACE(("%p: alloc: addr " BDBG_UINT64_FMT " size %#x file %s:%d", (void *)hMonitor, BDBG_UINT64_ARG(addr), (unsigned)size, fname?fname:"", line));

    BMRC_P_Monitor_GetClientsByFname(hMonitor, fname, &clients);

    region = BMMA_PoolAllocator_Alloc(hMonitor->poolAllocator);
    if(region==NULL) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto done;
    }
    if(size==0) {
        size = 1; /* region of size 0 is rather confusing, since it doesn't produce any valid inclusive ranges */
    }
    region->addr = addr;
    region->size = size;

    BKNI_AcquireMutex(hMonitor->pMutex);

    {
        BMRC_Monitor_P_AllocatedRegion *regionInserted;
        BKNI_EnterCriticalSection();
        regionInserted=BLST_AA_TREE_INSERT(BMRC_Monitor_P_AllocatedRegionTree,&hMonitor->regions, region->addr, region);
        BKNI_LeaveCriticalSection();
        BDBG_ASSERT(regionInserted == region);
        BSTD_UNUSED (regionInserted);
    }
    hMonitor->allocated += region->size;

#if 1
    /* optimization code */
    if(1) {
        unsigned i;
        bool need_full_update = false;
        bool partial_update = false;
        BMMA_DeviceOffset region_end = region->addr + region->size;

        for (i = 0; i < hMonitor->num_combined_regions; i++) {
            BMRC_Monitor_CombinedRegion *cur_region = &hMonitor->combined_regions[i];
            BMMA_DeviceOffset cur_region_end = cur_region->addr + cur_region->size;
            BDBG_MSG_TRACE(("%p: match [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region_end)));
            if(cur_region->addr <= region_end) { /* intersection */
                if(region->addr <= cur_region_end) {
                    bool update = false;
                    if(region->addr <= cur_region->addr + hMonitor->last_threshold ) {
                        if(cur_region_end > region_end) {
                            BDBG_MSG_TRACE(("%p: shrink head [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(%u) (%u)", (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region_end), region->size, hMonitor->last_threshold));
                            cur_region->size = cur_region_end - region_end;
                            cur_region->addr = region_end;
                            update = true;
                        }
                    } else if(region_end + hMonitor->last_threshold >= cur_region_end) {
                        if(region->addr > cur_region->addr) {
                            BDBG_MSG_TRACE(("%p: shrink tail [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "(%u) (%u)", (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region_end), region->size, hMonitor->last_threshold));
                            cur_region->size = region->addr - cur_region->addr;
                            update = true;
                        }
                    }
                    if(cur_region->size<hMonitor->last_threshold) {
                        need_full_update = true;
                    } else if(update) {
                        BMRC_P_Monitor_UpdateSingle(hMonitor, i);
                        partial_update = true;
                    } else {
                        need_full_update = true;
                    }
                    if(need_full_update) {
                        break;
                    }
                }
            } else {
                break; /* combined_regions sorted by address */
            }
        }
        if(need_full_update) {
            BMRC_P_Monitor_UpdateFull(hMonitor);
        } else if(partial_update) {
            BMRC_P_Monitor_UpdatePartial(hMonitor);
        }
    }
#else
    BMRC_P_Monitor_UpdateFull(hMonitor);
#endif

    BKNI_ReleaseMutex(hMonitor->pMutex);
done:
    return;
}

static void
BMRC_P_Monitor_Free(void *cnxt, BSTD_DeviceOffset addr)
{
    BMRC_Monitor_Handle hMonitor = cnxt;
    BMRC_Monitor_P_AllocatedRegion *region;

    BDBG_MSG_TRACE(("free: addr " BDBG_UINT64_FMT, BDBG_UINT64_ARG(addr)));

    BKNI_AcquireMutex(hMonitor->pMutex);

    region = BLST_AA_TREE_FIND(BMRC_Monitor_P_AllocatedRegionTree,&hMonitor->regions, addr);
    if (region)
    {
        unsigned i;
        bool need_full_update = false;
        BMMA_DeviceOffset region_end = region->addr + region->size;
        bool partial_update = true;

        BKNI_EnterCriticalSection();
        BLST_AA_TREE_REMOVE(BMRC_Monitor_P_AllocatedRegionTree,&hMonitor->regions, region);
        BKNI_LeaveCriticalSection();
        BDBG_ASSERT(hMonitor->allocated >= region->size);
        hMonitor->allocated -= region->size;

#if 1
        /* optimization code */
        for (i = 0; i < hMonitor->num_combined_regions; i++) {
            BMRC_Monitor_CombinedRegion *cur_region = &hMonitor->combined_regions[i];
            BDBG_MSG_TRACE(("%p: match [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region->addr+ cur_region->size)));
            if(region_end < cur_region->addr) {
                break; /* combined_regions sorted by address */
            } else if(region_end == cur_region->addr) {
                BDBG_MSG_TRACE(("%p: grow head [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region->addr+ cur_region->size)));
                cur_region->size += region->size;
                cur_region->addr = region->addr;
                BMRC_P_Monitor_UpdateSingle(hMonitor, i);
                partial_update = false;
                break;
            } else {
                BMMA_DeviceOffset cur_region_end = cur_region->addr + cur_region->size;
                if(region->addr == cur_region_end) {
                    BDBG_MSG_TRACE(("%p: grow tail [%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " - " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , (void *)hMonitor, i, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region_end), BDBG_UINT64_ARG(cur_region->addr), BDBG_UINT64_ARG(cur_region->addr+ cur_region->size)));
                    cur_region->size += region->size;
                    if(i+1<hMonitor->num_combined_regions) {
                        need_full_update = (cur_region->addr + cur_region->size >= hMonitor->combined_regions[i+1].addr); /* two free regions joined together */
                    }
                    if(!need_full_update) {
                        BMRC_P_Monitor_UpdateSingle(hMonitor, i);
                    }
                    partial_update = false;
                    break;
                }
            }
        }
        BMMA_PoolAllocator_Free(hMonitor->poolAllocator, region);
        if(need_full_update || hMonitor->allocated==0) {
            BMRC_P_Monitor_UpdateFull(hMonitor);
        } else if(partial_update) {
            BMRC_P_Monitor_UpdatePartial(hMonitor);
        }
#else
        BSTD_UNUSED(need_full_update);
        BSTD_UNUSED(region_end);
        BSTD_UNUSED(i);
        BMMA_PoolAllocator_Free(hMonitor->poolAllocator, region);
        BMRC_P_Monitor_Update(hMonitor, true);
#endif
    }
    else
    {
        BDBG_ERR(("Trying to free unknown block"));
    }

    BKNI_ReleaseMutex(hMonitor->pMutex);
    return;
}

/* this function combines all memory regions, what are accessible to the given clients, into the set of continuous regions, using selected threshold_ */
static bool
BMRC_P_Monitor_Combine(BMRC_Monitor_Handle hMonitor, size_t threshold, unsigned max_regions, unsigned *num_regions)
{
    unsigned i=0;
    BMRC_Monitor_P_AllocatedRegion *cur;
    BMMA_DeviceOffset hole_addr = hMonitor->mem_low;

    for(cur=BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions); cur ;cur=BLST_AA_TREE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions, cur))
    {
        size_t hole_size;

        BDBG_ASSERT(cur->addr >= hole_addr);
        hole_size = cur->addr - hole_addr;
        if(hole_size >= threshold ||
           (hole_size>0 && i==0) /* always allocate region for the first hole */
           ) {
            if(i>=max_regions) {
                *num_regions = i;
                return false;
            }
            hMonitor->combined_regions[i].size = hole_size;
            hMonitor->combined_regions[i].addr = hole_addr;
            i++;
        }
        hole_addr = cur->addr + cur->size;
    }
    if(hole_addr<hMonitor->mem_high) { /* and allocate region for the last hole */
        if(i<max_regions) {
            hMonitor->combined_regions[i].size = hMonitor->mem_high - hole_addr;
            hMonitor->combined_regions[i].addr = hole_addr;
            i++;
        } else {
            *num_regions = i;
            return false;
        }
    }
    *num_regions = i;
    return true;
}

static void
BMRC_P_Monitor_Disable(BMRC_Monitor_Handle hMonitor, unsigned arc_no)
{
    BMRC_Checker_Handle hChecker = hMonitor->ahCheckers[arc_no];
    struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[arc_no];
    state->active = false;
    BMRC_Checker_Disable(hChecker);
    return;
}

static bool
BMRC_P_Monitor_ProgramRange(BMRC_Monitor_Handle hMonitor, unsigned arc_no, BMMA_DeviceOffset addr, uint64_t size)
{
    const bool exclusive = false;
    BMMA_DeviceOffset aligned_size;
    struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[arc_no];
    BMMA_DeviceOffset aligned_addr;
    BMMA_DeviceOffset aligned_end;
    BMRC_Checker_Handle hChecker = hMonitor->ahCheckers[arc_no];

    /* disable read and write check first */
    BMRC_Checker_Disable(hChecker);

    /*
    ** Program up to, but do not include, the last address in the range. In some cases, calls to alloc() will
    ** will return memory that starts at that last, protected address. Accesses to that address could be a
    ** violation. In the 7038/3560, a bug in the ARC allowed those violations to go undetected. That bug is
    ** fixed in the 7401. See PR 13737.
    */

    aligned_addr = BMRC_P_MONITOR_ALIGNED_RANGE_START(addr, exclusive);
    aligned_end = BMRC_P_MONITOR_ALIGNED_RANGE_END(addr + size, exclusive);

 /* alignment may have changed starting or end addresses, size adjusted */
    aligned_size = aligned_end - aligned_addr;

    if (aligned_size) {
        state->addr = aligned_addr;
        state->size = aligned_size;
        BMRC_Checker_SetRange(hChecker, aligned_addr, aligned_size);
    } else {
        state->active = false;
    }

    return aligned_size>0;
}


static void
BMRC_P_Monitor_Program(BMRC_Monitor_Handle hMonitor, unsigned arc_no, BMMA_DeviceOffset addr, size_t size, const BMRC_Monitor_P_Clients *clients, bool exclusive, BMRC_AccessType accessType)
{
    BMRC_Checker_Handle hChecker = hMonitor->ahCheckers[arc_no];
    BMRC_Client client_id;

    BDBG_MSG(("%p: MEMC%d: programming ARC %u with " BDBG_UINT64_FMT "..." BDBG_UINT64_FMT "", (void *)hMonitor, hMonitor->mrcSettings.usMemcId, arc_no, BDBG_UINT64_ARG(addr), BDBG_UINT64_ARG(addr+size-1)));
#if 0
    /* useful for seeing client state when programming */
    {
    BMRC_Monitor_HwBlock_eCPU,
    BMRC_Monitor_HwBlock_eCPU,
        char buffer[256];
        BDBG_WRN(("ARC %d %s", arc_no, BMRC_P_Monitor_BuildClientIdString_isrsafe(clients, buffer, sizeof(buffer))));
    }
#endif
    if(!BMRC_P_Monitor_ProgramRange(hMonitor, arc_no, addr, size)) {
        return ;
    }


    /* program clients */

    for (client_id = 0; client_id < BMRC_Client_eMaxCount; client_id++)
    {
        BMRC_ClientInfo *client_info = &hMonitor->client_infos[client_id];

        if (client_info->eClient != BMRC_Client_eInvalid)
        {
            BMRC_Checker_SetClient(hChecker, (BMRC_Client)client_id, BMRC_P_MONITOR_CLIENT_MASK_IS_SET(clients->client_masks, client_id) ? BMRC_AccessType_eNone : BMRC_AccessType_eBoth);
        }
    }

    /* activate range checker */
    BMRC_Checker_SetAccessCheck(hChecker, BMRC_AccessType_eBoth);

    BMRC_Checker_SetBlock(hChecker, accessType);

    BMRC_Checker_SetExclusive(hChecker, exclusive);
    BMRC_Checker_Enable(hChecker);
    /* save state that was just programmed */
    {
        struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[arc_no];
        state->active = true;
        state->exclusive = exclusive;
        state->clients = *clients;
        state->accessType = accessType;
    }
    return ;
}

static void
BMRC_P_Monitor_ProgramAllocated(BMRC_Monitor_Handle hMonitor, unsigned arc_no, BMMA_DeviceOffset addr, size_t size, const BMRC_Monitor_P_Clients *clients)
{
    BMRC_AccessType accessType;
    /* assuming kernel is only on MEMC 0 */
    /*
    By default, block mode is set to BMRC_AccessType_eWrite for all memory violations.

    BMRC_AccessType_eWrite prevents memory corruption by HW cores that shouldn't access it.
    If you believe write access was blocked in error, please inspect BMRC_P_Monitor_astHwClients[] in bmrc_monitor_clients.c,
    BMRC_P_astClientTbl[] in bmrc_clienttable_priv.c

    Read will not be blocked, but you will see a BDBG_ERR if a violation occurs.

    This can be configured separately for kernel and non-kernel memory to block only reads, only writes, both, or neither
    when a violation occurs with the BMRC_Monitor_Settings structure used in BMRC_Monitor_Open().
    */
    if (hMonitor->mrcSettings.usMemcId == 0 && arc_no == 0 && hMonitor->protect_mem_low) {
        accessType = hMonitor->stSettings.eKernelBlockMode;
    } else {
        accessType = hMonitor->stSettings.eBlockMode;
    }
    BMRC_P_Monitor_Program(hMonitor, arc_no, addr, size, clients, false, accessType);
    return;
}

static void
BMRC_Monitor_P_Print_One_isrsafe(BMRC_Monitor_Handle hMonitor, unsigned i)
{
    const struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[i];
    char blocked[8];
#if BDBG_DEBUG_BUILD
    char bitmap[1+3*(BMRC_P_CLIENTS_MAX+7)/8];
#endif
    if(state->active) {
        BDBG_LOG(("ARC[%u%s]: " BDBG_UINT64_FMT "..." BDBG_UINT64_FMT " 0x%s", i, state->exclusive?" EXCL":"", BDBG_UINT64_ARG(state->addr), BDBG_UINT64_ARG(state->addr+state->size), BMRC_P_Monitor_BuildClientIdString_isrsafe(&state->clients, bitmap, sizeof(bitmap))));
        BKNI_Snprintf(blocked, sizeof(blocked),"%s%s", state->accessType&BMRC_AccessType_eWrite?"WR":"", state->accessType&BMRC_AccessType_eRead?" RD":"");
        BMRC_Monitor_P_PrintBitmap_isrsafe(&hMonitor->map, state->clients.client_masks, BMRC_P_MONITOR_CLIENT_MASK_ARRAY_SIZE, blocked);
    }
}

void
BMRC_Monitor_Print(BMRC_Monitor_Handle hMonitor)
{
    unsigned i;

    BDBG_LOG(("%p Range: " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", (void *)hMonitor, BDBG_UINT64_ARG(hMonitor->mem_low), BDBG_UINT64_ARG(hMonitor->mem_high)));
    for (i=0;i<sizeof(hMonitor->checkerState)/sizeof(hMonitor->checkerState[0]);i++) {
        BMRC_Monitor_P_Print_One_isrsafe(hMonitor, i);
    }
    return;
}


#if BDBG_DEBUG_BUILD
/* Monitor checker error messages are based on the SCB Protocol specifications at
   http://www.blr.broadcom.com/projects/DVT_BLR/Memc_Arch/. */
static void
BMRC_Monitor_P_PrintAllocation_isrsafe(const BMRC_Monitor_P_AllocatedRegion *region, const char *kind)
{
    if(region) {
        BDBG_WRN(("%s " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", kind, BDBG_UINT64_ARG(region->addr), BDBG_UINT64_ARG(region->addr + region->size)));
    }
}
#endif

const char *
BMRC_Monitor_GetRequestTypeName_isrsafe(unsigned requestType)
{
    unsigned i;

    for (i = 0; i < BMRC_P_MONITOR_SCB_ACCESS_TABLE_SIZE; i++)
    {
        if ((requestType & g_ScbCommandInfoTbl[i].ulMask) == g_ScbCommandInfoTbl[i].ulCommand)
        {
            return g_ScbCommandInfoTbl[i].pName;
        }
    }
    return "Unknown Command Type";
}

#if BDBG_DEBUG_BUILD
static void
BMRC_P_Monitor_Dump_isr(BMRC_Monitor_Handle hMonitor, unsigned arc_no, BMRC_CheckerInfo *pCheckerInfo)
{
    BMMA_DeviceOffset viol_start, viol_end;
    BMMA_DeviceOffset start;
    uint64_t size;
    BMRC_P_Monitor_ScbCommand eScbCommand = BMRC_P_Monitor_ScbCommand_eUnknown;
    unsigned i = 0;

    start = pCheckerInfo->ulStart;
    size = pCheckerInfo->ulSize;
    viol_start = pCheckerInfo->ulAddress;
    viol_end = pCheckerInfo->ulAddressEnd;

    BDBG_ERR(("Address Range Checker %d (ARC%d) has detected a memory access violation in MEMC%d", arc_no, arc_no, pCheckerInfo->usMemcId));

    if (pCheckerInfo->bExclusive)
    {
        BDBG_ERR(("violating access outside of exclusive range: " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", BDBG_UINT64_ARG(start), BDBG_UINT64_ARG(start+size-1)));
    }
    else
    {
        BDBG_ERR(("violation access in prohibited range: " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT "", BDBG_UINT64_ARG(start), BDBG_UINT64_ARG(start+size-1)));
    }

    BDBG_ERR(("violation start address: " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(viol_start)));
    BDBG_ERR(("violation end address:   " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(viol_end)));

    for (i = 0; i < BMRC_P_MONITOR_SCB_ACCESS_TABLE_SIZE; i++)
    {
        if ((pCheckerInfo->ulReqType & g_ScbCommandInfoTbl[i].ulMask) == g_ScbCommandInfoTbl[i].ulCommand)
        {
            eScbCommand = g_ScbCommandInfoTbl[i].eScbCommand;
            break;
        }
    }

    BDBG_ERR(("violation client: %d(%s)  request type: 0x%03x(%s)", pCheckerInfo->usClientId, pCheckerInfo->pchClientName,  pCheckerInfo->ulReqType, BMRC_Monitor_GetRequestTypeName_isrsafe(pCheckerInfo->ulReqType)));

    switch(eScbCommand)
    {
    uint32_t ulTransferSize, ulXSize, ulYLines;
    bool bFrameAccess;

    case BMRC_P_Monitor_ScbCommand_eLR:
    case BMRC_P_Monitor_ScbCommand_eLW:
#if BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50
    case BMRC_P_Monitor_ScbCommand_eLWWR:
#endif
    case BMRC_P_Monitor_ScbCommand_eCR:
        ulTransferSize = (pCheckerInfo->ulReqType & 0x01F)+1;
        BDBG_ERR(("transfer length %u bytes (%u Jwords)",
                 ulTransferSize * BMRC_P_MONITOR_JWORD_BYTES, ulTransferSize));
        break;

    case BMRC_P_Monitor_ScbCommand_eDR:
    case BMRC_P_Monitor_ScbCommand_eDW:
#if (BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50)
        ulTransferSize = (pCheckerInfo->ulReqType & 0x03F)+1;
#else
        ulTransferSize = (pCheckerInfo->ulReqType & 0x01F)+1;
#endif
        BDBG_ERR(("transfer length %u bytes (%u Gwords) %u NMBX %d",
                 ulTransferSize * BMRC_P_MONITOR_GWORD_BYTES, ulTransferSize,
                 pCheckerInfo->ulNmbx, pCheckerInfo->ulNmbx));
        break;

    case BMRC_P_Monitor_ScbCommand_eMR:
    case BMRC_P_Monitor_ScbCommand_eMW:
#if BMRC_MONITOR_P_SCB_PROTOCOL_VER >= 0x50
        ulXSize      = (pCheckerInfo->ulReqType & 0x040)? 4: 2;
#else
        ulXSize      = (pCheckerInfo->ulReqType & 0x040)? 2: 1;
#endif
        ulYLines     = (pCheckerInfo->ulReqType & 0x03E)+1;
        bFrameAccess = (pCheckerInfo->ulReqType & 0x001);

        BDBG_ERR(("X:%u bytes (%u Owords) Y:%u lines T:%s NMBX %u",
                 ulXSize * BMRC_P_MONITOR_OWORD_BYTES, ulXSize, ulYLines,
                 bFrameAccess ? "frame access" : "field access",
                 pCheckerInfo->ulNmbx));
        break;

    default:
        break;
    }

#if 0
    for(cur=BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions);cur;cur=BLST_AA_TREE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions,cur))
    {
        if (((viol_start >= cur->addr) && (viol_start <= (cur->addr + cur->size-1))) ||
            ((viol_end >= cur->addr) && (viol_end <= (cur->addr + cur->size-1))))
        {
            BDBG_ERR(("violated region: " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT ",", BDBG_UINT64_ARG(cur->addr), BDBG_UINT64_ARG(cur->addr + cur->size-1)));
            BDBG_ERR(("allocated by %s:%d", cur->fname, cur->line));
        }
    }
#endif

#if 0
    /* useful for seeing state of all regions at interrupt time */
    nmm{
        int i;
        char buffer[256];
        const BMRC_Monitor_P_AllocatedRegion *cur;
        for(i=0,cur=BLST_Q_FIRST(&hMonitor->regions);cur;cur=BLST_Q_NEXT(cur, list)) {
            BDBG_WRN(("region %d, %x..%x, clients %s", i++, cur->addr, cur->addr + cur->size,
            BMRC_P_Monitor_BuildClientIdString_isrsafe(&cur->clients, buffer, sizeof(buffer))));
        }
    }
#endif
    BMRC_Monitor_P_Print_One_isrsafe(hMonitor, arc_no);
    if(arc_no < hMonitor->max_ranges - hMonitor->numberOfUserRanges) {
        BMRC_Monitor_P_AllocatedRegion *region;
        region = BLST_AA_TREE_FIND_SOME(BMRC_Monitor_P_AllocatedRegionTree,&hMonitor->regions, viol_start);
        if(region) {
            const BMRC_Monitor_P_AllocatedRegion *next = BLST_AA_TREE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions,region);
            const BMRC_Monitor_P_AllocatedRegion *prev = BLST_AA_TREE_PREV(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->regions,region);
            BMRC_Monitor_P_PrintAllocation_isrsafe(prev, "prev");
            BMRC_Monitor_P_PrintAllocation_isrsafe(region, "match");
            BMRC_Monitor_P_PrintAllocation_isrsafe(next, "next");
        }

    }
    return;
}
#else
static void
BMRC_P_Monitor_Dump_isr(BMRC_Monitor_Handle hMonitor, unsigned arc_no, BMRC_CheckerInfo *pCheckerInfo)
{
    BSTD_UNUSED(hMonitor);
    BSTD_UNUSED(arc_no);
    BSTD_UNUSED(pCheckerInfo);
}
#endif

static void
BMRC_P_Monitor_UpdateSingle(BMRC_Monitor_Handle hMonitor, unsigned range)
{
    BMRC_P_Monitor_CombinedRegionVerify(hMonitor);
    hMonitor->stats.updates.single++;
    if(BMRC_P_Monitor_ProgramRange(hMonitor, range, hMonitor->combined_regions[range].addr, hMonitor->combined_regions[range].size)) {
        struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[range];
        state->active = true;
        BMRC_Checker_Enable(hMonitor->ahCheckers[range]);
    }
    return;
}

/* this function updates current range checker programming based on current memory map and jail */
static void
BMRC_P_Monitor_UpdateFull(BMRC_Monitor_Handle hMonitor)
{
    unsigned i;
    unsigned max_ranges;
    BDBG_ASSERT(hMonitor->max_ranges >= hMonitor->numberOfUserRanges);
    max_ranges = hMonitor->max_ranges - hMonitor->numberOfUserRanges;
    hMonitor->stats.updates.full++;
    for(i=0;i<max_ranges;i++) {
        BMRC_P_Monitor_Disable(hMonitor, i);
    }
    if (!hMonitor->enabled) {
        return;
    }

    if (BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->custom_regions)) {
        BMRC_Monitor_P_AllocatedRegion *region;

        BDBG_MSG_TRACE(("use custom ARC mapping"));
        for(i=0,region=BLST_AA_TREE_FIRST(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->custom_regions);region;i++,region=BLST_AA_TREE_NEXT(BMRC_Monitor_P_AllocatedRegionTree, &hMonitor->custom_regions,region)) {
            BMMA_DeviceOffset size = BMRC_P_MONITOR_ALIGNED_RANGE_END(region->addr+region->size, false) -
                BMRC_P_MONITOR_ALIGNED_RANGE_START(region->addr, false);

            if (size == 0)
            {
                BDBG_WRN(("aligned size " BDBG_UINT64_FMT " is 0, ARC %u will not be enabled", BDBG_UINT64_ARG(size), i));
            }
            BMRC_P_Monitor_ProgramAllocated(hMonitor, i, region->addr, region->size, &hMonitor->clients);
        }
        goto done;
    }

    if(1)
    {
        unsigned threshold = hMonitor->last_threshold;
        unsigned num_regions = hMonitor->num_combined_regions;
        hMonitor->partial_update_count = 0;
        threshold /= 2;
        threshold = threshold > BMRC_P_MONITOR_JWORD_BYTES ? threshold : BMRC_P_MONITOR_JWORD_BYTES;

        for(;threshold<hMonitor->mem_high - hMonitor->mem_low;threshold=threshold*2) {
            bool success = BMRC_P_Monitor_Combine(hMonitor, threshold, max_ranges, &num_regions);

            BDBG_MSG_TRACE(("combine %u regions status %d(%#x)", num_regions, (int)success, threshold));

            for(i=0;i<num_regions;i++) {
                BDBG_MSG_TRACE(("region %d addr=%#x size=%#x", i, (unsigned)hMonitor->combined_regions[i].addr, (unsigned)hMonitor->combined_regions[i].size));
            }

            if (success) {
                break;
            }
        }
        hMonitor->num_combined_regions = num_regions;
        hMonitor->last_threshold = threshold;
        BMRC_P_Monitor_CombinedRegionVerify(hMonitor);
    }

    for(i=0;i<hMonitor->num_combined_regions;i++) {
        BMRC_P_Monitor_ProgramAllocated(hMonitor, i, hMonitor->combined_regions[i].addr, hMonitor->combined_regions[i].size, &hMonitor->clients);
    }

done:
    return;
}

void
BMRC_Monitor_GetMemoryInterface(BMRC_Monitor_Handle hMonitor, BMRC_MonitorInterface *pInterface)
{
    pInterface->cnxt = hMonitor;
    pInterface->alloc = BMRC_P_Monitor_Alloc;
    pInterface->free = BMRC_P_Monitor_Free;
}

static void
BMRC_P_Monitor_isr( void *cnxt, int no, BMRC_CheckerInfo *pInfo)
{
    BMRC_Monitor_Handle hMonitor = cnxt;
    BERR_Code rc;

    rc = BMRC_Checker_DisableCallback_isr(hMonitor->ahCheckers[no]);
    if (rc !=BERR_SUCCESS) {
        BDBG_ERR(("BMRC_Checker_DisableCallback_isr failed with rc %#x, ignored", (unsigned)rc));
    }
    hMonitor->checkerState[no].interruptDisabled = true;
    BMRC_P_Monitor_Dump_isr(hMonitor, no, pInfo);

    return;
}

void BMRC_Monitor_RestoreInterrupts(BMRC_Monitor_Handle hMonitor)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(hMonitor, BMRC_Monitor);
    BKNI_AcquireMutex(hMonitor->pMutex);
    BKNI_EnterCriticalSection();
    for (i=0;i<sizeof(hMonitor->checkerState)/sizeof(hMonitor->checkerState[0]);i++) {
        struct BMRC_Monitor_P_CheckerState *state = &hMonitor->checkerState[i];
        if(state->interruptDisabled && hMonitor->ahCheckers[i]) {
            BDBG_MSG(("BMRC_Monitor_RestoreInterrupts:%p Enable interrupt for %u", (void *)hMonitor, i));
            state->interruptDisabled = false;
            BMRC_Checker_EnableCallback_isr(hMonitor->ahCheckers[i]);
        }
    }
    BKNI_LeaveCriticalSection();
    BKNI_ReleaseMutex(hMonitor->pMutex);
    return;
}

void BMRC_MonitorRegion_GetDefaultSettings(BMRC_MonitorRegion_Settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->blockRead = true;
    settings->blockWrite = true;
    settings->exclusive = false;
    settings->listType = BMRC_Monitor_ListType_eSpecifiedClients;
    return;
}

static void BMRC_MonitorRegion_P_Program( BMRC_Monitor_Handle hMonitor, BMRC_MonitorRegion_Handle hRegion, unsigned arc_no)
{
    struct BMRC_Monitor_P_ClientList clientList;
    BMRC_Monitor_P_Clients clients;
    BMRC_AccessType accessType = BMRC_AccessType_eNone;

    BKNI_Memset(&clientList,0,sizeof(clientList));
    if(hRegion->settings.listType == BMRC_Monitor_ListType_eSpecifiedClients) {
        BMRC_Monitor_P_SetHwBlocks(&hMonitor->map, hRegion->hwBlocks, &clientList, true);
    } else {
        unsigned i;
        for(i=0;i<sizeof(clientList.clients)/sizeof(clientList.clients[0]);i++) {
            clientList.clients[i] = true;
        }
        BMRC_Monitor_P_SetHwBlocks(&hMonitor->map, hRegion->hwBlocks, &clientList, false);
    }
    BMRC_P_Monitor_MasksClear(&clients);
    BMRC_P_Monitor_BuildMaskFromList(&clientList, &clients);
    BDBG_MODULE_MSG(BMRC_MonitorRegion,("Program: %p arc:%u " BDBG_UINT64_FMT "..." BDBG_UINT64_FMT "",(void *)hRegion, arc_no, BDBG_UINT64_ARG(hRegion->settings.addr), BDBG_UINT64_ARG(hRegion->settings.addr +hRegion->settings.length)));
    if(hRegion->settings.blockRead) {
        accessType |= BMRC_AccessType_eRead;
    }
    if(hRegion->settings.blockWrite) {
        accessType |= BMRC_AccessType_eWrite;
    }
    BMRC_P_Monitor_Program(hMonitor, arc_no, hRegion->settings.addr, hRegion->settings.length, &clients, hRegion->settings.exclusive, accessType);
    return;
}

BERR_Code BMRC_MonitorRegion_Add( BMRC_Monitor_Handle hMonitor, BMRC_MonitorRegion_Handle *phRegion, const BMRC_MonitorRegion_Settings *settings, const BMRC_Monitor_HwBlock *clientList, size_t clientListLength)
{
    BMRC_MonitorRegion_Handle hRegion;
    size_t rangeSize;
    unsigned arc_no;
    unsigned i;
    BDBG_OBJECT_ASSERT(hMonitor, BMRC_Monitor);
    if(phRegion == NULL || settings == NULL || (clientListLength !=0 &&clientList==NULL)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *phRegion = NULL;
    for(i=0;i<clientListLength;i++) {
        if(clientList[i] >= BMRC_Monitor_HwBlock_eInvalid) {
            return BERR_TRACE(BERR_INVALID_PARAMETER); /* BMRC_Monitor_HwBlock_eInvalid can't be used in middle of the list */
        }
    }
    if(hMonitor->numberOfUserRanges >= hMonitor->max_ranges) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    rangeSize = sizeof(*hRegion) + clientListLength * sizeof(*hRegion->hwBlocks);
    hRegion = BKNI_Malloc(rangeSize);
    if(hRegion==NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_AcquireMutex(hMonitor->pMutex);
    BDBG_OBJECT_INIT(hRegion, BMRC_MonitorRegion);
    hMonitor->numberOfUserRanges++;
    arc_no = hMonitor->max_ranges - hMonitor->numberOfUserRanges; /* allocate from the highest index */
    BMRC_P_Monitor_Disable(hMonitor, arc_no); /* disable selected range */
    BMRC_P_Monitor_UpdateFull(hMonitor); /* rebuild configuration based on smaller number of available ranged */
    hRegion->settings = *settings;
    for(i=0;i<clientListLength;i++) {
        hRegion->hwBlocks[i] = clientList[i];
    }
    hRegion->hwBlocks[clientListLength] = BMRC_Monitor_HwBlock_eInvalid; /* set terminator */
    BDBG_MODULE_MSG(BMRC_MonitorRegion,("Add: %p new region %p arc:%u",(void *)hMonitor, (void *)hRegion, arc_no));
    BMRC_MonitorRegion_P_Program( hMonitor, hRegion, arc_no);
    BLST_Q_INSERT_HEAD(&hMonitor->userRegions, hRegion, link);
    *phRegion = hRegion;
    BKNI_ReleaseMutex(hMonitor->pMutex);
    return BERR_SUCCESS;
}


void BMRC_MonitorRegion_Remove(BMRC_Monitor_Handle hMonitor, BMRC_MonitorRegion_Handle hRegion)
{
    BMRC_MonitorRegion_Handle cur;
    unsigned arc_no;

    BDBG_OBJECT_ASSERT(hMonitor, BMRC_Monitor);
    BDBG_OBJECT_ASSERT(hRegion, BMRC_MonitorRegion);
    for(arc_no=hMonitor->max_ranges-1,cur=BLST_Q_LAST(&hMonitor->userRegions);cur;cur=BLST_Q_PREV(cur, link),arc_no--) {
        if(cur==hRegion) {
            break;
        }
    }
    if(cur==NULL) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED); /* this region doesn't exist in the monitor */
        return;
    }
    BDBG_MODULE_MSG(BMRC_MonitorRegion,("Remove: %p remove %p arc:%u",(void *)hMonitor, (void *)hRegion, arc_no));
    BKNI_AcquireMutex(hMonitor->pMutex);
    BMRC_P_Monitor_Disable(hMonitor, arc_no); /* disable deleted region */
    for(cur=BLST_Q_PREV(cur,link);cur;cur=BLST_Q_PREV(cur, link)) { /* move regions up */
        BDBG_ASSERT(arc_no>0);
        arc_no--;
        BDBG_MODULE_MSG(BMRC_MonitorRegion,("Remove: %p move %p arc:%u",(void *)hMonitor, (void *)cur, arc_no));
        BMRC_P_Monitor_Disable(hMonitor, arc_no); /* disable old region */
        BMRC_MonitorRegion_P_Program( hMonitor, cur, arc_no+1);
    }
    BMRC_MonitorRegion_P_Remove(hMonitor, hRegion);
    BMRC_P_Monitor_UpdateFull(hMonitor); /* rebuild configuration based on smaller number of available ranged */
    BKNI_ReleaseMutex(hMonitor->pMutex);
    return;
}

BMRC_Monitor_HwBlock BMRC_Monitor_GetHwBlock( BMRC_Monitor_Handle hMonitor, BCHP_MemcClient clientId )
{
    BMRC_Client client = BMRC_P_GET_CLIENT_ENUM_isrsafe(hMonitor->mrcSettings.usMemcId, clientId);
    if (client < BMRC_Client_eInvalid) {
        unsigned i;
        for(i=0;i<sizeof(hMonitor->map.clientMap)/sizeof(hMonitor->map.clientMap[0]);i++) {
            if (hMonitor->map.clientMap[i].client == client) {
                return hMonitor->map.clientMap[i].block;
            }
        }
    }
    return BMRC_Monitor_HwBlock_eInvalid;
}

void BMRC_Monitor_SetEnabled( BMRC_Monitor_Handle hMonitor, bool enabled )
{
    BDBG_OBJECT_ASSERT(hMonitor, BMRC_Monitor);
    if (hMonitor->enabled != enabled) {
        hMonitor->enabled = enabled;
        BMRC_P_Monitor_UpdateFull(hMonitor);
    }

}

/* End of file */
