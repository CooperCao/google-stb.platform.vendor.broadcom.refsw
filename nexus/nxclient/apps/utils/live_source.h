/******************************************************************************
 *    (c)2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/
#ifndef LIVE_SOURCE_H__
#define LIVE_SOURCE_H__

#include "nexus_types.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_parser_band.h"
#endif
#include "tspsimgr3.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#else
/* stub version of NEXUS_FrontendHandle */
#include "nexus_platform.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
NOTE: This API is only example code. It is subject to change and
is not supported as a standard reference software deliverable.
**/

enum channel_source {
    channel_source_auto,
    channel_source_qam,
    channel_source_ofdm,
    channel_source_sat,
    channel_source_streamer,
    channel_source_max
};

/**
if qam.freq is non-zero, it is used.
if no other frontend type is set, streamer is used.
**/
struct btune_settings {
    enum channel_source source;
    unsigned freq; /* in Hz */
    unsigned mode;
    unsigned symbolRate;
    unsigned adc;
    bool toneEnabled;
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendDiseqcVoltage diseqcVoltage;
    NEXUS_FrontendSatelliteNetworkSpec satNetworkSpec;
#endif
};

void get_default_tune_settings(struct btune_settings *psettings);
int  tune(NEXUS_ParserBand parserBand, NEXUS_FrontendHandle frontend, const struct btune_settings *psettings,
    bool alreadyTuned /* don't retune frontend; just configure parser band. */
    );

void get_default_channels(struct btune_settings *psettings, const char **freq_list);

/**
channel scan
**/
typedef struct bchannel_scan *bchannel_scan_t;
typedef struct bchannel_scan_start_settings
{
    struct btune_settings tune;
    void (*scan_done)(void *context);
    void *context;
} bchannel_scan_start_settings;
/* return NULL if unable to start scan for lack of resources */
void bchannel_scan_get_default_start_settings(bchannel_scan_start_settings *psettings);
bchannel_scan_t bchannel_scan_start(const bchannel_scan_start_settings *psettings);
int bchannel_scan_restart(bchannel_scan_t, const bchannel_scan_start_settings *psettings);
/* return NEXUS_NOT_AVAILABLE if still pending */
int bchannel_scan_get_results(bchannel_scan_t scan, tspsimgr_scan_results *presults);
/* allow caller to use tuned/conencted frontend resources. they are not closed until stop. */
void bchannel_scan_get_resources(bchannel_scan_t scan, NEXUS_FrontendHandle *pFrontend, NEXUS_ParserBand *pParserBand);
/* stop scan and free frontend resources */
void bchannel_scan_stop(bchannel_scan_t scan);

void bchannel_source_print(char *str, unsigned n, const struct btune_settings *psettings);

/**
PSI injection for encrypted record using packetsub
**/
typedef struct bpsi_injection *bpsi_injection_t;
bpsi_injection_t bpsi_injection_open(NEXUS_ParserBand parserBand, const tspsimgr_scan_results *pscan_results, unsigned program);
NEXUS_PidChannelHandle bpsi_injection_get_pid_channel(bpsi_injection_t handle);
void bpsi_injection_close(bpsi_injection_t handle);

/* helper functions to alloc device memory with or without driver-side memory mapping */
void *bdevice_mem_alloc(unsigned size, bool full);
void  bdevice_mem_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* LIVE_SOURCE_H__ */
