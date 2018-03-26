/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

/*= Module Overview *********************************************************
The interrupt module provides an API to install and remove transport 
interrupt handlers. Functions are available for use by porting interface
clients, to enable and disable interrupts. Enabling the interrupt also 
installs a pointer to high-level interrupt handler.

Why is a separate API for transport interrupts needed? The Interrupt Interface
used by other modules can only support so-called L2 interrupts. Unfortunately,
the transport hardware does not support the L2 mechanism; all transport 
interrupts are L1. 

Transport interrupts fall in to 3 classes: 
	Message Ready - A complete message has been transfered into the DRAM
	message buffers.

	Message Overflow - A given buffer was not serviced quickly enough and
	has overflowed. 

	CPU Status - A catch-all for all the other interrupts the transport can
	generate, such as continuity-counter error, playback channel finished,
	or PCR arrival. 

An Enable and Disable call is provided for Message Ready and Message Overflow
classes of interrupts. The CPU Status interrupts are now supported by the
Interrupt Interface.

Three function calls are provided for use by the low-level interrupt service
routines. These three calls map the 'status' interrupt, the message ready
interrupt, and the message overflow interrupt to the high-level handler 
which was installed when the interrupt was enabled.
***************************************************************************/

#ifndef BXPT_INTERRUPT_H__
#define BXPT_INTERRUPT_H__

#include "bxpt.h"
#include "bint.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Enable a given CPU Message interrupt.

Description:
Enable the given message buffer interrupt. The specified callback will be 
called when the a complete message has been transfered into the message
buffer. That message could be an entire transport packet, a complete PSI 
message, or complete PES packet, etc. 

This function CANNOT be called from within a Magnum interrupt context.

The Parm1 and Parm2 arguments passed in will be used as the 
arguments for the Callback function when the interrupt fires. 

Returns:
    BERR_SUCCESS                - Interrupt enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_DisableMessageInt, BXPT_Interrupt_EnableMessageInt_isr,
BXPT_Interrupt_DisableMessageInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_EnableMessageInt( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum,			/* [in] Which message buffer to watch. */
	BINT_CallbackFunc Callback_isr, /* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);

/***************************************************************************
Summary:
Disable a given CPU Message interrupt.

Description:
Disable the given message buffer interrupt. The transport core will no longer
forward interrupts for this particular message buffer to the CPU for 
processing. The event could still occur, but no interrupt will be generated.

This function CANNOT be called from within a Magnum interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt disabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_EnableMessageInt, BXPT_Interrupt_EnableMessageInt_isr,
BXPT_Interrupt_DisableMessageInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_DisableMessageInt( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum		/* [in] Message interrupt to disable. */
	);

#if !BXPT_HAS_MESG_L2
/***************************************************************************
Summary:
Enable a given CPU Message interrupt, interrupt context version.

Description:
A version of BXPT_Interrupt_EnableMessageInt(), callable from within an
interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_DisableMessageInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_EnableMessageInt_isr( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum,		/* [in] Which message buffer to watch. */
	BINT_CallbackFunc Callback_isr, /* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);
#endif

/***************************************************************************
Summary:
Disable a given CPU Message interrupt, interrupt context version.

Description:
A version of BXPT_Interrupt_DisableMessageInt(), callable from within an
interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_EnableMessageInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_DisableMessageInt_isr( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum		/* [in] Message interrupt to disable. */
	);

/***************************************************************************
Summary:
Enable a given CPU Message Overflow interrupt.

Description:
Enable the given message buffer overflow interrupt. The specified callback 
will be called when the an overflow condition has occurred.

The Parm1 and Parm2 arguments passed in will be used as the 
arguments for the Callback function when the interrupt fires. 

This function CANNOT be called from within a Magnum interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_DisableMessageOverflowInt, 
BXPT_Interrupt_EnableMessageOverflowInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_EnableMessageOverflowInt( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum,			/* [in] Which message buffer to watch. */
	BINT_CallbackFunc Callback_isr, /* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);

/***************************************************************************
Summary:
Disable a given CPU Message Overflow interrupt.

Description:
Disable the given message buffer overflow interrupt. The transport core will 
no longer forward overflow interrupts for this particular message buffer to 
the CPU. The event could still occur, but no interrupt will be generated.

This function CANNOT be called from within a Magnum interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt disabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_EnableMessageOverflowInt,
BXPT_Interrupt_DisableMessageOverflowInt_isr
****************************************************************************/
BERR_Code BXPT_Interrupt_DisableMessageOverflowInt( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum		/* [in] Message interrupt to disable. */
	);

#if !BXPT_HAS_MESG_L2
/***************************************************************************
Summary:
Enable a given CPU Message Overflow interrupt, interrupt context version.

Description:
A version of BXPT_Interrupt_EnableMessageOverflowInt(), callable from within
an interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_DisableMessageOverflowInt
****************************************************************************/
BERR_Code BXPT_Interrupt_EnableMessageOverflowInt_isr( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum,			/* [in] Which message buffer to watch. */
	BINT_CallbackFunc Callback_isr, /* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);
#endif

/***************************************************************************
Summary:
Disable a given CPU Message Overflow interrupt, interrupt context version.

Description:
A version of BXPT_Interrupt_DisableMessageOverflowInt(), callable from within
an interrupt context.

Returns:
    BERR_SUCCESS                - Interrupt disabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Interrupt_EnableMessageOverflowInt
****************************************************************************/
BERR_Code BXPT_Interrupt_DisableMessageOverflowInt_isr( 
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	int MessageBufferNum		/* [in] Message interrupt to disable. */
	);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_INTERRUPT_H__ */

/* end of file */
