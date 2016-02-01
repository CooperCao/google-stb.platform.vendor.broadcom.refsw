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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BBOX_H__
#define BBOX_H__

#include "bavc_types.h"
#include "berr_ids.h"            /* Error codes */
#include "bbox_vdc.h"
#include "bbox_vce.h"
#include "bbox_rts.h"
#include "bbox_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The box module describes the list of capabilities available for a given
platform. The intent is to expose only those controls and capabilities that
pertain to the given platform.
****************************************************************************/

/***************************************************************************
Summary:
	List of errors unique to BOX
****************************************************************************/
#define BBOX_ID_NOT_SUPPORTED                  BERR_MAKE_CODE(BERR_BOX_ID, 0)
#define BBOX_RTS_ALREADY_LOADED                BERR_MAKE_CODE(BERR_BOX_ID, 1)
#define BBOX_ID_AND_RTS_MISMATCH               BERR_MAKE_CODE(BERR_BOX_ID, 2)
#define BBOX_INCORRECT_MEMC_COUNT              BERR_MAKE_CODE(BERR_BOX_ID, 3)
#define BBOX_WINDOW_SIZE_EXCEEDS_LIMIT         BERR_MAKE_CODE(BERR_BOX_ID, 4)
#define BBOX_FRAME_BUFFER_SIZE_EXCEEDS_LIMIT   BERR_MAKE_CODE(BERR_BOX_ID, 5)

/***************************************************************************
Summary:
	Used to specify memc index.
****************************************************************************/
typedef enum BBOX_MemcIndex
{
	BBOX_MemcIndex_0 = 0,
	BBOX_MemcIndex_1,
	BBOX_MemcIndex_2,
	BBOX_MemcIndex_3,
	BBOX_MemcIndex_Invalid

} BBOX_MemcIndex;

/***************************************************************************
Summary: Specifies settings for a box mode.

Description:
	This data structure specifies the unique identifier for a given box mode.

See Also:
	BBOX_Open
****************************************************************************/
typedef struct BBOX_Settings
{
	uint32_t		   ulBoxId; /* This provided by app or upper layer SW
								   and is determinate. */
} BBOX_Settings;


/***************************************************************************
Summary:
	Data structure describing memc index settings for the given box mode.

Description:

See Also: BBOX_GetMemConfig
****************************************************************************/
typedef struct BBOX_MemConfig
{
	/* VDC MEMC index */
	BBOX_Vdc_MemcIndexSettings     stVdcMemcIndex;

	uint32_t                       ulNumMemc;
	/* TODO: Add XVD MEMC index */

} BBOX_MemConfig;

/***************************************************************************
Summary:
	Data structure describing capabilities exposed by given PI modules
	and/or upper layer SW for the given box mode.

Description:

See Also: BBOX_GetConfig
****************************************************************************/
typedef struct BBOX_Config
{
	BBOX_Settings		  stBox;

	/* VDC features */
	BBOX_Vdc_Capabilities stVdc;

	/* VCE features */
	BBOX_Vce_Capabilities stVce;

	/* Audio features */
	BBOX_Audio_Capabilities stAudio;

	/* Add other module capabilities here */

	BBOX_MemConfig        stMemConfig;
} BBOX_Config;

/***************************************************************************
Summary:
	The handle for the BOX module.

Description:
	This is the main handle required to access box mode	capabilities of a
	given module.

See Also:
	BBOX_Open
****************************************************************************/
typedef struct BBOX_P_Context *BBOX_Handle;

/***************************************************************************
Summary:
	Opens the BOX module.

Description:

	A box mode ID is passed to determine the appropriate configuration.

	See BBOX_GetConfig on how the box mode ID is used.

Returns:
	Returns a handle to the BOX module.

	BBOX_ID_NOT_SUPPORTED - passed ID is not supported
	BERR_OUT_OF_SYSTEM_MEMORY - failed to allocate memory for BOX module
	BERR_SUCCESS - Successfully opened the BOX module

See Also:
	BBOX_GetConfig, BBOX_Close, BBOX_Settings
****************************************************************************/
BERR_Code BBOX_Open
	( BBOX_Handle					  *phBox,		   /* [out] BOX handle */
	  const BBOX_Settings			  *pBoxSettings ); /* [in]	*/


/***************************************************************************
Summary:
	Closes the BOX module.

Description:

Returns:

See Also:
	BBOX_Open
****************************************************************************/
BERR_Code BBOX_Close
	( BBOX_Handle					   hBox ); /* [in] BOX handle to close */

/*****************************************************************************
Summary:
	Get a box mode configuration

Description:
	This has 3 usages:

	1. For legacy chips that do not know about box modes, an upper layer
	software can pass a NULL for the BBOX_Handle parameter.
	This implies that a box mode ID of 0 is used. This is to address
	backwards compatibility for these legacy chips and whose SW does not
	call BBOX_Open.

		A filled BBOX_Config is returned.

	2. For legacy chips that know about box modes, an upper layer software
	passes a valid BBOX_Handle obtained from BBOX_Open. BBOX_Open is called
	with a box mode ID and will return a BBOX_Config struct according to the
	following.

		a) A box mode ID of 0 returns a filled BBOX_Config.
		b) A non-zero box mode ID returns an empty BBOX_Config.

	3. For chips that know about box modes, an upper layer software
	passes a valid BBOX_Handle obtained from BBOX_Open. BBOX_Open is called
	with a box mode ID and will return a BBOX_Config struct according to the
	following.

		a) A box mode ID of 0 returns an empty BBOX_Config.
		b) A valid box mode ID returns a filled BBOX_Config.
		c) An invalid non-zero box mode ID returns an empty BBOX_Config.

Returns:
	BERR_INVALID_PARAMETER

See Also:
	BBOX_Open, BBOX_Config
*****************************************************************************/
BERR_Code BBOX_GetConfig
	( BBOX_Handle                      hBox,
	  BBOX_Config                     *pBoxConfig ); /* [out] Box configuration. */

/***************************************************************************
Summary:
	Loads the RTS set according to the given box mode.

Description:
	Each box mode has an associated RTS. This function facilitates loading
	of this RTS set without rebooting the box. However, this function
	MUST only called at initialization and cannot be called at any point
	thereafter.

Returns:
	BERR_INVALID_PARAMETER
	BBOX_RTS_ALREADY_LOADED

See Also:
	BBOX_Open
****************************************************************************/

BERR_Code BBOX_LoadRts
	( BBOX_Handle                      hBox,
	  const BREG_Handle                hReg );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_H__ */

/* end of file */
