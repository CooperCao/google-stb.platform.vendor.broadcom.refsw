/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
#include "bstd.h"
#include "bkni.h"
#include "bvdc_resource_priv.h"
#include "bvdc_priv.h"
#include "bchp_scl_0.h"
#include "bchp_vnet_b.h"
#include "bvdc_capture_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_xsrc_priv.h"
#include "bvdc_tntd_priv.h"
#include "bvdc_hscaler_priv.h"
#include "bvdc_mad_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_anr_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_boxdetect_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_hist_priv.h"
#include "bvdc_vnetcrc_priv.h"
#include "bvdc_mcvp_priv.h"

BDBG_MODULE(BVDC_RESOURCE);
BDBG_OBJECT_ID(BVDC_RES);

/*--------------------------------------------------------------------*/
#define BVDC_P_RSRC_END_ON_FAIL(result) \
if ( BERR_SUCCESS != BERR_TRACE(result)) \
{\
	goto Done;  \
}

/* macro for "the number of shared HW modules of the shared resource type",
 * "the HW ID of the first shared HW module of a shared resource type",
 * Note: assume the HW ID of the shared HW modules of the same type are
 * contiguous */
#define BVDC_P_NUM_SHARED_VFD          0
#define BVDC_P_NUM_SHARED_CAP          0
#define BVDC_P_NUM_SHARED_MAD          BVDC_P_SUPPORT_MAD
#define BVDC_P_NUM_SHARED_MCVP         BVDC_P_SUPPORT_MCVP
#define BVDC_P_NUM_SHARED_DNR          BVDC_P_SUPPORT_DNR
#define BVDC_P_NUM_SHARED_XSRC         BVDC_P_SUPPORT_XSRC
#define BVDC_P_NUM_SHARED_TNTD         BVDC_P_SUPPORT_TNTD
#define BVDC_P_NUM_SHARED_HIST         BVDC_P_SUPPORT_HIST
#define BVDC_P_NUM_SHARED_BOX          BVDC_P_SUPPORT_BOX_DETECT
#if((BCHP_CHIP==7400) || (BCHP_CHIP==7405) || (BCHP_CHIP==7420) || \
    (BCHP_CHIP==7422) || (BCHP_CHIP==7425) || (BCHP_CHIP==7435) || \
    (BCHP_CHIP==7439) || (BCHP_CHIP==7366) || \
    ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0)))
#define BVDC_P_NUM_SHARED_SCL          1
#define BVDC_P_ID0_SHARED_SCL          BVDC_P_ScalerId_eScl3
#else
#define BVDC_P_NUM_SHARED_SCL          0
#define BVDC_P_ID0_SHARED_SCL          BVDC_P_ScalerId_eUnknown
#endif
#define BVDC_P_NUM_SHARED_FCH          BVDC_P_SUPPORT_FREE_CHANNEL
#define BVDC_P_ID0_SHARED_FCH          BVDC_P_FreeChId_eCh0
#define BVDC_P_NUM_SHARED_LPBK         BVDC_P_SUPPORT_LOOP_BACK
#define BVDC_P_NUM_SHARED_VNET_CRC     BVDC_P_SUPPORT_VNET_CRC
#define BVDC_P_NUM_SHARED_DRN_F        BVDC_P_SUPPORT_DRAIN_F
#define BVDC_P_NUM_SHARED_DRN_B        BVDC_P_SUPPORT_DRAIN_B

/* Make nicer formating. */
#define BVDC_P_NUM_SHARED_NONE         0
#define BVDC_P_NUM_SHARED_PCPLL        1

/* capabilities are or-ed during acquiring.
 * if all hw module of a type has the same capability set, s_ul*Able is not
 * defined, and 0 is used for acquiring */

#if (BVDC_P_NUM_SHARED_VFD > 0)
static uint32_t s_ulVfdAbleFlags[] =
{
	/* BVDC_P_FeederId_eVfd0 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd1 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd2 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd3 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd4 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd5 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd6 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eVfd7 */   (BVDC_P_Able_eMem0),
	/* BVDC_P_FeederId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_CAP > 0)
static const uint32_t s_ulCaptureAbleFlags[] =
{
	/* BVDC_P_CaptureId_eCap0 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap1 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap2 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap3 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap4 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap5 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eCap6 */  (BVDC_P_Able_eMem0),
	/* BVDC_P_CaptureId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_SCL > 0)
static const uint32_t s_ulScalerAbleFlags[] =
{
#if (BVDC_P_SUPPORT_SCL >= 2)
	/* BVDC_P_ScalerId_eScl0 */   (BVDC_P_Able_eHd),
#else
	/* BVDC_P_ScalerId_eScl0 */   (0),
#endif
#if (BVDC_P_SUPPORT_SCL >= 3)
	/* BVDC_P_ScalerId_eScl1 */   (BVDC_P_Able_eHd),
#else
	/* BVDC_P_ScalerId_eScl1 */   (0),
#endif
	/* BVDC_P_ScalerId_eScl2 */   (0),
	/* BVDC_P_ScalerId_eScl3 */   (0),
	/* BVDC_P_ScalerId_eScl4 */   (0),
	/* BVDC_P_ScalerId_eScl5 */   (0),
	/* BVDC_P_ScalerId_eScl6 */   (0),
	/* BVDC_P_ScalerId_eScl7 */   (0),
	/* BVDC_P_ScalerId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_MAD > 0)
static const uint32_t s_ulMadAbleFlags[] =
{
	/* BVDC_P_MadId_eMad0 */      (BVDC_P_Able_eMem0),
	/* BVDC_P_MadId_eMad1 */      (BVDC_P_Able_eHd | BVDC_P_Able_eMem1),
	/* BVDC_P_MadId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_MCVP > 0)
static const uint32_t s_ulMcvpAbleFlags[] =
{
	/* BVDC_P_McvpId_eMcvp0 */    (BVDC_P_Able_eHd),
	/* BVDC_P_McvpId_eMcvp1 */    (BVDC_P_Able_eHd | BVDC_P_Able_eMadr0),
	/* BVDC_P_McvpId_eMcvp2 */    (BVDC_P_Able_eHd | BVDC_P_Able_eMadr1),
	/* BVDC_P_McvpId_eMcvp3 */    (BVDC_P_Able_eHd | BVDC_P_Able_eMadr2),
	/* BVDC_P_McvpId_eMcvp4 */    (BVDC_P_Able_eHd | BVDC_P_Able_eMadr3),
	/* BVDC_P_McvpId_eMcvp5 */    (BVDC_P_Able_eHd | BVDC_P_Able_eMadr4),
	/* BVDC_P_McvpId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_DNR > 0)
static const uint32_t s_ulDnrAbleFlags[] =
{
	/* BVDC_P_DnrId_eDnr0 */      (BVDC_P_Able_e10bits | BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eDnr1 */      (BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eDnr2 */      (BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eDnr3 */      (BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eDnr4 */      (BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eDnr5 */      (BVDC_P_Able_e8bits),
	/* BVDC_P_DnrId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_XSRC > 0)
static const uint32_t s_ulXsrcAbleFlags[] =
{
	/* BVDC_P_XsrcId_eXsrc0 */    (0),
	/* BVDC_P_XsrcId_eXsrc1 */    (0),
	/* BVDC_P_XsrcId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_TNTD > 0)
static const uint32_t s_ulTntdAbleFlags[] =
{
	/* BVDC_P_TntdId_eTntd0 */    (0),
	/* BVDC_P_TntdId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_HIST > 0)
static const uint32_t s_ulHistAbleFlags[] =
{
	/* BVDC_P_HistId_eHist0 */    (0),
	/* BVDC_P_HistId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_VNET_CRC > 0)
static const uint32_t s_ulVnetCrcAbleFlags[] =
{
	/* BVDC_P_VnetCrcId_eVnetCrc0 */    (0),
	/* BVDC_P_VnetCrcId_eUnknown */
};
#endif

#if (BVDC_P_NUM_SHARED_DVI > 0)
static const uint32_t s_ulDviAbleFlags[] =
{
	(BVDC_P_Able_eHdmi0),
	(BVDC_P_Able_eHdmi1),
};
#endif

static const uint32_t s_ulDrnFrnAbleFlags[] =
{
	/* BVDC_P_DrainFrnId_eDrain0  */ (BVDC_P_Able_eAllSrc),
	/* BVDC_P_DrainFrnId_eDrain1  */ (0),
	/* BVDC_P_DrainFrnId_eDrain2  */ (0),
	/* BVDC_P_DrainFrnId_eDrain3  */ (BVDC_P_Able_eAllSrc),
	/* BVDC_P_DrainFrnId_eDrain4  */ (BVDC_P_Able_eAllSrc),
	/* BVDC_P_DrainFrnId_eDrain5  */ (BVDC_P_Able_eAllSrc),
	/* BVDC_P_DrainFrnId_eDrain6  */ (BVDC_P_Able_eAllSrc),
	/* BVDC_P_DrainFrnId_eUnknown */ (BVDC_P_Able_eAllSrc)
};

/* Short hande to make an entry. */
#define BVDC_P_MAKE_RES(resource, count, fisrt_entry)                \
{                                                                    \
	(BVDC_P_NUM_##count),                                            \
	(fisrt_entry),                                                   \
	(#resource),                                                     \
	(BVDC_P_ResourceType_##resource)                                 \
}

typedef struct
{
	uint32_t                           ulCount;
	uint32_t                           ulFirstId;
	const char                        *pchName;
	BVDC_P_ResourceType                eResource;

} BVDC_P_ResourceInfoEntry;

/* Table contains number of available resource and 1st id available */
static const BVDC_P_ResourceInfoEntry s_aResInfoTbl[] =
{
	/* shared resources represented by handle */
	BVDC_P_MAKE_RES(eVfd,        SHARED_VFD,   BVDC_P_FeederId_eVfd0         ),
	BVDC_P_MAKE_RES(eCap,        SHARED_CAP,   BVDC_P_CaptureId_eCap0        ),
	BVDC_P_MAKE_RES(eMad,        SHARED_MAD,   BVDC_P_MadId_eMad0            ),
	BVDC_P_MAKE_RES(eXsrc,       SHARED_XSRC,  BVDC_P_XsrcId_eXsrc0          ),
	BVDC_P_MAKE_RES(eTntd,       SHARED_TNTD,  BVDC_P_TntdId_eTntd0          ),
	BVDC_P_MAKE_RES(eMcvp,       SHARED_MCVP,  BVDC_P_McvpId_eMcvp0          ),
	BVDC_P_MAKE_RES(eDnr,        SHARED_DNR,   BVDC_P_DnrId_eDnr0            ),
	BVDC_P_MAKE_RES(eBoxDetect,  SHARED_BOX,   BVDC_P_BoxDetectId_eBoxDetect0),
	BVDC_P_MAKE_RES(eScl,        SHARED_SCL,   BVDC_P_ID0_SHARED_SCL         ),
	BVDC_P_MAKE_RES(eHist,       SHARED_HIST,  BVDC_P_HistId_eHist0          ),
	BVDC_P_MAKE_RES(eVnetCrc,    SHARED_VNET_CRC, BVDC_P_VnetCrcId_eVnetCrc0 ),

	/* separator between handle resources and HwId resources */
	BVDC_P_MAKE_RES(eHandleCntr, SHARED_NONE,  0                             ),

	/* shared resources idendified by Hw Id */
	BVDC_P_MAKE_RES(eFreeCh,     SHARED_FCH,   BVDC_P_ID0_SHARED_FCH         ),
	BVDC_P_MAKE_RES(eLpBck,      SHARED_LPBK,  BVDC_P_LpBckId_eLp0           ),
	BVDC_P_MAKE_RES(eDrainFrn,   SHARED_DRN_F, BVDC_P_DrainFrnId_eDrain0     ),
	BVDC_P_MAKE_RES(eDrainBck,   SHARED_DRN_B, BVDC_P_DrainBckId_eDrain0     ),

#if BVDC_P_ORTHOGONAL_VEC
	BVDC_P_MAKE_RES(e656,        SHARED_656,       0                         ),
	BVDC_P_MAKE_RES(eDvi,        SHARED_DVI,       0                         ),
	BVDC_P_MAKE_RES(eStg,        SHARED_STG,       0                         ),
	BVDC_P_MAKE_RES(eRf,         SHARED_RF,        0                         ),
	BVDC_P_MAKE_RES(eIt,         SHARED_IT,        0                         ),
	BVDC_P_MAKE_RES(eVf,         SHARED_VF,        0                         ),
	BVDC_P_MAKE_RES(eSecam,      SHARED_SECAM,     0                         ),
	BVDC_P_MAKE_RES(eSecam_HD,   SHARED_SECAM_HD,  0                         ),
	BVDC_P_MAKE_RES(eSdsrc,      SHARED_SDSRC,     0                         ),
	BVDC_P_MAKE_RES(eHdsrc,      SHARED_HDSRC,     0                         ),
	BVDC_P_MAKE_RES(eDac,        SHARED_DAC,       0                         ),
	BVDC_P_MAKE_RES(eMpaa,       SHARED_MPAA,      0                         ),
#endif

	BVDC_P_MAKE_RES(ePcPll,      SHARED_PCPLL, BVDC_P_PcPllId_ePll0          ),
	/* Must be last */
	BVDC_P_MAKE_RES(eInvalid,    SHARED_NONE,      0                         )
};

/***************************************************************************
 * {private}
 *
 */
BERR_Code  BVDC_P_Resource_Create
	( BVDC_P_Resource_Handle           *phResource,
	  BVDC_Handle                       hVdc )
{
	BVDC_P_ResourceContext  *pResource;
	BVDC_P_ResourceEntry *pResourceRecords, *pEntry;
	uint32_t  *pulIndex1stEntry;
	BVDC_P_ResourceType  eType;
	uint32_t   ii, ulNumEntries;
	BERR_Code eResult;

	BDBG_ENTER(BVDC_P_Resource_Create);
	BDBG_ASSERT(phResource);
	*phResource = NULL;

	BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

	/* Alloc the context and basic init. */
	pResource = (BVDC_P_ResourceContext*)
		(BKNI_Malloc(sizeof(BVDC_P_ResourceContext)));
	if(!pResource)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	BKNI_Memset((void*)pResource, 0x0, sizeof(BVDC_P_ResourceContext));
	BDBG_OBJECT_SET(pResource, BVDC_RES);

	/* alloc and init the array for the index of 1st entry of each type */
	eResult = BERR_OUT_OF_SYSTEM_MEMORY;
	pulIndex1stEntry = (uint32_t *)
		(BKNI_Malloc(sizeof(uint32_t)*BVDC_P_ResourceType_eInvalid));
	if(NULL == pulIndex1stEntry)
	{
		goto Done;
	}
	pResource->pulIndex1stEntry = pulIndex1stEntry;

	ulNumEntries = 0;
	for (eType = 0; eType < BVDC_P_ResourceType_eInvalid; eType++)
	{
		*(pulIndex1stEntry+eType) = ulNumEntries;
		ulNumEntries += s_aResInfoTbl[eType].ulCount;
	}

	/* alloc the array for acquiring record of each type */
	pResourceRecords = (BVDC_P_ResourceEntry *)
		(BKNI_Malloc(sizeof(BVDC_P_ResourceEntry) * ulNumEntries));
	if(NULL == pResourceRecords)
	{
		goto Done;
	}
	BKNI_Memset((void*)pResourceRecords, 0x0,
		sizeof(BVDC_P_ResourceEntry) * ulNumEntries);
	pResource->pResourceRecords = pResourceRecords;

	eResult = BERR_SUCCESS;

	/* init acquiring record for each shared resource, including create
	 * handle for each shared resource represented by a handle */
	pEntry = pResourceRecords;
	for (eType = 0; eType < BVDC_P_ResourceType_eInvalid; eType++)
	{
		BDBG_ASSERT(s_aResInfoTbl[eType].eResource == eType);

		for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
		{
			/* pEntry->ulAcquireCntr = 0; */
			/* pEntry->ulCapabilities = 0; */
			pEntry->eType = eType;
			pEntry->ulAcquireId = BVDC_P_ACQUIRE_ID_INVALID;
			pEntry->bAcquireLock = false;

			switch (eType)
			{
#ifndef BVDC_FOR_BOOTUPDATER
#if (BVDC_P_NUM_SHARED_VFD > 0)
			case BVDC_P_ResourceType_eVfd:
				eResult = BVDC_P_Feeder_Create(
					(BVDC_P_Feeder_Handle *) (void *)&(pEntry->Id.pvHandle), hVdc->hRdc, hVdc->hRegister,
					(BVDC_P_FeederId) (s_aResInfoTbl[eType].ulFirstId + ii), NULL,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
					hVdc->hTimer,
#endif
					NULL, pResource, false);
				pEntry->ulCapabilities = s_ulVfdAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_CAP > 0)
			case BVDC_P_ResourceType_eCap:
				eResult = BVDC_P_Capture_Create(
					(BVDC_P_Capture_Handle *) (void *)&(pEntry->Id.pvHandle), hVdc->hRdc, hVdc->hRegister,
					(BVDC_P_CaptureId) (s_aResInfoTbl[eType].ulFirstId + ii),
#if (!BVDC_P_USE_RDC_TIMESTAMP)
					hVdc->hTimer,
#endif
					pResource);
				pEntry->ulCapabilities = s_ulCaptureAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_MAD > 0)
			case BVDC_P_ResourceType_eMad:
				eResult = BVDC_P_Mad_Create(
					(BVDC_P_Mad_Handle *) (void *)&(pEntry->Id.pvHandle), hVdc->hRegister,
					(BVDC_P_MadId) (s_aResInfoTbl[eType].ulFirstId + ii), pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulMadAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_XSRC > 0)
			case BVDC_P_ResourceType_eXsrc:
				eResult = BVDC_P_Xsrc_Create(
					(BVDC_P_Xsrc_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_XsrcId) (s_aResInfoTbl[eType].ulFirstId + ii),
					pResource, hVdc->hRegister);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulXsrcAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_TNTD > 0)
			case BVDC_P_ResourceType_eTntd:
				eResult = BVDC_P_Tntd_Create(
					(BVDC_P_Tntd_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_TntdId) (s_aResInfoTbl[eType].ulFirstId + ii),
					pResource, hVdc->hRegister);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulTntdAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_MCVP > 0)
			case BVDC_P_ResourceType_eMcvp:
				eResult = BVDC_P_Mcvp_Create(
					(BVDC_P_Mcvp_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_McvpId) (s_aResInfoTbl[eType].ulFirstId + ii),
					hVdc->hRegister, pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulMcvpAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_DNR > 0)
			case BVDC_P_ResourceType_eDnr:
				eResult = BVDC_P_Dnr_Create(
					(BVDC_P_Dnr_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_DnrId) (s_aResInfoTbl[eType].ulFirstId + ii),
					hVdc->hRegister, pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulDnrAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_BOX > 0)
			case BVDC_P_ResourceType_eBoxDetect:
				eResult = BVDC_P_BoxDetect_Create(
					(BVDC_P_BoxDetect_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_BoxDetectId) (s_aResInfoTbl[eType].ulFirstId + ii),
					hVdc->hRegister, pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				break;
#endif

#if (BVDC_P_NUM_SHARED_SCL > 0)
			case BVDC_P_ResourceType_eScl:
				eResult = BVDC_P_Scaler_Create(
					(BVDC_P_Scaler_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_ScalerId) (s_aResInfoTbl[eType].ulFirstId + ii),
					pResource, hVdc->hRegister);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulScalerAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_HIST > 0)
			case BVDC_P_ResourceType_eHist:
				eResult = BVDC_P_Hist_Create(
					(BVDC_P_Hist_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_HistId) (s_aResInfoTbl[eType].ulFirstId + ii),
					hVdc->hRegister, pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulHistAbleFlags[ii];
				break;
#endif

#if (BVDC_P_NUM_SHARED_VNET_CRC > 0)
			case BVDC_P_ResourceType_eVnetCrc:
				eResult = BVDC_P_VnetCrc_Create(
					(BVDC_P_VnetCrc_Handle *) (void *)&(pEntry->Id.pvHandle),
					(BVDC_P_VnetCrcId) (s_aResInfoTbl[eType].ulFirstId + ii),
					hVdc->hRegister, pResource);
				BVDC_P_RSRC_END_ON_FAIL(eResult);
				pEntry->ulCapabilities = s_ulVnetCrcAbleFlags[ii];
				break;
#endif
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

			/* Front drain have different capabilities (or limiations).
			 * HD_DVI_1 can not use VNET_F_DRAIN_1_SRC and VNET_F_DRAIN_2_SRC */
			case BVDC_P_ResourceType_eDrainFrn:
				pEntry->Id.ulHwId = s_aResInfoTbl[eType].ulFirstId + ii;
				pEntry->ulCapabilities = s_ulDrnFrnAbleFlags[ii];
				break;

			case BVDC_P_ResourceType_eFreeCh:
			case BVDC_P_ResourceType_eLpBck:
			case BVDC_P_ResourceType_eDrainBck:
			case BVDC_P_ResourceType_ePcPll:
#if BVDC_P_ORTHOGONAL_VEC
			case BVDC_P_ResourceType_e656:
			case BVDC_P_ResourceType_eStg:
			case BVDC_P_ResourceType_eRf:
			case BVDC_P_ResourceType_eIt:
			case BVDC_P_ResourceType_eVf:
			case BVDC_P_ResourceType_eSecam:
			case BVDC_P_ResourceType_eSecam_HD:
			case BVDC_P_ResourceType_eSdsrc:
			case BVDC_P_ResourceType_eHdsrc:
			case BVDC_P_ResourceType_eDac:
			case BVDC_P_ResourceType_eMpaa:
#endif
				pEntry->Id.ulHwId = s_aResInfoTbl[eType].ulFirstId + ii;
				break;

#if BVDC_P_ORTHOGONAL_VEC
			case BVDC_P_ResourceType_eDvi:
				pEntry->Id.ulHwId = s_aResInfoTbl[eType].ulFirstId + ii;
				pEntry->ulCapabilities = s_ulDviAbleFlags[ii];
				break;
#endif

			default:
#ifndef BVDC_FOR_BOOTUPDATER
				BDBG_ASSERT(BVDC_P_ResourceType_eHandleCntr == eType);
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
				break;
			}

			/*BDBG_ERR(("create resource handle 0x%x: type %d entry 0x%x", pEntry->Id.pvHandle, pEntry->eType, pEntry));*/
			pEntry++;
		}
	}

	*phResource = (BVDC_P_Resource_Handle)pResource;

  Done:
#ifndef BVDC_FOR_BOOTUPDATER
	if (BERR_SUCCESS != eResult)
	{
		BVDC_P_Resource_Destroy((BVDC_P_Resource_Handle)pResource);
	}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

	BDBG_LEAVE(BVDC_P_Resource_Create);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 */
#ifndef BVDC_FOR_BOOTUPDATER
void  BVDC_P_Resource_Destroy
	( BVDC_P_Resource_Handle            hResource )
{
	BVDC_P_ResourceEntry *pEntry;
	BVDC_P_ResourceType  eType;
	uint32_t   ii;

	BDBG_ENTER(BVDC_P_Resource_Destroy);
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	pEntry = hResource->pResourceRecords;
	if (NULL != pEntry)
	{
		/* destroy all shared resource's handle */
		for (eType = 0; eType < BVDC_P_ResourceType_eHandleCntr; eType++)
		{
			for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
			{
				/*BDBG_ERR(("destroy resource handle 0x%x: type %d entry 0x%x", pEntry->Id.pvHandle, pEntry->eType, pEntry));*/

				if (NULL == pEntry->Id.pvHandle)
				{
					pEntry++;
					break;
				}

				BDBG_ASSERT(pEntry->eType == eType);
				switch (eType)
				{
#if (BVDC_P_NUM_SHARED_VFD > 0)
				case BVDC_P_ResourceType_eVfd:
					BVDC_P_Feeder_Destroy(
						(BVDC_P_Feeder_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_CAP > 0)
				case BVDC_P_ResourceType_eCap:
					BVDC_P_Capture_Destroy(
						(BVDC_P_Capture_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_MAD > 0)
				case BVDC_P_ResourceType_eMad:
					BVDC_P_Mad_Destroy(
						(BVDC_P_Mad_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_XSRC > 0)
				case BVDC_P_ResourceType_eXsrc:
					BVDC_P_Xsrc_Destroy(
						(BVDC_P_XsrcContext *) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_TNTD > 0)
				case BVDC_P_ResourceType_eTntd:
					BVDC_P_Tntd_Destroy(
						(BVDC_P_TntdContext *) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_MCVP > 0)
				case BVDC_P_ResourceType_eMcvp:
					BVDC_P_Mcvp_Destroy(
						(BVDC_P_McvpContext *) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_DNR > 0)
				case BVDC_P_ResourceType_eDnr:
					BVDC_P_Dnr_Destroy(
						(BVDC_P_Dnr_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_BOX > 0)
				case BVDC_P_ResourceType_eBoxDetect:
					BVDC_P_BoxDetect_Destroy(
						(BVDC_P_BoxDetect_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_SCL > 0)
				case BVDC_P_ResourceType_eScl:
					BVDC_P_Scaler_Destroy(
						(BVDC_P_Scaler_Handle) (pEntry->Id.pvHandle));
					break;
#endif

#if (BVDC_P_NUM_SHARED_HIST > 0)
			case BVDC_P_ResourceType_eHist:
				BVDC_P_Hist_Destroy(
					(BVDC_P_Hist_Handle)(pEntry->Id.pvHandle));
				break;
#endif

#if (BVDC_P_NUM_SHARED_VNET_CRC > 0)
			case BVDC_P_ResourceType_eVnetCrc:
				BVDC_P_VnetCrc_Destroy(
					(BVDC_P_VnetCrc_Handle)(pEntry->Id.pvHandle));
				break;
#endif

				default:
					BDBG_ASSERT(false);
				}

				pEntry++;
			}
		}

		BKNI_Free(hResource->pResourceRecords);
	}

	if (NULL != hResource->pulIndex1stEntry)
	{
		BKNI_Free(hResource->pulIndex1stEntry);
	}

	BDBG_OBJECT_DESTROY(hResource, BVDC_RES);
	BKNI_Free(hResource);

	BDBG_LEAVE(BVDC_P_Resource_Destroy);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/* if the resource type has already be acquired for an acquire ID successfully,
 * we give it the same resource module, and increase the acquiring cntr;
 * otherwise, we try to find a perfect match for capabilities; if we can not
 * find a perfect match, the first module that satisfies the capability
 * requirement will be used. */
static BERR_Code  BVDC_P_Resource_Acquire_isr
	( BVDC_P_ResourceEntry             *p1stEntry,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulCapabilities,
	  uint32_t                          ulAcquireId,
	  void                            **ppvResourceHandle,
	  uint32_t                         *pulHwId,
	  bool								bQuery)
{
	BVDC_P_ResourceEntry  *pEntry, *pCandidate;
	uint32_t  ii;

	BDBG_ASSERT((NULL != ppvResourceHandle) || (NULL != pulHwId));

	/* if this type has already be acquired for this Id, give the same module
	 * to it, and increase the ulAcquireId */
	pEntry = p1stEntry;
	for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
	{
		BDBG_ASSERT(pEntry->eType == eType);
		if ((ulAcquireId == pEntry->ulAcquireId) &&
			(false == pEntry->bAcquireLock))
		{
			BDBG_ASSERT(ulCapabilities ==
						(ulCapabilities & pEntry->ulCapabilities));
			BDBG_ASSERT(pEntry->ulAcquireCntr > 0);
			if (bQuery)
			{
				if (NULL != ppvResourceHandle)
					*ppvResourceHandle = (void *)BVDC_P_RESOURCE_ID_AVAIL;
				else
					*pulHwId = BVDC_P_RESOURCE_ID_AVAIL;

			}
			else
			{
				pEntry->ulAcquireCntr++;
				if (NULL != ppvResourceHandle)
					*ppvResourceHandle = pEntry->Id.pvHandle;
				else
					*pulHwId = pEntry->Id.ulHwId;
			}
			return BERR_SUCCESS;
		}
		pEntry++;
	}

	/* if this type has already be acquired for this Id, give the same module
	 * to it, and increase the ulAcquireId */
	pEntry = p1stEntry;
	pCandidate = NULL;
	for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
	{
		/* already used by some one? */
		if (BVDC_P_ACQUIRE_ID_INVALID != pEntry->ulAcquireId)
		{
			BDBG_ASSERT(pEntry->ulAcquireCntr > 0);
			pEntry++;
			continue;
		}
		BDBG_ASSERT((0 == pEntry->ulAcquireCntr) &&
					(false == pEntry->bAcquireLock));

		/* all required capabilities are satisfied? */
		if (ulCapabilities == (ulCapabilities & pEntry->ulCapabilities))
		{
			if (ulCapabilities == pEntry->ulCapabilities)
			{
				if (bQuery)
				{
					if (NULL != ppvResourceHandle)
						*ppvResourceHandle = (void *)BVDC_P_RESOURCE_ID_AVAIL;
					else
						*pulHwId = BVDC_P_RESOURCE_ID_AVAIL;
				}
				else
				{
					/* perfect match, use this module */
					pEntry->ulAcquireId = ulAcquireId;
					pEntry->ulAcquireCntr = 1;
					if (NULL != ppvResourceHandle)
						*ppvResourceHandle = pEntry->Id.pvHandle;
					else
						*pulHwId = pEntry->Id.ulHwId;
				}
				return BERR_SUCCESS;
			}
			else if (NULL == pCandidate)
			{
				/* the 1st found one will be used */
				pCandidate = pEntry;
			}
		}
		pEntry++;
	}

	if (NULL != pCandidate)
	{
		if (bQuery)
		{
			if (NULL != ppvResourceHandle)
				*ppvResourceHandle = (void *)BVDC_P_RESOURCE_ID_AVAIL;
			else
				*pulHwId = BVDC_P_RESOURCE_ID_AVAIL;
		}
		else
		{
			/* not perfect match, but the requirement is satisfied, use it */
			pCandidate->ulAcquireId = ulAcquireId;
			pCandidate->ulAcquireCntr = 1;
			if (NULL != ppvResourceHandle)
				*ppvResourceHandle = pCandidate->Id.pvHandle;
			else
				*pulHwId = pCandidate->Id.ulHwId;
		}
		return BERR_SUCCESS;
	}
	else
	{
		if (NULL != ppvResourceHandle)
			*ppvResourceHandle = NULL;
		else
			*pulHwId = BVDC_P_HW_ID_INVALID;
		return bQuery ? BVDC_ERR_RESOURCE_NOT_AVAILABLE : BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
	}
}

/***************************************************************************
 *
 */
BERR_Code  BVDC_P_Resource_Reserve_isr
	( BVDC_P_ResourceEntry             *p1stEntry,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulResourceIndex,
	  uint32_t                          ulAcquireId,
	  void                            **ppvResourceHandle,
	  uint32_t                         *pulHwId )
{
	uint32_t  i = 0;
	BVDC_P_ResourceEntry  *pEntry = p1stEntry;

#if !BDBG_DEBUG_BUILD
	BSTD_UNUSED(eType);
#endif

	BDBG_ASSERT((NULL != ppvResourceHandle) || (NULL != pulHwId));
	BDBG_ASSERT(ulResourceIndex < s_aResInfoTbl[eType].ulCount);

	/* Find the entry */
	while(i++ < ulResourceIndex)
		pEntry++;

	BDBG_ASSERT(pEntry->eType == eType);

	if(pEntry->ulAcquireCntr > 0)
	{
		BDBG_ERR(("Resource is already acquired by others"));
		return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
	}

	pEntry->ulAcquireId = ulAcquireId;
	pEntry->ulAcquireCntr = 1;
	if (NULL != ppvResourceHandle)
		*ppvResourceHandle = pEntry->Id.pvHandle;
	else
		*pulHwId = pEntry->Id.ulHwId;

	return BERR_SUCCESS;

}

/***************************************************************************
 * {private}
 *
 * Module assignment rule:
 *    if the resource type has already be acquired for the acquire ID
 * successfully, assign it the same resource module, and increase the
 * acquiring cntr; otherwise, we try to find a perfect match for capabilities;
 * if we can not find a perfect match, the first module that satisfies the
 * capability requirement will be used.
 */
BERR_Code  BVDC_P_Resource_AcquireHandle_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulCapabilities,
	  uint32_t                          ulAcquireId,
	  void                            **ppvResourceHandle,
	  bool								bQuery)
{
	BERR_Code eResult;

	BDBG_ENTER(BVDC_P_Resource_AcquireHandle_isr);

	BDBG_ASSERT(eType < BVDC_P_ResourceType_eHandleCntr);
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	eResult = BVDC_P_Resource_Acquire_isr(
		hResource->pResourceRecords + hResource->pulIndex1stEntry[eType],
		eType, ulCapabilities, ulAcquireId, ppvResourceHandle, NULL, bQuery);

	if (NULL == *ppvResourceHandle)
		BDBG_WRN(("Resource handle [type %d, cap %d] not available.", (int)eType, ulCapabilities));

	BDBG_LEAVE(BVDC_P_Resource_AcquireHandle_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * Module assignment rule:
 *    if the resource type has already be acquired for the acquire ID
 * successfully, assign it the same resource module, and increase the
 * acquiring cntr; otherwise, we try to find a perfect match for capabilities;
 * if we can not find a perfect match, the first module that satisfies the
 * capability requirement will be used.
 */
BERR_Code  BVDC_P_Resource_AcquireHwId_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulCapabilities,
	  uint32_t                          ulAcquireId,
	  uint32_t                         *pulHwId,
	  bool                              bQuery)
{
	BERR_Code eResult;

	BDBG_ENTER(BVDC_P_Resource_AcquireHwId_isr);

	BDBG_ASSERT(eType > BVDC_P_ResourceType_eHandleCntr);
	BDBG_ASSERT(eType < BVDC_P_ResourceType_eInvalid);
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	eResult = BVDC_P_Resource_Acquire_isr(
		hResource->pResourceRecords + hResource->pulIndex1stEntry[eType],
		eType, ulCapabilities, ulAcquireId, NULL, pulHwId, bQuery);

	BDBG_LEAVE(BVDC_P_Resource_AcquireHwId_isr);
	return bQuery ? eResult : BERR_TRACE(eResult);
}

/* when an acquiring record entry with the handle is found, its ulAcquireCntr
 * is decreased by 1. When ulAcquireCntr reaches 0, the module is really
 * released to be acquired with new acquire ID.
 */
static BERR_Code  BVDC_P_Resource_Release_isr
	( BVDC_P_ResourceEntry             *p1stEntry,
	  BVDC_P_ResourceType               eType,
	  void                             *pvResourceHandle,
	  uint32_t                          ulHwId )
{
	BVDC_P_ResourceEntry  *pEntry;
	uint32_t  ii;
	bool  bFound;

	BDBG_ASSERT((NULL != pvResourceHandle) || (BVDC_P_HW_ID_INVALID != ulHwId));

	/* if this type has already be acquired for this Id, give the same module
	 * to it, and increase the ulAcquireCntr */
	pEntry = p1stEntry;
	for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
	{
		BDBG_ASSERT(eType == pEntry->eType);

		bFound = (eType < BVDC_P_ResourceType_eHandleCntr)?
			(pvResourceHandle == pEntry->Id.pvHandle):
			(ulHwId == pEntry->Id.ulHwId);
		if (bFound)
		{
			BDBG_ASSERT(BVDC_P_ACQUIRE_ID_INVALID != pEntry->ulAcquireId);
			BDBG_ASSERT(pEntry->ulAcquireCntr > 0);
			if (pEntry->ulAcquireCntr > 0)
			{
				pEntry->ulAcquireCntr--;
			}
			else
			{
				return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_ACQUIRED);
			}

			/* really free it when ulAcquireCntr reaches 0 */
			if (0 == pEntry->ulAcquireCntr)
			{
				pEntry->bAcquireLock = false;
				pEntry->ulAcquireId = BVDC_P_ACQUIRE_ID_INVALID;
			}
			return BERR_SUCCESS;
		}

		pEntry++;
	}

	/* if we come here, we did not find a match */
	if (pvResourceHandle)
		BDBG_ERR(("Release eType %d, Handle %p", eType, (void *)pvResourceHandle));
	else
		BDBG_ERR(("Release eType %d, ulHwId %d", eType, ulHwId));
	/*BDBG_ERR(("Lets cause a core dump %d to see who cause this", 1 / 0));*/
	return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_RECORDED);
}

/***************************************************************************
 * {private}
 *
 * This func is used to release a resource that is represented by a handle.
 *
 * Module realease rule:
 *    when an acquiring record entry with the handle is found, its ulAcquireCntr
 * is decreased by 1. When ulAcquireCntr reaches 0, the module is really
 * released to be acquired with new acquire ID.
 */
BERR_Code  BVDC_P_Resource_ReleaseHandle_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  void                             *pvResourceHandle )
{
	BERR_Code eResult;

	BDBG_ENTER(BVDC_P_Resource_ReleaseHandle_isr);

	BDBG_ASSERT(eType < BVDC_P_ResourceType_eHandleCntr);
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	eResult = BVDC_P_Resource_Release_isr(
		hResource->pResourceRecords + hResource->pulIndex1stEntry[eType],
		eType, pvResourceHandle, BVDC_P_HW_ID_INVALID);

	BDBG_LEAVE(BVDC_P_Resource_ReleaseHandle_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * This func is used to release a resource that is NOT represented by a handle.
 *
 * Module realease rule:
 *    when an acquiring record entry with the handle is found, its ulAcquireCntr
 * is decreased by 1. When ulAcquireCntr reaches 0, the module is really
 * released to be acquired with new acquire ID.
 */
BERR_Code  BVDC_P_Resource_ReleaseHwId_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulHwId )
{
	BERR_Code eResult;

	BDBG_ENTER(BVDC_P_Resource_ReleaseHwId_isr);

	BDBG_ASSERT(eType > BVDC_P_ResourceType_eHandleCntr);
	BDBG_ASSERT(eType < BVDC_P_ResourceType_eInvalid);
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	eResult = BVDC_P_Resource_Release_isr(
		hResource->pResourceRecords + hResource->pulIndex1stEntry[eType],
		eType, NULL, ulHwId);

	BDBG_LEAVE(BVDC_P_Resource_ReleaseHwId_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 */
uint32_t  BVDC_P_Resource_GetHwIdAcquireCntr_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulHwId )
{
	BVDC_P_ResourceEntry  *pEntry;
	uint32_t  ii;

	BDBG_ENTER(BVDC_P_Resource_GetHwIdAcquireCntr_isr);

	if(eType <= BVDC_P_ResourceType_eHandleCntr)
	{
		BDBG_ASSERT(eType > BVDC_P_ResourceType_eHandleCntr);
		return 0;
	}
	if(eType >= BVDC_P_ResourceType_eInvalid)
	{
		BDBG_ASSERT(eType < BVDC_P_ResourceType_eInvalid);
		return 0;
	}
	BDBG_OBJECT_ASSERT(hResource, BVDC_RES);

	pEntry = hResource->pResourceRecords + hResource->pulIndex1stEntry[eType];
	for (ii = 0; ii < s_aResInfoTbl[eType].ulCount; ii++)
	{
		BDBG_ASSERT(eType == pEntry->eType);

		if (ulHwId == pEntry->Id.ulHwId)
		{
			BDBG_LEAVE(BVDC_P_Resource_GetHwIdAcquireCntr_isr);
			return pEntry->ulAcquireCntr;
		}
		pEntry++;
	}

	BDBG_LEAVE(BVDC_P_Resource_GetHwIdAcquireCntr_isr);
	return 0;
}

void BVDC_P_Resource_GetResourceId
	( BVDC_P_ResourceType               eType,
	  uint32_t                          ulCapabilities,
	  uint32_t                         *pulResourceId )
{
	uint32_t i, ulResourceCount;
	const uint32_t *pulCapabilitiesTable = NULL;

	ulResourceCount = s_aResInfoTbl[eType].ulCount;
	*pulResourceId = BVDC_P_HW_ID_INVALID;

	switch (eType)
	{
#if (BVDC_P_NUM_SHARED_VFD > 0)
		case BVDC_P_ResourceType_eVfd:
			*pulCapabilitiesTable = &s_ulVfdAbleFlags[0];
			break;
#endif

#if (BVDC_P_NUM_SHARED_CAP > 0)
		case BVDC_P_ResourceType_eCap:
			pulCapabilitiesTable = &s_ulCaptureAbleFlags[0];
			break;
#endif

#if (BVDC_P_NUM_SHARED_SCL > 0)
		case BVDC_P_ResourceType_eScl:
			pulCapabilitiesTable = &s_ulScalerAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_MAD)
		case BVDC_P_ResourceType_eMad:
			pulCapabilitiesTable = &s_ulMadAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_MCVP)
		case BVDC_P_ResourceType_eMcvp:
			pulCapabilitiesTable = &s_ulMcvpAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_DNR)
		case BVDC_P_ResourceType_eDnr:
			pulCapabilitiesTable = &s_ulDnrAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_XSRC)
		case BVDC_P_ResourceType_eXsrc:
			pulCapabilitiesTable = &s_ulXsrcAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_TNTD)
		case BVDC_P_ResourceType_eTntd:
			pulCapabilitiesTable = &s_ulTntdAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_HIST)
		case BVDC_P_ResourceType_eHist:
			pulCapabilitiesTable = &s_ulHistAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_VNET_CRC)
		case BVDC_P_ResourceType_eVnetCrc:
			pulCapabilitiesTable = &s_ulVnetCrcAbleFlags[0];
			break;
#endif

#if (BVDC_P_NUM_SHARED_DVI > 0)
		case BVDC_P_ResourceType_eDvi:
			pulCapabilitiesTable = &s_ulDviAbleFlags[0];
			break;
#endif

#if (BVDC_P_SUPPORT_DRAIN_F)
		case BVDC_P_ResourceType_eDrainFrn:
			pulCapabilitiesTable = &s_ulDrnFrnAbleFlags[0];
			break;
#endif
		default:
			break;
	}

	if (pulCapabilitiesTable)
	{
		for (i=0; i<=ulResourceCount; i++)
		{
			if (ulCapabilities == *(pulCapabilitiesTable + i))
			{
				*pulResourceId = i;
			}
		}
	}
}
/* End of file. */
