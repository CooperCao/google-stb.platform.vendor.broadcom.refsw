/*
 * Implementations of Broadcom secure string functions for non-Windows platforms
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

#ifndef __BCM_STR_H__
#define __BCM_STR_H__

#define bcm_strcpy_s(dst, noOfElements, src) \
	strcpy((dst), (src))
#define bcm_strncpy_s(dst, noOfElements, src, count) \
	strncpy((dst), (src), (count))
#define bcm_strcat_s(dst, noOfElements, src) \
	strcat((dst), (src))
#define bcm_sprintf_s(buffer, noOfElements, format, ...) \
	sprintf((buffer), (format) , ## __VA_ARGS__)

#endif /* __BCM_STR_H__ */
