/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Module Description: This file contains Broadcom smart card Porting 
 *                     Interface EMV private data structures, enums, 
 *                     definitions and functions prototypes.           
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSCD_EMV_PRIV_H__
#define BSCD_EMV_PRIV_H__

#include "berr_ids.h"
#include "bscd.h"

#ifdef __cplusplus
extern "C" {
#endif


BERR_Code BSCD_Channel_P_EMVATRReadNextByte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *outp_ucData, 
		unsigned long *outp_ulTotalAtrByteTimeInETU, 
		unsigned char *outp_ucParityErrorDetected 
);

BERR_Code BSCD_Channel_P_EMVATRCheckForAdditionalATRBytes(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_EMVATRReceiveAndDecode(
		BSCD_ChannelHandle	in_channelHandle
);


BERR_Code BSCD_Channel_P_EMVValidateTSByte(
		unsigned char in_ucTSByte
);

BERR_Code BSCD_Channel_P_EMVValidateT0Byte(
      unsigned char ucT0Byte, 
      unsigned int  *unNumHistoricalBytes
);

BERR_Code BSCD_Channel_P_EMVValidateTA1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTA1Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTB1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTB1Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTC1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTC1Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTD1Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTD1Byte, 
		bool *outp_bTCKRequired
);


BERR_Code BSCD_Channel_P_EMVValidateTA2Byte(
		unsigned char in_ucTA2Byte
);


BERR_Code BSCD_Channel_P_EMVValidateTB2Byte(
		unsigned char in_ucTB2Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTC2Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTC2Byte		
);

BERR_Code BSCD_Channel_P_EMVValidateTD2Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTD2Byte, 
		unsigned char in_ucTD1Byte, 
		bool * outp_bTCKRequired
);

BERR_Code BSCD_Channel_P_EMVValidateTA3Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTA3Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTB3Byte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char in_ucTB3Byte, 
		unsigned char in_ucFFactor, 
		unsigned char in_ucDFactor
);

BERR_Code BSCD_Channel_P_EMVValidateTC3Byte(
		unsigned char in_ucTC3Byte
);

BERR_Code BSCD_Channel_P_EMVValidateTCKByte(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *inp_ucATR, 
		unsigned int in_unATRLength
);

BERR_Code BSCD_Channel_P_EMVATRByteRead(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *inoutp_ucData,
		unsigned long in_ulMaxAtrByteTimeInETU,
		long in_lMaxTotalAtrByteTimeInETU,
		unsigned long *inoutp_ultotalAtrByteTimeInETU 
);



#ifdef __cplusplus
}
#endif

#endif /* BSCD_PRIV_H__ */
