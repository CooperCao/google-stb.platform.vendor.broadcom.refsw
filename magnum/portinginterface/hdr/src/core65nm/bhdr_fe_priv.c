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
#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"
#include "bhdm_cec_priv.h"


#include "bchp_hdmi_rx_fe_0.h"
#include "bchp_hdmi_rx_fe_1.h"

BDBG_MODULE(BHDR_FE_PRIV) ;


/******************************************************************************
void BHDR_FE_P_CEC_isr
Summary: Handle CEC interrupts from the HDMI Front End .
*******************************************************************************/
void BHDR_FE_P_CEC_isr( BHDR_FE_ChannelHandle hFeChannel )
	
{
	BREG_Handle hRegister ;
	BAVC_HDMI_CEC_IntMessageType CECMsgType ;
	uint32_t Register ;
	uint32_t ulOffset ;

	BDBG_ASSERT(hFeChannel) ;

	hRegister = hFeChannel->hRegister ;
	ulOffset = hFeChannel->ulOffset ;


	Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_5 + ulOffset) ;
	CECMsgType = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_5, RX_CEC_INT) ;
	
	if (CECMsgType == BAVC_HDMI_CEC_IntMessageType_eReceive)  /* Receive Type Message */
	{
		uint8_t i, j ;
		uint8_t RxCecWordCount ;
		uint32_t RegisterOffset ;

		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		RxCecWordCount = BCHP_GET_FIELD_DATA(Register, REGNAME_CEC_CNTRL_1, REC_WRD_CNT) ;
		RxCecWordCount++ ;
		hFeChannel->cecConfiguration.CecBufferedMsgLength = RxCecWordCount ;
		BDBG_MSG(("Cec Rx Msg Type; WordCount: %d", RxCecWordCount)) ;
		
		/* get the received words and place into the buffer */
		RegisterOffset = 0 ;
		for (i = 0 ; i < RxCecWordCount ; i = i + 4)
		{
			Register = BREG_Read32(hRegister, REGADDR_CEC_RX_DATA_1 + ulOffset + RegisterOffset) ;
			BDBG_MSG(("CEC RxReg %0X: %X", 
				REGADDR_CEC_RX_DATA_1 + ulOffset + (int) (i / 4) * 4, Register)) ;
			
			for (j = 0 ; j + i < RxCecWordCount; j++)
				hFeChannel->cecConfiguration.CecBufferedMsg[i+j] = Register  >> (8 * j) & 0xFF ;
				
			RegisterOffset = RegisterOffset + 4 ;
		}
		
		/* For debugging purposes */
		BDBG_MSG(("Length %d CEC Received Message:", hFeChannel->cecConfiguration.CecBufferedMsgLength)) ;
		for (i = 0 ; i <= hFeChannel->cecConfiguration.CecBufferedMsgLength; i++)
			BDBG_MSG(("CEC RxByte[%02d] = %02X ", i, hFeChannel->cecConfiguration.CecBufferedMsg[i])) ;
		

		/* HDMI TODO: Allow app to re-enable receiving CEC Messages */
#if 0		
		/* re-enable CEC */
		Register = BREG_Read32(hRegister, REGADDR_CEC_CNTRL_1 + ulOffset) ;
		
		Register |= BCHP_MASK_DVP(CEC_CNTRL_1, CLEAR_RECEIVE_OFF) ;
		BREG_Write32(hFeChannel->hRegister, REGADDR_CEC_CNTRL_1 + ulOffset, Register) ;	/* Wr 1 */
		
		Register &= ~ BCHP_MASK_DVP(CEC_CNTRL_1, CLEAR_RECEIVE_OFF) ;
		BREG_Write32(hRegister, REGADDR_CEC_CNTRL_1, Register) ;	/* Wr 0 */				
#endif		
	}
	else
	{
		BDBG_MSG(("CEC Tx Msg Type")) ;
	}
		
			
	
	if (hFeChannel->pfCecCallback_isr)
	{
		BDBG_MSG(("CH%d Calling cb...", hFeChannel->eChannel)) ;
		/* HDMI_TODO??? add structure for CEC info to pass for callback */
		hFeChannel->pfCecCallback_isr(
			hFeChannel->pvCecParm1, hFeChannel->iCecParm2, NULL) ;
	}
	else
	{
		BDBG_MSG(("CH%d No CEC callback installed...", hFeChannel->eChannel)) ;
		BHDR_CEC_EnableReceive(hFeChannel);
	}
	
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL) ;
	Register |= BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, Register) ;

	Register &= ~BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, Register) ;
}

