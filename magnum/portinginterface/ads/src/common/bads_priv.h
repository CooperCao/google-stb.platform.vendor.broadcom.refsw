/***************************************************************************
 *     Copyright (c) 2005, Broadcom Corporation
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

#ifndef BADS_PRIV_H__
#define BADS_PRIV_H__

#include "bchp.h"
#include "bads.h"

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
	The handle for Qam In-Band Downstream module.

Description:

See Also:
	BADS_Open()

****************************************************************************/
typedef struct BADS_P_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BADS_Settings settings;

	void *pImpl;						/* Device specific structure */
} BADS_P_Handle;

/***************************************************************************
Summary:
	The handle for Qam In-Band Downstream module.

Description:

See Also:
	BADS_OpenChannel()

****************************************************************************/
typedef struct BADS_P_ChannelHandle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BADS_Handle hAds;

	void *pImpl;						/* Device specific structure */
} BADS_P_ChannelHandle;



#ifdef __cplusplus
}
#endif
 
#endif

