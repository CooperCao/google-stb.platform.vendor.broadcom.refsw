/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
BUDP module parses userdata from MPEG-2 and AVC bitstreams.

</verbatim>
***************************************************************************/

#ifndef BUDP_H__
#define BUDP_H__

#include "berr.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
	Module specific standard BERR codes
 *****************************************************************************/

#define BERR_BUDP_NO_DATA         BERR_MAKE_CODE(BERR_UDP_ID, 0x0000)
#define BERR_BUDP_PARSE_ERROR     BERR_MAKE_CODE(BERR_UDP_ID, 0x0001)
#define BERR_BUDP_SOURCE_CHECK    BERR_MAKE_CODE(BERR_UDP_ID, 0x0002)

/* Programming note:
 * ----------------
 *  The above error code BERR_BUDP_SOURCE_CHECK indicates that parsing is
 *  successful. However, because the data was encoded into an MPEG sequence
 *  header or GOP header, this data is not suitable for text output.
 */

/*****************************************************************************
 * Structures
 *****************************************************************************/

typedef struct BUDP_SCTE_CC_Data
{
	uint8_t valid;
	uint8_t priority;
	uint8_t line_number;	/* Between 10 and 41, inclusive. */
	uint8_t cc_data_1;
	uint8_t cc_data_2;

} BUDP_SCTE_CC_Data;

typedef struct BUDP_SCTE_NRTV_Data
{
	uint8_t valid;
	uint8_t priority;
	uint8_t line_number;	/* Between 10 and 41, inclusive. */
	uint8_t sequence_number;
	uint8_t segment_number;
	uint8_t y_data[32];
	uint8_t cbcr_data[32];

} BUDP_SCTE_NRTV_Data;

typedef struct BUDP_SCTE_Mono_Data
{
	bool first_segment_flag;
	bool last_segment_flag;
	uint8_t line_number;	/* Between 10 and 41, inclusive. */
	uint16_t first_pixel_position;
	uint8_t n_pixels;
	uint8_t Y_data[256];

} BUDP_SCTE_Mono_Data;

#define BUDP_SCTE_MAX_ITEMS 32


/*****************************************************************************
  Summary:
    Structure for holding SCTE data
   
  Description:
    The BUDP_SCTE_Data structure is a container for an entire field of 
	SCTE data.

    The BUDP_SCTE_Data structure contains _size elements, _count
    elements, and data pointers.

	The user is responsible for setting the data pointers to
	reasonable values. However, if the corresponding _count
	element is zero, then software will not make use of
	the data pointer. Software does not allocate or free
	memory indicated by the data pointers in any case.

	The _size elements are for the convenience of the user,
	they are not used by software.

	The _count elements indicate the amount of data contained
	in the corresponding data pointers. The units of the _count
	elements vary, but they are defined below.

 *****************************************************************************/
typedef struct BUDP_SCTE_Data
{
	size_t cc_count;                 /* Number of meaningful entries in the
	                                    following cc_data array.             */
	size_t cc_size;                  /* Capacity of following cc_data array,
	                                    in units of BUDP_SCTE_CC_Data.       */
	BUDP_SCTE_CC_Data* cc_data;      /* The closed caption data. If zero,
	                                    then no space has been allocated.    */
	size_t nrtv_count;               /* Number of meaningful entries in the
	                                    following nrtv_data array. Must be 
									    zero or one.                         */
	size_t nrtv_size;                /* Capacity of following nrtv_data
	                                    array, in units of 
										BUDP_SCTE_NRTV_Data                  */
	BUDP_SCTE_NRTV_Data* nrtv_data;  /* The NRTV data. If zero, then no
	                                    space has been allocated.            */
	size_t pam_count;                /* Number of meaningful bytes in the
	                                    following luma_PAM_data buffer. The
										maximum value allowed is 4095.       */
	size_t pam_size;                 /* Capacity of following luma_PAM_data
	                                    buffer, in bytes. If zero, then no
										space has been allocated.            */
	uint8_t* luma_PAM_data;          /* Buffer containing one piece of
	                                    luma_PAM_data, as defined in SCTE-21 
										section 5.5.                         */
	size_t mono_count;               /* Number of meaningful entries in the
	                                    following mono_data array. Must be 
									    zero, one, or two.                   */
	size_t mono_size;                /* Capacity of following mono_data
	                                    array, in units of 
										BUDP_SCTE_Mono_Data                  */
	BUDP_SCTE_Mono_Data* mono_data;  /* The monochrome data. If zero, then 
	                                    no space has been allocated.         */
	uint8_t field_number;            /* Field number, from SCTE data
	                                    structure. In the case of SCTE 20 
										ATSC CC data, it is field_type. In 
										the case of monochrome data, this is 
										the field_parity element.            */
} BUDP_SCTE_Data;

#ifdef __cplusplus
}
#endif
 
#endif /* BUDP_H__ */
