/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bhsm_hmac_sha.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define ROUND_UP_4(a)  ( ((a)&3)? ( (a)+(4-((a)&3)) ) : (a) )

BDBG_MODULE(nexus_hmac_sha_cmd);

static BERR_Code NEXUS_UserHmacSha ( BHSM_Handle hHsm, BHSM_UserHmacShaIO_t *pIo, bool byteSwap, bool useBounceBuffer );


void NEXUS_HMACSHA_GetDefaultOpSettings(
    NEXUS_HMACSHA_OpSettings *pOpSettings /* [out] */
    )

{
    BKNI_Memset(pOpSettings, 0, sizeof(*pOpSettings));
    pOpSettings->dataSrc   = NEXUS_HMACSHA_DataSource_eDRAM;
    pOpSettings->keySource = NEXUS_HMACSHA_KeySource_eKeyVal;
    pOpSettings->context   = NEXUS_HMACSHA_BSPContext_eContext0;

    BDBG_CASSERT( (int)NEXUS_HMACSHA_Op_eSHA == (int)BPI_HmacSha1_Op_eSha1 );
    BDBG_CASSERT( (int)NEXUS_HMACSHA_Op_eHMAC == (int)BPI_HmacSha1_Op_eHmac );
    BDBG_CASSERT( (int)NEXUS_HMACSHA_Op_eMax == (int)BPI_HmacSha1_Op_eMax );

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
    bool byteSwap = false;
    bool useBounceBuffer = false; /*data need to be copied into NEXUS memory*/

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
    byteSwap               = pOpSettings->byteSwap;

    switch ( pOpSettings->context )
    {
        case NEXUS_HMACSHA_BSPContext_eContext0: hsmConf.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx0; break;
        case NEXUS_HMACSHA_BSPContext_eContext1: hsmConf.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx1; break;
        default: return BERR_TRACE( NEXUS_INVALID_PARAMETER ); /* including SAGE context which will not use NEXUS. */
    }

    if( pOpSettings->keySource == NEXUS_HMACSHA_KeySource_eKeyLadder )
    {
        hsmConf.VirtualKeyLadderID = NEXUS_Security_P_mapNexus2Hsm_VklId( pOpSettings->VKL );
        hsmConf.keyLayer = NEXUS_Security_P_mapNexus2Hsm_KeySource( pOpSettings->keyLayer );
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
        if( pOpSettings->keyIncMode  == NEXUS_HMACSHA_KeyInclusion_Op_eNo )
        {
            /* No key is included */
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }
    else if ( NEXUS_P_CpuAccessibleAddress( pOpSettings->dataAddress ) )
    {
        if( NEXUS_GetAddrType( pOpSettings->dataAddress ) == NEXUS_AddrType_eUnknown )
        {
            BDBG_WRN(("Data address (dataAddress) should be NEXUS memory. Continuing best effort. "));
            useBounceBuffer = true; /*Its not BMEM ... it must be system memory */
        }

#if NEXUS_ZEUS_VERSION > NEXUS_ZEUS_VERSION_CALC(2,2)
        if( hsmConf.unInputDataLen <= sizeof(hsmConf.inputData) )
        {
            BHSM_MemcpySwap( hsmConf.inputData,
                                pOpSettings->dataAddress,
                                hsmConf.unInputDataLen,
                                !byteSwap ); /* swap has inverse requirement for inplace. */

            hsmConf.dataInplace = true;
        }
#endif /* #if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(4,2) */
    }
    else
    {
        /* Not CPU accessible address */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* Submit the command now */
    rc = NEXUS_UserHmacSha( hHsm, &hsmConf, byteSwap, useBounceBuffer );
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




/*
    BHSM_UserHmacSha performs prepocesing on the input parameters:
        - if a byte swap is reqeusted, or memory data is stored in is no BMEM, a
          shadow buffer is created.
        - if the data is greater that 8M, it is submitted in chunks of of 8M to the BSP.

*/
BERR_Code NEXUS_UserHmacSha ( BHSM_Handle hHsm, BHSM_UserHmacShaIO_t *pIo, bool byteSwap, bool useBounceBuffer )
{
    BERR_Code rc = BERR_SUCCESS;

    #define MAX_BYTE_SWAP_BUFFER_SIZE (1024*1024)
    #define MAX_TRANSFER_SIZE 0x800000 /* max allowed by BSP. */

    if( pIo->dataInplace )
    {
        return BHSM_UserHmacSha( hHsm, pIo );
    }

    /* handle a byteswap, or sysem memory. */
    if( byteSwap || useBounceBuffer )
    {
        uint8_t *pInputData = pIo->pucInputData;
        uint32_t inputDataLength = pIo->unInputDataLen;
        uint8_t *pBuf = NULL;
        unsigned bufSize  = MIN( pIo->unInputDataLen, MAX_BYTE_SWAP_BUFFER_SIZE );
        BHSM_HMACSHA_ContinualMode_e contModeOrig = pIo->contMode;
        NEXUS_MemoryAllocationSettings memSetting;

        NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
        memSetting.alignment = 32;
        memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
        rc = NEXUS_Memory_Allocate( ROUND_UP_4(bufSize), &memSetting, (void*)&pBuf );
        if( rc ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

        if( inputDataLength > bufSize )
        {
            pIo->contMode  = BHSM_HMACSHA_ContinualMode_eMoreSeg;
        }

        pIo->unInputDataLen = bufSize;
        pIo->pucInputData = pBuf;

        while( inputDataLength > bufSize )
        {
            BHSM_MemcpySwap( pBuf, pInputData, bufSize, byteSwap );
            NEXUS_FlushCache( pBuf, ROUND_UP_4(bufSize) );

            rc = BHSM_UserHmacSha( hHsm, pIo );
            if( rc != BERR_SUCCESS )
            {
                NEXUS_Memory_Free( pBuf );
                return BERR_TRACE(rc);
            }

            pInputData += bufSize;
            inputDataLength -= bufSize;
        }

        pIo->unInputDataLen = inputDataLength;
        pIo->contMode = contModeOrig;

        BHSM_MemcpySwap( pBuf, pInputData, pIo->unInputDataLen, byteSwap );
        NEXUS_FlushCache( pBuf, ROUND_UP_4(bufSize) );

        rc = BHSM_UserHmacSha( hHsm, pIo );
        if( rc != BERR_SUCCESS )
        {
            NEXUS_Memory_Free( pBuf );
            return BERR_TRACE(rc);
        }

        NEXUS_Memory_Free( pBuf );
        return BERR_SUCCESS;
    }

    /* The BSP can only handle 8Mbytes at a time. Here, larger data blocks are split. */
    if( pIo->unInputDataLen > MAX_TRANSFER_SIZE )
    {
        uint32_t inputDataLength = pIo->unInputDataLen;
        BHSM_HMACSHA_ContinualMode_e contModeOrig = pIo->contMode;

        pIo->unInputDataLen = MAX_TRANSFER_SIZE;
        pIo->contMode   = BHSM_HMACSHA_ContinualMode_eMoreSeg;

        do
        {
            NEXUS_FlushCache( pIo->pucInputData, ROUND_UP_4(pIo->unInputDataLen) );
            rc = BHSM_UserHmacSha( hHsm, pIo );
            if( rc != BERR_SUCCESS ){ return BERR_TRACE(rc); }

            inputDataLength -= MAX_TRANSFER_SIZE;
            pIo->pucInputData += MAX_TRANSFER_SIZE;

        }while( inputDataLength > MAX_TRANSFER_SIZE );

        pIo->unInputDataLen = inputDataLength;
        pIo->contMode = contModeOrig;

        NEXUS_FlushCache( pIo->pucInputData, ROUND_UP_4(pIo->unInputDataLen) );
        rc = BHSM_UserHmacSha( hHsm, pIo );
        if( rc != BERR_SUCCESS ){ return BERR_TRACE(rc); }

        return BERR_SUCCESS;
    }

    NEXUS_FlushCache( pIo->pucInputData, ROUND_UP_4(pIo->unInputDataLen) );
    return BHSM_UserHmacSha( hHsm, pIo );
}
