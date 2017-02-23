/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "nexus_tsmf.h"
#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"
#include "priv/nexus_tsmf_priv.h"

#if NEXUS_TRANSPORT_EXTENSION_TSMF
#else

#if NEXUS_HAS_TSMF
#include "bxpt_tsmf.h"
#endif

BDBG_MODULE(nexus_tsmf);

struct NEXUS_Tsmf
{
    NEXUS_OBJECT(NEXUS_Tsmf);
};

void NEXUS_Tsmf_GetDefaultOpenSettings(NEXUS_TsmfOpenSettings *pOpenSettings)
{
    BSTD_UNUSED(pOpenSettings);
}

NEXUS_TsmfHandle NEXUS_Tsmf_Open(unsigned index, const NEXUS_TsmfOpenSettings* pOpenSettings)
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pOpenSettings);
    return NULL;
}

static void NEXUS_Tsmf_P_Finalizer(NEXUS_TsmfHandle handle)
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Tsmf, NEXUS_Tsmf_Close);

void NEXUS_Tsmf_GetSettings(NEXUS_TsmfHandle handle, NEXUS_TsmfSettings *pSettings)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Tsmf_SetSettings(NEXUS_TsmfHandle handle, const NEXUS_TsmfSettings *pSettings)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_PidChannelHandle NEXUS_Tsmf_OpenPidChannel( /* attr{destructor=NEXUS_PidChannel_Close} */
    NEXUS_TsmfHandle tsmf,
    unsigned pid,
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings /* attr{null_allowed=y} may be NULL for default settings */
    )
{
    BSTD_UNUSED(tsmf);
    BSTD_UNUSED(pid);
    BSTD_UNUSED(pSettings);
    return NULL;
}
#endif
