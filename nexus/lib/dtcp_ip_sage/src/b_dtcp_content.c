/********************************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*
 * Module Description:
 *     DTCP-IP Content management component
 *
 *********************************************************************************************/
#include <arpa/inet.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_stream.h"
#include "b_dtcp_content.h"
#include "b_dtcp_ip_constants.h"
#include "b_dtcp_status_codes.h"

#include "bcrypt_aescbc_sw.h"
#include "bcrypt_aesecb_sw.h"

BDBG_MODULE(b_dtcp_ip);
/*
 * DTCP-IP content key constants, defined in b_ecc_wrappers.c
 */
extern unsigned char gCa0[];
extern unsigned char gCb1[];
extern unsigned char gCb0[];
extern unsigned char gCc1[];
extern unsigned char gCc0[];
extern unsigned char gCd0[];

/*! \brief Data associated with a protected content packet */

#ifdef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
typedef struct DtcpPacketData
{
    BCRYPT_S_AESCBCSwCtrl_t *cipherContext;
    unsigned char NextIv[DTCP_AES_IV_SIZE];
    unsigned int    PayloadSize;
    unsigned int    BytesProcessed;
    B_ExEMI_T        ExtendedEmi;
} DtcpPacketData;
typedef struct {
    void * dma_in;
    void * dma_out;
    unsigned int  LastBufferLength;
} dma_t;
static dma_t  DtcpDma;


#include "drm_dtcp_ip.h"
#include "drm_common.h"
#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_platform_client.h"
static B_MutexHandle hSecurityMutex;

#else
#endif

void DtcpCleanupHwSecurityParams(void)
{
#ifdef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
    BDBG_MSG(("%s - Entered function", __FUNCTION__));
    if(hSecurityMutex != NULL)
        B_Mutex_Destroy(hSecurityMutex);
    hSecurityMutex = NULL;
    if(DtcpDma.dma_in != NULL)
        NEXUS_Memory_Free(DtcpDma.dma_in);
    DtcpDma.dma_in = NULL;

    if(DtcpDma.dma_out != NULL)
        NEXUS_Memory_Free(DtcpDma.dma_out);
    DtcpDma.dma_out = NULL;
    DtcpDma.LastBufferLength = 0;
#else
#endif /*B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT*/
}

#ifndef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
static void DtcpEncCompleteCallback(void *pParam, int iParam)
{
    BSTD_UNUSED(iParam);
    BKNI_SetEvent(pParam);
}
#endif

/*
 * Must be called by app, if using hardware descryption.
 */
#ifdef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
BERR_Code DtcpInitHWSecurityParams(void *nexusDmaHandle)
{
    BERR_Code rc = BERR_SUCCESS;
    BSTD_UNUSED(nexusDmaHandle);

    BDBG_MSG(("%s - Entered function", __FUNCTION__));
#if 0
    if(DRM_DtcpIp_Vendor_Initialize() != Drm_Success) {
        BDBG_ERR(("%s - Error initializing DRM DTCP-IP module", __FUNCTION__));
        rc = BERR_UNKNOWN;
        goto error;
    }
#endif
    if((hSecurityMutex = B_Mutex_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed: B_Mutex_Create\n"));
        goto error;
    }


error:
    return rc;
}
#endif /* end of B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT */

#ifndef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
static BERR_Code LoadKeyIv(unsigned char * CipherKey, unsigned char * CipherIv)
{
    BERR_Code retValue = BERR_SUCCESS;
    NEXUS_SecurityClearKey key;
    BDBG_ENTER(LoadKeyIv);

    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keySize = DTCP_AES_KEY_SIZE;
#if (BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7231 || \
        BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 7429)
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
#endif
    BKNI_Memcpy(key.keyData, CipherKey, DTCP_AES_KEY_SIZE);

    if(NEXUS_Security_LoadClearKey(DecKeyHandle, &key) != 0)
    {
        BDBG_ERR(("Failed to load key\n"));
        retValue = BERR_HW_SECURITY_FAILURE;
    }

    key.keySize = DTCP_AES_IV_SIZE;
#if (BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7231 || \
        BCHP_CHIP == 7344 || BCHP_CHIP == 7346 || BCHP_CHIP == 7429)
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
#else
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
#endif
    BKNI_Memcpy(key.keyData, CipherIv, DTCP_AES_IV_SIZE);

    if(NEXUS_Security_LoadClearKey(DecKeyHandle, &key) != 0)
    {
        BDBG_ERR(("Failed to load iv\n"));
        retValue = BERR_HW_SECURITY_FAILURE;
    }
    BDBG_LEAVE(LoadKeyIv);

    return retValue;
}
#endif /* end of B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT */

#if 0
static BERR_Code CreateContentKey(BCRYPT_Handle ghBcrypt, B_ExEMI_T aExtendedEmi,
                     unsigned char aExchangeKey[DTCP_EXCHANGE_KEY_SIZE],
                     unsigned char aNonce[DTCP_CONTENT_KEY_NONCE_SIZE],
                     unsigned char aCipherKey[DTCP_AES_KEY_SIZE],
                     unsigned char aCipherIv[DTCP_AES_IV_SIZE])
#else
static BERR_Code CreateContentKey(DRM_DtcpIpTlHandle hDtcpIpTl, B_ExEMI_T aExtendedEmi,
                     unsigned char aExchangeKey[DTCP_EXCHANGE_KEY_SIZE],
                     unsigned char aNonce[DTCP_CONTENT_KEY_NONCE_SIZE],
                     unsigned char aCipherKey[DTCP_AES_KEY_SIZE],
                     unsigned char aCipherIv[DTCP_AES_IV_SIZE],
                     unsigned int keyslotID)
#endif
{
    BERR_Code retValue = BERR_SUCCESS;
    #if 0
    unsigned char contentKeyConstant[DTCP_CONTENT_KEY_CONSTANT_SIZE] = {0};
    unsigned char ivConstant[DTCP_IV_CONSTANT_SIZE] = {0};
    unsigned char keyGenBuffer[2 * DTCP_AES_BLOCK_SIZE] = {0};
    unsigned char ivGenBuffer[DTCP_CONTENT_KEY_NONCE_SIZE + DTCP_IV_CONSTANT_SIZE] = {0};
    BCRYPT_S_AESECBSwCtrl_t cipherContext ;
    int i;

    BDBG_ASSERT(aExchangeKey);
    BDBG_ASSERT(aNonce);
    BDBG_ASSERT(aCipherKey);

    if (B_ExtEmi_eCopyNever == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCa0, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eCopyOneFC == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCb1, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eCopyOneFN == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCb0, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eMove == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCc1, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eNoMoreCopy == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCc0, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eCopyFreeEPN == aExtendedEmi)
    {
        BKNI_Memcpy(contentKeyConstant, gCd0, DTCP_CONTENT_KEY_CONSTANT_SIZE);
    }
    else if (B_ExtEmi_eCopyFree == aExtendedEmi)
    {
        /*retValue = SUCCESS_FALSE; */
    }
    else
    {
        retValue = BERR_INVALID_PARAMETER;
    }

    BKNI_Memcpy(keyGenBuffer,
           aExchangeKey,
           DTCP_EXCHANGE_KEY_SIZE);
    BKNI_Memcpy(keyGenBuffer + DTCP_EXCHANGE_KEY_SIZE,
           contentKeyConstant,
           DTCP_CONTENT_KEY_CONSTANT_SIZE);
    BKNI_Memcpy(keyGenBuffer + DTCP_EXCHANGE_KEY_SIZE + DTCP_CONTENT_KEY_CONSTANT_SIZE,
           aNonce,
           DTCP_CONTENT_KEY_NONCE_SIZE);
    cipherContext.pIn = keyGenBuffer + DTCP_AES_BLOCK_SIZE;
    cipherContext.pkey = (unsigned char *)keyGenBuffer;
    cipherContext.len = DTCP_AES_BLOCK_SIZE;
    cipherContext.bEncFlag = true;
    cipherContext.pOut = aCipherKey;
    cipherContext.keybits = 128;

    retValue = BCrypt_AESECBSw(ghBcrypt, &cipherContext);
    if(retValue == BCRYPT_STATUS_eOK)
    {
        for (i = 0; i < DTCP_AES_KEY_SIZE; ++i)
        {
            unsigned char *y0 = keyGenBuffer + DTCP_AES_BLOCK_SIZE;
            aCipherKey[i] ^= y0[i];
        }
        /* Create the IV payload buffer */
        {
            ivConstant[0] = 0x95;
            ivConstant[1] = 0xDC;
            ivConstant[2] = 0x3A;
            ivConstant[3] = 0x44;
            ivConstant[4] = 0x90;
            ivConstant[5] = 0x28;
            ivConstant[6] = 0xEB;
            ivConstant[7] = 0x3C;
            BKNI_Memcpy(ivGenBuffer, ivConstant, DTCP_IV_CONSTANT_SIZE);
            BKNI_Memcpy(ivGenBuffer + DTCP_IV_CONSTANT_SIZE, aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
        }
        cipherContext.pIn = ivGenBuffer;
        cipherContext.pkey = aCipherKey;
        cipherContext.len = DTCP_AES_BLOCK_SIZE;
        cipherContext.bEncFlag = true;
        cipherContext.pOut = aCipherIv;
        cipherContext.keybits = 128;

        if((retValue = BCrypt_AESECBSw(ghBcrypt, &cipherContext)) != BCRYPT_STATUS_eOK)
            retValue = BERR_CRYPT_FAILED;

    }else {
        retValue = BERR_CRYPT_FAILED;
    }
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("Creating content Key: exchange key:\n"));
    BDBG_BUFF(aExchangeKey, DTCP_EXCHANGE_KEY_SIZE);
    BDBG_MSG(("\n"));
    BDBG_MSG(("aNonce\n"));
    BDBG_BUFF(aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    BDBG_MSG(("\n"));
    BDBG_MSG(("cipherKey\n"));
    BDBG_BUFF(aCipherKey, DTCP_IP_CONTENT_KEY_SIZE);
#endif
#else
    DrmRC rc = Drm_Err;
    rc = DRM_DtcpIpTl_CreateContentKey(hDtcpIpTl, aExtendedEmi, aExchangeKey, aNonce, aCipherKey, aCipherIv, keyslotID);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s:%d - DRM_DtcpIpTl_CreateContentKey() failed\n",__FUNCTION__,__LINE__));
        retValue = BERR_CRYPT_FAILED;
    }
#endif

    return retValue;
}
void B_DTCP_Content_ClosePacket(B_PacketHandle_T pPacketHandle)
{
    DtcpPacketData *data = (DtcpPacketData *)pPacketHandle;
    BDBG_ASSERT(data);

    if(data->cipherContext)
    {
    #if 0
        BKNI_Free(data->cipherContext->pkey);
        BKNI_Free(data->cipherContext);
    #endif
    }

    BKNI_Free(data);
}

/*! \brief B_DTCP_Content_CreatePacketHeader create pcp header, called by source device only.
 *  \param[in] hAkehandle handle to AKE session.
 *  \param[in] Transport Stream transport type.
 *  \param[in] StreamData dtcp_ip stream data pointer
 *  \param[in] ContentSize total size of the content.
 *  \param[in,out] pPacketHeader Buffer to hold created packet header
 */
BERR_Code B_DTCP_Content_CreatePacketHeader( B_AkeHandle_T hAkeHandle, B_StreamTransport_T Transport,
        struct __b_dtcp_stream_data *StreamData,
        int ContentSize,
        unsigned char pPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE])
{
    BERR_Code retValue = BERR_SUCCESS;
    B_DTCP_IP_PCPHeader_T * pcph = NULL;
    DtcpPacketData *data = NULL;
    unsigned char exchangeKeyLabel = 0;
    unsigned char cipherKey[DTCP_AES_KEY_SIZE] = {0};
    unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE] = {0};
    unsigned char nonce[DTCP_CONTENT_KEY_NONCE_SIZE] = {0};
    unsigned char cipherIv[DTCP_AES_IV_SIZE] = {0};
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)hAkeHandle;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(pPacketHeader);
    BDBG_ENTER(B_DTCP_Content_CreatePacketHeader);

    /*
     * Caller must make sure they size is less then max dtcp packet size
     * before calling this function.
     */
    if(ContentSize > DTCP_MAXIMUM_PROTECTED_PACKET_SIZE)
        return BERR_CONTENT_SIZE_TOO_LARGE;

    if( (data = BKNI_Malloc(sizeof(DtcpPacketData)) ) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }

    data->BytesProcessed = 0;
    data->PayloadSize = ContentSize;
    data->ExtendedEmi = StreamData->Emi;

    /* Padding bytes ?*/
    if(data->PayloadSize % DTCP_AES_BLOCK_SIZE != 0)
    {
        data->PayloadSize += DTCP_AES_BLOCK_SIZE - (data->PayloadSize % DTCP_AES_BLOCK_SIZE);
    }

    /* Get Exchange key, label and Nc */
    if((retValue = B_DTCP_GetExchKeyFromCore(pSession->pAkeCoreData, B_ExchKeyCipher_eAes,
                    exchangeKey, &exchangeKeyLabel)) != BERR_SUCCESS)
    {
        retValue = BERR_UNKNOWN;
        goto ERR_FREE;
    }
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("Source Exchange Key Label: %d\n", exchangeKeyLabel));
    BDBG_MSG(("Source Exchange Key:\n"));
    BDBG_BUFF(exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
    BDBG_MSG(("\n"));
#endif
    if (pSession->pcp_ur_cap == 1) {
        B_DTCP_IP_PCP_UR_T *pcp_ur = (B_DTCP_IP_PCP_UR_T *)nonce;
        pcp_ur->UR_Mode = StreamData->ur_mode;
        pcp_ur->Content_type = StreamData->content_type;
        pcp_ur->APS = StreamData->aps;
        pcp_ur->ICT = StreamData->ict;
        pcp_ur->Source = 1;
        pcp_ur->AST = StreamData->ast;
        pcp_ur->Reserved = 0;
        BDBG_MSG(("stream attribute: pcp_ur: \nur_mode=%d\nContent_type=%d\nAPS=%d\nICT=%d\nSource=%d\nAST=%d\n",
                pcp_ur->UR_Mode, pcp_ur->Content_type, pcp_ur->APS, pcp_ur->ICT, pcp_ur->Source, pcp_ur->AST));
        BKNI_Memcpy(&nonce[2], &StreamData->HttpNonce[2], DTCP_CONTENT_KEY_NONCE_SIZE - 2);
    } else {
        BKNI_Memcpy(nonce, StreamData->HttpNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    }
    if(Transport == B_StreamTransport_eRtp ||
            Transport == B_StreamTransport_eUdp)
    {
        /* TODO: For RTP, Nc is updating periodically in a seperated thread.*/
        retValue = BERR_NOT_SUPPORTED;
        goto ERR_FREE;
    }
    if((retValue = CreateContentKey(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, StreamData->Emi,
                    exchangeKey, nonce, cipherKey, cipherIv, StreamData->keySlotID))
            != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to create content key\n"));
        retValue = BERR_UNKNOWN;
        goto ERR_FREE;
    }

    /* Popuplate packet header */
    pcph = (B_DTCP_IP_PCPHeader_T *)pPacketHeader;
    BKNI_Memset(pcph, 0, sizeof(B_DTCP_IP_PCPHeader_T));
    pcph->C_A = 0;    /* Cipher Algorithm, 0 AES128 */
    pcph->Emi = data->ExtendedEmi & 0x0f;
    pcph->ExchKeyLabel = exchangeKeyLabel;
    pcph->CL = htonl(ContentSize);
    BKNI_Memcpy(pcph->Nc, nonce, DTCP_CONTENT_KEY_NONCE_SIZE);

#ifdef DTCP_DEMO_MODE
    /* Dump packet raw header */
    BDBG_MSG(("pcp header:\n"));
    BDBG_BUFF(pPacketHeader, DTCP_CONTENT_PACKET_HEADER_SIZE);
#endif
    StreamData->hPacketHandle = data;

    BDBG_LEAVE(B_DTCP_Content_CreatePacketHeader);
    return retValue;
ERR_FREE:
    BKNI_Free(data);
    return retValue;
}


/*! \brief B_DTCP_Content_ConsumePacketHeader consume pcp header, called by sink device only.
 *  \param[in] hakeHandle handle to active AKE session.
 *  \param[in] pPacketHeader Buffer to hold packet header
 *  \param[in] StreamData stream data pointer.
 */
BERR_Code B_DTCP_Content_ConsumePacketHeader(B_AkeHandle_T hAkeHandle,
        unsigned char * pPacketHeader,
        struct __b_dtcp_stream_data *StreamData)
{
    B_DTCP_IP_PCPHeader_T * pcph = NULL;
    B_DTCP_IP_PCPURHeader_T *pcpurh = NULL;
    BERR_Code retValue = BERR_SUCCESS;
    DtcpPacketData *data = NULL;
    unsigned char exchKeyLabel = 0;
    unsigned char packetExchKeyLabel = 0;
    unsigned char cipherKey[DTCP_AES_KEY_SIZE] = {0};
    unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE] = {0};
    unsigned char nonce[DTCP_CONTENT_KEY_NONCE_SIZE] = {0};
    unsigned char cipherIv[DTCP_AES_IV_SIZE]= {0};
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)hAkeHandle;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(hAkeHandle);
    BDBG_ASSERT(pPacketHeader);
    BDBG_ENTER(B_DTCP_Content_ConsumePacketHeader);

    if(B_DTCP_IP_IsContKeyConfirmed(pSession) == false)
    {
        BDBG_ERR(("Content Key not confirmed! stopping decryption\n"));
        return BERR_CONT_KEY_CONF_FAILED;
    }

#ifdef DTCP_DEMO_MODE
    /* Dump packet raw header */
    BDBG_MSG(("pcp header:\n"));
    BDBG_BUFF(pPacketHeader, DTCP_CONTENT_PACKET_HEADER_SIZE);
#endif
    /* Allocate packet handle data. */
    if((data = (DtcpPacketData *)BKNI_Malloc(sizeof(DtcpPacketData))) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }
    if (pSession->pcp_ur_cap == 1) {
        pcpurh = (B_DTCP_IP_PCPURHeader_T *)pPacketHeader;
        StreamData->Emi = pcpurh->Emi & 0xF;
        StreamData->packet_content_length = ntohl(pcpurh->CL);
        packetExchKeyLabel = pcpurh->ExchKeyLabel;
        /* Extract PCP_UR from pcp header */
        StreamData->ur_mode = pcpurh->pcp_ur.UR_Mode;
        StreamData->content_type = pcpurh->pcp_ur.Content_type;
        StreamData->aps = pcpurh->pcp_ur.APS;
        StreamData->ict = pcpurh->pcp_ur.ICT;
        StreamData->ast = pcpurh->pcp_ur.AST;
        /* NC consit of PC_UR and 48 bits of SNC */
        BKNI_Memcpy(nonce, (void *)&pcpurh->pcp_ur, 2);
        BKNI_Memcpy(&nonce[2], pcpurh->SNc, DTCP_CONTENT_KEY_NONCE_SIZE - 2);
        BDBG_MSG(("stream attribute: pcp_ur:\nur_mode=%d\nContent_type=%d\nAPS=%d\nICT=%d\nAST=%d\n",
                StreamData->ur_mode, StreamData->content_type, StreamData->aps, StreamData->ict, StreamData->ast));

    }else {
        pcph = (B_DTCP_IP_PCPHeader_T *)pPacketHeader;
        StreamData->Emi = pcph->Emi & 0x0F;
        StreamData->packet_content_length = ntohl(pcph->CL);
        packetExchKeyLabel = pcph->ExchKeyLabel;
        BKNI_Memcpy(nonce, pcph->Nc, DTCP_CONTENT_KEY_NONCE_SIZE);
    }

    /* What about the Nc ? */
    /* Initialize packet handle data */

    data->BytesProcessed = 0;
    data->ExtendedEmi = StreamData->Emi;
    data->PayloadSize = StreamData->packet_content_length;

#if 0
    /* Padding */
    if(data->PayloadSize % DTCP_AES_BLOCK_SIZE != 0)
    {
        data->PayloadSize += DTCP_AES_BLOCK_SIZE - (data->PayloadSize % DTCP_AES_BLOCK_SIZE);
    }
#endif
    if(data->PayloadSize > DTCP_MAXIMUM_PROTECTED_PACKET_SIZE)
    {
        retValue= BERR_INVALID_PACKET_HEADER;
        BDBG_ERR(("Payloadsize too large %d \n", data->PayloadSize));
        goto ERR_FREE;
    }
    if((retValue = B_DTCP_GetExchKeyFromSession(pSession, B_ExchKeyCipher_eAes,
                    exchangeKey, &exchKeyLabel)) != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to get exch key from session\n"));
        retValue = BERR_UNKNOWN;
        goto ERR_FREE;
    }
#ifdef DTCP_DEMO_MODE
    BDBG_BUFF(exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
#endif

    if(exchKeyLabel != packetExchKeyLabel)
    {
        BDBG_ERR(("Exchange Key label mis-match! Packet Label: %d / Session Label: %d\n",
                packetExchKeyLabel, exchKeyLabel));
        retValue = BERR_INVALID_EXCHANGE_KEY_LABEL;
        goto ERR_FREE;
    }

    /* Initialze IV */
    /* Store nonce value contained in pcph into AKE Session . */
    B_DTCP_GetSetRealTimeNonce(pSession->hMutex, nonce, pSession->RealTimeNonce);

    /* Create cipher key */
    if( (retValue = CreateContentKey(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, data->ExtendedEmi, exchangeKey, nonce, cipherKey, cipherIv, StreamData->keySlotID))
            != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to create content cipher key and iv\n"));
        goto ERR_FREE;
    }
#if !defined(B_DTCP_IP_HW_DECRYPTION)
    data->cipherContext->keybits = 128;
    data->cipherContext->bEncFlag = false;
    data->cipherContext->piv = data->NextIv;
#endif
    StreamData->hPacketHandle = data;
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("Exchange Key: \n"));
    BDBG_BUFF(exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
    BDBG_MSG(("nonce:\n"));
    BDBG_BUFF(nonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    BDBG_MSG(("Content Key:\n"));
    BDBG_BUFF(cipherKey, DTCP_IP_CONTENT_KEY_SIZE);
    BDBG_MSG(("IV:\n"));
    BDBG_BUFF(data->NextIv, DTCP_AES_IV_SIZE);
#endif
    BDBG_LEAVE(B_DTCP_Content_ConsumePacketHeader);
    return retValue;
ERR_FREE:
    BKNI_Free(data);
    return retValue;
}

/*! \brief B_DTCP_Content_EncryptData encrypt protected content, called by source only.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] aPacketHandle handle to packet data (obtained from create packet header function).
 *  \param[in] aInBuffer input buffer.
 *  \param[in,out] aOutBuffer output buffer.
 *  \param[in] aBufferLength length of input buffer.
 *
 *  When B_DTCP_IP_HW_ENCRYPTION is defined, this function requires the aInBuffer and aOutBuffer
 *  allocated from heap (via NEXUS_Memory_Allocate() ).
 */
BERR_Code B_DTCP_Content_EncryptData(B_AkeHandle_T aAkeHandle, B_PacketHandle_T aPacketData,
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength,
                            NEXUS_KeySlotHandle dtcpIpKeyHandle,
                            bool scatterGatherStart,
                            bool scatterGatherEnd,
                            NEXUS_DmaHandle dtcpIpDmaHandle,
                            unsigned int keySlotID)
{
    BERR_Code retValue = BERR_SUCCESS;
    DtcpPacketData *data = NULL;
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)aAkeHandle;

#ifdef B_DTCP_IP_HW_ENCRYPTION
#ifndef B_DTCP_IP_COMMON_DRM_CONTENT_SUPPORT
    NEXUS_DmaJobStatus jobStatus;
#endif
#endif

    BDBG_ASSERT(aInBuffer);
    BDBG_ASSERT(aOutBuffer);
    BDBG_ASSERT(aPacketData);
    BSTD_UNUSED(keySlotID);
    BDBG_ENTER(B_DTCP_Content_EncryptData);
    if(!pSession)
        return BERR_NOT_INITIALIZED;

    if( aBufferLength % DTCP_AES_BLOCK_SIZE != 0)
    {
        BDBG_ERR(("Buffer length invalid: %d\n", aBufferLength));
        return BERR_INVALID_PARAMETER;
    }

    data = (DtcpPacketData*)aPacketData;
    /*
     * Check that buffer does not exceed length of payload for this packet
     */
    if(data->PayloadSize < data->BytesProcessed + aBufferLength)
    {
        BDBG_ERR(("Content exceeds packet size\n"));
        return BERR_INVALID_PARAMETER;
    }
    if(data->ExtendedEmi == B_ExtEmi_eCopyFree)
    {
        BKNI_Memcpy(aOutBuffer, aInBuffer, aBufferLength);
        return BERR_SUCCESS;
    }
    if (B_ExtEmi_eCopyFree == data->ExtendedEmi)
    {
        /* No encryption, clear buffer */
        BKNI_Memcpy(aOutBuffer, aInBuffer, aBufferLength);
    }
    else
    {
#ifdef B_DTCP_IP_HW_ENCRYPTION
        /* TODO: Verify aInBuffer and aOutBuffer, assert if it's not in the heap memory */
        B_Mutex_Lock(hSecurityMutex);

        if( aInBuffer && aOutBuffer)
        {
            if (DtcpIpTl_EncDecOperation(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                                     aInBuffer,
                                     aBufferLength,
                                     aOutBuffer,
                                     dtcpIpKeyHandle,
                                     scatterGatherStart,
                                     scatterGatherEnd,
                                     dtcpIpDmaHandle) != Drm_Success)
            {
                BDBG_ERR(("%s - Error encrypting", __FUNCTION__));
                retValue = BERR_UNKNOWN;
                B_Mutex_Unlock(hSecurityMutex);
                goto ErrorExit;
            }
            data->BytesProcessed += aBufferLength;
        }

        B_Mutex_Unlock(hSecurityMutex);
#endif
    }

#ifdef B_DTCP_IP_HW_ENCRYPTION
ErrorExit:
#endif
    BDBG_LEAVE(B_DTCP_Content_EncryptData);
    return retValue;
}

#ifdef  B_DTCP_IP_HW_DECRYPTION
static NEXUS_HeapHandle get_full_heap(void)
{
    unsigned i;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (clientConfig.heap[i]) {
            NEXUS_MemoryStatus status;
            NEXUS_Heap_GetStatus(clientConfig.heap[i], &status);
            if (status.memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) break;
        }
    }
    if (i == NEXUS_MAX_HEAPS) return NULL;
    return clientConfig.heap[i];
}
#endif

/*! \brief B_DTCP_Content_DecryptData decrypt protected content, called by source only.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] aPacketHandle handle to packet data (obtained from create packet header function).
 *  \param[in] aInBuffer input buffer.
 *  \param[in,out] aOutBuffer output buffer.
 *  \param[in] aBufferLength length of input buffer.
 */
BERR_Code B_DTCP_Content_DecryptData(B_AkeHandle_T aAkeHandle, B_PacketHandle_T  aPacketHandle,
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength,
                            NEXUS_KeySlotHandle dtcpIpKeyHandle,
                            bool scatterGatherStart,
                            bool scatterGatherEnd,
                            NEXUS_DmaHandle dtcpIpDmaHandle,
                            unsigned int keySlotID)
{
    BERR_Code retValue = BERR_SUCCESS;
    DtcpPacketData *data = (DtcpPacketData *)aPacketHandle;
    unsigned char tempIv[DTCP_AES_IV_SIZE] = {0};
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)aAkeHandle;

    BDBG_ASSERT(pSession);
    BDBG_ASSERT(aInBuffer);
    BDBG_ASSERT(aOutBuffer);
    BDBG_ASSERT(aBufferLength);
    BDBG_ASSERT(aPacketHandle);
    BSTD_UNUSED(keySlotID);

    BDBG_ENTER(B_DTCP_Content_DecryptData);
    if (0 != aBufferLength % DTCP_AES_BLOCK_SIZE)
    {
        retValue = BERR_INVALID_PARAMETER;
    }

    if (BERR_SUCCESS == retValue)
    {
        if (B_ExtEmi_eCopyFree == data->ExtendedEmi)
        {
            BKNI_Memcpy(aOutBuffer, aInBuffer, aBufferLength);
        }
        else
        {
#ifdef  B_DTCP_IP_HW_DECRYPTION

            B_Mutex_Lock(hSecurityMutex);

            #if 0
            (void)pSession;
            #endif
            /* re-allocate DMA buffer if the buffer length changed.*/
            BKNI_Memcpy(tempIv, &aInBuffer[aBufferLength - DTCP_AES_IV_SIZE], DTCP_AES_IV_SIZE);

            if( aInBuffer && aOutBuffer)
            {
                if(aBufferLength > DtcpDma.LastBufferLength)
                {
                    NEXUS_MemoryAllocationSettings settings;
                    NEXUS_HeapHandle heap_handle = get_full_heap();

                    if(DtcpDma.dma_in != NULL)
                        NEXUS_Memory_Free(DtcpDma.dma_in);
                    if(DtcpDma.dma_out != NULL)
                        NEXUS_Memory_Free(DtcpDma.dma_out);

                    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
                    if (!heap_handle) {
                        BDBG_ERR(("NULL heap handle"));
                    }
                    settings.heap = heap_handle;
                    if(NEXUS_Memory_Allocate(aBufferLength, &settings, &DtcpDma.dma_in))
                    {
                        BDBG_ERR(("Failed to allocate DMA memory, len=%d\n", aBufferLength));
                        retValue = BERR_UNKNOWN;
                    }
                    if(NEXUS_Memory_Allocate(aBufferLength, &settings, &DtcpDma.dma_out))
                    {
                        BDBG_ERR(("Failed to allocate DMA memory, len=%d\n", aBufferLength));
                        retValue = BERR_UNKNOWN;
                    }

                    DtcpDma.LastBufferLength = aBufferLength;
                }

                BKNI_Memcpy(DtcpDma.dma_in, aInBuffer, aBufferLength);
                if (DtcpIpTl_EncDecOperation(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                                         DtcpDma.dma_in,
                                         aBufferLength,
                                         DtcpDma.dma_out,
                                         dtcpIpKeyHandle,
                                         scatterGatherStart,
                                         scatterGatherEnd,
                                         dtcpIpDmaHandle) != Drm_Success)
                {
                    BDBG_ERR(("%s - Error decrypting", __FUNCTION__));
                    retValue = BERR_UNKNOWN;
                    B_Mutex_Unlock(hSecurityMutex);
                    goto ErrorExit;
                }
                BKNI_Memcpy(aOutBuffer, DtcpDma.dma_out, aBufferLength);

                /* Save next IV */
                data->BytesProcessed += aBufferLength;
#if 0
                DRM_DtcpIpTl_UpdateKeyIv(pSession->pAkeCoreData->pDeviceParams->hDtcpIpTl, tempIv, keySlotID);
#endif
            }
            B_Mutex_Unlock(hSecurityMutex);
#endif
        }
    }

ErrorExit:
    BDBG_LEAVE(B_DTCP_Content_DecryptData);
    return retValue;
}
