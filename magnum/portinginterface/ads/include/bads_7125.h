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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#ifndef BADS_7125_H__
#define BADS_7125_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bads.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	This function returns the default settings for Qam In-Band Downstream module.

Description:
	This function is responsible for returns the default setting for 
	BADS module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BADS_7125_Open()

****************************************************************************/
BERR_Code BADS_7125_GetDefaultSettings(
	BADS_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);


/***************************************************************************
Summary:
    Data for the TNR->ADS interrupt callback
****************************************************************************/
typedef struct BADS_7125_TnrInterruptData
{
	enum 
	{
	  BADS_7125_SetDpm,
	  BADS_7125_ResetDpm,
	  BADS_7125_RequestLnaGain
	} action;
    uint8_t iOutdivMs;
} BADS_7125_TnrInterruptData;

/***************************************************************************
Summary:
    BADS_7125_ProcessTnrInterrupt_isr
****************************************************************************/
BERR_Code BADS_7125_ProcessTnrInterrupt_isr(
	BADS_ChannelHandle hChannel,
	const BADS_7125_TnrInterruptData *pInterruptData
    );



#ifdef __cplusplus
}
#endif
 
#endif

