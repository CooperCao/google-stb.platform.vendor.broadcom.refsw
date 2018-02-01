/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
void BICP_P_EnableInt_isr(
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
void BICP_P_DisableInt_isr(
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
