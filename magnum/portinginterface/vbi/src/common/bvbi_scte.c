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
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"			/* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

BDBG_MODULE(BVBI);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetSCTEData_isr (
	BVBI_Field_Handle fieldHandle,
	BVBI_SCTE_Data      *pScteData
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BVBI_P_SCTE_Data* pData;
	BVBI_SCTE_CC_Data* cc_data;
	BVBI_SCTE_NRTV_Data* nrtv_data;
	BVBI_SCTE_Mono_Data* mono_data;
	BERR_Code eErr = BERR_SUCCESS;
	size_t line_count;
	size_t entry;
	size_t cc_count = 0;
	size_t nrtv_count = 0;
	size_t mono_count = 0;

	BDBG_ENTER(BVBI_Field_GetSCTEData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pScteData))
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_SCTE))
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_SCTE_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);
	pData = pVbi_Fld->pPScteData;
	if (!pData)
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);

	pScteData->field_number = pData->field_number;

	/*
	 * Line oriented data
	 */
	for (line_count = 0 ; line_count < pData->line_count ; ++line_count)
	{
		BVBI_P_SCTE_Line_Data* pLine = &pData->pLine[line_count];
		switch (pLine->eType)
		{
		case BVBI_SCTE_Type_CCONLY:
			if (cc_count >= pScteData->cc_size)
				return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
			cc_data = &pScteData->cc_data[cc_count];
			pScteData->cc_count = ++cc_count;
			cc_data->valid = pLine->valid;
			cc_data->priority = pLine->priority;
			cc_data->line_number = pLine->line_number;
			cc_data->cc_data_1 = pLine->d.CC.cc_data_1;
			cc_data->cc_data_2 = pLine->d.CC.cc_data_2;
			break;
		case BVBI_SCTE_Type_CCNRTV:
			if (nrtv_count >= pScteData->nrtv_size)
				return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
			nrtv_data = &pScteData->nrtv_data[nrtv_count];
			pScteData->nrtv_count = ++nrtv_count;
			nrtv_data->valid = pLine->valid;
			nrtv_data->priority = pLine->priority;
			nrtv_data->line_number = pLine->line_number;
			nrtv_data->sequence_number = pLine->d.NRTV.sequence_number;
			nrtv_data->segment_number = pLine->d.NRTV.segment_number;
			for (entry = 0 ; entry < 32 ; ++entry)
			{
				nrtv_data->y_data[entry]    = pData->nrtv_data[0][entry];
				nrtv_data->cbcr_data[entry] = pData->nrtv_data[1][entry];
			}
			break;
		case BVBI_SCTE_Type_CCMONO:
			if (mono_count >= pScteData->mono_size)
				return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
			mono_data = &pScteData->mono_data[mono_count];
			pScteData->mono_count = ++mono_count;
			mono_data->first_segment_flag = pLine->d.Mono.first_segment_flag;
			mono_data->last_segment_flag = pLine->d.Mono.last_segment_flag;
			mono_data->line_number = pLine->line_number;
			mono_data->first_pixel_position =
				pLine->d.Mono.first_pixel_position;
			mono_data->n_pixels = pLine->d.Mono.n_pixels;
			for (entry = 0 ; entry < mono_data->n_pixels ; ++entry)
			{
				mono_data->Y_data[entry] =
					pData->mono_data[pLine->d.Mono.iArray][entry];
			}
			break;
		default:
			return BERR_TRACE(BVBI_ERR_FIELD_BADDATA);
			break;
		}
	}

	/*
	 * PAM data
	 */
	if (pData->pam_data_count > pScteData->pam_size)
		return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
	BKNI_Memcpy (
		pScteData->luma_PAM_data, pData->pam_data, pData->pam_data_count);
	pScteData->pam_count = pData->pam_data_count;

	BDBG_LEAVE(BVBI_Field_GetSCTEData_isr);
	return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetSCTEData_Count_isr(
	BVBI_Field_Handle fieldHandle,
	size_t* pcc_count, size_t* pnrtv_count, size_t* ppam_count,
	size_t* pmono_count
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BVBI_P_SCTE_Data* pData;
	size_t iLine;
	BERR_Code eErr = BERR_SUCCESS;
	size_t cc_count = 0;
	size_t nrtv_count = 0;
	size_t mono_count = 0;

	BDBG_ENTER(BVBI_Field_GetSCTEData_Count_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if(!pVbi_Fld)
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	if ((!pcc_count) || (!pnrtv_count) || (!ppam_count) || (!pmono_count))
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_SCTE))
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_SCTE_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);
	pData = pVbi_Fld->pPScteData;
	if (!pData)
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);

	/* Search for the line oriented data */
	for (iLine = 0 ; iLine < pData->line_count ; ++iLine)
	{
		BVBI_P_SCTE_Line_Data* pLine = &(pData->pLine[iLine]);
		switch (pLine->eType)
		{
		case BVBI_SCTE_Type_CCONLY:
			++cc_count;
			break;
		case BVBI_SCTE_Type_CCNRTV:
			++nrtv_count;
			break;
		case BVBI_SCTE_Type_CCMONO:
			++mono_count;
			break;
		default:
			break;
		}
	}

	/* Return requested data */
	*pnrtv_count = nrtv_count;
	*pcc_count   = cc_count;
	*pmono_count = mono_count;
	*ppam_count  = pData->pam_data_count;

	BDBG_LEAVE(BVBI_Field_GetSCTEData_Count_isr);
	return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetSCTEData_Size_isr(
	BVBI_Field_Handle fieldHandle,
	size_t* pcc_size, size_t* pnrtv_size, size_t* ppam_size, size_t* pmono_size
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BVBI_P_SCTE_Data* pData;
	size_t cc_size;
	size_t nrtv_size;
	size_t mono_size;
	BERR_Code eErr = BERR_SUCCESS;

	BDBG_ENTER(BVBI_Field_GetSCTEData_Size_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if(!pVbi_Fld)
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	if ((!pcc_size) || (!pnrtv_size) || (!ppam_size) || (!pmono_size))
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* Verify that data is present on this field handle */
	if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_SCTE))
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);
	else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_SCTE_NOENCODE)
		eErr = (BVBI_ERR_FIELD_BADDATA);
	pData = pVbi_Fld->pPScteData;
	if (!pData)
		return BERR_TRACE(BVBI_ERR_FIELD_NODATA);

	/* Some simple calculations */
	nrtv_size = (pData->nrtv_data[0] == 0) ? 0 : 1;
	mono_size = (pData->mono_data[0] == 0) ? 0 : 2;
	cc_size = pData->line_size - nrtv_size - mono_size;

	/* Return requested data */
	*pnrtv_size = nrtv_size;
	*pcc_size   = cc_size;
	*pmono_size = mono_size;
	*ppam_size  = pData->pam_data_size;

	BDBG_LEAVE(BVBI_Field_GetSCTEData_Size_isr);
	return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetSCTEData_isr(
	BVBI_Field_Handle fieldHandle,
	BVBI_SCTE_Data      *pScteData
)
{
	BVBI_P_Field_Handle* pVbi_Fld;
	BVBI_P_SCTE_Data* pData;
	BMEM_Handle hMem;
	BERR_Code eErr;
	int entry;
	uint32_t cc_line_number;
	uint32_t nrtv_line_number;
	uint32_t mono_line_number;
	void* cached_ptr;
	size_t cc_count = 0;
	size_t nrtv_count = 0;
	size_t mono_count = 0;
	size_t line_count = 0;

	BDBG_ENTER(BVBI_Field_SetSCTEData_isr);

	/* check parameters */
	BVBI_P_GET_FIELD_CONTEXT(fieldHandle, pVbi_Fld);
	if((!pVbi_Fld) || (!pScteData))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Convenience */
	hMem = pVbi_Fld->pVbi->hMem;

	/* Verify that the field handle can hold SCTE data */
	pData = pVbi_Fld->pPScteData;
	if (!pData)
		return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);

	/* A few sanity checks */
	if (pScteData->nrtv_count > 1)
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	if (pScteData->mono_count > 2)
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	if (pScteData->pam_count > 4095)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	pData->field_number = pScteData->field_number;

	/*
	 * Line oriented data. CC, NRTV, and monochrome data must be merged
	 * according to line_number.
	 */
	while ((cc_count   < pScteData->cc_count  ) ||
		   (nrtv_count < pScteData->nrtv_count) ||
		   (mono_count < pScteData->mono_count)   )
	{
		BVBI_P_SCTE_Line_Data *pLine = &(pData->pLine[line_count]);
		if (line_count >= pData->line_size)
			return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
		if (cc_count < pScteData->cc_count)
			cc_line_number = pScteData->cc_data[cc_count].line_number;
		else
			cc_line_number = 0xFFFFFFFF;
		if (nrtv_count < pScteData->nrtv_count)
			nrtv_line_number = pScteData->nrtv_data[nrtv_count].line_number;
		else
			nrtv_line_number = 0xFFFFFFFF;
		if (mono_count < pScteData->mono_count)
			mono_line_number = pScteData->mono_data[mono_count].line_number;
		else
			mono_line_number = 0xFFFFFFFF;
		if ((cc_line_number <= nrtv_line_number) &&
		    (cc_line_number <= mono_line_number)   )
		{
			BVBI_SCTE_CC_Data* pCC = &(pScteData->cc_data[cc_count]);
			if ((pCC->line_number < 10) || (pCC->line_number > 41))
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			pLine->valid          = pCC->valid;
			pLine->priority       = pCC->priority;
			pLine->line_number    = pCC->line_number;
			pLine->eType          = BVBI_SCTE_Type_CCONLY;
			pLine->d.CC.cc_data_1 = pCC->cc_data_1;
			pLine->d.CC.cc_data_2 = pCC->cc_data_2;
			++cc_count;
		}
		else if ((nrtv_line_number <=   cc_line_number) &&
		         (nrtv_line_number <= mono_line_number)   )
		{
			BVBI_SCTE_NRTV_Data* pNRTV = &(pScteData->nrtv_data[nrtv_count]);
			if ((pNRTV->line_number < 10) || (pNRTV->line_number > 41))
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			pLine->valid                  = pNRTV->valid;
			pLine->priority               = pNRTV->priority;
			pLine->line_number            = pNRTV->line_number;
			pLine->eType                  = BVBI_SCTE_Type_CCNRTV;
			pLine->d.NRTV.sequence_number = pNRTV->sequence_number;
			pLine->d.NRTV.segment_number  = pNRTV->segment_number;
			for (entry = 0 ; entry < 32 ; ++entry)
			{
				pData->nrtv_data[0][entry] = pNRTV->y_data[entry];
				pData->nrtv_data[1][entry] = pNRTV->cbcr_data[entry];
			}
			++nrtv_count;
		}
		else /* Process monochrome data */
		{
			size_t iBulk;
			for (iBulk = 0 ; iBulk < pScteData->mono_count ; ++iBulk)
			{
				BVBI_SCTE_Mono_Data* pMono =
					&(pScteData->mono_data[mono_count]);
				if ((pMono->line_number < 10) || (pMono->line_number > 41))
					return BERR_TRACE(BERR_INVALID_PARAMETER);
				pLine->valid                  = true; /* fake */
				pLine->priority               = 1;    /* fake */
				pLine->line_number            = pMono->line_number;
				pLine->eType                  = BVBI_SCTE_Type_CCMONO;
				pLine->d.Mono.first_segment_flag   = pMono->first_segment_flag;
				pLine->d.Mono.last_segment_flag    = pMono->last_segment_flag;
				pLine->d.Mono.first_pixel_position =
					pMono->first_pixel_position;
				pLine->d.Mono.n_pixels = pMono->n_pixels;
				for (entry = 0 ; entry < pMono->n_pixels ; ++entry)
				{
					pData->mono_data[iBulk][entry] = pMono->Y_data[entry];
				}
				++mono_count;
			}
		}
		pData->line_count = ++line_count;
	}

	/*
	 * PAM data
	 */
	if (pScteData->pam_count > pData->pam_data_size)
		return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
		hMem, pData->pam_data, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		return eErr;
	}
	BKNI_Memcpy (cached_ptr,  pScteData->luma_PAM_data, pScteData->pam_count);
	BMEM_FlushCache_isr (hMem, cached_ptr, pScteData->pam_count);
	pData->pam_data_count = pScteData->pam_count;

	/* Indicate valid data is present */
	pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_SCTE;

	BDBG_LEAVE(BVBI_Field_SetSCTEData_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
* Implementation of supporting SCTE functions that are not in API
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_P_SCTE_Init( BVBI_P_Handle *pVbi )
{
#if (BVBI_NUM_SCTEE == 0)
	BSTD_UNUSED (pVbi);
#else
	uint8_t hwIndex;

	BDBG_ENTER(BVBI_P_SCTE_Init);

	/* Initialize SCTE encoders */
	for (hwIndex = 0 ; hwIndex < BVBI_NUM_SCTEE ; ++hwIndex)
		BVBI_P_SCTE_Enc_Init (pVbi->hReg, hwIndex);

	BDBG_LEAVE(BVBI_P_SCTE_Init);
#endif
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_SCTEData_Alloc (
	BMEM_Handle hMem, size_t cc_size, bool scteEnableNrtv, size_t pam_size,
	bool scteEnableMono, BVBI_P_SCTE_Data* pPScteData)
{
	int iBulk;
	size_t line_size;

	/*
	 * De-allocate existing data
	 */
	if (pPScteData->line_size != 0)
	{
		BKNI_Free (pPScteData->pLine);
		pPScteData->line_size = 0;
		pPScteData->line_count = 0;
	}
	if (pPScteData->nrtv_data[0] != 0)
	{
		for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
		{
			BKNI_Free (pPScteData->nrtv_data[iBulk]);
			pPScteData->nrtv_data[iBulk] = 0x0;
		}
	}
	if (pPScteData->pam_data_size != 0)
	{
		BMEM_Free (hMem, pPScteData->pam_data);
		pPScteData->pam_data_size = 0;
		pPScteData->pam_data_count = 0;
	}
	if (pPScteData->mono_data[0] != 0)
	{
		for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
		{
			BKNI_Free (pPScteData->mono_data[iBulk]);
			pPScteData->mono_data[iBulk] = 0x0;
		}
	}

	/* Allocations for line oriented data */
	line_size = cc_size;
	if (scteEnableNrtv)
		++line_size;
	if (scteEnableMono)
		line_size += 2;
	if (line_size != 0)
	{
		pPScteData->pLine =
			BKNI_Malloc (line_size * sizeof (BVBI_P_SCTE_Line_Data));
		if (!(pPScteData->pLine))
		{
			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		}
		pPScteData->line_size  = line_size;
		pPScteData->line_count = 0;
	}

	/*
	 * Allocations for NRTV data
	 */
	if (scteEnableNrtv)
	{
		for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
		{
			/* TODO: Change length to 32. */
			pPScteData->nrtv_data[iBulk] = BKNI_Malloc (704);
			if (!(pPScteData->nrtv_data[iBulk]))
			{
				return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			}
		}
	}

	/*
	 * Allocation for PAM data
	 */
	if (pam_size != 0)
	{
		pPScteData->pam_data = BMEM_AllocAligned (hMem, pam_size, 4, 0);
		if (!(pPScteData->pam_data))
		{
			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		}
		pPScteData->pam_data_size  = pam_size;
		pPScteData->pam_data_count = 0;
	}

	/*
	 * Allocations for monochrome data
	 */
	if (scteEnableMono)
	{
		for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
		{
			/* TODO: Change length to 256. */
			pPScteData->mono_data[iBulk] = BKNI_Malloc (720);
			if (!(pPScteData->mono_data[iBulk]))
			{
				return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			}
		}
	}

	return BERR_SUCCESS;
}

#if (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBI_P_Encode_AllocScte (
	BMEM_Handle hMem,
	BVBI_LineBuilder_Handle topScteNrtv[2],
	BVBI_LineBuilder_Handle botScteNrtv[2],
	BVBI_LineBuilder_Handle topScteMono[2],
	BVBI_LineBuilder_Handle botScteMono[2],
	uint8_t** pSctePamData
)
{
	int iBulk;
	BERR_Code eErr;

	/* Allocate empty space for SCTE data */
	for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
	{
		if ((eErr = BVBI_LineBuilder_Open (
			&topScteNrtv[iBulk], hMem, 704, 720)) !=
			BERR_SUCCESS)
		{
			return BERR_TRACE(eErr);
		}
		if ((eErr = BVBI_LineBuilder_Open (
			&botScteNrtv[iBulk], hMem, 704, 720)) !=
			BERR_SUCCESS)
		{
			BVBI_P_Encode_FreeScte (
				hMem, topScteNrtv, botScteNrtv, topScteMono,
				botScteMono, pSctePamData);
			return BERR_TRACE(eErr);
		}
		if ((eErr = BVBI_LineBuilder_Open (
			&topScteMono[iBulk], hMem, 720, 720)) !=
			BERR_SUCCESS)
		{
			BVBI_P_Encode_FreeScte (
				hMem, topScteNrtv, botScteNrtv, topScteMono,
				botScteMono, pSctePamData);
			return BERR_TRACE(eErr);
		}
		if ((eErr = BVBI_LineBuilder_Open (
			&botScteMono[iBulk], hMem, 720, 720)) !=
			BERR_SUCCESS)
		{
			BVBI_P_Encode_FreeScte (
				hMem, topScteNrtv, botScteNrtv, topScteMono,
				botScteMono, pSctePamData);
			return BERR_TRACE(eErr);
		}

	}
	*pSctePamData = BMEM_AllocAligned (hMem, 4096, 4, 0);
	if (!(*pSctePamData))
	{
		BVBI_P_Encode_FreeScte (
			hMem, topScteNrtv, botScteNrtv, topScteMono,
			botScteMono, pSctePamData);
		return BERR_TRACE(eErr);
	}
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void BVBI_P_Encode_FreeScte (
	BMEM_Handle hMem,
	BVBI_LineBuilder_Handle topScteNrtv[2],
	BVBI_LineBuilder_Handle botScteNrtv[2],
	BVBI_LineBuilder_Handle topScteMono[2],
	BVBI_LineBuilder_Handle botScteMono[2],
	uint8_t** pSctePam
)
{
	int iBulk;

	for (iBulk = 0 ; iBulk < 2 ; ++iBulk)
	{
		if (topScteNrtv[iBulk])
		{
			BVBI_LineBuilder_Close (topScteNrtv[iBulk]);
			topScteNrtv[iBulk] = 0x0;
		}
		if (botScteNrtv[iBulk])
		{
			BVBI_LineBuilder_Close (botScteNrtv[iBulk]);
			botScteNrtv[iBulk] = 0x0;
		}
		if (topScteMono[iBulk])
		{
			BVBI_LineBuilder_Close (topScteMono[iBulk]);
			topScteMono[iBulk] = 0x0;
		}
		if (botScteMono[iBulk])
		{
			BVBI_LineBuilder_Close (botScteMono[iBulk]);
			botScteMono[iBulk] = 0x0;
		}
	}

	if (*pSctePam)
	{
		BMEM_Free (hMem, *pSctePam);
		*pSctePam = 0x0;
	}
}

#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/


/***************************************************************************
* Static (private) functions
***************************************************************************/

/* End of file */
