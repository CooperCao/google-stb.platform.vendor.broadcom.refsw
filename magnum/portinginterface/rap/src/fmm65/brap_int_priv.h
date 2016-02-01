/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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

#ifndef BRAP_INT_PRIV_H__
#define BRAP_INT_PRIV_H__

#include "brap_priv.h"


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
Summary: 
    This structure maintains information for the BRAP application callbacks 
    associated with a Raptor Interrupt.
    
Note:  
    The parameters passed to BRAP_CallbackFunc are
        void *    pParm1    : Application specific (passed unchanged)
        int       iParm2    : Application specific (passed unchanged)
        void *    pRAP_data : Raptor specific data exchanged between the 
                              BINT_Callback and the Application callback
******************************************************************************/
typedef struct BRAP_P_AppIntCbInfo
{
    BRAP_CallbackFunc pfAppCb;  /* Application specified Callback fnc */
    void *            pParm1;   /* Application specified parameter that needs 
                                   to be passed unchanged to the callback */
    int               iParm2;   /* Application specified parameter that needs 
                                   to be passed unchanged to the callback */
} BRAP_P_AppIntCbInfo;



/***************************************************************************
Summary:
	Clear all previous Raptor interrupts and Mask ALL raptor interrupts

Description:    
    This should be called in device open ie BRAP_Open(). It masks all raptor 
    interrupts making sure no interrupts come before the interrupt interface
    has been installed.

****************************************************************************/
BERR_Code BRAP_P_ClearInterrupts (
        BREG_Handle hRegister   /* [in] The register handle */
);


/***************************************************************************
Summary:
	Initialises interrupt handling in Raptor

Description:    
    This installs a BINT callback for the top level Audio L2 interrupt  
    for each Raptor channel (with Param1 as the raptor channel handle, 
    param2 is not used).

    This may differ across chip versions.

****************************************************************************/
BERR_Code BRAP_P_InterruptInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
);


/***************************************************************************
Summary:
	CLoses interrupt handling in Raptor

Description:    
    This may differ across chip versions.

****************************************************************************/
BERR_Code BRAP_P_InterruptUnInstall (
	BRAP_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
);

#if (BRAP_7550_FAMILY !=1)
/***************************************************************************
Summary:
	Initialises interrupt handling in Raptor

Description:    
    This installs a BINT callback for the top level Audio L2 interrupt  
    for each Raptor channel (with Param1 as the raptor destination handle, 
    param2 is not used).

    This may differ across chip versions.

****************************************************************************/
BERR_Code BRAP_P_DestinationLevelInterruptInstall(
	BRAP_DestinationHandle        hDstHandle    /* [in] Raptor Destination handle */
);


/***************************************************************************
Summary:
	CLoses interrupt handling in Raptor

Description:    
    This may differ across chip versions.

****************************************************************************/
BERR_Code BRAP_P_DestinationLevelInterruptUnInstall (
	BRAP_DestinationHandle        hDstHandle    /* [in] Raptor Destination handle */
);

void BRAP_P_DSP2Host_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
);
#endif

/***************************************************************************
Description:
	This API will mask the bit for the corrosponding task id. So that FW can 
	not raise the interrupt for this task any more.
Returns:
	BERR_SUCCESS - If successful
***************************************************************************/
BERR_Code   BRAP_P_MaskTask(
        BRAP_FWIF_P_FwTaskHandle hTask
);

/***************************************************************************
Description:
	This API will unmask the bit for the corrosponding task id. So that FW can 
	raise the interrupt for this task.
Returns:
	BERR_SUCCESS - If successful
***************************************************************************/
BERR_Code   BRAP_P_UnmaskTask(
        BRAP_FWIF_P_FwTaskHandle hTask
);

void BRAP_P_EnableRfAudioInterrupts (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);

void BRAP_P_DisableRfAudioInterrupts (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);

BERR_Code BRAP_P_DeviceLevelInterruptInstall (
	BRAP_Handle		hRap
);

BERR_Code BRAP_P_DeviceLevelInterruptUnInstall (
	BRAP_Handle		hRap
);

void BRAP_P_FmmRbufFreeWaterMark_isr (
        void * pParm1,  /* [in] Raptor channel handle */
        int    iParm2   /* [in] RBUF index */
);

void BRAP_P_RampEnableTimer_isr(
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] RBUF index */
);


void BRAP_P_FmmDestinationRbufFullMark_isr (
	void *pParm1,       /* [in] Raptor destination handle */
	int dummy           
);

void BRAP_P_FmmDestinationRbufFlowControl_isr (
	void *pParm1,       /* [in] Raptor destination handle */
	int dummy           
);


/***************************************************************************
Description:
	This API will send the command to FW to disbale all the asynchronous events 
	for any task. This api will be called in the start of the task so that we 
	can enable event as per required later.
Returns:
	BERR_SUCCESS - If successful
***************************************************************************/
BERR_Code BRAP_P_DisableAllEvent(
    BRAP_FWIF_P_FwTaskHandle hTask
);

/***************************************************************************
Description:
	This API installs the decoder interrupts.
Returns:
	BERR_Code 
See Also:
	BRAP_P_InterruptInstall
***************************************************************************/
BERR_Code BRAP_P_DecoderInterrupt(
    BRAP_FWIF_P_FwTaskHandle    hTask		/* [in] Task handle */
);

/***************************************************************************
Description:
	This API Installs the top level interrupt handlers for DSP ping command.
Returns:
	BERR_Code 
See Also:
	BRAP_P_AckUnInstall
***************************************************************************/
BERR_Code BRAP_P_AckInstall(
    BRAP_DSP_Handle         hDsp
);    

/***************************************************************************
Description:
	This API uninstalls the top level interrupt handlers for DSP ping command.
Returns:
	BERR_Code 
See Also:
	BRAP_P_AckInstall
***************************************************************************/
BERR_Code BRAP_P_AckUnInstall(
    BRAP_DSP_Handle         hDsp
);

/***************************************************************************
Description:
    This Isr function is called when sample rate associated with any Destination is programmed.
    
Returns:
	void 
***************************************************************************/

void BRAP_P_DestinationSampleRateChange_isr (
        void * pParm1, /* [in] Raptor destination handle */
        unsigned int    iParm2  
);

/***************************************************************************
Description:
	This API will be a helper function and will search the TASK in which FW
	is doing decoding.
Returns:
    void
***************************************************************************/
 void BRAP_P_GetTask(
        BRAP_ChannelHandle          hRapCh,
        BRAP_FWIF_P_FwTaskHandle    *hTask
);


#ifdef __cplusplus
}
#endif

#endif /* BRAP_INT_PRIV_H__ */
/* End of File */    

