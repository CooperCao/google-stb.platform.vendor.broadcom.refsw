/*
 * EFI-DHD dhd app
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifdef EFI_WINBLD
#include "efi.h"
#include "efilib.h"
#include <atk_libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <typedefs.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#ifndef APPLE_BUILD
#include <Library/ShellCEntryLib.h>
#endif
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Library/EfiFileLib.h>
#include <Protocol/LoadFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#define PATH_MAX 256
#endif /* EFI_WINBLD */

#include <errno.h>
#include <dhdioctl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#ifdef EFI_WINBLD
#include <brcmefi.h>
#include <apple80211.h>
#else
#include <efi/brcmefi.h>
#include <efi/apple80211.h>
#endif
#include <dhdu.h>
#include <dhdu_common.h>

#define FS_PREFIX "fs"

static int rwl_os_type = EFI;
EFI_GUID BcmdhdIoctlSelector = BCMDHD_IOCTL_GUID;
EFI_GUID Apple80211Protocol = APPLE_80211_PROTOCOL_GUID;
APPLE_80211_PROTOCOL *gAppleproto = NULL;

/* dword align allocation */
static union {
	char bufdata[DHD_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_dhd;
static char *buf = (char*)&bufstruct_dhd.bufdata;

#if !defined(EFI_WINBLD) && !defined(APPLE_BUILD)
void
bzero(void *p, size_t size)
{
	memset(p, 0, size);
}
#endif /* EFI_WINBLD */

static int
dhd_ioctl(void *dhd, int cmd, void *buf, int len, bool set)
{
	int ret = 0;
	EFI_GUID cmd_guid;
	EFI_STATUS Status;

	bcopy(&BcmdhdIoctlSelector, &cmd_guid, sizeof(EFI_GUID));
	*((uint32 *)(&cmd_guid.Data4[4])) = cmd;

#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
	Status = LibLocateProtocol(&Apple80211Protocol, (VOID **)&gAppleproto);
#else
	Status = gBS->LocateProtocol(&Apple80211Protocol, NULL, (VOID **)&gAppleproto);
#endif

	if (Status != EFI_SUCCESS) {
		Print(L"Apple Proto failed: %r\n", Status);
		return -1;
	} else {
		Status = gAppleproto->Ioctl(gAppleproto, &cmd_guid, buf, len, NULL, NULL);
	}

	/* Sending a 3rd error status will prevent from printing the usage */
	if (Status != EFI_SUCCESS)
		return -3;
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

	return BCME_ERROR;

}


int
wl_validatedev(void *dev_handle)
{
	return 0;
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
	return 0;
}

void
rwl_shell_killproc(int pid)
{

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
	return 0;
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

#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
	InitializeLib(_LIBC_EFIImageHandle, _LIBC_EFISystemTable);
#endif

	for (++argv; *argv;) {

		/* command option */
		if ((status = dhd_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (help)
				break;
			continue;
		}

		/* parse error */
		else if (status == CMD_ERR) {
			break;
			}
		if ((ret = dhd_check((void *)ifname))) {
			fprintf(stderr, "%s: dhd driver adapter not found, ret = %d \n",
				dhdu_av0, ret);
			return 1;
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

int
dhd_control_signal(void *dhd, cmd_t *cmd, char **argv)
{
	char *varname;
	char *endptr = NULL, *endptr2 = NULL;
	struct control_signal_ops sigops, *sigops_ptr;
	int err = 0;

	if (cmd->set == -1) {
		printf("set not defined for %s\n", cmd->name);
		return BCME_ERROR;
	}

	varname = *argv++;
	strcpy(buf, varname);
	endptr2 = buf + strlen(buf) + 1;

	if (!strcmp(*argv, "wl_reg_on"))
		sigops.signal = WL_REG_ON;
	 else if (!strcmp(*argv, "device_wake"))
		sigops.signal = DEVICE_WAKE;
	 else if (!strcmp(*argv, "time_sync"))
		sigops.signal = TIME_SYNC;
	 else {
		fprintf(stderr, "Invalid signal name %s, valid signal name "
			"is wl_reg_on/device_wake\n", argv[1]);
		return BCME_USAGE_ERROR;
	}
	argv++;
	if (*argv) {
		sigops.val = strtol(*argv, &endptr, 0);
		if (*endptr != '\0') {
			/* not all the value string was parsed by strtol */
			printf("set: error parsing value \"%s\" as an integer for set of \"%s\"\n",
				*argv, varname);
			return BCME_USAGE_ERROR;
		}
		memcpy(endptr2, (void *)&sigops, sizeof(sigops));
		endptr2 += sizeof(sigops);
		err = dhd_set(dhd, DHD_SET_VAR, &buf[0], (int)(endptr2 - buf));
	}
	else {
		memcpy(endptr2, (void *)&sigops, sizeof(sigops));
		sigops_ptr = (struct control_signal_ops *)endptr2;
		endptr2 += sizeof(sigops);
		err = dhd_get(dhd, DHD_GET_VAR, &buf[0], (int)(endptr2 - buf));
		if (!err)
			printf("%d\n", sigops_ptr->val);
	}
	return err;
}
