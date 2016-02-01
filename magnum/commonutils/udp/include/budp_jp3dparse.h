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
<verbatim>


Overview
BUDPJP3Dparse module provides parsing of "JP3D" messages from MPEG-2
bitstreams.  These messages are defined in ISO/IEC JTC1/SC29/WG11 
"JNB comments on extension of MPEG-2 video and its 3D signaling."

In general, this software accepts a buffer of data that contains
the JP3D messages.  One function call finds the start of the JP3D 
data. Repeated calls to another function retrieve the individual 
messages, one at a time.
</verbatim>
***************************************************************************/

#ifndef BUDPJP3DPARSE_H__
#define BUDPJP3DPARSE_H__

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

/*****************************************************************************
 * Public API
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Locates the start of JP3D messages in MPEG userdata.

  Description:
    This function accepts a "packet" of MPEG userdata and searches it for
	the start of "JP3D" messages, indicated by this exact series of bytes:
	00 00 01 B2 4A 50 33 44. The first four bytes comprise the standard MPEG 
	userdata startcode. The final four bytes are specified by the controlling
	ISO/IEC standard and comprise the ASCII string "JP3D."

	The typical usage of this function is as follows:

    size_t offset = 0;
	eStatus = 
            BUDP_JP3Dstart_isr (info, offset, &bytesParsed);
    while (eStatus == BERR_SUCCESS)
    {
		offset += bytesParsed;
        eStatus = 
            BUDP_JP3Dparse_isr (info, offset, &bytesParsed, &sigtype);
        if (eStatus == BERR_SUCCESS)
        {
			// Process sigtype, the only variable data in the JP3D
			// message.
        }
		else if (eStatus == BERR_BUDP_NO_DATA)
		{
			// End of data has been found. No code here, just let the 
			// "while loop" terminate normally.
		}
		else
		{
            // A real error. Put some error processing here.
			// Further parsing may yield garbage.
            return eStatus;
		}
	}

  Returns:
  	BERR_SUCCESS              - Start of JP3D message data was found. Ready to 
	                            parse.
	BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
							    possibly NULL.
	BERR_BUDP_NO_DATA         - No JP3D messages were found.  
	BERR_BUDP_PARSE_ERROR  -    JP3D message(s) were detected, but could not
	                            be successfully parsed.

  See Also:
  	BUDP_JP3Dparse_isr
 *****************************************************************************/
BERR_Code BUDP_JP3Dstart_isr ( 
	const BAVC_USERDATA_info* 
	       pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
	size_t         offset, /*  [in] Parsing will start at this offset in the
	                                input buffer 
								    userdata_info->pUserDataBuffer.          */
	size_t*  pBytesParsed  /* [out] The number of bytes skipped from the 
	                                input buffer 
									userdata_info->pUserDataBuffer.          */
);
#define BUDP_JP3Dstart BUDP_JP3Dstart_isr


/*****************************************************************************
  Summary:
    Parses individual JP3D messages from userdata.

  Description:
    This function accepts a "packet" of JP3D data and extracts a 
	single message. For correct operation, the data must be 
	"positioned" by a previous call to either this function, or to 
	BUDP_JP3Dstart().

	See the documentation for function BUDP_JP3Dstart_isr() for a 
	complete programming example.

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
  	BUDP_JP3Dstart_isr
 *****************************************************************************/
BERR_Code BUDP_JP3Dparse_isr ( 
	const BAVC_USERDATA_info* 
	       pUserdata_info, /*  [in] The MPEG userdata to be parsed.          */
	size_t         offset, /*  [in] Parsing will start at this offset in the
	                                input buffer 
								    userdata_info->pUserDataBuffer.          */
	size_t*  pBytesParsed, /* [out] The number of bytes parsed from the 
	                                input buffer 
									userdata_info->pUserDataBuffer.          */
	uint16_t*     sigtype  /* [out] A single instance of extracted datum
				                    Stereo_Video_Format_Signaling_Type.      */
);
#define BUDP_JP3Dparse BUDP_JP3Dparse_isr

#ifdef __cplusplus
}
#endif

#endif /* BUDPJP3DPARSE_H__ */
