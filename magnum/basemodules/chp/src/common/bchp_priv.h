/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BCHP_PRIV_H__
#define BCHP_PRIV_H__

#include "bstd.h"
#include "bchp.h"
#include "breg_mem.h"
#include "bkni_multi.h"
#include "bchp_avs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BERR_Code (*BCHP_GetFeatureFunc)(
	const BCHP_Handle hChip,				/* [in] Chip handle. */
	const BCHP_Feature eFeature,			/* [in] Feature to be queried. */
	void *pFeatureValue						/* [out] Feature value .*/
	);

typedef struct BCHP_PWR_P_Context {
    BKNI_MutexHandle lock; /* for internal sync */

    unsigned *pubRefcnt;   /* public refcnt is non-recursive and enforces that you only release
                              what you've previously acquired. only applies to nonleaf nodes */

    unsigned *privRefcnt;  /* private refcnt is recursive. applies to all nodes */
    bool *init;            /* every HW node must be initialized before it can be acquired/released.
                              initialized means powered down */
    bool initComplete;     /* true if BCHP_PWR_P_Init() has completed */
    bool *magnumCtrl;      /* true if MAGNUM_CONTROLLED HW_ node */
    bool *sharedCtrl;      /* true if HW_ node has at least one HW_ node parent that is
                              MAGNUM_CONTROLLED and one that is not */
    bool *secureCtrl;      /* true if SECURE_ACCESS HW_ node */
    BCHP_PmapSettings *pMapSettings;
} BCHP_PWR_P_Context;

typedef struct BCHP_PWR_P_Context *BCHP_PWR_Handle;
typedef struct BCHP_AVS_P_Context *BCHP_AVS_Handle;

#if BCHP_UNIFIED_IMPL
struct BCHP_P_Info
{
    uint32_t      ulChipFamilyIdReg; /* family id including rev */
};
BCHP_Handle BCHP_P_Open(const BCHP_OpenSettings *pSettings, const struct BCHP_P_Info *pChipInfo);
#else
typedef struct BCHP_P_Info
{
    uint32_t      ulChipFamilyIdReg;
    uint16_t      usChipId; /* ignored */
    uint16_t      usMajor;  /* ignored */
    uint16_t      usMinor;  /* ignored */
} BCHP_P_Info;
BCHP_Handle BCHP_P_Open(BREG_Handle hRegister, const BCHP_P_Info *pChipInfo, unsigned chipInfoSize);
#endif

BDBG_OBJECT_ID_DECLARE(BCHP);
typedef struct BCHP_P_Context
{
    BDBG_OBJECT(BCHP)
	BCHP_MemoryInfo memoryInfo;
	bool memoryInfoSet;
	BREG_Handle regHandle;					/* register handle */
	BCHP_GetFeatureFunc pGetFeatureFunc;	/* ptr to GetFeature func. */
	BCHP_PWR_Handle pwrManager;				/* BCHP_PWR handle */
	BCHP_AVS_Handle avsHandle;				/* BCHP_AVS handle */
	BCHP_Info info;
	const struct BCHP_P_Info *pChipInfo;
	BCHP_P_AvsHandle hAvsHandle;
#if BCHP_UNIFIED_IMPL
	BCHP_OpenSettings openSettings;
#endif
    bool skipInitialReset;
} BCHP_P_Context;

typedef enum BCHP_PWR_P_ResourceType {
    BCHP_PWR_P_ResourceType_eLeaf,      /* a leaf node is always a HW_ node */
    BCHP_PWR_P_ResourceType_eNonLeaf,   /* a non-leaf node that is not a HW_ node */
    BCHP_PWR_P_ResourceType_eNonLeafHw, /* a non-leaf node that is also a HW_ node.
                                          these nodes can only have other HW_ nodes as dependencies */
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_P_ResourceType_eDiv
} BCHP_PWR_P_ResourceType;

struct BCHP_PWR_P_Resource {
    BCHP_PWR_P_ResourceType type;
    unsigned id; /* the #define number */
    const char *name;
};

typedef struct BCHP_PWR_P_Resource BCHP_PWR_P_Resource;

typedef struct BCHP_PWR_P_DivTable {
    unsigned mult;
    unsigned prediv;
    unsigned postdiv;
} BCHP_PWR_P_DivTable;

typedef struct BCHP_PWR_P_FreqMap {
    unsigned id; /* Resource id */
    const BCHP_PWR_P_DivTable *pDivTable;
} BCHP_PWR_P_FreqMap;

typedef struct BCHP_PWR_P_MuxTable {
    unsigned mux;
} BCHP_PWR_P_MuxTable;

typedef struct BCHP_PWR_P_MuxMap {
    unsigned id; /* Resource id */
    const BCHP_PWR_P_MuxTable *pMuxTable;
} BCHP_PWR_P_MuxMap;

extern const BCHP_PWR_P_Resource* const * const BCHP_PWR_P_DependList[];
extern const BCHP_PWR_P_Resource* const BCHP_PWR_P_ResourceList[];
extern const BCHP_PWR_P_FreqMap BCHP_PWR_P_FreqMapList[];
extern const BCHP_PWR_P_MuxMap BCHP_PWR_P_MuxMapList[];
extern const BCHP_PmapSettings BCHP_PWR_P_DefaultPMapSettings[];

#define PMAP(reg, field, val) {val, BCHP_##reg##_##field##_SHIFT, BCHP_##reg##_##field##_MASK, BCHP_##reg}
#define DIVTABLE(resource, n, p, m) const BCHP_PWR_P_DivTable BCHP_PWR_P_DivTable_##resource[] = {{n, p, m}}
#define FREQMAP(resource) {BCHP_PWR_##resource, BCHP_PWR_P_DivTable_##resource}

BERR_Code BCHP_PWR_Open(
    BCHP_PWR_Handle *pHandle, /* [out] */
    BCHP_Handle chp           /* [in] */
    );

void BCHP_PWR_Close(
    BCHP_PWR_Handle handle   /* [in] */
    );

BERR_Code BCHP_P_GetDefaultFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue );

void BCHP_P_MuxSelect(BCHP_Handle hChip);

#ifdef __cplusplus
}
#endif

#endif /*BCHP_PRIV_H__*/
