/*
 * dhd command-line swiss-army-knife utility. Generic (not OS-specific)
 * implementation that interfaces to WLAN driver API (wl_drv.h).
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */


/* ---- Include Files ---------------------------------------------------- */

#include "dhdu_cmd.h"
#include "wl_drv.h"
#include "dhdu.h"
#include "bcmutils.h"


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define NUM_ARGS	64


/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */

static int dhdu_ioctl(void *dhd, uint cmd, char *buf, uint buflen, bool set);


/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
* Function:   dhdu_ioctl
*
* Purpose:    Issue an IOCTL to the DHD driver.
*
* Parameters: dhd	(in) No used.
*             cmd	(in) IOCTL command id.
*             buf	(in) Optional IOCTL command buffer.
*             buflen	(in) IOCTL command buffer length in bytes.
*             set	(in) TRUE for set IOCTL, FALSE for get IOCTL.
*
* Returns:    0 for success, else error code.
*****************************************************************************
*/
static int
dhdu_ioctl(void *dhd, uint cmd, char *buf, uint buflen, bool set)
{
	wl_drv_ioctl_t ioc;
	int ret = 0;

	memset(&ioc, 0, sizeof(ioc));
	ioc.d.cmd = cmd;
	ioc.d.buf = buf;
	ioc.d.len = buflen;
	ioc.d.set = set;
	ioc.d.driver = DHD_IOCTL_MAGIC;

	if (wl_drv_ioctl(NULL, &ioc) < 0) {
		ret = BCME_IOCTL_ERROR;
	}

	return (ret);
}


/* ----------------------------------------------------------------------- */
int
dhd_get(void *dhd, int cmd, void *buf, int len)
{
	return (dhdu_ioctl(dhd, cmd, buf, len, FALSE));
}


/* ----------------------------------------------------------------------- */
int
dhd_set(void *dhd, int cmd, void *buf, int len)
{
	return (dhdu_ioctl(dhd, cmd, buf, len, TRUE));
}


/* ----------------------------------------------------------------------- */
int
dhdu_main_args(int argc, char **argv)
{
	cmd_t *cmd = NULL;
	int err = 0;
	int help = 0;
	int status = CMD_DHD;
	char *notused;

	dhdu_av0 = argv[0];

	/* Skip first arg. */
	argc--;
	argv++;

	while (*argv != NULL) {

		/* command option */
		if ((status = dhd_option(&argv, &notused, &help)) == CMD_OPT) {
			if (help)
				break;

			continue;
		}

		/* parse error */
		else if (status == CMD_ERR)
			break;

		/* search for command */
		for (cmd = dhd_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);

		/* defaults to using the set_var and get_var commands */
		if (cmd->name == NULL)
			cmd = &dhd_varcmd;

		/* do command */
		if (cmd->name)
			err = (*cmd->func)(NULL, cmd, argv);
		break;
	}


	if (help && *argv) {
		/* search for command */
		for (cmd = dhd_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);
		if (cmd->name != NULL)
			dhd_cmd_usage(cmd);
		else {
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       dhdu_av0, *argv);
		}
	} else if (!cmd)
		dhd_usage(NULL);
	else if (err == BCME_USAGE_ERROR)
		dhd_cmd_usage(cmd);
	else if (err == BCME_IOCTL_ERROR)
		dhd_printlasterror(NULL);

	return err;
}


/* ----------------------------------------------------------------------- */
int
dhdu_main_str(char *str)
{
	char *argv[NUM_ARGS];
	int argc;
	char *token;

	memset(argv, 0, sizeof(argv));

	/* Parse args string into white-space separated tokens. */
	argc = 0;
	while ((argc < (NUM_ARGS - 1)) &&
	       ((token = bcmstrtok(&str, " \t\n", NULL)) != NULL)) {
		argv[argc++] = token;
	}
	argv[argc] = NULL;

	return (dhdu_main_args(argc, argv));
}
