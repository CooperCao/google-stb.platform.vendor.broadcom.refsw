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
BUDPSCTEparse module provides parsing of SCTE digital data.
This data occurs as userdata in MPEG bitstreams.  There are two
standards supported:  SCTE20 and SCTE21.

In general, this software accepts a buffer of data that contains
the MPEG userdata.  It parses out the SCTE data, and returns it as
an array of complex data structures.  Each data structure contains
SCTE data for output to a single video field. The data structures
can be sent directly to the VBI encoding software in the porting
interface (BUDP module).

</verbatim>
***************************************************************************/

#ifndef BUDPSCTEPARSE_H__
#define BUDPSCTEPARSE_H__

#include "bstd.h"
#include "berr.h"
#include "bavc_types.h"
#include "budp.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * Structures
 *****************************************************************************/

/***************************************************************************
Summary:
    This enum specifies the format used to encode SCTE data into MPEG
    userdata.
***************************************************************************/
typedef enum {
    BUDP_SCTEparse_Format_Unknown = 0,
    BUDP_SCTEparse_Format_SCTE20,
    BUDP_SCTEparse_Format_SCTE21CC,
    BUDP_SCTEparse_Format_SCTE21ACC,
    BUDP_SCTEparse_Format_SCTE21PAM
}
BUDP_SCTEparse_Format;

/***************************************************************************
Summary:
    This structure describes a "piece" of SCTE data extracted
    from MPEG userdata.

Description:
    BUDP_SCTEparse_sctedata is a "holder" for the SCTE
    data structure used by the porting interface module
    BVBI. The BUDP data structure adds a few needed extra
    bits of information.

    The format field indicates the method of encoding the SCTE
    datum into MPEG userdata.

    The polarity field indicates whether the data belongs in
    a video top field or a bottom field.

***************************************************************************/
typedef struct {
    BAVC_Polarity polarity;
    BUDP_SCTEparse_Format format;
    BUDP_SCTE_Data data;
} BUDP_SCTEparse_sctedata;

/*****************************************************************************
 * Public API
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Parses SCTE 20 data from a buffer of MPEG userdata.

  Description:
    This function accepts a "packet" of MPEG userdata and searches it for
    SCTE 20 data.

    In order to avoid having an output array of indefinite length, this
    function will only parse at most one "packet" of MPEG userdata from its
    input.  A "packet" is defined as the userdata that lies between two
    successive userdata start codes (0x000001B2).  Therefore, several calls to
    this function may be necessary to parse out all the userdata.  Parsing will
    be complete when the function returns a *pBytesParsed argument such that
    (offset + *pBytesParsed == pUserdata_info->ui32UserDataBufSize).

    The typical usage of this function is as follows:

    size_t offset = 0;
    while (offset < info->ui32UserDataBufSize)
    {
        eStatus =
            BUDP_SCTE20parse_isr (
                info, offset, &bytesParsed, &scte_count, sctedata);
        if (eStatus == BERR_SUCCESS)
        {
            int icount;
            for (icount = 0 ; icount < scte_count ; ++icount)
            {
                BUDP_SCTEparse_sctedata* psctedata = &sctedata[icount];

                // Send psctedata->data to SCTE encoder. This
                // data  must be sent on video field with polarity
                // pscte->polarity.
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
    BERR_BUDP_NO_DATA      - No SCTE data was found.  As the above
                                programming example shows, this is not really
                                an error.
    BERR_BUDP_PARSE_ERROR  - SCTE data was detected, but could not
                                be successfully parsed.

 *****************************************************************************/
BERR_Code BUDP_SCTE20parse_isr (
    const BAVC_USERDATA_info*
           pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
    size_t         offset, /*  [in] Parsing will start at this offset in the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    size_t*  pBytesParsed, /* [out] The number of bytes parsed from the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    uint8_t*  pscte_count, /* [out] The length of the following output
                                    array.  Will be between 0 and 31.        */
    BUDP_SCTEparse_sctedata*
                psctedata  /* [out] An array holding extracted SCTE data.
                                    The user must provide an array of length
                                    32, to accomodate the maximum data that
                                    the standards will allow.                */
);
#define BUDP_SCTE20parse BUDP_SCTE20parse_isr

/*****************************************************************************
  Summary:
    Parses SCTE 21 data from a buffer of MPEG userdata.

  Description:
    This function accepts a "packet" of MPEG userdata and searches it for
    SCTE 21 data.

    In order to avoid having an output array of indefinite length, this
    function will only parse at most one "packet" of MPEG userdata from its
    input.  A "packet" is defined as the userdata that lies between two
    successive userdata start codes (0x000001B2).  Therefore, several calls to
    this function may be necessary to parse out all the userdata.  Parsing will
    be complete when the function returns a *pBytesParsed argument such that
    (offset + *pBytesParsed == pUserdata_info->ui32UserDataBufSize).

    The typical usage of this function is as follows:

    size_t offset = 0;
    while (offset < info->ui32UserDataBufSize)
    {
        eStatus =
            BUDP_SCTE21parse_isr (
                info, offset, &bytesParsed, &scte_count, sctedata);
        if (eStatus == BERR_SUCCESS)
        {
            int icount;
            for (icount = 0 ; icount < scte_count ; ++icount)
            {
                BUDP_SCTEparse_sctedata* psctedata = &sctedata[icount];

                // Send psctedata->data to SCTE encoder. This
                // data  must be sent on video field with polarity
                // pscte->polarity.
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
    BERR_BUDP_NO_DATA      - No SCTE data was found.  As the above
                                programming example shows, this is not really
                                an error.
    BERR_BUDP_PARSE_ERROR  - SCTE data was detected, but could not
                                be successfully parsed.

 *****************************************************************************/
BERR_Code BUDP_SCTE21parse_isr (
    const BAVC_USERDATA_info*
           pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
    size_t         offset, /*  [in] Parsing will start at this offset in the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    size_t*  pBytesParsed, /* [out] The number of bytes parsed from the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    uint8_t*  pscte_count, /* [out] The length of the following output
                                    array.  Will be between 0 and 31.        */
    BUDP_SCTEparse_sctedata*
                psctedata  /* [out] An array holding extracted SCTE data.
                                    The user must provide an array of length
                                    32, to accomodate the maximum data that
                                    the standards will allow.                */
);
#define BUDP_SCTE21parse BUDP_SCTE21parse_isr

#ifdef __cplusplus
}
#endif

#endif /* BUDPSCTEPARSE_H__ */
