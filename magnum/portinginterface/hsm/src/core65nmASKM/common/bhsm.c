/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bint.h"
#include "bchp_int_id_bsp.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"

#include "bhsm.h"



BDBG_MODULE(BHSM);

#ifdef USER_MODE_BHSM_DEBUG
#define BDBG_MSG(x) printf x 
#define BDBG_ERR(x) printf x
#endif 

/*******************************************************************************
*	Default Module and Channel Settings.  Note that we could only modify 
*	Module settings during BHSM_Open.
*******************************************************************************/
static const BHSM_Settings BHSM_defHsmSettings =
{
	2, /* max channels */
	2000, /* 2000 milli seconds timeout */
	NULL,   /* exception Callback function */
         /* default to ISR;  TODO: default to polling needs a BHSM_SetSettings() function call first*/
	0		
#if (BHSM_IPTV ==1) 
	,NULL
#endif
};

static const BHSM_ChannelSettings BHSM_defHsmChannelSettings =
{
	2
};


/*******************************************************************************
*	Public Module Functions
*******************************************************************************/
BERR_Code BHSM_GetDefaultSettings(
		BHSM_Settings		*outp_sSettings,
		BCHP_Handle		in_chipHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BHSM_GetDefaultSettings);
	BDBG_ASSERT( in_chipHandle );	
	BSTD_UNUSED(in_chipHandle);  /* Removed this when we start using this variable */

	*outp_sSettings = BHSM_defHsmSettings;

	BDBG_LEAVE(BHSM_GetDefaultSettings);
	return( errCode );
}


BERR_Code BHSM_SetSettings(
		BHSM_Handle				   hHsm,
	  	BHSM_NewSettings_t		* newSettings

)
{
	BERR_Code 		errCode = BERR_SUCCESS;	
	BDBG_MSG(("Inside BHSM_SetSettings\n"));
	BDBG_ENTER(BHSM_SetSettings);

	if (hHsm == NULL) {	return (BHSM_STATUS_INPUT_PARM_ERR);	}
	
	/*
		BHSM_defHsmSettings.uSpecialControl = ((BHSM_defHsmSettings.uSpecialControl >> leftPartShift) << leftPartShift)|
							((BHSM_defHsmSettings.uSpecialControl << rightPartShift) >> rightPartShift)|
							(ctrlValue << whichControl);
	*/

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR, 
						(newSettings->whichControl & (~BHSM_SPECIALCTROL_FLAGS))  != 0);

	if (((newSettings->whichControl & BHSM_CTRLS_TIMEOUT) == BHSM_CTRLS_TIMEOUT) && 
		 (newSettings->timeoutMs > BHSM_SPECIALCTROL_TIMEOUT_MAX) )
	{
		BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR, 1);
	}

	/* legal setting here */

	if ((newSettings->whichControl & BHSM_CTRLS_POLLINGORISR) == BHSM_CTRLS_POLLINGORISR)
	{
		hHsm->currentSettings.uSpecialControl = (newSettings->ctrlValue & 0x1) | (hHsm->currentSettings.uSpecialControl & (~0x1));
		hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->currentSettings.uSpecialControl = hHsm->currentSettings.uSpecialControl;
		BDBG_MSG(("BHSM_CTRLS_POLLINGORISR is %x\n",hHsm->currentSettings.uSpecialControl ));

		if ( (newSettings->ctrlValue & 0x1) ==1 )
		{
			BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DisableCallback(hHsm->IntCallback2) );

#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
			BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DisableCallback(hHsm->IntCallback) );
#endif
			
			BREG_Write32(hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN, 
							BREG_Read32(  hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle, 
										BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN) & 
										(~BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK));	 

			/* this clear may be optional, just to be safer*/
		       BREG_Write32(  hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle,
						BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
						BREG_Read32(  hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle, 
										BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) & 
										(~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));		

			BDBG_MSG(("//////////BHSM_CTRLS_POLLING to disable ISR2, %x, channel num=%d\n",
					hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->currentSettings.uSpecialControl,
					hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->ucChannelNumber));


		}else
		{
			BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_EnableCallback(hHsm->IntCallback2) );

#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
			BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_EnableCallback(hHsm->IntCallback) );	
#endif

			BREG_Write32(hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN, 
							BREG_Read32(  hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->regHandle, 	BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN) | 
										BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK);	 
			BDBG_MSG(("//////////BHSM_CTRLS_POLLING to enable ISR2, %x, channel num=%d\n",
					hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->currentSettings.uSpecialControl,
					hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->ucChannelNumber));

		}

	}
	
	
	if ((newSettings->whichControl & BHSM_CTRLS_TIMEOUT) == BHSM_CTRLS_TIMEOUT)
	{
		hHsm->currentSettings.ulTimeOutInMilSecs = newSettings->timeoutMs;
		hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->currentSettings.ulTimeOutInMilSecs =hHsm->currentSettings.ulTimeOutInMilSecs;
		BDBG_MSG(("BHSM_CTRLS_TIMEOUT is %x\n",hHsm->currentSettings.ulTimeOutInMilSecs ));
	}

	/* for 3563, this is not really used
	if ((newSettings->whichControl & BHSM_CTRLS_CHANNELNUM) == BHSM_CTRLS_CHANNELNUM)
	{
		hHsm->currentSettings.ucMaxChannels = newSettings->maxChannelNum;
		hHsm->channelHandles[BHSM_HwModule_eCmdInterface2]->moduleHandle->currentSettings.ulTimeOutInMilSecs =hHsm->currentSettings.ucMaxChannels ;
		BDBG_MSG(("BHSM_CTRLS_CHANNELNUM is %x\n",hHsm->currentSettings.ucMaxChannels  ));
	}   */

	
BHSM_P_DONE_LABEL:	
	
	BDBG_LEAVE(BHSM_SetSettings);
	BDBG_MSG(("Exit BHSM_SetSettings\n"));
	return( errCode );
}



BERR_Code BHSM_Open(
		BHSM_Handle			*outp_handle, 
		BREG_Handle			in_regHandle,     
		BCHP_Handle			in_chipHandle,
		BINT_Handle			in_interruptHandle,
		const BHSM_Settings	*inp_sSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
 	BHSM_Handle moduleHandle;
	unsigned int channelNum, i, j;

	BDBG_ENTER(BHSM_Open);
	BDBG_ASSERT( in_chipHandle );
	BDBG_ASSERT( in_regHandle );
	BDBG_ASSERT( in_interruptHandle );


	*outp_handle = NULL;

	/* Alloc memory from the system heap */
	if ((moduleHandle = 
				(BHSM_Handle) BKNI_Malloc( sizeof( BHSM_P_Handle))) 
				== NULL) {
		/* wrap initially detected error code */
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BHSM_P_DONE_LABEL;
	}

	BKNI_Memset(moduleHandle, 0, sizeof( BHSM_P_Handle ));
	
	moduleHandle->ulMagicNumber = BHSM_P_HANDLE_MAGIC_NUMBER;
	moduleHandle->chipHandle = in_chipHandle;
	moduleHandle->regHandle = in_regHandle;
	moduleHandle->interruptHandle = in_interruptHandle;

#if  (BHSM_IPTV ==1)	
	if ( inp_sSettings->hHeap  != NULL )
	{
		moduleHandle->hHeap = inp_sSettings->hHeap;		
		moduleHandle->pContiguousMem= BMEM_AllocAligned( inp_sSettings->hHeap, BHSM_CONTIGUOUS_MEMORY_SIZE, 6,0 ); /* 64 bytes allignement*/
		BDBG_MSG(("(%08X bytes continuous memeory inside HSM) pContiguousMem = %p\n", BHSM_CONTIGUOUS_MEMORY_SIZE, moduleHandle->pContiguousMem));
		BDBG_ASSERT( moduleHandle->pContiguousMem != NULL );
	}
#endif

	BKNI_CreateMutex( &(moduleHandle->caKeySlotMutexLock));
	BKNI_CreateMutex( &(moduleHandle->m2mKeySlotMutexLock));
	

	if (inp_sSettings == NULL)
		moduleHandle->currentSettings = BHSM_defHsmSettings;
	else {
		moduleHandle->currentSettings = *inp_sSettings;
	}	

	BHSM_SetIntrCallback(	moduleHandle, BHSM_IntrType_eException, BHSM_P_ExceptionInterruptCB_isr);

	BDBG_MSG(("Inside BHSM_Open: Before BINT_CreateCallback \n"));
	/*  Enable interrupt   */	

#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_CreateCallback(
			&(moduleHandle->IntCallback), moduleHandle->interruptHandle, 
			BCHP_INT_ID_BSP_OLOAD1_INTR,
			BHSM_P_IntHandler_isr, (void *) moduleHandle, 0x00 ) );	
	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_EnableCallback(moduleHandle->IntCallback) );
#endif

	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_CreateCallback(
			&(moduleHandle->IntCallback2), moduleHandle->interruptHandle, 
			BCHP_INT_ID_BSP_OLOAD2_INTR,
			BHSM_P_IntHandler_isr, (void *) moduleHandle, 0x01 ) );		

	BDBG_MSG(("Inside BHSM_Open: Before BINT_EnableCallback2 \n"));			
	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_EnableCallback(moduleHandle->IntCallback2) );

#if (BCHP_CHIP==7401) &&  (BCHP_VER == BCHP_VER_A0)
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_NONSECURE_GLB_HOST_INTR_EN, 
		0);
	/*		BCHP_BSP_GLB_NONSECURE_GLB_HOST_INTR_EN_HOST_INTR_STATUS_EN_MASK);	 */
#else
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN, 
		0);
	/*		BCHP_BSP_GLB_NONSECURE_GLB_HOST_INTR_EN_HOST_INTR_STATUS_EN_MASK);	 */
#endif

	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS, 0);
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_OLOAD2, 0);
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_ILOAD2, 0);
#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_OLOAD1, 0);
	BREG_Write32( moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_ILOAD1, 0);
#endif

	/* Get the chip information */
	BCHP_GetChipInfo( moduleHandle->chipHandle, &moduleHandle->chipId, &moduleHandle->chipRev );
	BDBG_MSG(( "Chip Information \n" ));
	BDBG_MSG(( "chipId=0x%x, chipRev=0x%x\n", moduleHandle->chipId, moduleHandle->chipRev ));

   	/* 
		If inp_sSettings->maxChannels == 0, set it to 
		BHSM_MAX_SUPPOTED_CHANNELS 
	*/
	if (moduleHandle->currentSettings.ucMaxChannels == 0)
		moduleHandle->currentSettings.ucMaxChannels = 
							BHSM_MAX_SUPPOTED_CHANNELS;
		
	for( channelNum = 0; 
		channelNum < moduleHandle->currentSettings.ucMaxChannels; 
		channelNum++ )	{
			
		moduleHandle->channelHandles[channelNum] = NULL;
	}


	/* Initialize members */
	for (i=0; i<BCMD_TOTAL_PIDCHANNELS*2; i++) {

		moduleHandle->aunCAKeySlotInfo[i].keySlotType = 
			BCMD_XptSecKeySlot_eTypeMax;	
		moduleHandle->aunCAKeySlotInfo[i].bIsUsed = 
					false;	

	}

	/* Initialize PidChannelToKeySlotNum matrices */
	for (i=0; i<BCMD_TOTAL_PIDCHANNELS; i++) {
		for (j=0; j < 2; j++) {
			moduleHandle->aPidChannelToCAKeySlotNum[i][j].keySlotType = 
				BCMD_XptSecKeySlot_eTypeMax;	
			moduleHandle->aPidChannelToCAKeySlotNum[i][j].unKeySlotNum = 
						BHSM_SLOT_NUM_INIT_VAL;	
		}
	}


	/* Initialize M2M Key Slot info matrices */
	for (i=0; i<BCMD_MAX_M2M_KEY_SLOT; i++) {

		moduleHandle->aunM2MKeySlotInfo[i].bIsUsed = 	false;	

	}	
	
	/* End of Initialize members */	

	*outp_handle = moduleHandle;

	moduleHandle->bIsOpen = true;
	
BHSM_P_DONE_LABEL:	     

	if( errCode != BERR_SUCCESS )
	{
		if( moduleHandle != NULL )
		{
			BKNI_Free( moduleHandle );

		}
	}	
	BDBG_LEAVE(BHSM_Open);
	BDBG_MSG(("Exit BHSM_Open\n"));	
	return( errCode );

}

BERR_Code BHSM_Close(
		BHSM_Handle inout_handle
)
{
	BERR_Code errCode = BERR_SUCCESS;


	BDBG_ENTER(BHSM_Close);
	BDBG_ASSERT( inout_handle );	

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, (inout_handle ==  NULL) );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inout_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inout_handle->bIsOpen ==  false) );

#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)	
	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DisableCallback( inout_handle->IntCallback));
#endif

	BHSM_P_CHECK_ERR_CODE_FUNC( errCode, BINT_DestroyCallback( inout_handle->IntCallback2 ) );
	

BHSM_P_DONE_LABEL:	
	BKNI_DestroyMutex( inout_handle->caKeySlotMutexLock);
	BKNI_DestroyMutex( inout_handle->m2mKeySlotMutexLock);
	
	inout_handle->bIsOpen = false;
	BKNI_Free(  inout_handle );	
	inout_handle = NULL;

	BDBG_LEAVE(BHSM_Close);
	return( errCode );
}

BERR_Code BHSM_GetTotalChannels(
		BHSM_Handle		in_handle, 
		unsigned char		*outp_ucTotalChannels
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BHSM_GetTotalChannels);
	BDBG_ASSERT( in_handle );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	*outp_ucTotalChannels = in_handle->currentSettings.ucMaxChannels;

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_GetTotalChannels);
	return( errCode );
}


BERR_Code BHSM_GetChannelDefaultSettings(
		BHSM_Handle				in_handle, 
		BHSM_HwModule			in_channelNo, 
		BHSM_ChannelSettings		*outp_sSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;


	BDBG_ENTER(BHSM_GetChannelDefaultSettings);
	BDBG_ASSERT( in_handle );
	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER, 
		(in_channelNo >= in_handle->currentSettings.ucMaxChannels) );

	*outp_sSettings = BHSM_defHsmChannelSettings;
	
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_GetChannelDefaultSettings);
	return( errCode );
}

BERR_Code BHSM_Channel_Open(
		BHSM_Handle					in_handle, 
		BHSM_ChannelHandle			*outp_channelHandle, 
		BHSM_HwModule				in_channelNo, 
		const BHSM_ChannelSettings	*inp_channelDefSettings 
)
{
	BERR_Code errCode = BERR_SUCCESS;
 	BHSM_ChannelHandle channelHandle = NULL;	

	BDBG_MSG(("Inside BHSM_Channel_Open\n"));
	BDBG_ENTER(BHSM_Channel_Open);
	BDBG_ASSERT( in_handle );
	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER, 
		(in_channelNo >= in_handle->currentSettings.ucMaxChannels) );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->bIsOpen ==  false) );

	/* channel handle must be NULL.  */
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->channelHandles[in_channelNo]  != NULL) );

	*outp_channelHandle = NULL;
	
	/* Alloc memory from the system heap */
	if ((channelHandle = 
			(BHSM_ChannelHandle) BKNI_Malloc(sizeof(BHSM_P_ChannelHandle))) 
			== NULL) {
		/* wrap initially detected error code */
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);		
		goto BHSM_P_DONE_LABEL;
	}	

	BKNI_Memset(channelHandle, 0, sizeof( BHSM_P_ChannelHandle ));

	channelHandle->ulMagicNumber = BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER;
	channelHandle->moduleHandle = in_handle;

	if (inp_channelDefSettings == NULL)
		channelHandle->currentChannelSettings.ucUnknown = 
		    BHSM_defHsmChannelSettings.ucUnknown;
	else {
		channelHandle->currentChannelSettings.ucUnknown = 
		    inp_channelDefSettings->ucUnknown;
	}

	BHSM_P_CHECK_ERR_CODE_FUNC(errCode,  BKNI_CreateMutex( &(channelHandle->mutexLock)));	
	
	BHSM_P_CHECK_ERR_CODE_FUNC(errCode,  BKNI_CreateEvent( &(channelHandle->oLoadWait)));	

	channelHandle->oLoadSet = 0;				
	
	BDBG_MSG(("in_channelNo = %d\n", in_channelNo));

	switch(in_channelNo) {
		case BHSM_HwModule_eCmdInterface1:
			channelHandle->ulInCmdBufAddr = BHSM_IN_BUF1_ADDR;
			channelHandle->ulOutCmdBufAddr = BHSM_OUT_BUF1_ADDR;
#if (BCHP_CHIP==7401) &&  (BCHP_VER == BCHP_VER_A0)			
			channelHandle->ulILoadRegAddr = BCHP_BSP_GLB_NONSECURE_GLB_ILOAD1;
			channelHandle->ulILoadVal = BCHP_BSP_GLB_NONSECURE_GLB_ILOAD1_CMD_ILOAD1_MASK;
			channelHandle->ulIReadyRegAddr = BCHP_BSP_GLB_NONSECURE_GLB_IRDY;
			channelHandle->ulIReadyVal= BCHP_BSP_GLB_NONSECURE_GLB_IRDY_CMD_IDRY1_MASK;			
#else
			channelHandle->ulILoadRegAddr = BCHP_BSP_GLB_CONTROL_GLB_ILOAD1;
			channelHandle->ulILoadVal = BCHP_BSP_GLB_CONTROL_GLB_ILOAD1_CMD_ILOAD1_MASK;
			channelHandle->ulIReadyRegAddr = BCHP_BSP_GLB_CONTROL_GLB_IRDY;
			channelHandle->ulIReadyVal= BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY1_MASK;			

#endif

			break;

		case BHSM_HwModule_eCmdInterface2:
			channelHandle->ulInCmdBufAddr = BHSM_IN_BUF2_ADDR;		
			channelHandle->ulOutCmdBufAddr = BHSM_OUT_BUF2_ADDR;	
#if (BCHP_CHIP==7401) &&  (BCHP_VER == BCHP_VER_A0)			
			channelHandle->ulILoadRegAddr = BCHP_BSP_GLB_NONSECURE_GLB_ILOAD2;			
			channelHandle->ulILoadVal = BCHP_BSP_GLB_NONSECURE_GLB_ILOAD2_CMD_ILOAD2_MASK;
			channelHandle->ulIReadyRegAddr = BCHP_BSP_GLB_NONSECURE_GLB_IRDY;
			channelHandle->ulIReadyVal= BCHP_BSP_GLB_NONSECURE_GLB_IRDY_CMD_IDRY2_MASK;
#else
			channelHandle->ulILoadRegAddr = BCHP_BSP_GLB_CONTROL_GLB_ILOAD2;			
			channelHandle->ulILoadVal = BCHP_BSP_GLB_CONTROL_GLB_ILOAD2_CMD_ILOAD2_MASK;
			channelHandle->ulIReadyRegAddr = BCHP_BSP_GLB_CONTROL_GLB_IRDY;
			channelHandle->ulIReadyVal= BCHP_BSP_GLB_CONTROL_GLB_IRDY_CMD_IDRY2_MASK;

#endif

			break;

		default:
			errCode = BERR_INVALID_PARAMETER;
			goto BHSM_P_DONE_LABEL;		
	}
	
	BDBG_MSG(("ulInCmdBufAddr = 0x%lx, ulOutCmdBufAddr = 0x%lx \n", 
			channelHandle->ulInCmdBufAddr, channelHandle->ulOutCmdBufAddr));

	channelHandle->ucChannelNumber =  in_channelNo;
	in_handle->channelHandles[in_channelNo] = channelHandle;

	*outp_channelHandle = channelHandle;
	channelHandle->bIsOpen = true;	

BHSM_P_DONE_LABEL:
	if( errCode != BERR_SUCCESS )
	{
		if( channelHandle != NULL )
		{
			BKNI_Free( channelHandle );
		}
	}

	BDBG_LEAVE(BHSM_Channel_Open);
	BDBG_MSG(("Exit BHSM_Channel_Open\n"));	
	return( errCode );
}

BERR_Code BHSM_Channel_Close(
		BHSM_ChannelHandle inout_channelHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_Handle moduleHandle;

	BDBG_ENTER(BHSM_Channel_Close);
	BDBG_MSG(("Enter BHSM_Channel_Close\n"));
	BDBG_ASSERT( inout_channelHandle );
	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inout_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );
	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inout_channelHandle->bIsOpen ==  false) );	

   
BHSM_P_DONE_LABEL:

	BKNI_DestroyMutex( inout_channelHandle->mutexLock);

	BKNI_DestroyEvent( inout_channelHandle->oLoadWait);		
	
	inout_channelHandle->bIsOpen = false;
	
	moduleHandle = inout_channelHandle->moduleHandle;	
	moduleHandle->channelHandles[inout_channelHandle->ucChannelNumber] = NULL;	
	
	BKNI_Free( inout_channelHandle );	
	inout_channelHandle = NULL;

	BDBG_MSG(("Leave BHSM_Channel_Close\n"));
	BDBG_LEAVE(BHSM_Channel_Close);
	return( errCode );
}

BERR_Code BHSM_Channel_GetDevice(
		BHSM_ChannelHandle	in_channelHandle, 
		BHSM_Handle			*outp_handle 
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BHSM_Channel_GetDevice);
	BDBG_ASSERT( in_channelHandle );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_channelHandle->moduleHandle->bIsOpen ==  false) );	

	*outp_handle = NULL;
	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

	*outp_handle = in_channelHandle->moduleHandle;

BHSM_P_DONE_LABEL:
	BDBG_LEAVE(BHSM_Channel_GetDevice);
	return( errCode );
}


BERR_Code BHSM_GetChannel(
		BHSM_Handle			in_handle, 
		BHSM_HwModule		in_channelNo, 
		BHSM_ChannelHandle	*outp_channelHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
 	BHSM_ChannelHandle channelHandle = NULL;

	BDBG_ENTER(BHSM_GetChannel);
	BDBG_ASSERT( in_handle );

	*outp_channelHandle = NULL;	
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER, 
		(in_channelNo >= in_handle->currentSettings.ucMaxChannels) );
	
	channelHandle = in_handle->channelHandles[in_channelNo];

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(channelHandle == NULL ) );

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );	
	

	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(channelHandle->bIsOpen ==  false) );	

	*outp_channelHandle = channelHandle;
	
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_GetChannel);
	return( errCode );
}


BERR_Code BHSM_SetIntrCallback(
		BHSM_Handle			in_handle,
		BHSM_IntrType		in_eIntType,
		BHSM_IsrCallbackFunc 	in_callback	
)
{
	BERR_Code errCode = BERR_SUCCESS;	

	BDBG_ASSERT( in_handle );
	BDBG_ASSERT( in_callback );
											   
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BERR_INVALID_PARAMETER, 
		(in_eIntType >= BHSM_IntrType_eMax ) );
		
	switch (in_eIntType) {

		case BHSM_IntrType_eException:
			BKNI_EnterCriticalSection();
			/* Assign the callback into the interrupt table. */
			in_handle->currentSettings.exceptionCBfunc	= in_callback;
			BKNI_LeaveCriticalSection();	
			break;
			
		default:
			BDBG_ERR(( "callback for in_eIntType %lu is not supported!", in_eIntType ));
			errCode = BERR_TRACE( BERR_INVALID_PARAMETER );

	}


	/*BDBG_MSG((", final ulVal = 0x%x\n ", ulVal)); */
	
BHSM_P_DONE_LABEL:

	BDBG_LEAVE(BHSM_Channel_EnableIntrCallback);
	return( errCode );

}



#if 0
BERR_Code BHSM_Channel_DisableIntrCallback(
		BHSM_Handle			in_handle,
		BHSM_IntrType    		in_eIntType
)
{
	BERR_Code errCode = BERR_SUCCESS;	
	
	BDBG_ASSERT( in_handle );

	BKNI_EnterCriticalSection();	
	BREG_Write32( in_handle->regHandle, BCHP_BSP_NONSECURE_INTR2_CPU_MASK_SET, 
		 				(1ul << in_eIntType));  
	BKNI_LeaveCriticalSection();		

	/* BDBG_MSG((", final ulVal = 0x%x\n ", ulVal)); */
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_Channel_DisableIntrCallback);
	return( errCode );
}
#endif

#ifdef BHSM_AUTO_TEST

BERR_Code BHSM_SubmitRawCommand (
		BHSM_Handle 		in_handle,
		BHSM_HwModule	in_interface,
		uint32_t			in_unInputParamLenInWord,  /* length in words, including header sections words*/
		uint32_t			*inp_unInputParamsBuf,    /* entire cmd, incluidng header section words*/
		uint32_t			*outp_unOutputParamLenInWord,  /* length in words, including header sections words*/
		uint32_t			*outp_unOutputParamsBuf        /* entire cmd, incluidng header section words*/
)
{
	BERR_Code 		errCode = BERR_SUCCESS;
	BHSM_P_CommandData_t 	commandData;
	unsigned int 		i;
	BHSM_ChannelHandle 	channelHandle = in_handle->channelHandles[in_interface];
	
	BDBG_MSG(("Inside BHSM_SubmitRawCommand\n"));	

	BKNI_Memset(&commandData, 0, sizeof( BHSM_P_CommandData_t ));
	
	commandData.unInputParamLen = in_unInputParamLenInWord*4;	/* passed-in param in words*/

	for (i=0; i<commandData.unInputParamLen/4; i++) {
		commandData.unInputParamsBuf[i] = inp_unInputParamsBuf[i];		/* including header section words*/
	}

	BHSM_P_CHECK_ERR_CODE_FUNC(errCode, 
		BHSM_P_CommonSubmitRawCommand(channelHandle, &commandData));

	*outp_unOutputParamLenInWord = (commandData.unOutputParamLen+3)/4;   /* to soapApp, translated in words*/
	
	for(i = 0; i<(commandData.unOutputParamLen+3)/4; i++) {
		outp_unOutputParamsBuf[i] = commandData.unOutputParamsBuf[i];
	}	

BHSM_P_DONE_LABEL:
	BDBG_MSG(("Exit BHSM_SubmitRawCommand\n"));		
	return errCode;
	
}

#endif
