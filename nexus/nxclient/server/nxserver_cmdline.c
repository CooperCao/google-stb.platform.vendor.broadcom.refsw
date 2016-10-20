/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nxserverlib.h"
#include "namevalue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(nxserver_cmdline);

static const namevalue_t g_pixelFormatStrs[] = {
    {"ARGB32", NEXUS_PixelFormat_eA8_R8_G8_B8},
    {"Compressed", NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8},
    {"ARGB16", NEXUS_PixelFormat_eA4_R4_G4_B4},
    {NULL, 0}
};

static void print_usage(void)
{
    printf(
    "usage: nexus nxserver [--help|-h]\n"
    );
    printf(
    "settings:\n"
    "  --help|-h      \tprint this help screen\n"
    "  -cfg FILE      \tload nxserver command line parameters from FILE\n"
    );
    printf(
    "  -mode {protected|untrusted} \tforce clients into specified mode\n"
    "  -password FILE \tPassword file. See nexus/nxclient/apps/resources/password.txt for template.\n"
    "  -grab off      \tdon't allow clients to grab resources from other clients\n"
    );
    printf(
    "  -sd off        \tdisable Standard Definition display (default is enabled)\n"
    "  -sd_graphics X,Y,W,H \tset manual graphics position for SD display\n"
    "  -sd_timebase   \tDedicate timebase1 for SD display\n"
    );
    print_list_option("display_format",g_videoFormatStrs);
    printf(
    "  -3d {lr|ou}    \tenable halfres stereoscopic (3D) display. Use -display_format for fullres.\n"
    "  -native_3d off \tdisable native stereoscopic (3D) video (default is enabled)\n"
    "  -cursor on     \tenable cursor\n"
    "  -framebuffers n\tnumber of framebuffers allocated for the primary display\n"
    "  -timeout X     \texit after X seconds. 0 (default) means forever.\n"
    "  -prompt        \tprompt user for input. does not work with background mode.\n"
    );
    printf(
    "  -ignore_edid   \tdon't consider HDMI EDID for audio/video output\n"
    "  -ignore_video_edid \tdon't consider HDMI EDID for video output\n"
    );
    printf(
#if NEXUS_HAS_IR_INPUT
    "  -ir {silver|black|a|b|rcmm|gisat|none} \tIR remote mode, comma-separated list of two supported, default is \"silver,none\"\n"
#endif
    "  -evdev off     \tdisable evdev input\n"
    "  -keypad on     \tenable keypad input\n"
    "  -transcode {on|off|sd} \toff = no transcode, sd = steal SD display when starting DSP transcode\n"
    "  -fbsize W,H    \tframebuffer allocated for this size (default is 1280,720)\n"
    "  -allowCompositionBypass \tallow SurfaceCompositor bypass if single push client\n"
    );
    printf(
    "  -outputs off   \tstart with hdmi, component, and composite video outputs off\n"
    "  -component off \tneeded for composite on chips with limited analog resources\n"
    );
    printf(
    "  -session{0|1} {sd,hd,encode,none} \tConfigure session for HD, SD or encode output. Use none for headless.\n"
    "  -video_cdb MBytes  \tsize of compressed video buffer, in MBytes, decimal allowed\n"
    "  -audio_cdb MBytes  \tsize of compressed audio buffer, in MBytes, decimal allowed\n"
    "  -audio_playback_fifo SIZE \tUse M or K suffix for units.\n"
    );
    printf(
    "  -standby_timeout X \tnumber of seconds to wait for clients to acknowledge standby\n"
    "  -heap NAME,SIZE    \tNAME is {main|gfx|gfx2|client|video_secure|export}. Use M or K suffix for units.\n"
    "  -audio_description \tenable audio description\n"
    "  -persistent_sad \tenable support for persistent simple audio decoders\n"
    "  -loudness [atsc|ebu]\tenable ATSC A/85 or EBU-R128 loudness equivalence.\n"
    "  -spdif4xCompressed \tenable support for spdif to output DDP compressed\n"
    );
    print_list_option("colorSpace",g_colorSpaceStrs);
    printf(
    "  -colorDepth {0|8|10} \t0 is auto\n"
    "  -dropFrame {enable|disable|track} \tHD display is drop frame, non-drop frame or tracks source (default)\n"
    "  -avc51         \tenable AVC Level 5.1 decoding\n"
    "  -frontend off  \tdon't init frontend\n"
    );
    printf(
    "  -avl {on|off}  \tauto volume level control\n"
    "  -truVolume {on|off}  \tenable SRS TruVolume stage\n"
    "  -ms11          \tenable MS11 post-processing. MS11 must also be enabled in BDSP, i.e. export BDSP_MS11_SUPPORT=y\n"
    "  -ms12          \tenable MS12 post-processing. MS12 must also be enabled in BDSP, i.e. export BDSP_MS12_SUPPORT=y\n"
    "  -karaoke       \tenable Karaoke post-processing support\n"
    "  -audio_output_delay X \tmaximum independent audio output delay (in msec)\n"
    );
    printf(
    "  -svp           \tUse NEXUS_VIDEO_SECURE_HEAP heap for video/audio CDB\n"
    "  -svp_urr       \tsvp option + set all picture buffers to secure only\n");
#if NEXUS_HAS_HDMI_OUTPUT
    printf(
    "  -hdcp2x_keys BINFILE \tspecify location of Hdcp2.x bin file\n"
    "  -hdcp1x_keys BINFILE \tspecify location of Hdcp1.x bin file\n"
    "  -hdcp {m|o}    \talways run [m]andatory or [o]ptional HDCP for system test\n"
    );
    printf(
    "  -hdcp_version {auto|follow|hdcp1x} - if hdcp is optional or mandatory, then\n"
    "          \tauto   - (default) Always authenticate using the highest version supported by HDMI receiver\n"
    "          \tfollow - If HDMI receiver is a Repeater, the HDCP_version depends on the Repeater downstream topology\n"
    "          \t         If Repeater downstream topology contains one or more HDCP 1.x device, then authenticate with Repeater using the HDCP 1.x.\n"
    );
    printf(
    "          \t         If Repeater downstream topology contains only HDCP 2.2 devices, then authenticate with Repeater using HDCP 2.2\n"
    "          \t         If HDMI Receiver is not a Repeater, then default to 'auto' selection\n"
    "          \thdcp1x - Always authenticate using HDCP 1.x mode (regardless of HDMI Receiver capabilities)\n"
    "          \thdcp22 - Always authenticate using HDCP 2.2 mode (regardless of HDMI Receiver capabilities)\n"
    );
    printf(
    "  -spd VENDOR,DESCRIPTION \tSPD vendorName and description to transmit in HDMI SpdInfoFrame.\n"
    );
#endif
    printf(
    "  -maxDataRate X \tincrease RS/XC buffer rates, in Mbps. User assumes responsibility to not exceed bandwidth or starve other channels.\n"
    "  -remux         \tenable transport remux\n"
    );
    printf(
    "  -memconfig videoDecoder,INDEX,MAXWIDTH,MAXHEIGHT\n"
    "  -memconfig videoDecoder,INDEX,mosaic,NUMMOSAICS,MAXWIDTH,MAXHEIGHT\n"
    "  -memconfig display,INDEX,MAXWIDTH,MAXHEIGHT\n"
    "  -memconfig videoDecoder,svp,{INDEX|all},{s|u|su} (CANNOT be used w/ -svp or -svp_urr)\n"
    "  -memconfig display,svp,{INDEX|all},{s|u|su} (CANNOT be used w/ -svp or -svp_urr)\n"
    "  -memconfig videoEncoder,INDEX,MAXWIDTH,MAXHEIGHT\n"
    );
    printf(
    "  -memconfig display,capture=off    \tDon't allocate BVN buffers for various capture uses\n"
    "  -memconfig display,5060=off       \tDon't allocate BVN buffers for 50/60Hz conversion\n"
    "  -memconfig display,hddvi=off      \tDon't allocate BVN buffers for HDMI Rx/HD-DVI\n"
    "  -memconfig display,mtg=off        \tDon't allocate BVN buffers for MTG\n"
    );
    printf(
    "  -growHeapBlockSize SIZE           \tGrow/shrink on-demand heap with this block size. Use M or K suffix for units.\n"
    );
    printf(
    "  -videoDacBandGapAdjust VALUE\n"
    "  -videoDacDetection\n"
    );
    printf(
    "  -enablePassthroughAudioPlayback\n"
    "  -dropPrivilege USERID,GROUPID\n"
    );
    print_list_option("pixelFormat",g_pixelFormatStrs);
}

static int parse_session_settings(struct nxserver_session_settings *session_settings, const char *str)
{
    session_settings->output.sd =
    session_settings->output.hd =
    session_settings->output.encode = false;
    while (1) {
        const char *end = strchr(str, ',');
        unsigned n = end?(unsigned)(end-str):strlen(str);
        if (!strncmp("sd",str,n)) {
            session_settings->output.sd = true;
        }
        else if (!strncmp("hd",str,n)) {
            session_settings->output.hd = true;
        }
        else if (!strncmp("encode",str,n)) {
            session_settings->output.encode = true;
        }
        else if (!strncmp("encode_video",str,n)) {
            session_settings->output.encode = true;
            session_settings->output.encode_video_only = true;
        }
        else if (!strncmp("none",str,n)) {
        }
        else {
            return -1;
        }
        if (!end) break;
        str = end+1;
    }
    return 0;
}

static NEXUS_VideoFormat lookup_format(unsigned width, unsigned height, bool interlaced)
{
    BSTD_UNUSED(width);
    if (height > 1088) {
        return NEXUS_VideoFormat_e4096x2160p60hz;
    }
    else if (height > 720) {
        return interlaced?NEXUS_VideoFormat_e1080i:NEXUS_VideoFormat_e1080p;
    }
    else if (height > 480) {
        return NEXUS_VideoFormat_e720p;
    }
    else {
        return interlaced?NEXUS_VideoFormat_eNtsc:NEXUS_VideoFormat_e480p;
    }
}

static NEXUS_SecureVideo nxserver_p_svpstr(const char *svpstr)
{
    bool s = strchr(svpstr, 's') != NULL;
    bool u = strchr(svpstr, 'u') != NULL;
    if (s && u) {
        return NEXUS_SecureVideo_eBoth;
    }
    else if (s) {
        return NEXUS_SecureVideo_eSecure;
    }
    else {
        return NEXUS_SecureVideo_eUnsecure;
    }
}

static int nxserverlib_apply_memconfig_str(NEXUS_PlatformSettings *pPlatformSettings, NEXUS_MemoryConfigurationSettings *pMemConfigSettings, const char * const *memconfig_str, unsigned memconfig_str_total)
{
    unsigned i;
    BSTD_UNUSED(pPlatformSettings);
    for (i=0;i<memconfig_str_total;i++) {
        unsigned index, numMosaics, maxWidth, maxHeight;
        char svpstr[32];
        BSTD_UNUSED(numMosaics);
        if (0) {
        }
#if NEXUS_HAS_VIDEO_DECODER
        else if (sscanf(memconfig_str[i], "videoDecoder,svp,%u,%s", &index, svpstr) == 2 && index < NEXUS_MAX_VIDEO_DECODERS) {
            NEXUS_SecureVideo secure = nxserver_p_svpstr(svpstr);
            pMemConfigSettings->videoDecoder[index].secure = secure;
        }
        else if (sscanf(memconfig_str[i], "videoDecoder,svp,all,%s", svpstr) == 1) {
            NEXUS_SecureVideo secure = nxserver_p_svpstr(svpstr);
            for (index=0;index<NEXUS_MAX_VIDEO_DECODERS;index++) {
                pMemConfigSettings->videoDecoder[index].secure = secure;
            }
        }
        else if (sscanf(memconfig_str[i], "videoDecoder,%u,%u,%u", &index, &maxWidth, &maxHeight) == 3 && index < NEXUS_MAX_VIDEO_DECODERS) {
            pMemConfigSettings->videoDecoder[index].maxFormat = lookup_format(maxWidth, maxHeight, false /* don't care */);
        }
        else if (sscanf(memconfig_str[i], "videoDecoder,%u,mosaic,%u,%u,%u", &index, &numMosaics, &maxWidth, &maxHeight) == 4 && index < NEXUS_MAX_VIDEO_DECODERS) {
            pMemConfigSettings->videoDecoder[index].mosaic.maxNumber = numMosaics;
            pMemConfigSettings->videoDecoder[index].mosaic.maxWidth = maxWidth;
            pMemConfigSettings->videoDecoder[index].mosaic.maxHeight = maxHeight;
        }
        else if (!strcmp(memconfig_str[i], "videoDecoder,dynamic")) {
            unsigned d;
            for (d=0;d<NEXUS_MAX_VIDEO_DECODERS;d++) {
                pMemConfigSettings->videoDecoder[d].dynamicPictureBuffers = true;
            }
            for (d=0;d<NEXUS_MAX_HEAPS;d++) {
                if (pPlatformSettings->heap[d].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) {
                    pPlatformSettings->heap[d].memoryType = NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
                }
            }
        }
#endif
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
        else if (sscanf(memconfig_str[i], "display,%u,%u,%u", &index, &maxWidth, &maxHeight) == 3 && index < NEXUS_MAX_DISPLAYS) {
            pMemConfigSettings->display[index].maxFormat = lookup_format(maxWidth, maxHeight, false /* don't care */);
        }
        else if (sscanf(memconfig_str[i], "display,svp,%u,%s", &index, svpstr) == 2 && index < NEXUS_MAX_DISPLAYS) {
            unsigned j;
            NEXUS_SecureVideo secure = nxserver_p_svpstr(svpstr);
            for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
                pMemConfigSettings->display[index].window[j].secure = secure;
            }
        }
        else if (sscanf(memconfig_str[i], "display,svp,all,%s", svpstr) == 1) {
            NEXUS_SecureVideo secure = nxserver_p_svpstr(svpstr);
            for (index=0;index<NEXUS_MAX_DISPLAYS;index++) {
                unsigned j;
                for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
                    pMemConfigSettings->display[index].window[j].secure = secure;
                }
            }
        }
#endif
#if NEXUS_HAS_VIDEO_ENCODER
        else if (sscanf(memconfig_str[i], "videoEncoder,%u,%u,%u", &index, &maxWidth, &maxHeight) == 3 && index < NEXUS_MAX_VIDEO_ENCODERS) {
            pMemConfigSettings->videoEncoder[index].maxWidth = maxWidth;
            pMemConfigSettings->videoEncoder[index].maxHeight = maxHeight;
        }
#endif
        else if (!strcmp(memconfig_str[i], "display,capture=off")) {
            unsigned d, w;
            for (d=0;d<NEXUS_MAX_DISPLAYS;d++) {
                for (w=0;w<NEXUS_NUM_VIDEO_WINDOWS;w++) {
                    pMemConfigSettings->display[d].window[w].capture = false;
                }
            }
        }
        else if (!strcmp(memconfig_str[i], "display,5060=off")) {
            unsigned d, w;
            for (d=0;d<NEXUS_MAX_DISPLAYS;d++) {
                for (w=0;w<NEXUS_NUM_VIDEO_WINDOWS;w++) {
                    pMemConfigSettings->display[d].window[w].convertAnyFrameRate = false;
                }
            }
        }
        else if (!strcmp(memconfig_str[i], "display,mtg=off")) {
            unsigned d, w;
            for (d=0;d<NEXUS_MAX_DISPLAYS;d++) {
                for (w=0;w<NEXUS_NUM_VIDEO_WINDOWS;w++) {
                    pMemConfigSettings->display[d].window[w].mtg = false;
                }
            }
        }
        else if (!strcmp(memconfig_str[i], "display,hddvi=off")) {
            pMemConfigSettings->videoInputs.hdDvi = false;
        }
        else {
            fprintf(stderr,"invalid argument -memconfig %s\n", memconfig_str[i]);
            return NEXUS_INVALID_PARAMETER;
        }
    }

    /* if no SD display, turn precisionLipSync off */
    if (pMemConfigSettings->display[1].maxFormat == NEXUS_VideoFormat_eUnknown) {
        /* only HD main window defaults on */
        pMemConfigSettings->display[0].window[0].precisionLipSync = false;
    }

    return 0;
}

#if NEXUS_HAS_TRANSPORT
static int nxserverlib_apply_transport_settings(NEXUS_PlatformSettings *pPlatformSettings, unsigned maxDataRate, bool remux)
{
    NEXUS_TransportModuleSettings *pSettings = &pPlatformSettings->transportModuleSettings;
    unsigned i, j;
    for (i=0;i<NEXUS_MAX_PARSER_BANDS;i++) {
        pSettings->clientEnabled.parserBand[i].mpodRs = false;
        for (j=0;j<NEXUS_MAX_REMUX;j++) {
            pSettings->clientEnabled.parserBand[i].remux[j] = remux;
        }
        if (maxDataRate > pSettings->maxDataRate.parserBand[i]) {
            pSettings->maxDataRate.parserBand[i] = maxDataRate;
        }
    }
    for (i=0;i<NEXUS_MAX_PLAYPUMPS;i++) {
        for (j=0;j<NEXUS_MAX_REMUX;j++) {
            pSettings->clientEnabled.playback[i].remux[j] = remux;
        }
        if (maxDataRate > pSettings->maxDataRate.playback[i]) {
            pSettings->maxDataRate.playback[i] = maxDataRate;
        }
    }
    return 0;
}
#endif

static unsigned nxserverlib_parse_size(const char *parse)
{
    if (strchr(parse, 'M') || strchr(parse, 'm')) {
        return atof(parse)*1024*1024;
    }
    else if (strchr(parse, 'K') || strchr(parse, 'k')) {
        return atof(parse)*1024;
    }
    else {
        return strtoul(parse, NULL, 0);
    }
}

static int parse_heap_str(const char *str, char *name, unsigned namelen, unsigned *size)
{
    const char *parse;
    unsigned len;
    parse = strchr(str, ',');
    if (!parse) return -1;
    len = parse - str;
    if (len > namelen) {
        len = namelen;
    }
    strncpy(name, str, len);
    name[len] = 0;
    *size = nxserverlib_parse_size(++parse);
    return 0;
}

#define MB (1024*1024)

int nxserver_heap_by_type(const NEXUS_PlatformSettings *pPlatformSettings, unsigned heapType)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (pPlatformSettings->heap[i].heapType & heapType) return i;
    }

    /* for platforms that don't set heapType, try older methods */
    switch (heapType) {
    case NEXUS_HEAP_TYPE_GRAPHICS:
        {
        NEXUS_PlatformConfiguration platformConfig;
        NEXUS_HeapHandle heap;
        NEXUS_Platform_GetConfiguration(&platformConfig);
        heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if (platformConfig.heap[i] == heap) return i;
        }
        }
        break;
    case NEXUS_HEAP_TYPE_MAIN:
        return 0;
    default:
        break;
    }
    return -1;
}

static int find_unused_heap(const NEXUS_PlatformSettings *pPlatformSettings)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pPlatformSettings->heap[i].size && !pPlatformSettings->heap[i].heapType) return i;
    }
    return -1;
}

static int nxserverlib_apply_heap_str(NEXUS_PlatformSettings *pPlatformSettings, const NEXUS_MemoryConfigurationSettings *pMemConfigSettings, const char * const *heap_str, unsigned heap_str_total, struct nxserver_settings *settings)
{
    unsigned i;
    for (i=0;i<heap_str_total;i++) {
        char name[32];
        unsigned size;
        int rc;
        rc = parse_heap_str(heap_str[i], name, sizeof(name), &size);
        if (rc) return rc;
        if (!strcmp(name,"gfx")) {
            int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_GRAPHICS);
            if (index != -1) {
                BDBG_LOG(("setting gfx heap[%d] to %dMB", index, size/MB));
                pPlatformSettings->heap[index].size = size;
            }
        }
        else if (!strcmp(name,"gfx2")) {
            int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS);
            if (index != -1) {
                BDBG_LOG(("setting gfx2 heap[%d] to %dMB", index, size/MB));
                pPlatformSettings->heap[index].size = size;
            }
        }
        else if (!strcmp(name,"video_secure")) {
            int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION);
            if (index != -1) {
                BDBG_LOG(("setting video_secure heap[%d] to %dMB", index, size/MB));
                pPlatformSettings->heap[index].size = size;
            }
        }
        else if (!strcmp(name,"main")) {
            int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_MAIN);
            if (index != -1) {
                BDBG_LOG(("setting main heap[%d] to %dMB", index, size/MB));
                pPlatformSettings->heap[index].size = size;
            }
        }
        else if (!strcmp(name,"export")) {
            int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_EXPORT_REGION);
            if (index != -1) {
                BDBG_LOG(("setting export heap[%d] to %dMB", index, size/MB));
                pPlatformSettings->heap[index].size = size;
            }
        }
        else if (!strcmp(name,"client")) {
#if BCHP_CHIP == 7125
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#else
            /* create a dedicated heap for the client */
            int unused_heap;
            int mainIndex = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_GRAPHICS);
            if (mainIndex == -1) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
            unused_heap = find_unused_heap(pPlatformSettings);
            if (unused_heap == -1) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
            settings->heaps.clientFullHeap = unused_heap;
            pPlatformSettings->heap[settings->heaps.clientFullHeap].size = size;
            pPlatformSettings->heap[settings->heaps.clientFullHeap].memcIndex = pPlatformSettings->heap[mainIndex].memcIndex;
            pPlatformSettings->heap[settings->heaps.clientFullHeap].subIndex = pPlatformSettings->heap[mainIndex].subIndex;
            pPlatformSettings->heap[settings->heaps.clientFullHeap].guardBanding = false; /* corruptions shouldn't cause server failures */
            pPlatformSettings->heap[settings->heaps.clientFullHeap].alignment = pPlatformSettings->heap[mainIndex].alignment;
            pPlatformSettings->heap[settings->heaps.clientFullHeap].memoryType = NEXUS_MemoryType_eFull; /* requires for packet blit and playpump */
#if NEXUS_HAS_SAGE
            /* sage must be told which heap the client's will be using */
            pPlatformSettings->sageModuleSettings.clientHeapIndex = settings->heaps.clientFullHeap;
#endif
#endif
        }
        else if (!strcmp(name,"securegfx")) {
            NEXUS_PlatformConfigCapabilities cap;
            NEXUS_GetPlatformConfigCapabilities(pPlatformSettings, pMemConfigSettings, &cap);
            if (cap.secureGraphics[0].valid) {
                BDBG_LOG(("creating securegfx heap[%d] on MEMC%u for %dMB", cap.secureGraphics[0].heapIndex, cap.secureGraphics[0].memcIndex, size/MB));
                pPlatformSettings->heap[cap.secureGraphics[0].heapIndex].size = size;
                pPlatformSettings->heap[cap.secureGraphics[0].heapIndex].memcIndex = cap.secureGraphics[0].memcIndex;
                pPlatformSettings->heap[cap.secureGraphics[0].heapIndex].heapType = NEXUS_HEAP_TYPE_SECURE_GRAPHICS;
                pPlatformSettings->heap[cap.secureGraphics[0].heapIndex].memoryType = NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED|NEXUS_MEMORY_TYPE_MANAGED|NEXUS_MEMORY_TYPE_SECURE;
                /* TODO: allow securegfx to GFD1. for now, apply "-sd off" */
                parse_session_settings(&settings->session[0], "hd");
            }
        }
        else {
            return -1;
        }
    }
    return 0;
}

static NEXUS_IrInputMode nxserver_ir_input_mode(const char *str)
{
    if (!strcmp("a", str) || !strcmp("black", str)) {
        return NEXUS_IrInputMode_eRemoteA;
    }
    else if (!strcmp("b", str)) {
        return NEXUS_IrInputMode_eRemoteB;
    }
    else if (!strcmp("rcmm", str)) {
        return NEXUS_IrInputMode_eCirRcmmRcu;
    }
    else if (!strcmp("gisat", str)) {
        return NEXUS_IrInputMode_eCirGISat;
    }
    else if (!strcmp("none", str)) {
        return NEXUS_IrInputMode_eMax;
    }
    else { /* silver */
        return NEXUS_IrInputMode_eCirNec;
    }
}

struct nx_argv {
#define MAX_ARGC 256
    char *lines[MAX_ARGC];
    int num_lines;
    char *argv[MAX_ARGC];
    int argc;
};

static char *nx_delimit_word(char *str, char **next)
{
#define WHITESPACE " \t\n\r"
    str += strspn(str, WHITESPACE);
    if (!*str || *str == '#') return NULL; /* end of line or line comment */
    *next = strpbrk(str, WHITESPACE);
    if (*next) {
        str[*next-str] = 0; /* null terminate in-place */
        (*next)++;
    }
    return str;
}

static int nx_load_cfg(const char *filename, struct nx_argv *nx_argv)
{
    FILE *file;
    file = fopen(filename, "r");
    if (!file) return -1;
    memset(nx_argv, 0, sizeof(*nx_argv));
    nx_argv->argv[nx_argv->argc++] = "nxserver";
    while (!feof(file) && nx_argv->num_lines < MAX_ARGC && nx_argv->argc < MAX_ARGC) {
        char *s;
        nx_argv->lines[nx_argv->num_lines] = malloc(256);
        if (!nx_argv->lines[nx_argv->num_lines]) break;
        s = fgets(nx_argv->lines[nx_argv->num_lines++], 256, file);
        while (s && nx_argv->argc < MAX_ARGC) {
            char *word = nx_delimit_word(s, &s);
            if (!word) break;
            nx_argv->argv[nx_argv->argc++] = word;
        }
    }
    return 0;
}

static void nx_unload_cfg(struct nx_argv *nx_argv)
{
    int i;
    for (i=1;i<nx_argv->num_lines;i++) free(nx_argv->lines[i]);
}

static int nxserver_parse_cmdline_aux(int argc, char **argv, struct nxserver_settings *settings, struct nxserver_cmdline_settings *cmdline_settings)
{
    int curarg = 1;
    bool session0_set = false;
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return -1;
        }
        else if (!strcmp(argv[curarg], "-cfg") && curarg+1<argc) {
            /* call nxserver_parse_cmdline_aux recursively, then keep going */
            struct nx_argv nx_argv;
            int rc = nx_load_cfg(argv[++curarg], &nx_argv);
            if (rc) return rc;
            rc = nxserver_parse_cmdline_aux(nx_argv.argc, nx_argv.argv, settings, cmdline_settings);
            nx_unload_cfg(&nx_argv); /* must unload if load succeeded */
            if (rc) return rc;
        }
#if NEXUS_HAS_HDMI_OUTPUT
        else if (!strcmp(argv[curarg], "-ignore_edid")) {
            settings->display.hdmiPreferences.followPreferredFormat = false;
            settings->display.hdmiPreferences.preventUnsupportedFormat = false;
        }
        else if (!strcmp(argv[curarg], "-ignore_video_edid")) {
            /* keep followPreferredFormat true for audio. preventUnsupportedFormat will be ignored. */
            settings->hdmi.ignoreVideoEdid = true;
        }
#endif
        else if (!strcmp(argv[curarg], "-frontend")) {
            /* for backward compat, allow just -frontend, but check for "-frontend off" */
            if (argc>curarg+1 && !strcmp(argv[curarg+1], "off")) {
                curarg++;
                cmdline_settings->frontend = false;
            }
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            settings->timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-prompt") || !strcmp(argv[curarg], "-p")) {
            settings->prompt = true;
        }
        else if (!strcmp(argv[curarg], "-transcode") && curarg+1<argc) {
            curarg++;
            if (!strcmp(argv[curarg], "off")) {
                settings->transcode = nxserver_transcode_off;
            }
            else if (!strcmp(argv[curarg], "on")) {
                settings->transcode = nxserver_transcode_on;
            }
            else if (!strcmp(argv[curarg], "sd")) {
                settings->transcode = nxserver_transcode_sd;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-cursor") && curarg+1<argc) {
            curarg++;
            if (strcmp(argv[curarg], "on")==0) {
                settings->cursor = true;
            }
        } else if(!strcmp(argv[curarg], "-framebuffers") && curarg+1<argc) {
            settings->framebuffers = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-3d") && curarg+1<argc) {
            curarg++;
            if (strcmp(argv[curarg], "lr")==0) {
                settings->display.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            }
            else if(strcmp(argv[curarg], "ou")==0) {
                settings->display.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-native_3d") && curarg+1<argc) {
            curarg++;
            if (strcmp(argv[curarg], "off")==0) {
                settings->native_3d = false;
        } else if (strcmp(argv[curarg], "on")==0) {
                settings->native_3d = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mode") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("untrusted", argv[curarg])) {
                settings->client_mode = NEXUS_ClientMode_eUntrusted;
            }
            else if (!strcmp("protected", argv[curarg])) {
                settings->client_mode = NEXUS_ClientMode_eProtected;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-password") && curarg+1 < argc) {
            if (nxclient_p_parse_password_file(argv[++curarg], &settings->certificate)) {
                print_usage();
                return -1;
            }
            /* If you set -password, then change the default client mode to eUntrusted, which is used
            if a matching password is not found. */
            settings->client_mode = NEXUS_ClientMode_eUntrusted;
        }
#if NEXUS_HAS_IR_INPUT
        else if (!strcmp(argv[curarg], "-ir") && curarg+1 < argc) {
            const char *comma;
            ++curarg;
            comma = strchr(argv[curarg], ',');
            if (comma) {
                char buf[32];
                unsigned len = comma - argv[curarg];
                strncpy(buf, argv[curarg], len);
                buf[len] = 0;
                settings->session[0].ir_input.mode[0] = nxserver_ir_input_mode(buf);
                settings->session[0].ir_input.mode[1] = nxserver_ir_input_mode(++comma);
            }
            else {
                settings->session[0].ir_input.mode[0] = nxserver_ir_input_mode(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-ir_standby_filter") && curarg+1 < argc) {
            settings->session[0].ir_input.standby_filter = strtoul(argv[++curarg], NULL, 0);
        }
#endif
        else if (!strcmp(argv[curarg], "-evdev") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("off", argv[curarg])) {
                settings->session[0].evdevInput = false;
            }
        }
        else if (!strcmp(argv[curarg], "-keypad") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("on", argv[curarg])) {
                settings->session[0].keypad = true;
            }
        }
        else if (!strcmp(argv[curarg], "-fbsize") && curarg+1 < argc) {
            sscanf(argv[++curarg], "%u,%u", &settings->fbsize.width, &settings->fbsize.height);
        }
        else if (!strcmp(argv[curarg], "-allowCompositionBypass")) {
            settings->allowCompositionBypass = true;
        }
        else if (!strcmp(argv[curarg], "-outputs") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("off", argv[curarg])) {
                settings->display.hdmiPreferences.enabled = false;
                settings->display.componentPreferences.enabled = false;
                settings->display.compositePreferences.enabled = false;
            }
        }
        else if (!strcmp(argv[curarg], "-component") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("off", argv[curarg])) {
                settings->display.componentPreferences.enabled = false;
            }
        }
        else if (!strcmp(argv[curarg], "-sd") && curarg+1<argc) {
            curarg++;
            if (!strcmp(argv[curarg], "off")) {
                if (parse_session_settings(&settings->session[0], "hd")) {
                    print_usage();
                    return -1;
                }
            }
            else if (!strcmp(argv[curarg], "on")) {
                if (parse_session_settings(&settings->session[0], "hd_sd")) {
                    print_usage();
                    return -1;
                }
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-sd_graphics") && curarg+1 < argc) {
            unsigned x, y, w, h;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x, &y, &w, &h) == 4) {
                settings->display_init.sd.graphicsPosition.x = x;
                settings->display_init.sd.graphicsPosition.y = y;
                settings->display_init.sd.graphicsPosition.width = w;
                settings->display_init.sd.graphicsPosition.height = h;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-sd_timebase")) {
            settings->display_init.sd.dedicatedTimebase = true;
        }
        else if (!strcmp(argv[curarg], "-session0") && curarg+1 < argc) {
            curarg++;
            if (parse_session_settings(&settings->session[0], argv[curarg])) {
                print_usage();
                return -1;
            }
            session0_set = true;
        }
        else if (!strcmp(argv[curarg], "-session1") && curarg+1 < argc) {
            curarg++;
            if (parse_session_settings(&settings->session[1], argv[curarg])) {
                print_usage();
                return -1;
            }
            if (!session0_set) {
                /* assume only one set of SD outputs and that SD display is always 1 (same as second HD display),
                so allow session1 to steal from session0 default. */
                if (IS_SESSION_DISPLAY_ENABLED(settings->session[1])) {
                    settings->session[0].output.sd = false;
                }
            }
        }
        else if (!strcmp(argv[curarg], "-video_cdb") && curarg+1 < argc) {
            float size;
            sscanf(argv[++curarg], "%f", &size);
            if (size > 100.0) /* is 100 M sufficient for this check? */
            {
                printf("\n### Specified video CDB size (%0.3f MB = %0.0f bytes) too large\n\n", size, size * MB);
                print_usage();
                return -1;
            }
            settings->videoDecoder.fifoSize = size * MB;
        }
        else if (!strcmp(argv[curarg], "-audio_cdb") && curarg+1 < argc) {
            float size;
            sscanf(argv[++curarg], "%f", &size);
            if (size > 20.0) /* is 20 M sufficient for this check? */
            {
                printf("\n### Specified audio CDB size (%0.3f MB = %0.0f bytes) too large\n\n", size, size * MB);
                print_usage();
                return -1;
            }
            settings->audioDecoder.fifoSize = size * MB;
        }
        else if (!strcmp(argv[curarg], "-standby_timeout") && argc>curarg+1) {
            settings->standby_timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-heap") && argc>curarg+1) {
            if (cmdline_settings->heap.total < sizeof(cmdline_settings->heap.str)/sizeof(cmdline_settings->heap.str[0])) {
                cmdline_settings->heap.str[cmdline_settings->heap.total++] = argv[++curarg];
            }
        }
        else if (!strcmp(argv[curarg], "-memconfig") && curarg+1 < argc) {
            if (cmdline_settings->memconfig.total < sizeof(cmdline_settings->memconfig.str)/sizeof(cmdline_settings->memconfig.str[0])) {
                cmdline_settings->memconfig.str[cmdline_settings->memconfig.total++] = argv[++curarg];
            }
        }
        else if (!strcmp(argv[curarg], "-audio_description")) {
            settings->audioDecoder.audioDescription = true;
        }
        else if (!strcmp(argv[curarg], "-display_format") && curarg+1 < argc) {
            settings->display.format = lookup(g_videoFormatStrs, argv[++curarg]);
            settings->display_init.hd.initialFormat = true;
        }
        else if (!strcmp(argv[curarg], "-dropFrame") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp(argv[curarg],"enable")) {
                settings->display_init.hd.dropFrame = NEXUS_TristateEnable_eEnable;
            }
            else if (!strcmp(argv[curarg],"disable")) {
                settings->display_init.hd.dropFrame = NEXUS_TristateEnable_eDisable;
            }
            else {
                /* eNotSet is "track" */
                settings->display_init.hd.dropFrame = NEXUS_TristateEnable_eNotSet;
            }
        }
        else if (!strcmp(argv[curarg], "-colorSpace") && argc>curarg+1) {
            settings->display.hdmiPreferences.colorSpace = lookup(g_colorSpaceStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-colorDepth") && argc>curarg+1) {
            settings->display.hdmiPreferences.colorDepth = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-avc51")) {
            cmdline_settings->avc51 = true;
        }
        else if (!strcmp(argv[curarg], "-avl") && argc>curarg+1) {
            settings->session[0].avl = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-truVolume") && argc>curarg+1) {
            settings->session[0].truVolume = !strcmp(argv[++curarg], "on");
        }
        else if (!strcmp(argv[curarg], "-ms11")) {
            settings->session[0].dolbyMs = nxserverlib_dolby_ms_type_ms11;
        }
        else if (!strcmp(argv[curarg], "-ms12")) {
            settings->session[0].dolbyMs = nxserverlib_dolby_ms_type_ms12;
        }
        else if (!strcmp(argv[curarg], "-loudness") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp("atsc", argv[curarg])) {
                cmdline_settings->loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eAtscA85;
            }
            else if (!strcmp("ebu", argv[curarg])) {
                cmdline_settings->loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eEbuR128;
            }
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-karaoke")) {
            settings->session[0].karaoke = true;
        }
        else if (!strcmp(argv[curarg], "-persistent_sad")) {
            settings->session[0].persistentDecoderSupport = true;
        }
        else if (!strcmp(argv[curarg], "-spdif4xCompressed")) {
            settings->session[0].allowSpdif4xCompressed = true;
        }
        else if (!strcmp(argv[curarg], "-maxDataRate") && argc>curarg+1) {
            /* Exceeding bandwidth on playback XC buffer may cause other playbacks to starve and underflow.
            Exceeding bandwidth on live parser band RS/XC buffers may cause data to be dropped. */
            cmdline_settings->maxDataRate = atoi(argv[++curarg]) * 1000 * 1000;
        }
        else if (!strcmp(argv[curarg], "-audio_output_delay") && argc>curarg+1) {
            cmdline_settings->audio_output_delay = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-audio_sample_rate") && argc>curarg+1) {
            settings->audioMixer.sampleRate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-i2s_input")) {
            settings->audioInputs.i2sEnabled = true;
        }
        else if (!strcmp(argv[curarg], "-svp_urr")) {
            settings->svp = nxserverlib_svp_type_cdb_urr;
        }
        else if (!strcmp(argv[curarg], "-svp")) {
            if(settings->svp == nxserverlib_svp_type_none) {
                settings->svp = nxserverlib_svp_type_cdb;
            }
        }
        else if (!strcmp(argv[curarg], "-remux")) {
            cmdline_settings->remux = true;
        }
#if NEXUS_HAS_HDMI_OUTPUT
        else if (!strcmp(argv[curarg], "-hdcp2x_keys") && argc>curarg+1) {
            settings->hdcp.hdcp2xBinFile = (char *)argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-hdcp1x_keys") && argc>curarg+1) {
            settings->hdcp.hdcp1xBinFile = (char *)argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-hdcp") && argc>curarg+1) {
            switch (argv[++curarg][0]) {
            case 'm': settings->hdcp.alwaysLevel = NxClient_HdcpLevel_eMandatory; break;
            case 'o': settings->hdcp.alwaysLevel = NxClient_HdcpLevel_eOptional; break;
            default: print_usage(); return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-hdcp_version") && argc>curarg+1) {
            curarg++;
            if      (!strcmp(argv[curarg],"auto"  ))  settings->hdcp.versionSelect = NxClient_HdcpVersion_eAuto;
            else if (!strcmp(argv[curarg],"follow"))  settings->hdcp.versionSelect = NxClient_HdcpVersion_eFollow;
            else if (!strcmp(argv[curarg],"hdcp1x"))  settings->hdcp.versionSelect = NxClient_HdcpVersion_eHdcp1x;
            else if (!strcmp(argv[curarg],"hdcp22"))  settings->hdcp.versionSelect = NxClient_HdcpVersion_eHdcp22;
            else {
                print_usage();
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-spd") && curarg+1<argc) {
            const char *comma = strchr(argv[++curarg], ',');
            if (comma) {
                unsigned n = comma - argv[curarg];
                if (n > sizeof(settings->hdmi.spd.vendorName)) n = sizeof(settings->hdmi.spd.vendorName);
                strncpy(settings->hdmi.spd.vendorName, argv[curarg], n);
                strncpy(settings->hdmi.spd.description, ++comma, sizeof(settings->hdmi.spd.description));
            }
            else {
                strncpy(settings->hdmi.spd.vendorName, argv[curarg], sizeof(settings->hdmi.spd.vendorName));
            }
        }
#endif
        else if (!strcmp(argv[curarg], "-growHeapBlockSize") && argc>curarg+1) {
            settings->growHeapBlockSize = nxserverlib_parse_size(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-grab") && argc>curarg+1) {
            settings->grab = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-audio_playback_fifo") && argc>curarg+1) {
            settings->audioPlayback.fifoSize = nxserverlib_parse_size(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-videoDacBandGapAdjust") && argc>curarg+1) {
            cmdline_settings->video.dacBandGapAdjust = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-videoDacDetection")) {
            cmdline_settings->video.dacDetection = true;
        }
        else if (!strcmp(argv[curarg], "-enablePassthroughAudioPlayback")) {
            settings->audioDecoder.enablePassthroughBuffer = true;
            if (settings->session[0].audioPlaybacks > 0){
                /* Reserve one for the decoder instead of playback */
                settings->session[0].audioPlaybacks--;
            }
        }
        else if (!strcmp(argv[curarg], "-dropPrivilege") && argc>curarg+1) {
            if (sscanf(argv[++curarg], "%u,%u", &cmdline_settings->permissions.userId, &cmdline_settings->permissions.groupId) != 2) {
                print_usage();
                return -1;
            }
        } else if(!strcmp(argv[curarg], "-pixelFormat") && curarg+1<argc) {
            settings->pixelFormat = lookup(g_pixelFormatStrs, argv[++curarg]);
        }
        else {
            fprintf(stderr,"invalid argument %s\n", argv[curarg]);
            print_usage();
            return -1;
        }
        curarg++;
    }
    return 0;
}

int nxserver_parse_cmdline(int argc, char **argv, struct nxserver_settings *settings, struct nxserver_cmdline_settings *cmdline_settings)
{
    int rc;
    memset(cmdline_settings, 0, sizeof(*cmdline_settings));
    cmdline_settings->frontend = true;
    cmdline_settings->loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eNone;
    settings->session[0].dolbyMs = nxserverlib_dolby_ms_type_none;

    rc = nxserver_parse_cmdline_aux(argc, argv, settings, cmdline_settings);
    if (rc) return rc;

#if NEXUS_HAS_IR_INPUT
    if (!settings->session[0].ir_input.standby_filter) {
        switch (settings->session[0].ir_input.mode[0]) {
        case NEXUS_IrInputMode_eCirNec:
            settings->session[0].ir_input.standby_filter = 0xf50aff00;
            break;
        case NEXUS_IrInputMode_eRemoteA:
            settings->session[0].ir_input.standby_filter = 0x600A;
            break;
        default: break;
        }
    }
#endif

    return 0;
}

int nxserver_modify_platform_settings(struct nxserver_settings *settings, const struct nxserver_cmdline_settings *cmdline_settings,
    NEXUS_PlatformSettings *pPlatformSettings, NEXUS_MemoryConfigurationSettings *pMemConfigSettings)
{
    unsigned i;
    int rc;

#if NEXUS_HAS_TRANSPORT
    nxserverlib_apply_transport_settings(pPlatformSettings, cmdline_settings->maxDataRate, cmdline_settings->remux);
#endif
#if NEXUS_HAS_AUDIO
    /* Audio Sanity Checks */
    if ( settings->session[0].dolbyMs == nxserverlib_dolby_ms_type_ms11 )
    {
        if ( pMemConfigSettings->audio.dolbyCodecVersion != NEXUS_AudioDolbyCodecVersion_eMS11 )
        {
            BDBG_ERR(("Dolby MS11 must be enabled by environment settings (export BDSP_MS11_SUPPORT=y) and associated DSP FW binaries must be present."));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    else if ( settings->session[0].dolbyMs == nxserverlib_dolby_ms_type_ms12 )
    {
        if ( pMemConfigSettings->audio.dolbyCodecVersion != NEXUS_AudioDolbyCodecVersion_eMS12 )
        {
            BDBG_ERR(("Dolby MS12 must be enabled by environment settings (export BDSP_MS12_SUPPORT=y) and associated DSP FW binaries must be present."));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
#endif
    pPlatformSettings->openCec = false; /* clients may open it */
    if (!cmdline_settings->frontend) {
        pPlatformSettings->openFrontend = false;
    }
#if NEXUS_HAS_AUDIO && NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if (settings->transcode != nxserver_transcode_off) {
        /* needed for quad transcode */
        pPlatformSettings->audioModuleSettings.numCompressedBuffers *= 3;
    }
    else {
        pPlatformSettings->audioModuleSettings.maxAudioDspTasks = 2;
    }
    if (cmdline_settings->audio_output_delay) {
        pPlatformSettings->audioModuleSettings.independentDelay = true;
        pPlatformSettings->audioModuleSettings.maxIndependentDelay = cmdline_settings->audio_output_delay;
    }
    if (settings->session[0].dolbyMs == nxserverlib_dolby_ms_type_ms11
        || settings->session[0].dolbyMs == nxserverlib_dolby_ms_type_ms12
        || settings->session[0].avl
        || settings->session[0].truVolume)
    {
        pPlatformSettings->audioModuleSettings.numPcmBuffers += 1;
    }
    pPlatformSettings->audioModuleSettings.allowSpdif4xCompressed = settings->session[0].allowSpdif4xCompressed;
    pPlatformSettings->audioModuleSettings.loudnessMode = cmdline_settings->loudnessMode;
#endif
#if NEXUS_HAS_VIDEO_DECODER
    pMemConfigSettings->videoDecoder[0].avc51Supported = cmdline_settings->avc51;
#endif

    if(settings->svp != nxserverlib_svp_type_none) {
        for(i=0;i<cmdline_settings->memconfig.total;i++) {
            if(strstr(cmdline_settings->memconfig.str[i], "svp")) {
                BDBG_ERR(("Cannot combine svp or svp_urr flag w/ svp memconfig settings!"));
                print_usage();
                return -1;
            }
        }
    }

    if(settings->svp == nxserverlib_svp_type_cdb_urr) {
        const char *svp_str[] = {
            "videoDecoder,svp,all,s",
             "display,svp,all,s"
        };

        rc = nxserverlib_apply_memconfig_str(pPlatformSettings, pMemConfigSettings, svp_str, 2);
        if (rc) {
            print_usage();
            return -1;
        }
    }

    rc = nxserverlib_apply_memconfig_str(pPlatformSettings, pMemConfigSettings, cmdline_settings->memconfig.str, cmdline_settings->memconfig.total);
    if (rc) {
        print_usage();
        return -1;
    }

    rc = nxserverlib_apply_heap_str(pPlatformSettings, pMemConfigSettings, cmdline_settings->heap.str, cmdline_settings->heap.total, settings);
    if (rc) {
        print_usage();
        return -1;
    }

    if (settings->growHeapBlockSize) {
        /* always create a small on-demand heap in the same place as the graphics heap, but that can grow */
        int unused_heap;
        int index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_GRAPHICS);
        if (index == -1 || !pPlatformSettings->heap[index].size) {
            BDBG_ERR(("-growHeapBlockSize requires platform implement NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX"));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        unused_heap = find_unused_heap(pPlatformSettings);
        if (unused_heap == -1) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        settings->heaps.dynamicHeap = (unsigned)unused_heap;
        pPlatformSettings->heap[settings->heaps.dynamicHeap].memcIndex = pPlatformSettings->heap[index].memcIndex;
        pPlatformSettings->heap[settings->heaps.dynamicHeap].subIndex = pPlatformSettings->heap[index].subIndex;
        pPlatformSettings->heap[settings->heaps.dynamicHeap].size = 4096; /* very small, but not zero */
        pPlatformSettings->heap[settings->heaps.dynamicHeap].memoryType = NEXUS_MEMORY_TYPE_MANAGED|NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED|NEXUS_MEMORY_TYPE_DYNAMIC;
    }
    if (cmdline_settings->video.dacBandGapAdjust) {
        for (i=0;i<NEXUS_MAX_VIDEO_DACS;i++) {
            pPlatformSettings->displayModuleSettings.dacBandGapAdjust[i] = cmdline_settings->video.dacBandGapAdjust;
        }
    }
    if (cmdline_settings->video.dacDetection) {
        pPlatformSettings->displayModuleSettings.dacDetection = NEXUS_VideoDacDetection_eOn;
    }
    pPlatformSettings->permissions.userId = cmdline_settings->permissions.userId;
    pPlatformSettings->permissions.groupId = cmdline_settings->permissions.groupId;

    return 0;
}

void nxserver_set_client_heaps(struct nxserver_settings *settings, const NEXUS_PlatformSettings *pPlatformSettings)
{
    NEXUS_PlatformConfiguration platformConfig;
    int index;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_GRAPHICS);
    if (index != -1 && pPlatformSettings->heap[index].size) {
        settings->client.heap[NXCLIENT_DEFAULT_HEAP] = platformConfig.heap[index];
    }
    /* if settings->heaps.clientFullHeap is unset, NXCLIENT_FULL_HEAP ends up being MAIN, which is right */
    settings->client.heap[NXCLIENT_FULL_HEAP] = platformConfig.heap[settings->heaps.clientFullHeap];
    if (settings->client.heap[NXCLIENT_FULL_HEAP] == settings->client.heap[NXCLIENT_DEFAULT_HEAP]) {
        settings->client.heap[NXCLIENT_FULL_HEAP] = NULL;
    }

    index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION);
    if (index != -1 && pPlatformSettings->heap[index].size) {
        settings->client.heap[NXCLIENT_VIDEO_SECURE_HEAP] = platformConfig.heap[index];
    }

    index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_EXPORT_REGION);
    if (index != -1 && pPlatformSettings->heap[index].size) {
        settings->client.heap[NXCLIENT_EXPORT_HEAP] = platformConfig.heap[index];
    }

    /* Untrusted clients will not be able to use the secondary heap (because of simple bounds check), so just don't provide it. */
    if (!settings->certificate.length) {
        index = nxserver_heap_by_type(pPlatformSettings, NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS);
        if (index != -1 && pPlatformSettings->heap[index].size) {
            settings->client.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP] = platformConfig.heap[index];
            if (settings->client.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP] == settings->client.heap[NXCLIENT_DEFAULT_HEAP]) {
                settings->client.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP] = NULL;
            }
        }
    }
    if (settings->heaps.dynamicHeap) {
        settings->client.heap[NXCLIENT_DYNAMIC_HEAP] = platformConfig.heap[settings->heaps.dynamicHeap];
    }

    /* clients only need a single secure graphics heap. prefer highest MEMC where there's usually more room. */
    settings->client.heap[NXCLIENT_SECURE_GRAPHICS_HEAP] = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
}
