/***************************************************************************
 *     (c)2004-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "nexus_client_resources.h"
#include "b_objdb.h"

BDBG_MODULE(nexus_client_resources);

int nexus_client_resources_Count_acquire(const char *name, unsigned offset, unsigned id) 
{
    struct b_objdb_client *client = (struct b_objdb_client *)b_objdb_get_client();
    int rc = 0;
    BSTD_UNUSED(id);
    if (client && client->mode == NEXUS_ClientMode_eUntrusted) {
        NEXUS_ClientResourceCount *pUsed = (NEXUS_ClientResourceCount *)&((uint8_t*)&client->resources.used)[offset];
        NEXUS_ClientResourceCount *pAllowed = (NEXUS_ClientResourceCount *)&((uint8_t*)&client->resources.allowed)[offset];
        NEXUS_LockModule();
        if (pUsed->total+1 <= pAllowed->total) { 
            pUsed->total += 1; 
        }
        else {
            BDBG_ERR(("%s acquire rejected: %d already used, %d allowed", name, pUsed->total, pAllowed->total));
            rc = NEXUS_NOT_AVAILABLE; 
        }
        NEXUS_UnlockModule();
    }
    return rc;
}

void nexus_client_resources_Count_release(const char *name, unsigned offset, unsigned id) 
{
    struct b_objdb_client *client = (struct b_objdb_client *)b_objdb_get_client();
    BSTD_UNUSED(id);
    BSTD_UNUSED(name);
    /* when system shuts down, client may be NULL */
    if (client && client->mode == NEXUS_ClientMode_eUntrusted) {
        NEXUS_ClientResourceCount *pUsed = (NEXUS_ClientResourceCount *)&((uint8_t*)&client->resources.used)[offset];
        NEXUS_LockModule();
        BDBG_ASSERT(pUsed->total);
        pUsed->total--;
        NEXUS_UnlockModule();
    }
}

int nexus_client_resources_IdList_acquire(const char *name, unsigned offset, unsigned id) 
{
    struct b_objdb_client *client = (struct b_objdb_client *)b_objdb_get_client();
    int rc = 0;
    if (client && client->mode == NEXUS_ClientMode_eUntrusted) {
        NEXUS_ClientResourceIdList *pUsed = (NEXUS_ClientResourceIdList *)&((uint8_t*)&client->resources.used)[offset];
        NEXUS_ClientResourceIdList *pAllowed = (NEXUS_ClientResourceIdList *)&((uint8_t*)&client->resources.allowed)[offset];
        unsigned i;
        NEXUS_LockModule();
        for (i=0;i<NEXUS_MAX_IDS;i++) {
            if (id == pAllowed->id[i]) {
                if (pUsed->id[i]) {
                    BDBG_WRN(("unbalanced NEXUS_CLIENT_RESOURCES_ACQUIRE for %s %u", name, id));
                }
                pUsed->id[i] = 1; /* mark as used */
                if (i >= pUsed->total) {
                    pUsed->total = i+1;
                }
                break;
            }
        }
        if (i == NEXUS_MAX_IDS) {
            BDBG_ERR(("%s acquire rejected: ID %d not allowed", name, id));
            rc = NEXUS_NOT_AVAILABLE;
        }
        NEXUS_UnlockModule();
    }
    return rc;
}

void nexus_client_resources_IdList_release(const char *name, unsigned offset, unsigned id)
{
    struct b_objdb_client *client = (struct b_objdb_client *)b_objdb_get_client();
    /* when system shuts down, client may be NULL */
    if (client && client->mode == NEXUS_ClientMode_eUntrusted) {
        NEXUS_ClientResourceIdList *pUsed = (NEXUS_ClientResourceIdList *)&((uint8_t*)&client->resources.used)[offset];
        NEXUS_ClientResourceIdList *pAllowed = (NEXUS_ClientResourceIdList *)&((uint8_t*)&client->resources.allowed)[offset];
        unsigned i;
        NEXUS_LockModule();
        for (i=0;i<NEXUS_MAX_IDS;i++) {
            /* don't count, just verify ID. module code will prevent multiple acquires if that's a problem */
            if (id == pAllowed->id[i] && pUsed->id[i]) {
                pUsed->id[i] = 0;
                if (i == pUsed->total-1) {
                    /* pack */
                    unsigned j = 0;
                    for (i=0;i<pUsed->total;i++) {
                        if (!pUsed->id[i]) continue;
                        if (j != i) pUsed->id[j] = pUsed->id[i];
                        j++;
                    }
                    pUsed->total = j;
                }
                break;
            }
        }
        if (i == NEXUS_MAX_IDS) {
            BDBG_ERR(("%s bad IdList release: %u", name, id));
        }
        NEXUS_UnlockModule();
    }
}

void *nexus_client_driver_alloc(struct b_objdb_client *client, size_t size)
{
    if(client && size > client->config.resources.temporaryMemory.sizeLimit) {
        BDBG_ERR(("client %p tried to allocate %u(%u) bytes", (void *)client, (unsigned)size, client->config.resources.temporaryMemory.sizeLimit));
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
    return BKNI_Malloc(size);
}

#define CHECK_TOTAL(TYPE) do { \
    if (pUsed->TYPE.total > pAllowed->TYPE.total) { \
        return BERR_TRACE(NEXUS_NOT_AVAILABLE); \
    }} while(0)
#define CHECK_IDS(TYPE) do { \
    unsigned i; \
    for (i=0;i<pUsed->TYPE.total;i++) { \
        if (pUsed->TYPE.id[i] && !pAllowed->TYPE.id[i]) { \
            BDBG_WRN(("%s %u still in use", #TYPE, pUsed->TYPE.id[i])); \
            return NEXUS_NOT_AVAILABLE; \
        } \
    } } while(0)


/* pAllowed is the new resource struct that will be allowed.
return failure if applying it will result in a violation. */
NEXUS_Error nexus_client_resources_check(const NEXUS_ClientResources *pAllowed, const NEXUS_ClientResources *pUsed)
{
    CHECK_IDS(simpleAudioDecoder);
    CHECK_IDS(simpleVideoDecoder);
    CHECK_IDS(simpleEncoder);
    CHECK_IDS(surfaceClient);
    CHECK_IDS(inputClient);
    CHECK_IDS(audioCapture);
    CHECK_TOTAL(dma);
    CHECK_TOTAL(graphics2d);
    CHECK_TOTAL(graphicsv3d);
    CHECK_TOTAL(pictureDecoder);
    CHECK_TOTAL(playpump);
    CHECK_TOTAL(recpump);
    CHECK_TOTAL(simpleAudioPlayback);
    CHECK_TOTAL(simpleStcChannel);
    CHECK_TOTAL(surface);
    return 0;
}
