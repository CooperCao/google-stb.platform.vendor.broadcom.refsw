/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 *****************************************************************************/
#ifndef B_DTCP_APPLIB_H
#define B_DTCP_APPLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DTCP_CONTENT_LENGTH_UNLIMITED   -1
#define DTCP_DESCRIPTOR_LENGTH  6


/* \enum B_DTCP_KeyFormat
 * \brief DTCP-IP key format, e.g. testing key or production key.
 */
typedef enum B_DTCP_KeyFormat
{
    B_DTCP_KeyFormat_eTest = 0,
    B_DTCP_KeyFormat_eCommonDRM = 1,
    B_DTCP_KeyFormat_eProduction
}B_DTCP_KeyFormat_T;

/* \enum B_DeviceMode
 * \brief Device mode
 */
typedef enum B_DeviceMode
{
    B_DeviceMode_eSource = 0,           /* Format-non-cognizant source function. */
    B_DeviceMode_eSink = 1,             /* Format-non-cognizant sink function. */
    B_DeviceMode_eFCSource = 2,         /* Format-cognizant source function */
    B_DeviceMode_eFCSink = 3,           /* Format-cognizant sink function. */
    B_DeviceMode_eRecord = 5,           /* Format-non-cognizant recording function. */
    B_DeviceMode_eFCRecord = 7,         /* Format-cognizant recording function. */
    B_DeviceMode_eAudioFCSource = 4,    /* Audio-Format-cognizant source function. */
    B_DeviceMode_eAudioFCSink = 9,      /* Audio-format-cognizant sink function.*/
    B_DeviceMode_eAudioFCRecord = 11 ,  /* Audio-format-cognizant recording function.*/
    /* Audio-format-non-cognizant sink/record function only available for bridge.*/
    B_DeviceMode_eBridge,
    B_DeviceMode_eSourceWithAsp,        /* Format-non-cognizant source function with Encryption offload to a ASP. */
    B_DeviceMode_eSinkWithAsp           /* Format-non-cognizant source function with Decryption offload to a ASP. */
}B_DeviceMode_T;

#define IS_SINK(mode)   ( mode & B_DeviceMode_eSink)
#define IS_SOURCE(mode) (!(mode & B_DeviceMode_eSink))

/*! \enum B_StackId
 *  \brief Stack ID, currently only DTCP-IP is implemented.
 */
typedef enum B_StackId
{
    B_StackId_eIP,
    B_StackId_e1394,
    B_StackId_eUSB
}B_StackId_T;
/*! \enum b_stream_transport
 *  \brief stream transport type.
 */
typedef enum b_stream_transport
{
    B_StreamTransport_eRtp,
    B_StreamTransport_eHttp,
    B_StreamTransport_eUdp
}B_StreamTransport_T;

/* \enum b_stream_type
 * \brief Stream type source/sink
 */
typedef enum b_stream_type
{
    B_Stream_eSource,
    B_Stream_eSink
}B_StreamType_T;
/* \enum b_ur_mode
 * \brief usage rule mode, if PCP-UR is used.
 */
typedef enum b_ur_mode
{
    B_UR_Mode_eNoInfo,
    B_UR_Mode_eCCI,
    B_UR_Mode_eNoCCI,
    B_UR_Mode_eReserved
}B_UR_Mode_T;
/* \enum b_content_type
 * \brief content type in PCP_UR header.
 */
typedef enum b_content_type
{
    B_Content_eAudioVisual,
    B_Content_eType1Audio,
    B_Content_eReserved1,
    B_Content_eReserved2,
    /* Additional type for PCP_UR handling*/
    B_Content_eMPEG_TS,
    B_Content_eType2Audio,
    B_Content_eMultiplex
}B_Content_Type_T;

/* \enum b_aps_cci
 * \brief analog copy protection info.
 */
typedef enum b_aps_cci
{
    B_APS_eCopyFree,
    B_APS_eType1,
    B_APS_eType2,
    B_APS_eType3
}B_APS_CCI_T;

/* \enum b_ict
 * \brief image constraint token.
 */
typedef enum b_ict
{
    B_ICT_e0,   /* HD Analog output in the form of constraint image.*/
    B_ICT_e1    /* HD analog output in HD analog form. */
}B_ICT_T;

/* \enum b_ast
 * \brief Analog Sunset Token
 */
typedef enum b_ast
{
    B_AST_e0,   /* ASP-unasserted. */
    B_AST_e1    /* ASP-asserted. */
}B_AST_T;

/* \enmu b_cci
 * \brief copy generation management information.
 */
typedef enum b_cci
{
    B_CCI_eCopyFree,
    B_CCI_eNoMoreCopy,
    B_CCI_eCopyOneGeneration,
    B_CCI_eCopyNever
}B_CCI_T;
/* \enum b_audio_cci
 * \brief CCI for audio content.
 */
typedef enum b_audio_cci
{
    B_Audio_CCI_eCopyFree,
    B_Audio_CCI_eNoMoreCopy,
    B_Audio_CCI_eCPT,       /* copy permitted per type.*/
    B_Audio_CCI_eNA
}B_Audio_CCI_T;

/* \enum b_retention_stte
 * \brief retention_state values.
 */
typedef enum b_retention_state
{
    B_Retention_Forever,
    B_Retention_1week,
    B_Retention_2days,
    B_Retention_1day,
    B_Retention_12hours,
    B_Retention_6hours,
    B_Retention_3hours,
    B_Retention_90minutes
}B_Retention_State;

/* Device info for
 * diagnostic purposes
 */
typedef struct B_DTCP_Info
{
    bool AL;                   /* Additional Localization Flag */
    bool CommonDeviceCert;     /* is the device using a common device certificate? */
    unsigned char SrmGen;      /* SRM Generation */
    unsigned short SrmVersion; /* SRM version */
    unsigned short SrmLength;  /* SRM Length */
}B_DTCP_Info;

/*
 * Get some common device info for diagnostic purposes.
 */
BERR_Code DtcpAppLib_GetDeviceInfo(void * ctx, B_DTCP_Info *info);

/*!\brief exproted library startup function,must be called before any other AKE function call.
 * \param[in] mode device mode source/sink
 */
void * DtcpAppLib_Startup(B_DeviceMode_T mode, bool use_pcp_ur, B_DTCP_KeyFormat_T key_format, bool ckc_check);
/*! \brief shut down dtcp-ip library, free any internal allocated resources.
 *  \param[in] ctx DTCP-IP stack context pointer, obtained from DtcpAppLib_Startup() function.
 *  \retval none.
 */
void DtcpAppLib_Shutdown(void * ctx);

/*! \brief Perform AKE procedure, for sink device only. If the AkeHandle is already available
 *  \application can use the same API to Verify Exchange Keys. This API combines the functionality
 *  \provided by DtcpAppLib_DoAke() and DtcpAppLib_VerifyExchKey() together.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp server's IP address.
 *  \param[in] aRemotePort server's DTCP port number.
 *  \param[out] aAkeHandle result AKE handle, dereferencing the handle to obtain AKE session.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_DoAkeOrVerifyExchKey(void * ctx, const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle);
/*! \brief Perform AKE procedure, for sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp server's IP address.
 *  \param[in] aRemotePort server's DTCP port number.
 *  \param[out] aAkeHandle result AKE handle, dereferencing the handle to obtain AKE session.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_DoAke(void * ctx, const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle);
/*! \brief  close AKE session, after the DoAke call, used by sink device only.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aAkeHandle active AKE handle to close.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CloseAke(void * ctx, void *aAkeHandle);
/*! \brief start DTCP/IP source functionon listen to sink device's AKE request
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aSourceIp the IP address to listen on, if it's NULL will listen on all interfaces.
 *  \param[in] aSourcePort DTCP-IP port number
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_Listen(const void * ctx, const char *aSourceIp, unsigned short aSourcePort);
/*! \brief stop DTCP/IP source function, cancel listening.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_CancelListen(void * ctx);
/*! \brief function to verify sink's exchange key is still valid or not.
 * \param[in] ctx DTCP-IP context pointer.
 * \param[in] aAkeHandle Ake session handle.
 * \retvalu true if the exchange key is valid or false otherwise.
 */
bool DtcpAppLib_VerifyExchKey(void * ctx, void *aAkeHandle);
/*! \brief for an exsiting AKE session whose exchange key has expired, call
 *  this function to re-authenticate with server to get new exchange key.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aAkeHandle AKE session handle.
 *  \retval BERR_SUCCESS or other error code.
 */
int DtcpAppLib_GetNewExchKey(void * ctx, void *aAkeHandle);

/*! \brief retrive active sink device's AKE session.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] aRemoteIp sink device's IP address.
 *  \param[out] aAkeHandle AkE handle pointer retrived.
 *
 *  This function is used by source device streaming function, once received streaming request from sink, it will
 *  try to retrive AKE handle for this sink, if AKE handle is NULL< streaming request should be rejected.
 */
void DtcpAppLib_GetSinkAkeSession(void * ctx, const char *aRemoteIp, void **aAkeHandle);

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
        int ContentLength, int cci, int content_type, int max_packet_size);

/*! \brief set source stream atribute (copy control information)
 *  \param[in[ stream stream handle.
 *  \param[in] content_has_cci flag to indicate if the content has CCI or not.
 *  \param[in] content_type type of the content.
 *  \param[in] aps analog copy right information.
 *  \param[in] ict image constraint token.
 *  \param[in] ast analog sunset token.
 */
void DtcpAppLib_SetSourceStreamAttribute(void *hStreamHandle, bool content_has_cci,
        int content_type, int aps, int ict, int ast);
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
        int *content_type, int *aps, int *ict, int *ast);

/*! \brief set the stream's emi value, if caller wish to override the emi value
 *  obtained internally by DTCP lib.
 *  \param[in] hStreamHandle handle to the sink stream.
 *  \param[in] emi emi value to set.
 */
void DtcpAppLib_SetSourceStreamEmi(void *hStreamHandle, int emi);
/*! \brief Get the stream's emi value.
 *  \param[in] hStreamHandle handle to the sink stream.
 */
int DtcpAppLib_GetSinkStreamEmi(void *hStreamHandle);

/*! \brief Get the DTCP descriptor based on stream info, to insert into PMT of MPEG_TS stream.
 * \param[in] hStreamHandle handle to the source stream.
 * \param[in] buf buffer to store the dtcp descriptor.
 * \param[in] length length of the buffer.
 */
BERR_Code DtcpAppLib_GetDescriptor(void *hStreamHandle, unsigned char *buf, int length);

/*! \brief Retrive active sink device's AKE & Stream related attributes.
 *  \param[in] ctx DTCP-IP context pointer.
 *  \param[in] stream stream handle.
 *  \param[out] pSinkAttribute filled-in with AKE & Stream related attributes defined in the B_DTCP_SinkAkeAttribute below.
 *
 *  This function is used by source device to retrive AKE & Stream related attributes such as Exchange Key,
 *  Exchange Key Label, initial Nonce, EMI modes. Caller can use these values to offload the
 *  encryption logic to a hardware assist (which will generate the content keys & directly encrypt the stream).
 *  This API must be called after DtcpAppLib_OpenSourceStream.
 */
typedef struct B_DTCP_SinkAkeStreamAttribute
{
#define B_DTCP_IP_EXCH_KEY_IN_BYTES 12
    unsigned char               exchKey[B_DTCP_IP_EXCH_KEY_IN_BYTES];
    unsigned char               exchKeyLabel;
    unsigned                    emiModes;
#define B_DTCP_IP_NONCE_IN_BYTES 8
    unsigned char               initialNonce[B_DTCP_IP_NONCE_IN_BYTES];
} B_DTCP_SinkAkeStreamAttribute;
BERR_Code DtcpAppLib_GetSinkAkeStreamAttribute(void *hStreamHandle, B_DTCP_SinkAkeStreamAttribute *pSinkAttribute);

/*! \brief Initialize a DTCP-IP sink stream.
 *  \param[in] aAkeHandle AKE session handle.
 *  \param[in] TransportType stream transport type.
 *  \retval a stream handle if success of NULL if failed.
 */
void * DtcpAppLib_OpenSinkStream(void *aAkeHandle, B_StreamTransport_T TransportType);

/*! \brief close a opened stream.
 * \param[in] hStreamHandle the stream handle.
 */
void DtcpAppLib_CloseStream(void * hStreamHandle);

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
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf, unsigned int * encrypted_buf_size, unsigned int * total);


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
        unsigned int pcpHeaderBufferSize, unsigned int *pcpHeaderOffset, bool *pcpHeaderInserted, bool eof );

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
        unsigned char * encrypted_buf, unsigned int encrypted_buf_size, unsigned char * clear_buf, unsigned int * clear_buf_size, unsigned int * total, unsigned * pcp_header_count);

/*! \brief Initialize hardware security context.
 *
 *  Only when DTCP_IP_HARDWARE_DECRYPTION or DTCP_IP_HARDWARE_ENCRYPTION is defined.
 */
BERR_Code DtcpInitHWSecurityParams(void * nexusDmaHandle);

/*! \brief cleanup hardware security context.
 *
 *  Only when DTCP_IP_HARDWARE_DECRYPTION or DTCP_IP_HARDWARE_ENCRYPTION is defined.
 */
void DtcpCleanupHwSecurityParams(void);

#if defined(B_DTCP_IP_HW_ENCRYPTION) || defined(B_DTCP_IP_HW_DECRYPTION)
/*
  CVP2 Authentication API's
*/
int DtcpAppLib_GetDtcpCert(unsigned char *pLocalCert, unsigned int *pLocalCertSize);
BERR_Code DtcpAppLib_SignData(void * ctx, unsigned char *pBuffer, int len, unsigned char *pSignature, unsigned int *pSignatureLen);
BERR_Code DtcpAppLib_VerifyData(void * ctx, unsigned char *pSignature, unsigned char *pBuffer, int len, unsigned char *pRemoteCert, int *valid);
BERR_Code DtcpAppLib_VerifyRemoteCert(void * ctx, unsigned char *pRemoteCert, int *valid);
/*
  End CPV2 Authentication
*/
#endif

#ifdef __cplusplus
}
#endif
#endif /* B_DTCP_APPLIB_H */
