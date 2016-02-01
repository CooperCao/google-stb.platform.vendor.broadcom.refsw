/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

 /*= Module Overview *********************************************************

* Handle DirecTV close caption data
 ***************************************************************************/
#ifndef BUDPDCCPARSEDSS_H__
#define BUDPDCCPARSEDSS_H__

#include "bstd.h"
#include "berr.h"
#include "bavc_types.h"
#include "budp.h"

typedef enum {
	BUDP_DCCparse_CC_Dss_Type_Undefined,
	BUDP_DCCparse_CC_Dss_Type_ClosedCaption,
	BUDP_DCCparse_CC_Dss_Type_Subtitle
} BUDP_DCCparse_Dss_CC_Type;

typedef struct {
	bool bIsAnalog;
	BAVC_Polarity polarity;
	BUDP_DCCparse_Dss_CC_Type eCCType;
	uint8_t cc_priority;
	uint8_t language_type;		/* not exist in dss-sd CC, sub_title_language_type in GLA subtitle */
	uint8_t cc_data_1;
	uint8_t cc_data_2;
} BUDP_DCCparse_dss_cc_subtitle;

/*****************************************************************************
  Summary:
    Parses digital closed caption data from a buffer of DSS userdata.

  Description:
    This function accepts a "packet" of DSS userdata and searches
    it for digital closed caption data.

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
            BUDP_DCCparse_DSS_isr (
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
BERR_Code BUDP_DCCparse_DSS_isr ( 
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
	BUDP_DCCparse_dss_cc_subtitle* 
	              pCCdata,  /* [out] An array holding extracted closed 
				                    caption data.  The user must provide an 
									array of length 32, to accomodate the 
									maximum data that the standards will 
									allow.                                   */
	uint8_t*                 psubtitle_count, /*count for subtitle data */
	BUDP_DCCparse_dss_cc_subtitle* pSubtitledata /*array holding extracted subtitle data */
);
#define BUDP_DCCparse_DSS BUDP_DCCparse_DSS_isr

#endif /* BUDPDCCPARSEDSS_H__ */



