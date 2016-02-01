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
#ifndef BICP_PRIV_H__
#define BICP_PRIV_H__

#include "bicp.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	This function sets the glitch rejector count

Description:
	This function is used to set the glitch rejector count for an ICP channel
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BICP_P_SetRejectCnt( 
	BICP_ChannelHandle 	hChn,			/* Device channel handle */
	uint8_t 			clks			/* number of clocks to reject */
	);

/***************************************************************************
Summary:
	This function enables ICAP int

Description:
	This function is used to enable an ICAP pin interrupt
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
void BICP_P_EnableInt(
	BICP_ChannelHandle 	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function disables ICAP int

Description:
	This function is used to disable an ICAP pin interrupt
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
void BICP_P_DisableInt(
	BICP_ChannelHandle 	hChn			/* Device channel handle */
	);

/***************************************************************************
Summary:
	This function is the ICAP ISR

Description:
	This function is the interrupt service routine for ICAP interrupt.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
static void BICP_P_HandleInterrupt_Isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
);

/***************************************************************************
Summary:
	This function is the RC6 protocol handler

Description:
	This function is will interpret the manchester encoding of the RC6
	protocol.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
static void BICP_P_RC6Handle
(
	BICP_ChannelHandle hIcpChan, 		/* Device channel handle */
	unsigned char reg					/* Value of icap status */
);

#ifdef __cplusplus
}
#endif
 
#endif



