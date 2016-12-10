/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BADS_PRIV_H__
#define BADS_PRIV_H__

#include "bchp.h"
#include "bads.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BERR_Code (*BADS_TuneIfDacFunc)(
    BADS_ChannelHandle hChn,                   /* [in] Device handle */
    BADS_IfDacSettings *pSettings  /* [in] IF DAC Settings */
    );

/***************************************************************************
Summary:
    This function resets the status of the IF DAC.

Description:
    This function is responsible for resetting If DAC status.

Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BADS_ResetIfDacStatusFunc)(
    BADS_ChannelHandle hChn        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for requesting the status to be calculated
    asynchronously for an IF DAC channel.

Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BADS_RequestIfDacStatusFunc)(
    BADS_ChannelHandle hChn        /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function gets the status asynchronously of the IF DAC channel.

Description:
    This function is responsible for asynchronously getting the complete status
    of the IF DAC.

Returns:
    TODO:

See Also:

****************************************************************************/
typedef BERR_Code (*BADS_GetIfDacStatusFunc)(
    BADS_ChannelHandle hChn,               /* [in] Device handle */
    BADS_IfDacStatus *pStatus  /* [in] IF DAC Status */
    );

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
    BADS_TuneIfDacFunc pTuneIfDac;  /* ptr to Tune IfDac function */
    BADS_ResetIfDacStatusFunc pResetIfDacStatus;    /* ptr to reset IfDac function */
    BADS_RequestIfDacStatusFunc pRequestIfDacStatus;    /* ptr to request IfDac function */
    BADS_GetIfDacStatusFunc pGetIfDacStatus;    /* ptr to get IfDac function */
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

