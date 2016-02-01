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

/* For debugging */
/* #define BUDP_P_GETUD_DUMP 1 */
#ifdef BUDP_P_GETUD_DUMP
static const char* BUDP_P_Getud_Filename = "userdata.getud";
#include <stdio.h>
#endif

#include "bstd.h"
#include "bdbg.h"
#include "bvlc.h"
#include "budp.h"
#include "budp_seiparse.h"

BDBG_MODULE(BUDP);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static size_t FindSeiUserdataStart_isr (
	const uint8_t* userdata, size_t length);

#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
	const BAVC_USERDATA_info* pUserdata_info, size_t offset);
#endif

/***************************************************************************
* Static data (tables, etc.)
***************************************************************************/


/***************************************************************************
* Implementation of "BUDP_SEIparse_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BUDP_SEIparse (
	const BAVC_USERDATA_info*  pUserdata_info,
	size_t                     offset,
	size_t*                    pBytesParsed,
	BUDP_SEIparse_3ddata*      p3ddata
)
{
	size_t bytesParsedSub;
	size_t length;
	uint8_t* userdata;
	uint8_t seiType;
	BERR_Code eErr;

	int      vlc_decoded;
	unsigned vlc_index;
	unsigned vlc_bit;

	BDBG_ENTER(BUDP_SEIparse);

	/* Check for obvious errors from user */
	if ((pUserdata_info == 0x0) ||
	    (pBytesParsed   == 0x0) ||
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

	/* jump past the first SEI userdata start code */
	bytesParsedSub = FindSeiUserdataStart_isr (userdata, length);
	*pBytesParsed = bytesParsedSub;
	length -= bytesParsedSub;
	userdata += bytesParsedSub;

	/* If we did not find a start code, bail out now */
	if (length == 0)
	{
		return BERR_BUDP_NO_DATA;
	}

	/* We must have at least one type byte and some VLC data */
	if (length < 2)
	{
		return BERR_BUDP_NO_DATA;
	}

	/* Check the message type byte */
	seiType = *userdata;
	++(*pBytesParsed);
	--length;
	if (seiType != 0x07)
	{
		return BERR_BUDP_NO_DATA;
	}
	++userdata;

	/* Initialize the return data */
	BKNI_Memset ((void*)(p3ddata), 0x0, sizeof(BUDP_SEIparse_3ddata));

	/* Drop into VLC mode */
	eErr = BERR_BUDP_PARSE_ERROR;
	do
	{
		vlc_index = 0;
		vlc_bit = 7;

		vlc_decoded = BVLC_Decode (
			userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
		if (vlc_decoded == -1)
			break;
		p3ddata->frame_packing_arrangement_id = vlc_decoded;

		vlc_decoded = BVLC_Decode (
			userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
		if (vlc_decoded == -1)
			break;
		p3ddata->frame_packing_arrangement_cancel_flag = (vlc_decoded != 0);

		if (!p3ddata->frame_packing_arrangement_cancel_flag)
		{
			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->frame_packing_arrangement_type = vlc_decoded;

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->quincunx_sampling_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->content_interpretation_type = vlc_decoded;

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->spatial_flipping_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->frame0_flipped_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->field_views_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->current_frame_is_frame0_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->frame0_self_contained_flag = (vlc_decoded != 0);

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->frame1_self_contained_flag = (vlc_decoded != 0);

			if (!(p3ddata->quincunx_sampling_flag) &&
			    (p3ddata->frame_packing_arrangement_type != 5))
			{
				vlc_decoded = BVLC_Decode (
					userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
				if (vlc_decoded == -1)
					break;
				p3ddata->frame0_grid_position_x = vlc_decoded;

				vlc_decoded = BVLC_Decode (
					userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
				if (vlc_decoded == -1)
					break;
				p3ddata->frame0_grid_position_y = vlc_decoded;

				vlc_decoded = BVLC_Decode (
					userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
				if (vlc_decoded == -1)
					break;
				p3ddata->frame1_grid_position_x = vlc_decoded;

				vlc_decoded = BVLC_Decode (
					userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
				if (vlc_decoded == -1)
					break;
				p3ddata->frame1_grid_position_y = vlc_decoded;
			}
			/* Reserved byte */
			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			if (vlc_decoded != 0)
				break;

			vlc_decoded = BVLC_Decode (
				userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
			if (vlc_decoded == -1)
				break;
			p3ddata->frame_packing_arrangement_repetition_period = vlc_decoded;
		}
		vlc_decoded = BVLC_Decode (
			userdata, length, vlc_index, vlc_bit, &vlc_index, &vlc_bit);
		if (vlc_decoded == -1)
			break;
		p3ddata->frame_packing_arrangement_extension_flag = (vlc_decoded != 0);

		/* Signify success */
		eErr = BERR_SUCCESS;
	}
	while (false);

	/* Process results from VLC mode */
	userdata += vlc_index;
	(*pBytesParsed) += vlc_index;
	length -= vlc_index;
		
	BDBG_LEAVE(BUDP_SEIparse);
	return eErr;
}


/***************************************************************************
 * This function finds the next userdata startcode 0x00000106.  It
 * indicates the byte following this startcode by its return value.
 * If no startcode was found, it simply returns the length of the
 * input data.
 */
static size_t FindSeiUserdataStart_isr (
	const uint8_t* userdata, size_t length)
{
    size_t count = 0;
	uint8_t saved[4];

	/* Special case (failure) */
	if (length < 4)
		return length;

	/* Initialize */
	saved[1] = *userdata++
	saved[2] = *userdata++
	saved[3] = *userdata++

	while (length >= 4)
	{
		/* Read in another byte */
		saved[0] = saved[1];
		saved[1] = saved[2];
		saved[2] = saved[3];
		saved[3] = *userdata++

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


#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
	const BAVC_USERDATA_info* pUserdata_info, size_t offset)
{
	unsigned int iByte;
	static FILE* fd = NULL;
	static unsigned int nPicture;
	uint8_t* userdata = (uint8_t*)(pUserdata_info->pUserDataBuffer) + offset;
	size_t   length   = pUserdata_info->ui32UserDataBufSize - offset;

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
