/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BVEE_H_
#define BVEE_H_

#include "bdsp.h"

/************************** Module Overview ******************************** 
VEE (Video Encoder Engine) is a porting interface (PI) module that 
encapsulates the video encoding using the DSP. 
 
Device: 
-------
An VEE Device is identified by a device handle (BVEE_Handle).  This is the 
top-level interface to the video encode subsystem using DSP.
 
Channel: 
-------- 
A VEE encoder is capable of encoding raw video content into the compressed buffers 
This is identified by a BVEE_ChannelHandle. Encoding 
is done in the DSP processor, and the output is put in the CDB/ITB buffer.

Outputs:
-------- 
Currently the only output supported is the CDB/ITB buffer.
 
Inputs: 
------- 
It is presumed that, either it is offline transcode or video phone usage, output buffer
of BVN is given as input to the VEE.
 
Example : Single video encoder : 
-------------------------------- 
Raw capture data or CDB/ITB ->VDC-> BVN -> BVEE_ChannelHandle -> BVEE_AVCtxt
 
 
***************************************************************************/


/***************************************************************************
Summary:
Device Handle
***************************************************************************/
typedef struct BVEE_Device *BVEE_Handle;

#include "bvee_channel.h"
/***************************************************************************
Summary:
VEE Device Settings
***************************************************************************/
typedef struct BVEE_OpenSettings
{
	unsigned maxdsptasks;   /* Maximum DSP tasks.  One task is required for each encode */
} BVEE_OpenSettings;


/***************************************************************************
Summary:
Get default settings for a video encoder device
***************************************************************************/
void BVEE_GetDefaultOpenSettings(
    BVEE_OpenSettings * pveeopensettings
    );

/***************************************************************************
Summary:
Open a video encoder device
***************************************************************************/
BERR_Code BVEE_Open(
    BVEE_Handle *phandle,   /* [out] returned handle */
    BCHP_Handle chphandle,
    BREG_Handle reghandle,
    BMMA_Heap_Handle mmahandle,
    BINT_Handle inthandle,
    BTMR_Handle tmrhandle,
    BDSP_Handle dsphandle,
    const BVEE_OpenSettings *psettings  /* NULL will use default settings */
    );

/***************************************************************************
Summary:
Close a video encoder device
***************************************************************************/
void BVEE_Close(
    BVEE_Handle handle
    );

/***************************************************************************
Summary:
Device Level Interrupts
***************************************************************************/
typedef struct BVEE_InterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;        
    } watchdog;
} BVEE_InterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BVEE_GetInterruptHandlers(
    BVEE_Handle handle,
    BVEE_InterruptHandlers *pinterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BVEE_SetInterruptHandlers(
    BVEE_Handle handle,
    const BVEE_InterruptHandlers *pinterrupts
    );
/***************************************************************************
Summary:
Process a watchdog interrupt
***************************************************************************/
BERR_Code BVEE_ProcessWatchdogInterrupt(
    BVEE_Handle handle
    );
/***************************************************************************
Summary:
Routine to get the Arrival to Presentation delay of Raaga video
encoder for different parameters.
***************************************************************************/
BERR_Code BVEE_GetA2PDelay(
    uint32_t *delayms,
    BVEE_Resolution resolution,
    BAVC_FrameRateCode framerate,
    uint32_t targetbitrate);
#endif
