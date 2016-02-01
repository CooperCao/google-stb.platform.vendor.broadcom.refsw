/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef PSIP_COMMON_H__
#define PSIP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	PSIP_no_ETM,
	PSIP_ETM,
	PSIP_ETM_channel_TSID
} PSIP_ETM_location;

typedef const uint8_t * PSIP_MSS_string;

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
