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

#ifndef BAPE_PROCESSOR_H_
#define BAPE_PROCESSOR_H_

/***************************************************************************
Summary:
Processor Handle
***************************************************************************/
typedef struct BAPE_Processor *BAPE_ProcessorHandle;

/***************************************************************************
Summary:
Processor Types
***************************************************************************/
typedef enum BAPE_ProcessorType
{
    BAPE_ProcessorType_eNone,
    BAPE_ProcessorType_eMax
} BAPE_ProcessorType;

/***************************************************************************
Summary:
Processor Create Settings 
***************************************************************************/
typedef struct BAPE_ProcessorCreateSettings
{
    BAPE_ProcessorType type;
} BAPE_ProcessorCreateSettings;

/***************************************************************************
Summary:
Get Default Processor Create Settings 
***************************************************************************/
void BAPE_Processor_GetDefaultCreateSettings(
    BAPE_ProcessorCreateSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Create a Processor
***************************************************************************/
BERR_Code BAPE_Processor_Create(
    BAPE_Handle deviceHandle,
    const BAPE_ProcessorCreateSettings *pSettings,
    BAPE_ProcessorHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
Destroy a Processor
***************************************************************************/
void BAPE_Processor_Destroy(
    BAPE_ProcessorHandle handle
    );

/***************************************************************************
Summary:
Processor Settings
***************************************************************************/
typedef union BAPE_ProcessorSettings
{
    void *pNone;    /* Placeholder, future algorithms will have structures here */
} BAPE_ProcessorSettings;

/***************************************************************************
Summary:
Get Processor Settings
***************************************************************************/
void BAPE_Processor_GetSettings(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set Processor Settings
***************************************************************************/
BERR_Code BAPE_Processor_SetSettings(
    BAPE_ProcessorHandle handle,
    const BAPE_ProcessorSettings *pSettings
    );

/***************************************************************************
Summary:
Add an input to this processor 
 
Description: 
Only outputs from a decoder or another processor may be used for 
post-processing.  Other connector types will return an error code. 
Inputs may not be added or removed while they are running. 
***************************************************************************/
BERR_Code BAPE_Processor_AddInput(
    BAPE_ProcessorHandle handle,
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Remove an input from this processor 
 
Description: 
Inputs may not be added or removed while they are running. 
***************************************************************************/
BERR_Code BAPE_Processor_RemoveInput(
    BAPE_ProcessorHandle handle,
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Remove an input from this processor 
 
Description: 
Inputs may not be added or removed while they are running. 
***************************************************************************/
BERR_Code BAPE_Processor_RemoveAllInputs(
    BAPE_ProcessorHandle handle
    );

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_Processor_GetConnector(
    BAPE_ProcessorHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

#endif /* #ifndef BAPE_PROCESSOR_H_ */
