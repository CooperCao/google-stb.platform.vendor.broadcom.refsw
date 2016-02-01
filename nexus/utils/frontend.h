/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#ifndef FRONTEND_H
#define FRONTEND_H

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#ifdef NEXUS_FRONTEND_4538
#include "nexus_frontend_4538.h"
#endif
#endif
#include "nexus_parser_band.h"
#include "cmdline_args.h"

typedef enum FrontendSource {
    FrontendSource_eVsb,
    FrontendSource_eQam,
    FrontendSource_eOfdm,
    FrontendSource_eSat,
    FrontendSource_eMax
} FrontendSource;

typedef struct FrontendSettings {
    FrontendSource source;
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendHandle handle;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendVsbSettings vsbSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_FrontendOfdmSettings ofdmSettings;
#endif
    struct {
#if NEXUS_HAS_FRONTEND
        NEXUS_FrontendVsbMode vsbMode;
        NEXUS_FrontendQamMode qamMode;
        NEXUS_FrontendOfdmMode ofdmMode;
        NEXUS_FrontendSatelliteMode satMode;
        NEXUS_FrontendDiseqcVoltage diseqcVoltage;
        NEXUS_FrontendSatelliteNetworkSpec satNetworkSpec;
        int toneEnabled;
        unsigned ksyms; /* in Ksyms */
        unsigned freq; /* in MHz */
        unsigned bandwidth; /* in MHz */
        int plpid;
        NEXUS_FrontendDvbt2Profile dvbt2Profile;
        int adc;
#else
        int dummy;
#endif
    } opts;
} FrontendSettings;


void frontend_init(FrontendSettings *frontendSettings);
int frontend_selected(FrontendSettings *settings);
int frontend_check_capabilities(FrontendSettings *settings, NEXUS_PlatformConfiguration *platformConfig);
int frontend_check_usage(FrontendSettings *settings);
void frontend_set_settings(FrontendSettings *settings, struct common_opts_t *common);
void frontend_set_parserbandsettings(FrontendSettings *settings, NEXUS_ParserBandSettings *parserBandSettings);
int frontend_tune(FrontendSettings *frontendSettings);
int frontend_getStrength(FrontendSettings *frontendSettings, int *pStrength, unsigned *pLevelPercent, unsigned *pQualityPercent);
void frontend_shutdown(FrontendSettings *frontendSettings);
#endif
