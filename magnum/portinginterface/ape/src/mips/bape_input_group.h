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

#ifndef BAPE_INPUT_GROUP_H_
#define BAPE_INPUT_GROUP_H_

/***************************************************************************
Summary:
Input Group Handle
***************************************************************************/
typedef struct BAPE_InputGroup *BAPE_InputGroupHandle;

/***************************************************************************
Summary:
Input Group Settings
***************************************************************************/
typedef struct BAPE_InputGroupSettings
{
    BAPE_MixerInput inputs[BAPE_ChannelPair_eMax];  /* Each input must provide stereo data */
} BAPE_InputGroupSettings;

/***************************************************************************
Summary:
Get Default Input Group Settings
***************************************************************************/
void BAPE_InputGroup_GetDefaultSettings(
    BAPE_InputGroupSettings *pSettings      /* [out] */
    );

/***************************************************************************
Summary:
Create an input group
***************************************************************************/
BERR_Code BAPE_InputGroup_Create(
    BAPE_Handle deviceHandle,
    const BAPE_InputGroupSettings *pSettings,
    BAPE_InputGroupHandle *pHandle              /* [out] */
    );

/***************************************************************************
Summary:
Destroy an input group
***************************************************************************/
void BAPE_InputGroup_Destroy(
    BAPE_InputGroupHandle handle
    );

/***************************************************************************
Summary:
Get the mixer input handle for an input group
***************************************************************************/
void BAPE_InputGroup_GetConnector(
    BAPE_InputGroupHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

#endif /* #ifndef BAPE_INPUT_GROUP_H_ */

