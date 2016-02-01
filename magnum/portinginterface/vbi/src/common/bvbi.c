/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bvbi.h"                /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"           /* VBI internal data structures */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BVBI);

/***************************************************************************
* Private data items
***************************************************************************/

static const BVBI_Settings s_DefaultSettings =
{
	24576,						/* in656bufferSize    */
	false                       /* tteShiftDirMsb2Lsb */
};

static const BVBI_XSER_Settings s_XSER_DefaultSettings =
{
	BVBI_TTserialDataContent_DataMagFrmRun,    /* xsSerialDataContent */
	BVBI_TTserialDataSync_EAV,                 /* ttSerialDataSync */
	27                                         /* iTTserialDataSyncDelay */
};
/* TODO: determine sensible defaults (above) from our customers. */

/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BVBI_GetDefaultSettings(  BVBI_Settings *pSettings )
{
	BDBG_ENTER(BVBI_GetDefaultSettings);

	if (!pSettings)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	*pSettings = s_DefaultSettings;

	BDBG_LEAVE(BVBI_GetDefaultSettings);

	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Open
	( BVBI_Handle		*pVbiHandle,
	  BCHP_Handle		 hChip,
	  BREG_Handle		 hRegister,
	  BMEM_Handle		 hMemory,
	  const BVBI_Settings		*pDefSettings )
{
	int type;
	BERR_Code eErr;
	BVBI_P_Handle *pVbi = NULL;
	const BVBI_Settings* settings =
		(pDefSettings ? pDefSettings : &s_DefaultSettings);

	BDBG_ENTER(BVBI_Open);

	if((!pVbiHandle) ||
	   (!hChip) ||
	   (!hRegister) ||
	   (!hMemory))
	{
		BDBG_ERR(("Invalid parameter"));
		eErr = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto BVBI_Open_Done;
	}

	/* Alloc the main VBI context. */
	pVbi = (BVBI_P_Handle*)(BKNI_Malloc(sizeof(BVBI_P_Handle)));

	if(!pVbi)
	{
		eErr = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto BVBI_Open_Done;
	}

	/* Clear out the context and set defaults. */
	BKNI_Memset((void*)pVbi, 0x0, sizeof(BVBI_P_Handle));

	/* Initialize magic number to the size of the struct */
	pVbi->ulBlackMagic = sizeof(BVBI_P_Handle);

	/* Store the hChip, hRegister, hMemory, and hRdc for later use. */
	pVbi->hChip = hChip;
	pVbi->hReg  = hRegister;
	pVbi->hMem  = hMemory;

	/* Store other settings from user */
	pVbi->in656bufferSize = settings->in656bufferSize;
	pVbi->tteShiftDirMsb2Lsb = settings->tteShiftDirMsb2Lsb;

	/* Initialize empty lists of decode and encode contexts */
	BLST_D_INIT(&pVbi->decode_contexts);
	BLST_D_INIT(&pVbi->encode_contexts);

	/* Initialize freelists for bulky data */
	BVBI_P_LCOP_INITFREELIST (&pVbi->ttFreelist);

	/* Clear out list of "in use" VEC cores */
	for (type = 0 ; type < BVBI_P_EncCoreType_eLAST ; ++type)
	{
		pVbi->vecHwCoreMask[type] = 0x0;
		pVbi->vecHwCoreMask_656[type] = 0x0;
	}

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
	BCHP_PWR_AcquireResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

	/* Bang on the hardware until it makes a nice sound */
	if ((eErr = BERR_TRACE (BVBI_P_CC_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
	if ((eErr = BERR_TRACE (BVBI_P_CGMS_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
	if ((eErr = BERR_TRACE (BVBI_P_WSS_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
	if ((eErr = BERR_TRACE (BVBI_P_VPS_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
	if ((eErr = BERR_TRACE (BVBI_P_TT_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#if (BVBI_NUM_GSE > 0)
	if ((eErr = BERR_TRACE (BVBI_P_GS_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif
#if (BVBI_NUM_AMOLE > 0)
	if ((eErr = BERR_TRACE (BVBI_P_AMOL_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif
#if (BVBI_NUM_SCTEE > 0)
	if ((eErr = BERR_TRACE (BVBI_P_SCTE_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif
	if ((eErr = BERR_TRACE (BVBI_P_VE_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#if (BVBI_P_HAS_XSER_TT)
	if ((eErr = BERR_TRACE (
		BVBI_P_ITU656_Init (pVbi->hReg, BVBI_P_GetDefaultXserSettings())))
		!= BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif
#if (BVBI_NUM_ANCI656_656 > 0)
	if ((eErr = BERR_TRACE (BVBI_P_A656_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif
#if (BVBI_NUM_IN656 > 0)
	if ((eErr = BERR_TRACE (BVBI_P_IN656_Init (pVbi))) != BERR_SUCCESS)
	{
		goto BVBI_Open_Done;
	}
#endif

	/* All done. now return the new fresh context to user. */
	*pVbiHandle = (BVBI_Handle)pVbi;

  BVBI_Open_Done:

	BDBG_LEAVE(BVBI_Open);

	if ((BERR_SUCCESS != eErr) && (NULL != pVbi)) {
		/* release power if open fails */
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
		BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
		BKNI_Free((void*)pVbi);
	}

	return eErr;
}


/***************************************************************************
 *
 */
void  BVBI_Close ( BVBI_Handle vbiHandle )
{
	BVBI_P_Handle *pVbi;
	BERR_Code eErr;

	BDBG_ENTER(BVBI_Close);
	BSTD_UNUSED (eErr);

	/* check parameters */
	BVBI_P_GET_CONTEXT(vbiHandle, pVbi);
	BDBG_ASSERT (pVbi != NULL);

	/* Refuse service if user left any decoder objects open */
	BDBG_ASSERT (NULL == BLST_D_FIRST (&pVbi->decode_contexts));

	/* Refuse service if user left any encoder objects open */
	BDBG_ASSERT (NULL == BLST_D_FIRST (&pVbi->encode_contexts));

	/* Close the individual cores.  These same functions were used in
	   BVBI_Open(). */
	eErr = BERR_TRACE (BVBI_P_CC_Init   (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	eErr = BERR_TRACE (BVBI_P_CGMS_Init (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	eErr = BERR_TRACE (BVBI_P_WSS_Init  (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	eErr = BERR_TRACE (BVBI_P_TT_Init   (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	eErr = BERR_TRACE (BVBI_P_VPS_Init  (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#if (BVBI_NUM_GSE > 0)
	eErr = BERR_TRACE (BVBI_P_GS_Init  (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_AMOLE > 0)
	eErr = BERR_TRACE (BVBI_P_AMOL_Init  (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_SCTEE > 0)
	eErr = BERR_TRACE (BVBI_P_SCTE_Init  (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
	eErr = BERR_TRACE (BVBI_P_VE_Init   (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#if (BVBI_P_HAS_XSER_TT)
	eErr =
		BERR_TRACE (
			BVBI_P_ITU656_Init (pVbi->hReg, BVBI_P_GetDefaultXserSettings()));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_ANCI656_656 > 0)
	eErr = BERR_TRACE (BVBI_P_A656_Init (pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif
#if (BVBI_NUM_IN656 > 0)
	eErr = BERR_TRACE (BVBI_P_IN656_Init(pVbi));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
#endif

	/* The handle is invalid */
	pVbi->ulBlackMagic = 0;

	/* Sanity check for bulky storage */
	BDBG_ASSERT (pVbi->ttFreelist.l_co == 0x0);

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
	BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

	/* Release context in system memory */
	BKNI_Free((void*)pVbi);

	BDBG_LEAVE(BVBI_Close);
}

static const BVBI_Capabilities s_Cap =
{
	BVBI_NUM_AMOLE,
	BVBI_NUM_AMOLE_656,
	BVBI_NUM_CCE,
	BVBI_NUM_CCE_656,
	BVBI_NUM_CGMSAE,
	BVBI_NUM_CGMSAE_656,
	BVBI_NUM_GSE,
	BVBI_NUM_GSE_656,
	BVBI_NUM_TTE,
	BVBI_NUM_TTE_656,
	BVBI_NUM_WSE,
	BVBI_NUM_WSE_656,
	BVBI_P_HAS_XSER_TT
};

/***************************************************************************
 *
 */
BERR_Code BVBI_GetCapabilities
	( BVBI_Handle                      vbiHandle,
	  BVBI_Capabilities               *pCapabilities )
{
	BVBI_P_Handle *pVbi;

	BDBG_ENTER(BVBI_GetCapabilities);

	/* check parameters */
	BVBI_P_GET_CONTEXT(vbiHandle, pVbi);
	BDBG_ASSERT (pVbi != NULL);

	if (pCapabilities)
	{
		*pCapabilities = s_Cap;
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Standby
	( BVBI_Handle hVbi,
	  BVBI_StandbySettings *pSettings )
{
	BVBI_P_Handle *pVbi;
	BSTD_UNUSED(pSettings); /* unused for now */

	BVBI_P_GET_CONTEXT(hVbi, pVbi);
	BDBG_ASSERT (pVbi != NULL);

	/* TODO: if BVBI is in use and there are clocks that need to be kept powered,
	   we should short-circuit and return an error. for now, no such check is deemed necessary */
	if (0) {
		BDBG_ERR(("Cannot enter standby due to ( ) in use"));
		return BERR_UNKNOWN;
	}

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
	BCHP_PWR_ReleaseResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_Resume (BVBI_Handle hVbi)
{
	BVBI_P_Handle *pVbi;

	BDBG_ENTER (BVBI_Resume);
	BSTD_UNUSED (pVbi);

	BVBI_P_GET_CONTEXT(hVbi, pVbi);
	BDBG_ASSERT (pVbi != NULL);

#ifdef BCHP_PWR_RESOURCE_VDC_VEC
	BCHP_PWR_AcquireResource(pVbi->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

	BDBG_LEAVE (BVBI_Resume);
	return BERR_SUCCESS;
}

/***************************************************************************
 * VBI private (top level) functions
 ***************************************************************************/

const BVBI_XSER_Settings* BVBI_P_GetDefaultXserSettings (void)
{
	return &s_XSER_DefaultSettings;
}

#if (BSTD_CPU_ENDIAN != BSTD_ENDIAN_LITTLE)
/***************************************************************************
 *
 */
uint32_t BVBI_P_LEBE_SWAP (uint32_t ulDatum)
{
	uint8_t temp;
	union {
		uint8_t b[4];
		uint32_t l;
	} scratch;

	scratch.l = ulDatum;
	P_SWAP (scratch.b[0], scratch.b[3], temp);
	P_SWAP (scratch.b[1], scratch.b[2], temp);

	return scratch.l;
}
#endif

/* End of File */
