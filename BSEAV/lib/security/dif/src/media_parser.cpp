/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#include <stdio.h>
#include <string.h>
#include "nexus_platform.h"
#include "bfile_stdio.h"
#include "media_parser.h"

#define BMP4_ISML BMP4_TYPE('i','s','m','l')
#define BMP4_DASH BMP4_TYPE('d','a','s','h')
#define BMP4_PIFF BMP4_TYPE('p','i','f','f')
#define BMP4_ISOM BMP4_TYPE('i','s','o','m')

BDBG_MODULE(media_parser);
#include "dump_hex.h"

using namespace media_parser;

const uint8_t kWidevineUUID[] = {0xed, 0xef, 0x8b, 0xa9, 0x79, 0xd6, 0x4a, 0xce, 0xa3, 0xc8, 0x27, 0xdc, 0xd5, 0x1d, 0x21, 0xed};
const uint8_t kPlayreadyUUID[] = {0x9A, 0x04, 0xF0, 0x79, 0x98, 0x40, 0x42, 0x86, 0xAB, 0x92, 0xE6, 0x5B, 0xE0, 0x88, 0x5F, 0x95};

const int kBufSize = (1024 * 1024 * 2);      /* 2MB */

MediaParser::MediaParser(FILE* filePtr):m_drmSchemeIndex(0),
                            m_psshLen(0),
                            m_psshDataStr(""),
                            m_filePtr(filePtr),
                            m_handle(NULL),
                            m_mediaType(media_type_eUnknown),
                            m_drmType(drm_type_eUnknown),
                            m_numOfDrmSchemes(0)
{}


MediaParser::~MediaParser()
{
    if (m_handle) mp4_parser_destroy(m_handle);
    bfile_stdio_read_detach(m_fd);
}



bool MediaParser::InitParser()
{
    LOGD(("%s: enter", BSTD_FUNCTION));

    if(m_filePtr == NULL){
        LOGE(("Invalid input file pointer"));
        return false;
    }

    m_fd = bfile_stdio_read_attach(m_filePtr);
    if(m_fd == NULL){
        LOGE(("Invalid input file handle"));
        return false;
    }

    LOGD(("Create media parser"));
    m_handle = mp4_parser_create(m_fd);
    if (!m_handle) {
        LOGE(("Unable to create media parser context"));
        return false;
    }

    /* now go back to the begining*/
    rewind(m_filePtr);

    LOGD(("Parse file type first"));
    bool fileTypeParsed = mp4_parser_scan_file_type(m_handle);

    if(!fileTypeParsed) {
        LOGE(("Failed to parse file type, can't continue..."));
        return false;
    }

    struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;
    if (BMP4_ISML == parser_context->filetype.major_brand){
        m_mediaType = media_type_ePiff;
        LOGD(("PIFF(isml) file type is detected"));
    } else if(BMP4_PIFF == parser_context->filetype.major_brand) {
        m_mediaType = media_type_ePiff;
        LOGD(("PIFF(isml) file type is detected"));
    } else if (BMP4_DASH == parser_context->filetype.major_brand) {
        m_mediaType = media_type_eCenc;
        LOGD(("CENC(dash) detected in major brand"));
    } else if (BMP4_ISOM == parser_context->filetype.major_brand) {
        m_mediaType = media_type_eCenc;
        LOGD(("CENC(dash) detected in major brand"));
    } else {
        for (unsigned i = 0; i < parser_context->filetype.ncompatible_brands; i++) {
            if (BMP4_ISML == parser_context->filetype.compatible_brands[i]) {
                m_mediaType = media_type_ePiff;
                LOGD(("PIFF(isml) detected in compatible brands"));
                break;
            }
            if (BMP4_PIFF == parser_context->filetype.compatible_brands[i]) {
                m_mediaType = media_type_ePiff;
                LOGD(("PIFF(isml) detected in compatible brands"));
                break;
            }
            if (BMP4_DASH == parser_context->filetype.compatible_brands[i]) {
                m_mediaType = media_type_eCenc;
                LOGD(("CENC(dash) detected in compatible brands"));
                break;
            }
        }
        if (m_mediaType == media_type_eUnknown) {
            LOGE(("Unknown file type detected:" B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(parser_context->filetype.major_brand)));
            return false;
        }
    }

    LOGD(("Parse movie information"));
    bool moovBoxParsed = mp4_parser_scan_movie_info(m_handle);

    if(!moovBoxParsed) {
        LOGE(("Failed to parse moov box, can't continue..."));
        return false;
    }

    LOGD(("Obtaining pssh"));
    bmp4_mp4_headers *mp4_header = &parser_context->mp4_mp4;
    m_numOfDrmSchemes = mp4_header->numOfDrmSchemes;
    if(m_numOfDrmSchemes < 1){
        LOGW(("No pssh found in the stream"));
    }

    m_psshData = mp4_parser_get_pssh(m_handle, &m_psshLen, 0); /*get first pssh by default*/

    if (!m_psshData)
        LOGW(("Failed to obtain pssh data - should be clear video"));
    else
        LOGD(("m_psshLen=%d", m_psshLen));

    if (m_psshLen) {
        m_psshDataStr = std::string(m_psshLen, (char)0);
        BKNI_Memcpy((void*)m_psshDataStr.data(), m_psshData, m_psshLen);
        dump_hex("pssh in the file", m_psshDataStr.data(), m_psshDataStr.size());

        LOGD(("Detecting system ID"));
        std::string system_id;
        system_id.resize(BMP4_UUID_LENGTH);
        BKNI_Memcpy((void*)system_id.data(),
            parser_context->mp4_mp4.psshSystemId[0].systemId.data,
            BMP4_UUID_LENGTH); /*fetch first systemID by default*/

        if (memcmp(system_id.data(), kWidevineUUID, sizeof(kWidevineUUID)) == 0) {
#ifdef ENABLE_WIDEVINE_3x
            m_drmType = drm_type_eWidevine3x;
#else
            m_drmType = drm_type_eWidevine;
#endif
            LOGD(("Widevine System Id was detected"));
        } else if (memcmp(system_id.data(), kPlayreadyUUID, sizeof(kWidevineUUID)) == 0) {

            m_drmType = drm_type_ePlayready;
            LOGD(("Playready System Id was detected"));
        } else {
            dump_hex("Unknown System ID is detected", system_id.data(), system_id.size(), true);
            return false;
        }
    }

    /* go back to the begining */
    rewind(m_filePtr);

    return true;
}

uint8_t MediaParser::GetNumOfDrmSchemes(DrmType drmTypes[], uint8_t arySize) const
{
    if (m_numOfDrmSchemes == 0) {
        drmTypes[0] = drm_type_eUnknown;
        return 0;
    }

    struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;
    std::string system_id;
    system_id.resize(BMP4_UUID_LENGTH);

    for(int i=0; i< m_numOfDrmSchemes && i< arySize; i++ ){
        BKNI_Memcpy((void*)system_id.data(),
            parser_context->mp4_mp4.psshSystemId[i].systemId.data, BMP4_UUID_LENGTH);
        if (memcmp(system_id.data(), kWidevineUUID, sizeof(kWidevineUUID)) == 0) {
#ifdef ENABLE_WIDEVINE_3x
            drmTypes[i] = drm_type_eWidevine3x;
#else
            drmTypes[i] = drm_type_eWidevine;
#endif
            LOGD(("Widevine System Id was detected"));
        } else if (memcmp(system_id.data(), kPlayreadyUUID, sizeof(kWidevineUUID)) == 0) {
            drmTypes[i] = drm_type_ePlayready;
            LOGD(("Playready System Id was detected"));
        } else {
            drmTypes[i] = drm_type_eUnknown;
            LOGE(("Unknown System ID is detected"));
        }
    }
    return m_numOfDrmSchemes;
}

bool MediaParser::SetDrmSchemes(uint8_t index)
{
    if(index >= m_numOfDrmSchemes){
        LOGE(("Wrong DRM scheme index: %d", index));
        return false;
    }else if(m_drmSchemeIndex == index){
        return true;
    }else{
        struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;

        LOGD(("Obtaining No. %d pssh", index));
        m_psshData = mp4_parser_get_pssh(m_handle, &m_psshLen, index);

        if (!m_psshData)
            LOGW(("Failed to obtain pssh data - should be clear video"));
        else
            LOGD(("m_psshLen=%d", m_psshLen));

        m_psshDataStr = std::string(m_psshLen, (char)0);
        BKNI_Memcpy((void*)m_psshDataStr.data(), m_psshData, m_psshLen);
        dump_hex("pssh in the file", m_psshDataStr.data(), m_psshDataStr.size());


        LOGD(("Detecting No. %d system ID", index));
        std::string system_id;
        system_id.resize(BMP4_UUID_LENGTH);
        BKNI_Memcpy((void*)system_id.data(),

        parser_context->mp4_mp4.psshSystemId[index].systemId.data, BMP4_UUID_LENGTH);

        if (memcmp(system_id.data(), kWidevineUUID, sizeof(kWidevineUUID)) == 0) {
#ifdef ENABLE_WIDEVINE_3x
            m_drmType = drm_type_eWidevine3x;
#else
            m_drmType = drm_type_eWidevine;
#endif
                LOGD(("Widevine System Id was detected"));
        } else if (memcmp(system_id.data(), kPlayreadyUUID, sizeof(kWidevineUUID)) == 0) {

            m_drmType = drm_type_ePlayready;
            LOGD(("Playready System Id was detected"));
        } else {
            dump_hex("Unknown System ID is detected", system_id.data(), system_id.size(), true);
            return false;
        }

        m_drmSchemeIndex = index;
        return true;
    }
}

void* MediaParser::GetFragmentData(mp4_parse_frag_info &fragInfo, uint8_t *pPayload, uint32_t &decoderLength)
{
    if(feof(m_filePtr)){
        LOGW(("Reached EOF"));
        return NULL;
    }

    if(!pPayload){
        LOGW(("Invaild app.pPayload"));
        return NULL;
    }

    if (!mp4_parser_scan_movie_fragment(m_handle, &fragInfo, pPayload, kBufSize, m_mediaType)) {
        if (feof(m_filePtr)) {
            LOGW(("Reached EOF"));
        } else {
            LOGE(("Unable to parse movie fragment"));
        }
        return NULL;
    }

    return  mp4_parser_get_dec_data(m_handle, &decoderLength, fragInfo.trackId);
}

bool MediaParser::GetProtectionInfo(bmp4_protection_info &protectionInfo)
{
    struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;
    uint32_t index;

	if (m_handle == NULL)
	{
        LOGE(("Invalid Parser Handle."));
        return false;
	}
	memset(&protectionInfo, 0, sizeof(bmp4_protection_info));

    bmp4_mp4_headers *mp4_header = &parser_context->mp4_mp4;

    protectionInfo.nbOfSchemes = mp4_header->nbOfSchemes;

    for (index = 0; index < protectionInfo.nbOfSchemes; index++) {
        bmp4_protectionSchemeInfo   *pProtectionInfo = &protectionInfo.schemeProtectionInfo[index];

        /* **FixMe** compensate for fact that code in bmp.c is using a 1-based index into a 0-based array
         *           when initializing and accessing the pool of protection schemes for the tracks.
         */
        BKNI_Memcpy(pProtectionInfo, &mp4_header->scheme[index+1], sizeof(bmp4_protectionSchemeInfo));
    }

    if (protectionInfo.nbOfSchemes == 0) {
        LOGD(("%s: Did not find any valid protection schemes.", BSTD_FUNCTION));
        return false;
    }

    return true;
}

bool MediaParser::GetProtectionInfoForTrack(bmp4_protectionSchemeInfo &trackProtectionInfo, uint32_t inTrackId)
{
    struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;
    uint32_t index;

	if (m_handle == NULL)
	{
        LOGE(("Invalid Parser Handle."));
        return false;
	}
	memset(&trackProtectionInfo, 0, sizeof(bmp4_protectionSchemeInfo));

    bmp4_mp4_headers *mp4_header = &parser_context->mp4_mp4;

    for (index = 0; index < mp4_header->nbOfSchemes; index++) {
        /* **FixMe** compensate for fact that code in bmp.c is using a 1-based index into a 0-based array
         *           when initializing and accessing the pool of protection schemes for the tracks.
         */
        bmp4_protectionSchemeInfo   *pScheme = &mp4_header->scheme[index+1];
        if (pScheme == NULL) {
            LOGE(("%s: Failed to find a valid Protection Scheme object at index %d.", BSTD_FUNCTION, index+1));
            return false;
        }
        if (inTrackId == pScheme->trackId) {
            /* found a protection scheme for requested trackId */
            BKNI_Memcpy(&trackProtectionInfo, pScheme, sizeof(bmp4_protectionSchemeInfo));
            return true;
        }
    }

    LOGD(("%s: Did not find a valid protection scheme for trackId %d.", BSTD_FUNCTION, inTrackId));
    return false;
}

bool MediaParser::GetVideoResolution(uint32_t* width, uint32_t* height)
{
    struct mp4_parser_context* parser_context = (struct mp4_parser_context *)m_handle;
    uint32_t index;

    if (m_handle == NULL)
    {
        LOGE(("Invalid Parser Handle."));
        return false;
    }

    if (width == NULL || height == NULL) {
        LOGE(("Invalid parameters"));
        return false;
    }

    *width = 0;
    *height = 0;

    bmp4_mp4_headers *mp4_header = &parser_context->mp4_mp4;

    for (index = 0; index < mp4_header->nbOfTracks; index++) {
        uint32_t w = mp4_header->trackHeader[index + 1].width >> 16;
        uint32_t h = mp4_header->trackHeader[index + 1].height >> 16;
        if (w != 0 && h != 0) {
            LOGD(("%s: trackId=%d %dx%d", BSTD_FUNCTION, index+1, w, h));
            *width = w;
            *height = h;
        }
    }

    LOGD(("%s: resolution %dx%d", BSTD_FUNCTION, *width, *height));
    return true;
}
