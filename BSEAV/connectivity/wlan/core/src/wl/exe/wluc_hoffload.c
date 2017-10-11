/*
 * wl hoffload command module
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wlioctl.h>
#include <bcm_hoffload_meta.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include "wlu_common.h"
#include "wlu.h"

#define HOFFLOAD_DHD_FILE "/sys/module/dhd/parameters/dhd_hmem_module_string"
#define MODULE_DIR "/lib/firmware/"
#define PROCESSOR_RATE 385.0 /* MHz */
#define DEFAULT_MODULES_FILE "modules.img"
#define MAXNAME 100

static cmd_func_t wl_hoffload_get_stats;
static cmd_func_t wl_hoffload_clr_stats;

static cmd_t wl_hoffload_cmds[] = {
	{ "hoffload_stats", wl_hoffload_get_stats, WLC_GET_VAR, -1,
	"dump host module offload statistics\n" },
	{ "hoffload_clear", wl_hoffload_clr_stats, WLC_GET_VAR, -1,
	"clear host module offload statistics\n" },
	{ NULL, NULL, 0, 0, NULL }
};

static uint8 mod_cnt;
static struct host_module_metadata *mod_meta = NULL;

/* module initialization */
void
wluc_hoffload_module_init(void)
{
	(void)g_swap;

	/* register hoffload commands */
	wl_module_cmds_register(wl_hoffload_cmds);
}

/* Get module metadata. */
static int
wluc_hoffload_read_meta(void)
{
	struct host_module_metadata mod_meta_buf[sizeof(*mod_meta)];
	char name[MAXNAME];
	FILE *fp = fopen(HOFFLOAD_DHD_FILE, "r");
	int rc = BCME_OK;

	/* Prepend module file directory, then read module file name. */
	(void)strncpy(name, MODULE_DIR, MAXNAME-1);
	name[MAXNAME-1] = '\0';
	if (fp == NULL) {
		fprintf(stderr,
			"hoffload: cannot read module file name from \"%s\", %s. ",
			HOFFLOAD_DHD_FILE, strerror(errno));
		fprintf(stderr, "Using default \"%s\".\n", DEFAULT_MODULES_FILE);
		(void)strncat(name, DEFAULT_MODULES_FILE, MAXNAME-strlen(name)-1);
		rc = BCME_NOTFOUND;
	}
	if (rc == BCME_OK) {
		fgets(name + strlen(name), sizeof(name) - strlen(name), fp);
		fclose(fp);
	}
	/* Suppress nl, cr. */
	name[strcspn(name, "\r\n")] = '\0';
	/* Open module file given its full name */
	fp = fopen(name, "r");
	if (fp == NULL) {
		fprintf(stderr,
			"hoffload: cannot read modules from \"%s\", %s.\n",
			name, strerror(errno));
		return BCME_NOTFOUND;
	}
	/* Read each module metadata, count modules. */
	mod_cnt = 0;
	while (fread(mod_meta_buf, sizeof(mod_meta_buf), 1, fp)) {
		mod_meta = (mod_cnt == 0) ? malloc(sizeof(*mod_meta)) :
			realloc(mod_meta, sizeof(*mod_meta) * (mod_cnt + 1));
		if (mod_meta == NULL) {
			fprintf(stderr,
				"hoffload: no memory, read metadata of %u modules.\n",
				mod_cnt);
			fclose(fp);
			return BCME_NOMEM;
		}
		(void)memcpy(mod_meta + mod_cnt, mod_meta_buf, sizeof(*mod_meta));
		mod_cnt++;
		if (!(mod_meta->flag & BCM_MODULE_HDR_METADATA_NEXT)) {
			break;
		}
		/* Skip to next module's metadata. */
		fseek(fp, mod_meta->size, SEEK_CUR);
	}
	if (ferror(fp)) {
		fprintf(stderr,
			"hoffload: error reading modules from \"%s\", %s.\n",
			name, strerror(errno));
		fclose(fp);
		return BCME_ERROR;
	}
	fclose(fp);
	return BCME_OK;
}

static void
wl_hoffload_free_meta(void)
{
	if (mod_meta != NULL) {
		free(mod_meta);
		mod_meta = NULL;
	}
}

static int
wl_hoffload_get_stats(void *wl, cmd_t *cmd, char **argv)
{
	const char *cmdname = "bus:hoffload_get_stats";
	uint8 mod_id;
	int err;
	hoffload_stats_t *st;

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	if ((err = wluc_hoffload_read_meta())) {
		wl_hoffload_free_meta();
		return err;
	}
	if ((err = wlu_var_getbuf(wl, cmdname, &mod_cnt, sizeof(mod_cnt),
			(void **)&st) < 0)) {
		fprintf(stderr, "hoffload: Cannot read stats.\n");
		wl_hoffload_free_meta();
		return err;
	}
	for (mod_id = 0; mod_id < mod_cnt; mod_id++, st++) {
		uint8 *mod_name = mod_meta[mod_id].name;

		printf("%s: loads %u faults %u load %.2fus ",
			mod_name, st->loads, st->faults,
			st->ld_cycles / PROCESSOR_RATE);
		printf("run %.2fus free %.2fK\n",
			st->cycles / PROCESSOR_RATE, st->free / 1024.0);
	}
	wl_hoffload_free_meta();
	return BCME_OK;
}

static int
wl_hoffload_clr_stats(void *wl, cmd_t *cmd, char **argv)
{
	const char *cmdname = "bus:hoffload_clr_stats";

	UNUSED_PARAMETER(cmd);
	UNUSED_PARAMETER(argv);

	return wlu_var_setbuf(wl, cmdname, NULL, 0);
}
