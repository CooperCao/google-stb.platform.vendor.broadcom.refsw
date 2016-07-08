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
/* #define BUDP_P_GETUD_DUMP 1 */
#ifdef BUDP_P_GETUD_DUMP
static const char* BUDP_P_Getud_Filename = "userdata.getud";
#include <stdio.h>
#endif

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "budp.h"
#include "budp_bitread.h"
#include "budp_dccparse.h"

BDBG_MODULE(BUDP);


/***************************************************************************
* Private data types
***************************************************************************/

/* Just used to switch among three types of internal parsers */
typedef enum {
    ParserType_eMpeg,     /* Good old MPEG-2 userdata.       */
    ParserType_eSEI       /* Userdata, older AVC flavor (SEI). */
} ParserType;

typedef struct {
    BUDP_DCCparse_Format type;
    BERR_Code (*parser) (
         const BAVC_USERDATA_info*     pUserdata_info,
        BUDP_Bitread_Context* pReader,
        size_t                   length,
        size_t*                  pBytesParsed,
        uint8_t*                 pcc_count,
        BUDP_DCCparse_ccdata* pCCdata
    );
} DigitalParser;


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static BERR_Code BUDP_P_DCCparse_isr (
    ParserType               eParserType,
    const BAVC_USERDATA_info*      pUserdata_info,
    size_t                   offset,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata);
static size_t FindMpegUserdataStart_isr (
    BUDP_Bitread_Context* pREader, size_t length);
static size_t FindSeiUserdataStart_isr (
    BUDP_Bitread_Context* pREader, size_t length);
static BERR_Code ParseDVS157Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata);
static BERR_Code ParseATSC53Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata);
static BERR_Code ParseDVS053Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata);
static BERR_Code ParseSEIData_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
);
static BERR_Code ParseSEIData2_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
);

static int swap_bits_isr (int src);

#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info, size_t offset);
#endif

/*
 * This is the collection of methods for setting field polarity (top vs.
 * bottom) for closed caption data.
 */
/* Uses only cc_type attribute */
static void SetFieldsParity_SIMPLE_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info);
/* Uses cc_type attribute and "top field first" attribute. */
static void SetFieldsParity_TFF_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info);
/* Uses only order of data presented. */
static void SetFieldsParity_NOCCTYPE_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info);
/* Uses field_number attribute */
static void SetFieldsParity_FIELDNUMBER_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info);
/* Special method just for CC data from SEI */
static void SetFieldsParity_SEI_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info);

/*
 * These macros determine which field polarity method (see above paragraph)
 * to use for parsing ATSC, DVS, and SEI closed caption data. You can override
 * these choices from the compiler command line. Make a choice from the above
 * paragraph.
 */
#ifndef BUDP_SETFIELDSPARITY_ATSC_isr
#define BUDP_SETFIELDSPARITY_ATSC_isr SetFieldsParity_SIMPLE_isr
#endif
#ifndef BUDP_SETFIELDSPARITY_DVS_isr
#define BUDP_SETFIELDSPARITY_DVS_isr SetFieldsParity_FIELDNUMBER_isr
#endif
#ifndef BUDP_SETFIELDSPARITY_SEI_isr
#define BUDP_SETFIELDSPARITY_SEI_isr SetFieldsParity_SEI_isr
#endif


/***************************************************************************
* Static data (tables, etc.)
***************************************************************************/

/* The array of digital CC data parsers.  It does not include the SEI parser.
 * The SEI parser must be handled as a special case.  This is because this
 * software collection cannot discover on its own that it is dealing with SEI
 * data.  This software _can_ discriminate between the other forms of CC data,
 * listed in the following array:
 */
static const DigitalParser s_mpegParsers[] = {
    {BUDP_DCCparse_Format_DVS157, ParseDVS157Data_isr},
    {BUDP_DCCparse_Format_ATSC53, ParseATSC53Data_isr},
    {BUDP_DCCparse_Format_DVS053, ParseDVS053Data_isr},
};
#define NUM_MPEG_PARSERS (sizeof(s_mpegParsers) / sizeof(s_mpegParsers[0]))

static const DigitalParser s_seiParsers[] = {
    {BUDP_DCCparse_Format_SEI, ParseSEIData_isr},
    {BUDP_DCCparse_Format_SEI2, ParseSEIData2_isr},
};
#define NUM_SEI_PARSERS (sizeof(s_seiParsers) / sizeof(s_seiParsers[0]))

static const bool bByteswap = (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE);


/***************************************************************************
* Implementation of "BUDP_DCCparse_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BUDP_DCCparse_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    size_t                   offset,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    return
        BUDP_P_DCCparse_isr (
            ParserType_eMpeg, pUserdata_info, offset,
            pBytesParsed, pcc_count, pCCdata);
}

/***************************************************************************
 *
 */
BERR_Code BUDP_DCCparse_SEI_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    size_t                   offset,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    return
        BUDP_P_DCCparse_isr (
            ParserType_eSEI, pUserdata_info, offset,
            pBytesParsed, pcc_count, pCCdata);
}


/***************************************************************************
* Implementation of private (static) functions
***************************************************************************/

/***************************************************************************
 *
 */
static BERR_Code BUDP_P_DCCparse_isr (
    ParserType               eParserType,
    const BAVC_USERDATA_info*      pUserdata_info,
    size_t                   offset,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    size_t bytesParsedSub;
    uint32_t length;
    uint8_t* userdata;
    unsigned int iparser;
    BERR_Code eErr;
    BUDP_Bitread_Context reader;
    BUDP_Bitread_Context savedReader;

    BDBG_ENTER(BUDP_DCCparse);

    /* Check for obvious errors from user */
    if ((pUserdata_info == 0x0) ||
        (pBytesParsed   == 0x0) ||
        (pcc_count      == 0x0) ||
        (pCCdata        == 0x0)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Programming note:  all function parameters are now validated */


#ifdef BUDP_P_GETUD_DUMP
    dump_getud_isr (pUserdata_info, offset);
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

    switch (eParserType)
    {
    case ParserType_eMpeg:

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

        /*********************************************************************\
        * Programming note:  each of the parsers will give up and return
        * BERR_BUDP_NO_DATA if it finds the wrong identifier byte.
        \*********************************************************************/

        /* Try all the available parsers */
        for (iparser = 0 ; iparser < NUM_MPEG_PARSERS ; ++iparser)
        {
            reader = savedReader;
            eErr = (*s_mpegParsers[iparser].parser) (
                pUserdata_info, &reader, length,
                &bytesParsedSub, pcc_count, pCCdata);
            switch (eErr)
            {
            case BERR_SUCCESS:
                /* Sarnoff compliance check */
                if ((pUserdata_info->eUserDataType == BAVC_USERDATA_Type_eSeq)
                    ||
                    (pUserdata_info->eUserDataType == BAVC_USERDATA_Type_eGOP))
                {
                    eErr = BERR_BUDP_SOURCE_CHECK;
                }
                /* fall into the next case */
            case BERR_BUDP_PARSE_ERROR:
                *pBytesParsed += bytesParsedSub;
                BDBG_LEAVE(BUDP_DCCparse);
                return eErr;
                break;
            case BERR_BUDP_NO_DATA:
                break;
            default:
                /* Programming error */
                BDBG_ASSERT (false);
                break;
            }
        }
        break;

    case ParserType_eSEI:

        /* jump past the first SEI userdata start code */
        bytesParsedSub = FindSeiUserdataStart_isr (&reader, length);
        *pBytesParsed = bytesParsedSub;
        length -= bytesParsedSub;
        savedReader = reader;

        /* If we did not find a start code, bail out now */
        if (length == 0)
        {
            return BERR_BUDP_NO_DATA;
        }

        /*********************************************************************\
        * Programming note:  each of the parsers will give up and return
        * BERR_BUDP_NO_DATA if it finds the wrong identifier byte.
        \*********************************************************************/

        /* Try all available parsers */
        for (iparser = 0 ; iparser < NUM_SEI_PARSERS ; ++iparser)
        {
            reader = savedReader;
            eErr = (*s_seiParsers[iparser].parser) (
                pUserdata_info, &reader, length,
                &bytesParsedSub, pcc_count, pCCdata);
            switch (eErr)
            {
                case BERR_SUCCESS:
                case BERR_BUDP_PARSE_ERROR:
                    *pBytesParsed += bytesParsedSub;
                    BDBG_LEAVE(BUDP_DCCparse);
                    return eErr;
                    break;
                case BERR_BUDP_NO_DATA:
                    break;
                default:
                    /* Programming error */
                    BDBG_ASSERT (false);
                    break;
            }
        }
    break;

    default:
        BDBG_ERR (("Programming error"));
        BDBG_ASSERT (0);
        break;

    }

    /* Programming note:
     * None of the parsers liked this bit of userdata.
     * It would be more elegant to skip to the next start code
     * before returning.  But it is more efficient to simply
     * stop parsing. */

    /* No userdata was found */
    BDBG_LEAVE(BUDP_DCCparse);
    return BERR_BUDP_NO_DATA;
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

/***************************************************************************
 * This function finds the next userdata startcode 0x00000106.  It
 * indicates the byte following this startcode by its return value.
 * If no startcode was found, it simply returns the length of the
 * input data.
 */
static size_t FindSeiUserdataStart_isr (
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
            (saved[3] == 0x06)    )
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

/* TODO: figure out where to put this */
#define MPEG_DVS157_VBI_DATA_FLAG 0x03

/***************************************************************************
 * This function parses DVS 157 data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseDVS157Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    size_t bitsParsed;
    int icount;

    /* Special case */
    if (length == 0)
    {
        *pcc_count = 0;
        return BERR_BUDP_PARSE_ERROR;
    }

    /* user_data_type_code(8) */
    if (BUDP_Bitread_Byte_isr (pReader) != MPEG_DVS157_VBI_DATA_FLAG)
    {
        /* not DVS 157 data */
        *pBytesParsed = 1;
        *pcc_count = 0;
        return BERR_BUDP_NO_DATA;
    }

    /* Start things off by recognizing that we have parsed one byte.
       If an error occurs now, don't bother updating the count of
       parsed bytes.
    */
    --length;
    *pBytesParsed = 1;

    if (length == 0)
    {
        /* Packet is too short */
        *pcc_count = 0;
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Skip '1000 000' bits (7) */
    /* vbi_data_flag(1) */
    if (!(BUDP_Bitread_Byte_isr (pReader) & 0x01))
    {
        /* No data for us.  Prevent other parsers from trying though. */
        *pcc_count = 0;
        return BERR_SUCCESS;
    }

    --length;
    *pBytesParsed += 1;
    if (length == 0)
    {
        /* Packet is too short */
        *pcc_count = 0;
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Start the bit level parsing */
    *pcc_count = BUDP_Bitread_Read_isr (pReader, 5);
    bitsParsed = 5;

    for (icount = 0 ; icount < *pcc_count ; ++icount)
    {
       BKNI_Memset( &pCCdata[icount], 0, sizeof(pCCdata[icount]));

        /* Each sub-packet of data is 26 bits */
        if (length < 4)
        {
            /* Minimum packet length */
            return BERR_BUDP_PARSE_ERROR;
        }

        /* Not part of DVS157 standard! */
        pCCdata[icount].cc_valid = 1;

        pCCdata[icount].cc_priority = BUDP_Bitread_Read_isr (pReader, 2);

        pCCdata[icount].seq.field_number = BUDP_Bitread_Read_isr (pReader, 2);

        pCCdata[icount].line_offset = BUDP_Bitread_Read_isr (pReader, 5);

        pCCdata[icount].cc_data_1 =
            swap_bits_isr(BUDP_Bitread_Byte_isr (pReader));
        pCCdata[icount].cc_data_2 =
            swap_bits_isr(BUDP_Bitread_Byte_isr (pReader));
        /* marker bit(1) */
        (void)BUDP_Bitread_Read_isr (pReader, 1);

        /* Check for length violation again */
        /* bitsParsed += 26; */
        if (bitsParsed < 6)
        {
            length -= 3;
            *pBytesParsed += 3;
            bitsParsed += 2;
        }
        else
        {
            length -= 4;
            *pBytesParsed += 4;
            bitsParsed -= 6;
        }
        pCCdata[icount].format = BUDP_DCCparse_Format_DVS157;
    }

    BUDP_SETFIELDSPARITY_DVS_isr (pCCdata, *pcc_count, pUserdata_info);
    return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses DVS 053 data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseDVS053Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    unsigned int cc_count;
    unsigned int icount;
    int data_found;

    /* Start counting */
    *pcc_count = 0;

    /* Special case */
    if (length < 4)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* ATSC identifier (4) */
    if ((BUDP_Bitread_Byte_isr (pReader) != 0x47) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x41) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x39) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x34)   )
    {
        /* not DVS 053 data */
        *pBytesParsed = 4;
        return BERR_BUDP_NO_DATA;
    }

    /* Start things off by recognizing that we have parsed four bytes.
       If an error occurs now, don't bother updating the count of
       parsed bytes.
    */
    length -= 4;
    *pBytesParsed = 4;

    if (length == 0)
    {
        /* Empty! */
        return BERR_BUDP_PARSE_ERROR;
    }

    /* user_data_type_code(8) */
    if (BUDP_Bitread_Byte_isr (pReader) != 0x04)
    {
        /* not DVS 053 data */
        *pBytesParsed = 5;
        return BERR_BUDP_NO_DATA;
    }

    --length;
    *pBytesParsed += 1;
    if (length == 0)
    {
        /* Packet too short */
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Skip marker bits(3) and dig out the count of cc data pairs(5) */
    cc_count = BUDP_Bitread_Byte_isr (pReader) & 0x1F;

    /* This will be the last length check */
    --length;
    *pBytesParsed += 1;
    if (length < (3 * cc_count))
    {
        /* Packet too short */
        return BERR_BUDP_PARSE_ERROR;
    }
    *pBytesParsed += (3 * cc_count);

    /* Dig out the closed caption data pairs */
    data_found = 0;
    for (icount = 0 ; icount < cc_count ; ++icount)
    {
       /* additional_cc_valid(1) */
       uint32_t additional_cc_valid = BUDP_Bitread_Read_isr (pReader, 1);

       BKNI_Memset( &pCCdata[data_found], 0, sizeof(pCCdata[data_found]));

       pCCdata[data_found].cc_valid = additional_cc_valid;

        /* additional_cc_line_offset(5) */
        pCCdata[data_found].line_offset = BUDP_Bitread_Read_isr (pReader, 5);

        /* additional_cc_field_number(2) */
        pCCdata[data_found].seq.field_number =
            BUDP_Bitread_Read_isr (pReader, 2);

        /* additional_cc_data_1(8) */
        pCCdata[data_found].cc_data_1 =
            BUDP_Bitread_Byte_isr (pReader);

        /* additional_cc_data_2(8) */
        pCCdata[data_found].cc_data_2 =
            BUDP_Bitread_Byte_isr (pReader);

        pCCdata[data_found].format = BUDP_DCCparse_Format_DVS053;

        /* Update count of items written to output data */
        ++data_found;
    }

    BUDP_SETFIELDSPARITY_DVS_isr (pCCdata, *pcc_count, pUserdata_info);

    *pcc_count = data_found;
    return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses ATSC 53 data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseATSC53Data_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    unsigned int cc_count;
    unsigned int cc_valid;
    unsigned int icount;
    int data_found;

    BSTD_UNUSED (pUserdata_info);

    /* Start counting */
    *pcc_count = 0;

    /* Start travelling pointer into data */
    if (length < 4)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* ATSC identifier (32) */
    if ((BUDP_Bitread_Byte_isr (pReader) != 0x47) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x41) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x39) ||
        (BUDP_Bitread_Byte_isr (pReader) != 0x34)   )
    {
        /* not ATSC-53 data */
        *pBytesParsed = 4;
        return BERR_BUDP_NO_DATA;
    }

    /* Start things off by recognizing that we have parsed four bytes.
       If an error occurs now, don't bother updating the count of
       parsed bytes.
    */
    length -= 4;
    *pBytesParsed = 4;

    if (length == 0)
    {
        /* Empty! */
        return BERR_BUDP_PARSE_ERROR;
    }

    /* user_data_type_code(8) */
    if (BUDP_Bitread_Byte_isr (pReader) != 0x03)
    {
        /* not ATSC-53 data */
        *pBytesParsed = 5;
        return BERR_BUDP_NO_DATA;
    }

    --length;
    *pBytesParsed = 5;
    if (length == 0)
    {
        /* Packet too short */
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Skip process_em_data_flag(1) */
    (void)BUDP_Bitread_Read_isr (pReader, 1);

    /* Dig out process_cc_data_flag(1) */
    if (!BUDP_Bitread_Read_isr (pReader, 1))
    {
        /* No closed caption data to process */
        *pBytesParsed = 6;
        return BERR_BUDP_NO_DATA;
    }

    /* Skip additional_data_flag(1) */
    (void)BUDP_Bitread_Read_isr (pReader, 1);

    /* Dig out the count of cc data pairs */
    cc_count = BUDP_Bitread_Read_isr (pReader, 5);

    --length;
    *pBytesParsed = 6;
    if (length == 0)
    {
        /* Packet too short */
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Skip em_data(8) */
    (void)BUDP_Bitread_Byte_isr (pReader);

    /* This will be the last length check */
    --length;
    *pBytesParsed = 7;
    if (length < (3 * cc_count))
    {
        /* Packet too short */
        return BERR_BUDP_PARSE_ERROR;
    }
    *pBytesParsed += (3 * cc_count);

    /* Dig out the closed caption data pairs */
    data_found = 0;
    for (icount = 0 ; icount < cc_count ; ++icount)
    {
       /* Skip marker bits(5) */
       (void)BUDP_Bitread_Read_isr (pReader, 5);

       BKNI_Memset( &pCCdata[data_found], 0, sizeof(pCCdata[data_found]));

       /* cc_valid(1) */
        cc_valid = BUDP_Bitread_Read_isr (pReader, 1);
        pCCdata[data_found].cc_valid = cc_valid;

        /* cc_type(2) */
        pCCdata[data_found].seq.cc_type = BUDP_Bitread_Read_isr (pReader, 2);

        /* Kludge */
        pCCdata[data_found].line_offset = 11;

        /* additional_cc_data_1(8) */
        pCCdata[data_found].cc_data_1 =
            BUDP_Bitread_Byte_isr (pReader);

        /* additional_cc_data_2(8) */
        pCCdata[data_found].cc_data_2 =
            BUDP_Bitread_Byte_isr (pReader);

        pCCdata[data_found].format = BUDP_DCCparse_Format_ATSC53;

        /* Update count of items written to output data */
        /* Store data ordinary, closed caption data as well as CC708 data */
        ++data_found;
    }

    BUDP_SETFIELDSPARITY_ATSC_isr (pCCdata, data_found, pUserdata_info);

    *pcc_count = data_found;
    return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses SEI user data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read. This is the old format used by a
 * particular customer circa June 2005.
 */
static BERR_Code ParseSEIData_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    int index;
    uint8_t seiType;
    uint8_t userDataLength;
    uint8_t country_code;
    uint16_t manufacturers_code;
    uint8_t user_data_type_code;
    uint8_t user_data_code_length;
    uint8_t miscBits;
    uint8_t process_cc_data_flag;
    uint8_t cc_count;
    uint8_t cc_valid, cc_type, cc_data_1, cc_data_2;

    /* Start counting */
    *pcc_count = 0;
    *pBytesParsed = 0;

    /* We must have at least one type byte and a length byte */
    if (length < 2)
    {
        return BERR_BUDP_NO_DATA;
    }

    /* Read 8-bit type field */
    seiType = BUDP_Bitread_Byte_isr (pReader);
    (*pBytesParsed)++;
    if (seiType != 0x04)
    {
        return BERR_BUDP_NO_DATA;;
    }

    /* Read 8-bit length field */
    userDataLength = BUDP_Bitread_Byte_isr (pReader);
    (*pBytesParsed)++;

    /* A consistency check */
    if (userDataLength > (unsigned)(length - 2))
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Prevent access violations */
    if (userDataLength < 3)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Read 8-bit country code */
    country_code = BUDP_Bitread_Byte_isr (pReader);
    (*pBytesParsed)++;
    --userDataLength;
    if (country_code != 0xb5)
    {
        return BERR_BUDP_NO_DATA;
    }

    /* Read 16-bit manufacturers code */
    manufacturers_code  = BUDP_Bitread_Byte_isr (pReader) << 8;
    manufacturers_code |= BUDP_Bitread_Byte_isr (pReader);
    (*pBytesParsed) += 2;
    userDataLength -= 2;
    if (manufacturers_code != 0x002F)
    {
        return BERR_BUDP_NO_DATA;
    }

    while(userDataLength >= 2)
    {
        /* Read 8-bit type field */
        user_data_type_code = BUDP_Bitread_Byte_isr (pReader);
        (*pBytesParsed)++;
        --userDataLength;

        /* Read another length field */
        user_data_code_length = BUDP_Bitread_Byte_isr (pReader);
        (*pBytesParsed)++;
        --userDataLength;

        if (user_data_type_code == 3)
        {
            /* Begin cc_data */

            /* Read and check reserved, process_cc_data_flag, and
               additional_data_flag (3 bits total) */
            miscBits =  BUDP_Bitread_Read_isr (pReader, 1);
            if (miscBits != 0x01)
            {
                return BERR_BUDP_PARSE_ERROR;
            }
            process_cc_data_flag = BUDP_Bitread_Read_isr (pReader, 1);
            miscBits =  BUDP_Bitread_Read_isr (pReader, 1);
            if (miscBits != 0x00)
            {
                return BERR_BUDP_PARSE_ERROR;
            }

            /* Read cc_count (5 bits) */
            cc_count = BUDP_Bitread_Read_isr (pReader, 5);

            (*pBytesParsed)++;
            --userDataLength;

            /* Consistency check */
            if ((user_data_code_length != (cc_count * 3 + 3)) &&
                (user_data_code_length != (cc_count * 3 + 4))    )
            {
                return BERR_BUDP_PARSE_ERROR;
            }
            /* Programming note: the second clause above is to support older,
             * non-compliant bitstreams.
             */

            /* Read 8 reserved bits */
            miscBits = BUDP_Bitread_Byte_isr (pReader);
            if (miscBits != 0xFF)
            {
                return BERR_BUDP_PARSE_ERROR;
            }
            (*pBytesParsed)++;
            --userDataLength;

            /* Length check for the following inner loop, plus trailing byte. */
            if (userDataLength < (cc_count * 3 + 1))
            {
                return BERR_BUDP_PARSE_ERROR;
            }

            for (index = 0 ; index < cc_count ; ++index)
            {
                /* Read and check marker bits (5) */
                miscBits = BUDP_Bitread_Read_isr (pReader, 5);
                if (miscBits != 0x1F)
                {
                    /* MiscBits should always be 0x1f but in practice
                    some streams do not have them set and this would
                    cause us to throw the whole thing out. So lets
                    just continue after printing a warning.
                    */
                    /*return BERR_BUDP_PARSE_ERROR;*/
                    BDBG_WRN(("Marker bits not set"));
                }

                /* Read some one-bit descriptors (2) */
                cc_valid = BUDP_Bitread_Read_isr (pReader, 1);
                cc_type = BUDP_Bitread_Read_isr (pReader, 2);

                (*pBytesParsed)++;
                --userDataLength;

                /* Read the two bytes of closed caption data (16) */
                cc_data_1 = BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;
                --userDataLength;
                cc_data_2 = BUDP_Bitread_Byte_isr (pReader);
                (*pBytesParsed)++;
                --userDataLength;

                /* Store the data if warranted */
                if (process_cc_data_flag && miscBits == 0x1F)
                {
                    pCCdata[*pcc_count].cc_valid = cc_valid;
                    pCCdata[*pcc_count].seq.cc_type = cc_type;
                    pCCdata[*pcc_count].cc_data_1 = cc_data_1;
                    pCCdata[*pcc_count].cc_data_2 = cc_data_2;

                    /* Kludge */
                    pCCdata[*pcc_count].line_offset = 11;

                    pCCdata[*pcc_count].format = BUDP_DCCparse_Format_SEI;
                    (*pcc_count)++;
                }

            }

            /* Read marker bits (8) */
            miscBits = BUDP_Bitread_Byte_isr (pReader);
            if (miscBits != 0xFF)
            {
                return BERR_BUDP_PARSE_ERROR;
            }
            (*pBytesParsed)++;
            --userDataLength;
        }
        else
        {
            /* Skip over non-CC data */
            if (user_data_code_length > 0)
            {
                for (index = 0 ; index < (user_data_code_length-1) ; ++index)
                    BUDP_Bitread_Byte_isr (pReader);
                *pBytesParsed += (user_data_code_length-1);
                userDataLength -= (user_data_code_length-1);
            }
            /* Programming note: within the above "if" clause, all instances of
             * (user_data_code_length-1) could be replaced with
             * user_data_code_length. But, I am trying to support older,
             * non-compliant bitstreams along with compliant bitstreams. In
             * this case, my strategy is to skip over the smaller number of
             * bytes, and trust that in the case of a compliant bitstream, the
             * next executing parser will manage to skip over the remaining
             * single byte.
             */
        }
    }

    BUDP_SETFIELDSPARITY_SEI_isr (pCCdata, *pcc_count, pUserdata_info);

    return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses SEI user data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read. This is the new format using
 * ATSC/53.
 */
static BERR_Code ParseSEIData2_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    uint8_t seiType;
    uint8_t userDataLength;
    uint16_t manufacturers_code;
    BERR_Code eErr;
    size_t lBytesParsed = 0;

    /* We must have at least one type byte and a length byte */
    if (length < 2)
    {
        return BERR_BUDP_NO_DATA;;
    }

    /* Read 8-bit type field */
    seiType = BUDP_Bitread_Byte_isr (pReader);
    ++lBytesParsed;
    if (seiType != 0x04)
    {
        return BERR_BUDP_NO_DATA;;
    }

    /* Read 8-bit length field */
    userDataLength = BUDP_Bitread_Byte_isr (pReader);
    ++lBytesParsed;

    /* A consistency check */
    if (userDataLength > (unsigned)(length - 2))
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Prevent access violations */
    if (userDataLength < 3)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Read and ignore 8-bit country code */
    (void)BUDP_Bitread_Byte_isr (pReader);
    ++lBytesParsed;
    --userDataLength;

    /* Read and ignore 16-bit provider code code */
    manufacturers_code  = BUDP_Bitread_Byte_isr (pReader) << 8;
    manufacturers_code |= BUDP_Bitread_Byte_isr (pReader);
    lBytesParsed += 2;
    userDataLength -= 2;

    {
       BUDP_Bitread_Context stSavedReader = *pReader;
       unsigned iparser;

       /* Try all the available parsers */
       for (iparser = 0 ; iparser < NUM_MPEG_PARSERS ; ++iparser)
       {
          *pReader = stSavedReader;

          eErr = (*s_mpegParsers[iparser].parser) (
             pUserdata_info, pReader, length - lBytesParsed,
             pBytesParsed, pcc_count, pCCdata );
          switch (eErr)
          {
             case BERR_SUCCESS:
                /* Sarnoff compliance check */
                if ((pUserdata_info->eUserDataType == BAVC_USERDATA_Type_eSeq)
                   ||
                   (pUserdata_info->eUserDataType == BAVC_USERDATA_Type_eGOP))
                {
                   eErr = BERR_BUDP_SOURCE_CHECK;
                }
                /* fall into the next case */
             case BERR_BUDP_PARSE_ERROR:
                *pBytesParsed += *pBytesParsed;
                BDBG_LEAVE(BUDP_DCCparse);
                return eErr;
                break;
             case BERR_BUDP_NO_DATA:
                break;
             default:
                /* Programming error */
                BDBG_ASSERT (false);
                break;
          }
       }
    }

    *pBytesParsed += lBytesParsed;

    return eErr;
}

/***************************************************************************
 * This function sets the topfield/bottomfield polarity for closed caption
 * data already extracted from the bitstream.  It uses the order * in which
 * the digital data is presented, and top_field_first.
 */
static void SetFieldsParity_NOCCTYPE_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    bool top_field_first           = pUserdata_info->bTopFieldFirst;
    bool repeat_first_field        = pUserdata_info->bRepeatFirstField;
    BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;
    int icount;

    /* TODO: consider the case where there is cc_valid==0 data. Is the icount
     * ----  handling correct?
     */

    for (icount = 0 ; icount < cc_count ; ++icount)
    {
        /* Skip invalid data */
        if (pCCdata->cc_valid == 0)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is the case of a (top or bottom) field picture */
        if ((pictureStructure == BAVC_Polarity_eTopField) ||
            (pictureStructure == BAVC_Polarity_eBotField)   )
        {
            pCCdata->polarity = pictureStructure;
            pCCdata->bIsAnalog = true;
            ++pCCdata;
            continue;
        }

        /* This is an MPEG syntax error.  Punt. */
        if (pictureStructure !=  BAVC_Polarity_eFrame)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is an error.  Punt. */
        if (((icount > 2) && !repeat_first_field) ||
            (icount > 3))
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is the normal case.
        We are not relying on pCCdata->seq.cc_type to have a correct value,
        therefore we just use the index position of the data. */
        if (cc_count == 1) {
            pCCdata->polarity = BAVC_Polarity_eTopField;
        }
        else {
            switch (icount&1)
            {
            case 0:
                pCCdata->polarity =
                    (top_field_first ?
                        BAVC_Polarity_eTopField : BAVC_Polarity_eBotField);
                break;
            default: /*case 1:*/
                pCCdata->polarity =
                    (top_field_first ?
                        BAVC_Polarity_eBotField : BAVC_Polarity_eTopField);
                break;
            }
        }
        pCCdata->bIsAnalog = true;
        ++pCCdata;
    }
}

/***************************************************************************
 * This function sets the topfield/bottomfield polarity for closed caption
 * data already extracted from the bitstream.  It relies on the field_number
 * attribute, and top_field_first.
 */
static void SetFieldsParity_FIELDNUMBER_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    int icount;
    uint8_t field_number;
    bool top_field_first           = pUserdata_info->bTopFieldFirst;
    bool repeat_first_field        = pUserdata_info->bRepeatFirstField;
    BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;

    for (icount = 0 ; icount < cc_count ; ++icount)
    {
        /* Skip invalid data */
        if (pCCdata->cc_valid == 0)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        field_number = pCCdata->seq.field_number;

        switch (pictureStructure)
        {
        case BAVC_Polarity_eTopField:
            pCCdata->polarity = BAVC_Polarity_eTopField;
            pCCdata->bIsAnalog = true;
            break;
        case BAVC_Polarity_eBotField:
            pCCdata->polarity = BAVC_Polarity_eBotField;
            pCCdata->bIsAnalog = true;
            break;
        case BAVC_Polarity_eFrame:
            switch (field_number)
            {
            case 0:
                /* Forbidden!  Punt! */
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                break;
            case 1:
                pCCdata->polarity =
                    (top_field_first ?
                        BAVC_Polarity_eTopField : BAVC_Polarity_eBotField);
                pCCdata->bIsAnalog = true;
                break;
            case 2:
                pCCdata->polarity =
                    (top_field_first ?
                        BAVC_Polarity_eBotField : BAVC_Polarity_eTopField);
                pCCdata->bIsAnalog = true;
                break;
            case 3:
                if (repeat_first_field)
                {
                    pCCdata->polarity =
                        (top_field_first ?
                            BAVC_Polarity_eTopField : BAVC_Polarity_eBotField);
                    pCCdata->bIsAnalog = true;
                }
                else
                {
                    /* Makes no sense!  Punt! */
                    pCCdata->polarity = BAVC_Polarity_eFrame;
                    pCCdata->bIsAnalog = false;
                }
                break;
            }
            break;
        default:
            /* No other picture types to consider!  Punt! */
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            break;
        }
        ++pCCdata;
    }
}

/***************************************************************************
 * This function sets the topfield/bottomfield polarity for closed caption
 * data already extracted from the bitstream.  It uses the cc_type attribute,
 * and top_field_first.
 */
static void SetFieldsParity_TFF_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    bool top_field_first           = pUserdata_info->bTopFieldFirst;
    bool repeat_first_field        = pUserdata_info->bRepeatFirstField;
    BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;
    int icount;

    for (icount = 0 ; icount < cc_count ; ++icount)
    {
        /* Skip invalid data */
        if (pCCdata->cc_valid == 0)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is the case of a (top or bottom) field picture */
        if ((pictureStructure == BAVC_Polarity_eTopField) ||
            (pictureStructure == BAVC_Polarity_eBotField)   )
        {
            switch (pCCdata->seq.cc_type)
            {
            case 0:
            case 1:
                /* Top or bottom field, according to picture structure */
                pCCdata->polarity = pictureStructure;
                pCCdata->bIsAnalog = true;
                break;
            default:
                /* Punt! */
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                break;
            }
            ++pCCdata;
            continue;
        }

        /* This is an MPEG syntax error.  Punt. */
        if (pictureStructure !=  BAVC_Polarity_eFrame)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is an error.  Punt. */
        if (((icount > 2) && !repeat_first_field) ||
            (icount > 3))
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is the normal case */
        switch (pCCdata->seq.cc_type)
        {
        case 0:
            pCCdata->polarity =
                (top_field_first ?
                    BAVC_Polarity_eTopField : BAVC_Polarity_eBotField);
            pCCdata->bIsAnalog = true;
            break;
        case 1:
            pCCdata->polarity =
                (top_field_first ?
                    BAVC_Polarity_eBotField : BAVC_Polarity_eTopField);
            pCCdata->bIsAnalog = true;
            break;
        default:
            /* Punt! */
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            break;
        }
        ++pCCdata;
    }
}

/***************************************************************************
 * This function sets the topfield/bottomfield polarity for closed caption
 * data already extracted from the bitstream.  It uses only the cc_type
 * attribute, and NOT top_field_first.
 */
static void SetFieldsParity_SIMPLE_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;
    int icount;
    int nAnalog = 0;

    for (icount = 0 ; icount < cc_count ; ++icount)
    {
        /* Skip invalid data */
        if (pCCdata->cc_valid == 0)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* EIA-708-B has an odd restriction in the case of field pictures */
        switch (pictureStructure)
        {
        case BAVC_Polarity_eTopField:
            switch (pCCdata->seq.cc_type)
            {
            case 0:
                BDBG_MSG(("Violation of EIA-708-B, section 4.4.2"));
                pCCdata->polarity = BAVC_Polarity_eTopField;
                pCCdata->bIsAnalog = true;
                break;
            case 1:
                pCCdata->polarity = BAVC_Polarity_eBotField;
                pCCdata->bIsAnalog = true;
                break;
            default:
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                ++pCCdata;
                continue;
                break;
            }
            break;

        /* Same restriction */
        case BAVC_Polarity_eBotField:
            switch (pCCdata->seq.cc_type)
            {
            case 0:
                pCCdata->polarity = BAVC_Polarity_eTopField;
                pCCdata->bIsAnalog = true;
                break;
            case 1:
                BDBG_MSG(("Violation of EIA-708-B, section 4.4.2"));
                pCCdata->polarity = BAVC_Polarity_eBotField;
                pCCdata->bIsAnalog = true;
                break;
            default:
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                ++pCCdata;
                continue;
                break;
            }
            break;

        /* This is the normal case. A nice, ordinary frame picture. */
        case BAVC_Polarity_eFrame:
            switch (pCCdata->seq.cc_type)
            {
            case 0:
                pCCdata->polarity = BAVC_Polarity_eTopField;
                pCCdata->bIsAnalog = true;
                break;
            case 1:
                pCCdata->polarity = BAVC_Polarity_eBotField;
                pCCdata->bIsAnalog = true;
                break;
            default:
                /* Punt! */
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                ++pCCdata;
                continue;
                break;
            }
            break;

        /* This is an MPEG syntax error.  Punt. */
        default:
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
            break;

        /* End selection of picture type */
        }

        /* Too much closed caption data cannot be displayed */
        if (pCCdata->bIsAnalog)
        {
            /* If I had frame rate available, I could take repeat_first_field
             * into account here. See Table 4 of CEA-708-C. The last row of
             * the table is where frame rate is required. */
            if (nAnalog > 2)
            {
                pCCdata->polarity = BAVC_Polarity_eFrame;
                pCCdata->bIsAnalog = false;
                ++pCCdata;
                continue;
            }
            ++nAnalog;
        }

        ++pCCdata;
    }
}

/***************************************************************************
 * This function sets the topfield/bottomfield polarity for closed caption
 * data already extracted from the bitstream.  It only uses cc_type. It is
 * even more simple than SetFieldsParity_SIMPLE_isr, and is for use with closed
 * caption data extracted from SEI.
 */
static void SetFieldsParity_SEI_isr (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                  cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;
    int icount;

    for (icount = 0 ; icount < cc_count ; ++icount)
    {
        /* Skip invalid data */
        if (pCCdata->cc_valid == 0)
        {
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* This is an MPEG syntax error.  Punt. */
        if (pictureStructure != BAVC_Polarity_eTopField &&
            pictureStructure != BAVC_Polarity_eBotField &&
            pictureStructure != BAVC_Polarity_eFrame)
        {
            BDBG_ERR(("Unrecognized picture structure received (%d)",
                pictureStructure));
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            ++pCCdata;
            continue;
        }

        /* Normal case */
        switch (pCCdata->seq.cc_type)
        {
        case 0:
            pCCdata->polarity = BAVC_Polarity_eTopField;
            pCCdata->bIsAnalog = true;
            break;
        case 1:
            pCCdata->polarity = BAVC_Polarity_eBotField;
            pCCdata->bIsAnalog = true;
            break;
        default:
            /* Punt! */
            pCCdata->polarity = BAVC_Polarity_eFrame;
            pCCdata->bIsAnalog = false;
            break;
        }
        ++pCCdata;
    }
}

/***************************************************************************
 * This function reverses the order of bits in a byte.
*/
static int swap_bits_isr(int src)
{
    int result;
    int i;

    for (result = 0, i = 0; i < 8; i++)
    {
        result = (result << 1) | (src & 1);
        src >>= 1;
    }
    return result;
}

/***************************************************************************
 * This function serves to get rid of a compiler warning
*/
void BUDP_P_Quiet_Compiler (
    BUDP_DCCparse_ccdata* pCCdata,
    uint8_t                        cc_count,
    const BAVC_USERDATA_info*      pUserdata_info)
{
    SetFieldsParity_SIMPLE_isr      ( pCCdata, cc_count, pUserdata_info);
    SetFieldsParity_TFF_isr         ( pCCdata, cc_count, pUserdata_info);
    SetFieldsParity_NOCCTYPE_isr    ( pCCdata, cc_count, pUserdata_info);
    SetFieldsParity_FIELDNUMBER_isr ( pCCdata, cc_count, pUserdata_info);
    SetFieldsParity_SEI_isr         ( pCCdata, cc_count, pUserdata_info);
}

#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
    const BAVC_USERDATA_info* pUserdata_info, size_t offset)
{
    unsigned int iByte;
    static FILE* fd = NULL;
    static unsigned int nPicture;
    uint8_t* userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
    uint32_t length   = pUserdata_info->ui32UserDataBufSize - offset;

    /* Initialization */
    if (fd == NULL)
    {
        if ((fd = fopen (BUDP_P_Getud_Filename, "w")) == 0)
        {
            fprintf (stderr, "ERROR: could not open %s for debug output\n",
                BUDP_P_Getud_Filename);
            return;
        }
        fprintf (fd, "getud output format version 1\n");
        nPicture = 0;
    }

    fprintf (fd, "\nPic %u LOC %06lx TR %u\n", ++nPicture, 0UL, 0U);
    fprintf (fd, "PS %d TFF %d RFF %d\n",
        pUserdata_info->eSourcePolarity,
        pUserdata_info->bTopFieldFirst,
        pUserdata_info->bRepeatFirstField);
    fprintf (fd, "UDBYTES %u\n", length);
    for (iByte = 0 ; iByte < length ; ++iByte)
    {
        fprintf (fd, "%02x", userdata[iByte]);
        if ((iByte % 16) == 15)
            putc ('\n', fd);
        else
            putc (' ', fd);
    }
    if ((iByte % 16) != 15)
        putc ('\n', fd);
}
#endif
