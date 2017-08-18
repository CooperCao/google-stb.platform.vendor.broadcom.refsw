/*
 * EFI port of wl command line utility
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

#ifdef EFI_WINBLD
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

#include <atk_libc.h>
#include "efi.h"
#include "efilib.h"
#include <bcmstdlib.h>
#include <typedefs.h>
#include "brcmefi.h"
#ifdef OLYMPIC_API
#include "apple80211.h"
#endif
#include <wlioctl.h>
#else /* EFI_WINBLD */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
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
#include <wlioctl.h>
#include <efi/brcmefi.h>
#ifdef OLYMPIC_API
#include <efi/apple80211.h>
#endif
#define PATH_MAX 256
#endif /* EFI_WINBLD */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include "wlu.h"

#define FS_PREFIX "fs"

EFI_GUID BcmWlIoctlSelector = BCMWL_IOCTL_GUID;

EFI_GUID Brcmwl = BCMWL_GUID;
BCMWL_PROTO *gBrcmwlproto = NULL;

#ifdef OLYMPIC_API
EFI_GUID Apple80211Protocol = APPLE_80211_PROTOCOL_GUID;
APPLE_80211_PROTOCOL *gAppleproto = NULL;
#endif

static cmd_t *wl_find_cmd(char* name);

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	int ret = 0;
	EFI_GUID cmd_guid;
	EFI_STATUS Status;

	bcopy(&BcmWlIoctlSelector, &cmd_guid, sizeof(EFI_GUID));

	*((uint32 *)(&cmd_guid.Data4[4])) = cmd;

#ifdef OLYMPIC_API
#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
	Status = LibLocateProtocol(&Apple80211Protocol, (VOID **)&gAppleproto);
#else
	Status = gBS->LocateProtocol(&Apple80211Protocol,
			NULL,
			(VOID **)&gAppleproto);
#endif /* EFI_WINBLD */
	if (Status != EFI_SUCCESS) {
		Print(L"Apple Proto failed: %r\n", Status);
		return -1;
	} else {
		Status = gAppleproto->Ioctl(gAppleproto, &cmd_guid, buf, len, NULL, NULL);
	}
#else
	Status = LibLocateProtocol(&Brcmwl, (VOID **)&gBrcmwlproto);
	if (Status != EFI_SUCCESS)  {
		Print(L"Brcm Proto failed: %r\n", Status);
		return -1;
	} else {
		Status = gBrcmwlproto->Ioctl(gBrcmwlproto, &cmd_guid, buf, len, NULL, NULL);
	}
#endif /* OLYMPIC_API */
	/* EFI driver prints the last error for ease of debugging as it can be
	 * used with apple's own tool
	 */
	/* Sending a 3rd error status will prevent from printing the usage */
	if (Status != EFI_SUCCESS) {
		return -3;
	}

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

void
wl_find(struct ifreq *ifr)
{
	/* Looks for wl0 f no arguments are given */
	strcpy((char *)ifr->ifr_name, "wl0");
}

int
main(int argc, char **argv)
{
	struct ifreq ifr;
	cmd_t *cmd = NULL;
	int help = FALSE;
	int err = 0;
	char *ifname = NULL;
	int status = 0;
	memset(&ifr, 0, sizeof(ifr));

	wlu_init();
#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
	InitializeLib(_LIBC_EFIImageHandle, _LIBC_EFISystemTable);
#endif

	wlu_av0 = argv[0];

	for (++argv; *argv;) {
		if ((status = wl_option(&argv, &ifname, &help)) == CMD_OPT) {
			if (help)
				break;
			continue;
		} else if (status == CMD_ERR)
			break;
		if (wl_check((void *)&ifr)) {
			fprintf(stderr, "wl driver adapter not found\n");
			return 1;
		}

		/* search for command */
		cmd = wl_find_cmd(*argv);

		/* defaults to using the set_var and get_var commands */
		if (!cmd)
			cmd = &wl_varcmd;

		/* do command */
		err = (*cmd->func)((void *) &ifr, cmd, argv);
		break;
	}

	if (help && *argv) {
		cmd = wl_find_cmd(*argv);
		if (cmd)
			wl_cmd_usage(stdout, cmd);
		else
			printf("%s: Unrecognized command \"%s\", type -h for help\n",
			       wlu_av0, *argv);
	}
	else if (!cmd) {
		if (help) {
			wl_usage(stdout, NULL); /* outputs multiple screens of help information */
		} else {
			fprintf(stderr, "type -h for help\n");
		}
	} else if (err == BCME_USAGE_ERROR)
		wl_cmd_usage(stderr, cmd);
	else if (err == BCME_IOCTL_ERROR)
		wl_printlasterror((void *) &ifr);
	return 0;
}

/* Search the  wl_cmds table for a matching command name.
 * Return the matching command or NULL if no match found.
 */
static cmd_t*
wl_find_cmd(char* name)
{
	return wlu_find_cmd(name);
}

#ifndef EFI_WINBLD
void
bzero(void *p, size_t size)
{
	memset(p, 0, size);
}
#endif /* EFI_WINBLD */


#if defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION >= 0x00020000)
char
_jp2uc(char c)
{
	return c;
}
#endif /* EDK_RELEASE_VERSION || EDK_RELEASE_VERSION >= 0x00020000 */
