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
 *     DTCP Streaming functions.
 *
 *********************************************************************************************/
#include <stdio.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_ip_ake.h"
#include "b_dtcp_status_codes.h"
#include "b_dtcp_stream.h"
#include "b_dtcp_content.h"
#include "b_dtcp_stack.h"
#include "b_dtcp_ip_stack.h"
#include "nexus_security_client.h"
#include "nexus_core_utils.h"

BDBG_MODULE(b_dtcp_ip);

/*#define DUMP_STREAM       0*/

static int pcpNum;
#ifdef DUMP_STREAM
static FILE * fout;
#endif

#define MIN(a, b) ((a > b)? b : a)

/*! \brief return proper EMI value for a stream given the content stream's CCI value and device mode.
 * \param[in] Session AKE session data.
 * \param[in] cci content stream's CCI value.
 * \param[in] content_type stream's content type.
 * \param[out] returned emi value.
 */
static void B_DTCP_IP_GetEmiFromCCI(B_AkeCoreSessionData_T *Session, int cci, int content_type, int *emi)
{
    switch (Session->DeviceMode) {
        case B_DeviceMode_eSource:
            /* Format-non-cognizant source function*/
            if (cci == B_CCI_eCopyNever)
                *emi = B_ExtEmi_eCopyNever;
            else if (cci == B_CCI_eCopyOneGeneration) {
                /* NOTE1: we set the emi to COG [Format-cognizant recording only].
                 * If it's desired that this stream is recordable by
                 * Format-non-cognizant device, reset it later by API.
                 */
                *emi = B_ExtEmi_eCopyOneFC;
            } else if (cci == B_CCI_eNoMoreCopy)
                *emi = B_ExtEmi_eNoMoreCopy;
            else if (cci == B_CCI_eCopyFree)
                *emi = B_ExtEmi_eCopyFree;
            else
                *emi = B_ExtEmi_eInvalid;
            break;
        case B_DeviceMode_eFCSource:
            /*Format-cognizant source function */
            if (cci == B_CCI_eCopyNever)
                *emi = B_ExtEmi_eCopyNever;
            else if (cci == B_CCI_eCopyOneGeneration &&
                    content_type == B_Content_eAudioVisual) {
                /* See NOTE1 above */
                *emi = B_ExtEmi_eCopyOneFC;
            } else if (cci == B_CCI_eNoMoreCopy)
                *emi = B_ExtEmi_eNoMoreCopy;
            else if (cci == B_CCI_eCopyFree)
                *emi = B_ExtEmi_eCopyFree;
            else
                *emi = B_ExtEmi_eInvalid;
            break;
        case B_DeviceMode_eAudioFCSource:
            /*NOTE2: for autio, copy-never is Type Specific!.*/
            if (cci == B_Audio_CCI_eCPT)
                *emi = B_ExtEmi_eCopyOneFC;
            else if (cci == B_Audio_CCI_eNoMoreCopy)
                *emi = B_ExtEmi_eNoMoreCopy;
            else if (cci == B_Audio_CCI_eCopyFree)
                *emi = B_ExtEmi_eCopyFree;
            else
                *emi = B_ExtEmi_eInvalid;
            break;
        default:
            *emi = B_ExtEmi_eInvalid;
    }

}
/*! \brief exported function to set the stream attribute, when PCP_UR is used.
 *  only set these attribute for source stream.
 *  \param[in] stream source stream handle.
 *  \param[in] content_has_cci, a flag indicate the content has embedded CCI.
 *  \param[in] content_type type of the content,e.g. AudioVisual, type1Audio, etc.
 *  \param[in] aps Analog copy protection information obtained from embedded CCI.
 *  \param[in] ict Image_Constraint_Token obtained from CCI.
 *  \param[in] ast Analog_sunset_token obtained from CCI.
 */
void B_DTCP_IP_SetSourceStreamAttribute(B_StreamHandle_T stream, bool content_has_cci,
        int content_type, int aps, int ict, int ast)
{
    BDBG_ASSERT(stream);
    stream->content_type = content_type;
    /* DTCP-IP V1SE1.31, section 4.23.3*/
    if (content_type == B_Content_eMPEG_TS ||
            content_type == B_Content_eType2Audio ||
            content_type == B_Content_eMultiplex) {
        BDBG_WRN(("Setting PCP_UR to ZERO\n"));
        return ;
    }
    /*
     * V1SE 1.31, section 4.23.3, use UR_MODE 01 if aps, ict, ast available
     * from embedded CCI.
     */
    if (content_has_cci == true)
        stream->ur_mode = B_UR_Mode_eCCI;
    else
        stream->ur_mode = B_UR_Mode_eNoCCI;
    if (content_type == B_Content_eAudioVisual) {
        stream->aps = aps;
        stream->ict = ict;
        stream->ast = ast;
    }else if (content_type == B_Content_eType1Audio) {
        stream->aps = 0;
        stream->ict = 0;
        stream->ast = 0;
    }
}
/*! \brief exported function to check the sink stream's attribute, to determine
 * if processing (decryption) is allowed.
 * \param[in] akeHandle AKE session handle.
 * \param[in] stream stream handle.
 * \param[in/out] content_type stream content type.
 * \param[in/out] aps analog copy right information.
 * \param[in/out] ict image constraint token.
 * \param[in/out] ast analog sunset token.
 */
BERR_Code B_DTCP_IP_GetSinkStreamAttribute(B_AkeHandle_T akeHandle, B_StreamHandle_T stream,
        int *content_type, int *aps, int *ict, int *ast)
{
    B_AkeCoreSessionData_T *Session = (B_AkeCoreSessionData_T *)akeHandle;
    BDBG_ASSERT(Session);

    if (Session->pcp_ur_cap == 0)
        return BERR_NOT_SUPPORTED;
    *content_type = stream->content_type;
    *aps = stream->aps;
    *ict = stream->ict;
    *ast = stream->ict;

    return BERR_SUCCESS;
}

/*! \brief set the stream's emi value, if caller wish to override the emi value
 *  obtained from calling GetEmiFromCCI() function.
 *  \param[in] stream stream handle.
 *  \param[in] emi emi value to set.
 */
void B_DTCP_IP_SetSourceStreamEmi(B_StreamHandle_T stream, int emi)
{
    BDBG_ASSERT(stream);
    stream->Emi = emi;
}
/*! \brief Get the sink stream's emi value
 *  \param[in] stream stream handle.
 *  \param[in] emi emi value to set.
 */
int B_DTCP_IP_GetSinkStreamEmi(B_StreamHandle_T stream)
{
    BDBG_ASSERT(stream);
    return  stream->Emi;
}
/*! \brief Get the DTCP descriptor based on stream info, to insert into PMT of MPEG_TS stream.
 * \param[in] stream stream handle.
 * \param[in] buf buffer to store the dtcp descriptor.
 * \param[in] length length of the buffer.
 */
BERR_Code B_DTCP_IP_GetDescriptor(B_StreamHandle_T stream, unsigned char *buf, int length)
{
    struct __b_dtcp_descriptor *descriptor;
    struct __b_dtcp_descriptor_private_data *private_data;
    BDBG_ASSERT(buf);
    BDBG_ASSERT(stream);

    if (length < DTCP_DESCRIPTOR_LENGTH)
        return BERR_INVALID_BUFFER_LENGTH;

    if (stream->content_type != B_Content_eMPEG_TS)
        return BERR_NOT_SUPPORTED;

    descriptor = (struct __b_dtcp_descriptor *)buf;
    descriptor->descriptor_tag = 0x88;
    descriptor->descriptor_length = DTCP_DESCRIPTOR_LENGTH;
    descriptor->CA_System_ID = 0x0fff;

    /*NOTE: We only considered the MPEG_TS video stream here.*/
    private_data = (struct __b_dtcp_descriptor_private_data *)descriptor->private_data;
    private_data->Retention_Move_mode = 1;
    private_data->DTCP_CCI = stream->cci;
    private_data->Image_Constraint_Token = stream->ict;
    private_data->APS = stream->aps;

    return BERR_SUCCESS;

}

/*! \brief function pointer to a function to open a source stream, after AKE.
 *  \param[in] akeHandle handle to AKE session.
 *  \param[in] Transport the stream's transport type.
 *  \param[in] content_length length of the content to be transmited.
 *  \param[in] content_type type of the content, e.g. AudioVisual/Type1_Audio.
 *  \param[in] cci embedded cci(copy control information) of this stream.
 *  \param[in] max_packet_size maximum packet size (pcp length).
 */
B_StreamHandle_T B_DTCP_IP_OpenSourceStream(B_AkeHandle_T akeHandle, B_StreamTransport_T Transport,
        int content_length, int cci, int content_type, int max_packet_size)
{
    struct __b_dtcp_stream_data * StreamData = NULL;
    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)akeHandle;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BDBG_ASSERT(content_length);

    BDBG_ENTER(B_DTCP_IP_OpenSourceStream);

    if(Session == NULL) {
        BDBG_ERR(("Invalid AKE handle!\n"));
        return NULL;
    }

    if((StreamData = (struct __b_dtcp_stream_data *)BKNI_Malloc(sizeof(struct __b_dtcp_stream_data)))
            == NULL)
    {
        BDBG_ERR(("Failed to allocate memory for stream : size=%zu\n", sizeof(struct __b_dtcp_stream_data)));
        return NULL;
    }else {
        BKNI_Memset(StreamData, 0, sizeof(struct __b_dtcp_stream_data));
    }
    if (!max_packet_size)
        StreamData->use_per_packet_pcp = true;
    else if (max_packet_size & 0xFFFFF) {
        BDBG_ERR(("max_packet_size must be multiple of 1MB\n"));
        BKNI_Free(StreamData);
        return NULL;
    } else
        StreamData->use_per_packet_pcp = false;
    BDBG_MSG(("Per Packet PCP: %s\n", StreamData->use_per_packet_pcp? "Yes":"No"));
    /* Initalize stream data */
    StreamData->StreamType = B_Stream_eSource;
    StreamData->Transport = Transport;
    StreamData->cci = cci;
    B_DTCP_IP_GetEmiFromCCI(Session, cci, content_type, &StreamData->Emi);
    if (StreamData->Emi == B_ExtEmi_eInvalid) {
        BDBG_MSG(("Unable to get proper emi value based on CCI %d and content_type %d\n",
            cci, content_type));
        BDBG_MSG(("Setting emi to copy-never\n"));
        StreamData->Emi = B_ExtEmi_eCopyNever;
    }
    if (Session->pcp_ur_cap == 1)
        BDBG_WRN(("Using PCP_UR mode!\n"));
    StreamData->content_length = content_length;
    StreamData->content_remain = content_length;
    StreamData->max_packet_size = max_packet_size;
    StreamData->packet_content_length = 0;
    StreamData->hPacketHandle = NULL;
    StreamData->AkeHandle = akeHandle;

    /**************************************************************************/
    /*          Allocate and configure keyslot                                */
    /**************************************************************************/
    /*void*/
    BDBG_MSG(("In function %s", __FUNCTION__));
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m; /* NEXUS_SecurityEngine_eM2m,
                                                              NEXUS_SecurityEngine_eCa */
    keySettings.client = NEXUS_SecurityClientType_eSage;
    StreamData->dtcpIpKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if (StreamData->dtcpIpKeyHandle == NULL)
    {
        BDBG_ERR(("Unable to allocate keyslot for the stream"));
    }
    else
    {
        NEXUS_Security_GetKeySlotInfo(StreamData->dtcpIpKeyHandle, &keyslotInfo);
        BDBG_MSG(("%s - Keyslot index = '%u'", __FUNCTION__, keyslotInfo.keySlotNumber));
        StreamData->keySlotID = keyslotInfo.keySlotNumber;
    }

    /*get dma*/
    StreamData->dtcpIpDmaHandle = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);

    BLST_S_INSERT_HEAD(&(Session->Stream_list), StreamData, node);

    /* Incerase RefCnt when Source stream open and decrease when Stream close*/
    Session->RefCnt++;
    BDBG_MSG(("STREAM OPEN RefCnt %d", Session->RefCnt));

    BDBG_LEAVE(B_DTCP_IP_OpenSourceStream);

    return StreamData;

}
/*! \brief close a stream.
 *  \param[in] hStreamHandle handle to the open stream.
 */
void B_DTCP_IP_CloseStream(B_StreamHandle_T hStreamHandle)
{
    B_AkeCoreSessionData_T * Session;

    struct __b_dtcp_stream_data * pStreamData = NULL;

    BDBG_ASSERT(hStreamHandle);
    BDBG_ENTER(B_DTCP_IP_CloseStream);
    pStreamData = (struct __b_dtcp_stream_data *)hStreamHandle;

    /*
     * TODO : In case other thread is sending/receiving
     * stream, might need to set content_length to 0 and
     * sleep for a while?
     */
    if(pStreamData->hPacketHandle != NULL)
    {
        B_DTCP_Content_ClosePacket(pStreamData->hPacketHandle);
    }

    Session = (B_AkeCoreSessionData_T *)(pStreamData->AkeHandle);
    if(Session != NULL)
    {
        BLST_S_REMOVE(&(Session->Stream_list), pStreamData, __b_dtcp_stream_data, node);
    }
    if (pStreamData->dtcpIpKeyHandle != NULL)
    {
        DRM_DtcpIpTl_FreeKeySlot(Session->pAkeCoreData->pDeviceParams->hDtcpIpTl, pStreamData->keySlotID);
        NEXUS_Security_FreeKeySlot(pStreamData->dtcpIpKeyHandle);
    }
    if (pStreamData->dtcpIpDmaHandle != NULL) {
        NEXUS_Dma_Close(pStreamData->dtcpIpDmaHandle);
    }

    /* Call SessionClean to decrease RefCnt and clean up the session list */
    if (Session != NULL && Session->DeviceMode == B_DeviceMode_eSource)
    {
        B_DTCP_IP_SourceSessionClean((B_DTCP_StackHandle_T)(Session->pAkeCoreData->pStack), Session);
    }

    BKNI_Free(hStreamHandle);
    hStreamHandle = NULL;
    BDBG_LEAVE(B_DTCP_IP_CloseStream);
}

/*! \brief function to open a sink stream, after AKE.
 *  \param[in] akeHandle handle to AKE session.
 *  \param[in] Transport the stream's transport type.
 */
B_StreamHandle_T B_DTCP_IP_OpenSinkStream(B_AkeHandle_T akeHandle, B_StreamTransport_T Transport)
{
    struct __b_dtcp_stream_data * StreamData = NULL;
    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)akeHandle;

    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;

    BDBG_ENTER(B_DTCP_IP_OpenSinkStream);

    if(akeHandle == NULL) {
        BDBG_ERR(("%s: Invalid AKE handle\n", __FUNCTION__));
        return NULL;
    }
    if((StreamData = (struct __b_dtcp_stream_data *)BKNI_Malloc(sizeof(struct __b_dtcp_stream_data)))
            == NULL)
    {
        BDBG_ERR(("Failed to allocate memory for stream : size=%zu\n", sizeof(struct __b_dtcp_stream_data)));
        return NULL;
    }
    BKNI_Memset(StreamData, 0, sizeof(struct __b_dtcp_stream_data));
    /* Initalize stream data */
    StreamData->StreamType = B_Stream_eSink;
    StreamData->Transport = Transport;
    StreamData->hPacketHandle = NULL;
    StreamData->AkeHandle = akeHandle;
    if (Session->pcp_ur_cap == 1)
        BDBG_WRN(("Using PCP_UR mode!\n"));

    /**************************************************************************/
    /*          Allocate and configure keyslot                                */
    /**************************************************************************/
    /*void*/
    BDBG_MSG(("In function %s", __FUNCTION__));
    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m; /* NEXUS_SecurityEngine_eM2m,
                                                              NEXUS_SecurityEngine_eCa */
    keySettings.client = NEXUS_SecurityClientType_eSage;
    StreamData->dtcpIpKeyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if (StreamData->dtcpIpKeyHandle == NULL)
    {
        BDBG_ERR(("Unable to allocate keyslot for the stream"));
    }
    else
    {
        NEXUS_Security_GetKeySlotInfo(StreamData->dtcpIpKeyHandle, &keyslotInfo);
        BDBG_MSG(("%s - Keyslot index = '%u'", __FUNCTION__, keyslotInfo.keySlotNumber));
        StreamData->keySlotID = keyslotInfo.keySlotNumber;
    }

    /*get dma*/
    StreamData->dtcpIpDmaHandle = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);

    BLST_S_INSERT_HEAD(&(Session->Stream_list), StreamData, node);

    BDBG_LEAVE(B_DTCP_IP_OpenSinkStream);

    return StreamData;

}

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
        unsigned int * encrypted_buf_size, unsigned int * total)
{
    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)hAkeHandle;
    B_AkeCoreData_T * Core;
    BERR_Code retValue = BERR_SUCCESS;
    unsigned char *buf_ptr = NULL;
    struct __b_dtcp_stream_data * pStreamData = (struct __b_dtcp_stream_data *)hStreamHandle;

    unsigned int content_to_send = 0;   /* clear content to send */
    unsigned int data_to_send = 0;      /* encrypted content to send */
    unsigned int content_padding = 0;   /* padding for clear content */
    bool scatterGatherStart = false;
    bool scatterGatherEnd = false;
    /* unsigned int aligned_buf_size = 0; */

    BDBG_ASSERT(hStreamHandle);
    BDBG_ASSERT(clear_buf);
    BDBG_ASSERT(encrypted_buf);

    if(Session == NULL)
        return BERR_NOT_INITIALIZED;

    if(pStreamData->content_length != DTCP_CONTENT_LENGTH_UNLIMITED && pStreamData->content_remain == 0)
        return retValue;

    *total = 0;
    buf_ptr = *encrypted_buf;

    Core = Session->pAkeCoreData;
    if (pStreamData->use_per_packet_pcp)
        pStreamData->max_packet_size = clear_buf_size;
    while(pStreamData->content_length == DTCP_CONTENT_LENGTH_UNLIMITED ||
            (pStreamData->content_remain > 0 && clear_buf_size > *total) )
    {
        content_to_send = 0;
        content_padding = 0;

        if(pStreamData->hPacketHandle == NULL)
        {
            int pcpl = 0;

            /*
             * Protect the Timer reset by mutex. In case of quick channel changes from multiple clients,
             * there is a chance that some previous tiemr is not canceled and will
             * expire the Exchange keys even when streaming sesions are active
            */
            B_Mutex_Lock(Core->hMutex);
            /* Reschedule the exchange key timer */
            retValue = B_DTCP_StartExchKeyTimer(Session, Session->pAkeCoreData, B_DeviceMode_eSource);
            B_Mutex_Unlock(Core->hMutex);

            /*
             * Build packet header (PCPH), PCPH is needed for starting of
             * stream or previous PCP has been transmited out.
             */
            if( pStreamData->content_length == DTCP_CONTENT_LENGTH_UNLIMITED)
                pcpl = pStreamData->max_packet_size;
            else {
                pcpl = (pStreamData->content_remain > pStreamData->max_packet_size)?
                    pStreamData->max_packet_size : pStreamData->content_remain;
            }
            pStreamData->packet_content_length = pcpl;

            if (!(pStreamData->content_processed & 0x7FFFFFF))
            {
                /* Retrieve RealTimeNonce value from AKE core.*/
                B_DTCP_GetSetRealTimeNonce(Core->hMutex, Core->RealTimeNonce,
                    pStreamData->HttpNonce);

                /* Increment nonce value, every 128MB of data transfer */
                B_DTCP_IncrementNonce(Core->hMutex, Core->RealTimeNonce);
            }
            retValue = B_DTCP_Content_CreatePacketHeader(pStreamData->AkeHandle, pStreamData->Transport,
                    pStreamData, pcpl, buf_ptr);
            if(retValue != BERR_SUCCESS)
            {
                BDBG_ERR(("Failed to create PCP header for stream: %d \n", retValue));
                return retValue;
            }
            pStreamData->packet_content_remain = pStreamData->packet_content_length;
            buf_ptr += DTCP_CONTENT_PACKET_HEADER_SIZE;
            data_to_send += DTCP_CONTENT_PACKET_HEADER_SIZE;
#ifdef B_HAS_PCP_16_BYTES

            /* If NETACCEL is enabled, this 14 bytes header cause great headache for 16 bytes AES block decryption!
             * so we add 2 more bytes to make it 16 bytes aligned.
             */
            buf_ptr += 2;
            data_to_send += 2;
#endif
            scatterGatherStart = true;
            scatterGatherEnd = false;
        }
        content_to_send = (pStreamData->packet_content_remain > (int)clear_buf_size)?
            (int)clear_buf_size : pStreamData->packet_content_remain ;
        /*
         * Make sure the output buffer is big enough to hold encrypted data.
         */
        if(*encrypted_buf_size - data_to_send < 16)
        {
            BDBG_MSG(("NO enough room for content encryption: sending : bytes %d\n", data_to_send));
            retValue = B_ERROR_OUT_OF_MEMORY;
            break;
        }
        /*
         * Output buffer must be 16 bytes aligned (AES block size)
         */
        /* aligned_buf_size = ((*encrypted_buf_size - data_to_send)/16)*16; */
        if (content_to_send >= 16)
            content_to_send = (content_to_send / 16 )*16;
        /* content_to_send = (content_to_send > aligned_buf_size)? aligned_buf_size : content_to_send;  */
        /*
         * Add padding.
         */
        if(pStreamData->packet_content_remain - content_to_send == 0)
        {
            content_padding = (content_to_send % 16) == 0 ? 0: (16 - (content_to_send) % 16);
            if(content_to_send + content_padding + data_to_send > *encrypted_buf_size)
            {
                BDBG_MSG(("No enough room for padding: available=%d ask =%d\n",
                            *encrypted_buf_size, content_to_send + content_padding + data_to_send));
                retValue = B_ERROR_OUT_OF_MEMORY;
                break;
            }
            BKNI_Memset(buf_ptr + content_to_send, 0, content_padding);
        }
        #if 0
        /* Encrypt data */
        BDBG_MSG(("clear_buf_size = %d content_to_send=%d content_padding=%d content_remain=%d packet_content_remain=%d\n",
                    clear_buf_size, content_to_send, content_padding,
                    pStreamData->content_remain, pStreamData->packet_content_remain));
        #endif

        retValue = B_DTCP_Content_EncryptData(
            pStreamData->AkeHandle,
            pStreamData->hPacketHandle,
            clear_buf,
            buf_ptr,
            content_to_send + content_padding,
            pStreamData->dtcpIpKeyHandle,
            scatterGatherStart,
            scatterGatherEnd,
            pStreamData->dtcpIpDmaHandle,
            pStreamData->keySlotID
            );
        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to encrypt data: %d\n", retValue));
            break;
        }
        scatterGatherStart = false;
        scatterGatherEnd = false;

        buf_ptr += content_to_send + content_padding;
        data_to_send += content_to_send + content_padding;
        *total += content_to_send;
        pStreamData->content_processed += content_to_send;

        pStreamData->packet_content_remain -= content_to_send;
        if(pStreamData->content_length != DTCP_CONTENT_LENGTH_UNLIMITED)
            pStreamData->content_remain -= content_to_send;

        clear_buf += content_to_send;
        clear_buf_size -= content_to_send;

        /*
         * if current PCP is done, close packet and free packet handle.
         */
        if( pStreamData->packet_content_remain == 0)
        {
            B_DTCP_Content_ClosePacket(pStreamData->hPacketHandle);
            pStreamData->hPacketHandle = NULL;
        }

        if (0 == clear_buf_size)
        {
            break;
        }

    } /* while loop */

    *encrypted_buf_size = data_to_send;
    return retValue;
}

BERR_Code B_DTCP_IP_PacketizeDataWithPcpHeader(B_AkeHandle_T hAkeHandle, B_StreamHandle_T hStreamHandle,
        unsigned char * clear_buf, unsigned int clear_buf_size, unsigned char ** encrypted_buf,
        unsigned int * encrypted_buf_size, unsigned int * total, unsigned char *pcpHeaderBuffer,
        unsigned int pcpHeaderBufferSize, unsigned int *pcpHeaderOffset, bool *pcpHeaderInserted, bool eof )
{
    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)hAkeHandle;
    B_AkeCoreData_T * Core;
    BERR_Code retValue = BERR_SUCCESS;
    unsigned char *buf_ptr = NULL;
    struct __b_dtcp_stream_data * pStreamData = (struct __b_dtcp_stream_data *)hStreamHandle;

    unsigned int content_to_send = 0;   /* clear content to send */
    unsigned int data_to_send = 0;      /* encrypted content to send */
    unsigned int content_padding = 0;   /* padding for clear content */
    unsigned int encrypted_buffer_size = *encrypted_buf_size;
    bool scatterGatherStart = false;
    bool scatterGatherEnd = false;
    /* unsigned int aligned_buf_size = 0; */

    BDBG_ASSERT(hStreamHandle);
    BDBG_ASSERT(clear_buf);
    BDBG_ASSERT(encrypted_buf);
    BDBG_ASSERT(pcpHeaderBuffer);

    if(Session == NULL)
        return BERR_NOT_INITIALIZED;

    if(pStreamData->content_length != DTCP_CONTENT_LENGTH_UNLIMITED && pStreamData->content_remain == 0)
        return retValue;

    if (eof && (clear_buf_size % 16 != 0))
    {
        content_padding = 16 - (clear_buf_size % 16);
        if (encrypted_buffer_size < clear_buf_size + content_padding) {
            BDBG_MSG(("Encrypted buffer size is not sufficent. It needs to be incremented by %d", content_padding));
            return BERR_INVALID_BUFFER_LENGTH;
        }
        BKNI_Memset(*encrypted_buf + clear_buf_size, 0, content_padding);

        BDBG_WRN(("Encrypted buffer size has to be MOD16 and equal to clear buffer size"));
    }
    else if (clear_buf_size > encrypted_buffer_size)
    {
        BDBG_ERR(("ERROR:: clear_buf_size[%d] and encrypted_buffer_size[%d] should be same", clear_buf_size, encrypted_buffer_size));
        return BERR_INVALID_BUFFER_LENGTH;
    }
    else if (clear_buf_size < encrypted_buffer_size) {
        encrypted_buffer_size = clear_buf_size;
    }

    *total = 0;
    *pcpHeaderInserted = false;
    buf_ptr = *encrypted_buf;

    Core = Session->pAkeCoreData;
    if (pStreamData->use_per_packet_pcp)
        pStreamData->max_packet_size = clear_buf_size;
    while(pStreamData->content_length == DTCP_CONTENT_LENGTH_UNLIMITED ||
            (pStreamData->content_remain > 0 && clear_buf_size > *total) )
    {
        content_to_send = 0;

        if(pStreamData->hPacketHandle == NULL)
        {
            int pcpl = 0;
            /*
             * Build packet header (PCPH), PCPH is needed for starting of
             * stream or previous PCP has been transmited out.
             */

            /*
             * Protect the Timer reset by mutex. In case of quick channel changes from multiple clients,
             * there is a chance that some previous tiemr is not canceled and will
             * expire the Exchange keys even when streaming sesions are active
            */
            B_Mutex_Lock(Core->hMutex);
            /* Reschedule the exchange key timer */
            retValue = B_DTCP_StartExchKeyTimer(Session, Session->pAkeCoreData, B_DeviceMode_eSource);
            B_Mutex_Unlock(Core->hMutex);

            if( pStreamData->content_length == DTCP_CONTENT_LENGTH_UNLIMITED)
                pcpl = pStreamData->max_packet_size;
            else {
                pcpl = (pStreamData->content_remain > pStreamData->max_packet_size)?
                    pStreamData->max_packet_size : pStreamData->content_remain;
            }
            pStreamData->packet_content_length = pcpl;

            if (clear_buf_size > (unsigned int)pcpl)
            {
                /* because we cannot have more that 1 pcp header in this case */
                BDBG_ERR(("Since we can insert only 1 PCP header per call, clear_buf_size[%d] cannot be greater than PCP payload size[%d]"
                          , clear_buf_size, pcpl));
                return BERR_INVALID_BUFFER_LENGTH;
            }

            if (!(pStreamData->content_processed & 0x7FFFFFF))
            {
                /* Retrieve RealTimeNonce value from AKE core.*/
                B_DTCP_GetSetRealTimeNonce(Core->hMutex, Core->RealTimeNonce,
                    pStreamData->HttpNonce);

                /* Increment nonce value, every 128MB of data transfer */
                B_DTCP_IncrementNonce(Core->hMutex, Core->RealTimeNonce);
            }
            if (pcpHeaderBufferSize < DTCP_CONTENT_PACKET_HEADER_SIZE)
            {
                BDBG_ERR(("PCP header buffer size [%d]. Minumum required is 14 bytes", pcpHeaderBufferSize));
                return BERR_INVALID_BUFFER_LENGTH;
            }

            retValue = B_DTCP_Content_CreatePacketHeader(pStreamData->AkeHandle, pStreamData->Transport,
                    pStreamData, pcpl, pcpHeaderBuffer);
            if(retValue != BERR_SUCCESS)
            {
                BDBG_ERR(("Failed to create PCP header for stream: %d \n", retValue));
                return retValue;
            }
            pStreamData->packet_content_remain = pStreamData->packet_content_length;
            *pcpHeaderInserted = true;
            scatterGatherStart = true;
            scatterGatherEnd = false;
        }

        content_to_send = (pStreamData->packet_content_remain > (int)clear_buf_size)?
            (int)clear_buf_size : pStreamData->packet_content_remain ;
        /*
         * Output buffer must be 16 bytes aligned (AES block size)
         */
        /* aligned_buf_size = ((*encrypted_buf_size - data_to_send)/16)*16; */
        if (content_to_send >= 16)
            content_to_send = (content_to_send / 16 )*16;
        /* content_to_send = (content_to_send > aligned_buf_size)? aligned_buf_size : content_to_send;  */
        /*
         * Add padding.
         */
        #if 0
        if (eof)
        {
            if(pStreamData->packet_content_remain - content_to_send == 0)
            {
                content_padding = (content_to_send % 16) == 0 ? 0: (16 - (content_to_send) % 16);
                if(content_to_send + content_padding + data_to_send > *encrypted_buf_size)
                {
                    BDBG_ERR(("No enough room for padding: available=%d ask =%d\n",
                                *encrypted_buf_size, content_to_send + content_padding + data_to_send));
                    retValue = B_ERROR_OUT_OF_MEMORY;
                    break;
                }
                BKNI_Memset(buf_ptr + content_to_send, 0, content_padding);
            }
        }
        #endif

        /* Encrypt data */
        #if 0
        BDBG_MSG(("clear_buf_size = %d content_to_send=%d content_remain=%d packet_content_remain=%d packet_content_lenght=%d\n",
                    clear_buf_size, content_to_send,
                    pStreamData->content_remain, pStreamData->packet_content_remain,
                    pStreamData->packet_content_length));
        #endif
        retValue = B_DTCP_Content_EncryptData(
            pStreamData->AkeHandle,
            pStreamData->hPacketHandle,
            clear_buf,
            buf_ptr,
            content_to_send + content_padding,
            pStreamData->dtcpIpKeyHandle,
            scatterGatherStart,
            scatterGatherEnd,
            pStreamData->dtcpIpDmaHandle,
            pStreamData->keySlotID
            );
        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to encrypt data: %d\n", retValue));
            break;
        }
        scatterGatherStart = false;
        scatterGatherEnd = false;

        buf_ptr += content_to_send;
        data_to_send += content_to_send;
        *total += content_to_send;
        pStreamData->content_processed += content_to_send;

        pStreamData->packet_content_remain -= content_to_send;
        if(pStreamData->content_length != DTCP_CONTENT_LENGTH_UNLIMITED)
            pStreamData->content_remain -= content_to_send;

        clear_buf += content_to_send;
        clear_buf_size -= content_to_send;

        /*
         * if current PCP is done, close packet and free packet handle.
         */
        if( pStreamData->packet_content_remain == 0)
        {
            B_DTCP_Content_ClosePacket(pStreamData->hPacketHandle);
            pStreamData->hPacketHandle = NULL;
        }

        if (0 == clear_buf_size)
        {
            *pcpHeaderOffset = encrypted_buffer_size + pStreamData->packet_content_remain - pStreamData->packet_content_length;
            break;
        }

    } /* while loop */

    *encrypted_buf_size = data_to_send;
    return retValue;
}


/*! \brief Depacketize DTCP PCP data received from source device.
 *  param[in] Akehandle AkeSession handle obtained from AKE procedure.
 *  param[in] encrypted_buf buffer holding the encrypted data.
 *  param[in] encrypted_buf_size size of the encrypted buffer.
 *  param[in] clear_buf buffer to hold the decrypted data.
 *  param[out] clear_buf_size size of decrypted bytes.
 *  param[out] total total length processed.
 *  param[out] pcp_header_found flag indicate this block of data contains PCP header.
 */
BERR_Code B_DTCP_IP_DepacketizeData(B_AkeHandle_T hAkeHandle, B_StreamHandle_T hStreamHandle,
        unsigned char * encrypted_buf, unsigned int encrypted_buf_size, unsigned char * clear_buf, unsigned int * clear_buf_size,
        unsigned int * total, bool * pcp_header_found)
{
    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)hAkeHandle;
    B_IpAkeSessionData_T *IpSession = (B_IpAkeSessionData_T *)Session->pProtocolData;
    BERR_Code retValue = BERR_SUCCESS;
    struct __b_dtcp_stream_data * StreamData = (struct __b_dtcp_stream_data *)hStreamHandle;
#ifdef DUMP_STREAM
    char fname[16];
#endif
    unsigned char *buf_ptr = NULL;
    unsigned int data_to_process = encrypted_buf_size;
    unsigned int content_processed = 0;
    unsigned int data_processed = 0;
    bool scatterGatherStart = false;
    bool scatterGatherEnd = false;

    BDBG_ASSERT(hAkeHandle);
    BDBG_ASSERT(StreamData);
    BDBG_ASSERT(encrypted_buf);
    BDBG_ASSERT(clear_buf);
    BDBG_ASSERT(clear_buf_size);

    *total = 0;

    buf_ptr = clear_buf;

    if(StreamData->hPacketHandle != NULL)
        *pcp_header_found = false;
    do {
        /*
         * If packet handle is NULL, it means PCP header hasn't been consumed yet.
         */
        if(StreamData->hPacketHandle == NULL)
        {
#ifdef DUMP_STREAM
            sprintf(fname, "pcp_%d.bin", pcpNum);
            fout = fopen(fname, "wb");
            fwrite(encrypted_buf, 1, 14, fout);
            fflush(fout);
#endif
            /* Check to see if the data we feed to ConsumePacketHeader is atleast 14 bytes. If not then PCP header is at the end of input encrypted buffer and we should not process it.
               The caller will cache the unprocessed leftover and send it again in the next call.*/
            if (data_to_process < DTCP_CONTENT_PACKET_HEADER_SIZE) {
                BDBG_WRN(("data %p[%d bytes] (incomplete pcp header), encrypted_buf_size 0x%x, content_processed 0x%x, total 0x%x",
                          encrypted_buf, data_to_process, encrypted_buf_size, content_processed,  *total));
                B_DTCP_DebugBuff(encrypted_buf, data_to_process);
                break;
            }

            /*
             * Protect the Timer reset by mutex. In case of quick channel changes from multiple clients,
             * there is a chance that some previous tiemr is not canceled and will
             * expire the Exchange keys even when streaming sesions are active
            */
            B_Mutex_Lock(Session->pAkeCoreData->hMutex);
            /* Reschedule the exchange key timer */
            retValue = B_DTCP_StartExchKeyTimer(Session, Session->pAkeCoreData, B_DeviceMode_eSink);
            B_Mutex_Unlock(Session->pAkeCoreData->hMutex);

            retValue = B_DTCP_Content_ConsumePacketHeader(hAkeHandle, encrypted_buf, StreamData);
            if(retValue != BERR_SUCCESS)
            {
                BDBG_ERR(("Failed to decode PCP header\n"));
                goto ERR_OUT;
            }
            *pcp_header_found = true;
            pcpNum++;

            scatterGatherStart = true;
            scatterGatherEnd = false;
            /*
             * Figuring out the padding bytes.
             * V1SE 4.22 specifies that PCP header's Length doesn't include any padding.
             */
            StreamData->packet_padding_bytes = (StreamData->packet_content_length % 16 != 0)?
                (16 - StreamData->packet_content_length % 16) : 0;
            StreamData->packet_bytes_remain = StreamData->packet_content_length + StreamData->packet_padding_bytes;

            encrypted_buf += DTCP_CONTENT_PACKET_HEADER_SIZE;
            *total += DTCP_CONTENT_PACKET_HEADER_SIZE;
            data_processed += DTCP_CONTENT_PACKET_HEADER_SIZE;

#ifdef B_HAS_PCP_16_BYTES
            /*
             * if Broadcom's NETACCEL is enabled, multiple packets are read once, they must be 16 bytes aligned.
             * The PacketizeData() added 2 bytes after PCP header for this situation.
             */
            encrypted_buf += 2;
            *total += 2;
            data_processed += 2;
            /* end of NetAccel fixes */
#endif
            if (Session->pAkeCoreData->ckc_check == true)
            {
                /*
                * If both soruce/sink AL flag is set,
                * check if content key confirmation failed. quit decryption if it did.
                */
                if (IpSession->ContKeyConfCount < 5)
                    IpSession->StartContKeyConf = true;
                BDBG_MSG(("ContKeyConfCount=%d ContKeyConfirmed=%d\n", IpSession->ContKeyConfCount, IpSession->ContKeyConfirmed));
            }
        }

        data_to_process = MIN(StreamData->packet_bytes_remain, (int)(encrypted_buf_size - data_processed));

        /* Make sure we don't process data more then output buffer can hold */
        data_to_process = MIN(data_to_process, *clear_buf_size - content_processed);

        data_to_process = (data_to_process % 16 != 0) ?
            (data_to_process - (data_to_process % 16)) : (data_to_process);

        if (data_to_process == 0)
        {
            retValue = BERR_SUCCESS;
            break;
        }

        retValue = B_DTCP_Content_DecryptData(StreamData->AkeHandle, StreamData->hPacketHandle,
                encrypted_buf, buf_ptr, data_to_process, StreamData->dtcpIpKeyHandle, scatterGatherStart, scatterGatherEnd,
                StreamData->dtcpIpDmaHandle, StreamData->keySlotID);

        if(retValue != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to decrypt content stream: %d\n", retValue));
            goto ERR_OUT;
        }
        scatterGatherStart = false;
        scatterGatherEnd = false;
#ifdef DUMP_STREAM
        fwrite(encrypted_buf, 1, data_to_process, fout);
        fflush(fout);
#endif
#if 0
        BDBG_MSG(("\r\n\t encrypted_buf_size=%d data_to_process=%d packet_bytes_remain=%d padding_bytes=%d content_length=%d\n",
                    encrypted_buf_size, data_to_process, StreamData->packet_bytes_remain,
                    StreamData->packet_padding_bytes, StreamData->content_length));
#endif
        /* book keeping */
        encrypted_buf += data_to_process;
        buf_ptr += data_to_process;         /* advancing clear buffer pointer */
        data_processed += data_to_process;
        StreamData->packet_bytes_remain -= data_to_process;
        *total += data_to_process;
        StreamData->content_length += data_to_process;

        if (StreamData->packet_bytes_remain == 0)
        {
            content_processed += data_to_process - StreamData->packet_padding_bytes;
            /*
             * This PCP is done, close packet, if new data come in , expecting a new PCP
             * header in it, consume header function will allocate a new handle for it.
             */
            B_DTCP_Content_ClosePacket(StreamData->hPacketHandle);
            StreamData->hPacketHandle = NULL;
        } else {
            content_processed += data_to_process;
        }

        data_to_process = encrypted_buf_size - data_processed;

     } while (0 != data_to_process);

    *clear_buf_size = content_processed;
#if 0
    if(StreamData->hPacketHandle != NULL)
    {
        BDBG_ERR(("Rescheduling exchange key timer!\n"));
        /* Reschedule the exchange key timer */
        retValue = B_DTCP_StartExchKeyTimer(Session, Session->pAkeCoreData, B_DeviceMode_eSink);
    }
#endif
ERR_OUT:
#ifdef DUMP_STREAM
    fclose(fout);
#endif
    return retValue;
}
/*! \brief force to close packet handle, called by source device's exchange key timer expire function.
 *  \param[in] hStreamHandle sream handle.
 *  \retval none.
 */
void B_DTCP_Stream_ClosePacketHandle(struct __b_dtcp_stream_data  *StreamData)
{
    if(StreamData->hPacketHandle != NULL)
    {
        B_DTCP_Content_ClosePacket(StreamData->hPacketHandle);
        StreamData->hPacketHandle = NULL;
    }
}
