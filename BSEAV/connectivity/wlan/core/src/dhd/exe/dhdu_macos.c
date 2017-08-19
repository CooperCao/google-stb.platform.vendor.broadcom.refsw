/*
 * MacOS dhd app
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
#include <ctype.h>
#include <unistd.h>
#include <net/if.h>
#include <typedefs.h>
#include <signal.h>
#include <errno.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <apple80211_ioctl.h>
#include <apple80211_var.h>

#include <dhdioctl.h>
#include <bcmutils.h>
#include <wlioctl.h>

#include "dhdu.h"
#include "dhdioctl_usr.h"
#include "dhdu_common.h"

static int rwl_os_type = MAC_OSX;
unsigned short defined_debug = DEBUG_ERR;
int remote_type = NO_REMOTE;
char interface_name[IFNAMSIZ];

static void
syserr(const char *s)
{
	fprintf(stderr, "%s: ", dhdu_av0);
	perror(s);
	exit(errno);
}


static int
dhd_ioctl(void *dhd, int cmd, void *buf, int len, bool set)
{


	struct apple80211req req;
	dhd_ioctl_t ioc;
	int ret = 0;
	int s;

	/* By default try to execute wl commands */
	int driver_magic = WLC_IOCTL_MAGIC;
	int get_magic = WLC_GET_MAGIC;

	/* For local dhd commands execute dhd. For wifi transport we still
	 * execute wl commands.
	 */
	if (remote_type == NO_REMOTE && strncmp (buf, RWL_WIFI_ACTION_CMD,
		strlen(RWL_WIFI_ACTION_CMD)) && strncmp(buf, RWL_WIFI_GET_ACTION_CMD,
		strlen(RWL_WIFI_GET_ACTION_CMD))) {
		driver_magic = DHD_IOCTL_MAGIC;
		get_magic = DHD_GET_MAGIC;
	}

	/* fill in the dhd ioctl structure */
	memset(&ioc, 0, sizeof(ioc));
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = set;
	ioc.driver = driver_magic;

	/* fill in the outer Apple ioctl request structure	*/
	memset(&req, 0, sizeof(req));
	strncpy(req.req_if_name, interface_name, sizeof(req.req_if_name)-1);
	req.req_type = APPLE80211_IOC_CARD_SPECIFIC;
	/* this will help the dhd mac os driver differentiate between wl and dhd ioctls */
	req.req_val = driver_magic;
	req.req_len = sizeof(ioc);
#ifdef RDR_5905993
	/* Apple requests this for 64bit support but it causes warnings.
	 * I suspect that they have headers that change the type of req_data
	 * from void* to user_addr_t which would work with this code.
	 */
	req.req_data = CAST_USER_ADDR_T(&ioc);
#else
	req.req_data = &ioc;
#endif

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	ret = ioctl(s, set ? SIOCSA80211 : SIOCGA80211, (char*)&req);
	close(s);

#ifdef MACOS_DHD_DONGLE
	if (ret < 0)
		ret = dhdioctl_usr_dongle(dhd, cmd, buf, len, set, TRUE);
#endif /* MACOS_DHD_DONGLE */

	if (ret < 0) {
		if (cmd != get_magic)
			ret = BCME_IOCTL_ERROR;
	}

	return ret;
}


/* This function is called by wl_get to execute either local dhd command
 * or send a dhd command over wl transport
 */
static int
ioctl_queryinformation_fe(void *wl, int cmd, void* input_buf, int *input_len)
{
	if (remote_type == NO_REMOTE) {
		return dhd_ioctl(wl, cmd, input_buf, *input_len, FALSE);
	}
#ifdef RWL_ENABLE
	else {
		return rwl_queryinformation_fe(wl, cmd, input_buf,
			(unsigned long*)input_len, 0, RDHD_GET_IOCTL);
	}
#else /* RWL_ENABLE */
	return BCME_IOCTL_ERROR;
#endif /* RWL_ENABLE */
}

/* This function is called by wl_set to execute either local dhd command
 * or send a dhd command over wl transport
 */
static int
ioctl_setinformation_fe(void *wl, int cmd, void* buf, int *len)
{
	if (remote_type == NO_REMOTE) {
		return dhd_ioctl(wl,  cmd, buf, *len, TRUE);
	}
#ifdef RWL_ENABLE
	else {
		return rwl_setinformation_fe(wl, cmd, buf, (unsigned long*)len, 0, RDHD_SET_IOCTL);

	}
#else /* RWL_ENABLE */
	return BCME_IOCTL_ERROR;
#endif /* RWL_ENABLE */
}


/* The function is replica of wl_get in wlu_linux.c. Optimize when we have some
 * common code between wlu_linux.c and dhdu_linux.c
 */
int
wl_get(void *wl, int cmd, void *buf, int len)
{
	int error = BCME_OK;
	/* For RWL: When interfacing to a Windows client, need t add in OID_BASE */
	if ((rwl_os_type == WIN32_OS) && (remote_type != NO_REMOTE)) {
		error = (int)ioctl_queryinformation_fe(wl, WL_OID_BASE + cmd, buf, &len);
	} else {
		error = (int)ioctl_queryinformation_fe(wl, cmd, buf, &len);
	}
	if (error == BCME_SERIAL_PORT_ERR)
		return BCME_SERIAL_PORT_ERR;

	if (error != 0)
		return BCME_IOCTL_ERROR;

	return error;
}

/* The function is replica of wl_set in wlu_linux.c. Optimize when we have some
 * common code between wlu_linux.c and dhdu_linux.c
 */
int
wl_set(void *wl, int cmd, void *buf, int len)
{
	int error = BCME_OK;

	error = (int)ioctl_setinformation_fe(wl, cmd, buf, &len);

	if (error == BCME_SERIAL_PORT_ERR)
		return BCME_SERIAL_PORT_ERR;

	if (error != 0) {
		return BCME_IOCTL_ERROR;
	}
	return error;
}

/* Verify the wl adapter found.
 * This is called by dhd_find to check if it supports wl
 * The reason for checking wl adapter is that we can still send remote dhd commands over
 * wifi transport. The function is copied from wlu.c.
 */
int
wl_check(void *wl)
{
	int ret;
	int val = 0;

	if (!dhd_check (wl))
		return 0;

	/*
	 *  If dhd_check() fails then go for a regular wl driver verification
	 */
	if ((ret = wl_get(wl, WLC_GET_MAGIC, &val, sizeof(int))) < 0)
		return ret;
	if (val != WLC_IOCTL_MAGIC)
		return BCME_ERROR;
	if ((ret = wl_get(wl, WLC_GET_VERSION, &val, sizeof(int))) < 0)
		return ret;
	if (val > WLC_IOCTL_VERSION) {
		fprintf(stderr, "Version mismatch, please upgrade\n");
		return BCME_ERROR;
	}
	return 0;
}


int
wl_validatedev(void *dev_handle)
{
	int retval = 1;
	struct ifreq *ifr = (struct ifreq *)dev_handle;
	/* validate the interface */
	if (!ifr->ifr_name || wl_check((void *)ifr)) {
		retval = 0;
	}
	return retval;
}

int
dhd_get(void *dhd, int cmd, void *buf, int len)
{
	return wl_get(dhd, cmd, buf, len);
}

int
dhd_set(void *dhd, int cmd, void *buf, int len)
{
	return wl_set(dhd, cmd, buf, len);
}


int
rwl_shell_createproc(void *wl)
{
	UNUSED_PARAMETER(wl);
	return fork();
}

void
rwl_shell_killproc(int pid)
{
	kill(pid, SIGKILL);
}


/*
* Search a string backwards for a set of characters
* This is the reverse version of strspn()
*
* @param	s	string to search backwards
* @param	accept	set of chars for which to search
* @return	number of characters in the trailing segment of s
*		which consist only of characters from accept.
*/
static size_t
sh_strrspn(const char *s, const char *accept)
{
	const char *p;
	size_t accept_len = strlen(accept);
	int i;

	if (s[0] == '\0') {
		return 0;
	}

	p = s + strlen(s);
	i = 0;
	do {
		p--;
		if (memchr(accept, *p, accept_len) == NULL) {
			break;
		}
		i++;
	} while (p != s);

	return i;
}

/*
* Parse the unit and subunit from an interface string such as wlXX or wlXX.YY
*
* @param	ifname	interface string to parse
* @param	unit	pointer to return the unit number, may pass NULL
* @param	subunit	pointer to return the subunit number, may pass NULL
* @return	Returns 0 if the string ends with digits or digits.digits, -1 otherwise.
*		If ifname ends in digits.digits, then unit and subuint are set
*		to the first and second values respectively. If ifname ends
*		in just digits, unit is set to the value, and subunit is set
*		to -1. On error both unit and subunit are -1. NULL may be passed
*		for unit and/or subuint to ignore the value.
*/
static int
get_ifname_unit(const char* ifname, int *unit, int *subunit)
{
	const char digits[] = "0123456789";
	char str[64];
	char *p;
	size_t ifname_len = strlen(ifname);
	size_t len;
	unsigned long val;

	if (unit) {
		*unit = -1;
	}
	if (subunit) {
		*subunit = -1;
	}

	if (ifname_len + 1 > sizeof(str)) {
		return -1;
	}

	strcpy(str, ifname);

	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);

	/* fail if there were no trailing digits */
	if (len == 0) {
		return -1;
	}

	/* point to the beginning of the last integer and convert */
	p = str + (ifname_len - len);
	val = strtoul(p, NULL, 10);

	/* if we are at the beginning of the string, or the previous
	* character is not a '.', then we have the unit number and
	* we are done parsing
	*/
	if (p == str || p[-1] != '.') {
		if (unit) {
			*unit = val;
		}
		return 0;
	} else {
		if (subunit) {
			*subunit = val;
		}
	}

	/* chop off the '.NNN' and get the unit number */
	p--;
	p[0] = '\0';

	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);

	/* fail if there were no trailing digits */
	if (len == 0) {
		return -1;
	}

	/* point to the beginning of the last integer and convert */
	p = p - len;
	val = strtoul(p, NULL, 10);

	/* save the unit number */
	if (unit) {
		*unit = val;
	}

	return 0;
}

int
get_bsscfg_idx(void *dhd, int *bsscfg_idx)
{
	struct ifreq *ifr = dhd;
	int vif;
	int ret = BCME_ERROR;
	ret = get_ifname_unit(ifr->ifr_name, NULL, &vif);
	/* only return values possible from get_ifname_unit are
	 * -1 or the positive subunit number for vif
	 *  representation will be in this format : wl<unit>.<subunit>
	 */
	if (vif != -1) {
		*bsscfg_idx = vif;
	} else {
		*bsscfg_idx = 0;
	}

	return ret;
}

int
main(int argc, char **argv)
{
	cmd_t *cmd = NULL;
	int err = 0;
	char *ifname = NULL;
	int help = 0;
	int status = CMD_DHD;
	int ret = 0;

	dhdu_av0 = argv[0];
	memset(interface_name, 0, sizeof(interface_name));

	for (++argv; *argv;) {

		/* command option */
		if ((status = dhd_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (ifname)
			{
				strncpy(interface_name, ifname, IFNAMSIZ);
			}
			if (help)
				break;
			continue;
		}

		/* parse error */
		else if (status == CMD_ERR)
			break;

		if ((ret = dhd_check((void *)ifname))) {
			fprintf(stderr, "%s: dhd driver adapter not found, ret = %d \n",
				dhdu_av0, ret);
			exit(1);
		}

		/* search for command */
		for (cmd = dhd_cmds; cmd->name && strcmp(cmd->name, *argv); cmd++);

		/* defaults to using the set_var and get_var commands */
		if (cmd->name == NULL)
			cmd = &dhd_varcmd;

		/* do command */
		if (cmd->name)
			err = (*cmd->func)((void *)ifname, cmd, argv);
		break;
	}

	if (!cmd)
		dhd_usage(NULL);
	else if (err == BCME_USAGE_ERROR)
		dhd_cmd_usage(cmd);
	else if (err == BCME_IOCTL_ERROR)
		dhd_printlasterror((void *)ifname);

	return err;
}
