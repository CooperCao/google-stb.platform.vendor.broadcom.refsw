/*
 * wl command-line swiss-army-knife utility. Generic (not OS-specific)
 * implementation that interfaces to WLAN driver API (wl_drv.h).
 *
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */

/* ---- Include Files ---------------------------------------------------- */

#include "bcmutils.h"
#include "wlioctl.h"
#include <stdio.h>
#include "wlu.h"
#include "wl_drv.h"


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define NUM_ARGS	64


/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */

static cmd_t* wl_find_cmd(char* name);
static int wl_ioctl(uint cmd, char *buf, uint buflen, bool set);


/* ---- Functions -------------------------------------------------------- */

/****************************************************************************
* Function:   wl_find_cmd
*
* Purpose:    Search the wl_cmds table for a matching command name.
*             Return the matching command or NULL if no match found.
*
* Parameters: name (in) Name of command to find.
*
* Returns:    Command structure. NULL if not found.
*****************************************************************************
*/
static cmd_t *
wl_find_cmd(char* name)
{
	return wlu_find_cmd(name);
}

/****************************************************************************
* Function:   wl_ioctl
*
* Purpose:    Issue an IOCTL to the DHD driver.
*
* Parameters: cmd	(in) IOCTL command id.
*             buf	(in) Optional IOCTL command buffer.
*             buflen	(in) IOCTL command buffer length in bytes.
*             set	(in) TRUE for set IOCTL, FALSE for get IOCTL.
*
* Returns:    0 for success, else error code.
*****************************************************************************
*/
static int
wl_ioctl(uint cmd, char *buf, uint buflen, bool set)
{
	int ret = 0;
	wl_drv_ioctl_t ioc;

	memset(&ioc, 0, sizeof(ioc));
	ioc.w.cmd = cmd;
	ioc.w.buf = buf;
	ioc.w.len = buflen;
	ioc.w.set = set;

	if (wl_drv_ioctl(NULL, &ioc) < 0) {
		ret = BCME_IOCTL_ERROR;
	}

	return (ret);
}

/* ----------------------------------------------------------------------- */
int
wl_get(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(cmd, buf, len, FALSE);
}

/* ----------------------------------------------------------------------- */
int
wl_set(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(cmd, buf, len, TRUE);
}


/* ----------------------------------------------------------------------- */
int
wlu_main_args(int argc, char **argv)
{
	cmd_t *cmd = NULL;
	int err = 0;
	int help = 0;
	int status = CMD_WL;
	char *notused;

	wlu_av0 = argv[0];

	/* Skip first arg. */
	argc--;
	argv++;

	wlu_init();

	while (*argv != NULL) {

		/* command option */
		if ((status = wl_option(&argv, &notused, &help)) == CMD_OPT) {
			if (help)
				break;

			continue;
		}
		/* parse error */
		else if (status == CMD_ERR)
			break;
		/* wl command */
		/*
		 * else if (status == CMD_WL)
		 *	;
		 */


		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &wl_varcmd;

		/* do command */
		err = (*cmd->func)(NULL, cmd, argv);

		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd)
			wl_cmd_usage(stdout, cmd);
		else {
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       wlu_av0, *argv);
		}
	} else if (!cmd)
		wl_usage(stdout, NULL);
	else if (err == BCME_USAGE_ERROR)
		wl_cmd_usage(stderr, cmd);
	else if (err == BCME_IOCTL_ERROR)
		wl_printlasterror(NULL);

	return err;
}

/* ----------------------------------------------------------------------- */
int
wlu_main_str(char *str)
{
	char *argv[NUM_ARGS];
	int argc;
	char *token;

	memset(argv, 0, sizeof(argv));

	argc = 0;
	while ((argc < (NUM_ARGS - 1)) &&
	       ((token = bcmstrtok(&str, " \t\n", NULL)) != NULL)) {
		argv[argc++] = token;
	}
	argv[argc] = NULL;

	return (wlu_main_args(argc, argv));
}
