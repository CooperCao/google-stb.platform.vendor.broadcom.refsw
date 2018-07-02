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
#ifndef WIFI_SA_H
#define WIFI_SA_H

/* to be included by dms and plugin */
#include "wifi.h"

/* spectrum analyzer I/Q pair */
typedef struct wifi_sample_pair {
	int sample_pair_I;
	int sample_pair_Q;
} wifi_sample_pair;

/* Has to match these
#define SC_MODE_0_sd_adc                0
#define SC_MODE_1_sd_adc_5bits          1
#define SC_MODE_2_cic0                  2
#define SC_MODE_3_cic1                  3
#define SC_MODE_4s_rx_farrow_1core      4
#define SC_MODE_4m_rx_farrow            5
#define SC_MODE_5_iq_comp               6
#define SC_MODE_6_dc_filt               7
#define SC_MODE_7_rx_filt               8
#define SC_MODE_8_rssi                  9
#define SC_MODE_9_rssi_all              10
#define SC_MODE_10_tx_farrow            11
#define SC_MODE_11_gpio                 12
#define SC_MODE_12_gpio_trans           13
#define SC_MODE_14_spect_ana            14
#define SC_MODE_5s_iq_comp              15
#define SC_MODE_6s_dc_filt              16
#define SC_MODE_7s_rx_filt              17
#define SC_MODE_0_28nm_sar_adc_nti      20
#define SC_MODE_2_28nm_dcc_out          21
#define SC_MODE_3_28nm_farrow_in        22
#define SC_MODE_4_28nm_farrow_out       23
 */

typedef enum wifi_sa_sample_type {
	SC_CH_I,
	SC_CH_Q,
	SC_CH_IQ,
	SC_CH_PSD,
	SC_CH_FFT,
	SC_CH_UNK
} wifi_sa_sample_type_t;

typedef struct wifi_sa_params {
	unsigned int size;	/* max size of the collect buffer */
	unsigned int source;	/* m variable in sample collect */
	wifi_sa_sample_type_t channel;	/* I:0, Q:1, I/Q:2 ... */
} wifi_sa_params_t;

typedef struct wifi_psd {
	unsigned char* bytes;
	unsigned long  count;
	const wifi_sa_params_t* params;
} wifi_psd_t;

/* spectrum analyzer plugin state */
typedef struct wifi_sa {
	wifi_sa_sample_type_t sample_type;
	wifi_sample_pair* samples;
	int count;
} wifi_sa;

int wifi_get_sa_data(const wifi_sa_params_t* params, wifi_psd_t *data);
unsigned int wifi_sa_convert_url_to_params(char* url, wifi_sa_params_t* params);
bool wifi_sa_params_validate(const wifi_sa_params_t* params);

int wifi_sa_convert_to_xml(const char* filter, bmon_wifi_t * wifi_out,char* output, unsigned int output_size);
int wifi_sa_convert_to_json(
    wifi_psd_t *wifi_data,
    char       *payload,
    int        payload_size,
    unsigned int tv_sec,
    unsigned int tv_usec);
#endif /* WIFI_SA_H */
