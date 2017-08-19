/*
 * Common EFI interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wlu.c 549439 2015-04-15 23:22:11Z $
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 */


#ifndef _BRCMEFI_H_
#define _BRCMEFI_H_

#define BCMWL_GUID \
	{0x21981f75, 0x0e3f, 0x4a04, { 0x83, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}

#ifndef BCMWL_IOCTL_GUID
#define BCMWL_IOCTL_GUID \
	{0xB4910A35, 0x88C5, 0x4328, { 0x90, 0x08, 0x9F, 0xB2, 0x00, 0x00, 0x0, 0x0 }}
#endif

#ifndef BCMDHD_IOCTL_GUID
#define BCMDHD_IOCTL_GUID \
	{0xB4910A35, 0x88C5, 0x4328, { 0x90, 0x08, 0x9F, 0xB3, 0x00, 0x00, 0x0, 0x0 }}
#endif

typedef struct {
	UINT32 State;
	EFI_MAC_ADDRESS HwAddress;
} BRCM_MODE;


typedef EFI_STATUS (EFIAPI *BRCM_IOCTL) (
	IN void		*This,
	IN EFI_GUID	*SelectorGuid,
	IN VOID		*InParams,
	IN UINTN	InSize,
	OUT VOID	*OutParams,
	IN OUT UINT32	*OutSize
);

typedef struct _BCMWL_PROTO_ {
	BRCM_IOCTL		Ioctl;
	BRCM_MODE		*mode;
} BCMWL_PROTO;

enum brcm80211_state {
	BRCM_80211_S_UNDEF     = 0,  /* default state (unpowered?) */
	BRCM_80211_S_IDLE      = 1,  /* powered & idle */
	BRCM_80211_S_SCAN      = 2,  /* scanning */
	BRCM_80211_S_ASSOC     = 3,  /* associating (includes 80211 Auth) */
	BRCM_80211_S_AUTH      = 4,  /* upper level authenticating */
	BRCM_80211_S_RUN       = 5,  /* associated (and upper level auth complete) failure states */
	BRCM_80211_S_BADKEY    = 6,  /* incorrect password  */
	BRCM_80211_S_BADAUTH   = 7,  /* auth method not supported */
	BRCM_80211_S_AUTHFAIL  = 8,  /* authentication failure */
	BRCM_80211_S_ASSOCFAIL = 9,  /* association failure */
	BRCM_80211_S_NONETWORK = 10, /* desired ssid not found */
	BRCM_80211_S_OTHER     = 11, /* indeterminate failure */
};


#endif /* _BRCMEFI_H_ */
