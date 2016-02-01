/***************************************************************************
 *    (c)2008-2015 Broadcom Corporation
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
 ***************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "nexus_memory.h"
#include "nexus_security.h"
#include "nexus_keyladder.h"

#include "decrypto.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#define CHK_RC(_rc)                                      \
{                                                        \
    if(_rc) {                                            \
        rc = _rc;                                        \
        goto ErrorExit;                                  \
    }                                                    \
}

static NEXUS_KeySlotHandle hDeKeySlot;

#if (BCHP_CHIP == 7420)
/******************************************************************************
 ** FUNCTION
 **   Crypto_Aes_RouteKey4(
 **
 ** DESCRIPTION:
 **  Route key4 to M2M keyslot
 **
 ** PARAMETERS:
 **   pProcIn1 -  proc in1 data
 **   pProcIn2 -  proc in2 data
 **   pProcIn2Len - size of proc in2
 **   keyVar_lo  -  cuskey low
 **   keyVar_hi  -  cuskey high
 **   CuskeySel - custom key sel
 **   hKeySlot -  destination key slot
 **
 ** RETURNS:
 **   true when the operation is succesful or an error.
 **
 ******************************************************************************/
static bool Crypto_Aes_RouteKey4(
    const uint8_t *pProcIn1,
    const uint8_t *pProcIn2,
    uint8_t  pProcIn2Len,
    uint8_t  KeyVar_lo,
    uint8_t  KeyVar_hi,
    uint8_t  CusKeySel,
    NEXUS_KeySlotHandle hKeySlot)
{
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;

    encryptedSessionkey.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    /*encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eDecrypt;*/
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.cusKeyL = CusKeySel;
    encryptedSessionkey.cusKeyH = CusKeySel;
    encryptedSessionkey.cusKeyVarL = KeyVar_lo;
    encryptedSessionkey.cusKeyVarH = KeyVar_hi;

    encryptedSessionkey.virtualKeyLadderID=NEXUS_SecurityVirtualKeyladderID_eVKL2;
    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.keyMode     = NEXUS_SecurityKeyMode_eRegular;


    /*encryptedSessionkey.pKeyData = pProcIn1; proc-in1, size is always 128bit*/
    memcpy(encryptedSessionkey.keyData, pProcIn1, 16);
    encryptedSessionkey.bRouteKey = false;

    encrytedCW.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize =  pProcIn2Len;
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;

    encrytedCW.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
    encrytedCW.virtualKeyLadderID=NEXUS_SecurityVirtualKeyladderID_eVKL2;
    encrytedCW.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.keyMode     = NEXUS_SecurityKeyMode_eRegular;


    /* ++++++++ */
    encrytedCW.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKL2;
    encrytedCW.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */

    /*encrytedCW.pKeyData = pProcIn2;  proc-in2, region key*/
    memcpy(encrytedCW.keyData, pProcIn2,16);
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;

    /* Route key for Decryption keyslot */
    if (NEXUS_Security_GenerateSessionKey(hKeySlot, &encryptedSessionkey) != NEXUS_SUCCESS)
        return false;

    if (NEXUS_Security_GenerateControlWord(hKeySlot, &encrytedCW) != NEXUS_SUCCESS)
        return false;

    return true;
}
#elif (BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7231 || \
                BCHP_CHIP == 7344 || BCHP_CHIP == 7346)
static bool Crypto_Aes_RouteKey4(
    const uint8_t *pProcIn1,
    const uint8_t *pProcIn2,
    uint8_t  pProcIn2Len,
    uint8_t  KeyVar_lo,
    uint8_t  KeyVar_hi,
    uint8_t  CusKeySel,
    NEXUS_KeySlotHandle hKeySlot)
{
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;

    NEXUS_Security_GetDefaultSessionKeySettings(&encryptedSessionkey);

    encryptedSessionkey.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.cusKeyL = CusKeySel;
    encryptedSessionkey.cusKeyH = CusKeySel;
    encryptedSessionkey.cusKeyVarL = KeyVar_lo;
    encryptedSessionkey.cusKeyVarH = KeyVar_hi;

    encryptedSessionkey.keyGenCmdID       = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp      = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode         = false;
    encryptedSessionkey.rootKeySrc        = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.bSwapAESKey       = false;
    encryptedSessionkey.keyDestIVType     = NEXUS_SecurityKeyIVType_eNoIV;


    /*encryptedSessionkey.pKeyData = pProcIn1; proc-in1, size is always 128bit*/
    memcpy(encryptedSessionkey.keyData, pProcIn1, 16);
    encryptedSessionkey.bRouteKey = false;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eOdd;


    encryptedSessionkey.virtualKeyLadderID=NEXUS_SecurityVirtualKeyladderID_eVKL2;
    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.keyMode     = NEXUS_SecurityKeyMode_eRegular;

    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCW);

    encrytedCW.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize =  pProcIn2Len;
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;


    /* ++++++++ */
    encrytedCW.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKL2;
    encrytedCW.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */

    /*encrytedCW.pKeyData = pProcIn2;  proc-in2, region key*/
    memcpy(encrytedCW.keyData, pProcIn2,16);
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;

    /* Route key for Decryption keyslot */
    if (NEXUS_Security_GenerateSessionKey(hKeySlot, &encryptedSessionkey) != NEXUS_SUCCESS)
        return false;

    if (NEXUS_Security_GenerateControlWord(hKeySlot, &encrytedCW) != NEXUS_SUCCESS)
        return false;

    return true;
}

#else
/******************************************************************************
** FUNCTION
**   Crypto_Aes_RouteKey4(
**
** DESCRIPTION:
**  Route key4 to M2M keyslot
**
** PARAMETERS:
**   pProcIn1 -  proc in1 data
**   pProcIn2 -  proc in2 data
**   pProcIn2Len - size of proc in2
**   keyVar_lo  -  cuskey low
**   keyVar_hi  -  cuskey high
**   CuskeySel - custom key sel
**   hKeySlot -  destination key slot
**
** RETURNS:
**   true when the operation is succesful or an error.
**
******************************************************************************/
static bool Crypto_Aes_RouteKey4(
    const uint8_t *pProcIn1,
    const uint8_t *pProcIn2,
    uint8_t  pProcIn2Len,
    uint8_t  KeyVar_lo,
    uint8_t  KeyVar_hi,
    uint8_t  CusKeySel,
    NEXUS_KeySlotHandle hKeySlot)
{
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;

    encryptedSessionkey.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    /*encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eDecrypt;*/
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.cusKeyL = CusKeySel;
    encryptedSessionkey.cusKeyH = CusKeySel;
    encryptedSessionkey.cusKeyVarL = KeyVar_lo;
    encryptedSessionkey.cusKeyVarH = KeyVar_hi;
    /*encryptedSessionkey.pKeyData = pProcIn1; proc-in1, size is always 128bit*/
    memcpy(encryptedSessionkey.keyData, pProcIn1, 16);
    /*encryptedSessionkey.pKeyData= pProcIn1; */
    encryptedSessionkey.bRouteKey = false;

    encrytedCW.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize =  pProcIn2Len;
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(encrytedCW.keyData, pProcIn2,16);
    /*encrytedCW.pKeyData = pProcIn2;   */
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;

    /* Route key for Decryption keyslot */
    if (NEXUS_Security_GenerateSessionKey(hKeySlot, &encryptedSessionkey) != NEXUS_SUCCESS)
        return false;

    if (NEXUS_Security_GenerateControlWord(hKeySlot, &encrytedCW) != NEXUS_SUCCESS)
        return false;

    return true;
}
#endif

/******************************************************************************
** FUNCTION
**  Crypto_M2MTransfer
**
** DESCRIPTION:
**  Do a M2M DMA transfer based on specified key slot
**
** PARAMETERS:
**   hDma       -  dma for M2M transfer
**   pSrc       -  DMA source buffer
**   pDest      -  DMA destination buffer
**   pSize      -  DMA size
**   hKeyHandle -  M2M key slot
**
** RETURNS:
**   true when the operation is succesful or an error.
**
******************************************************************************/
static bool  Crypto_M2MTransfer(NEXUS_DmaHandle    hDma,
                           NEXUS_KeySlotHandle keyHandle,
                               uint8_t            *pSrc,
                               uint8_t            *pDest,
                               uint32_t            size)
{
    NEXUS_DmaJobSettings  dmaJobSetting;
    NEXUS_DmaSettings dmaSettings;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobHandle hDmaJob;
    NEXUS_DmaJobStatus jobStatus;


    if ( !hDma || !pSrc || !pDest || !size )
        return false;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    dmaSettings.endianMode = NEXUS_DmaEndianMode_eBig;
#else
    dmaSettings.endianMode = NEXUS_DmaEndianMode_eLittle;
#endif
    dmaSettings.swapMode = NEXUS_DmaSwapMode_eNone;
    NEXUS_Dma_SetSettings (hDma, &dmaSettings);

    NEXUS_DmaJob_GetDefaultSettings (&dmaJobSetting);
    dmaJobSetting.numBlocks = 1;
    dmaJobSetting.completionCallback.callback = NULL;
    dmaJobSetting.dataFormat = NEXUS_DmaDataFormat_eBlock;
    dmaJobSetting.keySlot = keyHandle;

    if ( (hDmaJob = NEXUS_DmaJob_Create (hDma, &dmaJobSetting)) == NULL )
        return false;

    NEXUS_DmaJob_GetDefaultBlockSettings (&blockSettings);
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDest;
    blockSettings.blockSize = size;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = true;
    blockSettings.scatterGatherCryptoEnd = true;
#if (BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7231 || \
                BCHP_CHIP == 7344 || BCHP_CHIP == 7346)
    NEXUS_DmaJob_ProcessBlocks(hDmaJob, &blockSettings, 1);
    for (;;)
    {
        NEXUS_DmaJob_GetStatus(hDmaJob, &jobStatus);
        if(jobStatus.currentState == NEXUS_DmaJobState_eComplete)
        {
            break;
        }
        BKNI_Sleep(1);
    }
#else
    if ( NEXUS_DmaJob_SetBlockSettings (hDmaJob, 0, &blockSettings)!= BERR_SUCCESS )
        return false;

    if ( NEXUS_DmaJob_Start (hDmaJob)!= BERR_SUCCESS )
        return false;

    jobStatus.currentState = NEXUS_DmaJobState_eUnknown;
    while (jobStatus.currentState != NEXUS_DmaJobState_eComplete)
    {
        NEXUS_DmaJob_GetStatus(hDmaJob, &jobStatus);
        if ( jobStatus.currentState == NEXUS_DmaJobState_eComplete )
           break;
        BKNI_Sleep(1);

    }
#endif
    NEXUS_DmaJob_Destroy (hDmaJob);
    return true;
}

/******************************************************************************
** FUNCTION
**   _Crypto_3Des
**
** DESCRIPTION:
**    Do an AES ECB crypto operation on a buffer
**
** PARAMETERS:
**   pBuf - Buffer to encrypt/decrypt.
**   buf_size - Size of the buffer.
**   hKeySlot - key slot for Crypto operation
**
** RETURNS:
**   NEXUS_SUCCESS when the operation is succesful or an error.
**
******************************************************************************/
static NEXUS_Error _Crypto_3Des(
    uint8_t *pBuf,
    uint32_t buf_size,
    NEXUS_KeySlotHandle hKeySlot
)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_DmaHandle     hDma;
    uint8_t  *dmaBuf;

    BDBG_ASSERT(pBuf != NULL);
    BDBG_ASSERT(buf_size != 0);


     /*geti the key4 from key ladder and put into the keyslot of M2M*/
    if(!  Crypto_Aes_RouteKey4(FLASH_PROC_IN1_DATA,
                               FLASH_PROC_IN2_DATA,
                               16,  /*region key size*/
                               FLASH_KEY_KEY_VAR_LOW,
                               FLASH_KEY_KEY_VAR_HIGH,
                               CUSTOMER_KEY_SELECT,
                   hKeySlot))
    {
    CHK_RC(NEXUS_INVALID_PARAMETER);
    }

    /*get dma*/
    hDma = NEXUS_Dma_Open(0, NULL);

   /*allocate a continuous physical address buffer for DMA */
    if (NEXUS_Memory_Allocate(buf_size, NULL,(void **) &dmaBuf))
    {
    CHK_RC(NEXUS_INVALID_PARAMETER);
    }

    /*copy content to DMA buffer*/
    BKNI_Memcpy(dmaBuf, pBuf, buf_size);

    /*start M2M transfer*/
    if (! Crypto_M2MTransfer(hDma, hKeySlot, dmaBuf, dmaBuf, buf_size))
    {
    CHK_RC(NEXUS_INVALID_PARAMETER);
    }

    /*copy the result back */
    BKNI_Memcpy(pBuf, dmaBuf, buf_size);


ErrorExit:

    NEXUS_Memory_Free(dmaBuf);/*free the DMA buffer*/
    NEXUS_Dma_Close(hDma);
    return rc;
}


/******************************************************************************
** FUNCTION
**   drm_nf_crypto
**
** DESCRIPTION:
**   This function is used to do a crypto operation on a buffer.
**
** PARAMETERS:
**   pBuf - Pointer to the buffer.
**   size - Size of the buffer in bytes
**   op - Operation to do on the buffer, either BDVD_CRYPTO_NETFLIX_DEC or
**        BDVD_CRYPTO_NETFLIX_ENC
**
** RETURNS:
**   NEXUS_SUCCESS when the operation is succesful or an error.
**
******************************************************************************/
NEXUS_Error keyladder_crypto(uint8_t *pBuf, uint32_t size)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!hDeKeySlot)
    {
       printf("keyslot is not allocated! call Initialize_KeySlot() first\n");
       CHK_RC(NEXUS_INVALID_PARAMETER);
    }

    rc = _Crypto_3Des(pBuf, size, hDeKeySlot);
    CHK_RC(rc);

ErrorExit:
    return rc;

}

/******************************************************************************
** FUNCTION:
** Initialize_KeySlot
**
** DESCRIPTION:
**   This function initialize M2M key slots
******************************************************************************/
void Initialize_KeySlot(void)
{
    NEXUS_SecurityKeySlotSettings keySetting;
    NEXUS_SecurityAlgorithmSettings algConfig;

     /* Allocating  keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySetting);
    keySetting.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySetting.keySlotSource = NEXUS_SecurityKeySource_eFirstRam;

    hDeKeySlot = NEXUS_Security_AllocateKeySlot(&keySetting);

    NEXUS_Security_GetDefaultAlgorithmSettings(&algConfig);
    algConfig.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;
    algConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    algConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;

    /* Configure Decryption Keyslot */
    algConfig.operation = NEXUS_SecurityOperation_eDecrypt;
    NEXUS_Security_ConfigAlgorithm(hDeKeySlot, &algConfig);
}


/******************************************************************************
** FUNCTION:
**  Finalize_KeySlot
**
** DESCRIPTION:
**  This function frees the key slots for encryption/decryption
******************************************************************************/
void Finalize_KeySlot(void)
{
    NEXUS_Security_FreeKeySlot(hDeKeySlot);
}
