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
 *	Header file for Test functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BVDC_TEST_H__
#define BVDC_TEST_H__

#include "bvdc.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	A structure representing the captured field/frame that is provided to the
	user application.

Description:
	hPicBlock          - MMA block that has the captured picture derived from a field or frame
	ulPicBlockOffset   - offset for field or frame
	hPicBlock_R        - MMA block that has the captured picture derived from a right field or frame
	ulPicBlockOffset_R - offset for right field or frame
	ulWidth         - width of the captued image
	ulHeight        - height of the captued image
	ulPitch         - pitch of the captued image
	eFormat         - pixel format in the captued image
	eOutOrientation - display 3D orientation of the left and right surfaces
	eCapturePolarity - the polarity of the captured image.
	ulOrigPTS       - Original PTS of the picture.  Note this PTS value could be
	    either original coded PTS or interolated PTS by DM.  See also
	    BAVC_MFD_Picture.ulOrigPTS.
	bStallStc       - See also BAVC_XVD_Picture.bStallStc
	bIgnorePicture  - See also BAVC_XVD_Picture.bIgnorePicture
	ulPxlAspRatio_x - captured image aspect ratio x. See also BVDC_P_CompositorContext.ulStgPxlAspRatio_x_y
    ulPxlAspRatio_y - captured image aspect ratio y. See also BVDC_P_CompositorContext.ulStgPxlAspRatio_x_y
See Also:
	BVDC_Test_Window_GetBuffer_isr
****************************************************************************/
typedef struct
{
	BMMA_Block_Handle            hPicBlock;
    unsigned                     ulPicBlockOffset;
	BMMA_Block_Handle            hPicBlock_R;
    unsigned                     ulPicBlockOffset_R;
	uint32_t                     ulWidth;
	uint32_t                     ulHeight;
	uint32_t                     ulPitch;
	uint32_t                     ulEncPicId;
	uint32_t                     ulDecPicId;
	BPXL_Format                  ePxlFmt;
	BFMT_Orientation             eDispOrientation;
	BAVC_Polarity                eCapturePolarity;
	uint32_t                     ulSourceRate;
	uint32_t                     ulOrigPTS;
	bool                         bStallStc;
	bool                         bIgnorePicture;
	uint32_t                     ulPxlAspRatio_x;
	uint32_t                     ulPxlAspRatio_y;
} BVDC_Test_Window_CapturedImage;

/***************************************************************************
Summary:
	This function show/hide OSD of MAD for a window if that window enables
	deinterlace.

Description:
	Disable or enable OSD and set the OSD position.

Input:
	hWindow    - A valid window handle created ealier.
	bEnable    - boolean to enable/disable OSD.
	ulHpos     - On screen display horizontal position.
	             Note: ulHPos must be an even number due to YUV422 format.
	ulVpos     - On screen display vertical position.

Output:

Returns:
	BERR_SUCCESS - Successfully set the flag to shows/hides the OSD for the window.

See Also:
**************************************************************************/
BERR_Code BVDC_Test_Window_SetMadOsd
	( BVDC_Window_Handle               hWindow,
	  bool                             bEnable,
	  uint32_t                         ulHpos,
	  uint32_t                         ulVpos);

/***************************************************************************
Summary:
	This function gets OSD setting of MAD block if the window enables deinterlace.

Description:
	Get the OSD setting of window's MAD block.

Input:
	hWindow    - A valid window handle created ealier.
	bEnable    - boolean to enable/disable OSD.
	pulHpos    - Pointer to the On screen display horizontal position.
	pulVpos    - Pointer to the On screen display vertical position.

Output:

Returns:
	BERR_SUCCESS - Successfully Get the MAD's OSD status of the window.

See Also:
**************************************************************************/
BERR_Code BVDC_Test_Window_GetMadOsd
	( BVDC_Window_Handle               hWindow,
	  bool                            *pbEnable,
	  uint32_t                        *pulHpos,
	  uint32_t                        *pulVpos);

/***************************************************************************
Summary:
	This function sets source fix color output

Description:
	Overwrite the source color matrix to outputing a fix color

Input:
	hWindow    - A valid window handle created ealier.
	eFieldId   - Field polarity to have this fix color.
	bEnable    - boolean to enable/disable fix color.
	ucRed      - The RED component of the fix color.
	ucGreen    - The GREEN component of the fix color.
	ucBlue     - The BLUE component of the fix color.

Output:

Returns:
	BERR_SUCCESS - Successfully set the fix color for the source.

See Also:
	BVDC_Test_Source_GetFixColor
**************************************************************************/
BERR_Code BVDC_Test_Source_SetFixColor
	( BVDC_Source_Handle               hSource,
	  BAVC_Polarity                    eFieldId,
	  bool                             bEnable,
	  uint32_t                         ulRed,
	  uint32_t                         ulGreen,
	  uint32_t                         ulBlue );

/***************************************************************************
Summary:
	This function gets source fix color output

Description:
	Gets the source fix color for a field polarity if it is enable

Input:
	hWindow    - A valid window handle created ealier.
	eFieldId   - Field polarity to have this fix color.

Output:
	pbEnable   - boolean to enable/disable fix color.
	pucRed     - The RED component of the fix color.
	pucGreen   - The GREEN component of the fix color.
	pucBlue    - The BLUE component of the fix color.

Returns:
	BERR_SUCCESS - Successfully get the fix color for the source.

See Also:
	BVDC_Test_Source_SetFixColor
*************************************************************************/
BERR_Code BVDC_Test_Source_GetFixColor
	( BVDC_Source_Handle               hSource,
	  BAVC_Polarity                    eFieldId,
	  bool                            *pbEnable,
	  uint32_t                        *pulRed,
	  uint32_t                        *pulGreen,
	  uint32_t                        *pulBlue );

/***************************************************************************
Summary:
	Obtains the last captured buffer from the associated video window in
	_isr context

Description:
	This is the _isr version of BVDC_Window_GetBuffer. Refer to it for
	description first.

	An example application of this _isr version is for upper level software
	to get the current VDC display buffer and to pass to Raaga encoder,
	inside the callback function called from VDC at every display vsync. The
	callback function is installed by upper level software to VDC display.

	When field inversion happens, the captured buffer's polarity will not
	match the display polarity passed to the the callback function.

	Upper level software should call BVDC_Test_Window_GetBuffer_isr inside
	or after the display's per vsync callback. It should only be called ONCE.
	If more than once are called, the returned buffers are likely out of
	time order.

Input:
	hWindow - the window handle created earlier with BVDC_Window_Create.

Output:
	pCapturedImage - the last captured buffer associated with hWindow.

Returns:
	BVDC_ERR_CAPTURED_BUFFER_NOT_FOUND - No valid capture buffer to return
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Function succeed

See Also:
	BVDC_Test_Window_ReturnBuffer_isr, BVDC_Test_Window_CapturedImage,
	BVDC_Window_SetUserCaptureBufferCount, BVDC_Display_InstallCallback
	BVDC_Open, BVDC_Window_SetPixelFormat,
**************************************************************************/
BERR_Code BVDC_Test_Window_GetBuffer_isr
	( BVDC_Window_Handle               hWindow,
	  BVDC_Test_Window_CapturedImage  *pCapturedImage);

/***************************************************************************
Summary:
	Returns a captured buffer to the associated window in _isr context.

Description:
	This is _isr verion of BVDC_Window_ReturnBuffer. Refer to it for
	description first.

Input:
	hWindow - the window handle created earlier with BVDC_Window_Create.
	pCapturedImage - the captured buffer associated with hWindow.

Output:
	None

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Function succeeds

Note:
	An outstanding capture buffer must be returned and freed if the
	associated window is to be shut down or reconfigured.

See Also:
	BVDC_Test_Window_GetBuffer_isr, BVDC_Test_Window_CapturedImage,
	BVDC_Window_SetUserCaptureBufferCount, BVDC_Display_InstallCallback
	BVDC_Open, BVDC_Window_SetPixelFormat,
**************************************************************************/
BERR_Code BVDC_Test_Window_ReturnBuffer_isr
	( BVDC_Window_Handle               hWindow,
	  BVDC_Test_Window_CapturedImage  *pCapturedImage );

/***************************************************************************
Summary:
	Returns RDC scratch registers used by a given GFD.

Description:
	This function returns the RDC scratch registers where the
	surface addresses are stored. This function is typically used in
	applications that generate RULs that need to be ran independent
	of the BRCM driver, eg. CFE or any bootloader, that has a splash
	display feature.

Input:
	hSource - the source handle created. This is associated with the GFD used
	to feed in the surface to the BVN

Output:
	pulScratchReg1, pulScratchReg2 - the returned scratch registers.


Returns:
	BERR_SUCCESS - Function succeeds


See Also:
	BVDC_Source_Create, BVDC_Window_SetPixelFormat,
**************************************************************************/
BERR_Code BVDC_Test_Source_GetGfdScratchRegisters
	( BVDC_Source_Handle               hSource,
	  uint32_t                        *pulScratchReg1,
	  uint32_t                        *pulScratchReg2 );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_TEST_H__ */
/* End of file. */
