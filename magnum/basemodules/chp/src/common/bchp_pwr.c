/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bchp_pwr.h"
#ifdef BCHP_PWR_HAS_RESOURCES
#include "bchp_pwr_resources_priv.h"
#endif


/* This file contains generic implementation of public APIs */

BDBG_MODULE(BCHP_PWR);


#ifdef BCHP_PWR_HAS_RESOURCES

#define BDBG_MSG_TRACE(x) /*BDBG_MSG(x)*/

extern const BCHP_PWR_P_Resource **BCHP_PWR_P_DependList[];
extern const BCHP_PWR_P_Resource *BCHP_PWR_P_ResourceList[];
extern BCHP_PWR_P_FreqMap BCHP_PWR_P_FreqMapList[];

#if BCHP_PWR_SUPPORT
static BERR_Code BCHP_PWR_P_AcquireResource(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool from_init);
static BERR_Code BCHP_PWR_P_ReleaseResource(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool from_init);
void BCHP_PWR_P_Init(BCHP_Handle handle);
unsigned BCHP_PWR_P_GetInternalIndex(BCHP_PWR_ResourceId resourceId);
const BCHP_PWR_P_Resource* BCHP_PWR_P_GetResourceHandle(BCHP_PWR_ResourceId resourceId);
#else
static BERR_Code BCHP_PWR_P_AcquireResource(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool from_init);
void BCHP_PWR_P_InitOn(BCHP_Handle handle);
#endif


BERR_Code BCHP_PWR_Open(BCHP_PWR_Handle *pHandle, BCHP_Handle chp)
{
    BCHP_PWR_Handle pDev = NULL;
    unsigned size;
    BERR_Code rc;

#if BCHP_PWR_SUPPORT
    BDBG_ASSERT(pHandle);
#else
	BSTD_UNUSED(pHandle);
#endif
    BDBG_ASSERT(chp);
    BDBG_ASSERT(chp->regHandle);

    size = sizeof(unsigned)*BCHP_PWR_P_NUM_ALLNODES;
    pDev = BKNI_Malloc(sizeof(BCHP_PWR_P_Context));
    if (!pDev) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;

    }
    BKNI_Memset(pDev, 0, sizeof(BCHP_PWR_P_Context));

    rc = BKNI_CreateMutex(&(pDev->lock));
    if (rc != BERR_SUCCESS) {
#if BDBG_DEBUG_BUILD || B_REFSW_DEBUG_COMPACT_ERR
        BERR_TRACE(rc);
#endif
        goto error;
    }

    pDev->pubRefcnt = BKNI_Malloc(size);
    if (!pDev->pubRefcnt) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->pubRefcnt, 0, size);

    pDev->privRefcnt = BKNI_Malloc(size);
    if (!pDev->privRefcnt) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->privRefcnt, 0, size);

    size = sizeof(bool)*BCHP_PWR_P_NUM_ALLNODES;
    pDev->init = BKNI_Malloc(size);
    if (!pDev->init) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->init, 0, size);

    pDev->magnumCtrl = BKNI_Malloc(size);
    if (!pDev->magnumCtrl) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->magnumCtrl, 0, size);

    pDev->sharedCtrl = BKNI_Malloc(size);
    if (!pDev->sharedCtrl) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->sharedCtrl, 0, size);

    pDev->secureCtrl = BKNI_Malloc(size);
    if (!pDev->secureCtrl) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    BKNI_Memset(pDev->secureCtrl, 0, size);

#if BCHP_PWR_SUPPORT
    BDBG_WRN(("BCHP_PWR POWER MANAGEMENT ENABLED"));
    BDBG_MSG(("BCHP_PWR_Open: Non-HW %u, HW %u, Non-Leaf-HW %u", BCHP_PWR_P_NUM_NONLEAFS, BCHP_PWR_P_NUM_LEAFS, BCHP_PWR_P_NUM_NONLEAFSHW));

    chp->pwrManager = pDev;
    BCHP_PWR_P_Init(chp);

    *pHandle = pDev;
    return BERR_SUCCESS;
#else
    BDBG_WRN(("\n"));
    BDBG_WRN(("****************************************"));
    BDBG_WRN(("****************************************"));
    BDBG_WRN(("** BCHP_PWR POWER MANAGEMENT DISABLED **"));
    BDBG_WRN(("****************************************"));
    BDBG_WRN(("****************************************"));
    BDBG_WRN(("\n"));
#if !defined(EMULATION)
    /* turn on power for everything and never touch it again. this guarantees an initial power state similar to a chip-reset */
    chp->pwrManager = pDev;
    BCHP_PWR_P_InitOn(chp);
#endif
    chp->pwrManager = NULL;
    rc = BERR_SUCCESS;
    /* keep going */
#endif

error:
    if (pDev) {
        if (pDev->lock) {
            BKNI_DestroyMutex(pDev->lock);
        }
        if (pDev->pubRefcnt) {
            BKNI_Free(pDev->pubRefcnt);
        }
        if (pDev->privRefcnt) {
            BKNI_Free(pDev->privRefcnt);
        }
        if (pDev->init) {
            BKNI_Free(pDev->init);
        }
        if (pDev->magnumCtrl) {
            BKNI_Free(pDev->magnumCtrl);
        }
        if (pDev->sharedCtrl) {
            BKNI_Free(pDev->sharedCtrl);
        }
        if (pDev->secureCtrl) {
            BKNI_Free(pDev->secureCtrl);
        }
        BKNI_Free(pDev);
    }
    return rc;
}

void BCHP_PWR_P_Init(BCHP_Handle handle)
{
#if BCHP_PWR_SUPPORT
    const BCHP_PWR_P_Resource *resource = NULL;
    unsigned i, idx;
    unsigned refcnt[BCHP_PWR_P_NUM_ALLNODES];

    BDBG_MSG(("BCHP_PWR_P_Init: >"));
    BKNI_Memset(refcnt, 0, sizeof(refcnt));

    /* power up all HW resources to guarantee an initial power state (everything powered up; this mimics a board reset).
       this is done in two steps:
       1) do an Acquire on MAGNUM_CONTROLLED node (with normal refcnt on all nodes)
       2) go through the rest of the HW nodes (refcnt==0) and do an Acquire */

#ifdef BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED
    resource = BCHP_PWR_P_GetResourceHandle(BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED);
    (void)BCHP_PWR_P_AcquireResource(handle, resource, true);
    /* figure out which HW_ nodes are MAGNUM_CONTROLLED */
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        refcnt[idx] = handle->pwrManager->privRefcnt[idx];
        if (handle->pwrManager->privRefcnt[idx]>0) {
            handle->pwrManager->magnumCtrl[idx] = true;
        }
    }
#endif

    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (handle->pwrManager->privRefcnt[idx]==0) {
            (void)BCHP_PWR_P_AcquireResource(handle, resource, true);
        }
    }
    /* figure out which MAGNUM_CONTROLLED HW_ nodes also have a non-MAGNUM_CONTROLLED HW_ node parent */
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (refcnt[idx]>0 && handle->pwrManager->privRefcnt[idx]>refcnt[idx]) {
            handle->pwrManager->sharedCtrl[idx] = true;
            BDBG_MSG(("Shared HW_ node %#x (%s)", resource->id, resource->name));
        }
        refcnt[idx] = handle->pwrManager->privRefcnt[idx];
    }


#ifdef BCHP_PWR_RESOURCE_SECURE_ACCESS
    resource = BCHP_PWR_P_GetResourceHandle(BCHP_PWR_RESOURCE_SECURE_ACCESS);
    (void)BCHP_PWR_P_AcquireResource(handle, resource, true);
    /* figure out which HW_ nodes are SECURE_ACCESS */
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (refcnt[idx]>0 && handle->pwrManager->privRefcnt[idx]>refcnt[idx]) {
            /* Mark the SECURE ACCESS NODES as initialized */
            handle->pwrManager->init[idx] = true;
            handle->pwrManager->secureCtrl[idx] = true;
            BDBG_MSG(("Secure Access HW_ node %#x (%s)", resource->id, resource->name));
        }
    }

    /* Release secure resource that are not selected by mux */
#ifdef BCHP_PWR_P_NUM_MUXES
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if(handle->pwrManager->secureCtrl[idx] && resource->type == BCHP_PWR_P_ResourceType_eMux) {
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            unsigned mux, cnt=0;
            BDBG_ASSERT(resources);

            BCHP_PWR_P_MUX_Control(handle, resource, &mux, false);
            for (;*resources!=NULL;resources++,cnt++) {
                if(cnt != mux)
                    BCHP_PWR_P_ReleaseResource(handle, *resources, true);
            }
        }
    }
#endif
#endif

#if 0 /* debug dump */
    BCHP_PWR_Dump(handle);
#endif

    /* power down MAGNUM_CONTROLLED node and mark its HW nodes as initialized.
       the rest of the HW nodes are initialized via a call to InitAllHwResources() from upper-level SW */

#ifdef BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED
    resource = BCHP_PWR_P_GetResourceHandle(BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED);
    BCHP_PWR_P_ReleaseResource(handle, resource, true);
#endif

    /* at this point all MAGNUM_CONTROLLED HW nodes are init=1 and refcnt=0.
       all other HW nodes are init=0 and refcnt>0. */
    handle->pwrManager->initComplete = true;
    BDBG_MSG(("BCHP_PWR_P_Init: <"));

#if 0 /* debug dump */
    BCHP_PWR_Dump(handle);
#endif

#else /* BCHP_PWR_SUPPORT */
    BSTD_UNUSED(handle);
#endif
}

void BCHP_PWR_P_InitOn(BCHP_Handle handle)
{
#if (!BCHP_PWR_SUPPORT)
    unsigned i;
    const BCHP_PWR_P_Resource *resource = NULL;
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        (void)BCHP_PWR_P_AcquireResource(handle, resource, true);
    }
    return;
#else
    BSTD_UNUSED(handle);
#endif
}

void BCHP_PWR_Close(BCHP_PWR_Handle handle)
{
#if BCHP_PWR_SUPPORT
    unsigned i;
    bool clean = true;
    BDBG_ASSERT(handle);
    BDBG_MSG(("BCHP_PWR_Close: >"));

    for (i=0; i<BCHP_PWR_P_NUM_NONLEAFS; i++) {
        if (handle->pubRefcnt[i]) {
            BDBG_WRN(("BCHP_PWR_Close: Resource %#x (%s) still acquired (refcnt %u)",
                BCHP_PWR_P_ResourceList[i]->id, BCHP_PWR_P_ResourceList[i]->name, handle->pubRefcnt[i]));
            clean = false;
        }
    }

    /* sanity check: if public refcnts are all 0, then priv refcnts should also be all 0 */
    if (clean) {
        for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
            if (handle->privRefcnt[i]>0) {
                if (!handle->sharedCtrl[i] && handle->magnumCtrl[i] && !handle->secureCtrl[i]) {
                    BDBG_ERR(("BCHP_PWR_Close: HW resource %#x (%s) has non-zero refcnt %u",
                    BCHP_PWR_P_ResourceList[i]->id, BCHP_PWR_P_ResourceList[i]->name, handle->privRefcnt[i]));
                }
            }
        }
    }

    BKNI_DestroyMutex(handle->lock);
    BKNI_Free(handle->pubRefcnt);
    BKNI_Free(handle->privRefcnt);
    BKNI_Free(handle->init);
    BKNI_Free(handle->magnumCtrl);
    BKNI_Free(handle->sharedCtrl);
    BKNI_Free(handle->secureCtrl);
    BKNI_Free(handle);
    BDBG_MSG(("BCHP_PWR_Close: <"));
#else /* BCHP_PWR_SUPPORT */
    BSTD_UNUSED(handle);
#endif
}

#if 0 /* unused */
/* returns true if "resource" is a dependency of another HW node */
static bool BCHP_PWR_P_IsParentHwNode(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource)
{
    bool rc = false;
    BSTD_UNUSED(handle);

    if (resource->type==BCHP_PWR_P_ResourceType_eLeaf || resource->type==BCHP_PWR_P_ResourceType_eNonLeafHw) {
        unsigned i;
        /* go through all HW nodes and see if "resource" is a dependant of any */
        for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
            const BCHP_PWR_P_Resource **res = BCHP_PWR_P_DependList[i];
            if (res) {
                for (;*res!=NULL;res++) {
                    if (*res == resource) {
                        return true;
                    }
                }
            }
        }
    }
    return rc;
}
#endif

void BCHP_PWR_InitAllHwResources(BCHP_Handle handle)
{
#if BCHP_PWR_SUPPORT
    const BCHP_PWR_P_Resource *resource;
    unsigned i, idx;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(handle->pwrManager);
    BDBG_MSG(("BCHP_PWR_InitAllHwResources: >"));

    BKNI_AcquireMutex(handle->pwrManager->lock);

    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type!=BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);

        /* there can be 4 situations here:
           init=1, refcnt =0 => already initialized as part of MAGNUM_CONTROLLED node
           init=1, refcnt!=0 => assert
           init=0, refcnt =0 => assert
           init=0, refcnt!=0 => sharedCtrl or Magnum Controlled node that was not released. power down */

        if (!handle->pwrManager->init[idx]) {
            BDBG_ASSERT(handle->pwrManager->privRefcnt[idx]!=0);
            (void)BCHP_PWR_P_ReleaseResource(handle, resource, true); /* power down */
        } else {
            if (!handle->pwrManager->secureCtrl[idx])
                BDBG_ASSERT(handle->pwrManager->privRefcnt[idx]==0);
        }
    }

    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (handle->pwrManager->privRefcnt[idx]) {
            if (!handle->pwrManager->secureCtrl[idx])
                BDBG_WRN(("HW resource %#x (%s) not powered down", resource->id, resource->name));
        }
    }

    BKNI_ReleaseMutex(handle->pwrManager->lock);
    BDBG_MSG(("BCHP_PWR_InitAllHwResources: <"));

#if 0 /* debug dump */
    BCHP_PWR_Dump(handle);
#endif

#else /* BCHP_PWR_SUPPORT */
    BSTD_UNUSED(handle);
#endif
}

/* the #define ID's for both non-leaf nodes start at 1, not 0. for HW leaf nodes, it starts at 0xff000001.
internally, we number them starting at 0 with the non-HW nodes, followed by HW nodes. */
unsigned BCHP_PWR_P_GetInternalIndex(BCHP_PWR_ResourceId resourceId)
{
    unsigned index;
    if ((resourceId & 0xff000000)==0xff000000) { /* HW */
        index = resourceId - 0xff000000 - 1 + BCHP_PWR_P_NUM_NONLEAFS;
        BDBG_ASSERT(index < BCHP_PWR_P_NUM_ALLNODES);
    }
    else {
        index = resourceId - 1;
        BDBG_ASSERT(index < BCHP_PWR_P_NUM_NONLEAFS);
    }

    return index;
}

const BCHP_PWR_P_Resource* BCHP_PWR_P_GetResourceHandle(BCHP_PWR_ResourceId resourceId)
{
    unsigned index = BCHP_PWR_P_GetInternalIndex(resourceId);
    return BCHP_PWR_P_ResourceList[index];
}

/* Private functions to power On analog resources when BCHP_PWR_SUPPORT is not enabled */
#if (!BCHP_PWR_SUPPORT)
void BCHP_PWR_P_Acquire(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
#ifdef BCHP_PWR_RESOURCE_AUD_DAC
#ifdef BCHP_PWR_HW_AUD_DAC
    if(resourceId == BCHP_PWR_RESOURCE_AUD_DAC)
        BCHP_PWR_P_HW_ControlId(handle, BCHP_PWR_HW_AUD_DAC, true);
#endif
#endif
}

void BCHP_PWR_P_Release(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
#ifdef BCHP_PWR_RESOURCE_AUD_DAC
#ifdef BCHP_PWR_HW_AUD_DAC
    if(resourceId == BCHP_PWR_RESOURCE_AUD_DAC)
        BCHP_PWR_P_HW_ControlId(handle, BCHP_PWR_HW_AUD_DAC, false);
#endif
#endif
}
#endif

static BERR_Code BCHP_PWR_P_AcquireResource(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool from_init)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned idx = BCHP_PWR_P_GetInternalIndex(resource->id);
    int ref_cnt = handle->pwrManager->privRefcnt[idx];
    bool init = handle->pwrManager->init[idx];
    BDBG_MSG_TRACE(("  Acquire %#x (%s): %u", resource->id, resource->name, ref_cnt));

    if (!init && !from_init) { /* uninitialized HW node */
        if (resource->type==BCHP_PWR_P_ResourceType_eLeaf || resource->type==BCHP_PWR_P_ResourceType_eNonLeafHw) {
            if (handle->pwrManager->sharedCtrl[idx] || !handle->pwrManager->magnumCtrl[idx] ) {
                handle->pwrManager->init[idx] = true; /* mark as init and let refcnt increment */
            }
            else {
                BDBG_WRN(("HW resource %#x (%s) must be initialized before being acquired", resource->id, resource->name));
                return BERR_NOT_INITIALIZED;
            }
        }
    }

    switch(resource->type) {
        case BCHP_PWR_P_ResourceType_eMux:
        {
#ifdef BCHP_PWR_P_NUM_MUXES
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            unsigned mux, cnt=0;
            BDBG_ASSERT(resources);

            BCHP_PWR_P_MUX_Control(handle, resource, &mux, false);
            for (;*resources!=NULL;resources++,cnt++) {
                if((cnt == mux) || from_init)
                    rc = BCHP_PWR_P_AcquireResource(handle, *resources, from_init);
            }
#endif
            break;
        }
        case BCHP_PWR_P_ResourceType_eNonLeaf:
        case BCHP_PWR_P_ResourceType_eNonLeafHw:
        case BCHP_PWR_P_ResourceType_eDiv:
        {
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            if(resource->type == BCHP_PWR_P_ResourceType_eDiv) {
                /* Initialize clock frequency */
#ifdef BCHP_PWR_P_NUM_DIVS
                if (ref_cnt==0) {
                    if (from_init) {
                        unsigned i;
                        const BCHP_PWR_P_DivTable *divTable = NULL;
                        for (i=0; i<BCHP_PWR_P_NUM_DIVS; i++) {
                            if (resource->id == BCHP_PWR_P_FreqMapList[i].id) {
                                divTable = BCHP_PWR_P_FreqMapList[i].pDivTable;
                                if(divTable[0].postdiv) {
                                    BDBG_MSG(("Init %s : Mult %d, Prediv %d, Postdiv %d", resource->name, divTable[0].mult, divTable[0].prediv, divTable[0].postdiv));
                                    BCHP_PWR_P_DIV_Control(handle, resource, (unsigned *)&divTable[0].mult, (unsigned *)&divTable[0].prediv, (unsigned *)&divTable[0].postdiv, true);
                                }
                                break;
                            }
                        }
                    }
                }
#endif
                /* Leaf Divisor */
                if(resources == NULL) {
                    break;
                }
            }
            BDBG_ASSERT(resources);

            for (;*resources!=NULL;resources++) {
                rc |= BCHP_PWR_P_AcquireResource(handle, *resources, from_init);
            }
            if (resource->type==BCHP_PWR_P_ResourceType_eNonLeaf || resource->type==BCHP_PWR_P_ResourceType_eDiv) {
                break;
            }
            /* otherwise fall-through. for Acquire(), the recursive Acquire() must occur before activating the leaf HW node */
        }
        case BCHP_PWR_P_ResourceType_eLeaf:
        {
            if (ref_cnt==0) {
                BDBG_MSG_TRACE(("  Activate HW node %#x (%s)", resource->id, resource->name));
                BCHP_PWR_P_HW_Control(handle, resource, true);
            }
            break;
        }
    }

    handle->pwrManager->privRefcnt[idx]++;
    return rc;
}

#if BCHP_PWR_SUPPORT
static BERR_Code BCHP_PWR_P_ReleaseResource(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool from_init)
{
    BERR_Code rc = BERR_SUCCESS;
    unsigned idx = BCHP_PWR_P_GetInternalIndex(resource->id);
    int ref_cnt = handle->pwrManager->privRefcnt[idx];
    bool init = handle->pwrManager->init[idx];
    BDBG_MSG_TRACE(("  Release %#x (%s): %u", resource->id, resource->name, ref_cnt));

    if (!init && !from_init) { /* uninitialized HW node */
        if (resource->type==BCHP_PWR_P_ResourceType_eLeaf || resource->type==BCHP_PWR_P_ResourceType_eNonLeafHw) {
            BDBG_WRN(("HW resource %#x (%s) must be initialized before being released", resource->id, resource->name));
            BDBG_ASSERT(0); /* should not be reachable via public API */
            return BERR_NOT_INITIALIZED;
        }
        /* !init && from_init is the case when we're trying to initialize from P_Init()
           init && !from_init is the case when we're trying to release via public API
           init && from_init  means re-initializing, which is harmless. e.g. HW node with a HW parent */
    }

    if (ref_cnt==0) {
        BDBG_ERR(("Cannot release resource %#x (%s) with refcnt 0", resource->id, resource->name));
        BDBG_ASSERT(0); /* should not be reachable via public API */
        return BERR_NOT_INITIALIZED;
    }

    ref_cnt = --(handle->pwrManager->privRefcnt[idx]);

    switch(resource->type) {
        case BCHP_PWR_P_ResourceType_eMux:
        {
#ifdef BCHP_PWR_P_NUM_MUXES
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            unsigned mux, cnt=0;
            BDBG_ASSERT(resources);

            if (ref_cnt==0) {
                if(from_init) {
                    BDBG_MSG(("Initialize MUX resource %#x (%s)", resource->id, resource->name));
                    handle->pwrManager->init[idx] = true;
                }
            }

            BCHP_PWR_P_MUX_Control(handle, resource, &mux, false);
            for (;*resources!=NULL;resources++,cnt++) {
                if((cnt == mux) || from_init)
                    rc |= BCHP_PWR_P_ReleaseResource(handle, *resources, from_init);
            }
#endif
            break;
        }
        case BCHP_PWR_P_ResourceType_eLeaf:
        case BCHP_PWR_P_ResourceType_eNonLeafHw:
        {
            if (ref_cnt==0) {
                if (!from_init) {
                    BDBG_MSG_TRACE(("  Deactivate HW node %#x (%s)", resource->id, resource->name));
                }
                else {
                    BDBG_MSG(("Initialize HW resource %#x (%s)", resource->id, resource->name));
                    handle->pwrManager->init[idx] = true;
                }
                BCHP_PWR_P_HW_Control(handle, resource, false);
            }
            if (resource->type==BCHP_PWR_P_ResourceType_eLeaf) {
                break;
            }
            /* otherwise fall-through. for Release(), the recursive Release() must occur after activating the leaf HW node */
        }
        case BCHP_PWR_P_ResourceType_eNonLeaf:
        case BCHP_PWR_P_ResourceType_eDiv:
        {
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];

            if(resource->type == BCHP_PWR_P_ResourceType_eDiv) {
                if (ref_cnt==0) {
                    if (from_init) {
                        BDBG_MSG(("Initialize DIV resource %#x (%s)", resource->id, resource->name));
                        handle->pwrManager->init[idx] = true;
                    }
                }
                /* Leaf Divisor */
                if(resources == NULL) {
                    break;
                }
            }
            BDBG_ASSERT(resources);

            for (;*resources!=NULL;resources++) {
                rc |= BCHP_PWR_P_ReleaseResource(handle, *resources, from_init);
            }
            break;
        }
    }
    return rc;
}
#endif /* BCHP_PWR_SUPPORT */

BERR_Code BCHP_PWR_AcquireResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
#if BCHP_PWR_SUPPORT
    BERR_Code rc;
    const BCHP_PWR_P_Resource *resource;
    unsigned idx, refcnt;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(handle->pwrManager);

    if (!resourceId) {
        return BERR_INVALID_PARAMETER;
    }
    if (resourceId > BCHP_PWR_P_NUM_NONLEAFS) { /* keep in mind that the public resource ID count starts from 1 */
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_AcquireMutex(handle->pwrManager->lock);

    idx = BCHP_PWR_P_GetInternalIndex(resourceId);
    resource = BCHP_PWR_P_GetResourceHandle(resourceId);
    refcnt = handle->pwrManager->pubRefcnt[idx];

    BDBG_MSG(("Acquire resource %#x (%s) refcnt %u->%u", resource->id, resource->name, refcnt, refcnt+1));
    handle->pwrManager->pubRefcnt[idx]++;

    rc = BCHP_PWR_P_AcquireResource(handle, resource, false);

    if (rc!=BERR_SUCCESS) {
        BDBG_ERR(("Acquire resource %#x (%s) failed. refcnt %u", resource->id, resource->name, refcnt));
        handle->pwrManager->pubRefcnt[idx]--;
    }

    BKNI_ReleaseMutex(handle->pwrManager->lock);
    return rc;
#else /* BCHP_PWR_SUPPORT */
    BCHP_PWR_P_Acquire(handle, resourceId);
    return BERR_SUCCESS;
#endif
}

BERR_Code BCHP_PWR_ReleaseResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
#if BCHP_PWR_SUPPORT
    BERR_Code rc;
    const BCHP_PWR_P_Resource *resource;
    unsigned idx, refcnt;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(handle->pwrManager);

    if (!resourceId) {
        return BERR_INVALID_PARAMETER;
    }
    if (resourceId > BCHP_PWR_P_NUM_NONLEAFS) { /* keep in mind that the public resource ID count starts from 1 */
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_AcquireMutex(handle->pwrManager->lock);

    idx = BCHP_PWR_P_GetInternalIndex(resourceId);
    resource = BCHP_PWR_P_GetResourceHandle(resourceId);
    refcnt = handle->pwrManager->pubRefcnt[idx];

    /* you can only release what you've acquired */
    if (refcnt == 0) {
        BDBG_ERR(("Unacquired power resource %#x (%s) cannot be released", resource->id, resource->name));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else {
        BDBG_MSG(("Release resource %#x (%s) refcnt %u->%u", resource->id, resource->name, refcnt, refcnt-1));
        handle->pwrManager->pubRefcnt[idx]--;

        rc = BCHP_PWR_P_ReleaseResource(handle, resource, false);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("Release resource %#x (%s) failed. refcnt %u", resource->id, resource->name, refcnt));
            handle->pwrManager->pubRefcnt[idx]++;
        }
    }

    BKNI_ReleaseMutex(handle->pwrManager->lock);
    return rc;
#else /* BCHP_PWR_SUPPORT */
    BCHP_PWR_P_Release(handle, resourceId);
    return BERR_SUCCESS;
#endif
}

void BCHP_PWR_Dump(BCHP_Handle handle)
{
#if BCHP_PWR_SUPPORT
    int i;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(handle->pwrManager);

    BKNI_AcquireMutex(handle->pwrManager->lock);

    BKNI_Printf("%-24s %5s %5s %5s %5s %5s %5s\n", "resource name", "pub", "priv", "init", "magnum", "shared", "secure");
    for (i=0; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        /* cannot call BCHP_PWR_P_GetResourceHandle here because it requires the public resource ID */
        BKNI_Printf("%-24s %5u %5u %5u %5u %5u %5u\n", BCHP_PWR_P_ResourceList[i]->name,
            handle->pwrManager->pubRefcnt[i], handle->pwrManager->privRefcnt[i], handle->pwrManager->init[i],
            handle->pwrManager->magnumCtrl[i], handle->pwrManager->sharedCtrl[i], handle->pwrManager->secureCtrl[i]);
    }
    BKNI_Printf("\n");

    BKNI_ReleaseMutex(handle->pwrManager->lock);
    return;
#else /* BCHP_PWR_SUPPORT */
    BSTD_UNUSED(handle);
    return;
#endif
}

bool BCHP_PWR_P_IsResourceAcquired(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
#if BCHP_PWR_SUPPORT
    unsigned idx;
    bool acquired = false;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(handle->pwrManager);

    if (!resourceId) {
        return BERR_INVALID_PARAMETER;
    }
    if (resourceId > BCHP_PWR_P_NUM_NONLEAFS) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_AcquireMutex(handle->pwrManager->lock);

    idx = BCHP_PWR_P_GetInternalIndex(resourceId);
    if (handle->pwrManager->pubRefcnt[idx] > 0 || handle->pwrManager->privRefcnt[idx]) {
        acquired = true;
    }

    BKNI_ReleaseMutex(handle->pwrManager->lock);

    return acquired;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    return true;
#endif
}

#include "bchp_pwr_debug.c"
void BCHP_PWR_DebugPrint(BCHP_Handle handle)
{
    unsigned i, j;
#if BDBG_NO_LOG
	BSTD_UNUSED(handle);
#endif

    for(i=0; i<sizeof(cores)/sizeof(cores[0]); i++) {
        if(!cores[i].map[0].resource) {
            continue;
        }
        BDBG_LOG(("%s", cores[i].pCoreName));
        for(j=0; ; j++) {
            if(!cores[i].map[j].resource) {
                break;
            }

            BDBG_LOG(("    %-24s : %s", cores[i].map[j].pResourceName,
                    BCHP_PWR_P_IsResourceAcquired(handle, cores[i].map[j].resource) ? "ON" : "OFF"));
        }
        BDBG_LOG((" "));
    }
}

BERR_Code BCHP_PWR_Standby(BCHP_Handle handle, const BCHP_PWR_StandbySettings *pSettings)
{
#if BCHP_PWR_SUPPORT
    const BCHP_PWR_P_Resource *resource = NULL;
    unsigned i, idx;

    BDBG_ASSERT(handle);
    BSTD_UNUSED(pSettings);
    i = idx = 0;

    /* prints which clocks are still alive */
    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type != BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (handle->pwrManager->privRefcnt[idx]!=0) {
            BDBG_MSG(("Standby: clock %s still active", resource->name));
        }
    }

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_SUCCESS;
#endif
}

void BCHP_PWR_Resume(BCHP_Handle handle)
{
#if BCHP_PWR_SUPPORT
    const BCHP_PWR_P_Resource *resource = NULL;
    unsigned i, idx;
    unsigned *refcnt;
    BDBG_ASSERT(handle);

    /* on BCHP_PWR_Resume, we must re-apply the SW state to HW.
       we can't just blindly Release HW nodes with refcnt 0, because HW -> HW dependencies must be honored
       therefore, we must Acquire, then Release */
    BDBG_MSG(("BCHP_PWR_Resume:>"));

    /* save the current state, because we have to muck with it a little */
    refcnt = BKNI_Malloc(sizeof(unsigned)*BCHP_PWR_P_NUM_ALLNODES);
    if (!refcnt) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return;
    }
    BKNI_Memcpy(refcnt, handle->pwrManager->privRefcnt, sizeof(unsigned)*BCHP_PWR_P_NUM_ALLNODES);

    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type != BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (refcnt[idx]==0) { /* node that should be powered down, according to the SW state */
            BCHP_PWR_P_AcquireResource(handle, resource, false); /* if the HW is actually turned on, the Acquire call is only needed to increment the refcnt */
        }
    }

    for (i=BCHP_PWR_P_NUM_NONLEAFS; i<BCHP_PWR_P_NUM_ALLNODES; i++) {
        resource = BCHP_PWR_P_ResourceList[i];
        BDBG_ASSERT(resource->type != BCHP_PWR_P_ResourceType_eNonLeaf);
        idx = BCHP_PWR_P_GetInternalIndex(resource->id);
        if (refcnt[idx]==0) { /* node that should be powered down, according to the SW state */
            BCHP_PWR_P_ReleaseResource(handle, resource, false);
            BDBG_MSG(("Resume: clock %s powered down", resource->name));
        }
    }

    /* this is an important sanity check */
    i = BKNI_Memcmp(refcnt, handle->pwrManager->privRefcnt, sizeof(unsigned)*BCHP_PWR_P_NUM_ALLNODES);
    BDBG_ASSERT(i==0);

    BKNI_Free(refcnt);
    BDBG_MSG(("BCHP_PWR_Resume:<"));

    /* TODO : Restore Frequency and Mux Settings */
#else
    BSTD_UNUSED(handle);
    return;
#endif
}


#if BCHP_PWR_SUPPORT

#ifndef BASE_FREQ
#define BASE_FREQ 54*1000*1000
#endif

#ifndef BCHP_PWR_NUM_P_STATES
#define BCHP_PWR_NUM_P_STATES 2
#endif

#define MAX_INDEX  0
#define MIN_INDEX  BCHP_PWR_NUM_P_STATES-1

static const BCHP_PWR_P_DivTable *BCHP_PWR_P_ClockRateControl_priv(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    unsigned idx = BCHP_PWR_P_GetInternalIndex(resource->id);
    const BCHP_PWR_P_DivTable *divTable = NULL;

    switch(resource->type) {
        case BCHP_PWR_P_ResourceType_eMux:
        {
#ifdef BCHP_PWR_P_NUM_MUXES
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            unsigned mux, cnt=0;
            BDBG_ASSERT(resources);

            BCHP_PWR_P_MUX_Control(handle, resource, &mux, false);
            for (;*resources!=NULL;resources++,cnt++) {
                if(cnt == mux)
                    divTable = BCHP_PWR_P_ClockRateControl_priv(handle, *resources, mult, prediv, postdiv, set);
                if(divTable)
                    break;
            }
#endif
            break;
        }
        case BCHP_PWR_P_ResourceType_eNonLeaf:
        case BCHP_PWR_P_ResourceType_eNonLeafHw:
        {
            const BCHP_PWR_P_Resource **resources = BCHP_PWR_P_DependList[idx];
            BDBG_ASSERT(resources);

            for (;*resources!=NULL;resources++) {
                divTable = BCHP_PWR_P_ClockRateControl_priv(handle, *resources, mult, prediv, postdiv, set);
                if(divTable)
                    break;
            }
            break;
        }
        case BCHP_PWR_P_ResourceType_eDiv:
        {
#ifdef BCHP_PWR_P_NUM_DIVS
            unsigned i;
            BCHP_PWR_P_DIV_Control(handle, resource, mult, prediv, postdiv, set);
            for (i=0; i<BCHP_PWR_P_NUM_DIVS; i++) {
                if (resource->id == BCHP_PWR_P_FreqMapList[i].id) {
                    divTable = BCHP_PWR_P_FreqMapList[i].pDivTable;
                    break;
                }
            }
#endif
            break;
        }
        case BCHP_PWR_P_ResourceType_eLeaf:
        {
            break;
        }
    }

    return divTable;
}

static const BCHP_PWR_P_DivTable *BCHP_PWR_P_ClockRateControl(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    const BCHP_PWR_P_DivTable *divTable = NULL;

    divTable = BCHP_PWR_P_ClockRateControl_priv(handle, resource, mult, prediv, postdiv, set);
    return divTable;
}

static BERR_Code BCHP_PWR_P_GetClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *minRate, unsigned *maxRate, unsigned *curRate)
{
    BERR_Code rc = BERR_NOT_SUPPORTED;
    const BCHP_PWR_P_DivTable *divTable = NULL;
    const BCHP_PWR_P_Resource *resource;
    unsigned postdiv, prediv, mult;

    BDBG_ASSERT(handle);

    if (!resourceId) {
        return BERR_INVALID_PARAMETER;
    }
    if (resourceId > BCHP_PWR_P_NUM_NONLEAFS) { /* keep in mind that the public resource ID count starts from 1 */
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_AcquireMutex(handle->pwrManager->lock);

    resource = BCHP_PWR_P_GetResourceHandle(resourceId);
    divTable = BCHP_PWR_P_ClockRateControl(handle, resource, &mult, &prediv, &postdiv, false);
    if(divTable) {
        if (minRate) {
            *minRate = (((uint64_t)BASE_FREQ * divTable[MIN_INDEX].mult)/divTable[MIN_INDEX].prediv)/divTable[MIN_INDEX].postdiv;
            BDBG_MSG(("%s Min Clock Rate %u", resource->name, *minRate));
        }
        if (maxRate) {
            *maxRate = (((uint64_t)BASE_FREQ * divTable[MAX_INDEX].mult)/divTable[MAX_INDEX].prediv)/divTable[MAX_INDEX].postdiv;
            BDBG_MSG(("%s Max Clock Rate %u", resource->name, *maxRate));
        }
        if (curRate) {
            *curRate = (((uint64_t)BASE_FREQ * mult)/prediv)/postdiv;
            BDBG_MSG(("%s Current Clock Rate %u", resource->name, *curRate));
        }

        rc = BERR_SUCCESS;
    }

    BKNI_ReleaseMutex(handle->pwrManager->lock);

    return rc;
}
#endif

BERR_Code BCHP_PWR_GetMinClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
#if BCHP_PWR_SUPPORT
    return BCHP_PWR_P_GetClockRate(handle, resourceId, clkRate, NULL, NULL);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
#endif
}

BERR_Code BCHP_PWR_GetMaxClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
#if BCHP_PWR_SUPPORT
    return BCHP_PWR_P_GetClockRate(handle, resourceId, NULL, clkRate, NULL);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
#endif

}

BERR_Code BCHP_PWR_GetDefaultClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
#if BCHP_PWR_SUPPORT
    return BCHP_PWR_P_GetClockRate(handle, resourceId, NULL, clkRate, NULL);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
#endif
}

BERR_Code BCHP_PWR_GetClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
#if BCHP_PWR_SUPPORT
    return BCHP_PWR_P_GetClockRate(handle, resourceId, NULL, NULL, clkRate);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
#endif
}

BERR_Code BCHP_PWR_SetClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned clkRate)
{
    BERR_Code rc = BERR_NOT_SUPPORTED;
#if BCHP_PWR_SUPPORT
    const BCHP_PWR_P_DivTable *divTable = NULL;
    const BCHP_PWR_P_Resource *resource;
    unsigned postdiv, prediv, mult;
    unsigned maxRate, minRate;

    BDBG_ASSERT(handle);

    if (!resourceId) {
        return BERR_INVALID_PARAMETER;
    }
    if (resourceId > BCHP_PWR_P_NUM_NONLEAFS) { /* keep in mind that the public resource ID count starts from 1 */
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("Requested %u Clock Rate", clkRate));

    BKNI_AcquireMutex(handle->pwrManager->lock);

    resource = BCHP_PWR_P_GetResourceHandle(resourceId);
    divTable = BCHP_PWR_P_ClockRateControl(handle, resource, &mult, &prediv, &postdiv, false);

    if(divTable) {
        maxRate = (((uint64_t)BASE_FREQ * divTable[MAX_INDEX].mult)/divTable[MAX_INDEX].prediv)/divTable[MAX_INDEX].postdiv;
        minRate = (((uint64_t)BASE_FREQ * divTable[MIN_INDEX].mult)/divTable[MIN_INDEX].prediv)/divTable[MIN_INDEX].postdiv;
        BDBG_MSG(("min %u, max %u", minRate, maxRate));

        if(clkRate < minRate || clkRate > maxRate) {
            BDBG_WRN(("Frequency %u is out of range [%u, %u]", clkRate, minRate, maxRate));
            goto error;
        }

        postdiv = (((uint64_t)BASE_FREQ * mult)/prediv)/clkRate;
        BDBG_MSG(("clkrate %u, mult %u, prediv %u, postdiv %u", clkRate, mult, prediv, postdiv));

        if(BCHP_PWR_P_ClockRateControl(handle, resource, &mult, &prediv, &postdiv, true))
            rc = BERR_SUCCESS;
    }

error:
    BKNI_ReleaseMutex(handle->pwrManager->lock);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
#endif

    return rc;
}


#else /* BCHP_PWR_HAS_RESOURCES */


BERR_Code BCHP_PWR_Open(BCHP_PWR_Handle *pHandle, BCHP_Handle chp)
{

    BSTD_UNUSED(pHandle);
    BSTD_UNUSED(chp);
    BDBG_WRN(("\n"));
    BDBG_WRN(("*******************************************"));
    BDBG_WRN(("*******************************************"));
    BDBG_WRN(("** BCHP_PWR POWER MANAGEMENT DISABLED !!!**"));
    BDBG_WRN(("*******************************************"));
    BDBG_WRN(("*******************************************"));
    BDBG_WRN(("\n"));

    return BERR_SUCCESS;
}

void BCHP_PWR_Close(BCHP_PWR_Handle handle)
{
    BSTD_UNUSED(handle);
    return;
}

BERR_Code BCHP_PWR_AcquireResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    return BERR_SUCCESS;
}


BERR_Code BCHP_PWR_ReleaseResource(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    return BERR_SUCCESS;
}


void BCHP_PWR_InitAllHwResources(BCHP_Handle handle)
{
    BSTD_UNUSED(handle);
    return;
}


void BCHP_PWR_Dump(BCHP_Handle handle)
{
    BSTD_UNUSED(handle);
    return;
}


void BCHP_PWR_DebugPrint(BCHP_Handle handle)
{
    BSTD_UNUSED(handle);
    return;
}

BERR_Code BCHP_PWR_Standby(BCHP_Handle handle, const BCHP_PWR_StandbySettings *pSettings)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_SUCCESS;
}

void BCHP_PWR_Resume(BCHP_Handle handle)
{
    BSTD_UNUSED(handle);
    return;
}


BERR_Code BCHP_PWR_GetMinClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
}


BERR_Code BCHP_PWR_GetMaxClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
}


BERR_Code BCHP_PWR_GetDefaultClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
}


BERR_Code BCHP_PWR_GetClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned *clkRate)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;

}


BERR_Code BCHP_PWR_SetClockRate(BCHP_Handle handle, BCHP_PWR_ResourceId resourceId, unsigned clkRate)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(clkRate);
    return BERR_NOT_SUPPORTED;
}

#endif
