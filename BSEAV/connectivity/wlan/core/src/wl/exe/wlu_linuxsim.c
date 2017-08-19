/*
 * Linux port of wl command line utility for simulation environment
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

#error "wlu_linuxsim.c requires linuxsim environment"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu.h"

#define DEV_TYPE_LEN 3 /* length for devtype 'wl'/'et' */

static cmd_t *wl_find_cmd(char* name);

static void
syserr(char *s)
{
	fprintf(stderr, "%s: ", wlu_av0);
	perror(s);
#if !defined(BCMSIM_STDALONE)
	abort();
#else
	exit(errno);
#endif /* BCMSIM_STDALONE */
}

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *) wl;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		if (cmd != WLC_GET_MAGIC) {
			ret = BCME_IOCTL_ERROR;
		}
	}

	/* cleanup */
	close(s);
	return ret;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	}
	else
		strncpy(buf, info.driver, len);

	close(s);
	return ret;
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
 * For now search for eth<n> for n from 0 to 9; this may change to
 * some mechanism for sharing the interface configuration in the
 * future.
 */

static void
wl_find(struct ifreq *ifr)
{
	int i;
	const int index_offset = 3; /* Offset of 'n' in eth<n> string */
	char name[5];
	char dev_type[DEV_TYPE_LEN];

	ifr->ifr_name[0] = '\0';

	bzero(name, sizeof(name));
	strcpy(name, "eth");

	for (i = '0'; i < '9'; i++) {
		name[index_offset] = i;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "wl", 2)) {
			if (wl_check((void *) ifr) == 0) {
				return;
			}
		}
	}
	ifr->ifr_name[0] = '\0';  /* Mark as not-found */
}

int
main(int argc, char **argv)
{
	struct ifreq ifr;
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status = CMD_WL;

	wlu_av0 = argv[0];

	wlu_init();
	memset(&ifr, 0, sizeof(ifr));

	for (++argv; *argv;) {

		/* command option */
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (help) {
				break;
			}
			if (ifname) {
				strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
			}
			continue;
		}
		/* parse error */
		else if (status == CMD_ERR) {
			break;
		}
		/* wl command */
		/*
		 * else if (status == CMD_WL)
		 *	;
		 */

		/* use default interface */
		if (!*ifr.ifr_name) {
			wl_find(&ifr);
		}
		/* validate the interface */
		if (!*ifr.ifr_name || wl_check((void *)&ifr)) {
			fprintf(stderr, "%s: wl driver adapter not found\n", wlu_av0);
			return USAGE_ERROR;
		}

		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd) {
			cmd = &wl_varcmd;
		}

		/* do command */
		err = (*cmd->func)((void *) &ifr, cmd, argv);

		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd) {
			wl_cmd_usage(stdout, cmd);
		} else {
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       wlu_av0, *argv);
		}
	} else if (!cmd)
		wl_usage(stdout, NULL);
	else if (err == BCME_USAGE_ERROR)
		wl_cmd_usage(stderr, cmd);
	else if (err == BCME_IOCTL_ERROR)
		wl_printlasterror((void *) &ifr);

	return err;
}

/* Search the wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t *
wl_find_cmd(char* name)
{
	return wlu_find_cmd(name);
}
