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
#ifndef BHDM_AUTO_I2C_H__
#define BHDM_AUTO_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif


/* i2c channel (0..3) channel assignments */
typedef enum BHDM_AUTO_I2C_P_CHANNEL
{
	BHDM_AUTO_I2C_P_CHANNEL_ePollHdcp22RxStatus ,
	BHDM_AUTO_I2C_P_CHANNEL_ePollScdcUpdate0,
	BHDM_AUTO_I2C_P_CHANNEL_eRead,
	BHDM_AUTO_I2C_P_CHANNEL_eWrite,
	BHDM_AUTO_I2C_P_CHANNEL_eMax
} BHDM_AUTO_I2C_P_CHANNEL ;

/******************************************************************************
Summary:
Enumerated Type of different Auto I2c events available from the HDMI core

Description:
The HDMI core can support up to four automated I2c channels

See Also:
	o BHDM_AUTO_I2C_GetEventHandle

*******************************************************************************/
typedef enum
{
	BHDM_AUTO_I2C_EVENT_eScdcUpdate,
	BHDM_AUTO_I2C_EVENT_eHdcp22RxStatusUpdate,
	BHDM_AUTO_I2C_EVENT_eWrite,
	BHDM_AUTO_I2C_EVENT_eRead
} BHDM_AUTO_I2C_EVENT ;


/***************************************************************************
Summary:
	Get the event handle for checking HDMI Auto I2c events.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eEventChannel - Auto I2c event the requested handle is for

Output:
	pAutoI2cEvent - HDMI Auto I2c Event Handle

Returns:
	BERR_SUCCESS - Sucessfully returned the HDMI handle
	BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
	BHDM_AUTO_I2C_EVENT

****************************************************************************/
BERR_Code BHDM_AUTO_I2C_GetEventHandle(
   const BHDM_Handle hHDMI,           /* [in] HDMI handle */
   BHDM_AUTO_I2C_EVENT eEventChannel,
   BKNI_EventHandle *pAutoI2cEvent	/* [out] event handle */
) ;


/******************************************************************************
Summary:
Get the SCDC Update Data read by Auto I2c

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to read from (previously configured)
	length - size of bytes to read (NOTE: length is ignored for SCDC data)

Output:
	pBuffer - contains data from the read

Returns:
	<None>

See Also:
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_GetScdcData(const BHDM_Handle hHDMI,
	uint8_t *pBuffer, uint8_t length) ;


/******************************************************************************
Summary:
Get the HDCP Rx Status read by Auto I2c

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to read from (previously configured)
	length - size of bytes to read (NOTE: length is ignored for Hdcp Rx data)

Output:
	pBuffer - contains data from the read

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER

See Also:
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_GetHdcp22RxStatusData(const BHDM_Handle hHDMI,
	uint8_t *pBuffer, uint8_t length) ;


/******************************************************************************
Summary:

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	eChannel - Channel to read from (previously configured)
	enable - enable / disable of a configured Auto I2C Channel

Output:
	None.

Returns:
	BERR_SUCCESS
	BERR_INVALID_PARAMETER

See Also:
*******************************************************************************/

void BHDM_AUTO_I2C_EnableReadChannel(const BHDM_Handle hHDMI,
	BHDM_AUTO_I2C_P_CHANNEL eChannel, uint8_t enable
) ;

/******************************************************************************
Summary:  Enable/Disable Active Polling channels

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.
	enable - enable / disable of active Auto I2C Channel

Output:
	None.

Returns:
	None.

See Also:
*******************************************************************************/
void BHDM_AUTO_I2C_SetChannels_isr(const BHDM_Handle hHDMI,
	uint8_t enable) ;



/******************************************************************************
Summary: Check status of the Auto I2C HDCP HW Timers.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:
	availble - bool indicated whether the Auto I2C HW is current available

Returns:
	None.

See Also:
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_IsHdcp2xHWTimersAvailable_isrsafe(
	const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
	bool *available
);


/******************************************************************************
Summary: Reset Auto I2C block. This should only be called if the Auto I2C HDCP HW Timers are
		stuck.

Input:
	hHDMI - HDMI control handle that was previously opened by BHDM_Open.

Output:


Returns:
	None.

See Also:
*******************************************************************************/
BERR_Code BHDM_AUTO_I2C_Reset_isr(
	const BHDM_Handle hHDMI		   /* [in] HDMI handle */
);


#ifdef __cplusplus
}
#endif


#endif /* BHDM_AUTO_I2C_H__ */
