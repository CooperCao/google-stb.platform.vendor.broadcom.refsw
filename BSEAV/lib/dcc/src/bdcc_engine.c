/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/






/***************************************************************************
 *                         708 Rendering Engine
 *                         --------------------
 *
 * This Rendering Engine API is a wrapper around several lower-level 708
 * libraries.  In most cases, it is expected that this API will be used
 * instead of the lower-level APIs.  In those cases in which the customer/
 * system integrator has a need that isn't covered here, this code then
 * serves as example code.
 *
 * Scope
 *
 * This API wraps the 708 DTVCC rendering and 608 Transcoding libraries
 * but does not wrap the APIs used to extract Closed Captioning streams
 * from MPEG User Data or from analog Line 21.  Furthermore, it does not
 * wrap the API for inserting 608 back into Line 21 on analog output.
 *
 * 608 and 708
 *
 * The Rendering Engine has two main entry points, one each for 708 rendering
 * and 608 transcoding.  Higher level system/driver code is expected to call the
 * appropriate API to extract the Closed Captioning data from the input, be it
 * the MPEG API (ie., bcmMPIReadClosedCaptionStatus) or the CCDecoder API
 * (bCCDProcess), and then call one of these two entry points in this API:
 * B_Dcc_Process608 or DccEngine_Process708.
 *
 * This API has these features:
 *
 * 1.  manages the circular buffers
 * 2.  sequences the calls to the lower-level APIs
 * 3.  allows caller control over field and service numbers
 * 4.  allows caller ability to override various DTVCC attributes
 * 5.  provides reset
 * 6.  context-less:  all processing done on _ProcessXxx and _Periodic calls
 *
 ***************************************************************************/


				/*********************
				 *
				 * Includes
				 *
				 *********************/

#include "b_api_shim.h"
#include <assert.h>
#include "bdcc_cfg.h"
#include "bdcc.h"
#include "bdcc_cbuf.h"
#include "bdcc_coding.h"
#include "bdcc_packet.h"
#include "bdcc_608transcoder.h"

#include "b_dcc_lib.h"
#include "bdcc_service.h"
#include "bdcc_intgfx.h"

BDBG_MODULE(BDCCENGINE);

				/*********************
				 *
				 * Defines
				 *
				 *********************/
#define B_Dcc_P_ASPECT_4_3_COLUMN_SIZE	32
#define B_Dcc_P_ASPECT_16_9_COLUMN_SIZE	42
#define B_Dcc_P_DEF_CHARCELL_WIDTH		15
#define B_Dcc_P_DEF_CHARCELL_HEIGHT		25

#define B_Dcc_P_CBUF_SIZE				512

#define B_Dcc_P_CBUF_RESERVE			32
#define B_Dcc_P_SILIMIT_608				(B_Dcc_P_CBUF_SIZE - B_Dcc_P_CBUF_RESERVE)
/*
** B_Dcc_P_SILIMIT_708 was originally defined to be 128.
** I don't see any reason that this can't be the entire buffer minus "a bit".  7/7/06
*/
#define B_Dcc_P_SILIMIT_708				(B_Dcc_P_CBUF_SIZE - (2 * B_Dcc_P_CBUF_RESERVE))

/*
** How much the CC window needs to be indented to insure
** that it is visible.
** These really should be set by the application based on the
** screen resolution and display type.
*/
#define B_Dcc_P_SafeTitle_X    48 /* Set for 480i/p display */
#define B_Dcc_P_SafeTitle_Y    24 /* Set for 480i/p display */


				/*********************
				 *
				 * Types
				 *
				 *********************/
typedef struct B_Dcc_P_Object
{
	/*
	 * Circular Buffer:  Triplets
	 *
	 * This buffer holds either 608 or 708 byte
	 * triplets:  (field,cc1,cc2) or (cc_type,cc1,cc2)
	 */
	BDCC_CBUF		cbTriplets ;
	unsigned char	BufTriplets[B_Dcc_P_CBUF_SIZE*3] ;

	/*
	 * Circular Buffer:  Packet
	 *
	 * This buffer holds 708 packets, used only for
	 * DTVCC processing, not for 608 transcoding
	 */
	BDCC_CBUF		cbPacket ;
	unsigned char	BufPacket[B_Dcc_P_CBUF_SIZE] ;

	/*
	 * Circular Buffer:  Service
	 *
	 * This buffer holds 708 service blocks.
	 */
	BDCC_CBUF		cbService ;
	unsigned char	BufService[B_Dcc_P_CBUF_SIZE] ;

	/*
	 * Circular Buffer:  Coding
	 *
	 * This buffer feeds the coding and interpretation
	 * library.  This also serves as the Service Input
	 * Buffer, per the 708 spec.
	 */
	BDCC_CBUF		cbCoding ;
	unsigned char	BufCoding[B_Dcc_P_CBUF_SIZE] ;

	/*
	 * associated object data
	 */
	BDCC_608_hTranscoder	h608Transcoder ;
	BDCC_INT_P_Handle		hCodingInt ;
	BDCC_PKT_P_Object		PacketObject ;
 	/* The CCGFX Winlib module handle */
	BDCC_WINLIB_Handle	hWinLibHandle ;
 	/* The CCGFX module handle */
	BCCGFX_P_GfxHandle		hCCGfxHandle ;

	/*
	 * object attributes
	 */
	B_Dcc_Type	Type ;
	int				iCcService ;

	B_Dcc_Settings engineSettings;
	unsigned int	SILimit ;

	unsigned int    uiCurTimeMilliSecs;     /* roughtly the current system                                      */
	unsigned int    uiLastDataMilliSecs;    /* the last time CC data was received                           */
	bool                bDataReceived;          /* data has been received since the last screen clear   */

} B_Dcc_P_Object ;




				/*********************
				 *
				 * Prototypes
				 *
				 *********************/

BDCC_Error CheckForBufferOverflow(B_Dcc_Handle hEngine) ;
void ResetDownstreamOfPacket(B_Dcc_Handle hEngine);

static const B_Dcc_Settings sDefaultEngSettings = {
	B_Dcc_P_ASPECT_16_9_COLUMN_SIZE,
	B_Dcc_P_SafeTitle_X,
	B_Dcc_P_SafeTitle_Y,
	100,
	{
		NULL,NULL,NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,
		NULL
	},
	BDCC_Data_Timeout_MSecs,
};

void printCCserv(B_Dcc_Handle hEngine, int num)
{
	BDBG_ERR(("%d - %d", num, hEngine->iCcService));
}

BDCC_Error B_Dcc_P_Init(
	bool bResetOnly,
	B_Dcc_Handle hEngine,
	B_Dcc_Type Type,
	BDCC_WINLIB_Handle hWinLibHandle,
	int iCcService
	)
{
    unsigned int SILimit ;
    BDCC_608_hTranscoder h608Transcoder = NULL  ;
    BCCGFX_P_GfxHandle hCCGfxHandle = NULL ;
    BDCC_INT_P_Handle hCodingInt = NULL ;

    /*
    ** save the current parameters
    */
    hEngine->hWinLibHandle = hWinLibHandle ;
    hEngine->Type = Type ;
    hEngine->iCcService = iCcService ;
    hEngine->bDataReceived = false;

    /*
    ** TODO: is the following code block really needed?
    */
    if( bResetOnly )
    {
        h608Transcoder  = hEngine->h608Transcoder ;
        hCCGfxHandle = hEngine->hCCGfxHandle ;
        hCodingInt = hEngine->hCodingInt ;
    }

    /*
    ** init the circular buffers
    */
    BDCC_CBUF_Init(&hEngine->cbTriplets, hEngine->BufTriplets, sizeof(hEngine->BufTriplets), B_Dcc_P_CBUF_RESERVE) ;
    BDCC_CBUF_Init(&hEngine->cbPacket, hEngine->BufPacket, sizeof(hEngine->BufPacket), B_Dcc_P_CBUF_RESERVE) ;
    BDCC_CBUF_Init(&hEngine->cbService, hEngine->BufService, sizeof(hEngine->BufService), B_Dcc_P_CBUF_RESERVE) ;
    BDCC_CBUF_Init(&hEngine->cbCoding, hEngine->BufCoding, sizeof(hEngine->BufCoding), B_Dcc_P_CBUF_RESERVE) ;

    /*
    ** if we're doing 608 transcoding init its object structure
    */
    if ( Type == B_Dcc_Type_e608 )
    {
        int Field608, Chan608 ;
        switch ( iCcService )
        {
        case 1:
            Field608 = 1 ;
            Chan608 = 1 ;
            break ;
        case 2:
            Field608 = 1 ;
            Chan608 = 2 ;
            break ;
        case 3:
            Field608 = 2 ;
            Chan608 = 1 ;
            break ;
        case 4:
            Field608 = 2 ;
            Chan608 = 2 ;
            break ;
        default:
            return(BDCC_Error_eArgOutOfRange) ;
            break ;
        }

        /*
        ** If the library is being reset and the transcode engine has previously been
        ** opened, simply reset the transcode logic.  Otherwise allocate it.
        **
        ** TODO: can't we simply look at  "hEngine->h608Transcoder" to determine if we
        ** need to reset or allocate?
        */
        if ( bResetOnly && h608Transcoder )
        {
            hEngine->h608Transcoder = h608Transcoder ;
            BDCC_608_TranscodeReset(hEngine->h608Transcoder, Field608, Chan608);
        }
        else
        {
            BDCC_608_TranscodeOpen(&hEngine->h608Transcoder, Field608, Chan608) ;
        }

        SILimit = B_Dcc_P_SILIMIT_608 ;

        /* Flush out the 608 activity indicator - used to timeout captions that have
           been left dangling */
        BDCC_608_GetNewActivity(hEngine->h608Transcoder); /* resets activity indicator */
    }
    else if ( Type == B_Dcc_Type_e708 )
    {
        SILimit = B_Dcc_P_SILIMIT_708 ;
    }
    else
    {
        BKNI_Free(hEngine);
        return(BDCC_Error_eArgOutOfRange) ;
    }

    /*
    * * init the pkt, coding and interpretation objects
    ** TODO: wrap in "bResetOnly"?
    */
    BDCC_PKT_P_Init(&hEngine->PacketObject) ;
    hEngine->SILimit = SILimit ;

    if(bResetOnly)
    {
        hEngine->hCCGfxHandle = hCCGfxHandle ;
        hEngine->hCodingInt = hCodingInt ;
        /*
        ** TODO: are the following in the correct order?
        */
        BDCC_Coding_P_Reset(hEngine->hCodingInt, hEngine->Type) ;
	    BCCGFX_P_Reset( hEngine->hCCGfxHandle, &hEngine->engineSettings );
    }
    else
    {
        /*
        ** init the ccgfx layer
        */

        BCCGFX_P_Init( hEngine->hCCGfxHandle, &hEngine->engineSettings );

        BDCC_Coding_P_Open(
                   &hEngine->hCodingInt,
                    hEngine->hWinLibHandle,
                    hEngine->hCCGfxHandle,
                    hEngine->SILimit,
                    hEngine->engineSettings.Columns,
                    hEngine->Type
                    );
    }

    /*
    ** Resample the system clock.
    */
    BCCGFX_P_TimeReset( hEngine->hCCGfxHandle );

    return(BDCC_Error_eSuccess) ;

}

				/*********************
				 *
				 * API Entry Points
				 *
				 *********************/

/**************************************************************************
 *
 * Function:		B_Dcc_GetDefaultSettings
 *
 * Inputs:
 *					Type				- B_Dcc_Type_e608 or
 *                                                                      B_Dcc_Type_e708
 *					pEngineSettings		- B_Dcc_Settings structure
 *
 * Outputs:
 *					hEngine				- init'ed by this function
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function return the default and recommended values for various engine settings.
 *
 * See Also:
 *    B_Dcc_Open
 *
 **************************************************************************/
BDCC_Error B_Dcc_GetDefaultSettings(
	B_Dcc_Settings *pEngineSettings)
{
	assert(pEngineSettings);

	*pEngineSettings = sDefaultEngSettings;

	return BDCC_Error_eSuccess ;
}

BDCC_Error B_Dcc_GetSettings(
    	B_Dcc_Handle hEngine,
    	B_Dcc_Settings *pEngineSettings
    	)
{
	assert(hEngine);
	assert(pEngineSettings);

	*pEngineSettings = hEngine->engineSettings;

	return BDCC_Error_eSuccess ;
}


BDCC_Error B_Dcc_SetSettings(
    	B_Dcc_Handle hEngine,
    	B_Dcc_Settings *pEngineSettings
    	)
{
	assert(hEngine);
	assert(pEngineSettings);

	hEngine->engineSettings = *pEngineSettings;

	return BDCC_Error_eSuccess ;
}


/**************************************************************************
 *
 * Function:		B_Dcc_Open
 *
 * Inputs:
 *					Type				- B_Dcc_Type_e608 or B_Dcc_Type_e708
 *					iCcService				- CCx for 608 (1 to 4)
 *										  Service Number for 708 (0 to 63)
 *					Columns				- number of columns for DTVCC grid
 *										  use 32 for 4:3 and 42 for 16:9
 *
 * Outputs:
 *					hEngine				- init'ed by this function
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function inits the buffers used for 708 DTVCC Closed
 * Captioning processing.  The 'Type' argument, in effect, is announcing
 * which processing entry point will be called during normal processing.
 * To switch between 608 and 708 (or to switch any of the init args),
 * B_Dcc_Close and DccEngine_Init must be called again -- or
 * alternatively, DccEngine_Reset.
 *
 * See Also:
 *    B_Dcc_Close
 *    DccEngine_Reset
 *
 **************************************************************************/
BDCC_Error B_Dcc_Open(
	B_Dcc_Handle *phEngine,
	BDCC_WINLIB_Handle hWinLibHandle,
	BDCC_WINLIB_Interface *pWinLibInterface
	)
{
    int iError;

    B_Dcc_Handle hEngine;

    assert(hWinLibHandle);

    *phEngine = hEngine = (B_Dcc_Handle)BKNI_Malloc( sizeof(B_Dcc_P_Object));

    if( hEngine == NULL )
    {
        return BDCC_Error_eNoMemory;
    }

    BKNI_Memset(hEngine, 0, sizeof(B_Dcc_P_Object)) ;

    hEngine->hWinLibHandle = hWinLibHandle ;

    iError = BCCGFX_P_Open(&hEngine->hCCGfxHandle, hEngine->hWinLibHandle, pWinLibInterface);

    return iError;

} /* B_Dcc_Open */


BDCC_Error B_Dcc_Init(
	B_Dcc_Handle hEngine,
	int iCcService,
	B_Dcc_Type Type
	)
{
	return B_Dcc_P_Init(
			0,
			hEngine,
			Type,
			hEngine->hWinLibHandle,
			iCcService
			);

} /* B_Dcc_Init */


/**************************************************************************
 *
 * Function:		B_Dcc_Close
 *
 * Inputs:
 *					hEngine				- init'ed previously by DccEngine_Init
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function undoes the initialization of DccEngine_Init().
 *
 * See Also:
 *    DccEngine_Reset
 *    DccEngine_Init
 *
 **************************************************************************/
BDCC_Error B_Dcc_Close(B_Dcc_Handle hEngine)
{
	assert(hEngine);
	if ( (hEngine->Type == B_Dcc_Type_e608) && (hEngine->h608Transcoder) )
	{
		BDCC_608_TranscodeClose(hEngine->h608Transcoder);
	}
	if (hEngine->hCodingInt)
		BDCC_Coding_P_Close(hEngine->hCodingInt);
	BCCGFX_P_Close(hEngine->hCCGfxHandle) ;
	BKNI_Free(hEngine);

	return(BDCC_Error_eSuccess) ;

} /* B_Dcc_Close */


/**************************************************************************
 *
 * Function:		B_Dcc_Reset
 *
 * Inputs:
 *					hEngine				- init'ed previously by B_Dcc_Open
 *					iCcService				- CCx for 608 (1 to 4)
 *										  Service Number for 708 (0 to 63)
 *					Type				- 608, 708 or "NoChange"
 *					Columns				- number of columns for DTVCC grid
 *										  use 32 for 4:3 and 42 for 16:9
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * B_Dcc_Reset is logically equivalent to the sequence B_Dcc_Close()
 * followed by B_Dcc_Open().
 *
 * Note:  This function is provided in case some advantage can be gained by
 * consolidating _Fini and _Init.  For example, memory need not be free and
 * re-aquired which has an effect on memory fragmentation.  Treat this function
 * as a soft reset and the sequence _Fini and _Init as a hard reset.
 *
 * If Type is B_Dcc_Type_NoChange, then the reset is performed using the current
 * parameters and Type, iCcService, Columns are ignored.
 *
 * See Also:
 *    B_Dcc_Close
 *    B_Dcc_Open
 *
 **************************************************************************/
BDCC_Error B_Dcc_Reset(
	B_Dcc_Handle hEngine,
	bool bNoChange,
	B_Dcc_Type ccMode,
	int ccService
	)
{
    BDCC_Error bdccErr;
    unsigned int OverrideMask ;
    B_Dcc_OverRides Overrides ;

    BSTD_UNUSED( bNoChange );

    /*
    ** Save the overrides.
    */
    OverrideMask = hEngine->hCodingInt->OverrideMask ;
    Overrides = hEngine->hCodingInt->Overrides ;

    /*
    ** For now, only allow the CC service and mode to be reapplied.
    */
    bdccErr = B_Dcc_P_Init(
                        true,                                   /* bResetOnly */
                        hEngine,
                        ccMode,                             /* 608 or 708 */
                        hEngine->hWinLibHandle,
                        ccService
                        );
    /*
    ** re-apply the overrides
    */
    if (bdccErr == BDCC_Error_eSuccess)
	{
		bdccErr = B_Dcc_Override(hEngine, OverrideMask, &Overrides) ;

	}

    return( bdccErr );

}

/**************************************************************************
 *
 * Function:		B_Dcc_Process
 *
 * Inputs:
 *					hEngine			- init'ed previously by DccEngine_Init
 *					pTriplets			- ptr to buf of triplets (field,cc1,cc2)
 *					NumTriplets		- count = num_bytes / 3
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * Primarily a wrapper routine around the other BDCC functions.  Provides a single
 * entry point to simplify the interface for the application.
 *
 * If this function returns BDCC_Error_eBufferOverflow, it is expected that the
 * caller will call again to DccEngine_Reset.
 *
 **************************************************************************/
BDCC_Error B_Dcc_Process(
    B_Dcc_Handle hEngine,
    unsigned char * pTriplets,
    int NumTriplets )
{
    BDCC_Error bdccErr = BDCC_Error_eSuccess;

    bool bTimedOut;

    /*
    ** TODO: similar checks are performed in B_Dcc_Process708()
    ** and B_Dcc_Process608.  If this routine becomes the only entry point,
    ** remove those checks.
    */

    if ( NULL == hEngine  || NULL == pTriplets )
    {
        return( BDCC_Error_eNullPointer ) ;
    }

    /*
    ** Bump the clock and drive the time dependent events.
    ** ( flashing, scrolling, erasing windows, DELAY, FADE and WIPE )
    */

    B_Dcc_Periodic( hEngine );

    /*
    ** If data is availabe, call the appropriate processing rouinte.
    */

    if ( NumTriplets > 0 )
    {
        /*
        ** Based on the CC mode, direct the data to the appropriate pipe.
        */
        if ( B_Dcc_Type_e708 == hEngine->Type )
        {
            /*
            ** TODO: should we validate the data being passed by the application?
            */
            bdccErr = B_Dcc_Process708( hEngine, pTriplets, NumTriplets );

        }
        else if ( B_Dcc_Type_e608 == hEngine->Type )
        {
            /*
            ** TODO: should we validate the data being passed by the application?
            */
            bdccErr = B_Dcc_Process608( hEngine, pTriplets, NumTriplets );

        }
        else
        {
            return( BDCC_Error_eBadOutputType ) ;
        }

    }
	else {
		/* handle DELAY command case */
		if ((B_Dcc_Type_e708  == hEngine->Type)) {
            bdccErr = B_Dcc_Process708( hEngine, pTriplets, NumTriplets );
		}
	}

    /*
    ** Clear the screen if:
    ** - "engineSettings.uiTimeOutMilliSecs" milliseconds have passed since CC data was received
    ** - data has been received after the last time the screen was cleared
    */

    bTimedOut = ( hEngine->uiCurTimeMilliSecs - hEngine->uiLastDataMilliSecs > hEngine->engineSettings.uiTimeOutMilliSecs  );

    if ( bTimedOut && hEngine->bDataReceived  )
    {
        BDCC_Coding_P_ScreenClear( hEngine->hCodingInt );
        hEngine->bDataReceived = false;
    }

    return( bdccErr );

} /* B_Dcc_Process608 */
/**************************************************************************
 *
 * Function:		B_Dcc_Process608
 *
 * Inputs:
 *					hEngine				- init'ed previously by DccEngine_Init
 *					pTriplets			- ptr to buf of triplets (field,cc1,cc2)
 *					NumTriplets			- count = num_bytes / 3
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function sends the CC pairs matching the supplied Field through
 * the required processing for rendering.  This includes the 608 Transcoder.
 *
 * If this function returns BDCC_Error_eBufferOverflow, it is expected that the
 * caller will call again to DccEngine_Reset.
 *
 **************************************************************************/
BDCC_Error B_Dcc_Process608(
	B_Dcc_Handle hEngine,
	unsigned char * pTriplets,
	int NumTriplets)
{
	BDCC_Error err ;

	/*
	 * first do some validation
	 */
	if ( NULL == hEngine   ||   pTriplets == NULL )
	{
		return(BDCC_Error_eNullPointer) ;
	}

	if ( NumTriplets < 0 )
	{
		return(BDCC_Error_eArgOutOfRange) ;
	}

	if ( hEngine->Type != B_Dcc_Type_e608 )
	{
		return(BDCC_Error_eBadOutputType) ;
	}

	/*
	 * do one pass
	 */
	BDCC_CBUF_WritePtr(&hEngine->cbTriplets, pTriplets, NumTriplets * 3) ;
	BDCC_608_TranscodeProcess(hEngine->h608Transcoder,&hEngine->cbTriplets, &hEngine->cbService) ;
	BDCC_SIBuf_P_Process(hEngine->hCodingInt, &hEngine->cbService, &hEngine->cbCoding) ;
	BDCC_Coding_P_Process(hEngine->hCodingInt, &hEngine->cbCoding) ;

	/*
    ** Keep track of when caption data for the selected caption channel was last received.
    ** If theres a "timeout" (i.e. captions have been left dangling), we'll need to clear the screen.
    */
    if(BDCC_608_GetNewActivity(hEngine->h608Transcoder))
    {
        hEngine->uiLastDataMilliSecs = hEngine->uiCurTimeMilliSecs;
        hEngine->bDataReceived = true;
    }

	/*
	 * Now check for buffer overflow.
	 * This should never happen -- if it does
	 * either it will happen very often because
	 * the buffer sizes are not provisioned correctly
	 * or we have an errant stream.  In the first case
	 * the B_Dcc_P_CBUF_SIZE and B_Dcc_P_CBUF_RESERVE need to be
	 * corrected.  In the second case, it is appropriate to
	 * reset.  The caller of this function is expected to
	 * call again to DccEngine_Reset if this function returns
	 * BDCC_Error_eBufferOverflow.
	 */
	err = CheckForBufferOverflow(hEngine) ;

	return(err) ;

} /* B_Dcc_Process608 */


BDCC_Error WriteTripletsToCBuf(BDCC_CBUF * pCBuf, unsigned char ** ppTriplets, unsigned int * pNumTriplets)
{
	int TripletsThisTime = min(*pNumTriplets,pCBuf->FreeBytes/3) ;
	int TripletsLeftover = *pNumTriplets - TripletsThisTime ;

	BDCC_CBUF_WritePtr(pCBuf, *ppTriplets, TripletsThisTime * 3) ;
	*ppTriplets += TripletsThisTime * 3 ;
	*pNumTriplets -= TripletsThisTime ;

	BDBG_MSG(("WrTToCBuf:  Trip %d  LO %d\n", TripletsThisTime, TripletsLeftover)) ;

	if ( TripletsLeftover )
		return(BDCC_Error_eWrnPause) ;
	else
		return(BDCC_Error_eSuccess) ;
}


/**************************************************************************
 *
 * Function:		B_Dcc_Process708
 *
 * Inputs:
 *					hEngine				- init'ed previously by DccEngine_Init
 *					pTriplets			- ptr to buf of triplets (cc_type,cc1,cc2)
 *					NumTriplets			- count = num_bytes / 3
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function does the packet/service/coding/interpretation layers
 * of the 708 DTVCC spec.  It filters on the Service Number provided in the
 * previous _Init or _Reset call, as the iCcService argument.
 *
 * If this function returns BDCC_Error_eBufferOverflow, it is expected that the
 * caller will call again to DccEngine_Reset.
 *
 **************************************************************************/
#define MAX_ITER_CODING				20
BDCC_Error B_Dcc_Process708(
	B_Dcc_Handle hEngine,
	unsigned char * pTriplets,
	int NumTriplets)
{
	BDCC_Error err ;
	BDCC_Error rcA, rcB, rcC, rcD ;
	int InnerTimes = 0 ;
    bool newActivity;
	/*
	 * first do some validation
	 */
	if ( NULL == hEngine   ||   pTriplets == NULL )
	{
		return(BDCC_Error_eNullPointer) ;
	}

	if ( NumTriplets < 0 )
	{
		return(BDCC_Error_eArgOutOfRange) ;
	}
	else {
		/* handle DELAY command case */
		if (0 == NumTriplets) {
			BDCC_Update_Delay(hEngine->hCodingInt, &hEngine->cbCoding);
		}
	}

	if ( hEngine->Type != B_Dcc_Type_e708 )
	{
		return(BDCC_Error_eBadOutputType) ;
	}

	/*
	 * do one pass
	 *
	 * The idea here is to push the input triplets all the way
	 * through the processing pipe.
	 *
	 * The obvious approach is to call each of the processing handlers
	 * once in succession, each completely consuming its input before
	 * returning.  This works fine in most of the cases, but fails for
	 * 'bursty' streams.
	 *
	 * The approach used here degenerates down to the simple approach for
	 * the normal streams (so there's really no processing overhead for
	 * most cases), but allows a processing handler to declare that its not
	 * done with its input and should be called back after its downstream
	 * handlers have gotten a chance to consume their inputs.
	 *
	 * The basic pattern used here is ...
	 *     do
	 *     {
	 *         rcA = DccAAA_Process() ;
	 *         ... other code ...
	 *     } while ( rcA == BDCC_Error_eWrnPause ) ;
	 *
	 * Stamping this pattern several times, once for each processing
	 * level, yields...
	 *     do
	 *     {
	 *         rcA = DccAAA_Process() ;
	 *         do
	 *         {
	 *             rcB = DccBBB_Process() ;
	 *             do
	 *             {
	 *                 rcC = DccCCC_Process() ;
	 *             } while ( rcC == BDCC_Error_eWrnPause ) ;
	 *         } while ( rcB == BDCC_Error_eWrnPause ) ;
	 *     } while ( rcA == BDCC_Error_eWrnPause ) ;
	 *
	 * The code below is essentially this algorithm, with an additional
	 * sanity check that we don't get stuck forever.
	 */
	do
	{
		rcA = WriteTripletsToCBuf(&hEngine->cbTriplets, &pTriplets, (unsigned int *)&NumTriplets) ;
		do
		{
			rcB = BDCC_PKT_P_Process(&hEngine->PacketObject, &hEngine->cbTriplets, &hEngine->cbPacket) ;
			if ( rcB == BDCC_Error_eSequence )
			{
				BDBG_MSG(("%s: Error Sequence Resetting Down Stream", BSTD_FUNCTION));
				ResetDownstreamOfPacket(hEngine) ;
			}
			do
			{
				rcC = BDCC_SRV_P_Process(&hEngine->cbPacket, hEngine->iCcService, BDCC_SRV_SERVICE_ILLEGAL, &hEngine->cbService, NULL, &newActivity) ;

                /*
                ** Keep track of when caption data for the selected caption channel was last received.
                ** If a "timeout" (i.e. captions have been left dangling), we'll need to clear the screen.
                */
                if(newActivity)
                {
                    hEngine->uiLastDataMilliSecs = hEngine->uiCurTimeMilliSecs;
                    hEngine->bDataReceived = true;
                }

				do
				{
					rcD = BDCC_SIBuf_P_Process(hEngine->hCodingInt, &hEngine->cbService, &hEngine->cbCoding) ;
#if 1
					BDCC_Coding_P_Process(hEngine->hCodingInt, &hEngine->cbCoding) ;
#else
					/* consume without really doing anything, useful for debug */
					hEngine->hCodingInt.SIBuf_NumCmdsWritten = 0 ;
					hEngine->hCodingInt.SIBuf_StreamBytesWritten = 0 ;
					BDCC_CBUF_Clear(&hEngine->cbCoding) ;
#endif
					if ( (++InnerTimes) > MAX_ITER_CODING )
					{
						BDBG_WRN(("DccEngine_Process708:  max'ed out on iterations: %d\n", InnerTimes)) ;
						/*
						 * I know, GOTOs are bad, but this seems
						 * cleaner than adding extra conditions in
						 * each of the while clauses below to do a
						 * multi-level break.
						 */
						goto StopProcessing708 ;
					}
				} while ( rcD == BDCC_Error_eWrnPause ) ;
			} while ( rcC == BDCC_Error_eWrnPause ) ;
		} while ( rcB == BDCC_Error_eWrnPause ) ;
	} while ( rcA == BDCC_Error_eWrnPause ) ;
StopProcessing708 :

	/*
	 * Now check for buffer overflow.
	 * This should never happen -- if it does
	 * either it will happen very often because
	 * the buffer sizes are not provisioned correctly
	 * or we have an errant stream.  In the first case
	 * the B_Dcc_P_CBUF_SIZE and B_Dcc_P_CBUF_RESERVE need to be
	 * corrected.  In the second case, it is appropriate to
	 * reset.  The caller of this function is expected to
	 * call again to DccEngine_Reset if this function returns
	 * BDCC_Error_eBufferOverflow.
	 */
	err = CheckForBufferOverflow(hEngine) ;
	return(err) ;

} /* DccEngine_Process708 */


/**************************************************************************
 *
 * Function:		B_Dcc_Override
 *
 * Inputs:
 *					hEngine				- init'ed previously by DccEngine_Init
 *					OverrideMask		- bitmask of overridden attributes
 *					pOverrides			- structure of overrides
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function allows the caller to override some of the 708 DTVCC
 * interpretation attributes, such as pen size, font style and colors.
 *
 * The OverrideMask argument is a bitmask that identifies which of
 * structure members of *pOverrides are valid and hence overridden.  The
 * mask is absolute, not relative, meaning that overrides from a previous
 * call will be 'forgotten' if not also included in the present call.  To
 * undo all overrides and revert to the stream-supplied attributes, set
 * the OverrideMask arg to 0.
 *
 * The supported overrides are (as defined in bcmDccCoding.h):
 *
 *    UPM_PENSIZE
 *    UPM_FONTSTYLE
 *    UPM_PENFG
 *    UPM_PENBG
 *    UPM_EDGECOLOR
 *    UPM_EDGETYPE
 *
 **************************************************************************/
BDCC_Error B_Dcc_Override(
	B_Dcc_Handle hEngine,
	unsigned int OverrideMask,
	B_Dcc_OverRides * pOverrides)
{
	BDCC_Error err ;

	if ( hEngine == NULL   ||   pOverrides == NULL )
	{
		return(BDCC_Error_eNullPointer) ;
	}

	err = BDCC_Coding_P_Override(hEngine->hCodingInt, OverrideMask, pOverrides) ;

	return(err) ;

} /* B_Dcc_Override */


/**************************************************************************
 *
 * Function:		B_Dcc_Periodic
 *
 * Inputs:
 *					hEngine		- init'ed previously by DccEngine_Init
 *					usPeriod		- average us from previous call
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code
 *
 * Description:
 *
 * This function provides the Engine API a mechanism to do sequenced
 * effects.  This is used for flashing and smooth scrolling.  The usPeriod
 * arg is set to an average value.  For example, if this is driven from a
 * field interrupt, it can be set to 16683 for a 59.94 Hz field rate.
 *
 **************************************************************************/
BDCC_Error B_Dcc_Periodic( B_Dcc_Handle hEngine )
{
    /*
    ** Call down to the interpretation layer to sample/update the time.
    */
    BCCGFX_P_TimeUpdate( hEngine->hCCGfxHandle, &(hEngine->uiCurTimeMilliSecs) );

    /*
    ** call to the ccgfx library for scrolling and flashing by 2 surfaces
    */
    BCCGFX_P_Periodic(hEngine->hCCGfxHandle) ;

    /*
    ** call to the interpreter for flashing by re-rendering
    */
#if FLASH_BY_RERENDER
    BCCGFX_INT_P_Periodic(hEngine->hCodingInt) ;
#endif

    return( BDCC_Error_eSuccess ) ;

} /* B_Dcc_Periodic */




				/*********************
				 *
				 * Support Routines
				 *
				 *********************/

/**************************************************************************
 *
 * Function:		CheckForBufferOverflow
 *
 * Inputs:
 *					hEngine				- init'ed previously by DccEngine_Init
 *
 * Outputs:
 *
 * Returns:			BDCC_Error_eSuccess or BDCC_Error_eBufferOverflow
 *
 * Description:
 *
 * This function checks the ErrorCount of each of the circular buffers.
 *
 **************************************************************************/
BDCC_Error CheckForBufferOverflow(B_Dcc_Handle hEngine)
{
	BDBG_MSG(("cbBufs (%4d,%4d,%4d,%4d))\n",
		hEngine->cbTriplets.NumBytes,
		hEngine->cbPacket.NumBytes,
		hEngine->cbService.NumBytes,
		hEngine->cbCoding.NumBytes)) ;

	if ( hEngine->cbTriplets.ErrorCount )
	{
		BDBG_ERR(("ENGINE: cbTriplets overflow\n")) ;
		return(BDCC_Error_eBufferOverflow) ;
	}

	if ( hEngine->cbPacket.ErrorCount )
	{
		BDBG_ERR(("ENGINE: cbPacket overflow\n")) ;
		return(BDCC_Error_eBufferOverflow) ;
	}

	if ( hEngine->cbService.ErrorCount )
	{
		BDBG_ERR(("ENGINE: cbService overflow\n")) ;
		return(BDCC_Error_eBufferOverflow) ;
	}

	if ( hEngine->cbCoding.ErrorCount )
	{
		BDBG_ERR(("ENGINE: cbCoding overflow\n")) ;
		return(BDCC_Error_eBufferOverflow) ;
	}

	return(BDCC_Error_eSuccess) ;

} /* CheckForBufferOverflow */


void ResetDownstreamOfPacket(B_Dcc_Handle hEngine)
{
	unsigned int OverrideMask ;
	B_Dcc_OverRides Overrides ;

	BDBG_WRN(("ResetDownstreamOfPacket:  resetting\n")) ;

	/* first save the overrides */
	OverrideMask = hEngine->hCodingInt->OverrideMask ;
	Overrides = hEngine->hCodingInt->Overrides ;

	/*
	 * relevant parts of B_Dcc_Close
	 */
	BDCC_CBUF_Init(&hEngine->cbService, hEngine->BufService, sizeof(hEngine->BufService), B_Dcc_P_CBUF_RESERVE) ;
	BDCC_CBUF_Init(&hEngine->cbCoding, hEngine->BufCoding, sizeof(hEngine->BufCoding), B_Dcc_P_CBUF_RESERVE) ;
	BDCC_Coding_P_Reset(hEngine->hCodingInt, hEngine->Type) ;
	BCCGFX_P_Reset( hEngine->hCCGfxHandle, &hEngine->engineSettings );

	/* re-apply the overrides */
	B_Dcc_Override(hEngine, OverrideMask, &Overrides) ;

	BDBG_WRN(("ResetDownstreamOfPacket:  resetting - DONE\n")) ;
} /* ResetDownstreamOfPacket */


BDCC_Error BDCC_Coding_P_SendTestString(BDCC_INT_P_Handle hCodObject,
										const unsigned char *pTestSt,
										const unsigned int TestStLen);

BDCC_Error B_Dcc_SendTestString(B_Dcc_Handle hEngine,
									const unsigned char *pTestSt,
									const unsigned int TestStLen)
{

	assert(hEngine);

	return (BDCC_Coding_P_SendTestString(hEngine->hCodingInt, pTestSt, TestStLen));
}
