/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
*   Module name: SPDIF/HDMI In (Receiver)
*   This file contains the definitions and prototypes for the hardware SPDIF/HDMI
*   receiver abstraction.
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_SPDIFRX_PRIV_H__ /*{{{*/
#define _BRAP_SPDIFRX_PRIV_H__

/*{{{ Defines */

#define BRAP_P_SPDIFRX_INVALID_PREAMBLE_C      0x12340000	/* Maximum bits per sample */

/**********************************************************************
Summary:
    SPDIF Receiver Digital Input Format
Description:
   This structure returns information of Digital Input format whenever there
   is a change in the format. It is used for the notification to the application
   about the change in the format.
Notes: 
    This structure will be passed on to application callback function
    on Digital input format change
**********************************************************************/
typedef struct BRAP_P_SPDIFRX_InputFormat
{
    BRAP_SpdifRx_StreamType eStreamType;   /* Input Format. May be linear or compressed  */
    BRAP_DSPCHN_AudioType   eType;         /* Audio Type. This fiels is valid only 
                                                when the stream type is compressed */
    BAVC_AudioSamplingRate  eSampleRate;   /* Input Sampling Rate. This field in only 
                                                valid when the stream is PCM (linear) */
}BRAP_P_SPDIFRX_InputFormat;

/***************************************************************************
Summary:
    This enumeration is used by the SPDIF Rx for the State Machine Implementation
    in detecting the input change.
***************************************************************************/ 
typedef enum BRAP_P_SPDIFRX_InputState
{   
    BRAP_P_SPDIFRX_InputState_eStreamNone,
    BRAP_P_SPDIFRX_InputState_eStreamPCM,
    BRAP_P_SPDIFRX_InputState_eStreamHdmiPCM,    
    BRAP_P_SPDIFRX_InputState_eStreamPendingCompress,
    BRAP_P_SPDIFRX_InputState_eStreamGoodCompress,
    BRAP_P_SPDIFRX_InputState_eInvalid,
    BRAP_P_SPDIFRX_InputState_eMax
}BRAP_P_SPDIFRX_InputState;

/***************************************************************************
Summary:
    This structure is used as input by the SPDIF Rx State Machine Implementation
    in detecting the input change.
***************************************************************************/ 
typedef struct BRAP_P_SPDIFRX_DetectChange_InputParams
{
    /* The capture input port for which the input change is being monitored */
    unsigned int                eCapInputPort;     

    /* The Current Input State according to the detection logic */ 
    BRAP_P_SPDIFRX_InputState   eCurrentState;        

    /* The Current Compressed state. Valid only if the eCurrentState is GoodCompress */
    uint32_t                    ui32CurrentCompState; 
    
    /* The Current Esr Status Value stored in hRapch. To be used in detection logic */
    uint32_t                    ui32CurrentEsrStatus; 

}BRAP_P_SPDIFRX_DetectChange_InputParams;

typedef struct BRAP_P_SPDIFRX_DetectChange_OutputParams
{
    /* The Changed/New Input State according to the detection logic */ 
    BRAP_P_SPDIFRX_InputState       eNewState;

    /* The Changed/New Compressed state. Valid only if the eNewState is GoodCompress */
    uint32_t                        ui32NewCompState;    
    
    /* The information on the new changed input */
    BRAP_P_SPDIFRX_InputFormat      sInputFormatInfo;
}BRAP_P_SPDIFRX_DetectChange_OutputParams;


/* SPDIF Rx Private Functions */
BERR_Code 
BRAP_SPDIFRX_P_Open(
    BRAP_Handle         hRap,
    BRAP_CapInputPort	eSpdifRxInputPort
    );

BERR_Code 
BRAP_SPDIFRX_P_Close(
    BRAP_Handle         hRap
    );

BERR_Code 
BRAP_SPDIFRX_P_Start(
    BRAP_Handle          hRap
    );

BERR_Code 
BRAP_SPDIFRX_P_Stop(
    BRAP_Handle          hRap
    );

BERR_Code 
BRAP_SPDIFRX_P_GetRxStatus(
    BRAP_Handle          hRap,
    BRAP_SpdifRx_Status  *pStatus
    );

BERR_Code 
BRAP_SPDIFRX_P_GetChannelStatus (
    BRAP_Handle          hRap,
    BRAP_SpdifRx_ChannelStatus  *pChannelStatus
    );

BERR_Code 
BRAP_SPDIFRX_P_GetInputFormatInfo(
    BRAP_Handle          hRap,                 
    BRAP_P_SPDIFRX_InputFormat *psInputFormatInfo   
    );

bool 
BRAP_SPDIFRX_P_DetectInputChange (
    BRAP_Handle hRap, 
    BRAP_P_SPDIFRX_DetectChange_InputParams     *psInputParams,
    BRAP_P_SPDIFRX_DetectChange_OutputParams    *psOutputParams
    );


BRAP_P_SPDIFRX_InputState 
BRAP_SPDIFRX_P_SwitchToPES (
    BRAP_Handle hRap, 
    uint32_t ui32SpdifRxCtrlStatus 
    );

BERR_Code
BRAP_SPDIFRX_P_SwitchToPCM (
    BRAP_Handle hRap
    );

BERR_Code
BRAP_SPDIFRX_P_SwitchToNone (
    BRAP_Handle hRap
    );

BERR_Code
BRAP_SPDIFRX_P_SwitchToCompressed (
    BRAP_Handle hRap
    );


#endif /*}}} #ifndef _BRAP_SPDIFRX_PRIV_H__ */
	
/* End of File */
	

