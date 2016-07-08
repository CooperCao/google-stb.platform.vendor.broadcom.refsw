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

/*= Module Overview *********************************************************
<verbatim>


Overview
BUDPCCparse_divicom module provides parsing of digital closed caption
data in an older, DiviCom-specific format.  Digital closed caption
data occurs as userdata in MPEG bitstreams.

This software accepts a buffer of data that contains the MPEG
userdata.  It parses out the closed caption data, and returns it
as a pair of bytes.  These two bytes can then be sent directly to
a digital (EIA-708-B) closed caption encoder.  Byte pairs that have
an associated bIsAnalog value of "true" may be sent directly to an
analog closed caption (line 21) encoder.

</verbatim>
***************************************************************************/

#ifndef BUDPDCCPARSE_DIVICOM_H__
#define BUDPDCCPARSE_DIVICOM_H__

#include "bstd.h"
#include "berr.h"
#include "budp.h"
#include "budp_dccparse.h"

#define BUDP_DCCparse_Format_Divicom (99)

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
  Summary:
    Parses digital closed caption data from a buffer of userdata containing
    closed caption data in proprietary "Divicom" format.

  Description:
    This function accepts a "packet" of userdata and searches it for
    digital closed caption data. The closed caption data shall be in a
    proprietary format started by DiviCom Inc.

    In order to avoid having an output array of indefinite
    length, this function will only parse at most one "packet"
    of userdata from its input.  A "packet" is defined as the
    userdata that lies between two successive userdata start
    codes (0x000001B2).  Therefore, several calls to this
    function may be necessary to parse out all the userdata.
    Parsing will be complete when the function returns a
    *pBytesParsed argument such that (offset + *pBytesParsed ==
    pUserdata_info->ui32UserDataBufSize).

    Consider that the closed caption data can only be
    encoded analog if the associated bIsAnalog field is set.
    This consideration, together with the need to call the
    function multiple times (previous paragraph) implies that
    the typical usage of this function is as follows:

    size_t offset = 0;
    while (offset < info->ui32UserDataBufSize)
    {
        eStatus =
            BUDP_DCCparse_Divicom_isr (
                info, offset, &bytesParsed, &cc_count, ccdata);
        if (eStatus == BERR_SUCCESS)
        {
            int icount;
            for (icount = 0 ; icount < cc_count ; ++icount)
            {
                BUDP_DCCparse_ccdata* pccdata = &ccdata[icount];
                if (pccdata->bIsAnalog == true)
                {
                     // Send pccdata->cc_data_1 and pccdata->cc_data_2
                     // to analog closed caption encoder, if any.  These
                     // bytes must be sent on video field with polarity
                     // pcc->polarity.
                }
                // Possibly, send pccdata->cc_data_1 and pccdata->cc_data_2
                // to digital closed caption encoder.  The customer must
                // know if EIA-708-B processing is appropriate.
            }
        }
        else if (eStatus == BERR_BUDP_NO_DATA)
        {
            // Just keep looping, there may be more data to process.
        }
        else
        {
            // A real error, get out quick.
            return eStatus;
        }
        offset += bytesParsed;
    }

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_BUDP_NO_DATA      - No closed caption data was found.  As the above
                                programming example shows, this is not really
                                an error.
    BERR_BUDP_PARSE_ERROR  - Closed caption data was detected, but could not
                                be successfully parsed.

  See Also:
 *****************************************************************************/
BERR_Code BUDP_DCCparse_Divicom_isr (
    const BAVC_USERDATA_info*
           pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
    size_t         offset, /*  [in] Parsing will start at this offset in the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    size_t*  pBytesParsed, /* [out] The number of bytes parsed from the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    uint8_t*    pcc_count, /* [out] The length of the following output
                                    array.  Will be between 0 and 31.        */
    BUDP_DCCparse_ccdata*
                  pCCdata  /* [out] An array holding extracted closed
                                    caption data.  The user must provide an
                                    array of length 32, to accomodate the
                                    maximum data that the standards will
                                    allow.                                   */
);
#define BUDP_DCCparse_Divicom BUDP_DCCparse_Divicom_isr

#ifdef __cplusplus
}
#endif

#endif /* BUDPDCCPARSE_DIVICOM_H__ */
