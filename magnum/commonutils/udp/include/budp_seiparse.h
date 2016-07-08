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
BUDPSEIparse module provides parsing of SEI messages from AVC
bitstreams.  Currently, this software is limited to parsing
user_data_3d_structure() messages.

In general, this software accepts a buffer of data that contains
the SEI messages.  It parses out the data, and returns it in a
data structure
</verbatim>
***************************************************************************/

#ifndef BUDPSEIPARSE_H__
#define BUDPSEIPARSE_H__

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
    This structure describes a single user_data_3d_structure().

Description:
    BUDP_SEIparse_3ddata is a data structure that models
    user_data_3d_structure() syntax within SEI messages.
***************************************************************************/
typedef struct {
    uint32_t frame_packing_arrangement_id;
    bool     frame_packing_arrangement_cancel_flag;
    uint8_t  frame_packing_arrangement_type;
    bool     quincunx_sampling_flag;
    uint8_t  content_interpretation_type;
    bool     spatial_flipping_flag;
    bool     frame0_flipped_flag;
    bool     field_views_flag;
    bool     current_frame_is_frame0_flag;
    bool     frame0_self_contained_flag;
    bool     frame1_self_contained_flag;
    uint8    frame0_grid_position_x;
    uint8    frame0_grid_position_y;
    uint8    frame1_grid_position_x;
    uint8    frame1_grid_position_y;
    uint32_t frame_packing_arrangement_repetition_period;
    bool     frame_packing_arrangement_extension_flag;
}
BUDP_SEIparse_3ddata;

/*****************************************************************************
 * Public API
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Parses SEI messages from userdata.

  Description:
    This function accepts a "packet" of SEI data and searches it for
    recognized SEI messages.  Currently, it only recognizes
    3d_frame_packing_data().

    In order to avoid having an output array of indefinite
    length, this function will only parse at most one "packet"
    of data from its input.  A "packet" is defined as the
    userdata that lies between two successive userdata start
    codes (0x00000106).  Therefore, several calls to this
    function may be necessary to parse out all the data.
    Parsing will be complete when the function returns a
    *pBytesParsed argument such that (offset + *pBytesParsed ==
    pUserdata_info->ui32UserDataBufSize).

    The typical usage of this function is as follows:

    size_t offset = 0;
    while (offset < info->ui32UserDataBufSize)
    {
        eStatus =
            BUDP_SEIparse (info, offset, &bytesParsed, msgdata);
        if (eStatus == BERR_SUCCESS)
        {
            // Process (*msgdata) as a 3d_frame_packing_data() item
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
    BERR_BUDP_NO_DATA         - No SEI messages were found.  As the above
                                programming example shows, this is not really
                                an error.
    BERR_BUDP_PARSE_ERROR  -    SEI message(s) were detected, but could not
                                be successfully parsed.

  See Also:
 *****************************************************************************/
BERR_Code BUDP_SEIparse (
    const BAVC_USERDATA_info*
           pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
    size_t         offset, /*  [in] Parsing will start at this offset in the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    size_t*  pBytesParsed, /* [out] The number of bytes parsed from the
                                    input buffer
                                    userdata_info->pUserDataBuffer.          */
    BUDP_SEIparse_3ddata*
                  p3ddata  /* [out] A single instance of extracted
                                    user_data_3d_structure() data.           */
);

#ifdef __cplusplus
}
#endif

#endif /* BUDPSEIPARSE_H__ */
