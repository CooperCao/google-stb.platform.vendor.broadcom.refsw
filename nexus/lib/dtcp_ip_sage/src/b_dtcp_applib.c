/********************************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *   Top-level DTCP-IP application library.
 *********************************************************************************************/
/*! \file b_dtcp_applib.c
 *  \brief top-level dtcp application library interface.
 */
#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_srm.h"
#include "b_dtcp_ip_stack.h"
#include "b_dtcp_stream.h"
#include "b_ecc_wrapper.h"

BDBG_MODULE(b_dtcp_ip);

/* int DtcpAppLib_Startup();
 * Implemented in b_dtcp_applib_priv.c
 */

/*! \brief shut down dtcp-ip library, free any internal allocated resources.
 *  \param[in] ctx DTCP-IP stack context pointer, obtained from DtcpAppLib_Startup() function.
 *  \retval none.
 */
void DtcpAppLib_Shutdown(void * ctx)
{
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;
    BDBG_ASSERT(pStack);
#if 0
    B_DTCP_CleanupEccParams(&pStack->DeviceParams->EccParams);
#endif
    DRM_DtcpIpTl_Finalize(pStack->DeviceParams->hDtcpIpTl);
    if (pStack->DeviceParams->Cert != NULL) {
        BKNI_Free(pStack->DeviceParams->Cert);
    }
    BKNI_Free(pStack->DeviceParams);
    B_DTCP_Stack_UnInit(pStack);
}

/*****************************************************************************
 * DTCP-IP Source                                                            *
 *****************************************************************************/

/******************
 * Source globals *
 ******************/

static char gSourceIp[] = "0.0.0.0";    /* Source Ip Address to listen on */
static unsigned int gSourcePort = 0;    /* Source Port to bind to */

/********************
 * Source functions *
 ********************/

const char *DtcpAppLib_GetSourceIp(void)
{
    return &gSourceIp[0];
}

unsigned short DtcpAppLib_GetSourcePort(void)
{
    return gSourcePort;
}
/*! \brief start DTCP/IP source functionon listen to sink device's AKE request
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aSourceIp the IP address to listen on, if it's NULL will listen on all interfaces.
 *  \param[in] aSourcePort DTCP-IP port number
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_Listen(const void * ctx, const char *aSourceIp, unsigned short aSourcePort)
{
    int returnValue = BERR_SUCCESS;
    struct __dtcp_ip_stack_data sdata;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    BKNI_Memset(&sdata, 0, sizeof(struct __dtcp_ip_stack_data));
    if (NULL != aSourceIp)
    {
        strncpy(sdata.LocalIp, aSourceIp, strlen(aSourceIp)+1);
    }else {
        BDBG_MSG(("Local address not specified by caller of Listen, setting local address as 0.0.0.0 to enable listen for all interfaces"));
        strncpy(sdata.LocalIp, gSourceIp, strlen(gSourceIp)+1);
    }
    sdata.LocalPort = aSourcePort;

    returnValue = pStack->StartSource_Func(pStack, (void*)&sdata);

    if (returnValue == BERR_SUCCESS)
    {
        BDBG_MSG(("DTCP Listening on %s:%d\n\n", sdata.LocalIp, sdata.LocalPort));
    } else {
        BDBG_MSG(("DTCP Listen FAILED: %d\r\n", returnValue) );
    }
    return returnValue;
}

/*
 * Get some common device info for diagnostic purposes.
 */
BERR_Code DtcpAppLib_GetDeviceInfo(void * ctx, B_DTCP_Info *info)
{
    BERR_Code returnValue = BERR_SUCCESS;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);
    BDBG_ASSERT(info);
    B_BaselineFullCert_T * cert = (B_BaselineFullCert_T *)pStack->DeviceParams->Cert;
    info->AL = cert->AL & 0x01;
    info->CommonDeviceCert = pStack->DeviceParams->CommonDeviceCert;

    info->SrmVersion = B_DTCP_GetSrmV((unsigned char *)pStack->DeviceParams->Srm);
    info->SrmGen = B_DTCP_GetSrmC((unsigned char *)pStack->DeviceParams->Srm);
    info->SrmLength = B_DTCP_GetSrmLength((unsigned char *)pStack->DeviceParams->Srm);

    return returnValue;
}

/*! \brief stop DTCP/IP source function, cancel listening.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CancelListen(void * ctx)
{
    int returnValue = BERR_SUCCESS;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    returnValue = pStack->StopSource_Func(pStack);

    return returnValue;
}
/*! \brief Return sink AKE session handle.
 *  \param[in] aRemoteIp sink device's IP address.
 *  \param[in,out] aAkeHandle pointer to AKE session handle.
 *  \retval none *aAkeHandle will be NULL if session couldn't be retrived.
 *
 *  This function is used by streaming server, if sink device request streaming
 *  from source device, the source call this function to retrieve AKE handle.
 */
void DtcpAppLib_GetSinkAkeSession(void * ctx, const char *aRemoteIp, void **aAkeHandle)
{
    B_DTCP_IP_GetAkeHandle((B_DTCP_StackHandle_T)ctx, aRemoteIp, aAkeHandle, false);
}

/*****************************************************************************
 * DTCP-IP Sink                                                              *
 *****************************************************************************/
/*! \brief Perform AKE procedure, for sink device only. If the AkeHandle is already available
 *  \application can use the same API to Verify Exchange Keys. This API combines the functionality
 *  \provided by DtcpAppLib_DoAke() and DtcpAppLib_VerifyExchKey() together.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp server's IP address.
 *  \param[in] aRemotePort server's DTCP port number.
 *  \param[out] aAkeHandle result AKE handle, dereferencing the handle to obtain AKE session.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_DoAkeOrVerifyExchKey(void * ctx, const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle)
{
    int returnValue = BERR_SUCCESS;
    BDBG_ASSERT(ctx);

    if (*aAkeHandle)
    {
        if (DtcpAppLib_VerifyExchKey(ctx, *aAkeHandle))
        {
            BDBG_MSG(("VERIFED EXCH KEY"));
        }
        else
        {
            BDBG_WRN(("Unable to verify session exchange keys ... Keys may have expired "));
            if ((returnValue = DtcpAppLib_GetNewExchKey(ctx, *aAkeHandle)) != BERR_SUCCESS)
            {
                BDBG_ERR(("Unable to get new Exchange Keys for the session. Please close and clean existing AKE session and start over to create a new session."));
                *aAkeHandle = NULL;
            }
        }
    }

    if (!*aAkeHandle)
    {
        if((returnValue = DtcpAppLib_DoAke(ctx, aRemoteIp, aRemotePort, aAkeHandle)) != BERR_SUCCESS)
        {
            BDBG_ERR(("DTCP AKE Failed!!!\n"));
        }
    }

    return returnValue;
}

/*! \brief Perform AKE procedure, for sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp server's IP address.
 *  \param[in] aRemotePort server's DTCP port number.
 *  \param[out] aAkeHandle result AKE handle, dereferencing the handle to obtain AKE session.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_DoAke(void * ctx, const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle)
{
    int returnValue = BERR_SUCCESS;
    struct __dtcp_ip_stack_data sdata;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    if (NULL == aRemoteIp || 0 == aRemotePort || NULL == aAkeHandle )
    {
        return BERR_INVALID_PARAMETER;
    }
    if (NULL != *aAkeHandle )
    {
        *aAkeHandle = NULL;
        BDBG_WRN(("######################## Warning: App passed-in non-NULL akeHandle, API requires it to be NULL, setting it to NULL!!!! ########################"));
        BDBG_WRN(("Please make sure that you are closing the previous akeHandle using DtcpAppLib_CloseAke() and setting it to NULL after that & calling next DtcpAppLib_DoAke"));
    }
    strncpy(sdata.RemoteIp, aRemoteIp, strlen(aRemoteIp)+1);
    sdata.RemotePort = aRemotePort;

    returnValue = pStack->StartSink_Func(pStack, (void *)&sdata, aAkeHandle);

    if (returnValue == BERR_SUCCESS)
    {
        BDBG_MSG(("Do AKE succeeded\n"));
    }else {
        BDBG_MSG(("Do AKE failed: %d\n", returnValue));
    }

    return returnValue;
}
/*! \brief  close AKE session, after the DoAke call, used by sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aAkeHandle active AKE handle to close.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CloseAke(void * ctx, void *aAkeHandle)
{
    int ret = 0;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    ret = pStack->StopSink_Func(pStack, aAkeHandle);
    BKNI_Sleep(100);
    return ret;

}
/*! \brief function to verify sink's exchange key is still valid or not.
 * \param[in] ctx DTCP-IP context pointer.
 * \param[in] aAkeHandle Ake session handle.
 * \retvalu true if the exchange key is valid or false otherwise.
 */
bool DtcpAppLib_VerifyExchKey(void * ctx, void *aAkeHandle)
{
    bool Valid = false;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    if(pStack->VerifyExchKey_Func(pStack, aAkeHandle, &Valid) != BERR_SUCCESS)
    {
        BDBG_ERR(("Couldn't verify exchange Key for session: %p\n", (void *)aAkeHandle));
        return Valid;
    }

    return Valid;

}
/*! \brief function to verify sink's exchange key is still valid or not.
 * \param[in] ctx DTCP-IP context pointer.
 * \param[in] aAkeHandle Ake session handle.
 * \retvalu true if the exchange key is valid or false otherwise.
 */
#include "b_ecc_wrapper.h"
BERR_Code DtcpAppLib_SignData(void * ctx, unsigned char *pBuffer, int len, unsigned char *pSignature, unsigned int *pSignatureLen)
{
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    if(B_DTCP_SignData_BinKey(pStack->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                pSignature, pBuffer, len
                ) != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to sign the data", __FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }
    *pSignatureLen = DTCP_SIGNATURE_SIZE;

    BDBG_ERR(("%s: Successfully signed the data", __FUNCTION__));
    return BERR_SUCCESS;

}

BERR_Code DtcpAppLib_VerifyData(void * ctx, unsigned char *pSignature, unsigned char *pBuffer, int len, unsigned char *pRemoteCert, int *valid)
{
    B_BaselineFullCert_T * Cert;
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;

    BDBG_ASSERT(pStack);

    Cert = (B_BaselineFullCert_T *)(pRemoteCert);
    BDBG_MSG(("\n %s: Received other device's certificate:\n", __FUNCTION__));
    BDBG_MSG(("\n Type: %d\n Format:%d\n DevGen: %d\n AL:%d \n AP: %d\n",
                Cert->CertType & 0x0F, Cert->Format & 0x0F, Cert->DevGen & 0x0F,
                Cert->AL & 0x01, Cert->AP & 0x01));
    BDBG_MSG(("\n DevId: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                Cert->DevID[0], Cert->DevID[1], Cert->DevID[2], Cert->DevID[3], Cert->DevID[4]));

    if(B_DTCP_VerifyData_BinKey(pStack->pAkeCoreData->pDeviceParams->hDtcpIpTl, valid, pSignature, pBuffer, len,
                Cert->PublicKey) != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to verify the data signature", __FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }

    BDBG_ERR(("%s: Successfully verified the data signature", __FUNCTION__));
    return BERR_SUCCESS;
}

BERR_Code DtcpAppLib_VerifyRemoteCert(void * ctx, unsigned char *pRemoteCert, int *valid)
{
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;
    B_BaselineFullCert_T * Cert;

    BDBG_ASSERT(pStack);

    /* get the public key from the remoteCert */
    Cert = (B_BaselineFullCert_T *)(pRemoteCert);
    BDBG_MSG(("\n %s: Received other device's certificate:\n", __FUNCTION__));
    BDBG_MSG(("\n Type: %d\n Format:%d\n DevGen: %d\n AL:%d \n AP: %d\n",
                Cert->CertType & 0x0F, Cert->Format & 0x0F, Cert->DevGen & 0x0F,
                Cert->AL & 0x01, Cert->AP & 0x01));
    BDBG_MSG(("\n DevId: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                Cert->DevID[0], Cert->DevID[1], Cert->DevID[2], Cert->DevID[3], Cert->DevID[4]));

    if(B_DTCP_VerifyData_BinKey(pStack->pAkeCoreData->pDeviceParams->hDtcpIpTl,
                valid, (unsigned char *)Cert->Signature,
                (unsigned char *)Cert,
                DTCP_BASELINE_FULL_CERT_SIZE - DTCP_SIGNATURE_SIZE,
                pStack->DeviceParams->dtlaPublicKey)
            != Drm_Success)
    {
        BDBG_ERR(("%s: Failed to verify remote certificate\n", __FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }
    if(!*valid)
    {
        BDBG_ERR(("%s: Invalid certificate\n", __FUNCTION__));
        return BERR_INVALID_PARAMETER;
    }
    /*
       BKNI_Memcpy(Session->OtherDeviceId, Cert->DevID, DTCP_DEVICE_ID_SIZE);
       BKNI_Memcpy(Session->OtherPublicKey, Cert->PublicKey, DTCP_PUBLIC_KEY_SIZE);
       */

    BDBG_ERR(("%s: Successfully verified the remote certificate", __FUNCTION__));
    return BERR_SUCCESS;
}

/*! \brief for an exsiting AKE session whose exchange key has expired, call
 *  this function to re-authenticate with server to get new exchange key.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aAkeHandle AKE session handle.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_GetNewExchKey(void * ctx, void *aAkeHandle)
{
    B_DTCP_StackHandle_T pStack = (B_DTCP_StackHandle_T)ctx;
    BDBG_ASSERT(pStack);
    return B_DTCP_IP_GetNewExchKey(pStack, aAkeHandle);
}
#if 0
int DtcpAppLib_DoBgRtt(const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle)
{
    int returnValue = LOGIC_ERROR;

    if (NULL == aRemoteIp || 0 == aRemotePort || NULL == aAkeHandle ) //|| NULL == *aAkeHandle)
    {
        returnValue = INVALID_ARGUMENT;
        return returnValue;
    }

    returnValue = DtcpApi_OpenAkeIp((char *) aRemoteIp, aRemotePort, aAkeHandle);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpApi_DoBgRtt(*aAkeHandle);
        if (IS_FAILURE(returnValue))
        {
            DEBUG_MSG(MSG_ERR, ("Do BG-RTT failed: %d\n", returnValue));
        } else {
            DEBUG_MSG(MSG_INFO, ("Do BG-RTT succeeded.\n"));
        }
    }
    else
    {
        DEBUG_MSG(MSG_ERR, ("Open Ake failed: %d\n", returnValue));
    }

    return returnValue;
}
#endif
/*****************************************************************************
 * DTCP-IP Streaming interfaces.
 *****************************************************************************/
/*! \brief Initialize a DTCP-IP source stream
 *  \param[in] aAkehandle AKE session handle.
 *  \param[in] TransportType stream transport type.
 *  \param[in] ContentLength length of the raw content.
 *  \param[in] cci content's CCI (Copy Control Information) value.
 *  \param[in] content_type type of the content.
 *  \param[in] max_packet_size size of the PCP packet.
 *  \retval a stream handle if success or a NULL if failed.
 */
void * DtcpAppLib_OpenSourceStream(void *aAkeHandle, B_StreamTransport_T TransportType,
        int ContentLength, int cci, int content_type, int max_packet_size)
{
    if(TransportType != B_StreamTransport_eHttp)
    {
        BDBG_ERR(("Stream Transport: %d is not supported\n", TransportType));
        return NULL;
    }
    return B_DTCP_IP_OpenSourceStream((B_AkeHandle_T)aAkeHandle, TransportType,
            ContentLength, cci, content_type, max_packet_size);
}
/*! \brief set source stream atribute (copy control information)
 *  \param[in[ stream stream handle.
 *  \param[in] content_has_cci flag to indicate if the content has CCI or not.
 *  \param[in] content_type type of the content.
 *  \param[in] aps analog copy right information.
 *  \param[in] ict image constraint token.
 *  \param[in] ast analog sunset token.
 */
void DtcpAppLib_SetSourceStreamAttribute(void *hStreamHandle, bool content_has_cci,
        int content_type, int aps, int ict, int ast)
{
    B_DTCP_IP_SetSourceStreamAttribute( (B_StreamHandle_T)hStreamHandle,
            content_has_cci, content_type, aps, ict, ast);
}
/*! \brief get the sink stream's attribute, to determine
 * if processing (decryption) is allowed.
 * \param[in] akeHandle AKE session handle.
 * \param[in] stream stream handle.
 * \param[in/out] content_type stream content type.
 * \param[in/out] aps analog copy right information.
 * \param[in/out] ict image constraint token.
 * \param[in/out] ast analog sunset token.
 */
BERR_Code DtcpAppLib_GetSinkStreamAttribute(void *akeHandle, void *hStreamHandle,
        int *content_type, int *aps, int *ict, int *ast)
{
    return B_DTCP_IP_GetSinkStreamAttribute((B_AkeHandle_T)akeHandle,
            (B_StreamHandle_T)hStreamHandle, content_type,
            aps, ict, ast);
}
/*! \brief set the stream's emi value, if caller wish to override the emi value
 *  obtained internally by DTCP lib.
 *  \param[in] hStreamHandle handle to the sink stream.
 *  \param[in] emi emi value to set.
 */
void DtcpAppLib_SetSourceStreamEmi(void *hStreamHandle, int emi)
{
    B_DTCP_IP_SetSourceStreamEmi((B_StreamHandle_T)hStreamHandle, emi);
}
/*! \brief Get the stream's emi value.
 *  \param[in] hStreamHandle handle to the sink stream.
 */
int DtcpAppLib_GetSinkStreamEmi(void *hStreamHandle)
{
    return B_DTCP_IP_GetSinkStreamEmi((B_StreamHandle_T)hStreamHandle);
}
/*! \brief Get the DTCP descriptor based on stream info, to insert into PMT of MPEG_TS stream.
 * \param[in] hStreamHandle handle to the source stream.
 * \param[in] buf buffer to store the dtcp descriptor.
 * \param[in] length length of the buffer.
 */
BERR_Code DtcpAppLib_GetDescriptor(void *hStreamHandle, unsigned char *buf, int length)
{
    return B_DTCP_IP_GetDescriptor((B_StreamHandle_T)hStreamHandle, buf, length);
}
/*! \brief Initialize a DTCP-IP sink stream.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] TransportType stream transport type.
 *  \retval a stream handle if success of NULL if failed.
 */
void * DtcpAppLib_OpenSinkStream(void *aAkeHandle, B_StreamTransport_T TransportType)
{
    if(TransportType != B_StreamTransport_eHttp)
    {
        BDBG_ERR(("Stream Transport: %d is not supported\n", TransportType));
        return NULL;
    }
    return B_DTCP_IP_OpenSinkStream((B_AkeHandle_T)aAkeHandle, TransportType);
}
/*! \brief close a opened stream.
 * \param[in] hStreamHandle the stream handle.
 */
void DtcpAppLib_CloseStream(void * hStreamHandle)
{
    B_DTCP_IP_CloseStream((B_StreamHandle_T)hStreamHandle);
}
/*! \brief Packetize data, used by source device only.
 *  \param[in] hStreamHandle the source stream handle.
 *  \param[in] hAkeHandle handle to AKE session.
 *  \param[in] clear_buf buffer hold the input data.
 *  \param[in] clear_buf_size size of the input buffer.
 *  \param[in,out] encrypted_buf buffer for output data.
 *  \param[in] encrypted_buf_size size of output buffer.
 *  \param[out] total total bytes processed.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code DtcpAppLib_StreamPacketizeData(void * hStreamHandle, void * hAkeHandle,
        unsigned char * clear_buf, unsigned int clear_buf_size,
        unsigned char ** encrypted_buf, unsigned int * encrypted_buf_size, unsigned int * total)
{
    return B_DTCP_IP_PacketizeData((B_AkeHandle_T)hAkeHandle, (B_StreamHandle_T)hStreamHandle,
            clear_buf, clear_buf_size, encrypted_buf, encrypted_buf_size, total);
}

/*! \brief packetize data w/PCP header, produce PCP stream, called by source device only.
 *  unlike the B_DTCP_IP_PacketizeData API which adds the PCP header inline, this API will provide
 *  header in a separate PCP header buffer provided by the caller.
 *
 *  \param[in] hAkeHandle handle to AKE session.
 *  \param[in] clear_buf buffer hold the input data.
 *  \param[in] clear_buf_size size of the input buffer.
 *  \param[in,out] encrypted_buf buffer for output data.
 *  \param[in] encrypted_buf_size size of output buffer.
 *  \param[out] total total bytes processed            .
 *  \param[out] pcpHeaderBuffer buffer for PCP header data                                                 .
 *  \param[in] pcpHeaderBufferSize size of PCP header buffer                                                                                                       .
 *  \param[out] pcpHeaderOffset offset in the encrypted buffer where PCP header belongs
 *  \param[out] pcpHeaderInserted if true, read the pcpHeaderOffset, else ignore offset
 */
BERR_Code DtcpAppLib_StreamPacketizeDataWithPcpHeader(void * hStreamHandle, void * hAkeHandle,
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf,
        unsigned int * encrypted_buf_size, unsigned int * total, unsigned char *pcpHeaderBuffer,
        unsigned int pcpHeaderBufferSize, unsigned int *pcpHeaderOffset, bool *pcpHeaderInserted, bool eof )
{
    return B_DTCP_IP_PacketizeDataWithPcpHeader((B_AkeHandle_T)hAkeHandle, (B_StreamHandle_T)hStreamHandle,
            clear_buf, clear_buf_size, encrypted_buf, encrypted_buf_size, total, pcpHeaderBuffer, pcpHeaderBufferSize,
            pcpHeaderOffset, pcpHeaderInserted, eof);
}


/*! \brief Depacketize data, used by sink device only.
 *  \param[in] hStreamHandle the sink stream handle.
 *  \param[in] hAkeHandle handle to AKE session.
 *  \param[in] clear_buf buffer hold the input data.
 *  \param[in] clear_buf_size size of the input buffer.
 *  \param[in,out] encrypted_buf buffer for output data.
 *  \param[in] encrypted_buf_size size of output buffer.
 *  \param[out] total total bytes processed.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code DtcpAppLib_StreamDepacketizeData(void * hStreamHandle, void * hAkeHandle,
        unsigned char * encrypted_buf, unsigned int encrypted_buf_size,
        unsigned char * clear_buf, unsigned int * clear_buf_size, unsigned int * total, bool * pcp_header_found)
{
    return B_DTCP_IP_DepacketizeData((B_AkeHandle_T)hAkeHandle, (B_StreamHandle_T)hStreamHandle,
            encrypted_buf, encrypted_buf_size, clear_buf, clear_buf_size, total, pcp_header_found);
}
