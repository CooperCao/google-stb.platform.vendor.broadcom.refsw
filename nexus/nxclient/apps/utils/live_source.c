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
#if NEXUS_HAS_TRANSPORT
#include "live_source.h"
#include "bstd.h"
#include "bdbg.h"
#include "nexus_platform.h"
#include "namevalue.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

BDBG_MODULE(live);

#if NEXUS_HAS_FRONTEND
static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;
    BSTD_UNUSED(param);
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    BDBG_MSG(("frontend %p %s", (void*)frontend, status.lockStatus?"locked":"unlocked"));
}
#endif

void get_default_tune_settings(struct btune_settings *psettings)
{
    memset(psettings, 0, sizeof(*psettings));
    psettings->adc = 0xFFFFFFFF;
    psettings->index = -1;
}

#if NEXUS_HAS_FRONTEND
static int connect_frontend_to_transport(NEXUS_FrontendHandle frontend, NEXUS_ParserBand parserBand)
{
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_FrontendUserParameters userParams;
    int rc;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    if (rc) return BERR_TRACE(rc);

    return 0;
}
#endif

int tune(NEXUS_ParserBand parserBand, NEXUS_FrontendHandle frontend, const struct btune_settings *psettings, bool alreadyTuned)
{
    int rc;

    BSTD_UNUSED(frontend);
    BSTD_UNUSED(alreadyTuned);

    switch (psettings->source) {
#if NEXUS_HAS_FRONTEND
    case channel_source_qam:
        {
        NEXUS_FrontendQamSettings qamSettings;

        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = psettings->freq;
        switch (psettings->mode) {
        case 0: /* auto, same as default 64 */
        case 64:
            qamSettings.mode = NEXUS_FrontendQamMode_e64;
            qamSettings.symbolRate = psettings->symbolRate?psettings->symbolRate:5056900;
            break;
        case 256:
            qamSettings.mode = NEXUS_FrontendQamMode_e256;
            qamSettings.symbolRate = psettings->symbolRate?psettings->symbolRate:5360537;
            break;
        case 1024:
            qamSettings.mode = NEXUS_FrontendQamMode_e1024;
            qamSettings.symbolRate = psettings->symbolRate;
            break;
        default:
            BDBG_ERR(("unknown qam mode %d", psettings->mode));
            rc = -1;
            goto error;
        }
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_callback;
        qamSettings.lockCallback.context = frontend;

        rc = connect_frontend_to_transport(frontend, parserBand);
        if (rc) {rc = BERR_TRACE(rc); goto error;}

        if (alreadyTuned) {
            rc = NEXUS_Frontend_ReapplyTransportSettings(frontend);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }
        else {
            rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }

        BDBG_MSG(("Tuning QAM%d and %d MHz...", psettings->mode?psettings->mode:64, psettings->freq/1000000));

        /* TODO: wait for lock. for now, just start scanning and let it lock during scan */
        }
        break;
    case channel_source_ofdm:
        {
        NEXUS_FrontendOfdmSettings ofdmSettings;

        NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);
        ofdmSettings.frequency = psettings->freq;
        ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
        ofdmSettings.terrestrial = true;
        ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;
        ofdmSettings.mode = psettings->mode;
        ofdmSettings.lockCallback.callback = lock_callback;
        ofdmSettings.lockCallback.context = frontend;
        if(ofdmSettings.mode == NEXUS_FrontendOfdmMode_eDvbt2){
            ofdmSettings.dvbt2Settings.plpMode = true;
            ofdmSettings.dvbt2Settings.plpId = 0;
            ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
        }

        rc = connect_frontend_to_transport(frontend, parserBand);
        if (rc) {rc = BERR_TRACE(rc); goto error;}

        if (alreadyTuned) {
            rc = NEXUS_Frontend_ReapplyTransportSettings(frontend);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }
        else {
            rc = NEXUS_Frontend_TuneOfdm(frontend, &ofdmSettings);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }

        BDBG_MSG(("Tuning OFDM %d MHz...", psettings->freq/1000000));

        /* TODO: wait for lock. for now, just start scanning and let it lock during scan */
        }
        break;

    case channel_source_sat:
        {
        NEXUS_FrontendSatelliteSettings satSettings;

        if (psettings->adc != 0xFFFFFFFF) {
            NEXUS_FrontendSatelliteRuntimeSettings settings;
            NEXUS_Frontend_GetSatelliteRuntimeSettings(frontend, &settings);
            settings.selectedAdc = psettings->adc;
            NEXUS_Frontend_SetSatelliteRuntimeSettings(frontend, &settings);
        }

        NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
        satSettings.frequency = psettings->freq;
        satSettings.mode = psettings->mode;
        satSettings.symbolRate = psettings->symbolRate?psettings->symbolRate:20000000;
        satSettings.lockCallback.callback = lock_callback;
        satSettings.lockCallback.context = frontend;

        rc = connect_frontend_to_transport(frontend, parserBand);
        if (rc) {rc = BERR_TRACE(rc); goto error;}

        if (alreadyTuned) {
            rc = NEXUS_Frontend_ReapplyTransportSettings(frontend);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }
        else {
            NEXUS_FrontendCapabilities capabilities;
            NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
            if (capabilities.diseqc) {
                NEXUS_FrontendDiseqcSettings diseqcSettings;
                NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
                diseqcSettings.toneEnabled = psettings->toneEnabled;
                diseqcSettings.toneMode = NEXUS_FrontendDiseqcToneMode_eEnvelope;
                diseqcSettings.voltage = psettings->diseqcVoltage;
                NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
            }
            rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
        }

        BDBG_MSG(("Tuning SAT %d MHz...", psettings->freq/1000000));

        /* TODO: wait for lock. for now, just start scanning and let it lock during scan */
        }
        break;
#else
    case channel_source_qam:
    case channel_source_ofdm:
    case channel_source_sat:
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
    case channel_source_streamer:
        {
        NEXUS_ParserBandSettings parserBandSettings;
        NEXUS_InputBand inputBand;

        BDBG_MSG(("Scanning streamer %d...", psettings->freq));
        rc = NEXUS_Platform_GetStreamerInputBand(psettings->freq, &inputBand);
        if (rc) return BERR_TRACE(rc);
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        }
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return 0;

error:
    return rc;
}

const char *channel_source_str[channel_source_max] = {"auto", "qam", "ofdm", "sat", "streamer"};

/* Broadcom Irvine lab */
const char *qam_freq_list = "765,777,789";
const char *ofdm_isdbt_freq_list = "473";
const char *ofdm_dvbt_freq_list = "578";
const char *ofdm_dvbt2_freq_list = "602";
const char *sat_freq_list = "1119";

void get_default_channels(struct btune_settings *psettings, const char **freq_list)
{
#if NEXUS_HAS_FRONTEND
    /* auto-detect format */
    if (psettings->source == channel_source_auto) {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_FrontendHandle frontend;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.mode = NEXUS_FrontendAcquireMode_eByIndex;
        settings.index = NEXUS_ANY_ID;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (frontend) {
            NEXUS_FrontendCapabilities cap;
            NEXUS_Frontend_GetCapabilities(frontend, &cap);
            if (cap.qam) {
                psettings->source = channel_source_qam;
            }
            else if (cap.satellite) {
                psettings->source = channel_source_sat;
            }
            else if (cap.ofdm) {
                psettings->source = channel_source_ofdm;
            }
            else {
                psettings->source = channel_source_streamer;
            }
            NEXUS_Frontend_Release(frontend);
        }
        else
        {
            psettings->source = channel_source_streamer;
        }
    }

    if (!*freq_list) {
        switch (psettings->source) {
        case channel_source_qam:  *freq_list = qam_freq_list; break;
        case channel_source_ofdm:
            switch (psettings->mode) {
            case NEXUS_FrontendOfdmMode_eDvbt:
                *freq_list = ofdm_dvbt_freq_list;
                break;
            case NEXUS_FrontendOfdmMode_eIsdbt:
                *freq_list = ofdm_isdbt_freq_list;
                break;
            default:
            case NEXUS_FrontendOfdmMode_eDvbt2:
                *freq_list = ofdm_dvbt2_freq_list;
                break;
            }
            break;
        case channel_source_sat:  *freq_list = sat_freq_list; break;
        default: break;
        }
    }
#else
    psettings->source = channel_source_streamer;
    *freq_list = NULL;
#endif
}

#include "tshdrbuilder.h"
#include "nexus_packetsub.h"

struct bpsi_injection {
    NEXUS_PacketSubHandle packetSub;
    NEXUS_PidChannelHandle pidChannel;
};

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH 189
#define PAT_PMT_PAIR_SIZE (188*2)

static int build_psi(unsigned pmtPid, void *buffer, unsigned size, unsigned *pSizeUsed, const tspsimgr_scan_results *pscan_results, unsigned program)
{
    uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    unsigned streamNum;
    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program pat_program;
    TS_PMT_stream pmt_stream;
    uint8_t *pat, *pmt;

    if (size < PAT_PMT_PAIR_SIZE) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&pat_program, 1, pmtPid);
    TS_PAT_addProgram(&patState, &pmtState, &pat_program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    if (pscan_results->program_info[program].num_video_pids) {
        unsigned vidStreamType;
        switch(pscan_results->program_info[program].video_pids[0].codec) {
        case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
        case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
        case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
        case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
        default: return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        TS_PMT_stream_Init(&pmt_stream, vidStreamType, pscan_results->program_info[program].video_pids[0].pid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    if (pscan_results->program_info[program].num_audio_pids) {
        unsigned audStreamType;
        switch(pscan_results->program_info[program].audio_pids[0].codec) {
        case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
        case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
        case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
        case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
        case NEXUS_AudioCodec_eLpcm1394:     audStreamType = 0x83; break;
        default: return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }

        TS_PMT_stream_Init(&pmt_stream, audStreamType, pscan_results->program_info[program].audio_pids[0].pid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    if (pscan_results->program_info[program].num_other_pids) {
        /* add other[0] for DVR crypto detection */
        TS_PMT_stream_Init(&pmt_stream, TS_PSI_ST_14496_1_FlexMuxSection /*other*/, pscan_results->program_info[program].other_pids[0].pid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

    pat = buffer;
    pmt = pat+188;

    pat[0] = 0x47;
    pat[1] = 0x40; /* TEI = 0, Payload Unit Start = 1, Transport Priority = 0, 13 bit-pid# = 0 */
    pat[2] = 0x00;
    pat[3] = 0x10; /* scrambling = 0, adaptation field = 0, continuity counter = 0 */
    pat[4] = 0x00; /* pointer = 0 */
    BKNI_Memcpy(pat + 5, pat_pl_buf, pat_pl_size); /* PAT table */
    BKNI_Memset(pat + 5 + pat_pl_size, 0xff, 188 - 5 - pat_pl_size); /* stuffing bytes */

    pmt[0] = 0x47;
    pmt[1] = 0x40 | ((pmtPid >> 8) & 0x1f); /* TEI = 0, PUSI= 1, TP=0, 13-bit pid# */
    pmt[2] = pmtPid & 0xff;
    pmt[3] = 0x11; /* scrambling = 0, adaptation field = 0, continuity counter = 1 */
    pmt[4] = 0x00; /* pointer = 0 */
    BKNI_Memcpy(pmt + 5, pmt_pl_buf, pmt_pl_size); /* PMT table */
    BKNI_Memset(pmt + 5 + pmt_pl_size, 0xff, 188 - 5 - pmt_pl_size); /* stuffing bytes */

    *pSizeUsed = PAT_PMT_PAIR_SIZE;

    return 0;
}

NEXUS_PidChannelHandle bpsi_injection_get_pid_channel(bpsi_injection_t handle)
{
    return handle->pidChannel;
}

bpsi_injection_t bpsi_injection_open(NEXUS_ParserBand parserBand, const tspsimgr_scan_results *pscan_results, unsigned program)
{
    bpsi_injection_t handle;
    int rc;
    uint8_t *pkt;
    unsigned size;
    NEXUS_PacketSubOpenSettings packetSubOpenSettings;
    NEXUS_PacketSubSettings packetSubSettings;
    NEXUS_PidChannelSettings pidChannelSettings;

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    BKNI_Memset(handle, 0, sizeof(*handle));

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    handle->pidChannel = NEXUS_PidChannel_Open(parserBand, 0x1ffe, &pidChannelSettings);

    NEXUS_PacketSub_GetDefaultOpenSettings(&packetSubOpenSettings);
    packetSubOpenSettings.fifoSize = 1024;
    handle->packetSub = NEXUS_PacketSub_Open(NEXUS_ANY_ID, &packetSubOpenSettings);

    NEXUS_PacketSub_GetSettings(handle->packetSub, &packetSubSettings);
    packetSubSettings.pidChannel = handle->pidChannel;
    packetSubSettings.loop = false;
    rc = NEXUS_PacketSub_SetSettings(handle->packetSub, &packetSubSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = NEXUS_PacketSub_GetBuffer(handle->packetSub, (void **)&pkt, &size);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = build_psi(0x55, pkt, size, &size, pscan_results, program);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = NEXUS_PacketSub_WriteComplete(handle->packetSub, size);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = NEXUS_PacketSub_Start(handle->packetSub);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return handle;

error:
    bpsi_injection_close(handle);
    return NULL;
}

void bpsi_injection_close(bpsi_injection_t handle)
{
    if (handle->packetSub) {
        NEXUS_PacketSub_Close(handle->packetSub);
    }
    if (handle->pidChannel) {
        NEXUS_PidChannel_Close(handle->pidChannel);
    }
    BKNI_Free(handle);
}

void *bdevice_mem_alloc(unsigned size, bool full)
{
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_MemoryAllocationSettings settings;
    unsigned i;
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        void *buffer;
        int rc;
        if (!clientConfig.heap[i]) continue;
        if (full) {
            NEXUS_MemoryStatus status;
            rc = NEXUS_Heap_GetStatus(clientConfig.heap[i], &status);
            if (rc || status.memoryType != NEXUS_MemoryType_eFull) continue;
        }
        settings.heap = clientConfig.heap[i];
        rc = NEXUS_Memory_Allocate(size, &settings, &buffer);
        if (!rc) return buffer;
    }
    return NULL;
}

void bdevice_mem_free(void *ptr)
{
    NEXUS_Memory_Free(ptr);
}

/*****************************************************/

struct bchannel_scan
{
    NEXUS_FrontendHandle frontend;
    NEXUS_ParserBand parserBand;
    tspsimgr_t tspsimgr;
    bchannel_scan_start_settings settings;
};

void bchannel_scan_get_default_start_settings(struct bchannel_scan_start_settings *psettings)
{
    memset(psettings, 0, sizeof(*psettings));
}

bchannel_scan_t bchannel_scan_start(const struct bchannel_scan_start_settings *psettings)
{
    bchannel_scan_t scan;
    int rc;

    scan = BKNI_Malloc(sizeof(*scan));
    if (!scan) return NULL;
    memset(scan, 0, sizeof(*scan));

    scan->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    if (!scan->parserBand) {
        goto error;
    }

    scan->tspsimgr = tspsimgr_create();
    if (!scan->tspsimgr) {
        goto error;
    }

    rc = bchannel_scan_restart(scan, psettings);
    if (rc) goto error;

    return scan;

error:
    bchannel_scan_stop(scan);
    return NULL;
}

int bchannel_scan_restart(bchannel_scan_t scan, const bchannel_scan_start_settings *psettings)
{
    tspsimgr_scan_settings scan_settings;
    int rc;

    if (psettings->tune.source == channel_source_auto) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (psettings->tune.source != channel_source_streamer) {
#if NEXUS_HAS_FRONTEND
        if (scan->frontend) {
            NEXUS_FrontendCapabilities cap;
            bool release = false;
            NEXUS_Frontend_GetCapabilities(scan->frontend, &cap);
            switch (psettings->tune.source) {
            case channel_source_qam: if (!cap.qam) release = true; break;
            case channel_source_ofdm: if (!cap.ofdm) release = true; break;
            case channel_source_sat: if (!cap.satellite) release = true; break;
            default: release = true; break;
            }
            if (release) {
                NEXUS_Frontend_Release(scan->frontend);
                scan->frontend = NULL;
            }
        }
        if (!scan->frontend) {
            scan->frontend = acquire_frontend(&psettings->tune);
            if (!scan->frontend) {
                /* no WRN. we're just out of frontends. */
                return NEXUS_NOT_AVAILABLE;
            }
        }
#else
        return NEXUS_NOT_AVAILABLE;
#endif
    }

    rc = tune(scan->parserBand, scan->frontend, &psettings->tune, false);
    if (rc) return BERR_TRACE(rc);

    /* TODO: also fire scan_done if tune cannot acquire */
    tspsimgr_get_default_start_scan_settings(&scan_settings);
    scan_settings.parserBand = scan->parserBand;
    scan_settings.scan_done = psettings->scan_done;
    scan_settings.context = psettings->context;
    rc = tspsimgr_start_scan(scan->tspsimgr, &scan_settings);
    if (rc) return BERR_TRACE(rc);

    scan->settings = *psettings;

    return 0;
}

int bchannel_scan_get_results(bchannel_scan_t scan, tspsimgr_scan_results *presults)
{
    return tspsimgr_get_scan_results(scan->tspsimgr, presults)?NEXUS_NOT_AVAILABLE:0;
}

void bchannel_scan_get_resources(bchannel_scan_t scan, NEXUS_FrontendHandle *pFrontend, NEXUS_ParserBand *pParserBand)
{
    *pFrontend = scan->frontend;
    *pParserBand = scan->parserBand;
}

void bchannel_scan_stop(bchannel_scan_t scan)
{
    if (scan->tspsimgr) {
        tspsimgr_destroy(scan->tspsimgr);
    }
    if (scan->parserBand) {
        NEXUS_ParserBand_Close(scan->parserBand);
    }
#if NEXUS_HAS_FRONTEND
    if (scan->frontend) {
        NEXUS_Frontend_Release(scan->frontend);
    }
#endif
    BKNI_Free(scan);
}

void bchannel_source_print(char *str, unsigned n, const struct btune_settings *psettings)
{
    switch (psettings->source) {
#if NEXUS_HAS_FRONTEND
    case channel_source_ofdm:
        snprintf(str, n, "%s %s %dMhz", channel_source_str[psettings->source], lookup_name(g_ofdmModeStrs, psettings->mode), psettings->freq/1000000);
        break;
    case channel_source_sat:
        snprintf(str, n, "%s %s %dMhz", channel_source_str[psettings->source],  lookup_name(g_satModeStrs, psettings->mode), psettings->freq/1000000);
        break;
    case channel_source_qam:
        snprintf(str, n, "%s %dMhz", channel_source_str[psettings->source], psettings->freq/1000000);
        break;
#endif
    case channel_source_streamer:
        snprintf(str, n, "Streamer %d", psettings->freq);
        break;
    default:
        snprintf(str, n, "unknown %d", psettings->source);
        break;
    }
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs transport)!\n");
    return 0;
}
#endif

NEXUS_FrontendHandle acquire_frontend(const struct btune_settings *psettings)
{
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);

    if (psettings->index != -1) {
        settings.mode = NEXUS_FrontendAcquireMode_eByIndex;
        settings.index = psettings->index;
    }
    else {
        settings.mode = NEXUS_FrontendAcquireMode_eByCapabilities;
        settings.capabilities.qam = psettings->source == channel_source_qam;
        settings.capabilities.ofdm = psettings->source == channel_source_ofdm;
        settings.capabilities.satellite = psettings->source == channel_source_sat;
    }
    return NEXUS_Frontend_Acquire(&settings);
#else
    BSTD_UNUSED(psettings);
#endif
    return NULL;
}
