/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_INPUT_CAPTURE_H_
#define BAPE_INPUT_CAPTURE_H_

/***************************************************************************
Summary:
InputCapture Handle
***************************************************************************/
typedef struct BAPE_InputCapture *BAPE_InputCaptureHandle;

/***************************************************************************
Summary:
InputCapture open settings
***************************************************************************/
typedef struct BAPE_InputCaptureOpenSettings
{
    bool useLargeRingBuffers;   /* If true, large ringbuffers will be used, 
                                   otherwise standard buffers will be used. */
} BAPE_InputCaptureOpenSettings;

/***************************************************************************
Summary:
Get Default InputCapture Settings
***************************************************************************/
void BAPE_InputCapture_GetDefaultOpenSettings(
    BAPE_InputCaptureOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Open an input capture channel
***************************************************************************/
BERR_Code BAPE_InputCapture_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_InputCaptureOpenSettings *pSettings,
    BAPE_InputCaptureHandle *pHandle                    /* [out] */
    );

/***************************************************************************
Summary:
Close an input capture channel
***************************************************************************/
void BAPE_InputCapture_Close(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
InputCapture Start Settings
***************************************************************************/
typedef struct BAPE_InputCaptureStartSettings
{
    BAVC_Timebase timebase;
    BAPE_InputPort input;
} BAPE_InputCaptureStartSettings;

/***************************************************************************
Summary:
Get Default InputCapture Start Settings
***************************************************************************/
void BAPE_InputCapture_GetDefaultStartSettings(
    BAPE_InputCaptureStartSettings *pSettings       /* [out] */
    );

/***************************************************************************
Summary:
Start InputCapture
***************************************************************************/
BERR_Code BAPE_InputCapture_Start(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop InputCapture
***************************************************************************/
void BAPE_InputCapture_Stop(
    BAPE_InputCaptureHandle handle
    );

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_InputCapture_GetConnector(
    BAPE_InputCaptureHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

#endif /* #ifndef BAPE_INPUT_CAPTURE_H_ */

