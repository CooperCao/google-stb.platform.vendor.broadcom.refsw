/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Porting interface code for the rate smoothing buffer block in the 
 * transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXPT_RS_BUFFER_H__
#define BXPT_RS_BUFFER_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

#if BXPT_HAS_FIXED_RSBUF_CONFIG
	/* Newer chips set the RS config through the defaults struct at BXPT_Open() */
#else

/*= Module Overview *********************************************************

Overview

Usage / Sample Code

***************************************************************************/

/***************************************************************************
Summary:
Configure the given input band rate-smoothing buffers peak rate.

Description:
Set the peak input rate for the rate-smoothing buffer. Each input band has its
own buffer; this call allows each buffer to support a unique peak rate. The 
caller specifies the peak rate in Mbps. 

Returns:
    BERR_SUCCESS                - Defaults loaded into struct.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_RsBuf_SetBufferSize					
****************************************************************************/
BERR_Code BXPT_RsBuf_SetBandDataRate(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum,				/* [in] Which input band parser to configure */
	unsigned long PeakRate,			/* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen              /* [in] Packet size ,130 for dss and 188 for mpeg */
	);

/***************************************************************************
Summary:
Configure the given input band rate-smoothing buffer's size.

Description:
Set the DRAM buffer size for the rate-smoothing buffer.  Each input band has 
its own buffer; this call allows customizing each buffer's size. The DRAM 
buffer size is specified in bytes, but must be a multiple of 256; the size 
will be rounded down to the nearest multiple otherwise.

The buffer is allocated internally from the system heap. It will be freed 
when the XPT porting interface is closed.

Returns:
    BERR_SUCCESS                - Defaults loaded into struct.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:	
BXPT_RsBuf_SetBandDataRate				
****************************************************************************/
BERR_Code BXPT_RsBuf_SetBufferSize(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum,				/* [in] Which input band parser to configure */
	unsigned long BufferSize		/* [in] Buffer size in bytes */
	);

/***************************************************************************
Summary:
Configure the given Playback channel rate-smoothing buffers peak rate.

Description:
Set the peak input rate for the rate-smoothing buffer. Each playback channel
has its own buffer; this call allows each buffer to support a unique peak 
rate. The caller specifies the peak rate in Mbps. 

Returns:
    BERR_SUCCESS                - Defaults loaded into struct.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_RsBuf_SetPlaybackBufferSize					
****************************************************************************/
BERR_Code BXPT_RsBuf_SetPlaybackDataRate(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned PbNum,					/* [in] Which playback channel to configure */
	unsigned long PeakRate			/* [in] Max data rate (in bps) the band will handle. */
	);

/***************************************************************************
Summary:
Configure the given Playback channels rate-smoothing buffer's size.

Description:
Set the DRAM buffer size for the rate-smoothing buffer.  Each Playback channel has 
its own buffer; this call allows customizing each buffer's size. The DRAM 
buffer size is specified in bytes, but must be a multiple of 256; the size 
will be rounded down to the nearest multiple otherwise.

The buffer is allocated internally from the system heap. It will be freed 
when the XPT porting interface is closed.

Returns:
    BERR_SUCCESS                - Defaults loaded into struct.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:	
BXPT_RsBuf_SetPlaybackDataRate				
****************************************************************************/
BERR_Code BXPT_RsBuf_SetPlaybackBufferSize(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum,				/* [in] Which input band parser to configure */
	unsigned long BufferSize		/* [in] Buffer size in bytes */
	);


/*
** These functions are called internally from BXPT_Open() and BXPT_Close(). 
** Users should NOT uses these functions directly.
*/

#define BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET		( 8 )

/* used in bxpt.c and bxpt_rsbuf.c */
#define RS_BUFFER_PTR_REG_STEPSIZE     (BCHP_XPT_RSBUFF_BASE_POINTER_IBP1 - BCHP_XPT_RSBUFF_BASE_POINTER_IBP0)

BERR_Code BXPT_P_RsBuf_Init(
	BXPT_Handle hXpt, 	   	   	/* [in] Handle for this transport */
    const BXPT_DramBufferCfg *Cfg
	);

BERR_Code BXPT_P_RsBuf_Shutdown(
	BXPT_Handle hXpt 	   	   	/* [in] Handle for this transport */
	);

bool BXPT_P_RsBuf_IsBufferEnabled( 
	BXPT_Handle hXpt, 
	unsigned BandNum 
	);

unsigned long BXPT_P_RsBuf_GetBlockout(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum				/* [in] Which input band parser to configure */
	);

BERR_Code BXPT_P_RsBuf_SetBlockout(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
	);


BERR_Code BXPT_P_RsBuf_PlaybackSetBlockout(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
	);

uint32_t BXPT_P_RsBuf_ComputeBlockOut( 
	unsigned long PeakRate,			/* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    );

#endif /* BXPT_HAS_FIXED_RSBUF_CONFIG */

#ifdef __cplusplus
}
#endif

#endif	/* BXPT_RS_BUFFER_H__ */


