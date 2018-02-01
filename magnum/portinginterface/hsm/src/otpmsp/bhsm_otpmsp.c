/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"
#include "bhsm_private.h"
#include "bhsm_otpmsp.h"
#include "bhsm_bsp_msg.h"

#define SHA160_BYTE_SIZE (20)
BDBG_MODULE (BHSM);

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

BERR_Code BHSM_ProgramOTPPatternSequence (BHSM_Handle hHsm, BHSM_ProgramOtpPatSeqIO_t * pProgOtpPatSeqIO)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t         status;
    unsigned        i;

    BDBG_ENTER (BHSM_ProgramOTPPatternSequence);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pProgOtpPatSeqIO == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_ProgPatternSequence, &header);

    i = 0;
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0xBC32F4AC);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0xD18616B6);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0x9FEB4D54);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0x4A27BF4A);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0xCF1C3178);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0xE2DB98A0);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0x24F64BBA);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0x7698E712);
    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0x0000F48D);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    if (pProgOtpPatSeqIO->bProgCacheReq)
    {
        BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + (i++ * 4), 0xDC11E0BF);
    }
#endif

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);
    pProgOtpPatSeqIO->unStatus = status;
    if (status != 0)
    {
        BDBG_ERR (("%s status[0x%02X]", BSTD_FUNCTION, status)); /* invalid status returned from BSP */
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ProgramOTPPatternSequence);
    return rc;
}

BERR_Code BHSM_ReadOTP (BHSM_Handle hHsm, BHSM_ReadOtpIO_t * pReadOtp)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t         status;

    BDBG_ENTER (BHSM_ReadOTP);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pReadOtp == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOFFLINE_OTP_READ, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOfflineOtpRead_eEnum, pReadOtp->readOtpEnum);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOfflineOtpRead_eKeyType, pReadOtp->keyType);

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);
    pReadOtp->unStatus = status;
    if (status != 0)
    {
        BDBG_ERR (("%s status[0x%02X]", BSTD_FUNCTION, status)); /* invalid status returned from BSP */
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueLo, &pReadOtp->regValueLo[0], 4);

    if ((pReadOtp->readOtpEnum == BPI_Otp_CmdReadRegister_eKeyID) ||
        (pReadOtp->readOtpEnum == BPI_Otp_CmdReadRegister_eKeyHash))
    {
        BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdOfflineOtpRead_eRegValueHi, &pReadOtp->regValueHi[0], 4);
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ReadOTP);
    return rc;
}

BERR_Code BHSM_ProgramMSP (BHSM_Handle hHsm, BHSM_ProgramMspIO_t * pProgMspIO)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    uint8_t         status;
    BHSM_ProgramOtpPatSeqIO_t progOtpPatSeq;

    BDBG_ENTER (BHSM_ProgramMSP);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pProgMspIO == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if (pProgMspIO->progMspEnum >= BCMD_Otp_CmdMsp_eSize || pProgMspIO->progMode != 0x00010112)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR);
    }

    BKNI_Memset (&progOtpPatSeq, 0, sizeof (BHSM_ProgramOtpPatSeqIO_t));
    if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeq)) != BERR_SUCCESS)
    {
        if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeq)) != BERR_SUCCESS)       /* one retry */
        {
            return BERR_TRACE (rc);
        }
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOFFLINE_PROG_MSP, &header);

    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eMode32, pProgMspIO->progMode);
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack16 (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eEnum, pProgMspIO->progMspEnum);
    #else
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eEnum, pProgMspIO->progMspEnum);
    #endif
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eNumOfBits, pProgMspIO->ucDataBitLen);
    BHSM_BspMsg_PackArray (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eMask, &pProgMspIO->aucDataBitMask[0], 4);
    BHSM_BspMsg_PackArray (hMsg, BCMD_Otp_InCmdOfflineProgMsp_eData, &pProgMspIO->aucMspData[0], 4);

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);
    pProgMspIO->unStatus = status;
    if (status != 0)
    {
        BDBG_ERR (("%s status[0x%02X]", BSTD_FUNCTION, status)); /* invalid status returned from BSP */
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ProgramMSP);
    return rc;
}

BERR_Code BHSM_ReadMSP (BHSM_Handle hHsm, BHSM_ReadMspIO_t * pReadMsp)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;

    BDBG_ENTER (BHSM_ReadMSP);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pReadMsp == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOFFLINE_MSP_READ, &header);


    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BHSM_BspMsg_Pack16( hMsg, BCMD_Otp_InCmdOfflineMspRead_eEnum, pReadMsp->readMspEnum );
    #else
    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdOfflineMspRead_eEnum, pReadMsp->readMspEnum );
    #endif


    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    pReadMsp->unStatus = (uint32_t) status;
    if( status != 0 )
    {
        BDBG_ERR(("%s enum[%d] status[0x%X]", BSTD_FUNCTION, pReadMsp->readMspEnum, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdOfflineMspRead_eMspValue, pReadMsp->aucMspData, 4);
    BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdOfflineMspRead_eMspLockValue, pReadMsp->aucLockMspData, 4);

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ReadMSP);
    return rc;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
BERR_Code BHSM_ReadMSP32 (BHSM_Handle hHsm, BHSM_ReadMsp32IO_t * pReadMsp32)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;
    uint16_t        length;
    uint16_t        i;

    BDBG_ENTER (BHSM_ReadMSP32);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pReadMsp32 == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if (pReadMsp32->mspGroupType >= BPI_Otp_BPI_Otp_MspGroupType_eMax)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR);
    }

    if (pReadMsp32->unStartGroup > BHSM_MAX_MSP32_GROUP)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR);
    }

    if (pReadMsp32->unRange > BHSM_MAX_MSP32_RANGE)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR);
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_ReadMSP32, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOtpReadMSP32_eMspGroupType, pReadMsp32->mspGroupType);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOtpReadMSP32_eStartGroup, pReadMsp32->unStartGroup);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdOtpReadMSP32_eRange, pReadMsp32->unRange);

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    pReadMsp32->unStatus = (uint32_t) status;
    if (status != 0)
    {
        BDBG_ERR(("%s status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get16 (hMsg, BCMD_CommonBufferFields_eParamLen, &length);       /* read howmany words to read */

    length /= 4;                /* Convert to num words */
    length--;                   /* Account for status word in output data. */

    if (length > BHSM_MAX_MSP32_RANGE)
    {
        rc = BERR_TRACE (BHSM_STATUS_BSP_ERROR);        /* BSP is indicating too many words to read. */
        goto BHSM_P_DONE_LABEL;
    }

    for (i = 0; i < length; i++)
    {
        uint32_t tmp;

        BHSM_BspMsg_Get32 (hMsg, BCMD_Otp_OutCmdOtpReadMSP32_eMspValueStartAddr + (i * 4),
                           &pReadMsp32->punMspValueGroup[i]);

        /* Byte reverse (endian-swap) the returned 32-bit value */
        tmp =  (pReadMsp32->punMspValueGroup[i] & 0x000000ff) << 24;
        tmp |= (pReadMsp32->punMspValueGroup[i] & 0x0000ff00) << 8;
        tmp |= (pReadMsp32->punMspValueGroup[i] & 0x00ff0000) >> 8;
        tmp |= (pReadMsp32->punMspValueGroup[i] & 0xff000000) >> 24;

        pReadMsp32->punMspValueGroup[i] = tmp;
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ReadMSP32);
    return rc;
}

#endif

BERR_Code BHSM_ReadDataSect (BHSM_Handle hHsm, BHSM_ReadDataSectIO_t * pReadDataSect)
{

    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;
    unsigned        i;
    unsigned char   readData[BHSM_READ_DATA_SECTION_DATA_LEN] = { 0 };
    uint32_t        mspValue = 0;
    BHSM_ReadMspIO_t readMspParm;

    BDBG_ENTER (BHSM_ReadDataSect);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pReadDataSect == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if (pReadDataSect->readDsEnum >= BPI_Otp_DataSection_e_size)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR);
    }

    /* Check if the data section is read protected. */

    BKNI_Memset(&readMspParm, 0, sizeof(readMspParm));

    /* readMspParm.readMspEnum = BCMD_Otp_CmdMsp_eReserved36; */
    readMspParm.readMspEnum = 36;


    rc = BHSM_ReadMSP( hHsm, &readMspParm );
    if( rc != BERR_SUCCESS )
    {
        /* error, its likely that you don't have read permission on the OTP you are trying to read. */
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    for( i = 0; i < BHSM_MSP_DATA_LEN; i++ )
    {
        mspValue = (mspValue << 8) | (readMspParm.aucMspData[i] & readMspParm.aucLockMspData[i] );
    }


    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_DATA_SECTION_READ, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdDataSectionRead_eEnum, pReadDataSect->readDsEnum);

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    pReadDataSect->unStatus = (uint32_t) status;
    if (status != 0)
    {
        BDBG_ERR(("%s  status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    pReadDataSect->isReadProtected = mspValue & ( 1 << pReadDataSect->readDsEnum );

    if (pReadDataSect->isReadProtected)
    {
         BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdDataSectionRead_eArrayData, readData, SHA160_BYTE_SIZE);

         /*
         * Convert from BSP data format to 20-byte big endian array:
		 * BSP format:                           [159:128][127:96][95:64][63:32][31:0]
		 * Output format: [255:240][239:224] ... [159:128][127:96][95:64][63:32][31:0]
         */

         /* Clear up the high order numbers to make sure the correct output. */
         BKNI_Memset(pReadDataSect->aucDataSectData, 0, BHSM_DATA_SECTION_DATA_LEN);
         for (i = 0; i < SHA160_BYTE_SIZE; i++)
         {
             pReadDataSect->aucDataSectData[BHSM_DATA_SECTION_DATA_LEN - SHA160_BYTE_SIZE + i] = readData[i];
         }
    }
    else
    {
        BHSM_BspMsg_GetArray (hMsg, BCMD_Otp_OutCmdDataSectionRead_eArrayData, readData, sizeof (readData));
        /*
         * Convert from BSP data format to 32-byte big endian array:
	     * BSP format:    [15:0][31:16][47:32][63:48] ...  [239:224][255:240]
         * Output format: [255:240][239:224] ... [63:48][47:32][31:16][15:0]
         */
        for (i = 0; i < BHSM_DATA_SECTION_DATA_LEN; i += 2)
        {
            pReadDataSect->aucDataSectData[i] = readData[BHSM_DATA_SECTION_DATA_LEN - i - 2];
            pReadDataSect->aucDataSectData[i+1] = readData[BHSM_DATA_SECTION_DATA_LEN - i - 1];
        }
    }


  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ReadDataSect);
    return rc;
}

BERR_Code BHSM_ProgramDataSect (BHSM_Handle hHsm, BHSM_ProgramDataSectIO_t * pProgDataSect)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;
    BHSM_ProgramOtpPatSeqIO_t progOtpPatSeq;
    int             i, j;
    uint32_t        data[BCMD_BUFFER_BYTE_SIZE / 4];

    BDBG_ENTER (BHSM_ProgramDataSect);

    BDBG_OBJECT_ASSERT( hHsm, BHSM_P_Handle);

    if (pProgDataSect == NULL)
    {
        return BERR_TRACE (BHSM_STATUS_FAILED);
    }

    if (pProgDataSect->progDsEnum >= BPI_Otp_DataSection_e_size)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR); /* invalid data section. */
    }

    if (pProgDataSect->mode != BCMD_OTP_DATASECTIONPROG_MODE)
    {
        return BERR_TRACE (BHSM_STATUS_INPUT_PARM_ERR); /* invalid mode. */
    }

    BKNI_Memset (&progOtpPatSeq, 0, sizeof (BHSM_ProgramOtpPatSeqIO_t));
    if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeq)) != BERR_SUCCESS)
    {
        /* BUG in ROM code ... may  need to send OTP programming pattern twice */
        if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeq)) != BERR_SUCCESS)
        {
            return BERR_TRACE (rc);
        }
    }

    if ((rc = BHSM_BspMsg_Create (hHsm, &hMsg)) != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_DATA_SECTION_PROG, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdDataSectionProg_eEnum, pProgDataSect->progDsEnum);

    /* app-to-hsm: 32-byte-array in big endian
     * hsm-to-BSP:  a particular wrapping to help BSP word0/LSB, word7/MSB, word=[15:0] ||[31:16] */

    j = BHSM_READ_DATA_SECTION_DATA_LEN / 4 - 1;

    for (i = 0; i < BHSM_DATA_SECTION_DATA_LEN; i += 4, j--)
    {
        data[j] = ( (pProgDataSect->aucDataSectData[i] << 8) |
                    (pProgDataSect->aucDataSectData[i + 1]) |
                    (pProgDataSect->aucDataSectData[i + 2] << 24) |
                    (pProgDataSect->aucDataSectData[i + 3] << 16) );
    }

    for (i = 0, j = 0; i < BHSM_DATA_SECTION_DATA_LEN; i += 4, j++)
    {
        BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdDataSectionProg_eArrayData + i, data[j]);
    }

    BHSM_BspMsg_Pack32 (hMsg, BCMD_Otp_InCmdDataSectionProg_eMode32, pProgDataSect->mode);

    BHSM_BspMsg_PackArray (hMsg, BCMD_Otp_InCmdDataSectionProg_eCrc, pProgDataSect->aucCrc, BHSM_CRC_DATA_LEN);

    if ((rc = BHSM_BspMsg_SubmitCommand (hMsg)) != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    pProgDataSect->unStatus = (uint32_t) status;
    if (status != 0)
    {
        BDBG_ERR(("%s  status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ProgramDataSect);
    return rc;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)

BERR_Code BHSM_ReadAntiRollOverCounter (BHSM_Handle hHsm,
                                        BHSM_AntiRollOverCounter_t * pCounter, BHSM_AntiRollOverCounterValue_t * pValue)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;

    BDBG_ENTER (BHSM_ReadAntiRollOverCounter);

    rc = BHSM_BspMsg_Create (hHsm, &hMsg);
    if (rc != BERR_SUCCESS)
    {
        return BERR_TRACE (rc);
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_ROLLOVER_COUNTER_Op, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_ePartition, pCounter->partition);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_eOperation, BPI_Otp_RolloverCounterOp_eRead);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_eFlag, pCounter->flags);

    rc = BHSM_BspMsg_SubmitCommand (hMsg);
    if (rc != BERR_SUCCESS)
    {
        BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    if (status != 0)
    {
        BDBG_ERR(("%s  status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get32 (hMsg, BCMD_Otp_OutCmdCounterOp_eCounterValue, &(pValue->count)); /* read count */
    if (status != 0)
    {
        rc = BERR_TRACE (BHSM_STATUS_BSP_ERROR);
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    (void) BHSM_BspMsg_Destroy (hMsg);

    BDBG_LEAVE (BHSM_ReadAntiRollOverCounter);
    return rc;
}

BERR_Code BHSM_IncrementAntiRollOverCounter (BHSM_Handle hHsm, BHSM_AntiRollOverCounter_t * pCounter)
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    BHSM_ProgramOtpPatSeqIO_t progOtpPatSeqIO;
    unsigned char   status;

    BDBG_ENTER (BHSM_IncrementAntiRollOverCounter);

    BKNI_Memset (&progOtpPatSeqIO, 0x0, sizeof (progOtpPatSeqIO));

    if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeqIO)) != BERR_SUCCESS)
    {
        /* BUG in ROM code ... may  need to send OTP programming pattern twice */
        if ((rc = BHSM_ProgramOTPPatternSequence (hHsm, &progOtpPatSeqIO)) != BERR_SUCCESS)
        {
            (void) BERR_TRACE (rc);
            goto BHSM_P_DONE_LABEL;
        }
    }

    rc = BHSM_BspMsg_Create (hHsm, &hMsg);
    if (rc != BERR_SUCCESS)
    {
        (void) BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_GetDefaultHeader (&header);
    BHSM_BspMsg_Header (hMsg, BCMD_cmdType_eOTP_ROLLOVER_COUNTER_Op, &header);

    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_ePartition, pCounter->partition);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_eOperation, BPI_Otp_RolloverCounterOp_eIncrement);
    BHSM_BspMsg_Pack8 (hMsg, BCMD_Otp_InCmdCounterOp_eFlag, pCounter->flags);

    rc = BHSM_BspMsg_SubmitCommand (hMsg);
    if (rc != BERR_SUCCESS)
    {
        (void) BERR_TRACE (rc);
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8 (hMsg, BCMD_CommonBufferFields_eStatus, &status);  /* check status */
    if (status != 0)
    {
        BDBG_ERR(("%s  status[0x%X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    if (hMsg)
    {
        (void) BHSM_BspMsg_Destroy (hMsg);
    }

    BDBG_LEAVE (BHSM_IncrementAntiRollOverCounter);
    return rc;
}

#else
/* Anti rollover counter is not available on pre-Zeus 4.1 devices */

BERR_Code BHSM_ReadAntiRollOverCounter (BHSM_Handle hHsm,
                                        BHSM_AntiRollOverCounter_t * pCounter, BHSM_AntiRollOverCounterValue_t * pValue)
{
    BSTD_UNUSED (hHsm);
    BSTD_UNUSED (pCounter);
    BSTD_UNUSED (pValue);
    return BERR_TRACE (BERR_NOT_SUPPORTED);
}

BERR_Code BHSM_IncrementAntiRollOverCounter (BHSM_Handle hHsm, BHSM_AntiRollOverCounter_t * pCounter)
{
    BSTD_UNUSED (hHsm);
    BSTD_UNUSED (pCounter);
    return BERR_TRACE (BERR_NOT_SUPPORTED);
}

#endif /* HSM_IS_ASKM_28NM_ZEUS_4_1 */
