/*
 * wl tpc command module
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
 * $Id: wluc_tpc.c 458728 2014-02-27 18:15:25Z $
 */

#ifdef WIN32
#include <windows.h>
#endif

#include <wlioctl.h>


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

static cmd_func_t wl_tpc_lm;
#ifdef BCMINTDBG
static cmd_func_t wl_tpc_rpt_override;
#endif /* BCMINTDBG */

static cmd_t wl_tpc_cmds[] = {
	{ "tpc_mode", wl_varint, WLC_GET_VAR, WLC_SET_VAR,
	"Enable/disable AP TPC.\n"
	"Usage: wl tpc_mode <mode> \n"
	"\t0 - disable, 1 - BSS power control, 2 - AP power control, 3 - Both (1) and (2)"},
	{ "tpc_period", wl_varint, WLC_GET_VAR, WLC_SET_VAR,
	"Set AP TPC periodicity in secs.\n"
	"Usage: wl tpc_period <secs> "},
	{ "tpc_lm", wl_tpc_lm, WLC_GET_VAR, -1,
	"Get current link margins."},
#if defined(BCMINTDBG)
	{ "tpc_rpt_override", wl_tpc_rpt_override, WLC_GET_VAR, WLC_SET_VAR,
	"set/get tpc report txpwr and linkmargin values\n"
	"\tUsage: wl tpc_rpt_override [<txpwr> <link_margin>]"},
#endif /* BCMINTDBG */
	{ NULL, NULL, 0, 0, NULL }
};

static char *buf;

/* module initialization */
void
wluc_tpc_module_init(void)
{
	(void)g_swap;

	/* get the global buf */
	buf = wl_get_buf();

	/* register tpc commands */
	wl_module_cmds_register(wl_tpc_cmds);
}

static int
wl_tpc_lm(void *wl, cmd_t *cmd, char **argv)
{
	int ret;
	uint16 val;
	int8 aplm, stalm;

	UNUSED_PARAMETER(argv);

	if ((ret = wlu_iovar_getint(wl, cmd->name, (int *)(uintptr)&val)) < 0)
		return ret;

	stalm = val & 0xff;
	aplm = (val >> 8) & 0xff;

	printf("TPC: APs link margin:%d\t STAs link margin:%d\n", aplm, stalm);

	return 0;
}

#if defined(BCMINTDBG)
static int
wl_tpc_rpt_override(void *wl, cmd_t *cmd, char **argv)
{
	char *endptr;
	int err = 0;
	uint32 txpwr_linkm = 0;

	/* toss the command name */
	argv++;

	if (*argv == NULL) {
		/* get */
		err = wlu_iovar_getint(wl, cmd->name, (int*)&txpwr_linkm);
		if (err)
			return err;
		printf("TxPwr:%d Link Margin:%d\n", ((txpwr_linkm>> 8) & 0xff),
			(txpwr_linkm & 0xff));
	} else {
		int8 txpwr, linkm;
		/* set */
		if (argv[1] == NULL)
			return BCME_USAGE_ERROR;

		txpwr = (uint8)strtoul(*argv, &endptr, 0);
		if (*endptr != '\0') {
			fprintf(stderr, "%s: %s: error parsing \"%s\" as an integer\n",
			        wlu_av0, cmd->name, *argv);
			return BCME_USAGE_ERROR;
		}

		argv++;
		linkm = (uint8)strtoul(*argv, &endptr, 0);
		if (*endptr != '\0') {
			fprintf(stderr, "%s: %s: error parsing \"%s\" as an integer\n",
			        wlu_av0, cmd->name, *argv);
			return BCME_USAGE_ERROR;
		}

		txpwr_linkm = (txpwr << 8) | linkm;

		err = wlu_iovar_setint(wl, cmd->name, (int)txpwr_linkm);
		if (err)
			return err;
	}

	return err;
}
#endif /* BCMINTDBG */
