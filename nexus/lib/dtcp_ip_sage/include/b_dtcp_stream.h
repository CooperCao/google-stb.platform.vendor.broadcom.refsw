/********************************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * DTCP-IP streamer component
 *********************************************************************************************/
/*! \file b_dtcp_stream.h
 *  \brief define DTCP content management/streaming interface.
 */
#ifndef B_DTCP_STREAM_H
#define B_DTCP_STREAM_H

#include "b_dtcp_constants.h"
#include "nexus_dma.h"
#include "nexus_security.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define DTCP_AES_IV_BOUNDARY    96000


/*
 * Internal private data structure
 */
typedef struct __b_dtcp_stream_data
{
    BLST_S_ENTRY(__b_dtcp_stream_data) node;
    B_StreamType_T StreamType;      /* Stream type, source/sink */
    B_StreamTransport_T Transport;  /* Transport type, Rtp/Http/Udp */
    int Emi;                        /* Extended EMI. */
    int cci;                        /* Embedded CCI of the content */
    /* Variables for PCP_UR handling */
    int ur_mode;
    int content_type;
    int aps;
    int ict;
    int ast;
    unsigned char HttpNonce[DTCP_CONTENT_KEY_NONCE_SIZE];
    int content_length;             /* Total length for this stream, what about RTP, total length unknown */
    int content_remain;             /* Remainning content length to be processed, source only */
    int content_processed;          /* Size of clear content that has been processed */
    int packet_content_length;      /* PCP byte length, upto 128MB */
    int packet_content_remain;      /* Total content remain.*/
    int packet_bytes_remain;        /* bytes remain for this PCP */
    int packet_padding_bytes;
    int max_packet_size;
    bool use_per_packet_pcp;        /* if max_packet_size is == 0, then PCP is prepended to each encrypted block */
    void * hPacketHandle;           /* Packet handle, obtained from ConsumePacketHeader or CreatePacketHeader function.*/
    void * AkeHandle;               /* Associated AKE session for this stream */
    NEXUS_KeySlotHandle dtcpIpKeyHandle;
    NEXUS_DmaHandle dtcpIpDmaHandle;

    unsigned int keySlotID;
}B_DTCP_StreamData_T;

struct __b_dtcp_descriptor_private_data
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    unsigned int Descriptor_id:         1;
    unsigned int Retention_Move_mode:   1;
    unsigned int Retention_State:       3;
    unsigned int EPN:       1;
    unsigned int DTCP_CCI:  2;
    unsigned int Reserved:  5;
    unsigned int Image_Constraint_Token:    1;
    unsigned int APS:   2;
#else
    unsigned int DTCP_CCI:  2;
    unsigned int EPN:       1;
    unsigned int Retention_State:   3;
    unsigned int Retention_Move_mode:   1;
    unsigned int Descriptor_id:         1;
    unsigned int APS:   2;
    unsigned int Image_Constraint_Token:    1;
    unsigned int Reserved:  5;
#endif
};
struct __b_dtcp_audio_descriptor_private_data
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    unsigned int Descriptor_id: 1;
    unsigned int Reserved1:     5;
    unsigned int DTCP_CCI_Audio:    2;
    unsigned int Audio_Type:    3;
    unsigned int Reserved2:     5;
#else
    unsigned int DTCP_CCI_Audio:    2;
    unsigned int Reserved1:     5;
    unsigned int Descriptor_id: 1;
    unsigned int Reserved2:     5;
    unsigned int Audio_Type:    3;
#endif
};
struct __b_dtcp_descriptor
{
    unsigned char descriptor_tag;
    unsigned char descriptor_length;
    unsigned short CA_System_ID;
    unsigned char private_data[2];
};
typedef struct __b_dtcp_stream_data * B_StreamHandle_T;
/*! \brief function pointer to a function to open a source stream, after AKE.
 *  \param[in] akeHandle handle to AKE session.
 *  \param[in] Transport the stream's transport type.
 *  \param[in] content_length length of the content to be transmited.
 *  \param[in] emi emi value of this stream.
 *  \param[in] max_packet_size maximum packet size (pcp length).
 */
B_StreamHandle_T B_DTCP_IP_OpenSourceStream(B_AkeHandle_T akeHandle, B_StreamTransport_T Transport,
        int content_length, int cci, int content_type, int max_packet_size);

/*! \brief exported function to set the stream attribute, when PCP_UR is used.
 *  only set these attribute for source stream.
 *  param[in] stream source stream handle.
 *  param[in] content_has_cci, a flag indicate the content has embedded CCI.
 *  param[in] content_type type of the content,e.g. AudioVisual, type1Audio, etc.
 *  param[in] aps Analog copy protection information obtained from embedded CCI.
 *  param[in] ict Image_Constraint_Token obtained from CCI.
 *  param[in] ast Analog_sunset_token obtained from CCI.
 */
void B_DTCP_IP_SetSourceStreamAttribute(B_StreamHandle_T stream, bool content_has_cci,
        int content_type, int aps, int ict, int ast);
/*! \brief set the stream's emi value, if caller wish to override the emi value
 *  obtained from calling GetEmiFromCCI() function.
 *  \param[in] stream stream handle.
 *  \param[in] emi emi value to set.
 */
void B_DTCP_IP_SetSourceStreamEmi(B_StreamHandle_T stream, int emi);
int B_DTCP_IP_GetSinkStreamEmi(B_StreamHandle_T  stream);
/*! \brief exported function to check the sink stream's attribute, to determine
 * if processing (decryption) is allowed.
 * param[in] akeHandle AKE session handle.
 * param[in] stream stream handle.
 * param[in/out] content_type stream content type.
 * param[in/out] aps analog copy right information.
 * param[in/out] ict image constraint token.
 * param[in/out] ast analog sunset token.
 */
BERR_Code B_DTCP_IP_GetSinkStreamAttribute(B_AkeHandle_T akeHandle, B_StreamHandle_T stream,
        int *content_type, int *aps, int *ict, int *ast);

BERR_Code B_DTCP_IP_GetDescriptor(B_StreamHandle_T stream, unsigned char *buf, int length);

/*! \brief function to open a sink stream, after AKE.
 *  \param[in] akeHandle handle to AKE session.
 *  \param[in] Transport the stream's transport type.
 */
B_StreamHandle_T B_DTCP_IP_OpenSinkStream(B_AkeHandle_T akeHandle, B_StreamTransport_T Transport);

/*! \brief close a stream.
 *  \param[in] hStreamHandle handle to the open stream.
 */
void B_DTCP_IP_CloseStream(B_StreamHandle_T hStreamHandle);

/*! \brief packetize data, produce PCP stream, called by source device only.
 *  \param[in] hAkeHandle handle to AKE session.
 *  \param[in] clear_buf buffer hold the input data.
 *  \param[in] clear_buf_size size of the input buffer.
 *  \param[in,out] encrypted_buf buffer for output data.
 *  \param[in] encrypted_buf_size size of output buffer.
 *  \param[out] total total bytes processed.
 */
BERR_Code B_DTCP_IP_PacketizeData(B_AkeHandle_T hAkeHandle, B_StreamHandle_T hStreamHandle,
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf,
        unsigned int * encrypted_buf_size, unsigned int * total);

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
BERR_Code B_DTCP_IP_PacketizeDataWithPcpHeader(B_AkeHandle_T hAkeHandle, B_StreamHandle_T hStreamHandle,
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf,
        unsigned int * encrypted_buf_size, unsigned int * total, unsigned char *pcpHeaderBuffer,
        unsigned int pcpHeaderBufferSize, unsigned int *pcpHeaderOffset, bool *pcpHeaderInserted, bool eof );

/*! \brief Depacketize DTCP PCP data received from source device.
 *  param[in] Akehandle AkeSession handle obtained from AKE procedure.
 *  param[in] encrypted_buf buffer holding the encrypted data.
 *  param[in] encrypted_buf_size size of the encrypted buffer.
 *  param[in] clear_buf buffer to hold the decrypted data.
 *  param[out] clear_buf_size size of decrypted bytes.
 *  param[out] total total length processed.
 */
BERR_Code B_DTCP_IP_DepacketizeData(B_AkeHandle_T hAkeHandle, B_StreamHandle_T hStreamHandle,
        unsigned char * encrypted_buf, unsigned int encrypted_buf_size, unsigned char * clear_buf, unsigned int * clear_buf_size,
        unsigned int * total, unsigned * pcp_header_count);

/*! \brief force to close packet handle, called by source device's exchange key timer expire function.
 *  \param[in] hStreamHandle sream handle.
 *  \retval none.
 */
void B_DTCP_Stream_ClosePacketHandle(struct __b_dtcp_stream_data * StreamData);

#ifdef __cplusplus
}
#endif
#endif /* B_DTCP_STREAM_H */
