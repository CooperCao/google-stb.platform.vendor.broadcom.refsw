/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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


#ifndef BHDR_FE_PRIV_H__
#define BHDR_FE_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"

#include "bchp_hdmi_rx_fe_0.h"
#include "bchp_dvp_hr_intr2.h"
#include "bchp_int_id_dvp_hr_intr2.h"

#include "bhdm_cec_priv.h" 


#define	MAKE_INTR_FE_ENUM(IntName)	BHDR_FE_INTR_e##IntName
#define	MAKE_INTR_FE_NAME(IntName)	"BHDR_FE_" #IntName

#define	MAKE_INTR_FE_CHN_ENUM(IntName)	BHDR_FE_CHN_INTR_e##IntName
#define	MAKE_INTR_FE_CHN_NAME(IntName)	"BHDR_FE_CHN_" #IntName




typedef enum BHDR_FE_P_Channel
{
	BHDR_FE_P_eChannel0 = 0,
	BHDR_FE_P_eChannel1,
	BHDR_FE_P_eChannelMax
} BHDR_FE_P_Channel ;



/******************************************************************************
Summary:
Enumeration of BHDR_FE_Interrupts 
*******************************************************************************/
typedef enum
{
	 /* 15 */ MAKE_INTR_FE_ENUM(DUPLICATE_FE_SELECT),
	 /* 16 */ MAKE_INTR_FE_ENUM(MUX_0_IMPROPER_FE_SELECT_UPDATE), 
	 /* 17 */ MAKE_INTR_FE_ENUM(MUX_1_IMPROPER_FE_SELECT_UPDATE),
	 
	/*  */ MAKE_INTR_FE_ENUM(LAST)
	 
} BHDR_FE_P_InterruptMask ;



/******************************************************************************
Summary:
Enumeration of BHDR_FE_ChnInterrupts 
*******************************************************************************/
typedef enum
{
	 /* 00 */ MAKE_INTR_FE_CHN_ENUM(RX_HOTPLUG_UPDATE),
	 /* 01 */ MAKE_INTR_FE_CHN_ENUM(CEC_LOW),
	 /* 02 */ MAKE_INTR_FE_CHN_ENUM(CEC),
	 /* 03 */ MAKE_INTR_FE_CHN_ENUM(PLL_LOCK),   
	 /* 04 */ MAKE_INTR_FE_CHN_ENUM(FREQ_CHANGE),   


	/*  */ MAKE_INTR_FE_CHN_ENUM(LAST)
	 
} BHDR_FE_P_ChnInterruptMask ;




/* Get the offset of two groups of register. */
/* HDMI TODO Fix for >2 FrontEnds */
#define BHDR_FE_P_GET_REG_OFFSET(ChannelId) \
	((BHDR_FE_P_eChannel0 == (ChannelId)) ? 0 : (BCHP_HDMI_RX_FE_1_REG_START - BCHP_HDMI_RX_FE_0_REG_START))



/*******************************************************************************
Private HDMI Rx Frontend Channel Handle Declaration 
*******************************************************************************/
typedef struct BHDR_FE_P_ChannelHandle
{
	BINT_Handle   hInterrupt ;
	BREG_Handle hRegister ;
	
	BHDR_FE_P_Channel    eChannel  ;
	uint32_t                       ulOffset ;

	BHDR_FE_ChannelSettings settings ;
	
	bool bTxDeviceAttached ;
	bool bPreviousTxDeviceAttached ;
	bool bPllLocked ;

	BHDR_Handle hHDR ;
	uint8_t uiHdrSel ;
	uint32_t ulHdrOffset ;

	BINT_CallbackHandle hCallback[MAKE_INTR_FE_CHN_ENUM(LAST)] ;
	
	BHDR_FE_CallbackFunc pfHotPlugCallback_isr ;
	void                       *pvHotPlugParm1;
	int                         iHotPlugParm2;
	
	BHDR_FE_CallbackFunc pfCecCallback_isr ;
	void                       *pvCecParm1;
	int                         iCecParm2;
	
	uint32_t PreviousPixelClockCount	;
	bool PreviousbPllLocked ;
	uint32_t EstimatedPixelClockRate ;

	/******************/
	/* CEC variables  */
	/******************/
	BCEC_CONFIG cecConfiguration ;

	BHDR_FE_ChannelPowerSettings stPowerSettings ;

} BHDR_FE_P_ChannelHandle ;



/*******************************************************************************
Private FrontEnd Handle Declaration (encompasses all Front End channels)
*******************************************************************************/
typedef struct BHDR_FE_P_Handle
{
	BINT_Handle   hInterrupt ;
	BREG_Handle hRegister ;
	BCHP_Handle hChip ;
	
	uint32_t                       ulOffset ;

	BHDR_FE_Settings DeviceSettings ;
	
	BINT_CallbackHandle hCallback[MAKE_INTR_FE_ENUM(LAST)] ;

	BKNI_EventHandle BHDR_FE_Event_DUPLICATE_FE_SELECT ; 
	BKNI_EventHandle BHDR_FE_Event_MUX_0_IMPROPER_FE_SELECT_UPDATE ;
	BKNI_EventHandle BHDR_FE_Event_MUX_1_IMPROPER_FE_SELECT_UPDATE ;

} BHDR_FE_P_Handle ;



/******************************************************************************
Summary:
Handle interrupts from the HDMIRx core.

Description:
Interrupts received from the HDMIRx core must be handled.  The following 
is a list of possible interrupts.


Input:
	pParameter - pointer to interrupt specific information BHDR_Open.

Output:
	<None>
	
Returns:
	<None>

See Also:

*******************************************************************************/
void BHDR_FE_P_FrontEndIntr_isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
) ;
								

void BHDR_FE_P_ChannelIntr_isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
) ;
								

void BHDR_FE_P_GetPllLockStatus_isr(BHDR_FE_ChannelHandle hFeChannel, 
	bool *bLocked) ;

BERR_Code BHDR_FE_P_GetPixelClockEstimate_isr(BHDR_FE_ChannelHandle hFeChannel, 
	uint32_t *EstimatedPixelClockRate) ;

 void BHDR_FE_P_CEC_isr(BHDR_FE_ChannelHandle hFeChannel)  ;
void BHDR_FE_P_EnableInterrupts_isr(BHDR_FE_ChannelHandle hFeChannel, bool enable) ;

	
#ifdef __cplusplus
}
#endif
 
#endif
/* end bhdr_fe_ priv.h */

