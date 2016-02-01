/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bkni.h"
#include "breg_mem.h"            /* Chip register access memory mapped */
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv.h"
#include "bbox_vdc.h"

BDBG_MODULE(BBOX);
BDBG_OBJECT_ID(BBOX_BOX);

/***************************************************************************
 *
 */
BERR_Code BBOX_Open
	( BBOX_Handle                     *phBox,
	  const BBOX_Settings             *pBoxSettings )
{
	BBOX_P_Context *pBox = NULL;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BBOX_Open);
	BDBG_ASSERT(phBox);
	BDBG_ASSERT(pBoxSettings);

	/* The handle will be NULL if create fails. */
	*phBox = NULL;

	/* (1) Alloc the main BOX context. */
	pBox = (BBOX_P_Context*)(BKNI_Malloc(sizeof(BBOX_P_Context)));
	if(NULL == pBox)
	{
		eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
		goto BBOX_Open_Done;
	}

	/* Clear out the context and set defaults. */
	BKNI_Memset((void*)pBox, 0x0, sizeof(BBOX_P_Context));
	BDBG_OBJECT_SET(pBox, BBOX_BOX);

	/* Validate box id */
	eStatus = BBOX_P_ValidateId(pBoxSettings->ulBoxId);
	if (eStatus == BERR_SUCCESS)
	{
		pBox->stBoxConfig.stBox.ulBoxId = pBoxSettings->ulBoxId;
	}
	else
	{
		goto BBOX_Open_Done;
	}

	pBox->bRtsLoaded = false;

	/* All done. now return the new fresh context to user. */
	*phBox = (BBOX_Handle)pBox;

BBOX_Open_Done:
	BDBG_LEAVE(BBOX_Open);

	if ((BERR_SUCCESS != eStatus) && (NULL != pBox))
	{
		BDBG_OBJECT_DESTROY(pBox, BBOX_BOX);
		BKNI_Free((void*)pBox);
	}

	return BERR_TRACE(eStatus);
}

/***************************************************************************
 *
 */
BERR_Code BBOX_Close
	( BBOX_Handle                     hBox )
{
	BDBG_ENTER(BBOX_Close);

	if(!hBox)
	{
		goto done;
	}

	BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

	BDBG_OBJECT_DESTROY(hBox, BBOX_BOX);
	BKNI_Free((void*)hBox);

done:
	BDBG_LEAVE(BBOX_Close);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BBOX_GetConfig
	( BBOX_Handle                      hBox,
	  BBOX_Config                     *pBoxConfig )
{
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BBOX_GetConfig);

	if (hBox == NULL) /* Implied box mode ID 0 */
	{
		pBoxConfig->stBox.ulBoxId = 0;

		/* VDC */
		eStatus = BBOX_P_Vdc_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stVdc);
		if (eStatus != BERR_SUCCESS)
		{
			goto BBOX_GetConfig_Done;
		}

      /* VCE */
      eStatus = BBOX_P_Vce_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stVce);
      if (eStatus != BERR_SUCCESS)
      {
         goto BBOX_GetConfig_Done;
      }

      /* Audio */
      eStatus = BBOX_P_Audio_SetBoxMode(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stAudio);
      if (eStatus != BERR_SUCCESS)
      {
         goto BBOX_GetConfig_Done;
      }

		/* other PI/upper layer SW capabilities here */

		/* Get memconfig settings  */
		eStatus = BBOX_P_GetMemConfig(pBoxConfig->stBox.ulBoxId, &pBoxConfig->stMemConfig);
		if (eStatus != BERR_SUCCESS)
		{
			goto BBOX_GetConfig_Done;
		}

	}
	else
	{
		BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

		/* VDC */
		eStatus = BBOX_P_Vdc_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stVdc);
		if (eStatus != BERR_SUCCESS)
		{
			goto BBOX_GetConfig_Done;
		}

      /* VCE */
      eStatus = BBOX_P_Vce_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stVce);
      if (eStatus != BERR_SUCCESS)
      {
         goto BBOX_GetConfig_Done;
      }

      /* Audio */
      eStatus = BBOX_P_Audio_SetBoxMode(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stAudio);
      if (eStatus != BERR_SUCCESS)
      {
         goto BBOX_GetConfig_Done;
      }

		/* other PI/upper layer SW capabilities here */

		eStatus = BBOX_P_GetMemConfig(hBox->stBoxConfig.stBox.ulBoxId, &hBox->stBoxConfig.stMemConfig);
		if (eStatus != BERR_SUCCESS)
		{
			goto BBOX_GetConfig_Done;
		}

		*pBoxConfig = hBox->stBoxConfig;
	}

	BDBG_MSG(("hBox[%p] : ID=%d", (void *)hBox, pBoxConfig->stBox.ulBoxId));

BBOX_GetConfig_Done:
	BDBG_LEAVE(BBOX_GetConfig);
	return eStatus;
}

/***************************************************************************
 *
 */
BERR_Code BBOX_LoadRts
	( BBOX_Handle                      hBox,
	  const BREG_Handle                hReg )
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER(BBOX_LoadRts);

	BDBG_ASSERT(hBox);
	BDBG_ASSERT(hReg);
	BDBG_OBJECT_ASSERT(hBox, BBOX_BOX);

	if (hBox->bRtsLoaded)
	{
		BDBG_ERR(("RTS is already loaded."));
		err = BBOX_RTS_ALREADY_LOADED;
		goto BBOX_LoadRts_Done;
	}
	else
	{
		err = BBOX_P_LoadRts(hReg, hBox->stBoxConfig.stBox.ulBoxId);
		if (err != BERR_SUCCESS)
		{
			BDBG_ERR(("Failed to load RTS for Box Mode %d", hBox->stBoxConfig.stBox.ulBoxId));
		}
		else
		{
			hBox->bRtsLoaded = true;
		}
	}

BBOX_LoadRts_Done:
	BDBG_LEAVE(BBOX_LoadRts);
	return err;
}

/* end of file */
