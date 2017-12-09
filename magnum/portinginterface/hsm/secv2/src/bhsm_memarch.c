/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhsm_bsp_msg.h"
#include "bhsm_memarch.h"
#include "bhsm_p_memcarch.h"

BDBG_MODULE( BHSM );

BERR_Code BHSM_MemArch_EnableGeneric( BHSM_Handle hHsm, BHSM_MemArchEnableGeneric *pParam )
{
    BERR_Code rc;
    BHSM_P_MemcArchEnGeneric bspConfig;
    unsigned x;

    BDBG_ENTER( BHSM_MemArch_EnableGeneric );

    if( pParam->addressRange.upper & 0xFF ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pParam->addressRange.lower & 0xFF ) { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    bspConfig.in.modeControl          = pParam->modeControl;
    bspConfig.in.addrRangeStart       = pParam->addressRange.upper >> 8;
    bspConfig.in.addrRangeEnd         = pParam->addressRange.lower >> 8;
    bspConfig.in.bgCkEnable           = pParam->enableBackgroundCheck?1:0;
    bspConfig.in.userTag              = 0;

    for( x = 0;
         x < ( sizeof(bspConfig.in.scbClientReadRights) / sizeof(bspConfig.in.scbClientReadRights[0]) );
         x++ )
    {
        bspConfig.in.scbClientReadRights[x]  = pParam->clients[x].read;
        bspConfig.in.scbClientWriteRights[x] = pParam->clients[x].write;
    }

    rc = BHSM_P_MemcArch_EnGeneric( hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_MemArch_EnableGeneric );
    return BERR_SUCCESS;
}

BERR_Code BHSM_MemArch_QueryUserTag( BHSM_Handle hHsm,  BHSM_MemArchQueryUserTag *pParam )
{
    BERR_Code rc;
    BHSM_P_MemcArchQueryUserTag bspConfig;
    unsigned x;

    BDBG_ENTER( BHSM_MemArch_QueryUserTag );

    if( !pParam ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    rc = BHSM_P_MemcArch_QueryUserTag( hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    if( bspConfig.out.userTagCount > BHSM_MEMCARCH_MAX_TAGS ){ return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    pParam->numTags = bspConfig.out.userTagCount;

    for( x = 0; x < bspConfig.out.userTagCount; x++ )
    {
        unsigned entry = bspConfig.out.userTagEntry[x];

        pParam->arch[x].memcIndex = (entry >> 24) & 0xFF;
        pParam->arch[x].archIndex = (entry >> 16) & 0xFF;
        pParam->arch[x].userTag   =  entry & 0xFF;
    }

    BDBG_LEAVE( BHSM_MemArch_QueryUserTag );
    return BERR_SUCCESS;
}
