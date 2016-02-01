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
#include "bkni.h"
#include "bdbg.h"
#include "budp.h"
#include "budp_bitread.h"
#include "budp_scteparse.h"

BDBG_MODULE(BUDP);


#if !B_REFSW_MINIMAL /** { **/

/***************************************************************************
* Private data types
***************************************************************************/

typedef struct {
	BUDP_SCTEparse_Format type;
	BERR_Code (*parser) (
		BUDP_Bitread_Context* pReader,
		size_t					 length,
		size_t*                  pBytesParsed,
		uint8_t*                 pscte_count,
		BUDP_SCTEparse_sctedata* pScteData
	);
} DigitalParser;


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#ifndef MAX
	#define MAX(a,b) \
		(((a) >= (b)) ? (a) : (b))
#endif

static size_t FindMpegUserdataStart_isr (
	BUDP_Bitread_Context* pReader, size_t length);
static BERR_Code ParseSCTE20Data_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData);
static BERR_Code ParseSCTE21CCData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData);
static BERR_Code ParseSCTE21ACCData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData);
static BERR_Code ParseSCTE21PAMData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData);

static void P_ArrayInit_SCTE_isr (
	BUDP_SCTEparse_sctedata* pSCTEdata,
	const BAVC_USERDATA_info* pUserdata_info);

static int swap_bits_isr (int src);
static void P_EndSwap_isr (uint8_t* byteData, size_t byteLength);

#ifdef BUDP_P_GETUD_DUMP
static void dump_getud_isr (
	const BAVC_USERDATA_info* pUserdata_info, size_t offset);
#endif


/***************************************************************************
* Static data (tables, etc.)
***************************************************************************/

/* The arrays of digital CC data parsers.
 */
static const DigitalParser scte20Parsers[] = {
	{BUDP_SCTEparse_Format_SCTE20,         ParseSCTE20Data_isr},
};
#define NUM_20_PARSERS (sizeof(scte20Parsers) / sizeof(scte20Parsers[0]))
static const DigitalParser scte21Parsers[] = {
	{BUDP_SCTEparse_Format_SCTE21CC,       ParseSCTE21CCData_isr},
	{BUDP_SCTEparse_Format_SCTE21ACC,      ParseSCTE21ACCData_isr},
	{BUDP_SCTEparse_Format_SCTE21PAM,      ParseSCTE21PAMData_isr}
};
#define NUM_21_PARSERS (sizeof(scte21Parsers) / sizeof(scte21Parsers[0]))

static const bool bByteswap = (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE);


/***************************************************************************
* Implementation of "BUDP_SCTEparse_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BUDP_SCTE20parse_isr (
	const BAVC_USERDATA_info*      pUserdata_info,
	size_t                   offset,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData
)
{
	size_t bytesParsedSub;
	size_t length;
	uint8_t* userdata;
	unsigned int iparser;
	BERR_Code eErr;
	BUDP_Bitread_Context reader;
	BUDP_Bitread_Context savedReader;

	BDBG_ENTER(BUDP_SCTE20parse_isr);

	/* Check for obvious errors from user */
	if ((pUserdata_info   == 0x0) ||
	    (pBytesParsed     == 0x0) ||
	    (pscte_count      == 0x0) ||
	    (pScteData        == 0x0)   )
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

	/* Initialize output array */
	P_ArrayInit_SCTE_isr (pScteData, pUserdata_info);
	*pscte_count = 0;

	/*********************************************************************\
	* Programming note:  each of the parsers will give up and return
	* BERR_BUDP_NO_DATA if it finds the wrong identifier byte.
	\*********************************************************************/

	/* Try all the available parsers */
	for (iparser = 0 ; iparser < NUM_20_PARSERS ; ++iparser)
	{
		reader = savedReader;
		eErr = (*scte20Parsers[iparser].parser) (
			&reader, length, &bytesParsedSub, pscte_count, pScteData);
		switch (eErr)
		{
		case BERR_SUCCESS:
		case BERR_BUDP_PARSE_ERROR:
			*pBytesParsed += bytesParsedSub;
			BDBG_LEAVE(BUDP_SCTE20parse_isr);
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

	/* Programming note:
	 * None of the parsers liked this bit of userdata.
	 * It would be more elegant to skip to the next start code
	 * before returning.  But it is more efficient to simply
	 * stop parsing. */

	/* No userdata was found */
	BDBG_LEAVE(BUDP_SCTE20parse_isr);
	return BERR_BUDP_NO_DATA;
}


/***************************************************************************
 *
 */
BERR_Code BUDP_SCTE21parse_isr (
	const BAVC_USERDATA_info*      pUserdata_info,
	size_t                   offset,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData
)
{
	size_t bytesParsedSub;
	size_t length;
	uint8_t* userdata;
	unsigned int iparser;
	BERR_Code eErr;
	BUDP_Bitread_Context reader;
	BUDP_Bitread_Context savedReader;

	BDBG_ENTER(BUDP_SCTE21parse_isr);

	/* Check for obvious errors from user */
	if ((pUserdata_info   == 0x0) ||
	    (pBytesParsed     == 0x0) ||
	    (pscte_count      == 0x0) ||
	    (pScteData        == 0x0)   )
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

	/* Initialize output array */
	P_ArrayInit_SCTE_isr (pScteData, pUserdata_info);
	*pscte_count = 0;

	/*********************************************************************\
	* Programming note:  each of the parsers will give up and return
	* BERR_BUDP_NO_DATA if it finds the wrong identifier byte.
	\*********************************************************************/

	/* Try all the available parsers */
	for (iparser = 0 ; iparser < NUM_21_PARSERS ; ++iparser)
	{
		reader = savedReader;
		eErr = (*scte21Parsers[iparser].parser) (
			&reader, length, &bytesParsedSub, pscte_count, pScteData);
		switch (eErr)
		{
		case BERR_SUCCESS:
		case BERR_BUDP_PARSE_ERROR:
			*pBytesParsed += bytesParsedSub;
			BDBG_LEAVE(BUDP_SCTE21parse_isr);
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

	/* Programming note:
	 * None of the parsers liked this bit of userdata.
	 * It would be more elegant to skip to the next start code
	 * before returning.  But it is more efficient to simply
	 * stop parsing. */

	/* No userdata was found */
	BDBG_LEAVE(BUDP_SCTE21parse_isr);
	return BERR_BUDP_NO_DATA;
}


/***************************************************************************
* Implementation of private (static) functions
***************************************************************************/

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
 * This function parses SCTE20 data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseSCTE20Data_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData
)
{
	size_t bitsParsed;
	int index;
	int icount;
	int ifield;
	int cc_count;
	int nrtv_count;
	int field_number;
	BAVC_Polarity targetPolarity;
	BUDP_SCTE_CC_Data ccData;
	BUDP_SCTE_NRTV_Data nrtvData =
	{
		false, 0, 0, 0, 0,
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};

	/* Special case */
	if (length == 0)
	{
		return BERR_BUDP_PARSE_ERROR;
	}

	/* user_data_type_code(8) */
	if (BUDP_Bitread_Byte_isr (pReader) != 0x03)
	{
		/* not SCTE20 data */
		*pBytesParsed = 1;
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
		return BERR_BUDP_PARSE_ERROR;
	}

	/* Skip '1000 000' bits (7) */
	/* vbi_data_flag(1) */
	if (!(BUDP_Bitread_Byte_isr (pReader) & 0x01))
	{
		/* No data for us.  Prevent other parsers from trying though. */
		return BERR_SUCCESS;
	}

	--length;
	*pBytesParsed += 1;
	if (length == 0)
	{
		/* Packet is too short */
		return BERR_BUDP_PARSE_ERROR;
	}

	/* Debug code
	printf ("SCTE 20 parsing under way\n");
	*/

	/*
	 * Closed caption (like) data
	 */

	/* Start the bit level parsing */
	cc_count = BUDP_Bitread_Read_isr (pReader, 5);
	bitsParsed = 5;

	for (icount = 0 ; icount < cc_count ; ++icount)
	{
		/* Each sub-packet of data is 26 bits */
		if (length < 4)
		{
			/* Minimum packet length */
			return BERR_BUDP_PARSE_ERROR;
		}

		/* Not part of SCTE 20 standard! */
		ccData.valid = 1;

		ccData.priority = BUDP_Bitread_Read_isr (pReader, 2);

		field_number = BUDP_Bitread_Read_isr (pReader, 2);
		if (field_number < 1)
		{
			/* Violates spec */
			return BERR_BUDP_PARSE_ERROR;
		}
		ifield = field_number - 1;

		ccData.line_number = BUDP_Bitread_Read_isr (pReader, 5) + 10;

		ccData.cc_data_1 = swap_bits_isr(BUDP_Bitread_Byte_isr (pReader));
		ccData.cc_data_2 = swap_bits_isr(BUDP_Bitread_Byte_isr (pReader));
		/* marker bit(1) */
		(void)BUDP_Bitread_Read_isr (pReader, 1);

		/* Adjust length again */
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

		/* Copy data obtained above into user's space */
		/* TODO: fix this test. Should be using _size attribute. */
		if (pScteData[ifield].data.cc_data)
		{
			*pscte_count = MAX (*pscte_count, ifield + 1);
			pScteData[ifield].format = BUDP_SCTEparse_Format_SCTE20;
			pScteData[ifield].data.field_number = field_number;
			index = pScteData[ifield].data.cc_count;
			if (index < BUDP_SCTE_MAX_ITEMS)
			{
				pScteData[ifield].data.cc_data[index] = ccData;
				pScteData[ifield].data.cc_count = ++index;
			}
			/* TODO: complain if index is too large. */
		}
	}

	/*
	 * Near Real Time Video
	 */

	/* Resume bit level parsing */
	nrtv_count =  BUDP_Bitread_Read_isr (pReader, 4);
	bitsParsed += 4;

	for (icount = 0 ; icount < nrtv_count ; ++icount)
	{
		/* Account for the first half of the sub-packet */
		if (length < 1)
		{
			/* Minimum packet length */
			return BERR_BUDP_PARSE_ERROR;
		}

		/* Not part of SCTE 20 spec */
		nrtvData.valid = 1;

		nrtvData.priority = BUDP_Bitread_Read_isr (pReader, 2);
		nrtvData.sequence_number = BUDP_Bitread_Read_isr (pReader, 2);
		field_number = BUDP_Bitread_Read_isr (pReader, 1);
		nrtvData.line_number = BUDP_Bitread_Read_isr (pReader, 5) + 10;

		/* bitsParsed += 10 */
		if (bitsParsed < 6)
		{
			length -= 1;
			*pBytesParsed += 1;
			bitsParsed += 2;
		}
		else
		{
			length -= 2;
			*pBytesParsed -= 2;
			bitsParsed -= 6;
		}

		/* This field is optional, so it makes for more work here. */
		if (nrtvData.sequence_number != 0)
		{
			/* Account for the second half of the sub-packet */
			if (length < 64)
			{
				/* Minimum packet length */
				return BERR_BUDP_PARSE_ERROR;
			}

			nrtvData.segment_number =  BUDP_Bitread_Read_isr (pReader, 5);
			for (index = 0 ; index < 32 ; ++index)
			{
				nrtvData.y_data[index]      =
					BUDP_Bitread_Read_isr (pReader, 8);
			}
			for (index = 0 ; index < 16 ; ++index)
			{
				nrtvData.cbcr_data[2*index] =
					BUDP_Bitread_Read_isr (pReader, 8);
				nrtvData.cbcr_data[2*index+1] =
					BUDP_Bitread_Read_isr (pReader, 8);
			}

			/* Adjust length again */
			/* bitsParsed += 517 */
			if (bitsParsed < 3)
			{
				length -= 64;
				*pBytesParsed += 64;
				bitsParsed += 5;
			}
			else
			{
				length -= 65;
				*pBytesParsed += 65;
				bitsParsed -= 3;
			}
		}
		else
		{
			nrtvData.segment_number = 0;
		}
		
		/* NRTV data can go top or bottom field */
		targetPolarity =
			field_number ? BAVC_Polarity_eTopField : BAVC_Polarity_eBotField;
		if      (pScteData[0].polarity == targetPolarity)
			ifield = 0;
		else if (pScteData[1].polarity == targetPolarity)
			ifield = 1;
		else
			return BERR_BUDP_PARSE_ERROR;

		/* NRTV data can go on third field if first field is occupied */
		if (pScteData[ifield].data.nrtv_count)
		{
			if (ifield == 0)
			{
				ifield = 2;
				if (pScteData[ifield].polarity != targetPolarity)
					return BERR_BUDP_PARSE_ERROR;
				if (pScteData[ifield].data.nrtv_count)
					return BERR_BUDP_PARSE_ERROR;

			}
			else
			{
				return BERR_BUDP_PARSE_ERROR;
			}
		}

		/* Copy data obtained above into user's space */
		/* TODO: fix this test. Should be using _size attribute. */
		if (pScteData[ifield].data.nrtv_data)
		{
			*pscte_count = MAX (*pscte_count, ifield + 1);
			index = pScteData[ifield].data.nrtv_count;
			if (index < BUDP_SCTE_MAX_ITEMS)
			{
				pScteData[ifield].format                =
					BUDP_SCTEparse_Format_SCTE20;
				pScteData[ifield].data.field_number     = field_number;
				pScteData[ifield].data.nrtv_data[index] = nrtvData;
				pScteData[ifield].data.nrtv_count = ++index;
			}
			/* TODO: complain if index is too large. */
		}
	}

	/* What a journey it was */
	return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses SCTE 21 CC data.  It assumes that the userdata
 * startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseSCTE21CCData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData)
{
	unsigned int index;
	unsigned int icount;
	unsigned int ifield;
	unsigned int cc_count;
	unsigned int cc_valid;
	unsigned int cc_type;
	unsigned int field_number;
	BAVC_Polarity targetPolarity;
	BUDP_SCTE_CC_Data ccData;

	/* Start counting */
	*pscte_count = 0;

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
		/* not SCTE 21 data */
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
		/* not SCTE 21 CC data */
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

	/* Debug code
	printf ("SCTE 21 CC parsing under way\n");
	*/

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
	for (icount = 0 ; icount < cc_count ; ++icount)
	{
		/* Skip marker bits(5) */
		(void)BUDP_Bitread_Read_isr (pReader, 5);

		/* cc_valid(1) */
		cc_valid = BUDP_Bitread_Read_isr (pReader, 1);
		ccData.valid = cc_valid;

		/* Debug code
		if (cc_valid != 1)
			printf ("cc_valid =%d\n", cc_valid);
		*/
	
		/* cc_type(2) */
		cc_type = BUDP_Bitread_Read_isr (pReader, 2);
		/* Bogus */
		field_number = cc_type;

		/* Kludge */
		ccData.line_number = 21;

		/* cc_data_1(8) */
		ccData.cc_data_1 = BUDP_Bitread_Byte_isr (pReader);

		/* cc_data_2(8) */
		ccData.cc_data_2 = BUDP_Bitread_Byte_isr (pReader);

		/* Skip data that is not analog */
		if ((!cc_valid) || (cc_type > 1))
			continue;
		
		/* CC data can go top or bottom field */
		targetPolarity =
			cc_type ? BAVC_Polarity_eTopField : BAVC_Polarity_eBotField;
		if      (pScteData[0].polarity == targetPolarity)
			ifield = 0;
		else if (pScteData[1].polarity == targetPolarity)
			ifield = 1;
		else
			return BERR_BUDP_PARSE_ERROR;

		/* CC data can go in a third field, if first field is occupied. */
		if (pScteData[ifield].data.cc_count)
		{
			if (ifield == 0)
			{
				ifield = 2;
				if (pScteData[ifield].polarity != targetPolarity)
					return BERR_BUDP_PARSE_ERROR;
			}
			else
			{
				return BERR_BUDP_PARSE_ERROR;
			}
		}

		/* Copy data obtained above into user's space */
		/* TODO: fix this test. Should be using _size attribute. */
		if (pScteData[ifield].data.cc_data)
		{
			*pscte_count = MAX (*pscte_count, ifield + 1);
			index = pScteData[ifield].data.cc_count;
			if (index < BUDP_SCTE_MAX_ITEMS)
			{
				/* Not part of SCTE-21 standard */
				ccData.priority = 0;

				pScteData[ifield].format              =
					BUDP_SCTEparse_Format_SCTE21CC;
				pScteData[ifield].data.field_number   = field_number;
				pScteData[ifield].data.cc_data[index] = ccData;
				pScteData[ifield].data.cc_count = ++index;
			}
			/* TODO: complain if index is too large. */
		}
	}
	return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses SCTE21 "additional CC" data.  It assumes that the
 * userdata startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseSCTE21ACCData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData)
{
	unsigned int index;
	unsigned int icount;
	unsigned int ifield;
	unsigned int cc_count;
	unsigned int field_number;
	BUDP_SCTE_CC_Data ccData;

	/* Start counting */
	*pscte_count = 0;

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
		/* not SCTE21ACC data */
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
		/* not SCTE21ACC data */
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

	/* Debug code
	printf ("SCTE 21 A-CC parsing under way\n");
	*/

	/*
	 * Begin processing additional EIA-608 closed caption data
	 */

	/* Skip marker bits(3) and dig out the count of cc data pairs(5) */
	cc_count = BUDP_Bitread_Byte_isr (pReader) & 0x1F;

	/* This will be the last length check for standard CC data */
	--length;
	*pBytesParsed += 1;
	if (length < (3 * cc_count))
	{
		/* Packet too short */
		return BERR_BUDP_PARSE_ERROR;
	}
	*pBytesParsed += (3 * cc_count);

	/* Dig out the closed caption data pairs */
	for (icount = 0 ; icount < cc_count ; ++icount)
	{
		/* additional_cc_valid(1) */
		ccData.valid = BUDP_Bitread_Read_isr (pReader, 1);
	
		/* additional_cc_line_offset(5) */
		ccData.line_number = BUDP_Bitread_Read_isr (pReader, 5) + 9;

		/* additional_cc_field_number(2) */
		field_number = BUDP_Bitread_Read_isr (pReader, 2);
		if (field_number < 1)
		{
			/* Violates spec */
			return BERR_BUDP_PARSE_ERROR;
		}
		ifield = field_number - 1;

		/* additional_cc_data_1(8) */
		ccData.cc_data_1 = BUDP_Bitread_Byte_isr (pReader);

		/* additional_cc_data_2(8) */
		ccData.cc_data_2 = BUDP_Bitread_Byte_isr (pReader);

		/* Skip data that is not analog */
		if (!ccData.valid)
			continue;

		/* Copy data obtained above into user's space */
		/* TODO: fix this test. Should be using _size attribute. */
		if (pScteData[ifield].data.cc_data)
		{
			*pscte_count = MAX (*pscte_count, ifield + 1);
			index = pScteData[ifield].data.cc_count;
			if (index < BUDP_SCTE_MAX_ITEMS)
			{
				/* Not part of SCTE-21 standard */
				ccData.priority = 0;

				pScteData[ifield].format              =
					BUDP_SCTEparse_Format_SCTE21ACC;
				pScteData[ifield].data.field_number   = field_number;
				pScteData[ifield].data.cc_data[index] = ccData;
				pScteData[ifield].data.cc_count = ++index;
			}
			/* TODO: complain if index is too large. */
		}
	}

	/* Another happy landing */
	return BERR_SUCCESS;
}

/***************************************************************************
 * This function parses SCTE21 luma PAM data.  It assumes that the
 * userdata startcode 0x000001B2 has just been read.
 */
static BERR_Code ParseSCTE21PAMData_isr (
	BUDP_Bitread_Context* pReader,
	size_t					 length,
	size_t*                  pBytesParsed,
	uint8_t*                 pscte_count,
	BUDP_SCTEparse_sctedata* pScteData)
{
	unsigned int ifield;
	size_t bytesParsedSub;
	uint8_t* pam_data;

	/* Start counting */
	*pscte_count = 0;

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
		/* not SCTE21PAM data */
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
	if (BUDP_Bitread_Byte_isr (pReader) != 0x05)
	{
		/* not SCTE21PAM data */
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

	/* Debug code
	printf ("SCTE 21 PAM parsing under way\n");
	*/

	/* Record the luma data start */
	pam_data = (uint8_t*)(pReader->userdata_start);

	/* Skip past data, and then record data size. */
	(void)BUDP_Bitread_next_start_code_isr (pReader, length, &bytesParsedSub);
	length -=  bytesParsedSub;
	*pBytesParsed +=  bytesParsedSub;

	/* Loop over fields indicated in luma_PAM_data() structure */
	for (ifield = 0 ; ifield < 3 ; ++ifield)
	{
		if (pScteData[ifield].polarity == BAVC_Polarity_eFrame)
			continue;

		/* Copy data obtained above into user's space */
		/* TODO: fix this test. Should be using _size attribute. */
		if (pScteData[ifield].data.luma_PAM_data)
		{
			*pscte_count = MAX (*pscte_count, ifield + 1);
			pScteData[ifield].data.pam_size = bytesParsedSub;
			pScteData[ifield].format = BUDP_SCTEparse_Format_SCTE21PAM;
			BKNI_Memcpy (
				pScteData[ifield].data.luma_PAM_data, pam_data, bytesParsedSub);

			/* It is a shame that this is necessary */
			if (bByteswap)
			{
				P_EndSwap_isr (
					pScteData[ifield].data.luma_PAM_data, bytesParsedSub);
			}
		}
	}

	/* Another happy landing */
	return BERR_SUCCESS;
}

/***************************************************************************
 * This function initializes an array of BUDP_SCTEparse_sctedata according
 * to one piece of SCTE 20 userdata or SCTE 21 userdata.
 */
static void P_ArrayInit_SCTE_isr (
	BUDP_SCTEparse_sctedata* pSCTEdata,
	const BAVC_USERDATA_info*   pUserdata_info)
{
	int entry;
	bool top_field_first           = pUserdata_info->bTopFieldFirst;
	bool repeat_first_field        = pUserdata_info->bRepeatFirstField;
	BAVC_Polarity pictureStructure = pUserdata_info->eSourcePolarity;

	/* Dumb initialization */
	for (entry = 0 ; entry < 3 ; ++entry)
	{
		BUDP_SCTEparse_sctedata* sldata = &pSCTEdata[entry];
		BUDP_SCTE_Data* pidata = &(sldata->data);
		sldata->polarity = BAVC_Polarity_eFrame;
		sldata->format = BUDP_SCTEparse_Format_Unknown;
		pidata->cc_count = 0;
		pidata->nrtv_count = 0;
		pidata->pam_count = 0;
	}

	/* According to the MPEG userdata... */
	if ((pictureStructure == BAVC_Polarity_eTopField) ||
	    (pictureStructure == BAVC_Polarity_eBotField)   )
	{
		/* Field picture */
		pSCTEdata[0].polarity = pictureStructure;
	}
	else
	{
		/* Frame picture */
		if (top_field_first)
		{
			pSCTEdata[0].polarity = BAVC_Polarity_eTopField;
			pSCTEdata[1].polarity = BAVC_Polarity_eBotField;
			if (repeat_first_field)
			{
				pSCTEdata[2].polarity = BAVC_Polarity_eTopField;
			}
		}
		else /* Bottom field first */
		{
			pSCTEdata[0].polarity = BAVC_Polarity_eBotField;
			pSCTEdata[1].polarity = BAVC_Polarity_eTopField;
			if (repeat_first_field)
			{
				pSCTEdata[2].polarity = BAVC_Polarity_eBotField;
			}
		}
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
 * This function performs a big-endian / little-endian swap
*/
static void P_EndSwap_isr (uint8_t* byteData, size_t byteLength)
{
	size_t qLength = byteLength / 4;

	while (qLength-- > 0)
	{
		uint8_t entry;
		entry       = byteData[0];
		byteData[0] = byteData[3];
		byteData[3] = entry;
		entry       = byteData[1];
		byteData[1] = byteData[2];
		byteData[2] = entry;
		byteData += 4;
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

#endif /** } !B_REFSW_MINIMAL **/
