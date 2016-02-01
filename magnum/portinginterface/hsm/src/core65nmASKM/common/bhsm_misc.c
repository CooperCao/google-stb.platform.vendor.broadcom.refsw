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



/* From Aegis */


#include "bhsm_misc.h"



BDBG_MODULE(BHSM);

#ifdef USER_MODE_BHSM_DEBUG
#define BDBG_MSG(x) printf x 
#define BDBG_ERR(x) printf x
#endif 



BERR_Code BHSM_ReadScArray (
		BHSM_Handle				in_handle,
		BHSM_ReadScArrayIO_t		*inoutp_readScArrayIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unParamLen = 0, i, j;
	BHSM_P_CommandData_t commandData;	

	BDBG_MSG(("Inside BHSM_ReadScArray\n"));
	BDBG_ENTER(BHSM_ReadScArray);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	


	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));

	commandData.cmdId = BCMD_cmdType_eUSER_SC_ARRAY_STATUS;
	commandData.unContMode = 0;
	
	/* only fill the command specific input parameters */
	/* No input command parameter */

	
	BHSM_P_CHECK_ERR_CODE_FUNC(errCode, 
			BHSM_P_CommonSubmitCommand (channelHandle, 
					&commandData));

	/* Parse the command specific output parameters */
	inoutp_readScArrayIO->unStatus =  commandData.unOutputParamsBuf[0];
	unParamLen +=  4;

	/* DigitalTV has 4 word32 returned,*/
	for (i=0, j=1; i< 16; i++, j++) {
		inoutp_readScArrayIO->unScArrayStatus[i] =  
				commandData.unOutputParamsBuf[j];
		unParamLen +=  4;
	}

 	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inoutp_readScArrayIO->unStatus != 0) );
	
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_ReadScArray);
	return( errCode );
}




BERR_Code BHSM_SetMiscBits (
		BHSM_Handle			in_handle,
		BHSM_SetMiscBitsIO_t		*inoutp_setMiscBitsIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unParamLen = 0;
	BHSM_P_CommandData_t commandData;	

	BDBG_MSG(("Inside BHSM_SetMiscBits\n"));
	BDBG_ENTER(BHSM_SetMiscBits);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));

	commandData.cmdId = BCMD_cmdType_eOFFLINE_RAVE_COMMAND;
	commandData.unContMode = 0;
	
	/* only fill the command specific input parameters */

	commandData.unInputParamsBuf[unParamLen/4] =  inoutp_setMiscBitsIO->setMiscBitsSubCmd;
	unParamLen += 4;

	switch (inoutp_setMiscBitsIO->setMiscBitsSubCmd){
		
		case BCMD_SetMiscBitsSubCmd_eRaveBits:
			
			commandData.unInputParamsBuf[unParamLen/4] =
	      		  		(inoutp_setMiscBitsIO->bDisableClear << 4) |
			        (inoutp_setMiscBitsIO->bEnableEncBypass << 3) |
	      			  (inoutp_setMiscBitsIO->bEnableReadDMem << 2) |
					(inoutp_setMiscBitsIO->bEnableReadIMem << 1) |
					inoutp_setMiscBitsIO->bEnableWriteIMem;
			break;
			
		case 1: 		/*BCMD_SetMiscBitsSubCmd_eReserved1: eliminate the purge failure, will put it back later when the purge is stable 8/3/07 */
			commandData.unInputParamsBuf[unParamLen/4] =
	      			  ( (inoutp_setMiscBitsIO->bSetMiscReservedBit2 & 0x1)<< 2) |
					( (inoutp_setMiscBitsIO->bSetMiscReservedBit1 & 0x1) << 1) |
					(inoutp_setMiscBitsIO->bSetMiscReservedBit0 & 0x1);
			break;

		case 2:		/* BCMD_SetMiscBitsSubCmd_eReserved2:  */
			commandData.unInputParamsBuf[unParamLen/4] =
	      			  ( (inoutp_setMiscBitsIO->bSetMiscReservedBit2 & 0x1)<< 2) |
					( (inoutp_setMiscBitsIO->bSetMiscReservedBit1 & 0x1) << 1) |
					(inoutp_setMiscBitsIO->bSetMiscReservedBit0 & 0x1);
			break;
		
		default:
			errCode = BERR_TRACE((BHSM_STATUS_INPUT_PARM_ERR));		
			goto BHSM_P_DONE_LABEL;							



	}
	
			
    unParamLen += 4;
			
	commandData.unInputParamLen = unParamLen;
	
	BHSM_P_CHECK_ERR_CODE_FUNC(errCode, 
			BHSM_P_CommonSubmitCommand (channelHandle, 
					&commandData));

	unParamLen = 0;  
	
	inoutp_setMiscBitsIO->unStatus =  commandData.unOutputParamsBuf[unParamLen/4]; 
	unParamLen += 4;

	/* Check status */
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(0 != inoutp_setMiscBitsIO->unStatus  ) );			

	inoutp_setMiscBitsIO->ucRaveStatus =  commandData.unOutputParamsBuf[unParamLen/4];
	unParamLen +=  4;

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_SetMiscBits);
	return( errCode );
}


BERR_Code BHSM_SetArch (
		BHSM_Handle			in_handle,
		BHSM_SetArchIO_t	*inoutp_setArchIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unParamLen = 0;
	BHSM_P_CommandData_t commandData;	

	BDBG_MSG(("Inside BHSM_SetArch\n"));
	BDBG_ENTER(BHSM_SetArch);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	


	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));

	commandData.cmdId = BCMD_cmdType_eOFFLINE_ARCH_COMMAND;
	commandData.unContMode = 0;
	
	/* only fill the command specific input parameters */
	commandData.unInputParamsBuf[unParamLen/4] = 
			inoutp_setArchIO->unLowerRangeAddress;
    unParamLen += 4;

	commandData.unInputParamsBuf[unParamLen/4] = 
			inoutp_setArchIO->unUpperRangeAddress;
    unParamLen += 4;    

	commandData.unInputParamsBuf[unParamLen/4] = 
				inoutp_setArchIO->ArchSel         |
				(inoutp_setArchIO->PCIArch <<  8) |
				(inoutp_setArchIO->DRAMSel << 16);
		unParamLen += 4;	
	
	commandData.unInputParamLen = unParamLen;    
	
	BHSM_P_CHECK_ERR_CODE_FUNC(errCode, 
			BHSM_P_CommonSubmitCommand (channelHandle, 
					&commandData));

    unParamLen =0;    
					
	/* Parse the command specific output parameters */
	inoutp_setArchIO->unStatus =  commandData.unOutputParamsBuf[unParamLen/4];
	unParamLen +=  4;

 	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inoutp_setArchIO->unStatus != 0) );
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_SetArch);
	return( errCode );

}
