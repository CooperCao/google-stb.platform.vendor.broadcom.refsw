/*
 * wl adps command module
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifdef WIN32
#include <windows.h>
#endif
#include <wlioctl.h>
#include <wlioctl_utils.h>

#if	defined(DONGLEBUILD)
#include <typedefs.h>
#include <osl.h>
#endif

/* Because IL_BIGENDIAN was removed there are few warnings that need
 * to be fixed. Windows was not compiled earlier with IL_BIGENDIAN.
 * Hence these warnings were not seen earlier.
 * For now ignore the following warnings
 */
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)
#endif

#include <bcmutils.h>
#include <bcmendian.h>
#include "wlu_common.h"
#include "wlu.h"

#define ADPS_MAX_STEP_ENTRY	3
#define ADPS_STEP_ELEMENT_NUM	3

#define ADPS_RX			0
#define ADPS_TX			1

static cmd_func_t wl_adps;
#ifdef BCMINTDBG
static cmd_func_t wl_adps_params;
#endif	/* BCMINTDBG */
static cmd_func_t wl_adps_rssi;

static cmd_t wl_adps_cmds[] = {
	{"adps", wl_adps, WLC_GET_VAR, WLC_SET_VAR,
	"Enable/Disable ADaptive Power Save\n"
	"\tUsage: wl adps [band] <mode>\n\t[band] a|b|2g|5g\n"
	"\t<mode> 0: Disable, 1: PM2 step 1 is a default operation\n"
	"\t       2: PM1 is a default operation, 3: PM2 steps only working\n"
	},
#ifdef BCMINTDBG
	{ "adps_params", wl_adps_params, WLC_GET_VAR, WLC_SET_VAR,
	"Get/Set ADaptive Power Save parameters\n"
	"\tUsage: wl adps_params [band] <mode> <pspoll_prd> <number of step> "
	"<adps step #1> ... <adps step #n>\n"
	"\t[band] a|b|2g|5g\n"
	"\t<mode> 0: Disable\n"
	"\t       1: PM2 step 1 is a default operation\n"
	"\t       2: PM1 is a default operation\n"
	"\t       3: PM2 steps only working\n"
	"\t<pspoll_prd> pspoll frame sending period\n"
	"\t<number of step> Total number of adps pm step (max = 3)\n"
	"\t<adps step> Consist of 3 values\n"
	"\t\t<pm2_sleep_ret time> <Threshold in # of rx packet> <Threshold in # of tx packet\n"
	"\t\n\nex) wl adps_params a|b|2g|5g 1 20 3 10 25 25 60 80 80 200 200 200\n"
	},
#endif	/* BCMINTDBG */
	{ "adps_rssi", wl_adps_rssi, WLC_GET_VAR, WLC_SET_VAR,
	"Get/Set adps operation pause rssi threshold\n"
	"\tUsage: wl adps_rssi [band] <high rssi threshold> <low rssi threshold>\n"
	"\t[band] a|b|2g|5g\n"
	"\t<high rssi threshold> rssi value to resume adps operation\n"
	"\t<low rssi threshold> rssi value to pause adps operation\n"
	},
	{ NULL, NULL, 0, 0, NULL }
};

static char *buf;

/* module initialization */
void
wluc_adps_module_init(void)
{
	/* get thr global buf */
	buf = wl_get_buf();

	/* register adps commands */
	wl_module_cmds_register(wl_adps_cmds);
}

static int
wl_adps(void *wl, cmd_t *cmd, char **argv)
{
	int ret = BCME_OK;
	void *ptr = NULL;
	wl_adps_params_v1_t *adps = NULL;

	if ((adps = malloc(ADPS_PARAMS_V1_LEN)) == NULL) {
		fprintf(stderr, "Err alloc %u bytes\n", (unsigned int)ADPS_PARAMS_V1_LEN);
		return BCME_NOMEM;
	}
	memset(adps, 0, ADPS_PARAMS_V1_LEN);

	++argv;
	if (*argv && (!stricmp(*argv, "b") || !stricmp(*argv, "2g"))) {
		adps->band = WLC_BAND_2G;
	} else if (*argv && (!stricmp(*argv, "a") || !stricmp(*argv, "5g"))) {
		adps->band = WLC_BAND_5G;
	} else {
		ret = BCME_BADARG;
		goto exit;
	}

	if (!*++argv) {
		if ((ret = wlu_var_getbuf(wl, cmd->name, adps, ADPS_PARAMS_V1_LEN, &ptr)) < 0) {
			goto exit;
		}
		memcpy(adps, ptr, ADPS_PARAMS_V1_LEN);

		if (adps->ver != ADPS_VERSION) {
			ret = BCME_VERSION;
			goto exit;
		}
		printf("%d\n", adps->mode);
	} else {
		adps->mode = strtoul(*argv, NULL, 0);

		if (*++argv) {
			ret = BCME_BADARG;
			goto exit;
		}
		adps->ver = ADPS_VERSION;
		adps->len = ADPS_PARAMS_V1_LEN;

		ret = wlu_var_setbuf(wl, cmd->name, adps, sizeof(*adps));
	}

exit:
	free(adps);
	return ret;
}

#ifdef BCMINTDBG
static int
wl_adps_params(void *wl, cmd_t *cmd, char **argv)
{
	int i;
	int argc;
	int ret = BCME_OK;

	uint8 band;
	uint8 param_cnt = 0;

	uint len;
	wl_adps_step_params_v1_t *params = NULL;

	char *endptr = NULL;
	void *ptr = NULL;

	++argv;
	argc = ARGCNT(argv);

	len = ADPS_STEP_PARAMS_V1_FIXED_LEN + ADPS_STEP_LEN * ADPS_MAX_STEP_ENTRY;
	if ((params = malloc(len)) == NULL) {
		fprintf(stderr, "Error allocating %u bytes for adps params\n", len);
		return BCME_NOMEM;
	}
	memset(params, 0, len);

	if (*argv && (!stricmp(*argv, "b") || !stricmp(*argv, "2g"))) {
		band = WLC_BAND_2G;
	}
	else if (*argv && (!stricmp(*argv, "a") || !stricmp(*argv, "5g"))) {
		band = WLC_BAND_5G;
	}
	else {
		ret = BCME_BADARG;
		goto exit;
	}

	params->band = band;
	param_cnt++;
	if (!*++argv) {
		if ((ret = wlu_var_getbuf(wl, cmd->name, params, len, &ptr)) != BCME_OK) {
			printf("Fail to get adps params\n");
			goto exit;
		}

		memcpy(params, ptr, len);

		if (params->ver != ADPS_VERSION) {
			printf("Wrong Version\n");
			ret = BCME_VERSION;
			goto exit;
		}

		if (params->num_step == 0) {
			printf("DISABLED\n");
		}
		else {
			printf("Mode: %d\n", params->mode);
			printf("PS Poll: %d\n", params->pspoll_prd);
			for (i = 0; i < params->num_step; i++) {
				printf("Step #%d:\n", i + 1);
				printf("\tpm2_sleep_ret - %d\n",
					params->step[i].pm2_sleep_ret_time);
				printf("\trx_pkts_threshold - %d\n",
					params->step[i].pkt_cnt_threshold[ADPS_RX]);
				printf("\ttx_pkts_threshold - %d\n",
					params->step[i].pkt_cnt_threshold[ADPS_TX]);
			}
		}
	}
	else {
		params->mode = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			ret = BCME_BADARG;
			goto exit;
		}
		param_cnt++;

		if (*argv == '\0') {
			ret = BCME_BADARG;
			goto exit;
		}

		params->pspoll_prd = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			ret = BCME_BADARG;
			goto exit;
		}
		param_cnt++;

		if (*argv == '\0') {
			ret = BCME_BADARG;
			goto exit;
		}

		params->num_step = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			ret = BCME_BADARG;
			goto exit;
		}
		if (params->num_step == 0 || params->num_step > ADPS_MAX_STEP_ENTRY) {
			ret = BCME_BADARG;
			goto exit;
		}
		param_cnt++;

		if ((uint32)argc != (uint32)(params->num_step *
			ADPS_STEP_ELEMENT_NUM + param_cnt)) {
			ret = BCME_BADARG;
			goto exit;
		}

		for (i = 0; i < params->num_step; i++) {
			params->step[i].pm2_sleep_ret_time =
				strtoul(argv[i * ADPS_STEP_ELEMENT_NUM], &endptr, 0);
			params->step[i].pkt_cnt_threshold[ADPS_RX] =
				strtoul(argv[i * ADPS_STEP_ELEMENT_NUM + 1], &endptr, 0);
			params->step[i].pkt_cnt_threshold[ADPS_TX] =
				strtoul(argv[i * ADPS_STEP_ELEMENT_NUM + 2], &endptr, 0);
		}

		len = ADPS_STEP_PARAMS_V1_FIXED_LEN + params->num_step * ADPS_STEP_LEN;
		params->ver = ADPS_VERSION;
		params->len = len;
		ret = wlu_var_setbuf(wl, cmd->name, params, len);
	}
exit:
	if (params) {
		free(params);
	}

	return ret;
}
#endif	/* BCMINTDBG */

static int
wl_adps_rssi(void *wl, cmd_t *cmd, char **argv)
{
	uint8 band;
	int argc;
	int ret = BCME_OK;
	void *ptr = NULL;
	char *endptr = NULL;

	wl_adps_rssi_params_v1_t *rssi = NULL;

	if ((rssi = malloc(sizeof(wl_adps_rssi_params_v1_t))) == NULL) {
		fprintf(stderr, "Error allocating %u bytes for adps rssi\n",
			(unsigned int)ADPS_RSSI_PARAMS_V1_LEN);
		return BCME_NOMEM;
	}
	memset(rssi, 0, ADPS_RSSI_PARAMS_V1_LEN);

	++argv;
	argc = ARGCNT(argv);

	if (*argv && (!stricmp(*argv, "b") || !stricmp(*argv, "2g"))) {
		band = WLC_BAND_2G;
	}
	else if (*argv && (!stricmp(*argv, "a") || !stricmp(*argv, "5g"))) {
		band = WLC_BAND_5G;
	}
	else {
		ret = BCME_BADARG;
		goto exit;
	}

	rssi->band = band;
	if (!*++argv) {
		if ((ret = wlu_var_getbuf(wl, cmd->name, rssi,
			ADPS_RSSI_PARAMS_V1_LEN, &ptr)) != BCME_OK) {
			printf("Fail to get adsp rssi\n");
			goto exit;
		}

		memcpy(rssi, ptr, ADPS_RSSI_PARAMS_V1_LEN);

		if (rssi->ver != ADPS_VERSION) {
			printf("Wrong Version\n");
			ret = BCME_VERSION;
			goto exit;
		}

		printf("high threshold: %d\n", rssi->rssi.high_thres);
		printf("low threshold: %d\n", rssi->rssi.low_thres);
	}
	else {
		if (argc != 3) {
			ret = BCME_USAGE_ERROR;
			goto exit;
		}

		rssi->rssi.high_thres = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			ret = BCME_BADARG;
			goto exit;
		}

		rssi->rssi.low_thres = strtoul(*argv++, &endptr, 0);
		if (*endptr != '\0') {
			ret = BCME_BADARG;
			goto exit;
		}

		rssi->ver = ADPS_VERSION;
		rssi->len = ADPS_RSSI_PARAMS_V1_LEN;
		ret = wlu_var_setbuf(wl, cmd->name, rssi, ADPS_RSSI_PARAMS_V1_LEN);
	}
exit:
	if (rssi) {
		free(rssi);
	}

	return ret;
}
