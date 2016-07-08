/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/

/* For debugging */
/* #define BUDP_DSS_GETUD_DUMP 1 */
#include "bstd.h"
#include "bdbg.h"
#include "budp_bitread.h"
#include "budp_dccparse.h"
#include "budp_dccparse_dss.h"
#include "bdbg.h"

BDBG_MODULE(BUDP_dss);

static const bool bByteswap = (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE);

static BERR_Code ParseDSSData_isr (
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_dss_cc_subtitle* pCCdata,
    uint8_t*                 psubtitle_count,
    BUDP_DCCparse_dss_cc_subtitle* pSubtitledata);
static size_t FindMpegUserdataStart_isr (
    BUDP_Bitread_Context* pREader, size_t length);

#ifdef BUDP_DSS_GETUD_DUMP
static void dss_dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info, size_t offset);
#endif

#ifdef DIRECTV_SKY
#define ATSC_CC_DATA_FLG            0x40
#define ATSC_CC_DATA_CNT            0x1f
#define ATSC_ADDITIONAL_DATA_FLG    0x20
#define ATSC_CC_DATA_VALID          0x04
#define ATSC_CC_DATA_TYPE           0x03
#define ATSC_CC_FIELD_1             0x00
#define ATSC_CC_FIELD_2             0x01
#define ATSC_CC_A53_IDENTIFIER(buf)\
    ((*(buf) == 0x47) && (*((buf)+1) == 0x41) && (*((buf)+2) == 0x39) && (*((buf)+3) == 0x34))

#define ATSC_CC_A53_ADDITIONAL_CODE(buf)\
    ((*(buf) == 0x0) && (*((buf)+1) == 0x0) && (*((buf)+2) == 0x01))

static BERR_Code ParseATSCA53Data (
    uint8_t     *pUserData,
    size_t      length,
    size_t      *pBytesParsedSub,
    uint8_t     *pcc_count,
    BUDP_DCCparse_dss_cc_subtitle* pCCdata
);
#endif /* DIRECTV_SKY */

/***************************************************************************
 * This function parses DSS SD user data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseDSSData_isr (
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_dss_cc_subtitle* pCCdata,
    uint8_t*                 psubtitle_count,
    BUDP_DCCparse_dss_cc_subtitle* pSubtitledata
)
{
    uint8_t userDataLength, userDataType, ccCount, i, subtitleCount;

    /* Start counting */
    *pcc_count = 0;
    *pBytesParsed = 0;
    ccCount = 0;
    *psubtitle_count = 0;
    subtitleCount = 0;

    /* We must have at least one length byte and a type byte */
    if (length < 2)
    {
        return BERR_BUDP_NO_DATA;;
    }

    /* Read 8-bit length field */
    userDataLength = BUDP_Bitread_Byte_isr (pReader);
    (*pBytesParsed)++;

    if (userDataLength > length)
    {
        /* The userDataLength is somehow corrupted */
        return BERR_BUDP_PARSE_ERROR;
    }

    BDBG_MSG((" userDataLength is %d", userDataLength));
    while(userDataLength != 0)
    {
        /* Read 8-bit type field */
        userDataType = BUDP_Bitread_Byte_isr (pReader);
        BDBG_MSG((" userDataType is %d", userDataType));

        (*pBytesParsed)++;

        /* Type can be 2 bytes if first byte is 0xFF */
        if(userDataType == 0xFF)
        {
            userDataType = BUDP_Bitread_Byte_isr (pReader);
            (*pBytesParsed)++;
        }

        if((userDataType == 9 || userDataType == 10))
        {
           if (userDataLength != 3)
           {
                /* The userDataLength is somehow corrupted */
                /* It is out of spec. Return error. */
                return BERR_BUDP_PARSE_ERROR;
            }

            ccCount = (userDataLength - 1) / 2;

            /* All DirecTV SD CC data is intended for top field display. In the
             * case of 3:2 pulldown, we may actually end up with 2 pairs of CC
             * data, which needs to get spread across 2 successive frames when
            * the fields are repeated.
            */
            for(i=0;i<ccCount;i++)
            {
                pCCdata[*pcc_count].eCCType = BUDP_DCCparse_CC_Dss_Type_ClosedCaption;
                pCCdata[*pcc_count].bIsAnalog = true;

                /* All DirecTV SD CC data is intended for top field display, */
                /* while EDS data is intended for bottom field display.      */
                if(userDataType == 9)
                {
                pCCdata[*pcc_count].polarity = BAVC_Polarity_eTopField;
                pCCdata[*pcc_count].language_type = 0;
                }
                else
                {
                    pCCdata[*pcc_count].polarity = BAVC_Polarity_eBotField;
                    pCCdata[*pcc_count].language_type = 1;
                }

                /* cc_data_1(8) or eds_byte_1(8) */
                pCCdata[*pcc_count].cc_data_1 =
                    BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;

                /* cc_data_2(8) or eds_byte_1(8) */
                pCCdata[*pcc_count].cc_data_2 =
                    BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;

                BDBG_MSG((" ccdata1 is 0x%x, ccdata2 is 0x%x",
                    pCCdata[*pcc_count].cc_data_1,
                    pCCdata[*pcc_count].cc_data_2));
                (*pcc_count)++;
            }
        }
        else if(userDataType == 0x10)
        {
            /*Subtitle for GLA Project "Subtitling specification for GLA Project V2.2.pdf" */
            int i;
            BDBG_MSG((" userDataLength is %d", userDataLength));

            if (userDataLength > 13)
            {
                /* The userDataLength is somehow corrupted */
                /* the maximum number of subtitle language is 4, so the maximum of userdata_length =  No of Sub_title_language_type*3+1=4*3+1=13*/
                return BERR_BUDP_PARSE_ERROR;
            }

            subtitleCount = (userDataLength - 1) / 3;
            for(i=0; i<subtitleCount;i++)
            {
                pSubtitledata[*psubtitle_count].eCCType = BUDP_DCCparse_CC_Dss_Type_Subtitle;
                pSubtitledata[*psubtitle_count].bIsAnalog = false;
                pSubtitledata[*psubtitle_count].language_type =
                    BUDP_Bitread_Byte_isr (pReader) & 0x7f; /*7 bits for this field language type*/
                (*pBytesParsed)++;

                /* subtitle_data_1(8) */
                pSubtitledata[*psubtitle_count].cc_data_1 =
                    BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;

                /* subtitle_data_2(8) */
                pSubtitledata[*psubtitle_count].cc_data_2 =
                    BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;

                BDBG_MSG((" subtitle_language_type is 0x%x",
                    pSubtitledata[*psubtitle_count].language_type));
                BDBG_MSG((" subtitle_data_1 is 0x%x",
                    pSubtitledata[*psubtitle_count].cc_data_1));
                BDBG_MSG((" subtitle_data_2 is 0x%x",
                    pSubtitledata[*psubtitle_count].cc_data_2));

                (*psubtitle_count)++;
            }

        }
        else
        {
            if (userDataLength > length)
            {
                /* The userDataLength is somehow corrupted */
                /* Return an error */
                return BERR_BUDP_PARSE_ERROR;
            }
            else
            {
            /* Skip over non-CC data */
            for(i=0;i<(userDataLength-1);i++)
                BUDP_Bitread_Byte_isr (pReader);
            *pBytesParsed += (userDataLength-1);
              }
        }

        /* Read 8-bit length field of next chunk of user data */
        if(*pBytesParsed < length)
        {
            userDataLength = BUDP_Bitread_Byte_isr (pReader);
            (*pBytesParsed)++;
        }
        else
            userDataLength = 0;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BUDP_DCCparse_DSS_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    size_t                   offset,
    size_t*                  pBytesParsed,
    uint8_t*    pcc_count,
    BUDP_DCCparse_dss_cc_subtitle*  pCCdata,
    uint8_t*                 psubtitle_count,
    BUDP_DCCparse_dss_cc_subtitle* pSubtitledata
)
{
    size_t bytesParsedSub;
    uint32_t length;
    uint8_t* userdata;
    BERR_Code eErr;
    BUDP_Bitread_Context reader;
    BUDP_Bitread_Context savedReader;

    BDBG_ENTER(BUDP_DCCparse_DSS_isr);

    /* Check for obvious errors from user */
    if ((pUserdata_info == 0x0) ||
        (pBytesParsed   == 0x0) ||
        (pcc_count      == 0x0) ||
        (pCCdata        == 0x0)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Programming note:  all function parameters are now validated */
#ifdef BUDP_DSS_GETUD_DUMP
    dss_dump_getud_isr (pUserdata_info, offset);
#endif

    /* Take care of a special case */
    userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    length   = pUserdata_info->ui32UserDataBufSize - offset;
    if (length < 4)
    {
        *pBytesParsed = length;
        return BERR_BUDP_NO_DATA;
    }

    /* Prepare to play with bits */
    BUDP_Bitread_Init_isr (&reader, bByteswap, userdata);

    /* jump past the first MPEG userdata start code */
    bytesParsedSub = FindMpegUserdataStart_isr (&reader, length);
    *pBytesParsed = bytesParsedSub;
    length -= bytesParsedSub;
    savedReader = reader;

    /* If we did not find a start code, bail out now */
    if (length == 0)
    {
        return BERR_BUDP_NO_DATA;
    }

    reader = savedReader;

#ifdef DIRECTV_SKY
    /* Process ATSC A/53 encoded user data? */
    if ((length >= 4) && ATSC_CC_A53_IDENTIFIER(userdata+bytesParsedSub))
    {
        eErr = ParseATSCA53Data(userdata, length, &bytesParsedSub, pcc_count, pCCdata);
    }
    else
#endif /* DIRECTV_SKY */

    eErr = ParseDSSData_isr (
        &reader, length, &bytesParsedSub, pcc_count, pCCdata, psubtitle_count, pSubtitledata);
    switch (eErr)
    {
        case BERR_SUCCESS:
        case BERR_BUDP_PARSE_ERROR:
            *pBytesParsed += bytesParsedSub;
            BDBG_LEAVE(BUDP_DCCparse_isr);
            return eErr;
            break;
        case BERR_BUDP_NO_DATA:
            break;
        default:
            /* Programming error */
            BDBG_ASSERT (false);
            break;
    }

    BDBG_LEAVE(BUDP_DCCparse_DSS_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 * This function finds the next userdata startcode 0x000001B2.  It
 * indicates the byte following this startcode by its return value.
 * If no startcode was found, it simply returns the length of the
 * input data.
 */
static size_t FindMpegUserdataStart_isr (
    BUDP_Bitread_Context* pReader, size_t length)
{
    size_t count = 0;
    uint8_t saved[4];

    /* Special case (failure) */
    if (length < 4)
        return length;

    /* Initialize */
    saved[1] = BUDP_Bitread_Byte_isr (pReader);
    saved[2] = BUDP_Bitread_Byte_isr (pReader);
    saved[3] = BUDP_Bitread_Byte_isr (pReader);

    while (length >= 4)
    {
        /* Read in another byte */
        saved[0] = saved[1];
        saved[1] = saved[2];
        saved[2] = saved[3];
        saved[3] = BUDP_Bitread_Byte_isr (pReader);

        if ((saved[0] == 0x00) &&
            (saved[1] == 0x00) &&
            (saved[2] == 0x01) &&
            (saved[3] == 0xB2)    )
        {
            /* Found it! */
            break;
        }

        /* proceed to the next byte */
        --length;
        ++count;
    }

    if (length >= 4)
    {
        /* found the pattern before the end of stream */
        return count + 4;
    }
    else
    {
        /* Didn't find any start code */
        return count + 3;
    }
}

#ifdef BUDP_DSS_GETUD_DUMP
static void dss_dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info, size_t offset)
{
    unsigned int iByte;
    uint8_t* userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    uint32_t length   = pUserdata_info->ui32UserDataBufSize - offset;

    BDBG_MSG(("PS %d TFF %d RFF %d",
        pUserdata_info->eSourcePolarity,
        pUserdata_info->bTopFieldFirst,
        pUserdata_info->bRepeatFirstField));
    BDBG_MSG( ("User Data BYTES %d", length));
    for (iByte = 0 ; iByte < length ; ++iByte)
    {
        BDBG_MSG( ("%02x", (int)userdata[iByte]));
        if ((iByte % 16) == 15)
            BDBG_MSG( (" "));
        else
            BDBG_MSG( (" "));
    }
    if ((iByte % 16) != 15)
        BDBG_MSG((" "));

}
#endif


#ifdef DIRECTV_SKY

/***************************************************************************
 * This function parses ATSC SD user data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read before calling this function. The pBytesParseSub should
 * contains the offset to the ATSC identifier.
 */
static BERR_Code ParseATSCA53Data (
    uint8_t     *pUserData,
    size_t      length,
    size_t      *pBytesParsedSub,
    uint8_t     *pcc_count,
    BUDP_DCCparse_dss_cc_subtitle* pCCdata
)
{
    uint8_t  *pCurBuffer;
    uint8_t   u8SkyCCCnt=0;
    uint8_t   u8CCIndx;
    uint8_t   u8CurBufferOffset, u8CCInfo;
    uint8_t   u8CCType;
    uint8_t   u8AdditionalDataFlag;
    BERR_Code status = BERR_BUDP_PARSE_ERROR;

    /* Check for obvious errors from user */
    if ((pUserData == NULL) ||
        (pBytesParsedSub   == NULL) ||
        (pcc_count      == NULL) ||
        (pCCdata        == NULL)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Sanity check atleast length after the startcode >= 5 */
    if (length < 5)
    {
        *pBytesParsedSub = length;
        return BERR_BUDP_NO_DATA;
    }

    /* Skip the ATSC identifier */
    *pBytesParsedSub += 4;
    pCurBuffer = pUserData + (*pBytesParsedSub);

    /* Skip user data type byte */
    *pBytesParsedSub += 1;

    /* Adjust length: ATSC identifier + user data type byte */
    length -= 5;

    /* Check for user data type code */
    u8CurBufferOffset = 0;
    if ((pCurBuffer[u8CurBufferOffset] == 0x03) && (length >= 2))
    {
        /* Read the cc info byte */
        u8CCInfo = pCurBuffer[u8CurBufferOffset+1];

        /* Skip EMM Data & CC info byte */
        *pBytesParsedSub += 2;
        u8CurBufferOffset += 3;
        length -= 2;

        /* Check if need to process CC data */
        u8AdditionalDataFlag = u8CCInfo & ATSC_ADDITIONAL_DATA_FLG;
        if (u8CCInfo & ATSC_CC_DATA_FLG)
        {
            /* CC counts */
            u8SkyCCCnt = u8CCInfo & ATSC_CC_DATA_CNT;

            /* Reading CC data */
            for (u8CCIndx=0; (u8CCIndx<u8SkyCCCnt) && (length >= 3); u8CCIndx++)
            {
                /* CC valid ? */
                u8CCType = pCurBuffer[u8CurBufferOffset] & ATSC_CC_DATA_TYPE;
                if (pCurBuffer[u8CurBufferOffset] & ATSC_CC_DATA_VALID)
                {
                    pCCdata[*pcc_count].eCCType = BUDP_DCCparse_CC_Dss_Type_ClosedCaption;
                    pCCdata[*pcc_count].bIsAnalog = true;

                    /* All DirecTV SD CC data is intended for top field display, */
                    /* while EDS data is intended for bottom field display.      */
                    if (u8CCType == ATSC_CC_FIELD_1)
                    {
                        pCCdata[*pcc_count].polarity = BAVC_Polarity_eTopField;
                        pCCdata[*pcc_count].language_type = 0;
                    }
                    else if (u8CCType == ATSC_CC_FIELD_2)
                    {
                        pCCdata[*pcc_count].polarity = BAVC_Polarity_eBotField;
                        pCCdata[*pcc_count].language_type = 1;
                    }
                    else
                    {
                        BDBG_MSG(("[CC-DRV] BUDP_DCCparse_DSS: invalid Closed Caption type = 0x%x", u8CCType));
                        break;
                    }

                    /* cc_data_1(8) or eds_byte_1(8) */
                    pCCdata[*pcc_count].cc_data_1 = pCurBuffer[u8CurBufferOffset+1];

                    /* cc_data_2(8) or eds_byte_1(8) */
                    pCCdata[*pcc_count].cc_data_2 = pCurBuffer[u8CurBufferOffset+2];

                    (*pcc_count)++;
                }

                /* Adjust byte parsed counter */
                *pBytesParsedSub += 3;
                length -= 3;
                u8CurBufferOffset += 3;
            }
        }

        /* Skip Marker Byte */
        *pBytesParsedSub += 1;
        length -= 1;
        u8CurBufferOffset += 1;

        /* Check for additional data flag */
        if (u8AdditionalDataFlag)
        {
            BDBG_MSG(("[CC-DRV] BUDP_DCCparse_DSS: ATSC Additional Data Flag is set...... TODO TODO HERE...."));
            pCurBuffer = pUserData + *pBytesParsedSub;
            while (length >= 3 && ATSC_CC_A53_ADDITIONAL_CODE(pCurBuffer))
            {
                /* TODO: Parser unsupported additional data */
                length -= 4;
                *pBytesParsedSub += 4;
            }
        }

        /* Adjust pBytesParsedSub not including the 5-bytes userdata start code */
        if (*pBytesParsedSub >= 5) *pBytesParsedSub -= 5;

        status = BERR_SUCCESS;
    }
    else
    {
        BDBG_MSG(("[CC-DRV] BUDP_DCCparse_DSS: Invalid User Data Type Code for ATSC A53 = 0x%x length[%d]", pCurBuffer[u8CurBufferOffset], length));
        *pBytesParsedSub += length;
        status = BERR_BUDP_NO_DATA;
    }

    return status;

}
#endif /* DIRECTV_SKY */
