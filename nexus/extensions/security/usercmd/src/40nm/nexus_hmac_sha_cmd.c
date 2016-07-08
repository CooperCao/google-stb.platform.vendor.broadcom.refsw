/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/


#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bhsm_hmac_sha.h"


BDBG_MODULE(nexus_hmac_sha_cmd);


void NEXUS_HMACSHA_GetDefaultOpSettings(
    NEXUS_HMACSHA_OpSettings *pOpSettings /* [out] */
    )

{
    BKNI_Memset(pOpSettings, 0, sizeof(*pOpSettings));
    pOpSettings->dataSrc   = NEXUS_HMACSHA_DataSource_eDRAM;
    pOpSettings->keySource = NEXUS_HMACSHA_KeySource_eKeyVal;
    pOpSettings->context   = NEXUS_HMACSHA_BSPContext_eContext0;
    return;
}


NEXUS_Error NEXUS_HMACSHA_PerformOp(
    const NEXUS_HMACSHA_OpSettings *pOpSettings,    /* structure holding input parameters */
    NEXUS_HMACSHA_DigestOutput     *pOutput         /* [out] structure holding digest buffer and size */
    )
{
    BHSM_Handle             hHsm;
    BHSM_UserHmacShaIO_t    hsmConf;
    NEXUS_Error             rc = NEXUS_SUCCESS;

    NEXUS_Security_GetHsm_priv (&hHsm);
    if( NULL == hHsm )
    {
        return BERR_TRACE( NEXUS_NOT_INITIALIZED );
    }

    if( pOpSettings == NULL || pOutput == NULL )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( pOutput, 0, sizeof(*pOutput) );
    BKNI_Memset( &hsmConf, 0, sizeof(hsmConf) );

    hsmConf.oprMode        = pOpSettings->opMode;
    hsmConf.shaType        = pOpSettings->shaType;
    hsmConf.keySource      = pOpSettings->keySource;
    hsmConf.pucInputData   = pOpSettings->dataAddress;
    hsmConf.unInputDataLen = pOpSettings->dataSize;
    hsmConf.contMode       = pOpSettings->dataMode;
    hsmConf.keyIncMode     = (BHSM_HMACSHA_KeyInclusion_e)pOpSettings->keyIncMode;
    hsmConf.byteSwap       = pOpSettings->byteSwap;

    switch ( pOpSettings->context )
    {
        case NEXUS_HMACSHA_BSPContext_eContext0: hsmConf.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx0; break;
        case NEXUS_HMACSHA_BSPContext_eContext1: hsmConf.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx1; break;
        default: return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* including SAGE context which will not use NEXUS. */
    }

    if( pOpSettings->keySource == NEXUS_HMACSHA_KeySource_eKeyLadder )
    {
        hsmConf.VirtualKeyLadderID = pOpSettings->VKL;
        hsmConf.keyLayer = pOpSettings->keyLayer;
    }
    else
    {
        if( pOpSettings->keyLength )
        {
            hsmConf.unKeyLength = pOpSettings->keyLength;

            if( hsmConf.unKeyLength > sizeof(hsmConf.keyData) )
            {
                return BERR_TRACE( NEXUS_INVALID_PARAMETER );   /*  key too long */
            }

            BKNI_Memcpy( hsmConf.keyData, pOpSettings->keyData, hsmConf.unKeyLength );
        }
    }

    /* Make sure address is from a NEXUS_Memory_Allocate(), for this to work in both kernel and user mode Nexus */
    if( pOpSettings->dataAddress == NULL )
    {
        if(pOpSettings->keyIncMode  == NEXUS_HMACSHA_KeyInclusion_Op_eNo)
        {
            /* No key is included */
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }
    else if ( NEXUS_P_CpuAccessibleAddress( pOpSettings->dataAddress ) )
    {
        if( NEXUS_GetAddrType( pOpSettings->dataAddress ) == NEXUS_AddrType_eUnknown )
        {
            hsmConf.systemMemory = true; /*Its not BMEM ... it must be system memory */
        }
    }
    else
    {
        /* Not CPU accessible address */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* Submit the command now */
    rc = BHSM_UserHmacSha( hHsm, &hsmConf );
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    if( pOpSettings->dataMode == NEXUS_HMACSHA_DataMode_eAllIn ||
        pOpSettings->dataMode == NEXUS_HMACSHA_DataMode_eLast )
    {
        BKNI_Memcpy( (void *)pOutput->digestData, (void *)hsmConf.aucOutputDigest, hsmConf.digestSize );
        pOutput->digestSize = hsmConf.digestSize;
    }

    return BERR_SUCCESS;
}
