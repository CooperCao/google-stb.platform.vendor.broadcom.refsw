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
 * Porting interface code for the transport client buffer block. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXPT_XCBUFF_H__
#define BXPT_XCBUFF_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

/*= Module Overview *********************************************************

Overview
Usage / Sample Code

%CODE{"cpp"}%


%ENDCODE%
***************************************************************************/

#if BXPT_HAS_FIXED_XCBUF_CONFIG
	/* Newer chips set the XC config through the defaults struct at BXPT_Open() */
#else

/***************************************************************************
Summary:
****************************************************************************/
typedef enum BXPT_XcBuf_Id
{
#if ( BCHP_CHIP == 7635 )|| ( BCHP_CHIP == 7630 ) 
    BXPT_XcBuf_Id_RMX0_IBP0, 
    BXPT_XcBuf_Id_RMX0_IBP1, 
    BXPT_XcBuf_Id_RMX0_IBP2, 
    BXPT_XcBuf_Id_RMX0_IBP3, 
    BXPT_XcBuf_Id_RMX0_IBP4, 
    BXPT_XcBuf_Id_RMX0_IBP5, 

    BXPT_XcBuf_Id_RMX0_PBP0, 
    BXPT_XcBuf_Id_RMX0_PBP1,
    BXPT_XcBuf_Id_RMX0_PBP2,
    BXPT_XcBuf_Id_RMX0_PBP3,

    BXPT_XcBuf_Id_RMX1_IBP0, 
    BXPT_XcBuf_Id_RMX1_IBP1, 
    BXPT_XcBuf_Id_RMX1_IBP2, 
    BXPT_XcBuf_Id_RMX1_IBP3, 
    BXPT_XcBuf_Id_RMX1_IBP4, 
    BXPT_XcBuf_Id_RMX1_IBP5, 

    BXPT_XcBuf_Id_RMX1_PBP0, 
    BXPT_XcBuf_Id_RMX1_PBP1, 
    BXPT_XcBuf_Id_RMX1_PBP2,
    BXPT_XcBuf_Id_RMX1_PBP3,

    BXPT_XcBuf_Id_RAVE_IBP0, 
    BXPT_XcBuf_Id_RAVE_IBP1, 
    BXPT_XcBuf_Id_RAVE_IBP2, 
    BXPT_XcBuf_Id_RAVE_IBP3, 
    BXPT_XcBuf_Id_RAVE_IBP4, 
    BXPT_XcBuf_Id_RAVE_IBP5, 

    BXPT_XcBuf_Id_RAVE_PBP0, 
    BXPT_XcBuf_Id_RAVE_PBP1, 
    BXPT_XcBuf_Id_RAVE_PBP2,
    BXPT_XcBuf_Id_RAVE_PBP3,

    BXPT_XcBuf_Id_MSG_IBP0,  
    BXPT_XcBuf_Id_MSG_IBP1,  
    BXPT_XcBuf_Id_MSG_IBP2,  
    BXPT_XcBuf_Id_MSG_IBP3,  
    BXPT_XcBuf_Id_MSG_IBP4,  
    BXPT_XcBuf_Id_MSG_IBP5,  

    BXPT_XcBuf_Id_MSG_PBP0,  
    BXPT_XcBuf_Id_MSG_PBP1,  
    BXPT_XcBuf_Id_MSG_PBP2,
    BXPT_XcBuf_Id_MSG_PBP3,

#elif ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) 
    BXPT_XcBuf_Id_RMX0_IBP0, 
    BXPT_XcBuf_Id_RMX0_IBP1, 
    BXPT_XcBuf_Id_RMX0_IBP2, 
    BXPT_XcBuf_Id_RMX0_IBP3, 
    BXPT_XcBuf_Id_RMX0_IBP4, 
    BXPT_XcBuf_Id_RMX0_IBP5, 
    BXPT_XcBuf_Id_RMX0_IBP6, 

    BXPT_XcBuf_Id_RMX0_PBP0, 
    BXPT_XcBuf_Id_RMX0_PBP1,

	#if ( BCHP_CHIP == 7420 )  
    BXPT_XcBuf_Id_RMX0_PBP2,
    BXPT_XcBuf_Id_RMX0_PBP3,
    BXPT_XcBuf_Id_RMX0_PBP4,
    BXPT_XcBuf_Id_RMX0_PBP5,
    BXPT_XcBuf_Id_RMX0_PBP6,
    BXPT_XcBuf_Id_RMX0_PBP7,
	#endif

    BXPT_XcBuf_Id_RMX1_IBP0, 
    BXPT_XcBuf_Id_RMX1_IBP1, 
    BXPT_XcBuf_Id_RMX1_IBP2, 
    BXPT_XcBuf_Id_RMX1_IBP3, 
    BXPT_XcBuf_Id_RMX1_IBP4, 
    BXPT_XcBuf_Id_RMX1_IBP5, 
    BXPT_XcBuf_Id_RMX1_IBP6, 

    BXPT_XcBuf_Id_RMX1_PBP0, 
    BXPT_XcBuf_Id_RMX1_PBP1, 
	#if ( BCHP_CHIP == 7420 )  
    BXPT_XcBuf_Id_RMX1_PBP2,
    BXPT_XcBuf_Id_RMX1_PBP3,
    BXPT_XcBuf_Id_RMX1_PBP4,
    BXPT_XcBuf_Id_RMX1_PBP5,
    BXPT_XcBuf_Id_RMX1_PBP6,
    BXPT_XcBuf_Id_RMX1_PBP7,
	#endif

    BXPT_XcBuf_Id_RAVE_IBP0, 
    BXPT_XcBuf_Id_RAVE_IBP1, 
    BXPT_XcBuf_Id_RAVE_IBP2, 
    BXPT_XcBuf_Id_RAVE_IBP3, 
    BXPT_XcBuf_Id_RAVE_IBP4, 
    BXPT_XcBuf_Id_RAVE_IBP5, 
    BXPT_XcBuf_Id_RAVE_IBP6, 

    BXPT_XcBuf_Id_RAVE_PBP0, 
    BXPT_XcBuf_Id_RAVE_PBP1, 
	#if ( BCHP_CHIP == 7420 )  
    BXPT_XcBuf_Id_RAVE_PBP2,
    BXPT_XcBuf_Id_RAVE_PBP3,
    BXPT_XcBuf_Id_RAVE_PBP4,
    BXPT_XcBuf_Id_RAVE_PBP5,
    BXPT_XcBuf_Id_RAVE_PBP6,
    BXPT_XcBuf_Id_RAVE_PBP7,
	#endif

    BXPT_XcBuf_Id_MSG_IBP0,  
    BXPT_XcBuf_Id_MSG_IBP1,  
    BXPT_XcBuf_Id_MSG_IBP2,  
    BXPT_XcBuf_Id_MSG_IBP3,  
    BXPT_XcBuf_Id_MSG_IBP4,  
    BXPT_XcBuf_Id_MSG_IBP5,  
    BXPT_XcBuf_Id_MSG_IBP6,  

    BXPT_XcBuf_Id_MSG_PBP0,  
    BXPT_XcBuf_Id_MSG_PBP1,  
	#if ( BCHP_CHIP == 7420 ) 
    BXPT_XcBuf_Id_MSG_PBP2,
    BXPT_XcBuf_Id_MSG_PBP3,
    BXPT_XcBuf_Id_MSG_PBP4,
    BXPT_XcBuf_Id_MSG_PBP5,
    BXPT_XcBuf_Id_MSG_PBP6,
    BXPT_XcBuf_Id_MSG_PBP7,
	#endif

#elif ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )  || ( BCHP_CHIP == 7342 )
    BXPT_XcBuf_Id_RMX0_IBP0, 
    BXPT_XcBuf_Id_RMX0_IBP1, 
    BXPT_XcBuf_Id_RMX0_IBP2, 
    BXPT_XcBuf_Id_RMX0_IBP3, 
    BXPT_XcBuf_Id_RMX0_IBP4, 
    BXPT_XcBuf_Id_RMX0_IBP5, 
    BXPT_XcBuf_Id_RMX0_IBP6, 

    BXPT_XcBuf_Id_RMX0_PBP0, 
    BXPT_XcBuf_Id_RMX0_PBP1, 
    BXPT_XcBuf_Id_RMX0_PBP2, 
    BXPT_XcBuf_Id_RMX0_PBP3, 
    BXPT_XcBuf_Id_RMX0_PBP4, 
	#if ( BCHP_CHIP == 7342 )
    BXPT_XcBuf_Id_RMX0_PBP5, 
    BXPT_XcBuf_Id_RMX0_PBP6, 
    BXPT_XcBuf_Id_RMX0_PBP7, 
	#endif

    BXPT_XcBuf_Id_RMX1_IBP0, 
    BXPT_XcBuf_Id_RMX1_IBP1, 
    BXPT_XcBuf_Id_RMX1_IBP2, 
    BXPT_XcBuf_Id_RMX1_IBP3, 
    BXPT_XcBuf_Id_RMX1_IBP4, 
    BXPT_XcBuf_Id_RMX1_IBP5, 
    BXPT_XcBuf_Id_RMX1_IBP6, 

    BXPT_XcBuf_Id_RMX1_PBP0, 
    BXPT_XcBuf_Id_RMX1_PBP1, 
    BXPT_XcBuf_Id_RMX1_PBP2, 
    BXPT_XcBuf_Id_RMX1_PBP3, 
    BXPT_XcBuf_Id_RMX1_PBP4, 
	#if ( BCHP_CHIP == 7342 )
    BXPT_XcBuf_Id_RMX1_PBP5, 
    BXPT_XcBuf_Id_RMX1_PBP6, 
    BXPT_XcBuf_Id_RMX1_PBP7, 
	#endif

    BXPT_XcBuf_Id_RAVE_IBP0, 
    BXPT_XcBuf_Id_RAVE_IBP1, 
    BXPT_XcBuf_Id_RAVE_IBP2, 
    BXPT_XcBuf_Id_RAVE_IBP3, 
    BXPT_XcBuf_Id_RAVE_IBP4, 
    BXPT_XcBuf_Id_RAVE_IBP5, 
    BXPT_XcBuf_Id_RAVE_IBP6, 

    BXPT_XcBuf_Id_RAVE_PBP0, 
    BXPT_XcBuf_Id_RAVE_PBP1, 
    BXPT_XcBuf_Id_RAVE_PBP2, 
    BXPT_XcBuf_Id_RAVE_PBP3, 
    BXPT_XcBuf_Id_RAVE_PBP4, 
	#if ( BCHP_CHIP == 7342 )
    BXPT_XcBuf_Id_RAVE_PBP5, 
    BXPT_XcBuf_Id_RAVE_PBP6, 
    BXPT_XcBuf_Id_RAVE_PBP7, 
	#endif
    BXPT_XcBuf_Id_MSG_IBP0,  
    BXPT_XcBuf_Id_MSG_IBP1,  
    BXPT_XcBuf_Id_MSG_IBP2,  
    BXPT_XcBuf_Id_MSG_IBP3,  
    BXPT_XcBuf_Id_MSG_IBP4,  
    BXPT_XcBuf_Id_MSG_IBP5,  
    BXPT_XcBuf_Id_MSG_IBP6,  

    BXPT_XcBuf_Id_MSG_PBP0,  
    BXPT_XcBuf_Id_MSG_PBP1,  
    BXPT_XcBuf_Id_MSG_PBP2,  
    BXPT_XcBuf_Id_MSG_PBP3,  
    BXPT_XcBuf_Id_MSG_PBP4,  
	#if ( BCHP_CHIP == 7342 )
    BXPT_XcBuf_Id_MSG_PBP5,  
    BXPT_XcBuf_Id_MSG_PBP6,  
    BXPT_XcBuf_Id_MSG_PBP7,  
	#endif

#elif ( BCHP_CHIP == 7125)
    BXPT_XcBuf_Id_RMX0_IBP0, 
    BXPT_XcBuf_Id_RMX0_IBP1, 
    BXPT_XcBuf_Id_RMX0_IBP2, 
    BXPT_XcBuf_Id_RMX0_IBP3, 
    BXPT_XcBuf_Id_RMX0_IBP4, 
    BXPT_XcBuf_Id_RMX0_IBP5, 

    BXPT_XcBuf_Id_RMX0_PBP0, 
    BXPT_XcBuf_Id_RMX0_PBP1, 

    BXPT_XcBuf_Id_RMX1_IBP0, 
    BXPT_XcBuf_Id_RMX1_IBP1, 
    BXPT_XcBuf_Id_RMX1_IBP2, 
    BXPT_XcBuf_Id_RMX1_IBP3, 
    BXPT_XcBuf_Id_RMX1_IBP4, 
    BXPT_XcBuf_Id_RMX1_IBP5, 

    BXPT_XcBuf_Id_RMX1_PBP0, 
    BXPT_XcBuf_Id_RMX1_PBP1, 

    BXPT_XcBuf_Id_RAVE_IBP0, 
    BXPT_XcBuf_Id_RAVE_IBP1, 
    BXPT_XcBuf_Id_RAVE_IBP2, 
    BXPT_XcBuf_Id_RAVE_IBP3, 
    BXPT_XcBuf_Id_RAVE_IBP4, 
    BXPT_XcBuf_Id_RAVE_IBP5, 

    BXPT_XcBuf_Id_RAVE_PBP0, 
    BXPT_XcBuf_Id_RAVE_PBP1, 

	BXPT_XcBuf_Id_MSG_IBP0,  
    BXPT_XcBuf_Id_MSG_IBP1,  
    BXPT_XcBuf_Id_MSG_IBP2,  
    BXPT_XcBuf_Id_MSG_IBP3,  
    BXPT_XcBuf_Id_MSG_IBP4,  
    BXPT_XcBuf_Id_MSG_IBP5,  

    BXPT_XcBuf_Id_MSG_PBP0,  
    BXPT_XcBuf_Id_MSG_PBP1,  

#elif ( BCHP_CHIP == 7340)
    BXPT_XcBuf_Id_RMX0_IBP0, 
    BXPT_XcBuf_Id_RMX0_IBP1, 
    BXPT_XcBuf_Id_RMX0_IBP2, 
    BXPT_XcBuf_Id_RMX0_IBP3, 
    BXPT_XcBuf_Id_RMX0_IBP4, 
    BXPT_XcBuf_Id_RMX0_IBP5, 

    BXPT_XcBuf_Id_RMX0_PBP0, 
    BXPT_XcBuf_Id_RMX0_PBP1, 

    BXPT_XcBuf_Id_RMX1_IBP0, 
    BXPT_XcBuf_Id_RMX1_IBP1, 
    BXPT_XcBuf_Id_RMX1_IBP2, 
    BXPT_XcBuf_Id_RMX1_IBP3, 
    BXPT_XcBuf_Id_RMX1_IBP4, 
    BXPT_XcBuf_Id_RMX1_IBP5, 

    BXPT_XcBuf_Id_RMX1_PBP0, 
    BXPT_XcBuf_Id_RMX1_PBP1, 

    BXPT_XcBuf_Id_RAVE_IBP0, 
    BXPT_XcBuf_Id_RAVE_IBP1, 
    BXPT_XcBuf_Id_RAVE_IBP2, 
    BXPT_XcBuf_Id_RAVE_IBP3, 
    BXPT_XcBuf_Id_RAVE_IBP4, 
    BXPT_XcBuf_Id_RAVE_IBP5, 

    BXPT_XcBuf_Id_RAVE_PBP0, 
    BXPT_XcBuf_Id_RAVE_PBP1, 

	BXPT_XcBuf_Id_MSG_IBP0,  
    BXPT_XcBuf_Id_MSG_IBP1,  
    BXPT_XcBuf_Id_MSG_IBP2,  
    BXPT_XcBuf_Id_MSG_IBP3,  
    BXPT_XcBuf_Id_MSG_IBP4,  
    BXPT_XcBuf_Id_MSG_IBP5,  

    BXPT_XcBuf_Id_MSG_PBP0,  
    BXPT_XcBuf_Id_MSG_PBP1,  
#else
    #if ( BCHP_CHIP != 7440 )
        BXPT_XcBuf_Id_RMX0_A,       /* Remux 0, input A */   
        BXPT_XcBuf_Id_RMX0_B,       /* Remux 0, input B */
    #endif

    #if ( BCHP_CHIP == 3563 )|| ( BCHP_CHIP == 7440 )
        /* These chips have only 1 remux. */
    #else
        BXPT_XcBuf_Id_RMX1_A,       /* Remux 1, input A */
        BXPT_XcBuf_Id_RMX1_B,       /* Remux 1, input B */
    #endif

        BXPT_XcBuf_Id_RAVE_IBP0,    /* RAVE input band parser 0 */
        BXPT_XcBuf_Id_RAVE_IBP1,    /* RAVE input band parser 1 */
        BXPT_XcBuf_Id_RAVE_IBP2,    /* RAVE input band parser 2 */
        BXPT_XcBuf_Id_RAVE_IBP3,    /* RAVE input band parser 3 */
        BXPT_XcBuf_Id_RAVE_IBP4,    /* RAVE input band parser 4 */

    #if ( BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 3563 )
        /* 7118 does not have IB5 or 6 */
    #else
        BXPT_XcBuf_Id_RAVE_IBP5,    /* RAVE input band parser 5 */
        BXPT_XcBuf_Id_RAVE_IBP6,    /* RAVE input band parser 6 */
    #endif

    #if ( BCHP_CHIP == 3563 )
        /* 3563 has only PB0 */
        BXPT_XcBuf_Id_RAVE_PBP0,    /* RAVE playback band parser 0 */
    #else
        BXPT_XcBuf_Id_RAVE_PBP0,    /* RAVE playback band parser 0 */
        BXPT_XcBuf_Id_RAVE_PBP1,    /* RAVE playback band parser 1 */
    #endif

    #if ( BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 3563 )
        /* Not in either of these chips */
    #else
        BXPT_XcBuf_Id_RAVE_PBP2,    /* RAVE playback band parser 2 */
        #if ( BCHP_CHIP != 7440 )
        BXPT_XcBuf_Id_RAVE_PBP3,    /* RAVE playback band parser 3 */
        #endif
    #endif

    #if ( BCHP_CHIP == 7400 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7405 )
        BXPT_XcBuf_Id_RAVE_PBP4,    /* RAVE playback band parser 4 */
    #endif

        BXPT_XcBuf_Id_MSG_IBP0,     /* Mesg filters input band parser 0 */
        BXPT_XcBuf_Id_MSG_IBP1,     /* Mesg filters input band parser 1 */
        BXPT_XcBuf_Id_MSG_IBP2,     /* Mesg filters input band parser 2 */
        BXPT_XcBuf_Id_MSG_IBP3,     /* Mesg filters input band parser 3 */
        BXPT_XcBuf_Id_MSG_IBP4,     /* Mesg filters input band parser 4 */

    #if ( BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 3563 )
        /* 7118 doesn't have either IB5 or 6 */
    #else
        BXPT_XcBuf_Id_MSG_IBP5,     /* Mesg filters input band parser 5 */
        BXPT_XcBuf_Id_MSG_IBP6,     /* Mesg filters input band parser 6 */
    #endif

    #if ( BCHP_CHIP == 3563 )
        /* 3563 has only PB0 */
        BXPT_XcBuf_Id_MSG_PBP0,     /* Mesg filters playback band parser 0 */
    #else
        BXPT_XcBuf_Id_MSG_PBP0,     /* Mesg filters playback band parser 0 */
        BXPT_XcBuf_Id_MSG_PBP1,     /* Mesg filters playback band parser 1 */
    #endif

    #if ( BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 3563 )
        /* Not in either of these chips */
    #else
        BXPT_XcBuf_Id_MSG_PBP2,     /* Mesg filters playback band parser 2 */
        #if ( BCHP_CHIP != 7440 )
        BXPT_XcBuf_Id_MSG_PBP3,     /* Mesg filters playback band parser 3 */
        #endif
    #endif

    #if ( BCHP_CHIP == 7400 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7405 )
        BXPT_XcBuf_Id_MSG_PBP4,     /* Mesg filters playback band parser 4 */
    #endif
#endif
    BXPT_XcBuf_Id_END           /* Marks end of the list. */
}
BXPT_XcBuf_Id;

/***************************************************************************
Summary:
Set the max data rate for a given client.

Description:
Set the max average data rate for the given client. This is the max rate that
can be read on the transport data pipe. A default value of 25 Mbps will be
set when BXPT_Open() is called.

Returns:
    BERR_SUCCESS                - New rate has been set.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_Open, BXPT_XcBuf_SetBufferSize                     
****************************************************************************/
BERR_Code BXPT_XcBuf_SetBandDataRate(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    BXPT_XcBuf_Id Id,           /* [in] Which client buffer we are dealing with */
    unsigned long Rate,         /* [in] Max rate in Mbps */
    unsigned PacketLen          /* [in] size of mpeg packet */
    );

/***************************************************************************
Summary:
Set the buffer size for the given client.

Description:
Using the given size, allocate memory for the client's DRAM buffer. A default
size of 200kB will set when BXPT_Open() is called. Successive calls to this 
function will delete the old buffer before allocating new memory. The old
buffer will also be freed during BXPT_Close(). 

Returns:
    BERR_SUCCESS                - New buffer size has been set.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:                   
BXPT_Open, BXPT_XcBuf_SetBandDataRate, BXPT_Close                   
****************************************************************************/
BERR_Code BXPT_XcBuf_SetBufferSize(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    BXPT_XcBuf_Id Id,           /* [in] Which client buffer we are dealing with */
    unsigned long Size          /* [in] Size in bytes. Must be a multiple of 256. */
    );

/*
** These functions are called internally from BXPT_Open() and BXPT_Close(). 
** Users should NOT uses these functions directly.
*/

BERR_Code BXPT_P_XcBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_DramBufferCfg *Cfg
    );

BERR_Code BXPT_P_XcBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    );

BERR_Code BXPT_P_XcBuf_Pause(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    BXPT_XcBuf_Id Id,           /* [in] Which client buffer we are dealing with */
    bool Enable
    );

bool BXPT_P_XcBuf_IsBufferEnabled( 
    BXPT_Handle hXpt, 
    BXPT_XcBuf_Id Id
    );

BERR_Code BXPT_P_XcBuf_RemuxPause(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned RemuxNum,          /* [in] Which client buffer we are dealing with */
    bool Enable
    );

unsigned long BXPT_P_XcBuf_GetBlockout(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    BXPT_XcBuf_Id Id           /* [in] size of mpeg packet */
    );

BERR_Code BXPT_P_XcBuf_SetBlockout(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    BXPT_XcBuf_Id Id,           /* [in] Which client buffer we are dealing with */
    unsigned long NewB0        
    );

uint32_t BXPT_P_XcBuf_ComputeBlockOut( 
	unsigned long PeakRate,			/* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    );

#endif	/*  BXPT_HAS_FIXED_XCBUF_CONFIG */

#ifdef __cplusplus
}
#endif

#endif /* BXPT_XCBUFF_H__ */


