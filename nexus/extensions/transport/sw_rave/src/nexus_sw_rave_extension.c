/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
#include "priv/nexus_core.h"
#include "bchp_xpt_rave.h"

BDBG_MODULE(nexus_sw_rave_extension);

#define BDBG_MSG_FLOW(X) /* BDBG_WRN(X) */

/* The follow is template code. It is assumed you will modify it freely for your custom SW RAVE implementation.
This code is based on bxpt_rave.c's BXPT_Rave_AdvanceSoftContext() */

#define CDB_READ_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_WRAP_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_VALID_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define CDB_DEPTH_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_CDB_DEPTH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define ITB_READ_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_ITB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_END_PTR_OFFSET      ( BCHP_XPT_RAVE_CX0_AV_ITB_END_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_WRAP_PTR_OFFSET     ( BCHP_XPT_RAVE_CX0_AV_ITB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_VALID_PTR_OFFSET    ( BCHP_XPT_RAVE_CX0_AV_ITB_VALID_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )
#define ITB_DEPTH_OFFSET        ( BCHP_XPT_RAVE_CX0_AV_ITB_DEPTH - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR )

#define BXPT_ITB_SIZE 16

struct private_data
{
    uint32_t src_base_addr;
    uint32_t dest_base_addr;
    uint32_t src_itb_base;
    uint32_t dest_itb_base;
    uint32_t last_src_itb_valid;
};

NEXUS_Error NEXUS_P_SwRaveExtension_Init( NEXUS_RaveHandle rave )
{
    BREG_Handle regHandle = g_pCoreHandles->reg;
    struct private_data *pData;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);

    rave->swRave.extensionData = pData = BKNI_Malloc(sizeof(*pData));
    if (!rave->swRave.extensionData) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(pData, 0, sizeof(*pData));

    /* "WRITE" is the base register for CDB/ITB blocks, but is not available from context map. */
    pData->src_base_addr = rave->xptContextMap.CDB_Read-4;
    pData->dest_base_addr = rave->swRave.xptContextMap.CDB_Read-4;

    pData->src_itb_base = BREG_Read32(regHandle, rave->xptContextMap.ITB_Read-4);
    pData->last_src_itb_valid = pData->src_itb_base;
    pData->dest_itb_base = BREG_Read32(regHandle, rave->swRave.xptContextMap.ITB_Read-4);
    return 0;
}

void NEXUS_P_SwRaveExtension_Uninit( NEXUS_RaveHandle rave )
{
    if (rave->swRave.extensionData) {
        BKNI_Free(rave->swRave.extensionData);
    }
}

void NEXUS_P_SwRaveExtension_Advance( NEXUS_RaveHandle rave )
{
    uint32_t src_valid,  src_wrap, src_read;
    uint32_t dest_valid, dest_wrap, dest_read;
    uint32_t reg, overflow;
    BREG_Handle regHandle = g_pCoreHandles->reg;
    struct private_data *pData;

    BDBG_OBJECT_ASSERT(rave, NEXUS_Rave);

    pData = rave->swRave.extensionData;

    /* Copy over the overflow bits. Decoder needs them. */
    /* CDB */
    reg = BREG_Read32(regHandle, pData->src_base_addr + CDB_DEPTH_OFFSET );
    overflow = BCHP_GET_FIELD_DATA( reg, XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW );
    reg = BREG_Read32(regHandle, pData->dest_base_addr + CDB_DEPTH_OFFSET );
    reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW ) );
    reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_CDB_DEPTH, CDB_OVERFLOW, overflow ) );
    BREG_Write32(regHandle, pData->dest_base_addr + CDB_DEPTH_OFFSET, reg);

    /* ITB */
    reg = BREG_Read32(regHandle, pData->src_base_addr + ITB_DEPTH_OFFSET );
    overflow = BCHP_GET_FIELD_DATA( reg, XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW );
    reg = BREG_Read32(regHandle, pData->dest_base_addr + ITB_DEPTH_OFFSET );
    reg &= ~( BCHP_MASK( XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW ) );
    reg |= ( BCHP_FIELD_DATA( XPT_RAVE_CX0_AV_ITB_DEPTH, ITB_OVERFLOW, overflow ) );
    BREG_Write32(regHandle, pData->dest_base_addr + ITB_DEPTH_OFFSET, reg);

    /* CDB - always do this before ITB */
    src_valid = BREG_Read32(regHandle, pData->src_base_addr + CDB_VALID_PTR_OFFSET );
    src_wrap = BREG_Read32(regHandle, pData->src_base_addr + CDB_WRAP_PTR_OFFSET);
    dest_read = BREG_Read32(regHandle, pData->dest_base_addr + CDB_READ_PTR_OFFSET);

    /* produce: advance the dest VALID and WRAP pointers.
    always write WRAP before VALID, always produce on dest before consuming on src */
    BREG_Write32(regHandle, pData->dest_base_addr + CDB_WRAP_PTR_OFFSET, src_wrap);
    BREG_Write32(regHandle, pData->dest_base_addr + CDB_VALID_PTR_OFFSET, src_valid);

    /* consume: advance the src READ pointer */
    BREG_Write32(regHandle, pData->src_base_addr + CDB_READ_PTR_OFFSET, dest_read);

    /* ITB */
    src_valid = BREG_Read32(regHandle, pData->src_base_addr + ITB_VALID_PTR_OFFSET );
    src_wrap = BREG_Read32(regHandle, pData->src_base_addr + ITB_WRAP_PTR_OFFSET );
    dest_valid = BREG_Read32(regHandle, pData->dest_base_addr + ITB_VALID_PTR_OFFSET );
    dest_read = BREG_Read32(regHandle, pData->dest_base_addr + ITB_READ_PTR_OFFSET );
    dest_wrap = BREG_Read32(regHandle, pData->dest_base_addr + ITB_WRAP_PTR_OFFSET );

    /* This is based on the ePointersOnly logic in bxpt_rave.c:
    do nothing - just copy pointers
    this adds host based flow control. useful for debug, peeking into ITB, possibly modifying in-place.
    no ITB entries can be added/removed. */
    BDBG_MSG(("advance rave ITB: %x, %x, %x, %x", src_valid, src_wrap, dest_read));
    dest_valid = src_valid;
    dest_wrap = src_wrap;
    src_read = dest_read;

    /* produce: advance the dest VALID and WRAP pointers */
    BREG_Write32(regHandle, pData->dest_base_addr + ITB_WRAP_PTR_OFFSET, dest_wrap);
    BREG_Write32(regHandle, pData->dest_base_addr + ITB_VALID_PTR_OFFSET, dest_valid);

    /* consume: advance the src READ pointer */
    BREG_Write32(regHandle, pData->src_base_addr + ITB_READ_PTR_OFFSET, src_read);
}
