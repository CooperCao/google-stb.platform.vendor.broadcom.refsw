/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "parse_userdata.h"
#include "nexus_video_decoder_userdata.h"
#include "budp_dccparse.h"

BDBG_MODULE(parser_userdata);

struct buserdata
{
    buserdata_settings settings;
    unsigned char *buffer;
    bool useUserDataFormat;
    NEXUS_UserDataFormat userDataFormat;
    BUDP_DCCparse_Format currentUserDataFormat;
};

void buserdata_get_default_settings(buserdata_settings *psettings)
{
    BKNI_Memset(psettings, 0, sizeof(*psettings));
    psettings->bufferSize = 2048;
}

buserdata_t buserdata_create(const buserdata_settings *psettings)
{
    buserdata_t handle;
    buserdata_settings default_settings;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    if (!psettings) {
        buserdata_get_default_settings(&default_settings);
        psettings = &default_settings;
    }
    BKNI_Memset(handle, 0, sizeof(*handle));
    handle->settings = *psettings;
    handle->buffer = BKNI_Malloc(psettings->bufferSize);
    return handle;
}

void buserdata_destroy(buserdata_t handle)
{
    BKNI_Free(handle->buffer);
    BKNI_Free(handle);
}

static bool IsEquivalentFormat(NEXUS_UserDataFormat nexus_format, BUDP_DCCparse_Format bvbi_format)
{
    switch (nexus_format) {
    case NEXUS_UserDataFormat_eAtsc53:
        if (bvbi_format == BUDP_DCCparse_Format_ATSC53) { return true; }
        break;
    case NEXUS_UserDataFormat_eScte20:
        if (bvbi_format == BUDP_DCCparse_Format_DVS157) { return true; }
        break;
    case NEXUS_UserDataFormat_eScte21:
        if (bvbi_format == BUDP_DCCparse_Format_DVS053) { return true; }
        break;
    default:
    case NEXUS_UserDataFormat_eAny:
        return true;
        break;
    }
    return false;
}

static int check_format(buserdata_t handle, unsigned format)
{
    NEXUS_UserDataFormat nexusUserDataFormat;

    /* if preferredUserDataFormat is true, wait until that format is seen before using handle->userDataFormat for filtering */
    if (handle->settings.preferredUserDataFormat && !handle->useUserDataFormat) {
        if (IsEquivalentFormat(handle->userDataFormat, format)) {
            handle->useUserDataFormat = true;
        }
    }
    else { /* otherwise, just use handle->userDataFormat */
        handle->useUserDataFormat = true;
    }

    switch (format) {
    case BUDP_DCCparse_Format_ATSC53: nexusUserDataFormat = NEXUS_UserDataFormat_eAtsc53; break;
    case BUDP_DCCparse_Format_DVS157: nexusUserDataFormat = NEXUS_UserDataFormat_eScte20; break;
    case BUDP_DCCparse_Format_DVS053: nexusUserDataFormat = NEXUS_UserDataFormat_eScte21; break;
    default: nexusUserDataFormat = NEXUS_UserDataFormat_eAny; break;
    }

    switch (handle->useUserDataFormat ? handle->userDataFormat
                                            : NEXUS_UserDataFormat_eAny) {
    case NEXUS_UserDataFormat_eScte20:
    case NEXUS_UserDataFormat_eScte21:
    case NEXUS_UserDataFormat_eAtsc53:
        if (nexusUserDataFormat != handle->userDataFormat) {
            return -1;
        }
        break;
    case NEXUS_UserDataFormat_eAny:
    default:
        if (handle->currentUserDataFormat == BUDP_DCCparse_Format_Unknown) {
            /* Set it until the next Start. We never want to send >1 type of userdata to the VEC. */
            handle->currentUserDataFormat = format;
        }
        else if (handle->currentUserDataFormat != format) {
            return -1;
        }
        break;
    }
    return 0;
}

#if NEXUS_HAS_SIMPLE_DECODER
int buserdata_parse(buserdata_t handle, NEXUS_SimpleVideoDecoderHandle videoDecoder, NEXUS_ClosedCaptionData *pData, unsigned totalEntries, unsigned *pNumRead)
{
    unsigned char *bufferPtr;
    size_t size;
    const NEXUS_UserDataHeader *pHeader;
    unsigned total;
    int rc;

    total = *pNumRead = 0;

    rc = NEXUS_SimpleVideoDecoder_ReadUserDataBuffer(videoDecoder, handle->buffer, handle->settings.bufferSize, &size);
    if (rc) return BERR_TRACE(rc);

    /* there may be >1 header in a single read */
    bufferPtr = handle->buffer;
    while (size) {
        unsigned offset = 0;
        pHeader = (const NEXUS_UserDataHeader *)bufferPtr;
        if (size < sizeof(*pHeader) || size < pHeader->blockSize) {
            BDBG_WRN(("unable to process %d bytes", (unsigned)size));
            break;
        }

        /* Sanitize pHeader->payloadSize */
        if (pHeader->payloadSize > size)
            break;

        /* sample code for using UDP to parse userdata in an app */
        while (offset < pHeader->payloadSize) {
            BAVC_USERDATA_info info;
            size_t bytesParsed;
            uint8_t cc_count;
            BUDP_DCCparse_ccdata ccData[32]; /* 32 is hardcoded into UDP */
            unsigned i;

            /* yes, we must translate from magnum -> nexus -> magnum, but this gives the app maximum flexibility */
            BKNI_Memset(&info, 0, sizeof(info));
            info.pUserDataBuffer = (uint8_t*)pHeader + sizeof(*pHeader);
            info.ui32UserDataBufSize = pHeader->payloadSize;

            /* these CASSERT's are in nexus */
            BDBG_CASSERT(BAVC_Polarity_eFrame == (BAVC_Polarity)NEXUS_PicturePolarity_eFrame);
            BDBG_CASSERT(BAVC_USERDATA_Type_eMax == (BAVC_USERDATA_Type)NEXUS_UserDataType_eMax);
            BDBG_CASSERT(BAVC_USERDATA_PictureCoding_eB == (BAVC_PictureCoding)NEXUS_PictureCoding_eB);

            info.eSourcePolarity = pHeader->polarity;
            info.eUserDataType = pHeader->type;
            info.bTopFieldFirst = pHeader->topFieldFirst;
            info.bRepeatFirstField = pHeader->repeatFirstField;
            info.ePicCodingType = pHeader->pictureCoding;
            info.ui32PTS = pHeader->pts;
            info.bPTSValid = pHeader->ptsValid;
            info.ui32PicCodExt[0] = pHeader->pictureCodingExtension[0];
            info.ui32PicCodExt[1] = pHeader->pictureCodingExtension[1];

            cc_count = 0;
            rc = BUDP_DCCparse(&info, offset, &bytesParsed, &cc_count, ccData);
            if (cc_count == 0) rc = BUDP_DCCparse_SEI(&info, offset, &bytesParsed, &cc_count, ccData);
            if (rc) cc_count = 0;
            for (i=0;i<cc_count && total<totalEntries;i++) {
                if (ccData[i].bIsAnalog) {
                    if (ccData[i].cc_valid) {
                        if (check_format(handle, ccData[i].format)) {
                            continue;
                        }
                        /* EIA-608 */
                        pData[total].field = (ccData[i].polarity==BAVC_Polarity_eTopField)?0:1;
                        pData[total].data[0] = ccData[i].cc_data_1;
                        pData[total].data[1] = ccData[i].cc_data_2;
                        /* SW7364-348, need to clear noData flag, otherwise residual 708 data could mess 608 data */
                        pData[total].noData = 0;
                        total++;
                    }
                    /* don't send invalid 608 data */
                }
                else {
                    /* EIA-708 */
                    pData[total].field = ccData[i].seq.cc_type; /* can be any one of 0,1,2 or 3 */
                    pData[total].data[0] = ccData[i].cc_data_1;
                    pData[total].data[1] = ccData[i].cc_data_2;
                    pData[total].noData = !ccData[i].cc_valid; /* must still send invalid 708 data */
                    total++;
                }
            }
            if (i<cc_count) {
                BDBG_WRN(("dropped %d entries of cc data", cc_count - i));
            }

            offset += bytesParsed;
        }

        /* advance pointer and decrease size */
        size -= pHeader->blockSize;
        bufferPtr = (uint8_t*)bufferPtr + pHeader->blockSize;
    }

    *pNumRead = total;

    return 0;
}
#else
int buserdata_parse(buserdata_t handle, NEXUS_SimpleVideoDecoderHandle videoDecoder, NEXUS_ClosedCaptionData *pData, unsigned totalEntries, unsigned *pNumRead)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(videoDecoder);
    BSTD_UNUSED(pData);
    BSTD_UNUSED(totalEntries);
    BSTD_UNUSED(pNumRead);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
#endif
