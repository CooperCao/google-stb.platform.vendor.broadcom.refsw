/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "budp_jp3dparse.h"
#if B_REFSW_DSS_SUPPORT
#include "budp_dccparse_dss.h"
#endif

BDBG_MODULE(nexus_video_decoder_userdata);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

NEXUS_Error NEXUS_VideoDecoder_GetUserDataBuffer(NEXUS_VideoDecoderHandle videoDecoder, void **pBuffer, unsigned *pSize)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (!videoDecoder->openSettings.openSettings.userDataBufferSize) {
        BDBG_ERR(("You must specify NEXUS_VideoDecoderOpenSettings.userDataBufferSize to use this feature."));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BKNI_EnterCriticalSection();
    *pBuffer = &videoDecoder->userdata.buf[videoDecoder->userdata.rptr];
    if (videoDecoder->userdata.rptr <= videoDecoder->userdata.wptr) {
        *pSize = videoDecoder->userdata.wptr - videoDecoder->userdata.rptr;
    }
    else if (videoDecoder->userdata.wrap_ptr) {
        BDBG_ASSERT(videoDecoder->userdata.wrap_ptr > videoDecoder->userdata.rptr);
        *pSize = videoDecoder->userdata.wrap_ptr - videoDecoder->userdata.rptr;
    }
    else {
        *pSize = videoDecoder->openSettings.openSettings.userDataBufferSize - videoDecoder->userdata.rptr;
    }
    BKNI_LeaveCriticalSection();

    videoDecoder->userdata.lastGetBufferSize = *pSize;
    return 0;
}

void NEXUS_VideoDecoder_UserDataReadComplete(NEXUS_VideoDecoderHandle videoDecoder, unsigned size)
{
    unsigned userDataBufferSize;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    userDataBufferSize = videoDecoder->openSettings.openSettings.userDataBufferSize;
    if (size > videoDecoder->userdata.lastGetBufferSize) {
        BDBG_ERR(("invalid UserDataReadComplete %d > %d", size, videoDecoder->userdata.lastGetBufferSize));
        /* flush so that app has chance of recovery */
        NEXUS_VideoDecoder_FlushUserData(videoDecoder);
        return;
    }
    videoDecoder->userdata.lastGetBufferSize = 0;
    BKNI_EnterCriticalSection();
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_UserDataReadComplete %d, %d %d %d, %d", size, videoDecoder->userdata.rptr, videoDecoder->userdata.wptr, videoDecoder->userdata.wrap_ptr, userDataBufferSize));
    videoDecoder->userdata.rptr += size;
    if (videoDecoder->userdata.rptr >= userDataBufferSize || (videoDecoder->userdata.wrap_ptr && (videoDecoder->userdata.rptr >= videoDecoder->userdata.wrap_ptr))) {
        if (videoDecoder->userdata.rptr > userDataBufferSize) {
            /* this can't be, so ERR and discard data. */
            BDBG_ERR(("invalid NEXUS_VideoDecoder_UserDataReadComplete size %d (rptr %d %d)", size, videoDecoder->userdata.rptr, userDataBufferSize));
            videoDecoder->userdata.wptr = 0; /* wipe everything out to increase chances of recovery */
        }
        if (videoDecoder->userdata.wrap_ptr && videoDecoder->userdata.rptr > videoDecoder->userdata.wrap_ptr) {
            /* this can't be, so ERR and discard data. */
            BDBG_ERR(("invalid NEXUS_VideoDecoder_UserDataReadComplete size %d (wrap_ptr %d %d)", size, videoDecoder->userdata.rptr, videoDecoder->userdata.wrap_ptr));
            videoDecoder->userdata.wptr = 0; /* wipe everything out to increase chances of recovery */
        }
        videoDecoder->userdata.rptr = 0;
        videoDecoder->userdata.wrap_ptr = 0;
    }
    BDBG_ASSERT(!userDataBufferSize || videoDecoder->userdata.rptr < userDataBufferSize);
    BKNI_LeaveCriticalSection();
}

static void NEXUS_VideoDecoder_P_UserDataFreeSpace_isr(NEXUS_VideoDecoderHandle videoDecoder, unsigned *immediate_space, unsigned *after_wrap_space)
{
    if (!videoDecoder->openSettings.openSettings.userDataBufferSize) {
        *immediate_space = 0;
        *after_wrap_space = 0;
        return;
    }
    if (videoDecoder->userdata.rptr > videoDecoder->userdata.wptr) {
        *immediate_space = videoDecoder->userdata.rptr - videoDecoder->userdata.wptr - 1;
        *after_wrap_space = 0;
    }
    else {
        if (videoDecoder->userdata.rptr) {
            *immediate_space = videoDecoder->openSettings.openSettings.userDataBufferSize - videoDecoder->userdata.wptr;
            *after_wrap_space = videoDecoder->userdata.rptr - 1;
        }
        else {
            *immediate_space = videoDecoder->openSettings.openSettings.userDataBufferSize - videoDecoder->userdata.wptr - 1;
            *after_wrap_space = 0;
        }
    }
    BDBG_ASSERT(*after_wrap_space+*immediate_space < videoDecoder->openSettings.openSettings.userDataBufferSize);
}

static void NEXUS_VideoDecoder_P_AddUserData_isr(NEXUS_VideoDecoderHandle videoDecoder, void *buffer, unsigned size)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (!size) return;
    if (buffer) {
        unsigned userDataBufferSize = videoDecoder->openSettings.openSettings.userDataBufferSize;

        BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_AddUserData_isr %d, %d %d %d, %d", size, videoDecoder->userdata.rptr, videoDecoder->userdata.wptr, videoDecoder->userdata.wrap_ptr, userDataBufferSize));
        BKNI_Memcpy(&videoDecoder->userdata.buf[videoDecoder->userdata.wptr], buffer, size);
        videoDecoder->userdata.wptr += size;
        if (videoDecoder->userdata.wptr == userDataBufferSize) {
            videoDecoder->userdata.wptr = 0;
        }
        else {
            BDBG_ASSERT(videoDecoder->userdata.wptr <= userDataBufferSize);
        }
    }
    else {
        /* skip the remainder */
        BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_AddUserData_isr skip %d %d %d, %d", videoDecoder->userdata.rptr, videoDecoder->userdata.wptr, videoDecoder->userdata.wrap_ptr, videoDecoder->openSettings.openSettings.userDataBufferSize));
        videoDecoder->userdata.wrap_ptr = videoDecoder->userdata.wptr;
        videoDecoder->userdata.wptr = 0;
        if (videoDecoder->userdata.wrap_ptr == videoDecoder->userdata.rptr) {
            videoDecoder->userdata.rptr = 0;
            videoDecoder->userdata.wrap_ptr = 0;
        }
    }
}

void NEXUS_VideoDecoder_FlushUserData(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BKNI_EnterCriticalSection();
    videoDecoder->userdata.rptr = videoDecoder->userdata.wptr = videoDecoder->userdata.wrap_ptr = 0;
    BKNI_Memset(&videoDecoder->userdata.status, 0, sizeof(videoDecoder->userdata.status));
    BKNI_LeaveCriticalSection();
    videoDecoder->userdata.lastGetBufferSize = 0;
}

static bool IsEquivalentFormat_isr(NEXUS_UserDataFormat nexus_format, BUDP_DCCparse_Format budp_format)
{
    switch (nexus_format) {
    case NEXUS_UserDataFormat_eAtsc53:
        if (budp_format == BUDP_DCCparse_Format_ATSC53) { return true; }
        break;
    case NEXUS_UserDataFormat_eScte20:
        if (budp_format == BUDP_DCCparse_Format_DVS157) { return true; }
        break;
    case NEXUS_UserDataFormat_eScte21:
        if (budp_format == BUDP_DCCparse_Format_DVS053) { return true; }
        break;
    case NEXUS_UserDataFormat_eAtsc72:
        if (budp_format == BUDP_DCCparse_Format_SEI) { return true; }
        break;
    default:
    case NEXUS_UserDataFormat_eAny:
        return true;
        break;
    }
    return false;
}

static void NEXUS_VideoDecoder_P_ParseUserdata_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_USERDATA_info *info)
{
    uint32_t offset = 0;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_MSG(("ParseUserdata %d:%d:%p:%d:%d", info->eUserDataType, info->ui32UserDataBufSize,
        (void*)info->pUserDataBuffer, info->bTopFieldFirst, info->bRepeatFirstField));

    while (offset < info->ui32UserDataBufSize) {
        BERR_Code rc;
        unsigned i;
        size_t bytesParsed = 0;
        uint8_t ccCount = 0;
#if B_REFSW_DSS_SUPPORT
        uint8_t dssCcCount = 0;
        uint8_t dssSubtitleCount = 0;
#endif
        /* DCCparse data is large enough to overflow the stack, so it is stored in the handle's storage */
        BUDP_DCCparse_ccdata *ccData = videoDecoder->udpData.ccData;
#if B_REFSW_DSS_SUPPORT
        BUDP_DCCparse_dss_cc_subtitle *dssCcData = videoDecoder->udpData.dssCcData;
        BUDP_DCCparse_dss_cc_subtitle *dssSubtitle = videoDecoder->udpData.dssSubtitle;
#endif

        if (videoDecoder->startSettings.codec == NEXUS_VideoCodec_eH264 || videoDecoder->startSettings.codec == NEXUS_VideoCodec_eH265) {
            rc = BUDP_DCCparse_SEI_isr(info, offset, &bytesParsed, &ccCount, ccData);
        }
#if B_REFSW_DSS_SUPPORT
        else if (videoDecoder->transportType == NEXUS_TransportType_eDssEs) {
            /* BUDP_DCCparse_DSS_isr is only DSS SD. for DSS HD uses BUDP_DCCparse_isr.  */
            rc = BUDP_DCCparse_DSS_isr(info, offset, &bytesParsed, &dssCcCount, dssCcData, &dssSubtitleCount, dssSubtitle);
        }
#endif
        else {
            rc = BUDP_DCCparse_isr(info, offset, &bytesParsed, &ccCount, ccData);
#if B_REFSW_DSS_SUPPORT
            if (rc == BERR_BUDP_NO_DATA) {
                /* some TS streams have DSS userdata */
                BERR_Code rc2 = BUDP_DCCparse_DSS_isr(info, offset, &bytesParsed, &dssCcCount, dssCcData, &dssSubtitleCount, dssSubtitle);
                if (rc2 == BERR_SUCCESS) {
                    rc = rc2;
                }
            }
#endif

            /* check for MPEG2 userdata 3D signaling */
            if (videoDecoder->startSettings.codec == NEXUS_VideoCodec_eMpeg2) {
                if (rc == BERR_BUDP_NO_DATA && (offset+bytesParsed < info->ui32UserDataBufSize)) { /* bytesParsed can be non-zero even if rc!=0 */
                    rc = BUDP_JP3Dstart_isr(info, offset, &bytesParsed);
#if 0 /* for debug */
                    for (i=offset; i<offset+bytesParsed; i++) {
                        BKNI_Printf("%02x ", *((uint8_t*)(info->pUserDataBuffer)+i)); /* it should be 0x000001B2 0x4A503344 */
                    }
                    BKNI_Printf("\n");
#endif

                    if (rc == BERR_SUCCESS) {
                        while (1) {
                            uint16_t type;
                            offset += bytesParsed;
                            rc = BUDP_JP3Dparse_isr(info, offset, &bytesParsed, &type);
#if 0 /* for debug */
                            for (i=offset; i<offset+bytesParsed; i++) {
                                BKNI_Printf("%02x ", *((uint8_t*)(info->pUserDataBuffer)+i)); /* it should be 0x038?04FF, where ? is the 3D-orientation  */
                            }
                            BKNI_Printf("\n");
#endif

                            if (rc || !bytesParsed) break;
                            NEXUS_VideoDecoder_P_Jp3dSignal_isr(videoDecoder, type);
                        }
                    }
                    else {
                        offset += bytesParsed;
                    }
                    continue; /* no more userdata to consume */
                }
            }
        }
        if (bytesParsed==0) { /* we aren't going anywhere */
            break;
        }
        offset += bytesParsed;
        /* We process bytesParsed even with error code. seems a bit dangerous. */
        if (rc == BERR_BUDP_PARSE_ERROR) {
            break;
        }
        else if (rc != BERR_SUCCESS) {
            continue;
        }

        /* UDPlib takes pointer w/o size, so this must be true. Otherwise we have overflow, from which there is no recovery. */
        BDBG_ASSERT(ccCount <= B_MAX_VBI_CC_COUNT);
#if B_REFSW_DSS_SUPPORT
        BDBG_ASSERT(dssCcCount <= B_MAX_VBI_CC_COUNT);
        BDBG_ASSERT(dssSubtitleCount <= B_MAX_VBI_CC_COUNT);
#endif

        if (videoDecoder->trickState.hostTrickModesEnabled || videoDecoder->trickState.brcmTrickModesEnabled) {
            /* in a host trick mode, we parse 3DTV signaling, but not closed caption.
            This is relatively harmless in a race condition, so don't propagate a
            critical section on 'trickState' throughout. */
            continue;
        }

        for (i=0;i<ccCount;i++) {
            NEXUS_UserDataFormat nexusUserDataFormat;

            /* if preferredUserDataFormat is true, wait until that format is seen before using videoDecoder->userDataFormat for filtering */
            if (videoDecoder->settings.preferredUserDataFormat && !videoDecoder->useUserDataFormat) {
                if (IsEquivalentFormat_isr(videoDecoder->userDataFormat, ccData[i].format)) {
                    videoDecoder->useUserDataFormat = true;
                }
            }
            else { /* otherwise, just use videoDecoder->userDataFormat */
                videoDecoder->useUserDataFormat = true;
            }

            switch (ccData[i].format) {
            case BUDP_DCCparse_Format_ATSC53: nexusUserDataFormat = NEXUS_UserDataFormat_eAtsc53; break;
            case BUDP_DCCparse_Format_DVS157: nexusUserDataFormat = NEXUS_UserDataFormat_eScte20; break;
            case BUDP_DCCparse_Format_DVS053: nexusUserDataFormat = NEXUS_UserDataFormat_eScte21; break;
            case BUDP_DCCparse_Format_SEI: nexusUserDataFormat = NEXUS_UserDataFormat_eAtsc72; break;
            default: nexusUserDataFormat = NEXUS_UserDataFormat_eAny; break;
            }

            videoDecoder->userdata.status.formatParsed[nexusUserDataFormat]++;
            videoDecoder->userdata.status.lastFormatParsed = nexusUserDataFormat;

            switch (videoDecoder->useUserDataFormat ? videoDecoder->userDataFormat
                                                    : NEXUS_UserDataFormat_eAny) {
            case NEXUS_UserDataFormat_eScte20:
            case NEXUS_UserDataFormat_eScte21:
            case NEXUS_UserDataFormat_eAtsc53:
            case NEXUS_UserDataFormat_eAtsc72:
                if (nexusUserDataFormat != videoDecoder->userDataFormat) {
                    continue;
                }
                break;
            case NEXUS_UserDataFormat_eAny:
            default:
                if (videoDecoder->currentUserDataFormat == BUDP_DCCparse_Format_Unknown) {
                    /* Set it until the next Start. We never want to send >1 type of userdata to the VEC. */
                    videoDecoder->currentUserDataFormat = ccData[i].format;
                }
                else if (videoDecoder->currentUserDataFormat != ccData[i].format) {
                    if (++videoDecoder->userdataAnyFilterCnt == videoDecoder->settings.userDataFilterThreshold) {
                        /* after seeing some number of entries of another format, and nothing from currentUserDataFormat, reset the filter */
                        BDBG_WRN(("decoder %p: switching userdata filter from %d to %d", (void *)videoDecoder, videoDecoder->currentUserDataFormat, ccData[i].format));
                        videoDecoder->currentUserDataFormat = ccData[i].format;
                    }
                    else {
                        continue;
                    }
                }
                videoDecoder->userdataAnyFilterCnt = 0;
                break;
            }
            if (videoDecoder->userdata.vbiDataCallback_isr) {
                NEXUS_ClosedCaptionData data;

                BKNI_Memset(&data, 0, sizeof(data));
                data.type = NEXUS_VbiDataType_eClosedCaption;
                if (info->bPTSValid) {
                    data.pts = info->ui32PTS;
                }

                if (ccData[i].bIsAnalog) {
                    if (ccData[i].cc_valid) {
                        /* EIA-608 */
                        data.field = (ccData[i].polarity==BAVC_Polarity_eTopField)?0:1;
                        data.data[0] = ccData[i].cc_data_1;
                        data.data[1] = ccData[i].cc_data_2;
                        (*videoDecoder->userdata.vbiDataCallback_isr)(
                            videoDecoder->userdata.vbiVideoInput, true, &data);
                    }
                    /* don't send invalid 608 data */
                }
                else {
                    /* EIA-708 */
                    data.field = ccData[i].seq.cc_type; /* can be any one of 0,1,2 or 3 */
                    data.data[0] = ccData[i].cc_data_1;
                    data.data[1] = ccData[i].cc_data_2;
                    data.noData = !ccData[i].cc_valid; /* must still send invalid 708 data */
                    (*videoDecoder->userdata.vbiDataCallback_isr)(videoDecoder->userdata.vbiVideoInput, false, &data);
                }
            }
        }

#if B_REFSW_DSS_SUPPORT
        /* DSS CC & subtitle has special processing */
        if (videoDecoder->userdata.vbiDataCallback_isr) {
            NEXUS_ClosedCaptionData data;

            for (i=0;i<dssCcCount;i++) {
                if (dssCcData[i].eCCType == BUDP_DCCparse_CC_Dss_Type_ClosedCaption) {
                    BKNI_Memset(&data, 0, sizeof(data));
                    data.type = NEXUS_VbiDataType_eClosedCaption;
                    if (info->bPTSValid) {
                        data.pts = info->ui32PTS;
                    }

                    if (dssCcData[i].bIsAnalog) {
                        /* EIA-608 */
                        data.field = (dssCcData[i].polarity==BAVC_Polarity_eTopField)?0:1;
                        data.data[0] = dssCcData[i].cc_data_1;
                        data.data[1] = dssCcData[i].cc_data_2;
                        (*videoDecoder->userdata.vbiDataCallback_isr)(
                            videoDecoder->userdata.vbiVideoInput, true /*608*/,
                            &data);
                    }
                    else {
                        /* EIA-708 */
                        data.field = dssCcData[i].language_type; /* this is cc_type */
                        data.data[0] = dssCcData[i].cc_data_1;
                        data.data[1] = dssCcData[i].cc_data_2;
                        (*videoDecoder->userdata.vbiDataCallback_isr)(videoDecoder->userdata.vbiVideoInput, false /*708*/, &data);
                    }
                }
            }
            for (i=0;i<dssSubtitleCount;i++) {
                if (dssSubtitle[i].eCCType == BUDP_DCCparse_CC_Dss_Type_Subtitle) {
                    BKNI_Memset(&data, 0, sizeof(data));
                    data.type = NEXUS_VbiDataType_eSubtitle;
                    if (info->bPTSValid) {
                        data.pts = info->ui32PTS;
                    }

                    data.field = dssSubtitle[i].language_type;
                    data.data[0] = dssSubtitle[i].cc_data_1;
                    data.data[1] = dssSubtitle[i].cc_data_2;
                    (*videoDecoder->userdata.vbiDataCallback_isr)(videoDecoder->userdata.vbiVideoInput, false /*not 608*/, &data);
                }
            }
        }
#endif

    }
    return;
}

NEXUS_PicturePolarity NEXUS_P_PicturePolarity_FromMagnum_isrsafe(BAVC_Polarity polarity)
{
    switch (polarity) {
    case BAVC_Polarity_eTopField: return NEXUS_PicturePolarity_eTopField;
    case BAVC_Polarity_eBotField: return NEXUS_PicturePolarity_eBottomField;
    default:
    case BAVC_Polarity_eFrame:    return NEXUS_PicturePolarity_eFrame;
    }
}

void NEXUS_VideoDecoder_P_UserdataReady_isr(void *data, int unused, void *not_used)
{
    BERR_Code rc = BXVD_ERR_USERDATA_NONE;
    BAVC_USERDATA_info info;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)data;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BSTD_UNUSED(unused);
    BSTD_UNUSED(not_used);

    for ( ;; )
    {
        /* TODO: reduce XVD userdata buffer size because we evacuate XVD at isr time */

        /* get data */
        rc = BXVD_Userdata_Read_isr(videoDecoder->userdata.handle, &info);
        if ( rc==BXVD_ERR_USERDATA_NONE ) {
            break;
        }
        else if ( rc==BXVD_ERR_USERDATA_INVALID ) {
            continue; /* keep reading data */
        }
        else if ( rc!=BERR_SUCCESS ) {
            BDBG_ERR(("BXVD_Userdata_Read_isr returned error %#x, ignore", rc));
            break;
        }

        if (videoDecoder->settings.userDataEnabled) {
            unsigned pad;
            NEXUS_UserDataHeader header;
            unsigned after_wrap_space, immediate_space, needed_space;

            /* add padding to payload to ensure efficient 32 bit aligned access of the header */
            pad = 4 - ((sizeof(header) + info.ui32UserDataBufSize) % 4);
            if (pad == 4) pad = 0;

            BDBG_CASSERT(BAVC_USERDATA_Type_eMax == (BAVC_USERDATA_Type)NEXUS_UserDataType_eMax);
            BDBG_CASSERT(BAVC_USERDATA_PictureCoding_eB == (BAVC_PictureCoding)NEXUS_PictureCoding_eB);

            header.blockSize = sizeof(header) + info.ui32UserDataBufSize + pad;
            header.polarity = NEXUS_P_PicturePolarity_FromMagnum_isrsafe(info.eSourcePolarity);
            header.type = info.eUserDataType;
            header.topFieldFirst = info.bTopFieldFirst;
            header.repeatFirstField = info.bRepeatFirstField;
            header.ptsValid = info.bPTSValid;
            header.pts = info.ui32PTS;
            header.pictureCoding = info.ePicCodingType;
            BKNI_Memcpy(header.pictureCodingExtension, info.ui32PicCodExt, sizeof(header.pictureCodingExtension));
            header.payloadSize = info.ui32UserDataBufSize;

            /* userdata packets should be copied all or nothing. don't allow overflow to corrupt the header + payload format. */
            NEXUS_VideoDecoder_P_UserDataFreeSpace_isr(videoDecoder, &immediate_space, &after_wrap_space);
            needed_space = sizeof(header)+info.ui32UserDataBufSize+pad;
            if (needed_space <= immediate_space || needed_space <= after_wrap_space) {
                char padbytes[4] = {0,0,0,0};
                if (needed_space > immediate_space) {
                    /* skip the remaining space in the ring buffer so that header + payload + pad never wraps. */
                    NEXUS_VideoDecoder_P_AddUserData_isr(videoDecoder, NULL, immediate_space);
                }
                NEXUS_VideoDecoder_P_AddUserData_isr(videoDecoder, &header, sizeof(header));
                NEXUS_VideoDecoder_P_AddUserData_isr(videoDecoder, info.pUserDataBuffer, info.ui32UserDataBufSize);
                NEXUS_VideoDecoder_P_AddUserData_isr(videoDecoder, padbytes, pad);
                NEXUS_IsrCallback_Fire_isr(videoDecoder->userdataCallback);
            }
            else {
                /* do not write partial data. just discard. */
                BDBG_WRN(("userdata overflow"));
            }
        }

        /* parse and send data to VideoInput (in Display module) */
        if (videoDecoder->userdata.vbiDataCallback_isr || videoDecoder->extendedSettings.s3DTVStatusEnabled) {
            NEXUS_VideoDecoder_P_ParseUserdata_isr(videoDecoder, &info);
        }

        if (videoDecoder->displayConnection.userDataCallback_isr) {
            videoDecoder->displayConnection.userDataCallback_isr(videoDecoder->displayConnection.callbackContext, &info);
        }
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetUserDataFormatFilter( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_UserDataFormat format )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    videoDecoder->userDataFormat = format;
    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_GetUserDataStatus( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderUserDataStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pStatus = videoDecoder->userdata.status;
    return 0;
}
