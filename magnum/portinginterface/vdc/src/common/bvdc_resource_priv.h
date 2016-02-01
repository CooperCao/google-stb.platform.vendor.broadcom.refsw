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
#ifndef BVDC_RESOURCE_PRIV_H__
#define BVDC_RESOURCE_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************************/
/*
 * List of possible shared resource types. When a shared resource is acquired,
 * its type is passed as input, and the module handle or HW Id is output.
 * NOTE:  This enumeration is use to index into info table and swtich statement.
 *   Need to update following code when adding new resources.
 *
 * (1) s_aResInfoTbl
 * (2) 2 switch statements in create/destroy
 */
typedef enum BVDC_P_ResourceType
{
	/* shared resources represented by handle */
	BVDC_P_ResourceType_eVfd = 0,          /* shared VFD */
	BVDC_P_ResourceType_eCap,              /* shared CAP */
	BVDC_P_ResourceType_eMad,              /* shared MAD */
	BVDC_P_ResourceType_eXsrc,             /* shared XSRC */
	BVDC_P_ResourceType_eTntd,             /* shared TNTD */
	BVDC_P_ResourceType_eMcvp,             /* shared Mcvp */
	BVDC_P_ResourceType_eDnr,              /* shared DNR */
	BVDC_P_ResourceType_eBoxDetect,        /* shared box detetct */
	BVDC_P_ResourceType_eScl,              /* shared SCL */
	BVDC_P_ResourceType_eHist,             /* shared HIST */
	BVDC_P_ResourceType_eVnetCrc,          /* shared vnet_b crc */

	/* separator between handle resources and HwId resources */
	BVDC_P_ResourceType_eHandleCntr,

	/* shared resources idendified by Hw Id */
	BVDC_P_ResourceType_eFreeCh,           /* shared free channels */
	BVDC_P_ResourceType_eLpBck,            /* shared loop back */
	BVDC_P_ResourceType_eDrainFrn,         /* shared front drains */
	BVDC_P_ResourceType_eDrainBck,         /* shared back drains */
#if BVDC_P_ORTHOGONAL_VEC
	BVDC_P_ResourceType_e656,              /* VEC 656 output */
	BVDC_P_ResourceType_eDvi,              /* VEC DVI output  */
	BVDC_P_ResourceType_eStg,              /* VEC STG output  */
	BVDC_P_ResourceType_eRf,               /* VEC RFM output  */
	BVDC_P_ResourceType_eIt,               /* VEC IT */
	BVDC_P_ResourceType_eVf,               /* VEC VF */
	BVDC_P_ResourceType_eSecam,            /* VEC SECAM */
	BVDC_P_ResourceType_eSecam_HD,         /* VEC HD pass-thru SECAM */
	BVDC_P_ResourceType_eSdsrc,            /* VEC SD SRC */
	BVDC_P_ResourceType_eHdsrc,            /* VEC HD SRC */
	BVDC_P_ResourceType_eDac,              /* DAC */
	BVDC_P_ResourceType_eMpaa,             /* MPAA */
#endif
	BVDC_P_ResourceType_ePcPll,            /* PLL shared by vdec0/1 */
	BVDC_P_ResourceType_eInvalid           /* Invalid. Do not use! */
} BVDC_P_ResourceType;

/***************************************************************************
 * capability flags are or-ed during acquiring.
 */
typedef enum
{
	BVDC_P_Able_eMem0    =     (1<< 0),      /* able to access mem ctrl 0 */
	BVDC_P_Able_eMem1    =     (1<< 1),      /* able to access mem ctrl 1 */
	BVDC_P_Able_eMem2    =     (1<< 2),      /* able to access mem ctrl 2 */
	BVDC_P_Able_eAllSrc  =     (1<< 3),      /* able to use by all sources */
	BVDC_P_Able_eHd      =     (1<< 4),      /* able to handle HD size */
	BVDC_P_Able_eMadr0   =     (1<< 5),      /* able to handle transcode 0*/
	BVDC_P_Able_eMadr1   =     (1<< 6),      /* able to handle transcode 1*/
	BVDC_P_Able_eMadr2   =     (1<< 7),      /* able to handle transcode 2*/
	BVDC_P_Able_eMadr3   =     (1<< 8),      /* able to handle transcode 3*/
	BVDC_P_Able_eMadr4   =     (1<< 9),      /* able to handle transcode 4*/
	BVDC_P_Able_eMadr5   =     (1<<10),      /* able to handle transcode 5*/
	BVDC_P_Able_eHdmi0   =     (1<<11),      /* able to handle HDMI output 0 */
	BVDC_P_Able_eHdmi1   =     (1<<12),      /* able to handle HDMI output 1 */
	BVDC_P_Able_e10bits  =     (1<<13),      /* able to handle 10 bits data */
	BVDC_P_Able_e8bits   =     (1<<14),      /* able to handle 8 bits data  */
	BVDC_P_Able_eInvalid =     (0xffff)      /* cause acquire to fail */

} BVDC_P_Able;

#define BVDC_P_ACQUIRE_ID_INVALID     (0xffffffff)
#define BVDC_P_HW_ID_INVALID          (0xffffffff)
#define BVDC_P_RESOURCE_ID_AVAIL      (0xffff0001)
#define BVDC_P_RESOURCE_ID_TOBEFREED  (0xffff0003)

/* the acquiring record for a resource */
typedef struct BVDC_P_ResourceEntry
{
	union
	{
		void   *  pvHandle;       /* handle */
		uint32_t  ulHwId;         /* or HW ID */
	} Id;
	BVDC_P_ResourceType  eType;   /* type of resource shared */
	uint32_t  ulCapabilities;     /* or-ed mem ctrl access and hd capability */
	uint32_t  ulAcquireId;        /* id of acquirer or user */
	uint32_t  ulAcquireCntr;      /* aquiring times by this acquiring id */
	uint32_t  bAcquireLock;       /* lock from acquiring untill release */
} BVDC_P_ResourceEntry;

/***************************************************************************
 * BVDC_P_ResourceContext
 *
 * It contains the acquiring/releasing record for each shared resource.
 *
 ***************************************************************************/
typedef struct BVDC_P_ResourceContext
{
	BDBG_OBJECT(BVDC_RES)

	/* ptr to the array for all shared resources */
	BVDC_P_ResourceEntry *pResourceRecords;

	/* ptr to the array of index to the entry of the 1st HW module of
	 * each resource type */
	uint32_t  *pulIndex1stEntry;
} BVDC_P_ResourceContext;


/***************************************************************************
 * {private}
 *
 */
BERR_Code  BVDC_P_Resource_Create
	( BVDC_P_Resource_Handle           *phResource,
	  BVDC_Handle                       hVdc );

/***************************************************************************
 * {private}
 *
 */
void  BVDC_P_Resource_Destroy
	( BVDC_P_Resource_Handle            hResource );


/***************************************************************************
 * {private}
 *
 * This func is used to aquire a resource that is represented by a handle.
 * Input: hResource - BVDC_P_Resource_Handle
 *        eType - the type of shared resource
 *        ulCapabilties - required memeory access and HD support capabililies
 *            are or-ed, refer to s_ul*Able[] in bvdc_resource_priv.c for
 *            valid capability combinations. If s_ul*Able[] does not exist
 *            for a type, all hw modules of the type has the same capability
 *            set, and "0" is used for ulCapabilties during acquiring.
 *        ulAcquireId - Idendify the usage (For example, when two windows
 *            that share the same source want to use box detect, they should
 *            acquire box detect module with the same source Id or handle as
 *            ulAcquireId. Later on, one window could release the resource
 *            and another window could still occupy it. This is done with an
 *            internal acquire counter. As another example, a window might
 *            use a scaler in reader or writer side, it should use different
 *            acquireId for the reader usage and writer usage, such as WinId
 *            for reader usage and (0x8000 | WinId) for writer usage).
 *		  bQuery - query if the resource is available when this flag is true
 *
 * Output: the assigned module handle.
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
	  bool                              bQuery);

/***************************************************************************
 * {private}
 *
 * This func is used to aquire a resource that is NOT represented by a handle.
 * Input: hResource - BVDC_P_Resource_Handle
 *        eType - the type of shared resource
 *        ulCapabilties - required memeory access and HD support capabililies
 *            are or-ed, refer to s_ul*Able[] in bvdc_resource_priv.c for
 *            valid capability combinations. If s_ul*Able[] does not exist
 *            for a type, all hw modules of the type has the same capability
 *            set, and "0" is used for ulCapabilties during acquiring.
 *        ulAcquireId - Idendify the usage (For example, when two windows
 *            that share the same source want to use box detect, they should
 *            acquire box detect module with the same source Id or handle as
 *            ulAcquireId. Later on, one window could release the resource
 *            and another window could still occupy it. This is done with an
 *            internal acquire counter. As another example, a window might
 *            use a scaler in reader or writer side, it should use different
 *            acquireId for the reader usage and writer usage, such as WinId
 *            for reader usage and (0x8000 | WinId) for writer usage).
 *        bQuery - query if the resource is available when this flag is true
 *
 * Output: the assigned module HW ID.
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
	  bool                              bQuery);

/***************************************************************************
 * {private}
 *
 * This func is used to release a resource that is represented by a handle.
 * It is user's responsibilty to make sure that the HW resource is really no
 * longer be used in its vnet configuration, before calling to relase.
 *
 * Module realease rule:
 *    when an acquiring record entry with the handle is found, its ulAcquireCntr
 * is decreased by 1. When ulAcquireCntr reaches 0, the module is really
 * released to be acquired with new acquire ID.
 */
BERR_Code  BVDC_P_Resource_ReleaseHandle_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  void                             *pvResourceHandle );

/***************************************************************************
 * {private}
 *
 * This func is used to release a resource that is NOT represented by a handle.
 * It is user's responsibilty to make sure that the HW resource is really no
 * longer be used in its vnet configuration, before calling to relase.
 *
 * Module realease rule:
 *    when an acquiring record entry with the handle is found, its ulAcquireCntr
 * is decreased by 1. When ulAcquireCntr reaches 0, the module is really
 * released to be acquired with new acquire ID.
 */
BERR_Code  BVDC_P_Resource_ReleaseHwId_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulHwId );

/***************************************************************************
 * {private}
 *
 * This func returns the number of acquires of a HW ID resource.
 *
 * For an example of usage, the box detect HW should be disabled only if the
 * acquiring counter becomes 0. If two or more windows are using the box
 * detect, disabling with one window should not really shut down the HW, but
 * causes that one window no longer involves box detect. Notice that tearing
 * off the HW module from the vnet might take one or more field after apply.
 *
 */
uint32_t  BVDC_P_Resource_GetHwIdAcquireCntr_isr
	( BVDC_P_Resource_Handle            hResource,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulHwId );

/***************************************************************************
 * {private}
 *
 * This func returns whether reserve a HW ID resource is successful.
 *
 * BVDC_P_Resource_Reserve_isr specify the index of target instance in eType,
 * BVDC_P_Resource_AcquireHwId_isr obtains the very first available instance.
 *
 */
BERR_Code  BVDC_P_Resource_Reserve_isr
	( BVDC_P_ResourceEntry             *p1stEntry,
	  BVDC_P_ResourceType               eType,
	  uint32_t                          ulResourceIndex,
	  uint32_t                          ulAcquireId,
	  void                            **ppvResourceHandle,
	  uint32_t                         *pulHwId );

/***************************************************************************
 * {private}
 *
 * Based on ulCapabilities this func returns the corresponding index (pulResourceId)
 * to the resource table of the given resource type. The index is then used
 * for obtaining the corresponding resource enum. This is typically used
 * before the window is created and is used for evaluating the BVN cores that
 * are available for the window path.
 *
 * For example, if MCVP is the resource type, a returned index of 0 means
 * s_ulMcvpAbleFlags[0].
 */
void BVDC_P_Resource_GetResourceId
	( BVDC_P_ResourceType               eType,
	  uint32_t                          ulCapabilities,
	  uint32_t                         *pulResourceId );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_RESOURCE_PRIV_H__*/

/* End of file. */
