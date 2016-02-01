/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * DTCP Ake core functions, AKE base messaging interface.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake_msg.c
 *  \brief implement base DTCP Ake core functionalities's message producing and consuming
 *
 *  This file only implement AKE base messaging interface, it doesn't include protocol
 *  specific (i.e. DTCP over IP or DTCP over USB, etc) messages.
 *  When creating an AKE message, these functions allocate buffer and assigned it to AKE
 *  session's command buffer, caller of these function must free the buffer after command
 *  has been sent out.
 *  When consuming and AKE message, these functions will free the message buffer inside
 *  each Consume_XXX function.
 */
#include <arpa/inet.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "b_os_lib.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_ake_msg.h"
#include "b_dtcp_status_codes.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_srm.h"

BDBG_MODULE(b_dtcp_ip);

/*! \brief This function is used to create a message with no data field (header only)
 *
 *  This function can be used by creating AKE command with no data field, or creating AKE command response.
 *  \param[in] Session Ake session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops message header operation pointer
 *  \param[in] Ctype_Response command type of response.
 *  \param[in] Cmd AKE command.
 *  \param[in] CmdDep AKE command dependent field value.
 *  \param[in] Status status field value(only useful if Ctype_Response is a response code.)
 */
BERR_Code B_DTCP_CreateMsgNoData(B_AkeCoreSessionData_T * Session, int headroom, B_AkeHeader_ops_T * hops,
        int Ctype_Response,
        int Cmd,
        int CmdDep,
        int Status)
{
    BDBG_ASSERT(Session);
    BDBG_ASSERT(hops);
    BDBG_ENTER(B_DTCP_CreateMsgNoData);

    if((Session->CmdBuffer = BKNI_Malloc(headroom)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, headroom);
    hops->SetCtypeResponse(Session->CmdBuffer,Ctype_Response);
    hops->SetCommand(Session->CmdBuffer, Cmd);
    hops->SetAkeType(Session->CmdBuffer, (int)Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, (int)Session->ExchKeyType);
    hops->SetCmdDependent(Session->CmdBuffer, CmdDep);
    /*
     * Should check ctype_response, if it's response, set
     * the status to "status" parameter, but we set it
     * always, rally on the caller to set the "status" to 0
     * if it's not a response.
     */
    hops->SetStatus(Session->CmdBuffer, Status);
    BDBG_LEAVE(B_DTCP_CreateMsgNoData);

    return BERR_SUCCESS;
}
/*! \brief Consume message with no associated data field.
 *
 *  This function can be used by AKE command consumer and AKE response consumer.
 *  Session->ResponsBuffer will be freed after this function call.
 *  \param[in] Session AKE session data.
 *  \param[in] buffer The buffer contains the data to consume.
 *  \param[in] hops AKE header operation pointer.
 *  \param[out] Ctype_Response pointer to returned ctype code or response code.
 *  \param[out] Cmd pointer to returned AKE command.
 *  \param[out] CmdDep pointer to returned AKE command dependent field value.
 *  \param[out] Status pointer to returned status field value.
 */
BERR_Code B_DTCP_ConsumeMsgNoData(B_AkeCoreSessionData_T * Session, unsigned char * buffer,
        B_AkeHeader_ops_T * hops,
        int * Ctype_Response,
        int * Cmd,
        int * CmdDep,
        int * Status)
{
    int AkeType, ExchKeyType;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(hops);
    BDBG_ENTER(B_DTCP_ConsumeMsgNoData);

    hops->GetCtypeResponse(buffer, Ctype_Response);
    hops->GetCommand(buffer, Cmd);
    hops->GetCmdDependent(buffer, CmdDep);
    hops->GetStatus(buffer, Status);

    hops->GetAkeType(buffer, &AkeType);
    if(AkeType != (int)Session->AkeType)
        Session->AkeType = AkeType;

    hops->GetExchKeyType(buffer, &ExchKeyType);
    if(ExchKeyType != (int)Session->ExchKeyType)
        Session->ExchKeyType = ExchKeyType;

    BKNI_Free(buffer);
    BDBG_LEAVE(B_DTCP_ConsumeMsgNoData);

    return BERR_SUCCESS;
}
/*! \brief create AKE status command
 *  \param[in] DeviceParams device parameter pointer,
 *  \param[in] Session AKE session pointer.
 *  \param[in] headroom size of AKE message header.
 *  \param[in] hops AKE message header operation function pointer.
 */
BERR_Code B_DTCP_CreateStatusCmd(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int CmdDep;
    BDBG_ASSERT(Session);
    BDBG_ASSERT(hops);

    (void)DeviceParams;
    if (IS_SINK(Session->DeviceMode))
        CmdDep = 1;
    else
        CmdDep = 0;
    return B_DTCP_CreateMsgNoData(Session, headroom, hops, B_CType_eControl, B_AkeCmd_eStatus, CmdDep, B_AkeStatus_eNoInfo);
}
/*! \brief create challenge command,
 *  \param[in] DeviceParams device parameters.
 *  \param[in] Session Ake session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops Message header operation pointer
 */
BERR_Code B_DTCP_CreateChallenge(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int size, nonce_size = 0;
    unsigned char * pAkeData;

    BDBG_ASSERT(Session);
    BDBG_ASSERT(hops);
    BDBG_ENTER(B_DTCP_CreateChallenge);

    if( Session->CmdBuffer != NULL)
        BDBG_WRN(("%s: Cmd buffer is not NULL\n", __FUNCTION__));

    if(B_AkeType_eFull == Session->AkeType)
    {
        size = DTCP_FULL_AUTH_CHALLENGE_SIZE;
        nonce_size = DTCP_FULL_AUTH_NONCE_SIZE;
    }else if(B_AkeType_eRestricted == Session->AkeType)
    {
        size = DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE;
        nonce_size = DTCP_RESTRICTED_AUTH_NONCE_SIZE;
    }else if(B_AkeType_eEnRestricted == Session->AkeType)
    {
        /* TODO: nonce size ? */
        size = DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE;
    }else if(B_AkeType_eExtendedFull == Session->AkeType)
    {
        /* TODO: nonce size */
        size = DTCP_EXTENDED_FULL_AUTH_CHALLENGE_SIZE;
    }else {
        return BERR_INVALID_PARAMETER;
    }
    size += headroom ;
    if((Session->CmdBuffer = (unsigned char *)BKNI_Malloc(size) ) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else{
        Session->CmdBufferSize = size;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, size);
    hops->SetCtypeResponse(Session->CmdBuffer, (int)B_CType_eControl);
    hops->SetCommand(Session->CmdBuffer, (int)B_AkeCmd_eChallenge);
    hops->SetAkeType(Session->CmdBuffer, (int)Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, (int)Session->ExchKeyType);

    if(IS_SINK(Session->DeviceMode))
        hops->SetCmdDependent(Session->CmdBuffer, 1);
    else
        hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    pAkeData = Session->CmdBuffer + headroom;
    /* Get a random number,TODO: call RNG_max here */
    if(B_RNG(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, Session->Nonce, nonce_size) != BCRYPT_STATUS_eOK)
    {
        BKNI_Free(Session->CmdBuffer);
        BDBG_MSG(("%s: BCryptRNGSw failed\n", __FUNCTION__));
        return BERR_CRYPT_FAILED;
    }
    BKNI_Memcpy(pAkeData, Session->Nonce, nonce_size);
    Session->NonceSize = nonce_size;
    if(B_AkeType_eFull == Session->AkeType)
    {
        B_BaselineFullCert_T * cert = (B_BaselineFullCert_T *)DeviceParams->Cert;
        if(cert->Format != B_CertFormat_eBaselineFull) {
            BKNI_Free(Session->CmdBuffer);
            BDBG_MSG(("%s: Device certificate is invalid\n", __FUNCTION__));
            return BERR_NOT_SUPPORTED;
        }
        BKNI_Memcpy(pAkeData + nonce_size, DeviceParams->Cert, sizeof(B_BaselineFullCert_T));
    }else if (B_AkeType_eExtendedFull == Session->AkeType)
    {
        BKNI_Memcpy(pAkeData + nonce_size, DeviceParams->Cert, sizeof(B_ExtFullCert_T) );
    }else if(B_AkeType_eRestricted == Session->AkeType)
    {
        /* Restricted Authentication */
    }else
    {
        /* Enhanced restricted authentication*/
    }
    BDBG_LEAVE(B_DTCP_CreateChallenge);
    return BERR_SUCCESS;
}
/*! \brief consume/decode challenge command,
 *  \param[in] DeviceParams device parameters.
 *  \param[in] Session Ake session data.
 *  \param[in] buffer The buffer contains the data to consume.
 *  \param[in] len the length of the buffer.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_ConsumeChallenge(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    int sig_valid = 0;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeChallenge);
    (void)len;

    /* TODO Determine AKE type before copying NONCE value */
    BKNI_Memcpy(Session->OtherNonce, buffer + headroom, DTCP_FULL_AUTH_NONCE_SIZE);
    Session->OtherNonceSize = DTCP_FULL_AUTH_NONCE_SIZE;

    if(Session->AkeType == B_AkeType_eFull)
    {
        B_BaselineFullCert_T * Cert = (B_BaselineFullCert_T *)(buffer + headroom + DTCP_FULL_AUTH_NONCE_SIZE);
        BDBG_MSG(("\n Received other device's certificate:\n"));
        BDBG_MSG(("\n Type: %d\n Format:%d\n DevGen: %d\n AL:%d \n AP: %d\n",
                    Cert->CertType & 0x0F, Cert->Format & 0x0F, Cert->DevGen & 0x0F,
                    Cert->AL & 0x01, Cert->AP & 0x01));
        BDBG_MSG(("\n DevId: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                    Cert->DevID[0], Cert->DevID[1], Cert->DevID[2], Cert->DevID[3], Cert->DevID[4]));
#ifdef DTCP_DEMO_MODE
        BDBG_MSG(("\n Public Key\n"));
        BDBG_BUFF(Cert->PublicKey, 40);
        BDBG_MSG(("\n Signature \n"));
        BDBG_BUFF(Cert->Signature, 40);
#endif
        if(B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                    &sig_valid, (unsigned char *)Cert->Signature,
                    (unsigned char *)Cert,
                    DTCP_BASELINE_FULL_CERT_SIZE - DTCP_SIGNATURE_SIZE,
                    DeviceParams->dtlaPublicKey)
            != BERR_SUCCESS)
        {
            BKNI_Free(buffer);
            BDBG_ERR(("%s: Failed to verify certificate\n", __FUNCTION__));
            return BERR_CRYPT_FAILED;
        }
        if(!sig_valid)
        {
            BKNI_Free(buffer);
            BDBG_ERR(("%s: Invalid certificate\n", __FUNCTION__));
            return BERR_INVALID_CERTIFICATE;
        }
        Session->OtherAL = Cert->AL;
        Session->OtherAP = Cert->AP;
        Session->OtherSrmG = 0xFFFF & Cert->DevGen; /* ???? */
        BKNI_Memcpy(Session->OtherDeviceId, Cert->DevID, DTCP_DEVICE_ID_SIZE);
        BKNI_Memcpy(Session->OtherPublicKey, Cert->PublicKey, DTCP_PUBLIC_KEY_SIZE);
    }else if (Session->AkeType == B_AkeType_eExtendedFull ||
            Session->AkeType == B_AkeType_eRestricted ||
            Session->AkeType == B_AkeType_eEnRestricted)
    {
        BDBG_ERR(("Authentication %d is not supported\n", Session->AkeType));
        BKNI_Free(buffer);
        return BERR_NOT_SUPPORTED;
    }

    BKNI_Free(buffer);
    BDBG_LEAVE(B_DTCP_ConsumeChallenge);

    return BERR_SUCCESS;

}
/*! \brief create challenge command response.
 *  \param[in] DeviceParams device parameters.
 *  \param[in] Session AkeCore session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops Ake header operation pointer.
 */
BERR_Code B_DTCP_CreateResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int size = 0, s_size = 0, offset = 0;   /* s_size is the buffer size for signature computation */
    unsigned char * buf;
    unsigned char * pAkeData;
    unsigned char  SrmC;
    unsigned short SrmV;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_CreateResponse);

    if(Session->AkeType == B_AkeType_eFull || Session->AkeType == B_AkeType_eExtendedFull) {
        size = DTCP_FULL_AUTH_RESPONSE_SIZE ;
        s_size = DTCP_FULL_AUTH_RESPONSE_SIZE + DTCP_FULL_AUTH_NONCE_SIZE;
    }else if(Session->AkeType == B_AkeType_eRestricted || Session->AkeType == B_AkeType_eEnRestricted) {
        return BERR_NOT_SUPPORTED;
    }
    size += headroom ;
    if((Session->CmdBuffer = BKNI_Malloc(size)) == NULL) {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = size ;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, size);
    hops->SetCtypeResponse(Session->CmdBuffer, (int)B_CType_eControl);
    hops->SetCommand(Session->CmdBuffer, (int)B_AkeCmd_eResponse);
    hops->SetAkeType(Session->CmdBuffer, (int)Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, (int)Session->ExchKeyType);

    if(IS_SINK(Session->DeviceMode))
        hops->SetCmdDependent(Session->CmdBuffer, 1);
    else
        hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    pAkeData = Session->CmdBuffer + headroom ;

    /* Using temp buffer for signature computation */
    if((buf = BKNI_Malloc(s_size)) == NULL)
    {
        BKNI_Free(Session->CmdBuffer);
        return B_ERROR_OUT_OF_MEMORY;
    }
    /*Prepare for EC-DSA signature computation */
    BKNI_Memcpy(buf + offset, Session->OtherNonce, DTCP_FULL_AUTH_NONCE_SIZE);
    offset += DTCP_FULL_AUTH_NONCE_SIZE;
    BKNI_Memcpy(buf + offset, Session->FirstPhaseValue, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    offset += DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;
    /* Our SRM version number, the B_DTCP_GetSrmC() function returns SRM version number
     * in host byte order, convert to network byte order here.
     */
    SrmV = htons(B_DTCP_GetSrmV((unsigned char *)DeviceParams->Srm));
    BKNI_Memcpy(buf + offset, (unsigned char *)&SrmV, DTCP_SRM_VERSION_NUMBER_SIZE);
    offset += DTCP_SRM_VERSION_NUMBER_SIZE;
    /* Our SRM current reneration */
    SrmC = B_DTCP_GetSrmC((unsigned char *)DeviceParams->Srm);
    BKNI_Memcpy(buf + offset, &SrmC, DTCP_SRM_GENERATION_SIZE);
    offset += DTCP_SRM_GENERATION_SIZE;

    if(B_DTCP_SignData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pAkeData + DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE,
                buf, offset) != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to sign the data\n", __FUNCTION__));
        BKNI_Free(buf);
        BKNI_Free(Session->CmdBuffer);
        return BERR_CRYPT_FAILED;
    }
    /* Copy message (Except signature) into command buffer */
    BKNI_Memcpy(pAkeData, buf + DTCP_FULL_AUTH_NONCE_SIZE, DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE);
    /* Free temp buffer */
    BKNI_Free(buf);

    BDBG_LEAVE(B_DTCP_CreateResponse);
    return BERR_SUCCESS;
}
/*! \brief Consume Response data.
 *  \param[in] DeviceParams device parameters.
 *  \param[in,out] Session AkeCore session data.
 *  \param[in] buffer the buffer contains the data to consume.
 *  \param[in[ len the length of the buffer.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_ConsumeResponse(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    unsigned char * pBuf;
    int BufLen, offset;
    unsigned char * pSig;
    int sig_valid;
    unsigned short srmV;
    unsigned char srmC;
    /* char msg[1024];   debug buffer*/

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeResponse);

    (void)len;
    BufLen = DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE + DTCP_FULL_AUTH_NONCE_SIZE;
    if((pBuf = BKNI_Malloc(BufLen)) == NULL)
    {
        BDBG_ERR(("%s: Failed to alloc memory\n", __FUNCTION__));
        return B_ERROR_OUT_OF_MEMORY;
    }
    BKNI_Memset(pBuf, 0, BufLen);

    /* copy data to temp buffer to verify signature */
    BKNI_Memcpy(pBuf, Session->Nonce, DTCP_FULL_AUTH_NONCE_SIZE);
    BKNI_Memcpy(pBuf + DTCP_FULL_AUTH_NONCE_SIZE, buffer + headroom,
            DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE);

    pSig = buffer + headroom + (DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE);

    if(B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, &sig_valid, pSig, pBuf, BufLen,
                Session->OtherPublicKey)
            != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Failed to verify signature\n", __FUNCTION__));
        BKNI_Free(pBuf);
        BKNI_Free(buffer);
        return BERR_CRYPT_FAILED;
    }
    if (!sig_valid)
    {
        BKNI_Free(pBuf);
        BKNI_Free(buffer);
        return BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
    }
    offset = headroom;
    BKNI_Memcpy(Session->OtherFirstPhaseValue, buffer + headroom, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    offset += DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;

    /* Get other's SRM version number, convert from network byte order to host byte order.*/
    srmV = ntohs(*(unsigned short *)(buffer + offset));
    /* BKNI_Memcpy(&SrmV, buffer + offset, 2); */
    Session->OtherSrmV = srmV;
    offset += DTCP_SRM_VERSION_NUMBER_SIZE;
    srmC = *(buffer + offset);
    Session->OtherSrmC = (unsigned short)(0x0F & srmC);

    BKNI_Free(pBuf);
    BKNI_Free(buffer);

    /* If we are source and our device doesn't support pcp_ur, set the flag in session*/
    if (IS_SOURCE(DeviceParams->Mode) &&
            DeviceParams->pcp_ur_capability == 0) {
        Session->pcp_ur_cap = 0;
    } else if ((IS_SINK(DeviceParams->Mode) && (Session->pcp_ur_cap & PCP_UR_FLAG)) ||
            (IS_SOURCE(DeviceParams->Mode) && (Session->pcp_ur_cap & PCP_UR_SINK)))
    {
        /* If we are a sink, and session->pcp_ur_cap bit 0 is set, or
         * we are a source device and session->pcp_ur_cap bit 31 is set, verify the stored
         * capability signature obtained from CAPABILITY_EXCHANGE message
         */
        unsigned int pcp_ur_cap = htonl(Session->pcp_ur_cap);
        sig_valid = 0;
        if (B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, &sig_valid,
                Session->CapabilityExchSignature, (unsigned char *)&pcp_ur_cap,
                4, Session->OtherPublicKey) != BERR_SUCCESS) {
            BDBG_ERR(("%s: Failed to verify capability msg signature\n", __FUNCTION__));
            return BERR_CRYPT_FAILED;
        } else if (!sig_valid) {
            Session->pcp_ur_cap = 0;
            BDBG_WRN(("Incorrect Capability signature, discarding PCP_UR flag\n"));
        } else {
            Session->pcp_ur_cap = 1;
        }
    }
    BDBG_LEAVE(B_DTCP_ConsumeResponse);

    return BERR_SUCCESS;
}
/*!\brief Create exchange key command, (source device only)
 * \param[in] DeviceParams sending device's parameter.
 * \param[in] Session AkE core session data.
 * \param[in] headroom bytes to reserve in message header.
 * \param[in] hops AKE header operation pointer,
 */
BERR_Code B_DTCP_CreateExchangeKey(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int offset = 0;
    int i;
    unsigned char *pAkeData;
    unsigned char KeyData[DTCP_EXCHANGE_KEY_SIZE];
    B_AkeCoreData_T * AkeCoreData;
    B_ExchangeKeyCipher_T Cipher;

    BDBG_ENTER(B_DTCP_CreateExchangeKey);
    BDBG_ASSERT(Session);
    (void)DeviceParams;

    AkeCoreData = Session->pAkeCoreData;
    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_EXCHANGE_KEY_CMD_DATA_SIZE)) == NULL)
        return B_ERROR_OUT_OF_MEMORY;
    else
        Session->CmdBufferSize = headroom + DTCP_EXCHANGE_KEY_CMD_DATA_SIZE;

    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, headroom + DTCP_EXCHANGE_KEY_CMD_DATA_SIZE);
    hops->SetCtypeResponse(Session->CmdBuffer, B_CType_eControl);
    hops->SetCommand(Session->CmdBuffer, B_AkeCmd_eExchangeKey);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    pAkeData = Session->CmdBuffer + headroom ;
    BKNI_Memcpy(pAkeData, &AkeCoreData->ExchKeyData.Label, DTCP_EXCHANGE_KEY_LABEL_SIZE);
    offset += DTCP_EXCHANGE_KEY_LABEL_SIZE;
    /* TODO: get exchange key cipher algorithm from some where*/
    if(Session->ExchKeyType == B_ExchKey_eM6CopyNever ||
            Session->ExchKeyType == B_ExchKey_eM6CopyOne ||
            Session->ExchKeyType == B_ExchKey_eM6NoMoreCopy)
    {
        Cipher = B_ExchKeyCipher_eM6;
    }else {
        Cipher = B_ExchKeyCipher_eAes;
    }
    *(pAkeData + offset) = ((unsigned char)Cipher << 4);
    offset += 1;
    /* Scramble the key data */
    for(i = 0; i < DTCP_EXCHANGE_KEY_SIZE; i++)
        KeyData[i] = AkeCoreData->ExchKeyData.ExchangeKeys[Cipher][i] ^ Session->AuthKey[i] ;

    BKNI_Memcpy(pAkeData + offset, KeyData, DTCP_EXCHANGE_KEY_SIZE);
#ifdef DTCP_DEMO_MODE
    BDBG_MSG(("Creating exchange key: type=%d Ks:\n", Session->ExchKeyType));
    BDBG_BUFF(KeyData, DTCP_EXCHANGE_KEY_SIZE);
    BDBG_MSG(("K:\n"));
    BDBG_BUFF(AkeCoreData->ExchKeyData.ExchangeKeys[Cipher], DTCP_EXCHANGE_KEY_SIZE);
#endif
    BDBG_LEAVE(B_DTCP_CreateExchangeKey);

    return BERR_SUCCESS;

}
/*! \brief Consume exchange key command, (Sink device only)
 * \param[in] DeviceParams device's parameter.
 * \param[in,out] Session AkE core session data.
 * \param[in] buffer the buffer contains the data to consume.
 * \param[in] len the length of the buffer.
 * \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_ConsumeExchangeKey(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    int i;
    B_ExchangeKeyCipher_T ExchKeyCipher;
    unsigned char KeyData[DTCP_EXCHANGE_KEY_SIZE];

    BDBG_ASSERT(Session);
    (void)DeviceParams; (void)len;

    BDBG_ENTER(B_DTCP_ConsumeExchangeKey);

    Session->ExchKeyData.Label = *(buffer + headroom);

    /* SourceExchKeyLabel could change after sink issueing CONT_KEY_REQ command */
    Session->SourceExchKeyLabel = Session->ExchKeyData.Label;

    ExchKeyCipher = (*(buffer + headroom + 1) >> 4) & 0x0F;

    if(ExchKeyCipher != B_ExchKeyCipher_eM6 &&
            ExchKeyCipher != B_ExchKeyCipher_eAes)
    {
        BDBG_ERR(("%s: Exchange key type %d is not supported\n", __FUNCTION__, ExchKeyCipher));
        return BERR_NOT_SUPPORTED;
    }
    BKNI_Memcpy(KeyData,
            buffer + headroom + 2,
            DTCP_EXCHANGE_KEY_SIZE);
#ifdef DTCP_DEMO_MODE
    BDBG_ERR(("Received exchage key: ks:\n"));
    BDBG_BUFF(KeyData, DTCP_EXCHANGE_KEY_SIZE);
#endif
    /* De-scramble key data. */
    for(i = 0; i < DTCP_EXCHANGE_KEY_SIZE; i++)
        KeyData[i] = KeyData[i] ^ Session->AuthKey[i];

    BKNI_Memcpy((Session->ExchKeyData).ExchangeKeys[ExchKeyCipher], KeyData, DTCP_EXCHANGE_KEY_SIZE);
    Session->ExchKeyData.Valid |= (1 << ExchKeyCipher);
    if(Session->ExchKeyData.Initialized == false)
        Session->ExchKeyData.Initialized = true;

    BKNI_Free(buffer);
    BDBG_LEAVE(B_DTCP_ConsumeExchangeKey);

    return BERR_SUCCESS;
}
/*! \brief Create Srm message, can be called by sink or source device.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AkE session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_CreateSrm(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int BufLen, CrlLen;
    unsigned char *pAkeData;

    BDBG_ENTER(B_DTCP_CreateSrm);
    BDBG_ASSERT(Session);
    BDBG_ASSERT(DeviceParams);

    /* The following need to be modified, if the SRM generation is more then 2 */
    if(Session->OtherSrmReplaceRequired == true)
        CrlLen = B_DTCP_GetSrmLength((unsigned char *)DeviceParams->Srm);
    else
        CrlLen = DTCP_SRM_FIRST_GEN_MAX_SIZE;
    BufLen = headroom + 4 + CrlLen;

    if((Session->CmdBuffer = BKNI_Malloc(BufLen)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = BufLen;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, BufLen);
    hops->SetCommand(Session->CmdBuffer, B_AkeCmd_eSRM);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, 0);
    if(IS_SINK(Session->DeviceMode))
        hops->SetCmdDependent(Session->CmdBuffer, 1);
    else
        hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    pAkeData = Session->CmdBuffer + headroom ;
    BKNI_Memcpy(pAkeData, &DeviceParams->Srm->SrmHeader, sizeof(B_Srm_Header_T));
    BKNI_Memcpy((unsigned char *)pAkeData + sizeof(B_Srm_Header_T), (void*)DeviceParams->Srm->Crl, CrlLen-2);

    BDBG_LEAVE(B_DTCP_CreateSrm);
    return BERR_SUCCESS;
}
/*! \brief Consume Srm message, can be called by sink or source device.
 *  This function verfies the signature of the received SRM message, if the signature is valid,
 *  the input buffer will not be freed,  caller is responsible to free the input buffer after
 *  updating the SRM to non-volatile memory!
 *
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AkE core session data.
 *  \param[in] buffer the buffer contains the data to consume.
 *  \param[in] len length of the buffer.
 *  \param[in] headroom bytes to skip when processing buffer.
 *
 *  NOTE: This function will not free the input buffer
 */
BERR_Code B_DTCP_ConsumeSrm(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    int crlSize, sig_valid;
    unsigned char * pSignature, * pSrm;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeSrm);
    (void)len;

    pSrm = buffer + headroom;
    crlSize = B_DTCP_GetSrmLength(pSrm);
    pSignature = pSrm + 4 + crlSize - DTCP_SIGNATURE_SIZE;
    /*
     * Verify SRM signature
     */

    if(B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                    &sig_valid, pSignature, pSrm,
                    4 + crlSize - DTCP_SIGNATURE_SIZE,
                    DeviceParams->dtlaPublicKey) != BERR_SUCCESS)
    {
        BKNI_Free(buffer);
        BDBG_ERR(("%s: Failed to verify SRM message\n", __FUNCTION__));
        return BERR_CRYPT_FAILED;
    }
    if(!sig_valid)
    {
        BKNI_Free(buffer);
        BDBG_WRN(("Invalid SRM \n"));
        return BERR_INVALID_SRM;
    }

    /* strip out the AKE message header, only keep the SRM message */
    if((pSrm = BKNI_Malloc(crlSize + 4)) == NULL)
    {
        BDBG_ERR(("Failed to allocate memory for SRM\n"));
        return B_ERROR_OUT_OF_MEMORY;
    }
    BKNI_Memcpy(pSrm, buffer + headroom, crlSize + 4 );
    BKNI_Memcpy(buffer, pSrm, crlSize + 4);
    BDBG_MSG(("SRM UPDATE :: Updating with %d bytes", crlSize+4));
    BKNI_Free(pSrm);
    BDBG_LEAVE(B_DTCP_ConsumeSrm);

    return BERR_SUCCESS;
}
/*!\brief Create Ake Cancel message
 * \param[in] DeviceParams sending device's parameter.
 * \param[in] Session AkE core session data.
 * \param[in] headroom bytes to reserve for message header.
 * \param[in[ hops AKE header operation pointer.
 */
BERR_Code B_DTCP_CreateAkeCancel(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    int CmdDep;
    (void)DeviceParams;
    if (IS_SINK(Session->DeviceMode))
        CmdDep = 1;
    else
        CmdDep = 0;
    return B_DTCP_CreateMsgNoData(Session, headroom, hops,
                B_CType_eControl,
                B_AkeCmd_eAkeCancel,
                CmdDep,
                B_AkeStatus_eNoInfo);
}
/*! \brief Create Content_Key_Req message, sink device only.
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in] Session AkE session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_CreateContentKeyReq(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    BERR_Code retValue;
    (void)DeviceParams;
    retValue = B_DTCP_CreateMsgNoData(Session, headroom, hops,
                B_CType_eControl,
                B_AkeCmd_eContentKeyReq,
                0,
                B_AkeStatus_eNoInfo);
    /*Set AKE_procedure and Exchange_key to 0 instead of session specific value */
    hops->SetAkeType(Session->CmdBuffer, 0);
    hops->SetExchKeyType(Session->CmdBuffer, 0);
    return retValue;

}
/*! Create a content key request response message.
 */
BERR_Code B_DTCP_CreateContentKeyReqResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops, int Response, int Status)
{

    unsigned char *pAkeData;
    int offset;

    (void)DeviceParams;
    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_CreateContentKeyReqResponse);

    if (Session->CmdBuffer != NULL)
    {
        BKNI_Free(Session->CmdBuffer);
        Session->CmdBuffer = NULL;
    }

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_CONTENT_KEY_REQUEST_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_CONTENT_KEY_REQUEST_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, Response);
    hops->SetCommand(Session->CmdBuffer, B_AkeCmd_eContentKeyReq);
    /* AkE_procedure, Exchange_key and Subfunction dependent should be 0s */
    hops->SetAkeType(Session->CmdBuffer, 0);
    hops->SetExchKeyType(Session->CmdBuffer, 0);
    hops->SetCmdDependent(Session->CmdBuffer, 0);
    hops->SetStatus(Session->CmdBuffer, Status);
    pAkeData = Session->CmdBuffer + headroom ;
    offset = 0;
    BKNI_Memset(pAkeData, 0, 4 + DTCP_CONTENT_KEY_NONCE_SIZE);
    BKNI_Memcpy(pAkeData, &Session->pAkeCoreData->ExchKeyData.Label, DTCP_EXCHANGE_KEY_LABEL_SIZE);
    offset += DTCP_EXCHANGE_KEY_LABEL_SIZE;

    if(Session->ExchKeyType == B_ExchKey_eM6CopyNever ||
            Session->ExchKeyType == B_ExchKey_eM6CopyOne ||
            Session->ExchKeyType == B_ExchKey_eM6NoMoreCopy)
    {
        *(pAkeData + offset) = ((unsigned char)B_ExchKeyCipher_eM6 << 4);
    }else {
        *(pAkeData + offset) = ((unsigned char)B_ExchKeyCipher_eAes << 4);
    }
    offset += 3;
    BKNI_Memcpy(pAkeData + offset, &Session->pAkeCoreData->RealTimeNonce, DTCP_CONTENT_KEY_NONCE_SIZE);

    BDBG_LEAVE(B_DTCP_CreateContentKeyReqResponse);
    return BERR_SUCCESS;
}
/*! \brief Consume content key request response, called by sink only
 *  \param[in] Session AKE core session data.
 *  \param[in] buffer message buffer.
 *  \param[out] Cmd  returned command pointer.
 *  \param[out] Response returned response code poiter.
 *  \param[out] Status returned Status code pointer.
 *  \param[in] headroom the size of the message header.
 *  \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_ConsumeContKeyReqResponse(B_AkeCoreSessionData_T * Session,
        unsigned char * buffer, int * Cmd, int * Response, int * Status, int headroom, B_AkeHeader_ops_T * hops)
{
    BERR_Code retValue;
    int CmdDep;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeContKeyReqResponse);
    Session->SourceExchKeyLabel = *(buffer + headroom);

    /* Ignore Cipher_algorithm field for now */
    BKNI_Memcpy(Session->SourceRTNonce, Session->CmdBuffer + headroom + 3, DTCP_CONTENT_KEY_NONCE_SIZE);
    /* cmd buffer will be freed by following function.*/
    retValue = B_DTCP_ConsumeMsgNoData(Session, buffer, hops, Response,
            Cmd, &CmdDep, Status);
    BDBG_LEAVE(B_DTCP_ConsumeContKeyReqResponse);

    return retValue;
}
/*! \brief Create Response2 message, sink device only
 *  \param[in] DeviceParams sending device's parameter.
 *  \param[in,out] Session AkE session data.
 *  \param[in] headroom bytes to reserve for message header.
 *  \param[in] hops Ake header operation pointer.
 */
BERR_Code B_DTCP_CreateResponse2(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    unsigned char * buf;
    unsigned char * pAkeData;
    int offset, size, s_size ;
    unsigned char srmC;
    unsigned short srmV;
    unsigned int capability;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_CreateResponse2);

    if( Session->CmdBuffer != NULL)
        BDBG_WRN(("%s: Cmd buffer is not NULL\n", __FUNCTION__));

    if(Session->AkeType != B_AkeType_eFull && Session->AkeType != B_AkeType_eExtendedFull)
        return BERR_NOT_SUPPORTED;
    size = DTCP_FULL_AUTH_RESPONSE2_SIZE + headroom;
    s_size = DTCP_FULL_AUTH_RESPONSE2_SIZE + DTCP_FULL_AUTH_NONCE_SIZE;
    if((Session->CmdBuffer = BKNI_Malloc(size)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_FULL_AUTH_RESPONSE2_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, headroom + DTCP_FULL_AUTH_RESPONSE2_SIZE);
    hops->SetCommand(Session->CmdBuffer, B_AkeCmd_eResponse2);
    hops->SetAkeType(Session->CmdBuffer, Session->AkeType);
    hops->SetExchKeyType(Session->CmdBuffer, Session->ExchKeyType);
    /*RESPONSE2 is only sent from sink->source, not other way around!*/
    hops->SetCmdDependent(Session->CmdBuffer, 1);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);

    pAkeData = Session->CmdBuffer + headroom ;

    /* Using temp buffer for signature computation */
    if((buf = BKNI_Malloc(s_size)) == NULL)
    {
        BKNI_Free(Session->CmdBuffer);
        return B_ERROR_OUT_OF_MEMORY;
    }
    /* Copy message into temp buffer */
    offset = 0;
    BKNI_Memcpy(buf + offset, Session->OtherNonce, DTCP_FULL_AUTH_NONCE_SIZE);
    offset += DTCP_FULL_AUTH_NONCE_SIZE;
    BKNI_Memcpy(buf + offset, Session->FirstPhaseValue, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    offset += DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;
    /* Our SRM version number, the B_DTCP_GetSrmC() function returns SRM version number
     * in host byte order, convert to network byte order here.
     */
    srmV = htons(B_DTCP_GetSrmV((unsigned char *)DeviceParams->Srm));
    BKNI_Memcpy(buf + offset, (unsigned char *)&srmV, DTCP_SRM_VERSION_NUMBER_SIZE);
    offset += DTCP_SRM_VERSION_NUMBER_SIZE + 1;
    srmC = B_DTCP_GetSrmC((unsigned char *)DeviceParams->Srm);
    BKNI_Memcpy(buf + offset, (unsigned char *)&srmC, DTCP_SRM_GENERATION_SIZE);
    offset += DTCP_SRM_GENERATION_SIZE;

    /* Sink device capability bit , use device parameter value */
    capability = ntohl(*(unsigned int *)&DeviceParams->capability);
    BKNI_Memcpy(buf + offset, &capability, DTCP_DEVICE_CAPABILITY_SIZE);
    offset += DTCP_DEVICE_CAPABILITY_SIZE;
    BKNI_Memcpy(buf + offset, DeviceParams->IDu, DTCP_DEVICE_ID_SIZE);
    offset += DTCP_DEVICE_ID_SIZE;

    /* Sign the data */
    if(B_DTCP_SignData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pAkeData + DTCP_FULL_AUTH_RESPONSE2_SIZE - DTCP_SIGNATURE_SIZE,
                buf, offset) != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to sign"
                  " the data\n", __FUNCTION__));
        BKNI_Free(buf);
        BKNI_Free(Session->CmdBuffer);
        return BERR_CRYPT_FAILED;
    }
    /* Copy message into command buffer */
    BKNI_Memcpy(pAkeData, buf + DTCP_FULL_AUTH_NONCE_SIZE, DTCP_FULL_AUTH_RESPONSE_SIZE - DTCP_SIGNATURE_SIZE);
    /* Free temp buffer */
    BKNI_Free(buf);

    BDBG_LEAVE(B_DTCP_CreateResponse2);
    return BERR_SUCCESS;
}
/*! \brief consume Response2 message, source device only
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AkE core session data.
 *  \param[in] buffer the buffer contains the data to consume.
 *  \param[in] len the length of the buffer.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_ConsumeResponse2(B_DeviceParams_T * DeviceParams,
        B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    unsigned char * pBuf, *pSig;
    int BufLen, offset, sig_valid;
    unsigned short srmV;
    unsigned char srmC;

    (void)len; (void)DeviceParams;
    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeResponse2);

    BufLen = DTCP_FULL_AUTH_RESPONSE2_SIZE - DTCP_SIGNATURE_SIZE + DTCP_FULL_AUTH_NONCE_SIZE;
    if((pBuf = BKNI_Malloc(BufLen)) == NULL)
    {
        BDBG_ERR(("%s: Failed to alloc memory\n", __FUNCTION__));
        return B_ERROR_OUT_OF_MEMORY;
    }
    BKNI_Memset(pBuf, 0, BufLen);
    /* copy data to temp buffer to verify signature */
    BKNI_Memcpy(pBuf, Session->Nonce, DTCP_FULL_AUTH_NONCE_SIZE);
    BKNI_Memcpy(pBuf + DTCP_FULL_AUTH_NONCE_SIZE, buffer + headroom,
            DTCP_FULL_AUTH_RESPONSE2_SIZE - DTCP_SIGNATURE_SIZE);

    pSig = buffer + headroom + (DTCP_FULL_AUTH_RESPONSE2_SIZE - DTCP_SIGNATURE_SIZE);

    if (B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, &sig_valid, pSig, pBuf, BufLen,
                Session->OtherPublicKey)
            != BERR_SUCCESS) {
        BDBG_ERR(("%s: Failed to verify signature\n", __FUNCTION__));
        BKNI_Free(pBuf);
        BKNI_Free(buffer);
        return BERR_CRYPT_FAILED;
    }
    if (!sig_valid) {
        BKNI_Free(pBuf);
        BKNI_Free(buffer);
        BDBG_ERR(("signature invalid\n"));
        return BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
    }
    offset = headroom;
    BKNI_Memcpy(Session->OtherFirstPhaseValue, buffer + offset, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    offset += DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;
    /* BKNI_Memcpy(&SrmV, buffer + offset, 2);*/
    srmV = ntohs(*(unsigned short*)(buffer + offset));
    Session->OtherSrmV = srmV;
    offset += DTCP_SRM_VERSION_NUMBER_SIZE;
    srmC = *(buffer + offset);
    Session->OtherSrmC = (unsigned short)(0x0F & srmC);
    Session->sink_cap = ntohl(*(unsigned long *)(buffer + offset));
    offset += 4;
    if(Session->sink_cap & DTCP_CAPABILITY_NB_MASK)
    {
        /* Use IDu instead of real device id */
        BKNI_Memcpy(Session->OtherDeviceId, buffer+offset, DTCP_DEVICE_ID_SIZE);
    }
    BKNI_Free(pBuf);
    BKNI_Free(buffer);

    /* If we are source and our device doesn't support pcp_ur, set the flag in session*/
    if (IS_SOURCE(DeviceParams->Mode) &&
            DeviceParams->pcp_ur_capability == 0) {
        Session->pcp_ur_cap = 0;
    } else if ((IS_SINK(DeviceParams->Mode) && Session->pcp_ur_cap & PCP_UR_FLAG) ||
            (IS_SOURCE(DeviceParams->Mode) && Session->pcp_ur_cap & PCP_UR_SINK))
    {
        /* If we are a sink, and session->pcp_ur_cap bit 0 is set, or
         * we are a source device and session->pcp_ur_cap bit 31 is set, verify the stored
         * capability signature obtained from CAPABILITY_EXCHANGE message
        */
        unsigned int pcp_ur_cap = htonl(Session->pcp_ur_cap);
        sig_valid = 0;
        if (B_DTCP_VerifyData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, &sig_valid,
                Session->CapabilityExchSignature, (unsigned char *)&pcp_ur_cap,
                4, Session->OtherPublicKey) != BERR_SUCCESS) {
            BDBG_ERR(("%s: Failed to verify capability msg signature\n", __FUNCTION__));
            return BERR_CRYPT_FAILED;
        } else if (!sig_valid) {
            Session->pcp_ur_cap = 0;
            BDBG_WRN(("Incorrect Capability signature, discarding PCP_UR flag\n"));
        } else {
            Session->pcp_ur_cap = 1;
        }
    }
    BDBG_LEAVE(B_DTCP_ConsumeResponse2);

    return BERR_SUCCESS;
}
/*!\brief Create Capability_Req message, sink device only.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in,out] Session AkE core session data.
 * \param[in] headroom  bytes to reserve for message header.
 */
BERR_Code B_DTCP_CreateCapabilityReq(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{
    (void)DeviceParams;
    /* This is sink->source only, so subfunction dependent bit is 1 */
    return B_DTCP_CreateMsgNoData(Session, headroom, hops,
                B_CType_eControl,
                B_AkeCmd_eCapabilityReq,
                1,
                B_AkeStatus_eNoInfo);
}
/*!\brief Create Capability_Req response message, source device only.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in,out] Session AkE session data.
 * \param[in] headroom bytes to reserve for message header.
 * \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_CreateCapabilityReqResponse(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T * hops)
{

    unsigned int capability;
    BDBG_ENTER(B_DTCP_CreateCapabilityReqResponse);
    BDBG_ASSERT(Session);

    if((Session->CmdBuffer = BKNI_Malloc(headroom + DTCP_DEVICE_CAPABILITY_SIZE)) == NULL)
    {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = headroom + DTCP_DEVICE_CAPABILITY_SIZE;
    }
    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, Session->CmdBufferSize);
    hops->SetCtypeResponse(Session->CmdBuffer, B_CType_eStatus);
    hops->SetCommand(Session->CmdBuffer, B_AkeCmd_eCapabilityReq);
    hops->SetAkeType(Session->CmdBuffer, 0);
    hops->SetExchKeyType(Session->CmdBuffer, 1);
    hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eOK);
    hops->SetCmdDependent(Session->CmdBuffer, 0);
    capability = ntohl(*(unsigned int *)&DeviceParams->capability);
    BKNI_Memcpy(Session->CmdBuffer + headroom, (unsigned char *)&capability, DTCP_DEVICE_CAPABILITY_SIZE);

    BDBG_LEAVE(B_DTCP_CreateCapabilityReqResponse);
    return BERR_SUCCESS;
}
/*!\brief Create Capability Exchange command/response message, if called by sink, it creates
 * Capability exchange command, if called by source, it creates capability exchange response.
 * \param[in] DeviceParams sending device's parameter.
 * \param[in,out] Session AkE session data.
 * \param[in] headroom bytes to reserve for message header.
 * \param[in] hops AKE header operation pointer.
 */
BERR_Code B_DTCP_CreateCapabilityExchangeMsg(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session,
        int headroom, B_AkeHeader_ops_T  *hops)
{
    int size = 0;
    unsigned int capability = 0;
    unsigned char * pAkeData;
    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_CreateCapabilityExchangeCmd);

    size = DTCP_CAPABILITY_REQ_SIZE + DTCP_SIGNATURE_SIZE + headroom ;
    if((Session->CmdBuffer = BKNI_Malloc(size)) == NULL) {
        return B_ERROR_OUT_OF_MEMORY;
    }else {
        Session->CmdBufferSize = size ;
        BKNI_Memset(Session->CmdBuffer, 0, size);
    }
    pAkeData = Session->CmdBuffer + headroom ;

    hops->Init(Session->CmdBuffer, Session->AkeLabel);
    hops->SetLength(Session->CmdBuffer, size);
    hops->SetCommand(Session->CmdBuffer, (int)B_AkeCmd_eCapabilityExchange);
    hops->SetAkeType(Session->CmdBuffer, 0);
    hops->SetExchKeyType(Session->CmdBuffer, 0);
    hops->SetCmdDependent(Session->CmdBuffer, 0);

    if(IS_SINK(Session->DeviceMode)) {
        hops->SetCtypeResponse(Session->CmdBuffer, (int)B_CType_eControl);
        hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eNoInfo);
        capability |= htonl(PCP_UR_SINK);

    } else {
        hops->SetCtypeResponse(Session->CmdBuffer, (int)B_Response_eAccepted);
        hops->SetStatus(Session->CmdBuffer, B_AkeStatus_eOK);
        capability = htonl(DeviceParams->pcp_ur_capability);
    }
    BKNI_Memcpy(pAkeData, &capability, sizeof(unsigned int));

    if(B_DTCP_SignData_BinKey(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pAkeData + DTCP_CAPABILITY_REQ_SIZE,
                pAkeData, DTCP_CAPABILITY_REQ_SIZE
                ) != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to sign the data\n", __FUNCTION__));
        BKNI_Free(Session->CmdBuffer);
        return BERR_CRYPT_FAILED;
    }

    BDBG_LEAVE(B_DTCP_CreateCapabilityExchangeCmd);
    return BERR_SUCCESS;
}
/*! \brief Consume capability exchange message.
 *  \param[in] DeviceParams receiving device's parameter.
 *  \param[in,out] Session AkE core session data.
 *  \param[in] buffer the buffer contains the data to consume.
 *  \param[in] len length of the buffer.
 *  \param[in] headroom bytes to skip when processing buffer.
 */
BERR_Code B_DTCP_ConsumeCapabilityExchangeMsg( B_AkeCoreSessionData_T * Session,
        unsigned char * buffer,
        int len,
        int headroom)
{
    BERR_Code retValue = BERR_SUCCESS;

    BDBG_ASSERT(Session);
    BDBG_ENTER(B_DTCP_ConsumeCapabilityExchangeMsg);
    (void)len;

    BKNI_Memcpy(Session->CapabilityExchSignature,
            buffer + headroom + sizeof(unsigned int),
            DTCP_SIGNATURE_SIZE);
    Session->pcp_ur_cap = ntohl(*(unsigned int *)(buffer + headroom));
    BKNI_Free(buffer);
    Session->CmdBuffer = NULL;
    Session->CmdBufferSize = 0;
    BDBG_LEAVE(B_DTCP_ConsumeCapabilityExchangeMsg);

    return retValue;
}

/*! \brief exported function, Initialize the message interface pointer based on stack type.
 *  \param[in] StackId the stack ID, see b_dtcp_types.h for details.
 *  \retval pointer to DTCP message interface pointer.
 */
B_DTCP_AkeMessageInterface_T * B_DTCP_InitMessageInterface(B_StackId_T StackId)
{
    if(StackId == B_StackId_eIP)
        return &gIpAkeMessageInterface;
    else
        return NULL;
}
/*! \brief cleanup message interface, free allocated resources.
 *  \param[in] pMessenger. pointer to messange interface.
 */
void B_DTCP_UnInitMessageInterface(B_StackId_T StackId, B_DTCP_AkeMessageInterface_T * pMessenger)
{
    (void)pMessenger; (void)StackId;
}
/* ENd of File */
