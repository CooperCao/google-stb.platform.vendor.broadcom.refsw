/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_types.h"
#include "bhsm_pcie_window.h"
#include "bhsm_p_memcarch.h"

BDBG_MODULE( BHSM );


BERR_Code BHSM_PcieWindow_Set( BHSM_Handle hHsm, BHSM_PcieWindowSettings *pSettings )
{
    BHSM_P_MemcArchEnPcie bspConfig;
    BERR_Code rc = BERR_UNKNOWN;
    uint32_t addressStart = 0;
    uint32_t addressEnd = 0;

    BDBG_ENTER( BHSM_PcieWindow_Set );

    if( !hHsm ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pSettings ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    /* currently only PCIe interface index 0 and 1 supported. */
    if( pSettings->index > 1 ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    /* offset must be 1024 byte aligned */
    if( pSettings->baseOffset & 0xFF ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    /* size must be a multiple of 1024 byte */
    if( pSettings->size & 0xFF ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    addressStart = (uint32_t)(pSettings->baseOffset >> 8);
    addressEnd = addressStart + (pSettings->size >> 8) - 1;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.addrRangeStart = addressStart;
    bspConfig.in.addrRangeEnd   = addressEnd;
    bspConfig.in.exclusiveMode  = 1; /* exclusive mode */
    bspConfig.in.enablePcie0    = (pSettings->index == 0)?1:0;
    bspConfig.in.enablePcie1    = (pSettings->index == 1)?1:0;
    bspConfig.in.cmdPending     = 0;

    rc =  BHSM_P_MemcArch_EnPcie( hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_MSG(( "Set PCIe window start[0x%08X] size[%u] end[0x%08X] memc[%u] arch[%u]"
                                                                        , addressStart
                                                                        , (unsigned)pSettings->size
                                                                        , addressEnd
                                                                        , bspConfig.out.memcIndex
                                                                        , bspConfig.out.archIndex ));
    BDBG_LEAVE( BHSM_PcieWindow_Set );
    return BERR_SUCCESS;
}
