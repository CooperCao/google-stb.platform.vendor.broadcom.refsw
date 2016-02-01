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
 * Module Description:
 *   This module print out helpful information to debug BVN
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BVDC_DBG_H__
#define BVDC_DBG_H__

#include "bstd.h"
#include "bvdc.h"

#define BVDC_DBG_NOT_APPLICABLE 0xffffffff

/***************************************************************************
Summary:
	List of BVN errors.

Description:
	This is the enumerated list of BVN module errors.

See Also:
****************************************************************************/
typedef enum
{
	BVDC_Bvn_eRdc,
	BVDC_Bvn_eMfd,
	BVDC_Bvn_eVfd,
	BVDC_Bvn_eScl,
	BVDC_Bvn_eDnr,
	BVDC_Bvn_eMad,
	BVDC_Bvn_eMvp,
	BVDC_Bvn_eMcdi,
	BVDC_Bvn_eMctf,
	BVDC_Bvn_eHscl,
	BVDC_Bvn_eCap,
	BVDC_Bvn_eGfd,
	BVDC_Bvn_eCmp_V0,
	BVDC_Bvn_eCmp_V1,
	BVDC_Bvn_eCmp_G0,
	BVDC_Bvn_eMaxCount
} BVDC_BvnModule;

typedef struct
{
	uint32_t ulNumErr;
} BVDC_Source_DebugStatus;

typedef struct
{
	uint32_t ulNumErr;
    uint32_t ulVsyncCnt; /* Vsync heartbeat */
    uint32_t ulLineNumber; /* Raster line number for non-STG displays. Not applicable for STG-based displays */
} BVDC_Window_DebugStatus;

typedef struct
{
	bool bEnableError;
} BVDC_Scaler_DebugStatus;

BERR_Code BVDC_Dbg_InstallCallback
	( BVDC_Handle                      hVdc,
	  const BVDC_CallbackFunc_isr      pfCallback,
	  void                            *pvParm1,
	  int                              iParm2 );

/***************************************************************************
Summary:
	This function clears the BVN error counter.

Description:

Input:
	hVdc - The VDC handle that application created earlier.

Returns:
    None.

See Also:
	BVDC_Dbg_GetBvnErrorStatus
**************************************************************************/

void BVDC_Dbg_ClearBvnError
	( BVDC_Handle                  hVdc );

/***************************************************************************
Summary:
	This function prints the accumulated BVN errors that occurred since
	the system started or since BVDC_Dbg_ClearBvnError was invoked.

Description:
	The errors can come from any active BVN component.

Input:
	hVdc - The VDC handle that application created earlier.

Returns:
    None.

See Also:
	BVDC_Dbg_ClearBvnError
**************************************************************************/
void BVDC_Dbg_GetBvnErrorStatus
	( BVDC_Handle                  hVdc );

/***************************************************************************
Summary:
	This function prints the accumulated BVN errors that occurred since
	the system started or since BVDC_Dbg_ClearBvnError was invoked.
	This function is safe to call from ISR context.

Description:
	The errors can come from any active BVN component.

Input:
	hVdc - The VDC handle that application created earlier.

Returns:
    None.

See Also:
	BVDC_Dbg_ClearBvnError
**************************************************************************/
void BVDC_Dbg_GetBvnErrorStatus_isr
	( BVDC_Handle                  hVdc );

/***************************************************************************
Summary:
	This function gets a information that would aid in debugging window-based
	errors.

Description:
	Currently this function returns the number of errors (contained in the
	struct) that occurred since	the window was created. The errors may come
	from different BVN modules associated with the given window.

Input:
	hWindow - The window handle that application created earlier.

Output:
	pDbgInfo - struct that contains the error count

Returns:
    None.

See Also:
	BVDC_Window_DebugStatus
**************************************************************************/

void BVDC_Dbg_Window_GetDebugStatus
	( const BVDC_Window_Handle         hWindow,
	  BVDC_Window_DebugStatus         *pDbgInfo );

/***************************************************************************
Summary:
	This function gets a information that would aid in debugging source-based
	errors.

Description:
	Currently this function returns the number of errors (contained in the
	struct) that occurred since	the source was created. Source here means
	that it is one of the available MFD, VFD, or GFD modules.

Input:
	hSource - The source handle that application created earlier.

Output:
	pDbgInfo - struct that contains the error count

Returns:
    None.

See Also:
	BVDC_Source_DebugStatus
**************************************************************************/
void BVDC_Dbg_Source_GetDebugStatus
	( const BVDC_Source_Handle         hSource,
	  BVDC_Source_DebugStatus         *pDbgInfo );

/***************************************************************************
Summary:
	This masks/unmasks a BVN error.

Description:
	By default all BVN errors are unmasked.

Input:
	hVdc - The VDC handle that application created earlier.
    eBvnModule - the BVN module eg., MFD, MFD, SCL, MAD, etc
    ulModuleIdx - the specific BVN module, eg., MFD0, SCL1, etc
                - Note that this must be an integer
    bMaskOff - this masks or unmasks the given BVN module

Output:
	pDbgInfo - struct that contains the error count

Returns:
    BERR_SUCCESS, BERR_INVALID_PARAMETER.

See Also:

**************************************************************************/

BERR_Code BVDC_Dbg_MaskBvnErrorCb
	( BVDC_Handle                  hVdc,
	  BVDC_BvnModule               eBvnModule,
	  uint32_t                     ulModuleIdx,
	  bool                         bMaskOff );


/***************************************************************************
Summary:
	This function gets a source handle.

Description:
	Get the current active source by id.

	DBG function for VTLib (Video Tool Library) getting vdc handles
	that was created _bsp_ software that already abstract these handles from
	VTLib but only expose thru hVdc main handle.  For eId there are eAuto, but
	these are for the _Create() function.  BVDC_Dbg_GetXXXHandle functions will
	accept these and treat them as invalid parameter.  If the handle is not
	active it will return NULL.

	WARNING: It's expected that APPLICATION/DRIVER does not call of any of these
	get handle function.  These get handle function only return what the current
	state VDC.  Caller expted to be VTLib equivalent, and caller must ensure
	the synchronization between APPLICATION/DRIVER and VTLib.

Input:
	hVdc - The VDC handle that application created earlier.

	eSourceId - Specify what source Id should VDC tries to get. eId must
	be a valid source id for this chip set.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Source_Create.
**************************************************************************/
BVDC_Source_Handle BVDC_Dbg_GetSourceHandle
	( const BVDC_Handle                hVdc,
	  BAVC_SourceId                    eSourceId );

/***************************************************************************
Summary:
	This function gets a compositor handle.

Description:
	Get the current active compositor by id.

Input:
	hVdc - The VDC handle that application created earlier.

	eCompositorId - Specify what compositor Id should VDC tries to get.
	eCompositorId must be a valid compositor id for this chip set.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Compositor_Create.
**************************************************************************/
BVDC_Compositor_Handle BVDC_Dbg_GetCompositorHandle
	( const BVDC_Handle                hVdc,
	  BVDC_CompositorId                eCompositorId );

/***************************************************************************
Summary:
	This function gets a window handle.

Description:
	Get the current active window by id.

Input:
	hVdc - The VDC handle that application created earlier.

	eCompositorId - Specify what compositor Id should VDC tries to get the
	window from.

	eWindowId - Specify what compositor Id should VDC tries to get.  eWindowId
	must be a valid window id for this chip set.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Window_Create.
**************************************************************************/
BVDC_Window_Handle BVDC_Dbg_GetWindowHandle
	( const BVDC_Handle                hVdc,
	  BVDC_CompositorId                eCompositorId,
	  BVDC_WindowId                    eWindowId );

/***************************************************************************
Summary:
	This function gets a source handle.

Description:
	Get the current active source from a given window.

Input:
	hWindow - The VDC handle that application created earlier.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Dbg_GetWindowHandle.
**************************************************************************/
BVDC_Source_Handle BVDC_Dbg_Window_GetSourceHandle
	( const BVDC_Window_Handle         hWindow );

/***************************************************************************
Summary:
	This function gets a compositor handle.

Description:
	Get the current active compositor from a given window.

Input:
	hWindow - The VDC handle that application created earlier.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Dbg_GetWindowHandle.
**************************************************************************/
BVDC_Compositor_Handle BVDC_Dbg_Window_GetCompositorHandle
	( const BVDC_Window_Handle         hWindow );

/***************************************************************************
Summary:
	This function gets Scaler Status.

Description:
	Get the status register value of the Scaler associate with the current
	window.

Input:
	hWindow - The VDC handle that application created earlier.

Output:
	pStatus - structure to return scaler error status

Returns:
	Return the SCL BVB status register value.  0 if window not created.

See Also:
**************************************************************************/
uint32_t BVDC_Dbg_Window_GetScalerStatus
	( const BVDC_Window_Handle         hWindow,
	  BVDC_Scaler_DebugStatus         *pStatus );

/***************************************************************************
Summary:
	This function gets a display handle.

Description:
	Get the current active display from a given compositor.

Input:
	hCompositor - The VDC handle that application created earlier.

Returns:
	Return current active handle if success.  Otherwise return NULL.

See Also:
	BVDC_Dbg_GetCompositorHandle.
**************************************************************************/
BVDC_Display_Handle BVDC_Dbg_Compositor_GetDisplayHandle
	( const BVDC_Compositor_Handle     hCompositor );

#if (BVDC_BUF_LOG == 1)
/*****************************************************************************
Summary:
    Enum for users to specify how they want the multi-buffering events
    be logged and dumped.

See Also:
    BVDC_SetBufLogStateAndDumpTrigger
*****************************************************************************/

typedef enum    {
    BVDC_BufLogState_eReset=0,           /* Reset/inactive */
    BVDC_BufLogState_eManual,            /* Manually trigger the dump */
    BVDC_BufLogState_eAutomatic,         /* Automatic trigger - whenever a skip/repeat event occurs */
    BVDC_BufLogState_eAutomaticReduced   /* Automatic trigger - whenever a skip/repeat event occurs, only shows skip/repeat */
} BVDC_BufLogState;

/***************************************************************************
Summary:
	Set when to start logging multi-buffering events and how to notify user
	the log can be dumped.

Description:
	This function allows user to specify how the log can be dumped.
	If BVDC_BufLogState_eAutomatic or BVDC_BufLogState_eAutomaticReduced is
	chosen, a user callback function must be registered.

Inputs:
	eLogState		- Log and dump trigger state
	pfCallback      - User callback function
	pvParam1        - User defined data structure casted to void.
	iParam2         - Additional user defined value.

Returns:

	BERR_SUCCESS			  - The function call succeeded.
	BERR_INVALID_PARAMETER	  - A supplied parameter was invalid,
								possibly NULL.

See Also:
	BVDC_P_BufLogState
****************************************************************************/
BERR_Code BVDC_SetBufLogStateAndDumpTrigger
	( BVDC_BufLogState                 eLogState,
  	  const BVDC_CallbackFunc_isr 	   pfCallback,
	  void							   *pvParm1,
	  int 							   iParm2 );


/***************************************************************************
Summary:
	Print out the captured multi-buffering events log.

Description:
	This function dumps the captured multi-buffering events log.

Inputs:

Returns:

See Also:

****************************************************************************/
void BVDC_DumpBufLog (void );

/***************************************************************************
Summary:
	Prepares the manual trigger of the multi-buffering event log

Description:
	This function prepares the multi-buffering log internals such that
    the next BVDC_DumpBufLog() dumps up to the most recent events,
    regardless any skip/repeat event have been logged in the recent past.

Inputs:

Returns:

See Also:

****************************************************************************/
void BVDC_SetBufLogManualTrigger(void);

/***************************************************************************
Summary:
	Enable/Disable multi-buffering logging for a specific window

Description:
	This function allows user to specify whether to enable or disable
	multi-buffering logging for a specific window. Multi-buffering logging
	is enabled for all video window by default.

Inputs:
	hWindow      - The VDC handle that application created earlier.
	bEnable      - flag to indicate whether to enable or disable multi-buffering
	               logging.

Returns:

	BERR_SUCCESS			  - The function call succeeded.

See Also:
	BVDC_P_BufLogState

****************************************************************************/
BERR_Code BVDC_Window_EnableBufLog
	( const BVDC_Window_Handle         hWindow,
	  bool                             bEnable );

#endif

#ifdef BVDC_P_DISPLAY_DUMP
void BVDC_Display_DumpAll_aulVfTable (void);
void BVDC_Display_DumpAll_aulChFilterTbl (void);
void BVDC_Display_DumpAll_aulRmTable (void);
void BVDC_Display_DumpAll_aulItTable (void);
void BVDC_Display_DumpAll_ulItConfig (void);
void BVDC_Display_DumpAll_aulSmTable (void);
void BVDC_Display_DumpTables (void);

void BVDC_Display_MV_DumpAll_aulVfTable (void);
void BVDC_Display_MV_DumpAll_aulItTable (void);
void BVDC_Display_MV_DumpAll_ulItConfig (void);
void BVDC_Display_MV_DumpTables (void);
#endif

#endif /* #ifndef BVDC_DBG_H__ */

/* end of file */
