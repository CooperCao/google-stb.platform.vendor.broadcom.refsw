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


#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"

#include "bhsm_user_cmds.h"


BDBG_MODULE(BHSM);

#ifdef USER_MODE_BHSM_DEBUG
#define BDBG_MSG(x) printf x 
#define BDBG_ERR(x) printf x
#endif 

BERR_Code      BHSM_UserRSA_PackParam (
		BHSM_UserRsaIO_t	*inoutp_userRsaIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen		
);

BERR_Code      BHSM_UserDSA_PackParam (
		BHSM_UserDsaIO_t	*inoutp_userDsaIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen		
);

BERR_Code      BHSM_UserDH_PackParam (
		BHSM_UserDhIO_t	*inoutp_userDhIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen
	);

BERR_Code      BHSM_UserHmacSha_PackParam (
		BHSM_UserHmacShaIO_t	*inoutp_userHmacShaIO,
		BHSM_P_CommandData_t    *p_commandData,
		uint32_t                *p_unParamLen		
);


BERR_Code    BHSM_Token_SubmitCommand (
		BHSM_Handle		in_handle,		
		BHSM_P_CommandData_t *p_commandData,
		bool	*p_token,
		int	seqNo
	);

void BHSM_Pack_8BIT_TO_32BIT(uint32_t *p_dest, unsigned char *p_src, unsigned int len);
void BHSM_UnPack_32BIT_TO_8BIT(unsigned char *p_dest, uint32_t *p_src, unsigned int len);


static const int HmacShaKeySize[] = { 20, 28, 32 };

BERR_Code BHSM_UserRandomNumber (
		BHSM_Handle					in_handle,
		BHSM_UserRandomNumberIO_t	*inoutp_userRandomNumberIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unParamLen = 0; 	/* Parameter length based on Unit32( 32Bit ) */
	BHSM_P_CommandData_t commandData;

	BDBG_MSG(("Inside BHSM_UserRandomNumber\n"));
	BDBG_ENTER(BHSM_UserRandomNumber);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));

	commandData.cmdId = BCMD_cmdType_eUSER_RANDOM_NUMBER;
	commandData.unContMode = 0;

	unParamLen = 0;
	/* only fill the command specific input parameters */
	if ( inoutp_userRandomNumberIO->ucRandomNumType != BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGRAW &&
		inoutp_userRandomNumberIO->ucRandomNumType != BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_FIPS186_X  &&
		inoutp_userRandomNumberIO->ucRandomNumType != BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_FIPS186_K  &&
		inoutp_userRandomNumberIO->ucRandomNumType != BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGSHA )
	{
/*		errCode = BHSM_STATUS_PARM_VALUE_ERR; */
		errCode = BHSM_STATUS_INPUT_PARM_ERR;				
			goto BHSM_P_DONE_LABEL;
	}		
	
	commandData.unInputParamsBuf[unParamLen] = inoutp_userRandomNumberIO->ucRandomNumType;
	unParamLen ++;

	/* check keyDeat and user input data length */
	switch ( inoutp_userRandomNumberIO->keyDest )
	{
		case RANDOM_NUMBER_OUTPUT_TO_HOST :
			if (inoutp_userRandomNumberIO->unDataLen > BHSM_MAX_RANDOM_NUM_LEN)
			{
/*				errCode = BHSM_STATUS_INDATA_TOOLONG_ERR; */
				errCode = BHSM_STATUS_INPUT_PARM_ERR;			
				goto BHSM_P_DONE_LABEL;
			}
			break;

		case BCMD_KeyRamBuf_eFirstRNRam1024 :
			if (inoutp_userRandomNumberIO->unDataLen > MAX_DATA_LENTH_USER_RANDOM_NUMBER_RNRam1024)
			{
/*				errCode = BHSM_STATUS_INDATA_TOOLONG_ERR; */
				errCode = BHSM_STATUS_INPUT_PARM_ERR;
				goto BHSM_P_DONE_LABEL;
			}		
			break;

		case BCMD_KeyRamBuf_eSecondRNRam1024:	
			if (inoutp_userRandomNumberIO->unDataLen > MAX_DATA_LENTH_USER_RANDOM_NUMBER_RNRam1024)
			{
/*				errCode = BHSM_STATUS_INDATA_TOOLONG_ERR; */
				errCode = BHSM_STATUS_INPUT_PARM_ERR;
				goto BHSM_P_DONE_LABEL;
			}		
			break;
			
		default:
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
			goto BHSM_P_DONE_LABEL;
			
	}		
	commandData.unInputParamsBuf[unParamLen]	= inoutp_userRandomNumberIO->keyDest;
	unParamLen ++;

	if (inoutp_userRandomNumberIO->unDataLen%UINT32_SIZE_IN_BYTES != 0 )
	{
		errCode = BHSM_STATUS_PARM_LEN_ERR;
		goto BHSM_P_DONE_LABEL;
	}	
	commandData.unInputParamsBuf[unParamLen]	= inoutp_userRandomNumberIO->unDataLen;
	unParamLen ++;

	commandData.unInputParamLen = unParamLen*UINT32_SIZE_IN_BYTES;

	if ((errCode = BHSM_P_CommonSubmitCommand (channelHandle, &commandData)))
	{
		errCode = BERR_TRACE(errCode);		
		goto BHSM_P_DONE_LABEL;				
			
	}

	unParamLen = 0;
	/* Parse the command specific output parameters */
	inoutp_userRandomNumberIO->unStatus =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check status  */
	if( inoutp_userRandomNumberIO->unStatus != BSP_RETURN_SUCESS ) 
	{
		errCode = BHSM_STATUS_FAILED;
		goto BHSM_P_DONE_LABEL;
	}

	/* Check output paramter length */
	if ( (inoutp_userRandomNumberIO->unDataLen -1)/UINT32_SIZE_IN_BYTES + 1  != 
		commandData.unOutputParamLen/UINT32_SIZE_IN_BYTES -unParamLen )
	{
		errCode = BHSM_STATUS_PARM_LEN_ERR;
		goto BHSM_P_DONE_LABEL;
	}

	BHSM_UnPack_32BIT_TO_8BIT( inoutp_userRandomNumberIO->aucData, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								inoutp_userRandomNumberIO->unDataLen );

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserRandomNumber);
	return( errCode );
}

BERR_Code BHSM_UserRSA (
		BHSM_Handle		in_handle,
		BHSM_UserRsaIO_t	*inoutp_userRsaIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
/*	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2]; */
	uint32_t unParamLen = 0; 	/* Parameter length based on Unit32( 32Bit ) */
	uint32_t remainDataLen,dataBufOffset;
	BHSM_P_CommandData_t commandData;
	int seqNo;

	BDBG_MSG(("Inside BHSM_UserRSA\n"));
	BDBG_ENTER(BHSM_UserRSA);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	if ( (errCode = BHSM_UserRSA_PackParam ( inoutp_userRsaIO,&commandData, &unParamLen)))
	{
		errCode = BERR_TRACE(errCode);		
		goto BHSM_P_DONE_LABEL;
	}

	if ( commandData.unContMode == CONT_MODE_ALL_DATA )
	{
		commandData.unInputParamsBuf[unParamLen] = inoutp_userRsaIO->unInputDataLen;
		unParamLen ++;

		BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
									inoutp_userRsaIO->aucInputData,
									inoutp_userRsaIO->unInputDataLen );
		unParamLen += (inoutp_userRsaIO->unInputDataLen -1 )/UINT32_SIZE_IN_BYTES + 1;		

		commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

		if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), 0 )))
		{
			errCode = BERR_TRACE(errCode);	
			goto BHSM_P_DONE_LABEL;						
		}

	}
	else
	{
		BDBG_ASSERT( (BCMD_PKE_RSA_MAX_DATA_LENGTH%UINT32_SIZE_IN_BYTES) == 0);
	
		remainDataLen = inoutp_userRsaIO->unInputDataLen;
		dataBufOffset = unParamLen;
		seqNo = 0;
		
		while( remainDataLen > 0 ) 
		{					
			if ( remainDataLen <= BCMD_PKE_RSA_MAX_DATA_LENGTH  ) 
			{
				commandData.unContMode = CONT_MODE_LAST_DATA;
				commandData.unInputParamsBuf[unParamLen]= remainDataLen;
				unParamLen ++;

				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userRsaIO->aucInputData[inoutp_userRsaIO->unInputDataLen-remainDataLen],
					remainDataLen) ;
				unParamLen += (remainDataLen -1) /UINT32_SIZE_IN_BYTES + 1;							
				remainDataLen = 0;
			}			
			else
			{
				commandData.unContMode = CONT_MODE_MORE_DATA;	
				commandData.unInputParamsBuf[unParamLen]= BCMD_PKE_DSA_MAX_DATA_LENGTH;					
				unParamLen ++;
				
				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userRsaIO->aucInputData[inoutp_userRsaIO->unInputDataLen -remainDataLen],
					BCMD_PKE_DSA_MAX_DATA_LENGTH);
				unParamLen += BCMD_PKE_DSA_MAX_DATA_LENGTH /UINT32_SIZE_IN_BYTES;	
				
				remainDataLen = remainDataLen - BCMD_PKE_DSA_MAX_DATA_LENGTH;
			}	
			
			commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;
			if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), seqNo )))
			{
				errCode = BERR_TRACE(errCode);	
				goto BHSM_P_DONE_LABEL;						
			}

				
			unParamLen = dataBufOffset ;	/*only pack the data  from the offset of field "inpDataLen" */
			seqNo ++;
			
		}	/*end of while( remainDataLen > 0 ) */;
	}	

	unParamLen = 0;
	/* Parse the command specific output parameters */
	inoutp_userRsaIO ->unStatus =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check status  */
	if ( inoutp_userRsaIO ->unStatus  != BSP_RETURN_SUCESS )
	{
		errCode = BHSM_STATUS_FAILED;
		goto BHSM_P_DONE_LABEL;
	}

	inoutp_userRsaIO ->unOutputDataLen =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	
	
	/* Check output paramter length */
	if ( ((inoutp_userRsaIO ->unOutputDataLen+7)/8 +UINT32_SIZE_IN_BYTES-1)/UINT32_SIZE_IN_BYTES != 
		commandData.unOutputParamLen/UINT32_SIZE_IN_BYTES - unParamLen  ) 
	{
		errCode = BHSM_STATUS_PARM_LEN_ERR;
		goto BHSM_P_DONE_LABEL;
	}
		
	BHSM_UnPack_32BIT_TO_8BIT( inoutp_userRsaIO->aucOutputData, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								(inoutp_userRsaIO ->unOutputDataLen+7)/8 );


BHSM_P_DONE_LABEL:
	/*  debug code only
	#include <stdio.h>  
	int i;
	fprintf(stderr,"inoutp_userRsaIO ->unOutputDataLen=%.8X\n commandData.unOutputParamLen=%.8X\nunParamLen=%d\n",
			inoutp_userRsaIO ->unOutputDataLen,  commandData.unOutputParamLen, unParamLen );	

	for (i=0; i<4; i++)
		fprintf(stderr, "commandData.unOutputParamsBuf[%d]=%.8X\n", i, commandData.unOutputParamsBuf[i]);
	*/
	
	BDBG_LEAVE(BHSM_UserRSA);
	return( errCode );
}

BERR_Code BHSM_UserRSA_PackParam (
		BHSM_UserRsaIO_t	*inoutp_userRsaIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen		
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_MSG(("BHSM_UserRSA_PackParam \n"));	
	BDBG_ENTER(BHSM_UserRSA_PackParam);

	BKNI_Memset(p_commandData, 0, sizeof(BHSM_P_CommandData_t));

	p_commandData->cmdId = BCMD_cmdType_eUSER_RSA;
	p_commandData->unContMode = 0;

	*p_unParamLen = 0;
	/* only fill the command specific input parameters */
	switch (inoutp_userRsaIO->ucOprMode)
	{
/*		case BPI_PKE_RSAOperation_ePublicKeyDec:  */
		case BPI_PKE_RSAOperation_eNormal:
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userRsaIO->unInputDataLen > 256));
			*/
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userRsaIO->unInputDataLen > 256));
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;		
			break;
			
/*		case BPI_PKE_RSAOperation_ePrivateKeyEnc:	*/
		case BPI_PKE_RSAOperation_eCrt:
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userRsaIO->unInputDataLen > 256));
			*/
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userRsaIO->unInputDataLen > 256));			
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;							
			break;
			
		case BPI_PKE_PASS_THROUGH:	
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userRsaIO->unInputDataLen > 128*5 + 256 + 8));
			*/
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userRsaIO->unInputDataLen > 128*5 + 256 + 8));
			
			if (  inoutp_userRsaIO->unInputDataLen <= BCMD_PKE_RSA_MAX_DATA_LENGTH )
				p_commandData->unContMode = CONT_MODE_ALL_DATA ;	
			else
				p_commandData->unContMode = CONT_MODE_MORE_DATA ;	
			
			break;

		default:
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;		
			goto BHSM_P_DONE_LABEL;
			
	}
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userRsaIO->ucOprMode;
	(*p_unParamLen) ++;

	if ( inoutp_userRsaIO->ucOprMode != BPI_PKE_PASS_THROUGH )
	{
		if ( inoutp_userRsaIO->keySrcIndex1 !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userRsaIO->keySrcIndex1 != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userRsaIO->keySrcIndex1 !=  BCMD_KeyRamBuf_eThirdRam2048)
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;					
				goto BHSM_P_DONE_LABEL;
		}		

		if ( inoutp_userRsaIO->expSrcIdx !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userRsaIO->expSrcIdx != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userRsaIO->expSrcIdx !=  BCMD_KeyRamBuf_eThirdRam2048)
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;					
				goto BHSM_P_DONE_LABEL;
		}		

/*		if (inoutp_userRsaIO->ucOprMode == BPI_PKE_RSAOperation_ePrivateKeyEnc) */
		if (inoutp_userRsaIO->ucOprMode == BPI_PKE_RSAOperation_eCrt )			
			if ( inoutp_userRsaIO->keySrcIndex2 !=  BCMD_KeyRamBuf_eFirstRam2048 &&
				inoutp_userRsaIO->keySrcIndex2 != BCMD_KeyRamBuf_eSecondRam2048 &&
				inoutp_userRsaIO->keySrcIndex2 !=  BCMD_KeyRamBuf_eThirdRam2048)
			{
/*				errCode = BHSM_STATUS_PARM_VALUE_ERR; */
				errCode = BHSM_STATUS_INPUT_PARM_ERR;				
					goto BHSM_P_DONE_LABEL;
			}		
	}


	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userRsaIO->keySrcIndex1;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userRsaIO->expSrcIdx;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userRsaIO->keySrcIndex2;
	(*p_unParamLen) ++;


BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserRSA_PackParam);
	return( errCode );
}


BERR_Code BHSM_UserDSA (
		BHSM_Handle		in_handle,
		BHSM_UserDsaIO_t	*inoutp_userDsaIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
/*	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2]; */
	uint32_t unParamLen; 	/* Parameter length based on Unit32( 32Bit ) */
	uint32_t remainDataLen,dataBufOffset;
	BHSM_P_CommandData_t commandData;
	int seqNo;

	BDBG_MSG(("Inside BHSM_UserDSA\n"));
	BDBG_ENTER(BHSM_UserDSA);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	if ((errCode = BHSM_UserDSA_PackParam ( inoutp_userDsaIO,&commandData, &unParamLen)))
	{
		errCode = BERR_TRACE((errCode));		
		goto BHSM_P_DONE_LABEL;
	}

	if ( commandData.unContMode == CONT_MODE_ALL_DATA )
	{
		commandData.unInputParamsBuf[unParamLen] = inoutp_userDsaIO->unInputDataLen;
		unParamLen ++;

		BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
									inoutp_userDsaIO->aucInputData,
									inoutp_userDsaIO->unInputDataLen );
		unParamLen += (inoutp_userDsaIO->unInputDataLen - 1)/UINT32_SIZE_IN_BYTES + 1;		

		commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

		if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), 0 )))
		{
			errCode = BERR_TRACE(errCode);	
			goto BHSM_P_DONE_LABEL;						
		}
	}
	else
	{
		BDBG_ASSERT( (BCMD_PKE_DSA_MAX_DATA_LENGTH%UINT32_SIZE_IN_BYTES) == 0);
	
		remainDataLen = inoutp_userDsaIO->unInputDataLen;
		dataBufOffset = unParamLen;
		seqNo = 0;
		
		while( remainDataLen > 0 ) 
		{					
			if ( remainDataLen <= BCMD_PKE_DSA_MAX_DATA_LENGTH  )
			{
				commandData.unContMode = CONT_MODE_LAST_DATA;
				commandData.unInputParamsBuf[unParamLen]= remainDataLen;
				unParamLen ++;

				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userDsaIO->aucInputData[inoutp_userDsaIO->unInputDataLen -remainDataLen],
					remainDataLen) ;
				unParamLen += (remainDataLen -1 )/UINT32_SIZE_IN_BYTES + 1;			
				remainDataLen = 0;
			}			
			else
			{
				commandData.unContMode = CONT_MODE_MORE_DATA;	
				commandData.unInputParamsBuf[unParamLen]= BCMD_PKE_DSA_MAX_DATA_LENGTH;					
				unParamLen ++;
				
				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userDsaIO->aucInputData[inoutp_userDsaIO->unInputDataLen -remainDataLen],
					BCMD_PKE_DSA_MAX_DATA_LENGTH);
				unParamLen += BCMD_PKE_DSA_MAX_DATA_LENGTH/UINT32_SIZE_IN_BYTES;				
				remainDataLen = remainDataLen - BCMD_PKE_DSA_MAX_DATA_LENGTH;
			}	
			
			commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;
			if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), seqNo )))
			{
				errCode = BERR_TRACE(errCode);	
				goto BHSM_P_DONE_LABEL;						
			}
					
			unParamLen = dataBufOffset ;	/*only pack the data  from the offset of field "inpDataLen" */
			seqNo ++;
			
		}	/*end of while( remainDataLen > 0 ) */;
	}	

	unParamLen = 0;
	/* Parse the command specific output parameters */
	inoutp_userDsaIO ->unStatus =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check status  */
	if ( inoutp_userDsaIO ->unStatus  != BSP_RETURN_SUCESS) 
	{
		errCode = BHSM_STATUS_FAILED;	
		goto BHSM_P_DONE_LABEL;						
	
	}
	
	/* Check output paramter length */
	if( inoutp_userDsaIO->ucOprMode  == BPI_PKE_DSAOperation_ePrivateKeySign ) 
		if (commandData.unOutputParamLen /UINT32_SIZE_IN_BYTES - unParamLen !=40 / UINT32_SIZE_IN_BYTES) 
		{
			errCode = BHSM_STATUS_PARM_LEN_ERR;	
			goto BHSM_P_DONE_LABEL;						
		}

	if (inoutp_userDsaIO->ucOprMode  == BPI_PKE_DSAOperation_ePublicKeyVer)  
		if (commandData.unOutputParamLen /UINT32_SIZE_IN_BYTES - unParamLen != 20/UINT32_SIZE_IN_BYTES) 	
		{
			errCode = BHSM_STATUS_PARM_LEN_ERR;	
			goto BHSM_P_DONE_LABEL;						
		}
	
	BHSM_UnPack_32BIT_TO_8BIT( inoutp_userDsaIO->aucOutputData, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								commandData.unOutputParamLen - unParamLen * UINT32_SIZE_IN_BYTES);
	
BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserDSA);
	return( errCode );
}

BERR_Code BHSM_UserDSA_PackParam (
		BHSM_UserDsaIO_t	*inoutp_userDsaIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen		
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_MSG(("BHSM_UserDSA_PackParam \n"));	
	BDBG_ENTER(BHSM_UserDSA_PackParam);

	BKNI_Memset(p_commandData, 0, sizeof(BHSM_P_CommandData_t));

	p_commandData->cmdId = BCMD_cmdType_eUSER_DSA;

	*p_unParamLen = 0;
	/* only fill the command specific input parameters */
	BDBG_ASSERT( (BCMD_PKE_DSA_MAX_DATA_LENGTH%UINT32_SIZE_IN_BYTES) == 0);
	
	switch (inoutp_userDsaIO->ucOprMode)
	{
		case BPI_PKE_DSAOperation_ePrivateKeySign:
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDsaIO->unInputDataLen > 20));
			*/	
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDsaIO->unInputDataLen > 20));			
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;		
			break;
			
		case BPI_PKE_DSAOperation_ePublicKeyVer:				
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDsaIO->unInputDataLen > 128+ 60));
			*/	
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDsaIO->unInputDataLen > 128+ 60));
			
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;							
			break;
			
		case BPI_PKE_PASS_THROUGH:	
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDsaIO->unInputDataLen > 3*256 + 100 + 12));
			*/
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDsaIO->unInputDataLen > 3*256 + 100 + 12));			
			
			if (  inoutp_userDsaIO->unInputDataLen <= BCMD_PKE_DSA_MAX_DATA_LENGTH )
				p_commandData->unContMode = CONT_MODE_ALL_DATA ;	
			else
				p_commandData->unContMode = CONT_MODE_MORE_DATA ;	
			
			break;

		default:
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode =BHSM_STATUS_INPUT_PARM_ERR;
			goto BHSM_P_DONE_LABEL;
			
	}

	if (inoutp_userDsaIO->ucOprMode !=  BPI_PKE_PASS_THROUGH )
	{
		if ( inoutp_userDsaIO->srcPIndex !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userDsaIO->srcPIndex != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userDsaIO->srcPIndex !=  BCMD_KeyRamBuf_eThirdRam2048)
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;					
				goto BHSM_P_DONE_LABEL;
		}		

		if ( inoutp_userDsaIO->srcGIndex !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userDsaIO->srcGIndex != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userDsaIO->srcGIndex !=  BCMD_KeyRamBuf_eThirdRam2048 )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
				goto BHSM_P_DONE_LABEL;
		}		

		if ( inoutp_userDsaIO->srcQIndex !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userDsaIO->srcQIndex != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userDsaIO->srcQIndex !=  BCMD_KeyRamBuf_eThirdRam2048 )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
				goto BHSM_P_DONE_LABEL;
		}		
		
		if ( inoutp_userDsaIO->privKeyXIndex !=  BCMD_KeyRamBuf_eFirstRNRam1024 &&
			inoutp_userDsaIO->privKeyXIndex != BCMD_KeyRamBuf_eSecondRNRam1024 )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
				goto BHSM_P_DONE_LABEL;
		}		

		if ( inoutp_userDsaIO->privKeyKIndex !=  BCMD_KeyRamBuf_eFirstRNRam1024 &&
			inoutp_userDsaIO->privKeyKIndex != BCMD_KeyRamBuf_eSecondRNRam1024 )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
				goto BHSM_P_DONE_LABEL;
		}		
	}

	/*Copy user input paramenter even though they might not be used in some operation mode */
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->ucOprMode;
	(*p_unParamLen) ++;
	
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->srcPIndex;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->srcGIndex;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->srcQIndex;
	(*p_unParamLen) ++;
	
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->privKeyXIndex;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDsaIO->privKeyKIndex;
	(*p_unParamLen) ++;

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserDSA_PackParam );
	return( errCode );
}


BERR_Code BHSM_UserDH (
		BHSM_Handle		in_handle,
		BHSM_UserDhIO_t	*inoutp_userDhIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
/*	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];  */
	uint32_t unParamLen = 0; 	/* Parameter length based on Unit32( 32Bit ) */
	BHSM_P_CommandData_t commandData;
	uint32_t maxBufDataLen,remainDataLen ,dataBufOffset;
	int seqNo;

	BDBG_MSG(("Inside BHSM_UserDH\n"));
	BDBG_ENTER(BHSM_UserDH);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BHSM_UserDH_PackParam ( inoutp_userDhIO, &commandData, &unParamLen);

	if ( commandData.unContMode == CONT_MODE_ALL_DATA )
	{
		commandData.unInputParamsBuf[unParamLen] = inoutp_userDhIO->unInputDataLen;
		unParamLen ++;

		BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
									inoutp_userDhIO->aucInputData,
									inoutp_userDhIO->unInputDataLen );
		unParamLen += inoutp_userDhIO->unInputDataLen /UINT32_SIZE_IN_BYTES;		

		commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

		if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), 0)))
		{
			errCode = BERR_TRACE(errCode);	
			goto BHSM_P_DONE_LABEL;						
		}

	}
	else
	{
		BDBG_ASSERT( (BCMD_PKE_DH_MAX_DATA_LENGTH%UINT32_SIZE_IN_BYTES) == 0);
	
		maxBufDataLen = BCMD_PKE_DH_MAX_DATA_LENGTH ;
		remainDataLen = inoutp_userDhIO->unInputDataLen;
		dataBufOffset = unParamLen;
		seqNo = 0;

		while( remainDataLen > 0 ) 
		{
						
			if ( remainDataLen <=  maxBufDataLen )	
			{
				commandData.unContMode = CONT_MODE_LAST_DATA;
				commandData.unInputParamsBuf[unParamLen]= remainDataLen;
				unParamLen ++;

				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userDhIO->aucInputData[inoutp_userDhIO->unInputDataLen -remainDataLen],
					remainDataLen);
				unParamLen += (remainDataLen - 1) /UINT32_SIZE_IN_BYTES + 1;		
				remainDataLen = 0;
			}			
			else
			{
				commandData.unContMode = CONT_MODE_MORE_DATA;	
				commandData.unInputParamsBuf[unParamLen]= maxBufDataLen;					
				unParamLen ++;
				
				BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
					&inoutp_userDhIO->aucInputData[inoutp_userDhIO->unInputDataLen -remainDataLen],
					maxBufDataLen);
				unParamLen += maxBufDataLen/UINT32_SIZE_IN_BYTES;	
				remainDataLen = remainDataLen - maxBufDataLen;
			}	
			
			commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

			if ( (errCode = BHSM_Token_SubmitCommand ( in_handle,&commandData, &(in_handle->bIsBusyPKE), seqNo )))
			{
				errCode = BERR_TRACE(errCode);	
				goto BHSM_P_DONE_LABEL;						
			}
			
			unParamLen = dataBufOffset ;	/*only pack the data  from the offset of field "inpDataLen" */
			seqNo ++ ;
						
		}/* end of while(remainDataLen > 0)*/
	}	

	unParamLen = 0;
	/* Parse the command specific output parameters */
	inoutp_userDhIO ->unStatus =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check status  */
	if ( inoutp_userDhIO ->unStatus  != BSP_RETURN_SUCESS) 
	{
		errCode = BHSM_STATUS_FAILED;	
		goto BHSM_P_DONE_LABEL;						
	
	}
	
	inoutp_userDhIO ->unOutputDataLen =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check output paramter length */
	/* According to the firmware document, this length is in Bits */
	if (  (inoutp_userDhIO ->unOutputDataLen/8 -1 )/ UINT32_SIZE_IN_BYTES  + 1 != 
				commandData.unOutputParamLen /UINT32_SIZE_IN_BYTES -  unParamLen  )	
	{
		errCode = BHSM_STATUS_PARM_LEN_ERR;
		goto BHSM_P_DONE_LABEL;
	}

	/* 1) rest bit at MSB of the last Word32, (inoutp_userDhIO ->unOutputDataLen+7)/8
	     2) rest bit at LSB of the last Word32, (inoutp_userDhIO ->unOutputDataLen+31)/32*UINT32_SIZE_IN_BYTES
	*/
	BHSM_UnPack_32BIT_TO_8BIT( inoutp_userDhIO->aucOutputData, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								(inoutp_userDhIO ->unOutputDataLen+31)/32*UINT32_SIZE_IN_BYTES);

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserDH);
	return( errCode );
}

BERR_Code BHSM_UserDH_PackParam (
		BHSM_UserDhIO_t	*inoutp_userDhIO,
		BHSM_P_CommandData_t *p_commandData,
		uint32_t *p_unParamLen
	)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_MSG(("BHSM_UserDH_PackParam \n"));	
	BDBG_ENTER(BHSM_UserDH_PackParam);

	BKNI_Memset(p_commandData, 0, sizeof(BHSM_P_CommandData_t));

	p_commandData->cmdId = BCMD_cmdType_eUSER_DH;

	*p_unParamLen = 0;
	/* only fill the command specific input parameters */
	switch (inoutp_userDhIO->ucOprMode)
	{
		case BPI_PKE_DHOperation_ePublicKeyGen:
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDhIO->unInputDataLen > 0));
			*/	
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDhIO->unInputDataLen > 0));			
			
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;	
			break;
			
		case BPI_PKE_DHOperation_eSecretKeyGen:				
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDhIO->unInputDataLen > 256));
			*/	
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDhIO->unInputDataLen > 256));
			p_commandData->unContMode = CONT_MODE_ALL_DATA ;							
			break;
			
		case BPI_PKE_PASS_THROUGH:	
			/* check user input data length */
			/*
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INDATA_TOOLONG_ERR,
				(inoutp_userDhIO->unInputDataLen > 3*256 + 12));
			*/			
			BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_INPUT_PARM_ERR,
				(inoutp_userDhIO->unInputDataLen > 3*256 + 12));			
			
			if (  inoutp_userDhIO->unInputDataLen <= BCMD_PKE_DH_MAX_DATA_LENGTH )
				p_commandData->unContMode = CONT_MODE_ALL_DATA ;	
			else
				p_commandData->unContMode = CONT_MODE_MORE_DATA ;	
			
			break;

		default:
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;
			goto BHSM_P_DONE_LABEL;
			
	}
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDhIO->ucOprMode;
	(*p_unParamLen) ++;

	if (inoutp_userDhIO->ucOprMode !=  BPI_PKE_PASS_THROUGH )
	{

		if ( inoutp_userDhIO->srcPIndex !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userDhIO->srcPIndex != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userDhIO->srcPIndex !=  BCMD_KeyRamBuf_eThirdRam2048)
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
			goto BHSM_P_DONE_LABEL;
		}		

		if ( inoutp_userDhIO->srcGIndex !=  BCMD_KeyRamBuf_eFirstRam2048 &&
			inoutp_userDhIO->srcGIndex != BCMD_KeyRamBuf_eSecondRam2048 &&
			inoutp_userDhIO->srcGIndex !=  BCMD_KeyRamBuf_eThirdRam2048 &&
			inoutp_userDhIO->srcGIndex !=  BPI_PKE_PASS_THROUGH )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;					
			goto BHSM_P_DONE_LABEL;
		}		
		
		if ( inoutp_userDhIO->privKeyXIndex !=  BCMD_KeyRamBuf_eFirstRNRam1024 &&
			inoutp_userDhIO->privKeyXIndex != BCMD_KeyRamBuf_eSecondRNRam1024 )
		{
/*			errCode = BHSM_STATUS_PARM_VALUE_ERR; */
			errCode = BHSM_STATUS_INPUT_PARM_ERR;			
			goto BHSM_P_DONE_LABEL;
		}		
	}

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDhIO->srcPIndex;
	(*p_unParamLen) ++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDhIO->srcGIndex;
	(*p_unParamLen) ++;
	
	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userDhIO->privKeyXIndex;
	(*p_unParamLen) ++;

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserDH_PackParam );
	return( errCode );
}

BERR_Code BHSM_UserDesAes (
		BHSM_Handle			in_handle,
		BHSM_UserDesAesIO_t	*inoutp_userDesAesIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unParamLen = 0; 	/* Parameter length based on Unit32( 32Bit ) */
	BHSM_P_CommandData_t commandData;
	unsigned char				tempChar, i;

	BDBG_MSG(("Inside BHSM_UserDesAes\n"));	
	BDBG_ENTER(BHSM_UserDesAes);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));
#ifdef FIX_LATER
	commandData.cmdId = BCMD_cmdType_eUSER_DES_AES;
	commandData.unContMode = 0;
#else
	commandData.cmdId = 0;

#endif
	unParamLen = 0;
	/* only fill the command specific input parameters */
	/*
	BDBG_ASSERT( (KEY_INPUT_PARAMETER_SIZE%UINT32_SIZE_IN_BYTES) == 0);
	*/


	if(inoutp_userDesAesIO->ucAlgorithm == BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABA_ECB){
		for (i=0; i<8; i++){
			tempChar = inoutp_userDesAesIO->auKeyData[i];
			inoutp_userDesAesIO->auKeyData[i] = inoutp_userDesAesIO->auKeyData[i+8];
			inoutp_userDesAesIO->auKeyData[i+8]= tempChar;
		}
	}


	BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
								inoutp_userDesAesIO->auKeyData,
								KEY_INPUT_PARAMETER_SIZE);

	unParamLen += (KEY_INPUT_PARAMETER_SIZE - 1)/UINT32_SIZE_IN_BYTES + 1;

	if ( 	inoutp_userDesAesIO->ucAlgorithm != BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_ONE_DES_ECB &&
		inoutp_userDesAesIO->ucAlgorithm != BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_THREE_DES_ABA_ECB&&
		inoutp_userDesAesIO->ucAlgorithm != BCMD_USER_DES_AES_INPUT_ALGORITHM_SEL_AES_128_ECB)
	{
/*		errCode = BHSM_STATUS_PARM_VALUE_ERR; */
		errCode = BHSM_STATUS_INPUT_PARM_ERR;			
		
		goto BHSM_P_DONE_LABEL;
	}			
	commandData.unInputParamsBuf[unParamLen] = inoutp_userDesAesIO->ucAlgorithm ;
	unParamLen ++;

	if ( inoutp_userDesAesIO->ucOprMode != BHSM_USER_DES_AES_OPRMODE_DECRYPTION &&
		inoutp_userDesAesIO->ucOprMode != BHSM_USER_DES_AES_OPRMODE_ENCRYPTION)
	{
/*		errCode = BHSM_STATUS_PARM_VALUE_ERR; */
		errCode = BHSM_STATUS_INPUT_PARM_ERR;			
		goto BHSM_P_DONE_LABEL;
	}		
	commandData.unInputParamsBuf[unParamLen]	= inoutp_userDesAesIO->ucOprMode;
	unParamLen ++;

	/* check user input data length */
	if(inoutp_userDesAesIO->unInputDataLen > BCMD_USER_DES_AES_INPUT_DATA_MAX_LENGTH)
	{
/*		errCode = BHSM_STATUS_INDATA_TOOLONG_ERR; */
		errCode = BHSM_STATUS_INPUT_PARM_ERR;
		goto BHSM_P_DONE_LABEL;
	}

	commandData.unInputParamsBuf[unParamLen]	= inoutp_userDesAesIO->unInputDataLen;
	unParamLen ++;

	BHSM_Pack_8BIT_TO_32BIT( 	&commandData.unInputParamsBuf[unParamLen] , 	
								inoutp_userDesAesIO->aucInputData,
								inoutp_userDesAesIO->unInputDataLen );
	unParamLen += (inoutp_userDesAesIO->unInputDataLen -1)/UINT32_SIZE_IN_BYTES + 1;
	
	commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

	if ((errCode = BHSM_P_CommonSubmitCommand (channelHandle, &commandData)))
	{
		errCode = BERR_TRACE(errCode);		
		goto BHSM_P_DONE_LABEL;							
	}

	unParamLen = 0;
	/* Parse the command specific output parameters */
	inoutp_userDesAesIO ->unStatus =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	

	/* Check status  */
	if( inoutp_userDesAesIO ->unStatus  != BSP_RETURN_SUCESS)
	{
		errCode = BHSM_STATUS_FAILED;
		goto BHSM_P_DONE_LABEL;
	}

	inoutp_userDesAesIO ->unOutputDataLen =  commandData.unOutputParamsBuf[unParamLen];
	unParamLen ++;	
	
	/* Check output paramter length */
	if (  (inoutp_userDesAesIO ->unOutputDataLen -1 )/ UINT32_SIZE_IN_BYTES + 1 != 
				commandData.unOutputParamLen /UINT32_SIZE_IN_BYTES - unParamLen  )	
	{
		errCode = BHSM_STATUS_PARM_LEN_ERR;
		goto BHSM_P_DONE_LABEL;
	}

	BHSM_UnPack_32BIT_TO_8BIT( inoutp_userDesAesIO->aucOutputData, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								inoutp_userDesAesIO ->unOutputDataLen);


BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserDesAes);
	return( errCode );
}

BERR_Code BHSM_UserHmacSha (
		BHSM_Handle				in_handle,
		BHSM_UserHmacShaIO_t	*inoutp_userHmacShaIO
)
{
	BERR_Code             errCode = BERR_SUCCESS;
	uint32_t              unParamLen = 0; 	/* Parameter length based on Unit32( 32Bit ) */
	BHSM_P_CommandData_t  commandData;
	int                   HmacShaDigestLen;

	BDBG_MSG(("Inside BHSM_UserHmacSha\n"));	
	BDBG_ENTER(BHSM_UserHmacSha);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	unParamLen = 0;
	if ( (errCode = BHSM_UserHmacSha_PackParam ( inoutp_userHmacShaIO,&commandData, &unParamLen)))
	{
		errCode = BERR_TRACE((errCode));		
		goto BHSM_P_DONE_LABEL;
	}
	HmacShaDigestLen = HmacShaKeySize[inoutp_userHmacShaIO->shaType];

	commandData.unInputParamLen = unParamLen * UINT32_SIZE_IN_BYTES;

	if ( (errCode = BHSM_Token_SubmitCommand ( in_handle, &commandData, 
			&(in_handle->bIsBusyHmacSha1[inoutp_userHmacShaIO->contextSwitch-BPI_HmacSha1_Context_eHmacSha1Ctx0]), 
			0)))
	{
		errCode = BERR_TRACE(errCode);	
		goto BHSM_P_DONE_LABEL;						
	}

	if ( commandData.unContMode == CONT_MODE_MORE_DATA)
	{
		/* do nothing, fall through, just return the 'errCode' which is set to BERR_SUCCESS for all
		     non-final data piece.   No real SHA1 finger print data returned to upper layers
		*/
	}else	/*CONT_MODE_ALL_DATA or  CONT_MODE_LAST_DATA */
	{
		unParamLen = 0;
		/* Parse the command specific output parameters */
		inoutp_userHmacShaIO->unStatus =  commandData.unOutputParamsBuf[unParamLen];
		unParamLen ++;	

		/* Check status  */
		if ( inoutp_userHmacShaIO ->unStatus  != BSP_RETURN_SUCESS )
		{
			errCode = BHSM_STATUS_FAILED;
			goto BHSM_P_DONE_LABEL;
		}

		/* Check output paramter length */
		if (commandData.unOutputParamLen/UINT32_SIZE_IN_BYTES  - unParamLen	!= HmacShaDigestLen/UINT32_SIZE_IN_BYTES ) 
		{
			errCode = BHSM_STATUS_PARM_LEN_ERR;
			goto BHSM_P_DONE_LABEL;
		}

		BHSM_UnPack_32BIT_TO_8BIT( inoutp_userHmacShaIO->aucOutputDigest, 
								&commandData.unOutputParamsBuf[unParamLen] , 
								commandData.unOutputParamLen - unParamLen *UINT32_SIZE_IN_BYTES);
		/* return also the digest length */
		inoutp_userHmacShaIO->digestSize = commandData.unOutputParamLen - unParamLen *UINT32_SIZE_IN_BYTES;
	
	}

BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserHmacSha);
	return( errCode );
}



BERR_Code BHSM_UserHmacSha_PackParam (
		BHSM_UserHmacShaIO_t	*inoutp_userHmacShaIO,
		BHSM_P_CommandData_t    *p_commandData,
		uint32_t                *p_unParamLen		
)
{
	BERR_Code errCode = BERR_SUCCESS;
	int       HmacShaKeyLen;

	BDBG_MSG(("BHSM_UserHmacSha_PackParam \n"));	
	BDBG_ENTER(BHSM_UserHmacSha_PackParam);

	BKNI_Memset(p_commandData, 0, sizeof(BHSM_P_CommandData_t));


	p_commandData->cmdId      = BCMD_cmdType_eUSER_SHA1;
	p_commandData->unContMode = inoutp_userHmacShaIO->contMode;

	/* only fill the command specific input parameters */
	if ( inoutp_userHmacShaIO->oprMode != BPI_HmacSha1_Op_eSha1 &&
		inoutp_userHmacShaIO->oprMode  != BPI_HmacSha1_Op_eHmac )

	{
		errCode = BHSM_STATUS_INPUT_PARM_ERR;
			goto BHSM_P_DONE_LABEL;
	}		

	p_commandData->unInputParamsBuf[*p_unParamLen] = 
			inoutp_userHmacShaIO->oprMode |
			(inoutp_userHmacShaIO->VirtualKeyLadderID << BHSM_HMACSHA_VKL_SHIFT) |
			(inoutp_userHmacShaIO->keyLayer           << BHSM_HMACSHA_KL_SHIFT);
	(*p_unParamLen) ++;

	if ( inoutp_userHmacShaIO->contextSwitch <  BPI_HmacSha1_Context_eHmacSha1Ctx0  ||
		inoutp_userHmacShaIO->contextSwitch  >= BPI_HmacSha1_Context_eMax)
	{

		errCode = BHSM_STATUS_INPUT_PARM_ERR;
		goto BHSM_P_DONE_LABEL;		
	}			

#ifdef USER_SHA_KEY_INCLUDE
	p_commandData->unInputParamsBuf[*p_unParamLen] = 
			inoutp_userHmacShaIO->contextSwitch |
			(inoutp_userHmacShaIO->shaType    << BHSM_HMACSHA_SHATYPE_SHIFT) |
			(inoutp_userHmacShaIO->dataSource << BHSM_HMACSHA_DRAM_SHIFT) |
			(inoutp_userHmacShaIO->unIncludeKey << BHSM_HMACSHA_INCLUDEKEY_SHIFT);
	(*p_unParamLen) ++;
#else
	p_commandData->unInputParamsBuf[*p_unParamLen] = 
			inoutp_userHmacShaIO->contextSwitch |
			(inoutp_userHmacShaIO->shaType    << BHSM_HMACSHA_SHATYPE_SHIFT) |
			(inoutp_userHmacShaIO->dataSource << BHSM_HMACSHA_DRAM_SHIFT);
	(*p_unParamLen) ++;
#endif
	
#ifdef USER_SHA_FIPS_KEY_256
	p_commandData->unInputParamsBuf[*p_unParamLen] = 
			inoutp_userHmacShaIO->unKeyLength;
	(*p_unParamLen) ++;
#endif

	if(inoutp_userHmacShaIO->unKeyLength != 0)
		{
	HmacShaKeyLen = HmacShaKeySize[inoutp_userHmacShaIO->shaType];
	BHSM_Pack_8BIT_TO_32BIT(&p_commandData->unInputParamsBuf[*p_unParamLen] , 	
							inoutp_userHmacShaIO->keyData,
							HmacShaKeyLen);
	*p_unParamLen += (HmacShaKeyLen - 1)/UINT32_SIZE_IN_BYTES + 1;
		}

	p_commandData->unInputParamsBuf[*p_unParamLen] = inoutp_userHmacShaIO->unInputDataLen;
	(*p_unParamLen)++;

	p_commandData->unInputParamsBuf[*p_unParamLen] = (uint32_t)inoutp_userHmacShaIO->pucInputData;
	(*p_unParamLen)++;


BHSM_P_DONE_LABEL:
	
	BDBG_LEAVE(BHSM_UserHmacSha_PackParam);
	return( errCode );

}




BERR_Code BHSM_Token_SubmitCommand (
		BHSM_Handle		in_handle,		
		BHSM_P_CommandData_t *p_commandData,
		bool	*p_token,
		int	seqNo
	)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface2];
	uint32_t unStatus;

	BDBG_MSG(("Inside BHSM_Token_SubmitCommand \n"));	
	BDBG_ENTER(BHSM_Token_SubmitCommand);

	BSTD_UNUSED(seqNo);

	/*Lock the Mutex 
	if ( (errCode = BKNI_AcquireMutex( channelHandle->mutexLock)) )
	{
		errCode = BERR_TRACE(errCode);	
		goto BHSM_P_DONE_LABEL;	
	}*/

#if 0
	/*Acquire the context token*/
	if ( seqNo == 0 || p_commandData->unContMode == CONT_MODE_ALL_DATA  ) 
	{
	 /* Only acquire context token in the first time submit */
		if ( !*p_token ) 
			*p_token = true;	 /* repeatedly this could be set once for every 1st 320-byte piece in each segment, this is not desired yet it's harmless so far */
		else
		{
			/*This Context token has been used*/
			errCode = BHSM_STATUS_HW_BUSY_ERR;	
			goto BHSM_P_DONE_LABEL;	
		}
	}					
				
	p_commandData->bLockWaive = true;

#endif

	if ( (errCode = BHSM_P_CommonSubmitCommand (channelHandle, p_commandData)))
	{
		errCode = BERR_TRACE(errCode);
		*p_token = false;				
		goto BHSM_P_DONE_LABEL;	
	}

	unStatus =  p_commandData->unOutputParamsBuf[0];

	if ( p_commandData->unContMode == CONT_MODE_MORE_DATA)
	{     /* 1)if there is a new "More" defined in bsp_err.h, this line may be affected.
		     2) suggest to define just one "More" enum a3/a8/etc... to be one value.
	       */
		if ( unStatus  != BSP_NEED_MORE_DATA_PKE  && unStatus  != BSP_NEED_MORE_DATA_HMACSHA1)
		{
			errCode = BHSM_STATUS_FAILED;
			*p_token = false;			
			goto BHSM_P_DONE_LABEL;
		}	
	}	

	/*Release the context token*/
	if (  p_commandData->unContMode == CONT_MODE_ALL_DATA  || 
		p_commandData->unContMode == CONT_MODE_LAST_DATA)  
		*p_token = false;			

BHSM_P_DONE_LABEL:

	/* UnLock the Mutex
	BKNI_ReleaseMutex( channelHandle->mutexLock);*/
	
	BDBG_LEAVE(BHSM_Token_SubmitCommand);
	return( errCode );
}



BERR_Code BHSM_CancelCmd (
		BHSM_Handle			in_handle,
		BHSM_CancelCmdIO_t	*inoutp_cancelCmdIO
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_ChannelHandle channelHandle = in_handle->channelHandles[BHSM_HwModule_eCmdInterface1];
	uint32_t unParamLen = 0;
	BHSM_P_CommandData_t commandData;

	BDBG_MSG(("Inside BHSM_CancelCmd\n"));	
	BDBG_ENTER(BHSM_CancelCmd);
	BDBG_ASSERT( in_handle );
	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(in_handle->ulMagicNumber != BHSM_P_HANDLE_MAGIC_NUMBER ) );	

	BKNI_Memset(&commandData, 0, sizeof(BHSM_P_CommandData_t));

	commandData.cmdId = BCMD_cmdType_eSESSION_BRCM_CANCEL_COMMAND;
	commandData.unContMode = 0;
	
	/* only fill the command specific input parameters */

	BHSM_P_CHECK_ERR_CODE_FUNC(errCode, 
			BHSM_P_CommonSubmitCommand (channelHandle, 
					&commandData));

	/* Parse the command specific output parameters */
	inoutp_cancelCmdIO->unStatus =  commandData.unOutputParamsBuf[0];
	unParamLen += 4;	

 	BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED, 
		(inoutp_cancelCmdIO->unStatus != 0) );		
	
BHSM_P_DONE_LABEL:

	
	BDBG_LEAVE(BHSM_CancelCmd);
	BDBG_MSG(("Exit BHSM_CancelCmd\n"));	
	return( errCode );
}


void BHSM_Pack_8BIT_TO_32BIT(uint32_t *p_dest, unsigned char *p_src, unsigned int len)
{
	unsigned int i;

	if(len==0) return;

	for ( i = 0 ; i < len ; i++ )
	{
		if ( i % UINT32_SIZE_IN_BYTES == 0)	*(p_dest + i/UINT32_SIZE_IN_BYTES ) = 0;
		*(p_dest + i /UINT32_SIZE_IN_BYTES )  |=  (*(p_src + i ) << (32 - ( i%UINT32_SIZE_IN_BYTES + 1)*8));	
	}	

}


void BHSM_UnPack_32BIT_TO_8BIT(unsigned char *p_dest, uint32_t *p_src, unsigned int len)
{
	unsigned int i;

	for ( i = 0 ; i < len ; i++ )
		*(p_dest + i )  =  (unsigned char)((*(p_src+i /UINT32_SIZE_IN_BYTES) 
										>> 	(32 - ( i %UINT32_SIZE_IN_BYTES +1) *8)) & 0xFF) ;

}
