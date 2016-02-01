/***************************************************************************
 *     (c)2002-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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


#ifndef BDCCENGINE_H
#define BDCCENGINE_H

#ifdef __cplusplus
extern "C" {
#endif


/*=Module Overview: *******************************************************
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
 * B_Dcc_Process608 or B_Dcc_Process708.
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
				 
#include "bdcc.h"
#include "bcc_winlib.h"

				/*********************
				 *
				 * Defines
				 *
				 *********************/
				 



				/*********************
				 *
				 * Types
				 *
				 *********************/

/***************************************************************************
Summary:
	The close caption types the Engine can handle

Description:
	The close caption types the Engine can handle

See Also:
	
****************************************************************************/
typedef enum B_Dcc_Type
{
    B_Dcc_Type_NoChange, /* used when reseting the library and reusing the same modes */
    B_Dcc_Type_e608,         /* Input 608 stream is transcoded to 708 and displayed as graphics */
    B_Dcc_Type_e708          /* 708 stream is natively displayed as graphics */
    
} B_Dcc_Type ;

typedef struct B_Dcc_Settings
{
    /*
    ** TODO: per the standard, use 32 for both 4:3 and 16:9.
    ** We can probably get rid of the "Columns" element.
    */
    int Columns ;			/* number of columns for DTVCC grid use 32 for 4:3 and 42 for 16:9 */
    
    int iSafeTitleX;                /* how much the CC window needs to be indented in the X direction */
    int iSafeTitleY;                /* how much the CC window needs to be indented in the Y direction */
	uint32_t ScaleFactor;  /* in hundredths */

	BDCC_WINLIB_Interface WinLibInterface ; /* callbacks to customer window library */

    unsigned int    uiTimeOutMilliSecs;     /* if no data for this period of time, clear the screen    */
    
} B_Dcc_Settings ;


/***************************************************************************
Summary:
	The handle to the Close Caption engine.

Description:
	The handle to the Close Caption engine.

See Also:
	
****************************************************************************/
typedef struct B_Dcc_P_Object *B_Dcc_Handle ;


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
	B_Dcc_Settings *pEngineSettings);

BDCC_Error B_Dcc_GetSettings( B_Dcc_Handle hEngine, B_Dcc_Settings *pEngineSettings );
BDCC_Error B_Dcc_SetSettings( B_Dcc_Handle hEngine, B_Dcc_Settings *pEngineSettings );


/**************************************************************************                
 *  
 * Function:		B_Dcc_Open    
 *     
 * Inputs:			  
 *                                phEngine                      - Pointer to a Handle used to return the
 *                                                                      Engine handle
 *                                hWinLibHandle              - Handle from the lowest graphics  
 *                                                                      abstraction layer. Opened seperately.
 *					iCcService				- CCx for 608 (1 to 4)
 *										  Service Number for 708 (0 to 63)
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
 * This function inits the buffers used for 708 DTVCC Closed     
 * Captioning processing.  The 'Type' argument, in effect, is announcing
 * which processing entry point will be called during normal processing.
 * To switch between 608 and 708 (or to switch any of the init args), 
 * B_Dcc_Close and B_Dcc_Open must be called again -- or 
 * alternatively, B_Dcc_Reset.
 *
 * See Also:
 *    B_Dcc_Close
 *    B_Dcc_Reset
 *                
 **************************************************************************/  
BDCC_Error B_Dcc_Open(
	B_Dcc_Handle *phEngine, 
	BDCC_WINLIB_Handle hWinLibHandle, 
	BDCC_WINLIB_Interface *pWinLibCallbacks
	);
	
BDCC_Error B_Dcc_Init(
	B_Dcc_Handle hEngine, 
	int iCcService,
	B_Dcc_Type Type
	);
/**************************************************************************                
 *  
 * Function:		B_Dcc_Close   
 *     
 * Inputs:			  
 *					hEngine				- init'ed previously by B_Dcc_Open   
 *    
 * Outputs:		              
 *  
 * Returns:			BDCC_Error_eSuccess or standard BDCC_Error error code     
 *    
 * Description:                
 *         
 * This function undoes the initialization of B_Dcc_Open().
 *
 * See Also:
 *    B_Dcc_Reset
 *    B_Dcc_Open
 *                
 **************************************************************************/  
BDCC_Error B_Dcc_Close(B_Dcc_Handle hEngine);


/**************************************************************************                
 *  
 * Function:		B_Dcc_Reset    
 *     
 * Inputs:			  
 *					hEngine				- init'ed previously by B_Dcc_Open   
 *					iCcService				- CCx for 608 (1 to 4)
 *										  Service Number for 708 (0 to 63)
 *					Type				- 608, 708  or "NoChange"
 *					Columns				- number of columns for DTVCC grid
 *										  use 32 for 4:3 and 42 for 16:9
 *					CharCell_Width		- pixel width for char cell, must be
 *										  compatible with font/glyph design
 *					CharCell_Height		- pixel height for char cell, must be
 *										  compatible with font/glyph design
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
 * parameters and Type, iCcService, Columns, CharCell_Xxx are ignored.
 *
 * See Also:
 *    B_Dcc_Close
 *    B_Dcc_Open
 *                
 **************************************************************************/  
BDCC_Error B_Dcc_Reset(
	B_Dcc_Handle hEngine, 
	bool bNoChange,
	B_Dcc_Type Type,
	int iCcService
	);

/**************************************************************************                
 *  
 * Function:		B_Dcc_Process    
 *     
 * Inputs:			  
 *					hEngine			- init'ed previously by B_Dcc_Open  
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
	int NumTriplets);

/**************************************************************************                
 *  
 * Function:		B_Dcc_Process608    
 *     
 * Inputs:			  
 *					hEngine			- init'ed previously by B_Dcc_Open  
 *					pTriplets			- ptr to buf of triplets (field,cc1,cc2)
 *					NumTriplets		- count = num_bytes / 3
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
 * If this function returns BDCC_Error_eErrBufferOverflow, it is expected that the
 * caller will call again to B_Dcc_Reset.
 *
 **************************************************************************/  
BDCC_Error B_Dcc_Process608(
	B_Dcc_Handle hEngine,
	unsigned char * pTriplets,
	int NumTriplets);


/**************************************************************************                
 *  
 * Function:		B_Dcc_Process708   
 *     
 * Inputs:			  
 *					hEngine				- init'ed previously by B_Dcc_Open  
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
 * If this function returns BDCC_Error_eErrBufferOverflow, it is expected that the
 * caller will call again to B_Dcc_Reset.
 *
 **************************************************************************/  
BDCC_Error B_Dcc_Process708(
	B_Dcc_Handle hEngine,
	unsigned char * pTriplets,
	int NumTriplets);


/**************************************************************************                
 *  
 * Function:		B_Dcc_Override   
 *     
 * Inputs:			  
 *					hEngine				- init'ed previously by B_Dcc_Open  
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
	B_Dcc_OverRides * pOverrides);
	

/**************************************************************************                
 *  
 * Function:		B_Dcc_Periodic   
 *     
 * Inputs:			  
 *					hEngine				- init'ed previously by B_Dcc_Open  
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
BDCC_Error B_Dcc_Periodic( B_Dcc_Handle hEngine );


/*
** Data types and routines associated with managing fonts.
*/

typedef struct BDCC_FONT_DESCRIPTOR
{
    const char *          pszFontFile;  /*name and path to the font file */
    int                   iFontSize;    
} BDCC_FONT_DESCRIPTOR;
 
#ifdef __cplusplus
}
#endif

#endif /* BDCCENGINE_H */


