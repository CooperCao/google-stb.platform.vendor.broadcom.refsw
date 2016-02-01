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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


/*= Module Overview *********************************************************
<verbatim>

Overview
The ICP PI module controls the Input Capture core within the BCM7038.  
The ICP core supports up to 4 separate capture pins.  Each pin is individually
configurable as positive or negative edge triggered.  When a pin is triggered,
the count is captured and can be read.

Design
The design for BICP PI API is broken into two parts.
  Part 1 (open/close/configuration):
    These APIs are used for opening and closing BICP device/device channel.
    When a device channel is opened, the glitch rejector count for
	that channel can be programmed.
    
  Part 2 (command):
    These APIs are used to configure and arm the trigger.

Usage
The usage of BICP involves the following:
   * Configure/Open of BICP
      * Configure BICP device for the target system
      * Open BICP device
      * Configure BICP device channel for the target system
      * Open BICP device channel
   * Configure and arm the trigger
      * Configure edge triggerring
   * Get count after trigger
      * Read counter after edge is triggered

Sample Code
void main( void )
{
    BICP_Handle       hIcp;
    BICP_ChannelHandle   hIcpChan;
    BREG_Handle       hReg;
    BCHP_Handle       hChip;
    BINT_Handle       hInt;
    BICP_ChannelSettings defChnSettings;
    unsigned int ChanNo;
    bool triggered = FALSE;
    uint16_t cnt;

    // Do other initialization, i.e. for BREG, BCHP, etc
    BICP_Open( &hIcp, hChip, hReg, hInt, (BICP_Settings *)NULL );
   
    ChanNo = 0;	// example for channel 0
    BICP_GetChannelDefaultSettings( hIcp, ChanNo, &defChnSettings );

    // Make any changes required from the default values
    defChnSettings.rejectCnt = 8;
    BICP_OpenChannel( hIcp, &hIcpChan, ChanNo, &defChnSettings );

    // Set pin to negative edge trigger
    BICP_EnableEdge (hIcpChan, BICP_EdgeConfig_eNegative);

    // Wait until the pin is triggered
    do
    {
        BICP_PollTimer (hIcpChan, &triggered, &cnt);
    } while (!triggered);
}

</verbatim>
***************************************************************************/


#ifndef BICP_H__
#define BICP_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Maximum number of PWM channels supported

Description:

See Also:

****************************************************************************/
#define MAX_ICP_CHANNELS			4

/***************************************************************************
Summary:
	Error Codes specific to BICP

Description:

See Also:

****************************************************************************/
#define BICP_ERR_NOTAVAIL_CHN_NO			BERR_MAKE_CODE(BERR_ICP_ID, 0)

/***************************************************************************
Summary:
	The handles for icp module.

Description:
	Since BICP is a device channel, it has main device handle as well
	as a device channel handle.

See Also:
	BICP_Open(), BICP_OpenChannel()

****************************************************************************/
typedef struct BICP_P_Handle				*BICP_Handle;
typedef struct BICP_P_ChannelHandle			*BICP_ChannelHandle;

/***************************************************************************
Summary:
	Enumeration for edge configuration

Description:
	This enumeration defines edge trigger configuration for each ICP pin

See Also:
	None.

****************************************************************************/
typedef enum
{
   BICP_EdgeConfig_eNegative,
   BICP_EdgeConfig_ePositive,
   BICP_EdgeConfig_eBoth
} BICP_EdgeConfig;

/***************************************************************************
Summary:
	Required default settings structure for ICP module.

Description:
	The default setting structure defines the default configure of
	ICP when the device is opened.  Since BICP is a device
	channel, it also has default settings for a device channel.
	Currently there are no parameters for device setting.

See Also:
	BICP_Open(), BICP_OpenChannel()

****************************************************************************/
typedef void *BICP_Settings;

typedef struct BICP_ChannelSettings
{
   uint8_t	rejectCnt;					/* glitch reject count */
   bool		intMode;					/* interrupt mode flag */
} BICP_ChannelSettings;

/***************************************************************************
Summary:
	Callback used for event notification.

Description:
	When this PI wants to notify an application, it will call this callback
	function the callback function is registered.

See Also:
	None.

****************************************************************************/
typedef void (*BICP_Callback)(uint16_t ctrl, uint8_t mode, uint16_t data);

/***************************************************************************
Summary:
	This function opens ICP module.

Description:
	This function is responsible for opening BICP module. When BICP is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BICP_Close(), BICP_OpenChannel(), BICP_CloseChannel(),
	BICP_GetDefaultSettings()

****************************************************************************/
BERR_Code BICP_Open(
	BICP_Handle *pICP,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	BINT_Handle hInterrupt,				/* [in] Interrupt handle */
	const BICP_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes ICP module.

Description:
	This function is responsible for closing BICP module. Closing BICP 
	will free main BICP handle. It is required that all opened 
	BICP channels must be closed before calling this function. If this 
	is not done, the results will be unpredicable.

Returns:
	TODO:

See Also:
	BICP_Open(), BICP_CloseChannel()

****************************************************************************/
BERR_Code BICP_Close(
	BICP_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for ICP module.

Description:
	This function is responsible for returns the default setting for 
	BICP module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BICP_Open()

****************************************************************************/
BERR_Code BICP_GetDefaultSettings(
	BICP_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function returns the total number of channels supported by 
	ICP module.

Description:
	This function is responsible for getting total number of channels
	supported by BICP module, since BICP device is implemented as a
	device channel.

Returns:
	TODO:

See Also:
	BICP_OpenChannel(), BICP_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BICP_GetTotalChannels(
	BICP_Handle hDev,					/* [in] Device handle */
	unsigned int *totalChannels			/* [out] Returns total number downstream channels supported */
	);

/***************************************************************************
Summary:
	This function gets default setting for a ICP module channel.

Description:
	This function is responsible for returning the default setting for
	channel of BICP. The return default setting is used when opening
	a channel.

Returns:
	TODO:

See Also:
	BICP_OpenChannel()

****************************************************************************/
BERR_Code BICP_GetChannelDefaultSettings(
	BICP_Handle hDev,					/* [in] Device handle */
	unsigned int channelNo,				/* [in] Channel number to default setting for */
    BICP_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
	This function opens ICP module channel.

Description:
	This function is responsible for opening BICP module channel. When a
	BICP channel is	opened, it will create a module channel handle and
	configure the module based on the channel default settings. Once a 
	channel is opened, it must be closed before it can be opened again.

Returns:
	TODO:

See Also:
	BICP_CloseChannel(), BICP_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BICP_OpenChannel(
	BICP_Handle hDev,					/* [in] Device handle */
	BICP_ChannelHandle *phChn,			/* [out] Returns channel handle */
	unsigned int channelNo,				/* [in] Channel number to open */
	const BICP_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
	);

/***************************************************************************
Summary:
	This function closes ICP module channel.

Description:
	This function is responsible for closing BICP module channel. Closing
	BICP channel it will free BICP channel handle. It is required that all
	opened BICP channels must be closed before closing BICP.

Returns:
	TODO:

See Also:
	BICP_OpenChannel(), BICP_CloseChannel()

****************************************************************************/
BERR_Code BICP_CloseChannel(
	BICP_ChannelHandle hChn				/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function gets ICP module device handle based on
	the device channel handle.

Description:
	This function is responsible returning BICP module handle based on the
	BICP module channel.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BICP_GetDevice(
	BICP_ChannelHandle hChn,			/* [in] Device channel handle */
	BICP_Handle *pICP					/* [out] Returns Device handle */
	);

/***************************************************************************
Summary:
	This function enables an edge trigger

Description:
	This function is used to configure an edge and arm the trigger.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BICP_EnableEdge( 
	BICP_ChannelHandle 	hChn,			/* [in] Device channel handle */
	BICP_EdgeConfig 	edge			/* [in] edge config */
	);

/***************************************************************************
Summary:
	This function disables an edge trigger

Description:
	This function is used to configure an edge to disable triggering
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BICP_DisableEdge( 
	BICP_ChannelHandle 	hChn,			/* [in] Device channel handle */
	BICP_EdgeConfig 	edge			/* [in] edge config */
	);

/***************************************************************************
Summary:
	This function gets the timer count for a channel.

Description:
	This function is used to read the counter of an ICP channel after a trigger
	happens.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BICP_GetTimerCnt( 
	BICP_ChannelHandle 	hChn,			/* [in] Device channel handle */
	uint16_t 			*timerCnt		/* [out] pointer to count */
	);

/***************************************************************************
Summary:
	This function checks to see if a trigger has happened.

Description:
	This function is used to poll the status of a trigger.  If a trigger
	happens, it reads and outputs the counter.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BICP_PollTimer( 
	BICP_ChannelHandle 	hChn,			/* [in] Device channel handle */
	bool 				*triggered,		/* [out] status of trigger */
	uint16_t 			*timerCnt		/* [out] pointer to count */
	);

/***************************************************************************
Summary:
	This function gets the event handle for BICP module channel.

Description:
	This function is responsible for getting the event handle. The
	application code should use this function get BICP's event handle, 
	which the application should use to pend on.  The ICP ISR will 
	set the event.
	
Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BICP_GetEventHandle(
	BICP_ChannelHandle hChn,			/* [in] Device channel handle */
	BKNI_EventHandle *phEvent			/* [out] Returns event handle */
	);

/***************************************************************************
Summary:
	This function enables the icap interrupt..

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_EnableInt(
	BICP_ChannelHandle hChn				/* [in] Device channel handle */
	);

/***************************************************************************
Summary:
	This function disables the icap interrupt..

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_DisableInt(
	BICP_ChannelHandle hChn				/* [in] Device channel handle */
	);
	
/***************************************************************************
Summary:
	This function resets the interrupt count.

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_ResetIntCount(
	BICP_ChannelHandle hChn				/* [in] Device channel handle */
	);
	
/***************************************************************************
Summary:
	This function gets the interrupt count.

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_GetIntCount(
	BICP_ChannelHandle hChn,			/* [in] Device channel handle */
	uint32_t *data
	);
	
/***************************************************************************
Summary:
	This function enables the handling of the RC6 protocol

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_EnableRC6(
	BICP_ChannelHandle hChn,			/* [in] Device channel handle */
	BICP_Callback pCallback	    		/* Pointer to completion callback. */
	);

/***************************************************************************
Summary:
	This function disables the handling of the RC6 protocol

Description:
	
Returns:
	TODO:

See Also:

****************************************************************************/
void BICP_DisableRC6(
	BICP_ChannelHandle hChn				/* [in] Device channel handle */
	);

#ifdef __cplusplus
}
#endif
 
#endif
