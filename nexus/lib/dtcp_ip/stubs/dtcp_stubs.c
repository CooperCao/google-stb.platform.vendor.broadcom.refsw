/********************************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
********************************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "bstd.h"
#include "b_dtcp_applib.h"

#define BERR_Code uint32_t
#define BERR_SUCCESS 0

BDBG_MODULE(b_dtcp_ip);

#ifdef __cplusplus
extern "C"
{
#endif

#define IS_SINK(mode)   ( mode & B_DeviceMode_eSink)
#define IS_SOURCE(mode) (!(mode & B_DeviceMode_eSink))

/*!\brief exproted library startup function,must be called before any other AKE function call.
 * \param[in] mode device mode source/sink
 */
void * DtcpAppLib_Startup(B_DeviceMode_T mode, bool use_pcp_ur, B_DTCP_KeyFormat_T key_format, bool ckc_check)
    {BSTD_UNUSED(mode); BSTD_UNUSED(use_pcp_ur); BSTD_UNUSED(key_format); BSTD_UNUSED(ckc_check); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return NULL;}
/*! \brief shut down dtcp-ip library, free any internal allocated resources.
 *  \param[in] ctx DTCP-IP stack context pointer, obtained from DtcpAppLib_Startup() function.
 *  \retval none.
 */
void DtcpAppLib_Shutdown(void * ctx){BSTD_UNUSED(ctx); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));}

/*! \brief Perform AKE procedure, for sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp server's IP address.
 *  \param[in] aRemotePort server's DTCP port number.
 *  \param[out] aAkeHandle result AKE handle, dereferencing the handle to obtain AKE session.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_DoAke(void * ctx, const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle)
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aRemoteIp); BSTD_UNUSED(aRemotePort); BSTD_UNUSED(aAkeHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

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
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aRemoteIp); BSTD_UNUSED(aRemotePort); BSTD_UNUSED(aAkeHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

/*! \brief  close AKE session, after the DoAke call, used by sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aAkeHandle active AKE handle to close.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CloseAke(void * ctx, void *aAkeHandle)
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aAkeHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

/*! \brief start DTCP/IP source functionon listen to sink device's AKE request
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aSourceIp the IP address to listen on, if it's NULL will listen on all interfaces.
 *  \param[in] aSourcePort DTCP-IP port number
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_Listen(const void * ctx, const char *aSourceIp, unsigned short aSourcePort)
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aSourceIp); BSTD_UNUSED(aSourcePort); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

/*! \brief function to verify sink's exchange key is still valid or not.
 * \param[in] ctx DTCP-IP context pointer.
 * \param[in] aAkeHandle Ake session handle.
 * \retvalu true if the exchange key is valid or false otherwise.
 */
bool DtcpAppLib_VerifyExchKey(void * ctx, void *aAkeHandle)
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aAkeHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return false;}

/*! \brief retrive active sink device's AKE session.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp sink device's IP address.
 *  \param[out] aAkeHandle AkE handle pointer retrived.
 *
 *  This function is used by source device streaming function, once received streaming request from sink, it will
 *  try to retrive AKE handle for this sink, if AKE handle is NULL< streaming request should be rejected.
 */
void DtcpAppLib_GetSinkAkeSession(void * ctx, const char *aRemoteIp, void **aAkeHandle)
        {BSTD_UNUSED(ctx); BSTD_UNUSED(aRemoteIp); BSTD_UNUSED(aAkeHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));}


/*! \brief Initialize a DTCP-IP source stream
 *  \param[in] aAkehandle AKE session handle.
 *  \param[in] TransportType stream transport type.
 *  \param[in] ContentLength length of the raw content.
 *  \param[in] cci content's CCI (Copy Control Information) value.
 *  \param[in] content_type type of the content, e.g AudioVisual, etc.
 *  \param[in] max_packet_size size of the PCP packet.
 *  \retval a stream handle if success or a NULL if failed.
 */
void * DtcpAppLib_OpenSourceStream(void *aAkeHandle, B_StreamTransport_T TransportType,
        int ContentLength, int cci, int content_type, int max_packet_size)
    {BSTD_UNUSED(aAkeHandle); BSTD_UNUSED(TransportType); BSTD_UNUSED(ContentLength); BSTD_UNUSED(cci); BSTD_UNUSED(content_type); BSTD_UNUSED(max_packet_size);
    BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return NULL;}

/*! \brief set the stream's emi value, if caller wish to override the emi value
 *  obtained internally by DTCP lib.
 *  \param[in] hStreamHandle handle to the sink stream.
 *  \param[in] emi emi value to set.
 */
void DtcpAppLib_SetSourceStreamEmi(void *hStreamHandle, int emi)
        {BSTD_UNUSED(hStreamHandle); BSTD_UNUSED(emi); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));}

/*! \brief Get the stream's emi value.
 *  \param[in] hStreamHandle handle to the sink stream.
 */
int DtcpAppLib_GetSinkStreamEmi(void *hStreamHandle)
    {BSTD_UNUSED(hStreamHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

/*! \brief Initialize a DTCP-IP sink stream.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] TransportType stream transport type.
 *  \retval a stream handle if success of NULL if failed.
 */
void * DtcpAppLib_OpenSinkStream(void *aAkeHandle, B_StreamTransport_T TransportType)
        {BSTD_UNUSED(aAkeHandle); BSTD_UNUSED(TransportType); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return NULL;}

/*! \brief close a opened stream.
 * \param[in] hStreamHandle the stream handle.
 */
void DtcpAppLib_CloseStream(void * hStreamHandle)
    {BSTD_UNUSED(hStreamHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));}

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
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf, unsigned int * encrypted_buf_size, unsigned int * total)
        {BSTD_UNUSED(hStreamHandle); BSTD_UNUSED(hAkeHandle); BSTD_UNUSED(clear_buf); BSTD_UNUSED(clear_buf_size); BSTD_UNUSED(encrypted_buf); BSTD_UNUSED(encrypted_buf_size); BSTD_UNUSED(total);
        BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

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
        {BSTD_UNUSED(hStreamHandle); BSTD_UNUSED(hAkeHandle); BSTD_UNUSED(clear_buf); BSTD_UNUSED(clear_buf_size); BSTD_UNUSED(encrypted_buf); BSTD_UNUSED(encrypted_buf_size); BSTD_UNUSED(total);
        BSTD_UNUSED(pcpHeaderBuffer); BSTD_UNUSED(pcpHeaderBufferSize);BSTD_UNUSED(pcpHeaderOffset);BSTD_UNUSED(pcpHeaderInserted); BSTD_UNUSED(eof);
        BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

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
        unsigned char * encrypted_buf, unsigned int encrypted_buf_size, unsigned char * clear_buf, unsigned int * clear_buf_size, unsigned int * total, unsigned * pcp_header_count)
        {BSTD_UNUSED(hStreamHandle); BSTD_UNUSED(hAkeHandle); BSTD_UNUSED(clear_buf); BSTD_UNUSED(clear_buf_size); BSTD_UNUSED(encrypted_buf); BSTD_UNUSED(encrypted_buf_size); BSTD_UNUSED(total);
        BSTD_UNUSED(pcp_header_count);
        BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}


/*! \brief Initialize hardware security context.
 *
 *  Only when DTCP_IP_HARDWARE_DECRYPTION or DTCP_IP_HARDWARE_ENCRYPTION is defined.
 */
BERR_Code DtcpInitHWSecurityParams(void * nexusDmaHandle)
        {BSTD_UNUSED(nexusDmaHandle); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

/*! \brief cleanup hardware security context.
 *
 *  Only when DTCP_IP_HARDWARE_DECRYPTION or DTCP_IP_HARDWARE_ENCRYPTION is defined.
 */
void DtcpCleanupHwSecurityParams(void)
        {BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));}


/*! \brief stop DTCP/IP source function, cancel listening.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CancelListen(void * ctx)
        {BSTD_UNUSED(ctx); BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION)); return BERR_NOT_SUPPORTED;}

BERR_Code DtcpAppLib_SignData(void * ctx, unsigned char *pBuffer, int len, unsigned char *pSignature, unsigned int *pSignatureLen)
{
    BSTD_UNUSED(ctx); BSTD_UNUSED(pBuffer); BSTD_UNUSED(len); BSTD_UNUSED(pSignature); BSTD_UNUSED(pSignatureLen);
    BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));
    return BERR_NOT_SUPPORTED;
}

BERR_Code DtcpAppLib_VerifyData(void * ctx, unsigned char *pSignature, unsigned char *pBuffer, int len, unsigned char *pRemoteCert, int *valid)
{
    BSTD_UNUSED(ctx); BSTD_UNUSED(pBuffer); BSTD_UNUSED(len); BSTD_UNUSED(pSignature); BSTD_UNUSED(pRemoteCert); BSTD_UNUSED(valid);
    BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));
    return BERR_NOT_SUPPORTED;
}

BERR_Code DtcpAppLib_VerifyRemoteCert(void * ctx, unsigned char *pRemoteCert, int *valid)
{
    BSTD_UNUSED(ctx); BSTD_UNUSED(pRemoteCert); BSTD_UNUSED(valid);
    BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));
    return BERR_NOT_SUPPORTED;
}

BERR_Code DtcpAppLib_GetSinkAkeStreamAttribute(void *hStream, B_DTCP_SinkAkeStreamAttribute *pSinkAttribute)
{
    BSTD_UNUSED(hStream); BSTD_UNUSED(pSinkAttribute);
    BDBG_WRN(("%s: This API is not implented for this version of software. If you are a DTLA licensee and intend to use this API, please contact appropriate SQA member to get the DTCP-IP sources\n", BSTD_FUNCTION));
    return BERR_NOT_SUPPORTED;
}
#ifdef __cplusplus
}
#endif
