/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bdbg.h"
#include "budp.h"
#include "budp_priv.h"
#include "budp_bitread.h"
#include "budp_dccparse.h"
#include "budp_dccparse_divicom.h"

BDBG_MODULE(BUDP);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static BERR_Code ParseDivicomData_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
);

/***************************************************************************
* Static data (tables, etc.)
***************************************************************************/

/***************************************************************************
* Implementation of "BUDP_DCCparse_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BUDP_DCCparse_Divicom_isr (
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
    BERR_Code eErr;
    BUDP_Bitread_Context reader;

    BDBG_ENTER(BUDP_DCCparse_Divicom);

    /* Check for obvious errors from user */
    if ((pUserdata_info == 0x0) ||
        (pBytesParsed   == 0x0) ||
        (pcc_count      == 0x0) ||
        (pCCdata        == 0x0)   )
    {
        return BERR_INVALID_PARAMETER;
    }

    /* Programming note:  all function parameters are now validated */

#if (BUDP_P_GETUD_DUMP)
    BUDP_P_dump_getud_isr (pUserdata_info, offset, BUDP_P_Getud_Filename);
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
    BUDP_Bitread_Init_isr (&reader, (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE), userdata);

    /* jump past the first MPEG userdata start code */
    bytesParsedSub = BUDP_P_FindMpegUserdataStart_isr (&reader, length);
    *pBytesParsed = bytesParsedSub;
    length -= bytesParsedSub;

    /* If we did not find a start code, bail out now */
    if (length == 0)
    {
        return BERR_BUDP_NO_DATA;
    }

    eErr = ParseDivicomData_isr (
        pUserdata_info, &reader, length,
        &bytesParsedSub, pcc_count, pCCdata);
    switch (eErr)
    {
    case BERR_SUCCESS:
        /* fall into the next case */
    case BERR_BUDP_PARSE_ERROR:
        *pBytesParsed += bytesParsedSub;
        BDBG_LEAVE(BUDP_DCCparse_Divicom);
        return eErr;
        break;
    case BERR_BUDP_NO_DATA:
        break;
    default:
        /* Programming error */
        BDBG_ASSERT (false);
        break;
    }

    /* If we did not find a start code, bail out now */
    if (length == 0)
    {
        return BERR_BUDP_NO_DATA;
    }

    /* No userdata was found */
    BDBG_LEAVE(BUDP_DCCparse_Divicom);
    return BERR_BUDP_NO_DATA;
}

/***************************************************************************
* Private functions
***************************************************************************/

/***************************************************************************
 * This function parses Divicom data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseDivicomData_isr (
    const BAVC_USERDATA_info*      pUserdata_info,
    BUDP_Bitread_Context* pReader,
    size_t                   length,
    size_t*                  pBytesParsed,
    uint8_t*                 pcc_count,
    BUDP_DCCparse_ccdata* pCCdata
)
{
    unsigned int cc_count;
    unsigned int vbi_field_type;
    unsigned int icount;
    int data_found;

    BSTD_UNUSED (pUserdata_info);

    /* Start counting */
    *pcc_count = 0;
    *pBytesParsed = 0;
    data_found = 0;

    /* Start travelling pointer into data */
    if (length < 2)
    {
        return BERR_BUDP_PARSE_ERROR;
    }

    /* Dig out first count of cc data pairs,  and field type */
    cc_count       = BUDP_Bitread_Byte_isr (pReader);
    vbi_field_type = BUDP_Bitread_Byte_isr (pReader);

    /* It seems that a zero cc_count marks the end of data. This is not
     * documented */
    while (cc_count != 0)
    {
        /* Only a few legal values for cc_count and vbi_field_type */
        if ((cc_count != 2) && (cc_count != 4))
        {
            return BERR_BUDP_PARSE_ERROR;
        }
        if ((vbi_field_type != 0x09) && (vbi_field_type != 0x0a))
        {
            return BERR_BUDP_PARSE_ERROR;
        }

        /* Account for cc_count and vbi_field_type bytes */
        length -= 2;
        *pBytesParsed += 2;

        /* Account for closed caption bytes about to be parsed */
        if (length < cc_count)
        {
            /* Packet too short */
            return BERR_BUDP_PARSE_ERROR;
        }
        length -= cc_count;
        *pBytesParsed += cc_count;

        /* Dig out the closed caption data pairs */
        for (icount = 0 ; icount < cc_count/2 ; icount+=2)
        {
            /* Kludge */
            pCCdata[data_found].line_offset = 11;
            pCCdata[data_found].cc_valid    =  1;
            pCCdata[data_found].cc_priority =  0;

            /* Here is the closed caption data, finally. */
            pCCdata[data_found].cc_data_1 =
                BUDP_Bitread_Byte_isr (pReader);
            pCCdata[data_found].cc_data_2 =
                BUDP_Bitread_Byte_isr (pReader);
            pCCdata[data_found].format = BUDP_DCCparse_Format_Divicom;
            pCCdata[data_found].bIsAnalog = true;

            /* Parity of extracted data is determined by vbi_field_type */
            if (vbi_field_type == 0x09)
            {
                pCCdata[data_found].polarity = BAVC_Polarity_eTopField;
                pCCdata[data_found].seq.cc_type = 0;
            }
            else
            {
                pCCdata[data_found].polarity = BAVC_Polarity_eBotField;
                pCCdata[data_found].seq.cc_type = 1;
            }

            /* Update count of items written to output data */
            ++data_found;
        }

        /* Dig out next cc_count */
        cc_count       = BUDP_Bitread_Byte_isr (pReader);
        vbi_field_type = BUDP_Bitread_Byte_isr (pReader);
    }

    *pcc_count = data_found;
    return BERR_SUCCESS;
}

/* End of file */
