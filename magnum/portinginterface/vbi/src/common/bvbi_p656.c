/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * This module provides software parsing of ITU-R 656 ancillary data packets
 * for the VBI porting interface (BVBI).  This module is private to BVBI.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"			/* standard types */
#include "bkni.h"			/* Just for zeroing out DRAM */
#include "bdbg.h"			/* Dbglib */
#include "bvbi.h"			/* VBI processing, this module. */
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);

/* This indicates how to handle the various video standards within this
   module.  It reflects the fact that only NTSC and PAL are supported. */
typedef enum {
	BVBI_P_SupportedVideo_Not=0,
	BVBI_P_SupportedVideo_NTSC,
	BVBI_P_SupportedVideo_PAL

} BVBI_P_SupportedVideo;

/* Function type for parsing the various ancillary data formats */
typedef BERR_Code (*BVBI_P_P656_AncilParser) (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

static BVBI_P_SupportedVideo BVBI_P_PALorNTSC_isr (BFMT_VideoFmt eVideoFormat);

static BERR_Code BVBI_P_P656_Alloc   (
	BMEM_Handle hMem, uint8_t** i656data, size_t nBytes);
static void      BVBI_P_P656_DeAlloc (
	BMEM_Handle hMem, uint8_t** i656data);

/* The array of parsers.  The implied index is the enum BVBI_656Fmt. */
static BERR_Code BVBI_P_P656_SAA7113Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData);
static BERR_Code BVBI_P_P656_SAA7114Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData);
static BERR_Code BVBI_P_P656_SAA7115Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData);
static BERR_Code BVBI_P_P656_SMPTE291Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData);
static const BVBI_P_P656_AncilParser AncilParserArray[BVBI_656Fmt_LAST] = {
	BVBI_P_P656_SAA7113Parser,
	BVBI_P_P656_SAA7113Parser,
	BVBI_P_P656_SAA7114Parser,
	BVBI_P_P656_SAA7114Parser,
	BVBI_P_P656_SAA7114Parser,
	BVBI_P_P656_SAA7114Parser,
	BVBI_P_P656_SAA7115Parser,
	BVBI_P_P656_SMPTE291Parser
};

/* The data movement functions for individual VBI data types */
static uint32_t BVBI_P_P656_MoveCC_isr  (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, uint16_t* usCCData);
static uint32_t BVBI_P_P656_MoveWSS_isr (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, uint16_t* usWSSData);
static uint32_t BVBI_P_P656_MoveTT_isr  (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, BVBI_P_TTData* TTData);
/* TODO: VPS mover */

/* Various parity bit checkers */
static BERR_Code P_CheckE (uint8_t dataByte);
static BERR_Code P_CheckEE (uint8_t dataByte);
static BERR_Code P_CheckO (uint8_t dataByte);
static BERR_Code P_CheckEE11 (uint8_t dataByte, uint8_t* payload);

/* Simultaneously check for SAV/EAV control code, and 16-byte end-of-data
   marker */
static uint8_t* P_CheckControl (uint8_t* start, uint8_t* end);

/* Check for SMPTE-291M ancillary data packet flag */
static uint8_t* P_CheckControl_291 (uint8_t* start, uint8_t* end);

/* (even) parity of numbers between 0 and 255 */
static const unsigned char P_bitParity[256] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting VBI_DEC functions that are not in API
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_P_P656_Init ( BVBI_P_Decode_Handle* pVbi_Dec )
{
	void* cached_ptr;
	BVBI_P_Handle* pVbi = pVbi_Dec->pVbi;
	BMEM_Handle hMem = pVbi->hMem;
	BERR_Code eErr = BERR_SUCCESS;

	/* Allocate space for double buffering ITU-R 656 ancillary data */
	if ((eErr = BERR_TRACE (BVBI_P_P656_Alloc (
		pVbi->hMem, &pVbi_Dec->top656Data, pVbi->in656bufferSize))) !=
			BERR_SUCCESS)
	{
		goto init_done;
	}
	if ((eErr = BERR_TRACE (BVBI_P_P656_Alloc (
		pVbi->hMem, &pVbi_Dec->bot656Data, pVbi->in656bufferSize))) !=
			BERR_SUCCESS)
	{
		goto init_done;
	}

	/* Zero out the data */
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached (
		hMem, pVbi_Dec->top656Data, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		goto init_done;
	}
	BKNI_Memset (cached_ptr, 0, pVbi->in656bufferSize);
	BMEM_FlushCache (hMem, cached_ptr, pVbi->in656bufferSize);
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached (
		hMem, pVbi_Dec->bot656Data, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		goto init_done;
	}
	BKNI_Memset (cached_ptr, 0, pVbi->in656bufferSize);
	BMEM_FlushCache (hMem, cached_ptr, pVbi->in656bufferSize);

init_done:

	if (eErr != BERR_SUCCESS)
	{
		if (pVbi_Dec->top656Data != NULL)
		{
			BVBI_P_P656_DeAlloc (hMem, &(pVbi_Dec->top656Data));
		}
		if (pVbi_Dec->bot656Data != NULL)
		{
			BVBI_P_P656_DeAlloc (hMem, &(pVbi_Dec->bot656Data));
		}
	}
	return eErr;
}

/***************************************************************************
 *
 */
void BVBI_P_P656_DeInit ( BVBI_P_Decode_Handle* pVbi_Dec )
{
	BVBI_P_Handle* pVbi = pVbi_Dec->pVbi;

	/* Release space for double buffering ITU-R 656 ancillary data */
	BVBI_P_P656_DeAlloc (pVbi->hMem, &pVbi_Dec->top656Data);
	BVBI_P_P656_DeAlloc (pVbi->hMem, &pVbi_Dec->bot656Data);
}


/***************************************************************************
 *
 */
BERR_Code BVBI_P_P656_Process_Data_isr (
	BAVC_Polarity polarity,
	BVBI_P_Decode_Handle* pVbi_Dec,
	BVBI_P_Field_Handle* pVbi_Fld)
{
	uint8_t* rawData;
	uint8_t* currData;
	void* cached_ptr;
	uint8_t* vbiData;
	int currLength;
	int lineWidth;
	uint32_t packetLength;
	uint32_t whatActive;
	bool isPal;
	BVBI_P_P656_AncilParser ancilParser;
	BVBI_SMPTE291M_Description pktInfo;
	BVBI_P_TTData* ttData;
	BMEM_Handle hMem = pVbi_Dec->pVbi->hMem;
	BERR_Code eErr = BERR_SUCCESS;
	uint32_t ulErrInfo = 0x0;

	BDBG_ENTER (BVBI_P_P656_Process_Data_isr);

	/* Determine where data is coming from */
	switch (polarity)
	{
	case BAVC_Polarity_eTopField:
		rawData = pVbi_Dec->top656Data;
		break;
	case BAVC_Polarity_eBotField:
		rawData = pVbi_Dec->bot656Data;
		break;
	default:
		BDBG_LEAVE (BVBI_P_P656_Process_Data_isr);
		return BERR_TRACE (BERR_INVALID_PARAMETER);
		break;
	}

	/* Use cached memory interface to DRAM */
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
		hMem, rawData, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		return eErr;
	}
	BMEM_FlushCache_isr (hMem, cached_ptr, pVbi_Dec->pVbi->in656bufferSize);

	/* Will process captured data in multiple passes */
	currData = (uint8_t*)cached_ptr;
	currLength = pVbi_Dec->pVbi->in656bufferSize;

	/* Determine if PAL or NTSC (for teletext, mostly). */
	switch (BVBI_P_PALorNTSC_isr (pVbi_Dec->curr.eVideoFormat))
	{
    case BVBI_P_SupportedVideo_NTSC:
		isPal = false;
		lineWidth = 34;
		break;

    case BVBI_P_SupportedVideo_PAL:
		isPal = true;
		lineWidth = 43;
		break;

	default:
		lineWidth = 0;
		isPal = false;
		break;
	}

	/* Initialize teletext data in field handle, if present. */
	BVBI_P_LCOP_WRITE_isr (
		pVbi_Fld, TTDataO, &pVbi_Dec->pVbi->ttFreelist, clink);
	ttData = BVBI_P_LCOP_GET_isr (pVbi_Fld, TTDataO);
	ttData->ucLineSize = lineWidth;
	ttData->ucLines = 0;
	if (ttData->ucDataSize > 0)
		*(uint32_t*)(ttData->pucData) = 0x00000000;

	/* Determine which ancillary data parser to use */
	ancilParser = AncilParserArray[pVbi_Dec->curr.e656Format];

	/* Convenience */
	whatActive = pVbi_Dec->curr.ulActive_Standards;

	/* Loop over captured ancillary data packets */
	while (currLength > 0)
	{
		if ((*ancilParser) (pVbi_Dec->curr.eVideoFormat, polarity,
							&(pVbi_Dec->curr.SMPTE291Moptions), currData,
							currLength, &pktInfo, &packetLength, &vbiData)
			== BERR_SUCCESS)
		{
			/* Determine which VBI standard to parse */
			if ((whatActive & BVBI_P_SELECT_CC) &&
				((pktInfo.vbiType == BVBI_656_VbiType_EuroCC) ||
				 (pktInfo.vbiType == BVBI_656_VbiType_USCC  )))
			{
				ulErrInfo = BVBI_P_P656_MoveCC_isr (
					vbiData, &pktInfo, isPal, &pVbi_Fld->usCCData);
				if ((ulErrInfo & BVBI_LINE_ERROR_CC_NODATA) == 0)
					pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_CC;
				pVbi_Fld->ulErrInfo |= ulErrInfo;
			}
			if ((whatActive & BVBI_P_SELECT_WSS) &&
				(pktInfo.vbiType == BVBI_656_VbiType_WSS))
			{
				ulErrInfo = BVBI_P_P656_MoveWSS_isr (
					vbiData, &pktInfo, isPal, &pVbi_Fld->usWSSData);
				if ((ulErrInfo & BVBI_LINE_ERROR_WSS_NODATA) == 0)
					pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_WSS;
				pVbi_Fld->ulErrInfo |= ulErrInfo;
			}
			if ((whatActive & BVBI_P_SELECT_TT) &&
				((pktInfo.vbiType == BVBI_656_VbiType_TT   ) ||
				 (pktInfo.vbiType == BVBI_656_VbiType_NABTS)))
			{
				ulErrInfo = BVBI_P_P656_MoveTT_isr (
					vbiData, &pktInfo, isPal, ttData);
				if ((ulErrInfo & BVBI_LINE_ERROR_TELETEXT_NODATA) == 0)
					pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_TT;
				pVbi_Fld->ulErrInfo |= ulErrInfo;
			}
			/* TODO: VPS decoding. */
		}

		/* Advance to next ancillary data packet */
		currData   += packetLength;
		currLength -= packetLength;
		if (currLength < 5)
			break;
	}

	BDBG_LEAVE (BVBI_P_P656_Process_Data_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
uint8_t BVBI_P_p656_SetEEbits (uint8_t arg)
{
	uint8_t retval = arg & 0x3F;
	uint8_t parityBit = P_bitParity[retval];
	uint8_t oppositeBit = !parityBit;
	return retval | (parityBit << 6) | (oppositeBit << 7);
}

/***************************************************************************
* Static (private) functions
***************************************************************************/


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_P656_Alloc (
	BMEM_Handle hMem, uint8_t** i656data, size_t nBytes)
{
	*i656data =
		(uint8_t*)(BMEM_AllocAligned (
			hMem,
			nBytes,
			5,
			0));
	if (!(*i656data))
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static void BVBI_P_P656_DeAlloc (
	BMEM_Handle hMem, uint8_t** i656data)
{
	BMEM_Free (hMem, *i656data);
	*i656data = 0;
}


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_P656_SAA7113Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData)
{
	uint16_t msn16, lsn16;
	uint8_t vbiType;
	int nBytes;
	BAVC_Polarity parsedPolarity;
	uint8_t* currData = rawData;
	uint8_t* endData  = rawData + rawLength;
	uint8_t* outData  = rawData;
	BERR_Code eErr = BERR_SUCCESS;

	/* This is part of the SAA7113 syntax */
	static const uint8_t payloadSize[16] = {
		/*  0: -------------------- */	 0,
		/*  1: Euro. Closed Caption */	 2,
		/*  2: -------------------- */	 0,
		/*  3: WSS                  */	 3,
		/*  4: WST (not supported)  */	34,
		/*  5: U.S. Closed Caption  */	 2,
		/*  6: -------------------- */	 0,
		/*  7: -------------------- */	 0,
		/*  8: Euro. teletext       */	42,
		/*  9: -------------------- */	 0,
		/* 10: -------------------- */	 0,
		/* 11: -------------------- */	 0,
		/* 12: NABTS                */	34,
		/* 13: -------------------- */	 0,
		/* 14: -------------------- */	 0,
		/* 15: -------------------- */	 0
	};

	/* This translates from SAA7113 syntax to BVBI private enum */
	static const BVBI_656_VbiType payloadType[16] = {
		/*  0: -------------------- */	 BVBI_656_VbiType_None,
		/*  1: Euro. Closed Caption */	 BVBI_656_VbiType_EuroCC,
		/*  2: -------------------- */	 BVBI_656_VbiType_None,
		/*  3: WSS                  */	 BVBI_656_VbiType_WSS,
		/*  4: WST (not supported)  */	 BVBI_656_VbiType_None,
		/*  5: U.S. Closed Caption  */	 BVBI_656_VbiType_USCC,
		/*  6: -------------------- */	 BVBI_656_VbiType_None,
		/*  7: -------------------- */	 BVBI_656_VbiType_None,
		/*  8: Euro. teletext       */	 BVBI_656_VbiType_TT,
		/*  9: -------------------- */	 BVBI_656_VbiType_None,
		/* 10: -------------------- */	 BVBI_656_VbiType_None,
		/* 11: -------------------- */	 BVBI_656_VbiType_None,
		/* 12: NABTS                */	 BVBI_656_VbiType_NABTS,
		/* 13: -------------------- */	 BVBI_656_VbiType_None,
		/* 14: -------------------- */	 BVBI_656_VbiType_None,
		/* 15: -------------------- */	 BVBI_656_VbiType_None
	};

	/* This translates from SAA7113 syntax to another BVBI private enum */
	static const BVBI_P_SupportedVideo videoType[16] = {
		/*  0: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  1: Euro. Closed Caption */	 BVBI_P_SupportedVideo_PAL,
		/*  2: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  3: WSS                  */	 BVBI_P_SupportedVideo_PAL,
		/*  4: WST (not supported)  */	 BVBI_P_SupportedVideo_Not,
		/*  5: U.S. Closed Caption  */	 BVBI_P_SupportedVideo_NTSC,
		/*  6: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  7: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  8: Euro. teletext       */	 BVBI_P_SupportedVideo_PAL,
		/*  9: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 10: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 11: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 12: NABTS                */	 BVBI_P_SupportedVideo_NTSC,
		/* 13: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 14: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 15: -------------------- */	 BVBI_P_SupportedVideo_Not
	};

	BSTD_UNUSED (pMoptions);

	/* Will overwrite input data */
	*vbiData = rawData;

	/* Search for three byte header or end-of-data */
	currData = P_CheckControl (currData, endData);

	/* Bail out if no data found */
	if ((currData + 5) >= endData)
	{
		*packetLength = rawLength;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}

	/* TODO: parse the SAV byte */
	++currData;

	/* Check the SOURCE ID byte for corruption */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	++currData;

	/* Dig out and ignore the DWORD COUNT */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	++currData;

	/* Dig out (part of) line count and field indicator,
	   checking for corruption. */
	if (P_CheckO (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	parsedPolarity =
		(((*currData) & 0x40) == 0) ?
			BAVC_Polarity_eTopField : BAVC_Polarity_eBotField;
	pktInfo->polarity = parsedPolarity;
	msn16 = (*currData) & 0x3f;
	++currData;

	/* Insist on agreement for field parity */
	if (parsedPolarity != polarity)
	{
		/* Programming note: Broadcom ANCI_656 core does not
		   provide parity bit. */
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}

	/* Dig out (part of) line count and VBI type indicator,
	   checking for corruption. */
	if (P_CheckO (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	lsn16 = ((*currData) & 0x70) >> 4;
	pktInfo->lineNumber = (msn16 << 3) | lsn16;
	vbiType = (*currData) & 0x0F;
	++currData;

	/* Check for unsupported VBI type, otherwise return info to caller. */
	if (payloadType[vbiType] == BVBI_656_VbiType_None)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	pktInfo->vbiType = payloadType[vbiType];

	/* Check for conflict with video standard (PAL vs NTSC) */
	if (videoType[vbiType] != BVBI_P_PALorNTSC_isr (eVideoFormat))
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}

	/* Compute the "payload" size */
	nBytes = payloadSize[vbiType];
	if (nBytes == 0)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	if (currData + 2 * nBytes > endData)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}

	/* Dig out the "payload" */
	while (nBytes-- > 0)
	{
		uint8_t msn = 0x0;
		uint8_t lsn = 0x0;
		if (P_CheckEE11 (*currData, &lsn) != BERR_SUCCESS)
		{
			*packetLength = currData - rawData;
			eErr = BVBI_ERR_656_PARSE;
			goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
		}
		++currData;
		if (P_CheckEE11 (*currData, &msn) != BERR_SUCCESS)
		{
			*packetLength = currData - rawData;
			eErr = BVBI_ERR_656_PARSE;
			goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
		}
		++currData;
		*outData++ = (msn << 4) | lsn;
	}

	/* "Parse" the packet suffix */
	if (currData + 4 > endData)
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	/* TODO: parse the EAV byte */
	if ((currData[0] != 0xFF) || (currData[1] != 0x00) ||
	    (currData[2] != 0x00))
	{
		*packetLength = currData - rawData;
		eErr = BVBI_ERR_656_PARSE;
		goto ErrorTransfer_BVBI_P_P656_SAA7113Parser;
	}
	currData += 4;

	/* Packet parsing complete! */
	*packetLength = currData - rawData;

ErrorTransfer_BVBI_P_P656_SAA7113Parser:
	return eErr;

}


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_P656_SAA7114Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData)
{
	uint16_t msn16, lsn16;
	uint8_t vbiType;
	uint8_t checksum;
	int nBytes;
	int dataByteCounter;
	BAVC_Polarity parsedPolarity;
	uint8_t* currData = rawData;
	uint8_t* endData  = rawData + rawLength;
	uint8_t* outData  = rawData;

	/* This is part of the SAA7114 syntax */
	static const uint8_t payloadSize[16] = {
		/*  0: -------------------- */	 0,
		/*  1: Euro. Closed Caption */	 2,
		/*  2: VPS (TODO)           */	 0,
		/*  3: WSS                  */	 3,
		/*  4: NABTS; fr. code x27  */	34,
		/*  5: U.S. Closed Caption  */	 2,
		/*  6: -------------------- */	 0,
		/*  7: -------------------- */	 0,
		/*  8: WST (not supported)  */	42,
		/*  9: -------------------- */	 0,
		/* 10: -------------------- */	 0,
		/* 11: -------------------- */	 0,
		/* 12: NABTS                */	34,
		/* 13: -------------------- */	 0,
		/* 14: -------------------- */	 0,
		/* 15: -------------------- */	 0
	};

	/* This translates from SAA7114 syntax to BVBI private enum */
	static const BVBI_656_VbiType payloadType[16] = {
		/*  0: -------------------- */	 BVBI_656_VbiType_None,
		/*  1: Euro. Closed Caption */	 BVBI_656_VbiType_EuroCC,
		/*  2: -------------------- */	 BVBI_656_VbiType_None,
		/*  3: WSS                  */	 BVBI_656_VbiType_WSS,
		/*  4: WST (not supported)  */	 BVBI_656_VbiType_None,
		/*  5: U.S. Closed Caption  */	 BVBI_656_VbiType_USCC,
		/*  6: -------------------- */	 BVBI_656_VbiType_None,
		/*  7: -------------------- */	 BVBI_656_VbiType_None,
		/*  8: Euro. teletext       */	 BVBI_656_VbiType_TT,
		/*  9: -------------------- */	 BVBI_656_VbiType_None,
		/* 10: -------------------- */	 BVBI_656_VbiType_None,
		/* 11: -------------------- */	 BVBI_656_VbiType_None,
		/* 12: NABTS                */	 BVBI_656_VbiType_NABTS,
		/* 13: -------------------- */	 BVBI_656_VbiType_None,
		/* 14: -------------------- */	 BVBI_656_VbiType_None,
		/* 15: -------------------- */	 BVBI_656_VbiType_None
	};

	/* This translates from SAA7114 syntax to another BVBI private enum */
	static const BVBI_P_SupportedVideo videoType[16] = {
		/*  0: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  1: Euro. Closed Caption */	 BVBI_P_SupportedVideo_PAL,
		/*  2: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  3: WSS                  */	 BVBI_P_SupportedVideo_PAL,
		/*  4: WST (not supported)  */	 BVBI_P_SupportedVideo_Not,
		/*  5: U.S. Closed Caption  */	 BVBI_P_SupportedVideo_NTSC,
		/*  6: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  7: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/*  8: Euro. teletext       */	 BVBI_P_SupportedVideo_PAL,
		/*  9: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 10: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 11: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 12: NABTS                */	 BVBI_P_SupportedVideo_NTSC,
		/* 13: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 14: -------------------- */	 BVBI_P_SupportedVideo_Not,
		/* 15: -------------------- */	 BVBI_P_SupportedVideo_Not
	};

	BSTD_UNUSED (pMoptions);

	/* Will overwrite input data */
	*vbiData = rawData;

	/* Search for three byte header or end-of-data */
	currData = P_CheckControl (currData, endData);

	/* Bail out if no data found */
	if ((currData + 5) >= endData)
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}


	/* Checksum begins with SAV byte */
	/* TODO: Parse the SAV byte */
	checksum = *currData;
	++currData;

	/* Check the SOURCE ID byte for corruption */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	checksum += (*currData);
	++currData;

	/* Dig out the DWORD COUNT */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	dataByteCounter = 4 * ((*currData) & 0x3F);
	checksum += (*currData);
	++currData;

	/* Dig out (part of) line count and field indicator,
	   checking for corruption. */
	if (P_CheckO (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	checksum += (*currData);
	parsedPolarity =
		(((*currData) & 0x40) == 0) ?
			BAVC_Polarity_eTopField : BAVC_Polarity_eBotField;
	pktInfo->polarity = parsedPolarity;
	msn16 = (*currData) & 0x3f;
	++currData;
	--dataByteCounter;

	/* Insist on agreement for field parity */
	if (parsedPolarity != polarity)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Dig out (part of) line count and VBI type indicator,
	   checking for corruption. */
	if (P_CheckO (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	checksum += (*currData);
	--dataByteCounter;
	lsn16 = ((*currData) & 0x70) >> 4;
	pktInfo->lineNumber = (msn16 << 3) | lsn16;
	vbiType = (*currData) & 0x0F;
	++currData;

	/* Check for unsupported VBI type, otherwise return info to caller. */
	if (payloadType[vbiType] == BVBI_656_VbiType_None)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	pktInfo->vbiType = payloadType[vbiType];

	/* Check for conflict with video standard (PAL vs NTSC) */
	if (videoType[vbiType] != BVBI_P_PALorNTSC_isr (eVideoFormat))
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Compute the "payload" size */
	nBytes = payloadSize[vbiType];
	if (nBytes == 0)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	if (currData + 2 * nBytes > endData)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Dig out the "payload" */
	while (nBytes-- > 0)
	{
		*outData++ = *currData;
		checksum += (*currData);
		--dataByteCounter;
		++currData;
	}

	/* Count the fill bytes */
	while (dataByteCounter-- > 0)
	{
		checksum += (*currData);
		++currData;
	}

	/* Verify the checksum byte */
	if (P_CheckE (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	if ( ((*currData) & 0x7F) != (checksum & 0x7F) )
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Parse out the BYTE COUNT byte */
	if (P_CheckO (*currData) != BERR_SUCCESS)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	/* TODO: interpret the BYTE COUNT byte */
	++currData;

	/* "Parse" the packet suffix */
	if (currData + 4 > endData)
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	/* TODO: parse the EAV byte */
	if ((currData[0] != 0xFF) || (currData[1] != 0x00) ||
	    (currData[2] != 0x00))
	{
		*packetLength = currData - rawData;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	currData += 4;

	/* Packet parsing complete! */
	*packetLength = currData - rawData;
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_P656_SAA7115Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData)
{
	/* TODO: check for differences between SAA7114 and SAA 7115 */
	return
		BERR_TRACE (BVBI_P_P656_SAA7114Parser (
			eVideoFormat, polarity, pMoptions, rawData, rawLength, pktInfo,
			packetLength, vbiData));
}


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_P656_SMPTE291Parser (
	BFMT_VideoFmt eVideoFormat, BAVC_Polarity polarity,
	BVBI_P_SMPTE291Moptions* pMoptions, uint8_t* rawData, int rawLength,
	BVBI_SMPTE291M_Description* pktInfo, uint32_t* packetLength,
	uint8_t** vbiData)
{
	uint8_t checksum;
	uint8_t data_id;
	uint8_t second_id = 0x0;
	uint8_t data_count;
	int dataByteCounter;
	uint8_t* packet;
	uint8_t* currData = rawData;
	uint8_t* endData  = rawData + rawLength;

	/* Search for header */
	if (pMoptions->bLongHeader)
	{
		/* Search for three byte header or end-of-data */
		currData = P_CheckControl_291 (currData, endData);
	}
	else
	{
		while (currData < endData)
		{
			if (currData[0] == 0xFF)
			{
				break;
			}
			++currData;
		}
		++currData;
	}

	/* Bail out if no data found */
	if ((currData + 5) >= endData)
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Checksum begins with DATA ID byte */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	data_id = (*currData) & 0x3F;
	if (data_id == 0x0)
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	checksum = *currData;
	++currData;

	/* Second Data ID or block number: */
	/* Check the SECOND ID byte for corruption */
	if (P_CheckEE (*currData) != BERR_SUCCESS)
	{
		/* Programming note: SDID seems to be always "1"
		   coming from Broadcom's ANCI_656_656 core. */
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}
	second_id = (*currData) & 0x3F;
	checksum += (*currData);
	++currData;

	/* Dig out the DWORD COUNT */
	if (pMoptions->bBrokenDataCount)
	{
		data_count = *currData;
		dataByteCounter = data_count;
	}
	else
	{
		if (P_CheckEE (*currData) != BERR_SUCCESS)
		{
			/* Programming note: Broadcom's ANCI656_656
			   core doesn't add parity bits */
			*packetLength = rawLength;
			return BERR_TRACE (BVBI_ERR_656_PARSE);
		}
		data_count = (*currData) & 0x3F;
		dataByteCounter = 4 * data_count;
	}
	checksum += (*currData);
	++currData;

	/* This is needed to avoid infinite loop in calling function */
	if (dataByteCounter == 0)
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* Here is the raw data to pass to the callback function */
	packet = currData;

	/* Checksum the "payload" */
	while (dataByteCounter > 0)
	{
		checksum += (*currData);
		++currData;
		--dataByteCounter;
	}

	/* Verify the checksum byte */
	if ( (*currData) != checksum )
	{
		*packetLength = rawLength;
		return BERR_TRACE (BVBI_ERR_656_PARSE);
	}

	/* The user callback does the rest */
	if (!(pMoptions->bBrokenDataCount))
		data_count *= 4;
	return BERR_TRACE ((pMoptions->fParseCb) (
		pMoptions->fParseCbArg0,
		eVideoFormat,
		polarity,
		data_id,
		second_id,
		data_count,
		packet,
		vbiData,
		pktInfo));
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Decode_656_SMPTE291MbrcmCb_isr (
	void* arg0,
	BFMT_VideoFmt eVideoFormat,
	BAVC_Polarity polarity,
	uint8_t data_id,
	uint8_t second_id,
	uint8_t data_count,
	uint8_t* packet,
    uint8_t**  vbiData,
	BVBI_SMPTE291M_Description* pktDesc
)
{
	bool isPal;
	uint8_t bytesRequired;

	BSTD_UNUSED (arg0);
	BSTD_UNUSED (second_id);

	/* PAL or NTSC? */
	switch (BVBI_P_PALorNTSC_isr (eVideoFormat))
	{
	case BVBI_P_SupportedVideo_PAL:
		isPal = true;
		break;
	case BVBI_P_SupportedVideo_NTSC:
		isPal = false;
		break;
	default:
		return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
		break;
	}

	/* The raw VBI data is (almost) immediate */
	*vbiData = packet + 2;

	/* Some of the descriptive information cannot be checked, just echo
	   defaults and hope for the best. */
	pktDesc->polarity     = polarity;
	pktDesc->lineNumber   = (packet[0] << 8) | packet[1];

	/* Interpret the proprietary VBI type indicator */
	switch (data_id)
	{
	case 0x05:
		pktDesc->vbiType = BVBI_656_VbiType_USCC;
		bytesRequired = 4;
		break;
	case 0x01:
		pktDesc->vbiType = BVBI_656_VbiType_EuroCC;
		bytesRequired = 4;
		break;
	case 0x04:
		/* TODO: verify byte count */
		pktDesc->vbiType = BVBI_656_VbiType_NABTS;
		bytesRequired = 34;
		break;
	case 0x08:
		/* TODO: verify byte count */
		pktDesc->vbiType = BVBI_656_VbiType_TT;
		bytesRequired = 43;
		break;
	case 0x03:
		pktDesc->vbiType = BVBI_656_VbiType_WSS;
		/* TODO: verify this: */
		bytesRequired = 3;
		break;
	/* TODO: fix this numeric code */
	case 0x33:
		pktDesc->vbiType =
			isPal ? BVBI_656_VbiType_VPS : BVBI_656_VbiType_None;
		/* TODO: verify this: */
		bytesRequired = 2;
		break;
	default:
		pktDesc->vbiType = BVBI_656_VbiType_None;
		bytesRequired = 0;
		break;
	}
	if (pktDesc->vbiType == BVBI_656_VbiType_None)
		return BERR_TRACE (BVBI_ERR_656_PARSE);

	/* Verify that there is enough data to copy */
	if (data_count < bytesRequired)
		return BERR_TRACE (BVBI_ERR_656_PARSE);

	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static uint32_t BVBI_P_P656_MoveCC_isr  (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, uint16_t* usCCData)
{
	uint32_t retval = 0x0;

	/* Consistency checks */
	if (pktInfo->lineNumber != 0)
	{
		int correctLine = isPal ?
			((pktInfo->polarity == BAVC_Polarity_eTopField) ? 22 : 335) :
			((pktInfo->polarity == BAVC_Polarity_eTopField) ? 21 : 284) ;
		if (pktInfo->lineNumber != correctLine)
		{
			retval = BVBI_LINE_ERROR_CC_NODATA;
			goto ErrorTransfer_BVBI_P_P656_MoveCC_isr;
		}
	}

	/* Copy the data itself */
	BKNI_Memcpy (usCCData, vbiData, 2);

ErrorTransfer_BVBI_P_P656_MoveCC_isr:
	return retval;
}


/***************************************************************************
 *
 */
static uint32_t BVBI_P_P656_MoveWSS_isr  (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, uint16_t* usWSSData)
{
	/* Consistency checks */
	if (!isPal)
		return BVBI_LINE_ERROR_WSS_NODATA;
	if (pktInfo->lineNumber != 22)
		return BVBI_LINE_ERROR_WSS_NODATA;

	/* Copy the data itself */
	BKNI_Memcpy (usWSSData, vbiData+1, 2);
	/* TODO: verify that the first byte is sync bits */

	/* Success! */
	return 0;
}


/***************************************************************************
 *
 */
static uint32_t BVBI_P_P656_MoveTT_isr  (
	uint8_t* vbiData, BVBI_SMPTE291M_Description* pktInfo,
	bool isPal, BVBI_P_TTData* TTData)
{
	int startLine;
	int numLines;
	int lineOffset;
	int ttSize;
	uint8_t* start;

	/* Calculate region of valid teletext */
	startLine = isPal ?
		((pktInfo->polarity == BAVC_Polarity_eTopField) ?  6 : 318) :
		((pktInfo->polarity == BAVC_Polarity_eTopField) ? 10 : 273) ;
	numLines = isPal ? 18 : 11;
	ttSize = isPal ? 43 : 34;

	/* Consistency checks */
	if (pktInfo->lineNumber == 0)
	{
		lineOffset = 0;
	}
	else
	{
		lineOffset = pktInfo->lineNumber - startLine;
		if ((lineOffset < 0) || (lineOffset >= numLines))
			return BVBI_LINE_ERROR_TELETEXT_NODATA;
	}

	*(uint32_t*)(TTData->pucData) |= (uint32_t)(1 << lineOffset);
	if (TTData->ucLines < (lineOffset + 1))
		TTData->ucLines = lineOffset + 1;

	/* Copy the data itself, but check for buffer overflow */
	start = TTData->pucData + (1 + (lineOffset * ttSize));
	if ((start + ttSize) > (TTData->pucData + ttSize))
	{
		return BVBI_LINE_ERROR_FLDH_CONFLICT;
	}
	BKNI_Memcpy (start, vbiData, ttSize);

	/* Success! */
	return 0;
}


/***************************************************************************
 *
 */
static BERR_Code P_CheckE (uint8_t dataByte)
{
	bool computedParity = (P_bitParity[dataByte & 0x7F] != 0);
	bool observedParity = ((dataByte & 0x80) != 0);
	BERR_Code retval;

	if (computedParity == observedParity)
		retval =  BERR_SUCCESS ;
	else
		retval = BERR_INVALID_PARAMETER;

	return retval;
}


/***************************************************************************
 *
 */
static BERR_Code P_CheckEE (uint8_t dataByte)
{
	bool computedParity = (P_bitParity[dataByte & 0x3F] != 0);
	BERR_Code retval;

	if (computedParity)
	{
		retval = ((dataByte & 0xC0) == 0x40) ?
			BERR_SUCCESS : BERR_INVALID_PARAMETER;
	}
	else
	{
		retval = ((dataByte & 0xC0) == 0x80) ?
			BERR_SUCCESS : BERR_INVALID_PARAMETER;
	}

	return retval;
}


/***************************************************************************
 *
 */
static BERR_Code P_CheckO (uint8_t dataByte)
{
	bool computedParity = (P_bitParity[dataByte & 0x7F] != 0);
	bool observedParity = ((dataByte & 0x80) != 0);
	BERR_Code retval;

	if (computedParity != observedParity)
		retval =  BERR_SUCCESS ;
	else
		retval = BERR_INVALID_PARAMETER;

	return retval;
}


/***************************************************************************
 *
 */
static BERR_Code P_CheckEE11 (uint8_t dataByte, uint8_t* payload)
{
	bool computedParity = (P_bitParity[dataByte & 0x3F] != 0);
	BERR_Code retval;

	if (computedParity)
	{
		retval = ((dataByte & 0xC0) == 0x40) ?
			BERR_SUCCESS : BERR_INVALID_PARAMETER;
	}
	else
	{
		retval = ((dataByte & 0xC0) == 0x80) ?
			BERR_SUCCESS : BERR_INVALID_PARAMETER;
	}

	/* Check incorrect parity */
	if (retval != BERR_SUCCESS)
		return retval;

	/* Check incorrect low order bits */
	if ((dataByte & 0x03) != 0x03)
		return BERR_INVALID_PARAMETER;

	/* Return the "payload" nibble */
	*payload = ((dataByte >> 2) & 0x0F);

	/* Success! */
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static BVBI_P_SupportedVideo BVBI_P_PALorNTSC_isr (BFMT_VideoFmt eVideoFormat)
{
	BVBI_P_SupportedVideo retval;

	switch (eVideoFormat)
	{
	case BFMT_VideoFmt_eNTSC:
	case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
		retval = BVBI_P_SupportedVideo_NTSC;
		break;

	case BFMT_VideoFmt_ePAL_B:
	case BFMT_VideoFmt_ePAL_B1:
	case BFMT_VideoFmt_ePAL_D:
	case BFMT_VideoFmt_ePAL_D1:
	case BFMT_VideoFmt_ePAL_G:
	case BFMT_VideoFmt_ePAL_H:
	case BFMT_VideoFmt_ePAL_K:
	case BFMT_VideoFmt_ePAL_I:
	case BFMT_VideoFmt_ePAL_M:
	case BFMT_VideoFmt_ePAL_N:
	case BFMT_VideoFmt_ePAL_NC:
	case BFMT_VideoFmt_eSECAM_L:
	case BFMT_VideoFmt_eSECAM_B:
	case BFMT_VideoFmt_eSECAM_G:
	case BFMT_VideoFmt_eSECAM_D:
	case BFMT_VideoFmt_eSECAM_K:
	case BFMT_VideoFmt_eSECAM_H:
		retval = BVBI_P_SupportedVideo_PAL;
		break;

	default:
		retval = BVBI_P_SupportedVideo_Not;
		break;
	}

	return retval;
}


/***************************************************************************
 *
 */
static uint8_t* P_CheckControl (uint8_t* start, uint8_t* end)
{
	while ((start+2) < end)
	{
		/* Search for SAV/EAV control code */
		if ((start[0] == 0xFF) && (start[1] == 0x00) &&
		    (start[2] == 0x00))
		{
			/* SAV/EAV control code found.  Point to it's last byte
			   and exit loop immediately. */
			start += 3;
			break;
		}
		/* Search for 16-bytes of zeroes to mark end of data */
		else
		{
			int index;
			bool bZeroes = true;
			for (index = 0 ; index < 16 ; ++index)
			{
				if (start == end)
					break;
				if (start[index] != 0x00)
				{
					bZeroes = false;
					break;
				}
			}
			if (bZeroes)
			{
				/* End of data indicated.  Point to physical end of data
				   and cause outer loop to terminate. */
				start = end;
			}
			else
			{
				++start;
			}
		}
	}
	return start;
}


/***************************************************************************
 *
 */
static uint8_t* P_CheckControl_291 (uint8_t* start, uint8_t* end)
{
	while ((start+2) < end)
	{
		/* Search for SMPTE-291M ancillary data flag */
		if ((start[0] == 0x00) && (start[1] == 0xFF) &&
		    (start[2] == 0xFF))
		{
			/* Ancillary data flag found.  Point to it's last byte
			   and exit loop immediately. */
			start += 3;
			break;
		}
	}
	return start;
}

/* End of file */
