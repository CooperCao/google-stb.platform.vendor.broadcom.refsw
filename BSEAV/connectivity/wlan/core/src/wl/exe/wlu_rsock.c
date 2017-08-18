/*
 * Remote Sockets port of wl command line utility for Linux User Mode.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wlioctl.h>
#include <bcmutils.h>

#include <rsock/types.h>
#include <rsock/socket.h>

#include "wlu.h"

static cmd_t *wl_find_cmd(char* name);

#define MAX_IOCTL_SIZE		448	/* Can't exceed rsock datagram MTU */

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct sockaddr_if ifaddr;
	struct rsock_ifcontrol *ifctl;
	char *ifname = wl;
	char msg[sizeof(struct rsock_ifcontrol) + MAX_IOCTL_SIZE];
	int msglen, s;

	/*
	 * Currently, ioctl transfers are truncated similar to the way CDC does it.
	 * For some reason RSock USB MTU is limited to 500 bytes -- problem not yet solved.
	 */
	if (len > MAX_IOCTL_SIZE)
		len = MAX_IOCTL_SIZE;

	/* open rsock control socket */
	if ((s = rsock_socket(PF_RSOCK, SOCK_DGRAM, set)) < 0) {
		fprintf(stderr, "%s: rsock_socket(): %s\n", wlu_av0, strerror(errno));
		return IOCTL_ERROR;
	}

	memset(&ifaddr, 0, sizeof(ifaddr));
	ifaddr.sif_family = AF_RSOCK;
	strncpy(ifaddr.sif_name, ifname, sizeof(ifaddr.sif_name));
	ifaddr.sif_name[sizeof(ifaddr.sif_name) - 1] = 0;

	ifctl = (struct rsock_ifcontrol *)msg;
	ifctl->cmd = cmd;
	ifctl->set = set;
	ifctl->status = 0;
	ifctl->datalen = len;
	memcpy(&ifctl[1], buf, len);

	msglen = (int)(sizeof(struct rsock_ifcontrol) + len);

	if (rsock_sendto(s, ifctl, msglen, 0,
	                 (struct sockaddr *)&ifaddr, sizeof(ifaddr)) < 0) {
		fprintf(stderr, "%s: rsock_sendto(): %s\n", wlu_av0, strerror(errno));
		rsock_close(s);
		return IOCTL_ERROR;
	}

	if (rsock_recv(s, msg, sizeof(msg), 0) < 0) {
		fprintf(stderr, "%s: rsock_recv(): %s\n", wlu_av0, strerror(errno));
		rsock_close(s);
		return IOCTL_ERROR;
	}

	if (ifctl->status != 0)
		fprintf(stderr, "%s: wl_ioctl non-zero status %d\n", wlu_av0, ifctl->status);
	else
		memcpy(buf, &ifctl[1], len);

	rsock_close(s);
	return 0;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(wl, cmd, buf, len, FALSE);
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	return wl_ioctl(wl, cmd, buf, len, TRUE);
}

/*
 * If ifname_in is NULL or empty, finds the first available interface and copies its
 * name into ifname_out.  Returns 0 on success, -1 on error.
 *
 * If ifname is non-NULL, checks if it's an existing interface, and if so, copies its
 * name into ifname_out.  Returns 0 on success, -1 on error.
 */

static int
wl_find(char *ifname_in, char ifname_out[IFNAMSIZ])
{
	struct rsock_ifconfig ifc;
	int s;

	if ((s = rsock_socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "rsock_socket(): %s\n", strerror(errno));
		return -1;
	}

	memset(&ifc, 0, sizeof(ifc));

	if (ifname_in == NULL || ifname_in[0] == 0)
		ifc.ifc_index = 0;
	else {
		strncpy(ifc.ifc_name, ifname_in, sizeof(ifc.ifc_name));
		ifc.ifc_name[sizeof(ifc.ifc_name) - 1] = 0;
	}

	if (rsock_ioctl(s, SIOCGIFCONFIG, &ifc) < 0) {
		fprintf(stderr, "rsock_ioctl(SIOCGIFCONFIG): %s\n", strerror(errno));
		rsock_close(s);
		return -1;
	}

	strcpy(ifname_out, ifc.ifc_name);

	rsock_close(s);

	return 0;
}

int
wlu_main(int argc, char **argv)
{
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname_ptr = NULL;
	char ifname_arg[IFNAMSIZ];
	char ifname[IFNAMSIZ];
	int help = 0;
	int status = CMD_WL;

	wlu_av0 = argv[0];

	wlu_init();
	memset(ifname_arg, 0, sizeof(ifname_arg));

	for (++argv; *argv;) {
		/* command option */
		if ((status = wl_option(&argv, &ifname_ptr, &help)) == CMD_OPT) {
			if (help)
				break;
			if (ifname_ptr) {
				strncpy(ifname_arg, ifname_ptr, sizeof(ifname_arg));
				ifname_arg[sizeof(ifname_arg) - 1] = 0;
			}
			continue;
		}

		if (status == CMD_ERR)
			break;		/* parse error */

		/* find/validate the interface */
		if (wl_find(ifname_arg, ifname) < 0) {
			fprintf(stderr, "%s: wl driver adapter not found\n", wlu_av0);
			return -1;
		}

		if (wl_check((void *)ifname)) {
			fprintf(stderr, "%s: wl driver adapter check failed\n", wlu_av0);
			return -1;
		}

		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &wl_varcmd;

		/* do command */
		err = (*cmd->func)((void *)ifname, cmd, argv);

		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd)
			wl_cmd_usage(stdout, cmd);
		else
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       wlu_av0, *argv);
	} else if (!cmd)
		wl_usage(stdout, NULL);
	else if (err == BCME_USAGE_ERROR)
		wl_cmd_usage(stderr, cmd);
	else if (err == BCME_IOCTL_ERROR)
		wl_printlasterror((void *)ifname);

	return err;
}

/* Search the wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t*
wl_find_cmd(char* name)
{
	return wlu_find_cmd(name);
}
