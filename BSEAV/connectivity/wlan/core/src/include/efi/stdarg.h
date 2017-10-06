/*
 * EFI stdarg file that maps EFI defines
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

#ifndef _STDARG_H_
#define _STDARG_H_

#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
#include "Tiano.h"
#endif

typedef VA_LIST va_list;
#define va_start VA_START
#define va_end VA_END
#define va_arg VA_ARG
#endif /* _STDARG_H_ */
