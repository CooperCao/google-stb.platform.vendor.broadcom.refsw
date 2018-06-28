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
* WARRIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRIES
* OF TITLE, MERCHABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
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
/* Coomon file to be shared by plugin and dms parser*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stddef.h>

#include "bmon_utils.h"
#include "wifi_bwl_interface.h"

#include "wifi_convert.h"
#include "wifi_sa.h"

#undef  SA_FILL_PAYLOAD			/* fill into payload when we have streaming support */
#define WIFI_SAMPLECOLLECT_MAXLEN	(10240)		/* WLAN sample collect max buffer */
#define WIFI_MAX_SC_MODES		(23)		/* SC_MODE_4_28nm_farrow_out */
#define WIFI_SA_DEFAULT_SRC		(5)		/* default source */
#define WIFI_SA_DEFAULT_CHN		(SC_CH_IQ)	/* I/Q */

#define DEPOSIT_PARAM(str, idx, val, ptr_type) \
	(*(ptr_type*)((unsigned char*)str + subcmds[idx].off)) = val;

#define ARR_SIZEOF(arr) (sizeof(arr)/sizeof(arr[0]))
#define MAX_HEADER_SIZE	(1024)

typedef enum sep {
	SEP_AMP_IDX,
	SEP_EQ_IDX,
	SEP_QM_IDX
} sep_t;
static const char* seps[] = {"&", "=", "?"};

typedef enum subcmd_idx {
	SUBCMD_DEF_SOURCE,
	SUBCMD_DEF_CHANNEL,
	SUBCMD_DEF_SIZE,
	SUBCMD_DEF_UNK
} subcmd_idx_t;

typedef enum subcmd_type {
	SUBCMD_TYPE_CHAR,
	SUBCMD_TYPE_INT,
	SUBCMD_TYPE_LONG,
	SUBCMD_TYPE_STR,
	SUBCMD_TYPE_UNK
} subcmd_type_t;

typedef struct subcmd {
	subcmd_idx_t idx;
	const char* name;
	unsigned int off;
	subcmd_type_t type;
} subcmd_t;

static const subcmd_t subcmds[] = {
        { SUBCMD_DEF_SOURCE,  "source",  offsetof(wifi_sa_params_t, source),  SUBCMD_TYPE_INT},
	{ SUBCMD_DEF_CHANNEL, "channel", offsetof(wifi_sa_params_t, channel), SUBCMD_TYPE_INT},
	{ SUBCMD_DEF_SIZE,    "size",    offsetof(wifi_sa_params_t, size),    SUBCMD_TYPE_INT}
};

int wifi_sa_convert_to_xml(
    const char     *filter,
    bmon_wifi_t *wifi_out,
    char           *output,
    unsigned int    output_size
    )
{
    return( 0 );
}

/* Get the actual sa data from bmon/BWL
 */
int wifi_get_sa_data(const wifi_sa_params_t* params, wifi_psd_t *data)
{
	int               tRetCode = BWL_ERR_SUCCESS;
	WiFiSamples_t     tSamples;

	tSamples.buff = malloc(WIFI_SAMPLECOLLECT_MAXLEN);
	if (tSamples.buff == NULL) {
		fprintf(stderr, "Failed to allocate dump buffer of %d bytes\n",
				WIFI_SAMPLECOLLECT_MAXLEN);
		return ENOMEM;
	}

	if ((tRetCode == BWL_ERR_SUCCESS) && ((tRetCode = bmon_wifi_get_samples(WIFI_INTERFACE_NAME,
						params->source, &tSamples)) != BWL_ERR_SUCCESS)) {

		fprintf( stderr, "%s:%u: bstbmon_wifi_get_samples() failed ... rc %d \n",
				__FUNCTION__, __LINE__, tRetCode );
	}

    data->count = tSamples.count;
    data->bytes = tSamples.buff;
    data->params = params;
}

static void
wifi_sa_params_init(wifi_sa_params_t* params)
{
	params->size = WIFI_SAMPLECOLLECT_MAXLEN;
	params->source = WIFI_SA_DEFAULT_SRC;
	params->channel = WIFI_SA_DEFAULT_CHN;
}

/* Convert commandline URL/argv to meaningful params struct
 */
unsigned int
wifi_sa_convert_url_to_params(char* url, wifi_sa_params_t* params)
{
	unsigned int pc = 0, i = 0; /* param count */
	char name[16], *eq_pos = NULL, *val = NULL;

	wifi_sa_params_init(params);
	char* url_pos = strstr(url, seps[SEP_QM_IDX]);
	char* url_loc = url_pos ? url_pos + 1 : url;	/* handle existence of ? in the input */
	char* tok = strtok((char*)url_loc, seps[SEP_AMP_IDX]);

	while (tok != NULL) {
		if (eq_pos = strstr(tok, seps[SEP_EQ_IDX])) {
			memset(name, 0, sizeof(name));
			strncpy(name, tok, eq_pos - tok);
			val = eq_pos + 1;
			for (i = 0; i < ARR_SIZEOF(subcmds); i++) {
				if (strncmp(name, subcmds[i].name, sizeof(name)) == 0) {
					switch (subcmds[i].type) {
						case SUBCMD_TYPE_INT:
							DEPOSIT_PARAM(params, i, atoi(val), int);
							break;
						case SUBCMD_TYPE_LONG:
							DEPOSIT_PARAM(params, i, atol(val), long);
							break;
						case SUBCMD_TYPE_STR:
						case SUBCMD_TYPE_CHAR:
						case SUBCMD_TYPE_UNK:
							/* TODO: support other types */
						default:
							break;
					}
					pc++;
					break;
				}
			}
		}
		tok = strtok(NULL, seps[SEP_AMP_IDX]);
	}

	return (pc);
}

/* Validate input params for proper ranging
 */
bool
wifi_sa_params_validate(const wifi_sa_params_t* params)
{
	bool ret = true;

#if defined(WIFI_PLUGIN_DEBUG) || 0
	fprintf(stderr, "input params: channel=%d source=%d size=%d\n",
		params.channel, params.source, params.size);
#endif /* WIFI_PLUGIN_DEBUG */

	if ((params->size > WIFI_SAMPLECOLLECT_MAXLEN) ||
		(params->source > WIFI_MAX_SC_MODES) ||
		(params->channel > SC_CH_UNK)) {
		fprintf(stderr, "invalid params size:%d channel:%d source:%d\n",
			params->size, params->channel, params->source);
		ret = false;
	}

	return (ret);
}

/**
 *  Function: This function will fetch the IQ sample data and print it in JSON format
 **/
int wifi_sa_convert_to_json(
    wifi_psd_t     *wifi_data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec)
{

    unsigned int count = 0;
    unsigned char* ptr = wifi_data->bytes;
    char   one_line[256];

    wifi_timestamp_convert_json(payload, payload_size, tv_sec, tv_usec);
    if (strlen(payload) > 4) {strncat( payload, ",", payload_size ); }

    snprintf(one_line, sizeof(one_line), "\"psd count\":\"%d\",\n",
		    wifi_data->count);
    strncat(payload, one_line, payload_size);

    snprintf(one_line, sizeof(one_line), "\"source\":\"%d\",\n",
		    wifi_data->params->source);
    strncat(payload, one_line, payload_size);

    snprintf(one_line, sizeof(one_line), "\"channel\":\"%d\",\n",
		    wifi_data->params->channel);
    strncat(payload, one_line, payload_size);

    snprintf(one_line, sizeof(one_line), "\"bytes\":\"\n");
    strncat( payload, one_line, payload_size );
    printf("%s\n", payload);
    memset(payload, 0, payload_size);

    while (count < wifi_data->count) {
	    if (count == MAX_HEADER_SIZE) {
		    snprintf (one_line, sizeof(one_line), "\n");
		    strncat(payload, one_line, payload_size);
		    break;
	    }
#if defined(SA_FILL_PAYLOAD)
	    if (!(count % 16) && count) {
		    snprintf(one_line, sizeof(one_line), "\n");
		    strncat(payload, one_line, payload_size);
	    }
	    if (!(count % 8) && count) {
		    snprintf(one_line, sizeof(one_line), "  ");
		    strncat(payload, one_line, payload_size);
	    }
	    snprintf(one_line, sizeof(one_line), "%.2x ", *ptr);
	    strncat(payload, one_line, payload_size);
#else
	    printf("%.2x ", *ptr);
#endif
	    ptr++; count++;
    }

    strncat(payload, "\"", payload_size);
    strncat(payload, " }", payload_size);

    free(wifi_data->bytes);
    wifi_data->bytes = NULL;

    return(0);

} /* wifi_sa_convert_to_json */
