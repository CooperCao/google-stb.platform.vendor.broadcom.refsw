/******************************************************************************
* Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
#ifndef WIFI_CONVERT_H
#define WIFI_CONVERT_H

/* to be included by dms and plugin */
#include "wifi.h"
#include "plugin_header.h"
#include "bmon_defines.h"
#include "bmon_json.h"
#include "wifi_bwl_interface.h"

#define PATH_BASE                          NULL

typedef enum Wifi_Modes
{
    wifi_none  =   0x000,
    wifi_a     =   0x001,
    wifi_b     =   0x002,
    wifi_g     =   0x004,
    wifi_n     =   0x008,
    wifi_ac    =   0x010
} Wifi_Modes_t;

int wifi_convert_to_xml(
    const char* filter,
    bmon_wifi_t * wifi_out,
    char* output,
    unsigned int output_size);
int wifi_convert_to_json(
    const char          *filter,
    bmon_wifi_t      *wifi_data,
    char                *payload,
    int                  payload_size,
    unsigned int         tv_sec,
    unsigned int         tv_usec
    );
int wifi_timestamp_convert_json(
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec
    );
int wifi_txpkt_stats_convert_to_json(
    struct wifi_tx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_rxpkt_stats_convert_to_json(
    struct wifi_rx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_trigger_spectrum_data_convert_to_json(
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );

int wifi_spectrum_data_convert_to_json(
    struct spectrum_data_t *data,
    unsigned int    num,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_rxampdu_stats_convert_to_json(
    struct wifi_rxampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_txampdu_stats_convert_to_json(
    struct wifi_txampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_ampdu_chart_convert_to_json(
    struct wifi_ampdu_chart_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    );
int wifi_command_list_convert_to_json(
    const char     *data[],
    unsigned int    num,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter
    );
int wifi_all_convert_to_json(
    struct spectrum_data_t *spec_data,
    unsigned int    num,
    struct wifi_tx_stats_t *txstats_data,
    struct wifi_rx_stats_t *rxstats_data,
    struct wifi_txampdu_stats_t *txampdu_data,
    struct wifi_rxampdu_stats_t *rxampdu_data,
    struct wifi_ampdu_chart_t *ampdu_data,
    struct wifi_current_stats_t *currstat_data,
    struct wifi_power_stats_t *power_data,
    struct wifi_chanim_stats_t *chanim_data,
    unsigned int    chanim_count,
    struct wifi_connected_sta_t *connected_data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    const char*     filter
    );
#ifdef WIFI_DE
int wifi_de_convert_to_json(
    struct wifi_dataelements_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter
    );
#endif
int wifi_current_stats_convert_to_json(
    struct wifi_current_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char *    filter,
    cJSON *         parentobj
);
int wifi_power_stats_convert_to_json(
    struct wifi_power_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
);
int wifi_chanim_stats_convert_to_json(
    struct wifi_chanim_stats_t *data,
    unsigned int    count,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
);
int wifi_connected_sta_list_convert_to_json(
    wifi_connected_sta_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
);
#endif /* WIFI_CONVERT_H */
