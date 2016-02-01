/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 **************************************************************************/

#include "nexus_transport_module.h"

BDBG_MODULE(nexus_message_sam);

BDBG_OBJECT_ID(NEXUS_Message_Sam);

/* some platforms do not have SAM */
#if (BXPT_P_MAX_SAM_PID_CHANNEL || BXPT_NUM_SAM_PID_CHANNEL) && !NEXUS_USE_SW_FILTER
#define NEXUS_MESSAGE_HAS_SAM   1
#endif

#if NEXUS_MESSAGE_HAS_SAM
#include "bxpt_sam.h"
#endif

void NEXUS_Message_GetSamAddresses(
    NEXUS_MessageSamAddresses *pAddresses /* [out] */
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BXPT_SAMAddresses addr;
    BDBG_ASSERT(pAddresses);
    
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, physicalAddress,     BXPT_SAMAddresses, PhysicalAddr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, type0NetworkAddress, BXPT_SAMAddresses, Type0NetworkAddr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, type1NetworkAddress, BXPT_SAMAddresses, Type1NetworkAddr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, type0M40Address,     BXPT_SAMAddresses, Type0M40Addr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, type1M40Address,     BXPT_SAMAddresses, Type1M40Addr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, m24Address,          BXPT_SAMAddresses, M24Addr);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, idRejectHighLimit,   BXPT_SAMAddresses, IdRejectHiLimit);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamAddresses, idRejectLowLimit,    BXPT_SAMAddresses, IdRejectLowLimit);
    NEXUS_ASSERT_STRUCTURE(NEXUS_MessageSamAddresses, BXPT_SAMAddresses);

    BXPT_SAM_GetAddresses(pTransport->xpt, &addr);
    BKNI_Memcpy(pAddresses, &addr, sizeof(NEXUS_MessageSamAddresses));
#else
    BSTD_UNUSED(pAddresses);
#endif

    return;
}

NEXUS_Error NEXUS_Message_SetSamAddresses(
    const NEXUS_MessageSamAddresses *pAddresses
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BXPT_SAMAddresses addr;
    BKNI_Memcpy(&addr, pAddresses, sizeof(NEXUS_MessageSamAddresses));
    BXPT_SAM_SetAddresses(pTransport->xpt, &addr);
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(pAddresses);
    return NEXUS_NOT_SUPPORTED;
#endif
}

void NEXUS_Message_GetSamM16Address(
    NEXUS_MessageHandle handle,
    NEXUS_MessageSamM16Address *pAddress /* [out] */
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BXPT_SAM_M16Addresses addr;
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_Message);

    NEXUS_ASSERT_FIELD(NEXUS_MessageSamM16Address, address0, BXPT_SAM_M16Addresses, Address0);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamM16Address, address1, BXPT_SAM_M16Addresses, Address1);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamM16Address, address2, BXPT_SAM_M16Addresses, Address2);
    NEXUS_ASSERT_FIELD(NEXUS_MessageSamM16Address, address3, BXPT_SAM_M16Addresses, Address3);
    NEXUS_ASSERT_STRUCTURE(NEXUS_MessageSamM16Address, BXPT_SAM_M16Addresses);
    rc = BXPT_SAM_GetM16Addresss(pTransport->xpt, handle->MesgBufferNum, &addr);
    if (rc == BERR_SUCCESS) {
        BKNI_Memcpy(pAddress, &addr, sizeof(NEXUS_MessageSamM16Address));
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pAddress);
#endif

    return;
}

NEXUS_Error NEXUS_Message_SetSamM16Address(
    NEXUS_MessageHandle handle,
    const NEXUS_MessageSamM16Address *pAddress
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BXPT_SAM_M16Addresses addr;
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_Message);
    
    BKNI_Memcpy(&addr, pAddress, sizeof(NEXUS_MessageSamM16Address));
    rc = BXPT_SAM_SetM16Addresss(pTransport->xpt, handle->MesgBufferNum, &addr);
    if (rc != BERR_SUCCESS) {
        return NEXUS_UNKNOWN;
    }
    else {
        return NEXUS_SUCCESS;
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pAddress);
    return NEXUS_NOT_SUPPORTED;
#endif
}

void NEXUS_Message_GetDefaultSamSettings(
    NEXUS_MessageSamSettings *pSettings /* out */
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#else
    BSTD_UNUSED(pSettings);
#endif
    return;
}

NEXUS_Error NEXUS_Message_SetSamSettings(
    NEXUS_MessageHandle handle,
    const NEXUS_MessageSamSettings *pSettings
    )
{
#if NEXUS_MESSAGE_HAS_SAM
    BXPT_SAM_Enables enables;
    BDBG_OBJECT_ASSERT(handle, NEXUS_Message);
    NEXUS_ASSERT_STRUCTURE(NEXUS_MessageSamSettings, BXPT_SAM_Enables);
    
    BKNI_Memcpy(&enables, pSettings, sizeof(NEXUS_MessageSamSettings));
    BXPT_SAM_DoFiltering(pTransport->xpt, handle->MesgBufferNum, &enables);
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
#endif
}

