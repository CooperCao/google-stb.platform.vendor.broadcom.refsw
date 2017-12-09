/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhsm_bsp_msg.h"
#include "bhsm_p_otpdatasection.h"
#include "bhsm_otp_datasection.h"
#include "bhsm_otp_msp.h"
#include "bhsm_p_otpmisc.h"

BDBG_MODULE( BHSM );

#define SHA160_BYTE_SIZE     (20)
#define BHSM_CRC_DATA_LEN    (4)


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

BERR_Code BHSM_OtpDataSection_Write( BHSM_Handle hHsm, const BHSM_DataSectionWrite *pParam )
{
    BHSM_P_OtpDataSectionProg bspParam;
    BHSM_P_OtpMiscProgPatternSet bspPattern;
    uint8_t dataCrc[BHSM_CRC_DATA_LEN];
    unsigned i, j;
    BERR_Code rc;

    BKNI_Memset( &bspPattern, 0, sizeof(bspPattern) );
    bspPattern.in.patternArray[0] = 0xBC32F4AC;
    bspPattern.in.patternArray[1] = 0xD18616B6;
    bspPattern.in.patternArray[2] = 0x9FEB4D54;
    bspPattern.in.patternArray[3] = 0x4A27BF4A;
    bspPattern.in.patternArray[4] = 0xCF1C3178;
    bspPattern.in.patternArray[5] = 0xE2DB98A0;
    bspPattern.in.patternArray[6] = 0x24F64BBA;
    bspPattern.in.patternArray[7] = 0x7698E712;
    bspPattern.in.patternArray[8] = 0x0000F48D;
    rc = BHSM_P_OtpMisc_ProgPatternSet( hHsm, &bspPattern );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspParam, 0, sizeof(bspParam) );

    bspParam.in.dataSectionIndex = pParam->index;
    BHSM_OtpDataSectionCRC_Caculate( pParam->data, dataCrc );
    BHSM_MemcpySwap( &bspParam.in.dataSectionCrc, dataCrc, 4 );
    bspParam.in.numErrorsMax = 0;

    /* app-to-hsm: 32-byte-array in big endian
     * hsm-to-BSP:  a particular wrapping to help BSP word0/LSB, word7/MSB, word=[15:0] ||[31:16] */
    j = BHSM_DATA_SECTION_LENGTH / 4 - 1;
    for( i = 0; i < BHSM_DATA_SECTION_LENGTH; i += 4, j-- ) {
        bspParam.in.dataSectionData[j] = ( ( pParam->data[i] << 8 ) |
                    ( pParam->data[i + 1] )  |
                    ( pParam->data[i + 2] << 24 ) |
                    ( pParam->data[i + 3] << 16 ) );
    }

    rc = BHSM_P_OtpDataSection_Prog( hHsm, &bspParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    return BERR_SUCCESS;

    return 0;
}

BERR_Code BHSM_OtpDataSection_Read( BHSM_Handle hHsm, BHSM_DataSectionRead *pParam )
{
    BHSM_OtpMspRead readParam;
    BHSM_P_OtpDataSectionRead bspParam;
    BERR_Code rc;

    BKNI_Memset( &readParam, 0, sizeof(readParam) );
    readParam.index = 96;

    /* Read MSP bits to check if the coresponding data section is read protected. */
    rc = BHSM_OtpMsp_Read( hHsm, &readParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspParam, 0, sizeof(bspParam) );

    bspParam.in.dataSectionNum = pParam->index;
    rc = BHSM_P_OtpDataSection_Read( hHsm, &bspParam );
    if( rc != BERR_SUCCESS ) {
        BDBG_ERR(("%s Can't read DATASECTION-OTP[%d]", BSTD_FUNCTION, pParam->index ));
        return BERR_TRACE( rc );
    }

    /* Check if the MSP bit indicates the DS being readable. */
      readParam.data &= readParam.valid;
      pParam->accessible = !( (readParam.data) & ( 1 << pParam->index ) );

      /* Clear up the high order numbers to make sure the correct output. */
      BKNI_Memset( pParam->data, 0, BHSM_DATA_SECTION_LENGTH );
      if( !pParam->accessible ) {
          /*
           * Convert from BSP data format to 20-byte big endian array:
           * BSP format:                           [159:128][127:96][95:64][63:32][31:0]
           * Output format: [255:240][239:224] ... [159:128][127:96][95:64][63:32][31:0]
           */
          BKNI_Memcpy(&pParam->data[BHSM_DATA_SECTION_LENGTH - SHA160_BYTE_SIZE], bspParam.out.dataSectionData, SHA160_BYTE_SIZE);
      }
      else {
          uint8_t *readData = (uint8_t *)bspParam.out.dataSectionData;
          unsigned        i;
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

    return BERR_SUCCESS;

}
