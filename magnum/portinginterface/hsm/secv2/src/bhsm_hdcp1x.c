/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_types.h"
#include "bhsm_p_hdcp1x.h"
#include "bhsm_hdcp1x.h"

BDBG_MODULE(BHSM);

#define BHSM_P_HDCP_KEY_OFFSET 0x80


BERR_Code BHSM_Hdcp1x_RouteKey( BHSM_Handle hHsm, const BHSM_Hdcp1xRouteKey *pParam )
{
    BERR_Code rc;

    BHSM_P_Hdcp1xGenAndRouteHdcp1x bspConfig;
    BDBG_ENTER( BHSM_Hdcp1x_RouteKey );

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );

    switch( pParam->algorithm ) {
        case BHSM_CryptographicAlgorithm_e3DesAba:  { bspConfig.in.keyLadderType = 0; break; }
        case BHSM_CryptographicAlgorithm_eAes128:   { bspConfig.in.keyLadderType = 1; break; }
        default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    }

    switch( pParam->root.type )
    {
        case BHSM_KeyLadderRootType_eOtpDirect:
        case BHSM_KeyLadderRootType_eOtpAskm:
        {
            bspConfig.in.rootKeySrc = pParam->root.otpKeyIndex;
            break;
        }
        case BHSM_KeyLadderRootType_eGlobalKey:
        {
            bspConfig.in.rootKeySrc = Bsp_RootKeySrc_eAskmGlobalKey;
            bspConfig.in.globalKeyIndex = pParam->root.globalKey.index;
            switch( pParam->root.globalKey.owner )
            {
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0: { bspConfig.in.stbOwnerIdSel = 0;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1: { bspConfig.in.stbOwnerIdSel = 1;  break; }
                case BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne:  { bspConfig.in.stbOwnerIdSel = 2;  break; }
                default: { BERR_TRACE( BERR_INVALID_PARAMETER ); }
            }
            break;
        }
        default:
        {
            return BERR_TRACE( BERR_NOT_SUPPORTED );
        }
    }

    if( pParam->root.type == BHSM_KeyLadderRootType_eOtpAskm ||
        pParam->root.type == BHSM_KeyLadderRootType_eGlobalKey )
    {
        bspConfig.in.caVendorId            = (uint16_t)pParam->root.askm.caVendorId;
        bspConfig.in.stbOwnerIdSel         = (uint8_t)pParam->root.askm.stbOwnerSelect;
        switch( pParam->root.askm.caVendorIdScope )
        {
            case BHSM_KeyladderCaVendorIdScope_eChipFamily: bspConfig.in.askmMaskKeySel = 0; break;
            case BHSM_KeyladderCaVendorIdScope_eFixed:      bspConfig.in.askmMaskKeySel = 2; break;
            default: return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
    }

    BDBG_MSG(("HDCP Key [%02d] high[%08X] low[%08X]",pParam->hdcpKeyIndex, pParam->key.high, pParam->key.low ));

    bspConfig.in.hdcp1xAddr = (uint16_t)pParam->hdcpKeyIndex + BHSM_P_HDCP_KEY_OFFSET;

    BKNI_Memcpy( (void*)(bspConfig.in.procIn+2), (void*)&pParam->key.high, 4 );
    BKNI_Memcpy( (void*)(bspConfig.in.procIn+3), (void*)&pParam->key.low,  4 );

    rc = BHSM_P_Hdcp1x_GenAndRouteHdcp1x( hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Hdcp1x_RouteKey );
    return BERR_SUCCESS;
}
