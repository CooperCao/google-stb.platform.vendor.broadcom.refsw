/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BBOX_RTS_H__
#define BBOX_RTS_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	uint32_t              ulAddr;
	uint32_t              ulData;
} BBOX_Rts_PfriClient;

/***************************************************************************
Summary: Specifies RTS settings for a box mode.

Description:
	This data structure specifies the RTS settings for a given box mode.

See Also:
	BBOX_LoadRts
****************************************************************************/
typedef struct BBOX_Rts
{
	char            *pchRtsVersion;
	uint32_t         ulChipFamilyId;
	uint32_t         ulBoxId;
	uint32_t         ulNumMemc;
	uint32_t         ulNumMemcEntries;
	const uint32_t **paulMemc;
	uint32_t         ulNumPfriClients;
	const BBOX_Rts_PfriClient *pastPfriClient;
} BBOX_Rts;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_RTS_H__ */

/* end of file */
