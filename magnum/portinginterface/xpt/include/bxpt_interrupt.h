/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 * Porting interface code for the data transport interrupt handlers. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
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
	BINT_CallbackFunc Callback,		/* [in] Handler for this interrupt. */
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
	BINT_CallbackFunc Callback,		/* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);

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
	BINT_CallbackFunc Callback,		/* [in] Handler for this interrupt. */
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
	BINT_CallbackFunc Callback,		/* [in] Handler for this interrupt. */
	void *Parm1,					/* [in] First arg to be passed to callback */
	int Parm2						/* [in] Second arg to be passed to callback */
	);

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

