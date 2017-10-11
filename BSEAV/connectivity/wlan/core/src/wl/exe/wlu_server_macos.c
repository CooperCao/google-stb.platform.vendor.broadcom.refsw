/*
 * Macos port of Remote wl server
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
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <Kernel/IOKit/apple80211/apple80211_ioctl.h>
#include <Kernel/IOKit/apple80211/apple80211_var.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <bcmcdc.h>
#include "wlu_remote.h"


const char *rwl_server_name;

unsigned short defined_debug = DEBUG_ERR | DEBUG_INFO;
extern int remote_server_exec(int argc, char **argv, void *ifr);

/* Global to have the PID of the current sync command
 * This is required in case the sync command fails to respond,
 * the alarm handler shall kill the PID upon a timeout
 */
int g_shellsync_pid;
unsigned char g_return_stat = 0;
static bool g_swap = FALSE;
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))

static void
syserr(char *s)
{
	perror(s);
	exit(errno);
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct apple80211req *req = (struct apple80211req *) wl;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = cmd;
	req->req_len = len;
	req->req_data = CAST_USER_ADDR_T(buf);

	ret = ioctl(s, set ? SIOCSA80211 : SIOCGA80211, (char*)req);
	close(s);

#ifdef MACOS_DHD_DONGLE
	if (ret < 0)
		ret = dhdioctl_usr_dongle(wl, cmd, buf, len, set, TRUE);
#endif /* MACOS_DHD_DONGLE */

	if (ret < 0) {
		if (cmd != WLC_GET_MAGIC)
			ret = -45; /* BCME_IOCTL_ERROR */
	}

	return ret;
}

/* Functions copied from wlu.c to check for the driver adapter in the server machine */
int
wl_check(void *wl)
{
	int ret;
	int val;

	if ((ret = wl_ioctl(wl, WLC_GET_MAGIC, &val, sizeof(int), FALSE)) < 0)
		return ret;

	/* Detect if IOCTL swapping is necessary */
	if (val == (int)bcmswap32(WLC_IOCTL_MAGIC))
	{
		val = bcmswap32(val);
		g_swap = TRUE;
	}
	if (val != WLC_IOCTL_MAGIC)
		return -1;
	if ((ret = wl_ioctl(wl, WLC_GET_VERSION, &val, sizeof(int), FALSE)) < 0)
		return ret;
	val = dtoh32(val);
	if (val > WLC_IOCTL_VERSION) {
		fprintf(stderr, "Version mismatch, please upgrade\n");
		return -1;
	}
	return 0;
}

void
wl_find(struct apple80211req *req)
{
	struct ifaddrs *ifastart;
	struct ifaddrs *ifalist;
	int err;

	err = getifaddrs(&ifastart);
	if (err)
		syserr("getifaddrs");

	for (ifalist = ifastart; ifalist; ifalist = ifalist->ifa_next) {
		int name_len = strlen(ifalist->ifa_name);

		/* Make sure the ifaddr.ifa_name fits in the apple80211req.req_if_name field.
		 * If not, skip this interface.
		 */
		if (name_len + 1 > IFNAMSIZ) {
			continue;
		}

		memset(req, 0, sizeof(*req));

		/* copy the interface name and NUL termination */
		memcpy(req->req_if_name, ifalist->ifa_name, name_len + 1);

		if (!wl_check(req))
			break;
	}
	if (!ifalist)
		memset(req, 0, sizeof(*req));

	freeifaddrs(ifastart);
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

extern int set_ctrlc;
void handle_ctrlc(int unused)
{
	UNUSED_PARAMETER(unused);
	set_ctrlc = 1;
	return;
}

volatile sig_atomic_t g_sig_chld = 1;
void rwl_chld_handler(int num)
{
	int child_status;

	UNUSED_PARAMETER(num);
	/* g_return_stat is being set with the return status of sh commands */
	waitpid(g_shellsync_pid, &child_status, WNOHANG);
	if (WIFEXITED(child_status))
		g_return_stat = WEXITSTATUS(child_status);
	else if (g_rem_ptr->msg.flags == (unsigned)CTRLC_FLAG)
		g_return_stat = 0;
	else
		g_return_stat = 1;
	g_sig_chld = 0;
}

/* Alarm handler called after SHELL_TIMEOUT value
 * This handler kills the non-responsive shell process
 * with the PID value g_shellsync_pid
 */
static void
sigalrm_handler(int s)
{
	UNUSED_PARAMETER(s);

	if (g_shellsync_pid) {
		kill(g_shellsync_pid, SIGINT);
	}
	g_sig_chld = 0;
}

static void
def_handler(int s)
{
	UNUSED_PARAMETER(s);
	kill(g_shellsync_pid, SIGKILL);
	exit(0);
}

static void
pipe_handler(int s)
{
	UNUSED_PARAMETER(s);
	kill(g_shellsync_pid, SIGKILL);
}

int
main(int argc, char **argv)
{
	int err = 0;
	int name_len;
	struct apple80211req req = {{0}};

	rwl_server_name = argv[1];

	if (rwl_server_name) {
		name_len = strlen(rwl_server_name);

		if (name_len + 1 > IFNAMSIZ) {
			fprintf(stderr, "interface name \"%s\" is too long\n", rwl_server_name);
			exit(1);
		}

		/* copy the interface name and NUL termination */
		memcpy(req.req_if_name, rwl_server_name, name_len + 1);
	}

	/* validate provided interface name */
	if (*req.req_if_name && wl_check((void *)&req)) {
		fprintf(stderr, "interface \"%s\" is not a valid wl driver\n", req.req_if_name);
		exit(1);
	}
	/* find a default interface */
	if (!*req.req_if_name) {
		wl_find(&req);
		if (!*req.req_if_name) {
			fprintf(stderr, "wl driver adapter not found\n");
			exit(1);
		}
	}

	/* Register signal handlers */
	signal(SIGCHLD, rwl_chld_handler);
	signal(SIGALRM, sigalrm_handler);
	signal(SIGTERM, def_handler);
	signal(SIGPIPE, pipe_handler);
	signal(SIGABRT, def_handler);
#ifdef RWL_DONGLE
	signal(SIGINT, handle_ctrlc);
#endif

	/* Main server process for all transport types */
	err = remote_server_exec(argc, argv, &req);

	return err;
}
