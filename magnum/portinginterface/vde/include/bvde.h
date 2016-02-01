/***************************************************************************
 *     Copyright (c) 2006-2014, Broadcom Corporation
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
 * Module Description: Video Processing Interface for DSP
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BVDE_H_
#define BVDE_H_

#include "bdsp.h"

/************************** Module Overview ******************************** 
VDE (Video Decoder Engine) is a porting interface (PI) module that 
encapsulates the video processing using the DSP. 
 
Device: 
-------
An VDE Device is identified by a device handle (BVDE_Handle).  This is the 
top-level interface to the video decode subsystem using DSP.
 
Channel: 
-------- 
A VDE decoder is capable of decoding digital video into the display buffers 
This is identified by a BVDE_ChannelHandle.  Decoding is done in the DSP 
processor, and the output is put in the Picture Display Queues.  

Outputs:
-------- 
Currently the only output supported is the Picture Display Queue (PDQ).
 
Inputs: 
------- 
Inputs to the Decoder can be provided as a start time parameter of the video
decoder. Only CDB/ITB is supported currently
 
Example : Single video decoder : 
-------------------------------- 
CDB/ITB -> BVDE_ChannelHandle -> BVDE_PDQ
 
 
***************************************************************************/


/***************************************************************************
Summary:
Device Handle
***************************************************************************/
typedef struct BVDE_Device *BVDE_Handle;


#include "bvde_channel.h"

/***************************************************************************
Summary:
Device Settings
***************************************************************************/
typedef struct BVDE_Settings
{
	unsigned maxDspTasks;   /* Maximum DSP tasks.  One task is required for each decoder */

    unsigned stripeWidth;   /* TBD: XVD has this internally #defined. Need to see if Nexus can provide it*/

} BVDE_Settings;

/***************************************************************************
Summary:
Get default settings for a video decoder device
***************************************************************************/
void BVDE_GetDefaultSettings(
    BVDE_Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open a video decoder device
***************************************************************************/
BERR_Code BVDE_Open(
    BVDE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMMA_Heap_Handle mmaHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    BDSP_Handle dspHandle,
    const BVDE_Settings *pSettings  /* NULL will use default settings */
    );

/***************************************************************************
Summary:
Close a video decoder device
***************************************************************************/
void BVDE_Close(
    BVDE_Handle handle
    );

/***************************************************************************
Summary:
Device Level Interrupts
***************************************************************************/
typedef struct BVDE_InterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;        
    } watchdog;
} BVDE_InterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BVDE_GetInterruptHandlers(
    BVDE_Handle handle,
    BVDE_InterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BVDE_SetInterruptHandlers(
    BVDE_Handle handle,
    const BVDE_InterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Process a watchdog interrupt
***************************************************************************/
BERR_Code BVDE_ProcessWatchdogInterrupt(
    BVDE_Handle handle
    );

#endif
