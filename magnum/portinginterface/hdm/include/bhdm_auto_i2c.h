/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * This HDMI Auto i2c definitionsl
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
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
void BHDM_AUTO_I2C_SetChannels_isrsafe(const BHDM_Handle hHDMI,
	uint8_t enable) ;


#ifdef __cplusplus
}
#endif


#endif /* BHDM_AUTO_I2C_H__ */
