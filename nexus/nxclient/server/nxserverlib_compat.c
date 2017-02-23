/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nxserverlib_impl.h"
#if B_IPC_COMPAT_SUPPORT
#include "b_ipc_nxclient_p_compat.h"
BDBG_MODULE(nxserverlib_compat);

NEXUS_Error NxClient_P_Compat_From_size_t(const B_NEXUS_COMPAT_TYPE(size_t) *src, size_t *dst)
{
    *dst = *src;
    return NEXUS_SUCCESS;
}

void *NxClient_P_Compat_From_Handle(B_NEXUS_COMPAT_TYPE(ptr) id)
{
    void *object;
    NEXUS_Error rc = NEXUS_Platform_GetObjectFromId(id, &object);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); object = NULL; }
    BDBG_MSG(("NxClient_P_Compat_From_Handle:%#x -> %p", id, object));
    return object;
}

B_NEXUS_COMPAT_TYPE(ptr) NxClient_P_Compat_To_Handle(const void *object)
{
    NEXUS_BaseObjectId id;
    NEXUS_Error rc = NEXUS_Platform_GetIdFromObject((void *)object, &id);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); id = 0; }
    BDBG_MSG(("NxClient_P_Compat_To_Handle:%p -> %#x", object, id));
    return id;
}

NEXUS_Error NxClient_P_Compat_To_size_t(const size_t *src, B_NEXUS_COMPAT_TYPE(size_t) *dst)
{
    B_NEXUS_COMPAT_TYPE(size_t) result = *src;

    if(result!=*src) {
        result = 0xFFFFFFFFu;
        BDBG_WRN(("NxClient_P_Compat_To_size_t: Truncating %lu to %u", (unsigned long)*src, (unsigned)result));
    }
    *dst = result;
    return NEXUS_SUCCESS;
}
#endif /* B_IPC_COMPAT_SUPPORT */
