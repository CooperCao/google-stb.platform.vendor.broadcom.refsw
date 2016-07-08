/******************************************************************************
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
 ******************************************************************************/
#ifndef NAMEVALUE_H__
#define NAMEVALUE_H__

#include "bmedia_types.h"
#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* nexus enum/string lookups for example/test apps */

typedef struct {
    const char *name; /* string for nexus enum */
    int value; /* nexus enum */
} namevalue_t;

extern const namevalue_t g_videoFrameRateStrs[];
extern const namevalue_t g_videoFormatStrs[];
extern const namevalue_t g_videoFrameRateStrs[];
extern const namevalue_t g_transportTypeStrs[];
extern const namevalue_t g_videoCodecStrs[];
extern const namevalue_t g_videoCodecProfileStrs[];
extern const namevalue_t g_videoCodecLevelStrs[];
extern const namevalue_t g_audioCodecStrs[];
extern const namevalue_t g_stcChannelMasterStrs[];
extern const namevalue_t g_endOfStreamActionStrs[];
extern const namevalue_t g_tsTimestampType[];
extern const namevalue_t g_contentModeStrs[];
extern const namevalue_t g_vsbModeStrs[];
extern const namevalue_t g_qamModeStrs[];
extern const namevalue_t g_ofdmModeStrs[];
extern const namevalue_t g_dvbt2ProfileStrs[];
extern const namevalue_t g_satModeStrs[];
extern const namevalue_t g_diseqcVoltageStrs[];
extern const namevalue_t g_diseqcToneEnabledStrs[];
extern const namevalue_t g_satNetworkSpecStrs[];
extern const namevalue_t g_videoErrorHandling[];
extern const namevalue_t g_videoOrientation[];
extern const namevalue_t g_videoSourceBuffer[];
extern const namevalue_t g_sourceOrientation[];
extern const namevalue_t g_channelChangeMode[];
extern const namevalue_t g_displayAspectRatioStrs[];
extern const namevalue_t g_securityAlgoStrs[];
extern const namevalue_t g_audioChannelModeStrs[];
extern const namevalue_t g_dolbyDrcModeStrs[];
extern const namevalue_t g_audioDdreCodecs[];
extern const namevalue_t g_platformStandbyModeStrs[];
extern const namevalue_t g_colorSpaceStrs[];
extern const namevalue_t g_audioLoudnessStrs[];
extern const namevalue_t g_macrovisionStrs[];
extern const namevalue_t g_videoEotfStrs[];
extern const namevalue_t g_syncModeStrs[];
extern const namevalue_t g_matrixCoeffStrs[];

unsigned lookup(const namevalue_t *table, const char *name);
const char *lookup_name(const namevalue_t *table, int value);
void print_list(const namevalue_t *table);
void print_list_option(const char *name, const namevalue_t *list);

/* convert bmedia types to nexus types. useful for bmedia_probe */
NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value);
NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value);
NEXUS_TransportType b_mpegtype2nexus(bstream_mpeg_type settop_value);

#ifdef __cplusplus
}
#endif

#endif
