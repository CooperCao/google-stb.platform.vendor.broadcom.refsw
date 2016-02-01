/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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

#ifndef BTC2_PRIV_H__
#define BTC2_PRIV_H__

#include "bchp.h"
#include "btc2.h"

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
	The handle for Qam In-Band Downstream module.

Description:

See Also:
	BTC2_Open()

****************************************************************************/
typedef struct BTC2_P_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BTC2_Settings settings;

	void *pImpl;						/* Device specific structure */
} BTC2_P_Handle;

/***************************************************************************
Summary:
	The handle for Qam In-Band Downstream module.

Description:

See Also:
	BTC2_OpenChannel()

****************************************************************************/
typedef struct BTC2_P_ChannelHandle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	BTC2_Handle hTc2;

	void *pImpl;						/* Device specific structure */
} BTC2_P_ChannelHandle;



#ifdef __cplusplus
}
#endif
 
#endif

