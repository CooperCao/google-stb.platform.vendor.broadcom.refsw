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

#ifndef BAPE_INPUT_H_
#define BAPE_INPUT_H_

/***************************************************************************
Summary:
I2S Input Handle
***************************************************************************/
typedef struct BAPE_I2sInput *BAPE_I2sInputHandle;

/***************************************************************************
Summary:
I2S Input Settings
***************************************************************************/
typedef struct BAPE_I2sInputSettings
{
    BAVC_AudioSamplingRate sampleRate;
    unsigned bitsPerSample;     /* Values from 1 to 32 are supported. */

    BAPE_I2sJustification   justification;    /* Data Justification*/
    BAPE_I2sDataAlignment   dataAlignment;    /* Data Alignment */
    BAPE_I2sLRClockPolarity lrPolarity;       /* LRCK polarity */
    BAPE_I2sSclkPolarity    sclkPolarity;     /* SCLK polarity */
} BAPE_I2sInputSettings;

/***************************************************************************
Summary:
Get Default I2S Input Settings
***************************************************************************/
void BAPE_I2sInput_GetDefaultSettings(
    BAPE_I2sInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Open an I2S Input
***************************************************************************/
BERR_Code BAPE_I2sInput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sInputSettings *pSettings
    );

/***************************************************************************
Summary:
Close an I2S Input
***************************************************************************/
void BAPE_I2sInput_Close(
    BAPE_I2sInputHandle handle
    );

/***************************************************************************
Summary:
Get I2S Input Settings
***************************************************************************/
void BAPE_I2sInput_GetSettings(
    BAPE_I2sInputHandle handle,
    BAPE_I2sInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Set I2S Input Settings
***************************************************************************/
void BAPE_I2sInput_SetSettings(
    BAPE_I2sInputHandle handle,
    const BAPE_I2sInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get Input Port Connector
***************************************************************************/
void BAPE_I2sInput_GetConnector(
    BAPE_I2sInputHandle handle,
    BAPE_InputPort *pPort
    );

#endif /* #ifndef BAPE_INPUT_H_ */

