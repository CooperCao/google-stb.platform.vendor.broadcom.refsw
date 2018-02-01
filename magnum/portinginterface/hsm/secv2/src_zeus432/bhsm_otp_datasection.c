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
#include "bhsm.h"
#include "bhsm_priv.h"
#include "bsp_s_otp_common.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_otp_datasection.h"
#include "bhsm_otp_msp.h"
#include "bhsm_otp_priv.h"
#include "bsp_s_otp.h"

BDBG_MODULE( BHSM );

#define SHA160_BYTE_SIZE     (20)
#define BHSM_CRC_DATA_LEN    (4)

static void BHSM_OtpDataSectionCRC_Caculate( const unsigned char *data_section, unsigned char *crc );

BERR_Code BHSM_OtpDataSection_Write( BHSM_Handle hHsm, const BHSM_DataSectionWrite *pParam )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    BHSM_BspMsgHeader_t header;
    unsigned char   status;
    uint32_t        data[BCMD_BUFFER_BYTE_SIZE / 4];
    uint8_t         dataCrc[BHSM_CRC_DATA_LEN];
    int             i, j;

    BDBG_ENTER( BHSM_OtpDataSection_Write );

    if( !pParam )                                      { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( pParam->index >= BPI_Otp_DataSection_e_size )  { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }

    if( ( rc = BHSM_OTPPatternSequence_Program_priv( hHsm, false ) ) != BERR_SUCCESS ) {
        /* In case historic BUG in ROM code ... may need to send OTP programming pattern twice */
        if( ( rc = BHSM_OTPPatternSequence_Program_priv( hHsm, false ) ) != BERR_SUCCESS ) {
            return BERR_TRACE( rc );
        }
    }

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOTP_DATA_SECTION_PROG, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdDataSectionProg_eEnum, pParam->index );

    /* app-to-hsm: 32-byte-array in big endian
     * hsm-to-BSP:  a particular wrapping to help BSP word0/LSB, word7/MSB, word=[15:0] ||[31:16] */

    j = BHSM_DATA_SECTION_LENGTH / 4 - 1;

    for( i = 0; i < BHSM_DATA_SECTION_LENGTH; i += 4, j-- ) {
        data[j] = ( ( pParam->data[i] << 8 ) |
                    ( pParam->data[i + 1] )  |
                    ( pParam->data[i + 2] << 24 ) |
                    ( pParam->data[i + 3] << 16 ) );
    }

    for( i = 0, j = 0; i < BHSM_DATA_SECTION_LENGTH; i += 4, j++ ) {
        BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdDataSectionProg_eArrayData + i, data[j] );
    }

    BHSM_OtpDataSectionCRC_Caculate( pParam->data, dataCrc );

    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdDataSectionProg_eMode32, BCMD_OTP_DATASECTIONPROG_MODE );

    BHSM_BspMsg_PackArray( hMsg, BCMD_Otp_InCmdDataSectionProg_eCrc, dataCrc, BHSM_CRC_DATA_LEN );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status ); /* check status */
    if( status != 0 ) {
        BDBG_ERR( ( "%s  status[0x%X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_OtpDataSection_Write );
    return rc;
}

BERR_Code BHSM_OtpDataSection_Read( BHSM_Handle hHsm, BHSM_DataSectionRead *pParam )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    unsigned char   status;
    unsigned        i;
    unsigned char   readData[BHSM_DATA_SECTION_LENGTH] = { 0 };
    BHSM_BspMsgHeader_t header;
    BHSM_OtpMspRead readParam;

    BDBG_ENTER( BHSM_OtpDataSection_Read );

    if( pParam->index >= BPI_Otp_DataSection_e_size )  { return BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR ); }
    if( !pParam )                       { return BERR_TRACE( BHSM_STATUS_FAILED ); }

    BKNI_Memset( &readParam, 0, sizeof(readParam) );
    readParam.index = BCMD_Otp_CmdMsp_eReserved36;

    /* Read MSP bits to check if the coresponding data section is read protected. */
    rc = BHSM_OtpMsp_Read( hHsm, &readParam );
    if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); goto BHSM_P_DONE_LABEL; }
        /* error, may not have read permission on the MSP OTP */

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOTP_DATA_SECTION_READ, &header );

    BHSM_BspMsg_Pack8( hMsg, BCMD_Otp_InCmdDataSectionRead_eEnum, pParam->index );

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status ); /* check status */
    if( status != 0 ) {
        BDBG_ERR( ( "%s  status[0x%X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

    /* Check if the MSP bit indicates the DS being readable. */
    readParam.data &= readParam.valid;
    pParam->accessible = !( readParam.data & ( 1 << pParam->index ) );

    if( !pParam->accessible ) {
        BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdDataSectionRead_eArrayData, readData, SHA160_BYTE_SIZE );

        /*
         * Convert from BSP data format to 20-byte big endian array:
         * BSP format:                           [159:128][127:96][95:64][63:32][31:0]
         * Output format: [255:240][239:224] ... [159:128][127:96][95:64][63:32][31:0]
         */

        /* Clear up the high order numbers to make sure the correct output. */
        BKNI_Memset( pParam->data, 0, BHSM_DATA_SECTION_LENGTH );
        for( i = 0; i < SHA160_BYTE_SIZE; i++ ) {
            pParam->data[BHSM_DATA_SECTION_LENGTH - SHA160_BYTE_SIZE + i] = readData[i];
        }
    }
    else {
        BHSM_BspMsg_GetArray( hMsg, BCMD_Otp_OutCmdDataSectionRead_eArrayData, readData, sizeof( readData ) );
        /*
         * Convert from BSP data format to 32-byte big endian array:
         * BSP format:    [15:0][31:16][47:32][63:48] ... [239:224][255:240]
         * Output format: [255:240][239:224] ... [63:48][47:32][31:16][15:0]
         */
        for( i = 0; i < BHSM_DATA_SECTION_LENGTH; i += 2 ) {
            pParam->data[i] = readData[BHSM_DATA_SECTION_LENGTH - i - 2];
            pParam->data[i + 1] = readData[BHSM_DATA_SECTION_LENGTH - i - 1];
        }
    }

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_OtpDataSection_Read );
    return rc;
}

BERR_Code BHSM_OTPPatternSequence_Program_priv( BHSM_Handle hHsm,
                                                bool bProgCacheReq )
{
    BERR_Code       rc = BERR_SUCCESS;
    BHSM_BspMsg_h   hMsg = NULL;
    uint8_t         status;
    unsigned        i;
    BHSM_BspMsgHeader_t header;

    BDBG_ENTER( BHSM_ProgramOTPPatternSequence );

    if( ( rc = BHSM_BspMsg_Create( hHsm, &hMsg ) ) != BERR_SUCCESS ) {
        return BERR_TRACE( rc );
    }

    BHSM_BspMsg_GetDefaultHeader( &header );
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eOTP_ProgPatternSequence, &header );

    i = 0;
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0xBC32F4AC );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0xD18616B6 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0x9FEB4D54 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0x4A27BF4A );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0xCF1C3178 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0xE2DB98A0 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0x24F64BBA );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0x7698E712 );
    BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0x0000F48D );

    if( bProgCacheReq ) {
        BHSM_BspMsg_Pack32( hMsg, BCMD_Otp_InCmdOtpProgPatternSequence_ePatternArray + ( i++ * 4 ), 0xDC11E0BF );
    }

    if( ( rc = BHSM_BspMsg_SubmitCommand( hMsg ) ) != BERR_SUCCESS ) {
        BERR_TRACE( rc );
        goto BHSM_P_DONE_LABEL;
    }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if( status != 0 ) {
        BDBG_ERR( ( "%s status[0x%02X]", BSTD_FUNCTION, status ) );
        rc = BHSM_STATUS_BSP_ERROR;
        goto BHSM_P_DONE_LABEL;
    }

  BHSM_P_DONE_LABEL:

    ( void ) BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_ProgramOTPPatternSequence );
    return rc;
}

/* Shall we make the CRC caculation from a lib? */
static void BHSM_OtpDataSectionCRC_Caculate( const unsigned char *data_section,
                                             unsigned char *crc )
{
    unsigned char   crcArray[32];
    int             i, j, k, m;
    unsigned char   c1, c2, c3, c4;
    unsigned char   bitDataArray[16];
    unsigned char   feedback;
    unsigned long   crcValue;
    double          X = 0;
    unsigned char   H[32] =
        { 0x1, 0x1, 0x1, 0x0, 0x1, 0x1, 0x0, 0x1,
          0x1, 0x0, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0,
          0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1,
          0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0 };

    BDBG_ENTER( BHSM_OtpDataSectionCRC_Caculate );

    for( i = 0; i < 32; i++ ) {
        crcArray[i] = 0x01;
    }

    for( m = 31; m > 0; m -= 2 ) {
        c1 = ( data_section[m - 1] >> 4 ) & 0xF;
        c2 = data_section[m - 1] & 0xF;
        c3 = ( data_section[m] >> 4 ) & 0xF;
        c4 = data_section[m] & 0xF;

        for( i = 4; i > 0; i-- ) {
            bitDataArray[4 - i] = ( c1 >> ( i - 1 ) ) & 0x01;
            bitDataArray[8 - i] = ( c2 >> ( i - 1 ) ) & 0x01;
            bitDataArray[12 - i] = ( c3 >> ( i - 1 ) ) & 0x01;
            bitDataArray[16 - i] = ( c4 >> ( i - 1 ) ) & 0x01;
        }

        for( j = 15; j >= 0; j-- ) {
            feedback = crcArray[31];
            for( k = 31; k > 0; k-- ) {
                if( H[k] != 0 )
                    crcArray[k] = bitDataArray[j] ^ crcArray[k - 1] ^ feedback;
                else
                    crcArray[k] = crcArray[k - 1];
            }
            crcArray[0] = bitDataArray[j] ^ feedback;
        }
    }

    crcValue = 0;
    X = 2;

    for( i = 0; i < 32; i++ ) {
        /* raise X (2) to the power of i */
        k = 1 << i;
        crcValue += ( crcArray[i] * k );
    }

    crc[0] = ( crcValue >> 24 ) & 0xFF;
    crc[1] = ( crcValue >> 16 ) & 0xFF;
    crc[2] = ( crcValue >> 8 ) & 0xFF;
    crc[3] = crcValue & 0xFF;

    BDBG_LEAVE( BHSM_OtpDataSectionCRC_Caculate );

    return;
}
