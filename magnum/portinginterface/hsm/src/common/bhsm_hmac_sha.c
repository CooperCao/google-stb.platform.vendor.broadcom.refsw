/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/

#include "bhsm.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"

#define BSP_PKE_POLLING_CMD  /*required within  bsp_s_commands.h */
#include "bsp_s_commands.h"

#define USER_SHA_FIPS_KEY_256  (1) /* required in bsp_s_hmac_sha1.h*/
#define USER_SHA_KEY_INCLUDE   (1)
#include "bsp_s_hmac_sha1.h"
#include "bhsm_hmac_sha.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_private.h"
#include "bhsm_usercmd_common.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define ROUND_UP_4(a)  ( ((a)&3)? ( (a)+(4-((a)&3)) ) : (a) )

BDBG_MODULE(BHSM);

static BERR_Code BHSM_P_UserHmacSha_impl ( BHSM_Handle hHsm, BHSM_UserHmacShaIO_t *pIo )
{
    BHSM_BspMsg_h       hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BERR_Code           rc = BERR_SUCCESS;
    uint8_t             status;
    unsigned            algorithmKeyLen = 0; /* key length required for the algorithm. */
    unsigned            keyLen = 0;          /* the actual key length passed in */
    unsigned            offset_Key = 0;
    unsigned            offset_DataLength = 0;  /*offset in BSP message to data length */
    unsigned            offset_Data = 0;        /*offset in BSP message to data length */
    uint16_t            digestSize = 0;
    uint32_t            inputDataOffset;
    #define MIN_DATA_BY_DRAM (1)  /* if data length is less that this value, the data will be include *within* the BSP packet */

    BDBG_ENTER( BHSM_P_UserHmacSha_impl );

    if( (hHsm == NULL) || (hHsm->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER) || (pIo == NULL) )
    {
        return BERR_TRACE( BHSM_STATUS_FAILED );
    }

    if( pIo == NULL )
    {
        return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }

    pIo->digestSize = 0;
    keyLen = pIo->unKeyLength;

    switch( pIo->shaType )
    {
        case BPI_HmacSha1_ShaType_eSha160: algorithmKeyLen = 20; break;
        case BPI_HmacSha1_ShaType_eSha224: algorithmKeyLen = 28; break;
        case BPI_HmacSha1_ShaType_eSha256: algorithmKeyLen = 32; break;
        default: return  BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
    }
    if( keyLen > algorithmKeyLen )
    {
        BDBG_WRN(( "Key too long[%d]. Croping[%d].", keyLen, algorithmKeyLen ));
        keyLen = algorithmKeyLen;
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS )
    {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    header.hChannel = hHsm->channelHandles[BSP_CmdInterface];
    header.continualMode = pIo->contMode;
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_SHA1, &header );

    if( pIo->oprMode == BPI_HmacSha1_Op_eHmac && pIo->keySource == BHSM_HMACSHA_KeySource_eKeyLadder )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eVKL, BHSM_RemapVklId(pIo->VirtualKeyLadderID) );
        BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eKeyLayer, pIo->keyLayer );
    }
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eOperation,  pIo->oprMode );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eIncludeKey, pIo->keyIncMode );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eIsAddress, (pIo->unInputDataLen > MIN_DATA_BY_DRAM) ? 1 : 0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eShaType,    pIo->shaType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eContextId,  pIo->contextSwitch );

    offset_Key = ROUND_UP_4( BCMD_HmacSha1_InCmdField_eContextId  );
    offset_DataLength = ROUND_UP_4( offset_Key + algorithmKeyLen );
    offset_Data = ROUND_UP_4( offset_DataLength + 1 );

    #if BHSM_ZEUS_VERSION != BHSM_ZEUS_VERSION_CALC(1,0)  /*Zues 1 does not have the key length paramter. */
    if ( hHsm->hsmPiRunningFullRom == false )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eKeyLen, algorithmKeyLen );

        /* adjust offset of location of subsequent paramters.  */
        offset_Key += 4;
        offset_DataLength += 4;
        offset_Data += 4;
    }
    #endif

    BHSM_BspMsg_PackArray( hMsg, offset_Key, pIo->keyData, keyLen );

    BHSM_BspMsg_Pack32( hMsg, offset_DataLength, pIo->unInputDataLen );

    if( pIo->unInputDataLen <= MIN_DATA_BY_DRAM ) /* include it within the BSP packet */
    {
        BHSM_BspMsg_PackArray( hMsg, offset_Data, pIo->pucInputData, pIo->unInputDataLen );
    }
    else
    {
        rc = BMEM_Heap_ConvertAddressToOffset( hHsm->hHeap, (void*)pIo->pucInputData, &inputDataOffset );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

        BHSM_BspMsg_Pack32( hMsg, offset_Data, inputDataOffset );
    }

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS )
    {
        rc = BERR_TRACE(rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    pIo->unStatus = (uint32_t)status;
    if(    ( status != 0 )
        && ( status != 0xA8 )    /* more data expected. */ )
    {
        BDBG_ERR(( "BHSM_UserHmacSha failed status[0x%02X]", status ));
        rc = BHSM_STATUS_FAILED;
        goto BHSM_P_DONE_LABEL;
    }

    /* Collect output */
    BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &digestSize );
    digestSize -= 4; /* remove status paramter bytes from length */

    if( ( digestSize > 0 ) && ( digestSize <= sizeof(pIo->aucOutputDigest) ) )
    {
        BHSM_BspMsg_GetArray( hMsg, BCMD_HmacSha1_OutCmdField_eDigest, pIo->aucOutputDigest, digestSize );
        pIo->digestSize = digestSize;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_P_UserHmacSha_impl );
    return rc;
}

/*
    BHSM_UserHmacSha performs prepocesing on the input parameters:
        - if a byte swap is reqeusted, or memory data is stored in is no BMEM, a
          shadow buffer is created.
        - if the data is greater that 8M, it is submitted in chunks of of 8M to the BSP.

*/
BERR_Code BHSM_UserHmacSha ( BHSM_Handle hHsm, BHSM_UserHmacShaIO_t *pIo )
{
    BERR_Code rc = BERR_SUCCESS;

    #define MAX_BYTE_SWAP_BUFFER_SIZE (1024*1024)
    #define MAX_TRANSFER_SIZE 0x800000 /* max allowed by BSP. */

    /* handle a byteswap, or sysem memory. */
    if( pIo->byteSwap || pIo->systemMemory )
    {
        uint8_t *pInputData = pIo->pucInputData;
        uint32_t inputDataLength = pIo->unInputDataLen;
        uint8_t *pBuf = NULL;
        unsigned bufSize  = MIN( pIo->unInputDataLen, MAX_BYTE_SWAP_BUFFER_SIZE );
        void* pCachedBufPtr = NULL; /* ACPU accessible pointer. */
        BHSM_HMACSHA_ContinualMode_e contModeOrig = pIo->contMode;

        pBuf = (uint8_t*)BMEM_AllocAligned( hHsm->hHeap, ROUND_UP_4(bufSize), 6, 0 );
        if( pBuf == NULL ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

        rc = BMEM_ConvertAddressToCached( hHsm->hHeap, pBuf, &pCachedBufPtr );
        if( rc != BERR_SUCCESS )
        {
            BMEM_Free( hHsm->hHeap, pBuf );
            return BERR_TRACE( BHSM_STATUS_MEMORY_PHYCOVERTING_ERR );
        }

        if( inputDataLength > bufSize )
        {
            pIo->contMode  = BHSM_HMACSHA_ContinualMode_eMoreSeg;
        }

        pIo->unInputDataLen = bufSize;
        pIo->pucInputData = pBuf;

        while( inputDataLength > bufSize )
        {
            BHSM_MemcpySwap( pCachedBufPtr, pInputData, bufSize, pIo->byteSwap );
            BMEM_FlushCache( hHsm->hHeap, pCachedBufPtr, ROUND_UP_4(bufSize) );

            rc = BHSM_P_UserHmacSha_impl( hHsm, pIo );
            if( rc != BERR_SUCCESS )
            {
                BMEM_Free ( hHsm->hHeap, pBuf );
                return BERR_TRACE(rc);
            }

            pInputData += bufSize;
            inputDataLength -= bufSize;
        }

        pIo->unInputDataLen = inputDataLength;
        pIo->contMode = contModeOrig;

        BHSM_MemcpySwap( pCachedBufPtr, pInputData, pIo->unInputDataLen, pIo->byteSwap );
        BMEM_FlushCache( hHsm->hHeap, pCachedBufPtr, ROUND_UP_4(bufSize) );

        rc = BHSM_P_UserHmacSha_impl( hHsm, pIo );
        if( rc != BERR_SUCCESS )
        {
            BMEM_Free ( hHsm->hHeap, pBuf );
            return BERR_TRACE(rc);
        }

        BMEM_Free ( hHsm->hHeap, pBuf );
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
            rc = BHSM_P_UserHmacSha_impl( hHsm, pIo );
            if( rc != BERR_SUCCESS ){ return BERR_TRACE(rc); }

            inputDataLength -= MAX_TRANSFER_SIZE;
            pIo->pucInputData += MAX_TRANSFER_SIZE;

        }while( inputDataLength > MAX_TRANSFER_SIZE );

        pIo->unInputDataLen = inputDataLength;
        pIo->contMode = contModeOrig;

        rc = BHSM_P_UserHmacSha_impl( hHsm, pIo );
        if( rc != BERR_SUCCESS ){ return BERR_TRACE(rc); }

        return BERR_SUCCESS;
    }

    return BHSM_P_UserHmacSha_impl( hHsm, pIo );
}
