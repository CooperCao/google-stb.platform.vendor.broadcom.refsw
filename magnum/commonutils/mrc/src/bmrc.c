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

#include "bstd.h"
#include "bmrc.h"
#include "bmrc_priv.h"
#include "bmrc_clienttable_priv.h"
#include "bkni.h"                /* Memory management */

#if (BMRC_P_CHECKER_USE_MEMC_GEN_VER > 1)
#if (BMRC_P_MEMC_NUM > 2)
#include "bchp_memc_gen_2.h"
#include "bchp_memc_gen_2_2.h"
#endif
#if (BMRC_P_MEMC_NUM > 1)
#include "bchp_memc_gen_1.h"
#include "bchp_memc_gen_2_1.h"
#endif
#include "bchp_memc_gen_0.h"
#include "bchp_memc_gen_2_0.h"

#elif BMRC_P_CHECKER_USE_MEMC_GEN_VER
#if (BMRC_P_MEMC_NUM > 1)
#include "bchp_memc_gen_1.h"
#endif
#include "bchp_memc_gen_0.h"

#elif BMRC_P_CHECKER_USE_MEMC_ARC_VER
#if (BMRC_P_MEMC_NUM > 2)
#include "bchp_memc_arc_2.h"
#endif
#if (BMRC_P_MEMC_NUM > 1)
#include "bchp_memc_arc_1.h"
#endif
#include "bchp_memc_arc_0.h"
#else /* BMRC_P_CHECKER_USE_MEMC_ARC_VER */
#if (BMRC_P_MEMC_NUM > 2)
#include "bchp_memc_2.h"
#endif
#if (BMRC_P_MEMC_NUM > 1)
#include "bchp_memc_1.h"
#endif
#include "bchp_memc_0.h"
#endif /*(BMRC_P_CHECKER_USE_MEMC_GEN_VER > 1) */

#if (BMRC_P_MEMC_NUM > 2)
#define BMRC_P_BCHP_MEMC_REG(hMrc, reg) ((hMrc->usMemcId == 0) ? BMRC_P_BCHP_MEMC_0_REG(reg) : \
                                         (hMrc->usMemcId == 1) ? BMRC_P_BCHP_MEMC_1_REG(reg) : \
                                         BMRC_P_BCHP_MEMC_2_REG(reg))
#elif (BMRC_P_MEMC_NUM > 1)
#define BMRC_P_BCHP_MEMC_REG(hMrc, reg) ((hMrc->usMemcId == 0) ? BMRC_P_BCHP_MEMC_0_REG(reg) : \
                                         BMRC_P_BCHP_MEMC_1_REG(reg))
#else
#define BMRC_P_BCHP_MEMC_REG(hMrc, reg) (BMRC_P_BCHP_MEMC_0_REG(reg))
#endif

#if (BMRC_P_CHECKER_USE_MEMC_GEN_VER > 2)
#define BMRC_P_BCHP_MEMC_0_REG(reg) BCHP_MEMC_GEN_2_0##_##reg
#define BMRC_P_BCHP_MEMC_1_REG(reg) BCHP_MEMC_GEN_2_1##_##reg
#define BMRC_P_BCHP_MEMC_2_REG(reg) BCHP_MEMC_GEN_2_2##_##reg
#define BMRC_P_GET_FIELD_DATA(mem, reg, field) BCHP_GET_FIELD_DATA(mem, MEMC_GEN_2_0##_##reg, field)
#define BMRC_P_FIELD_DATA(reg, field, data)    BCHP_FIELD_DATA(MEMC_GEN_2_0##_##reg, field, data)
#define BMRC_P_FIELD_ENUM(reg, field, name)    BCHP_FIELD_ENUM(MEMC_GEN_2_0##_##reg, field, name)

#elif BMRC_P_CHECKER_USE_MEMC_GEN_VER
#define BMRC_P_BCHP_MEMC_0_REG(reg) BCHP_MEMC_GEN_0##_##reg
#define BMRC_P_BCHP_MEMC_1_REG(reg) BCHP_MEMC_GEN_1##_##reg
#define BMRC_P_BCHP_MEMC_2_REG(reg) BCHP_MEMC_GEN_2##_##reg
#define BMRC_P_GET_FIELD_DATA(mem, reg, field) BCHP_GET_FIELD_DATA(mem, MEMC_GEN_0##_##reg, field)
#define BMRC_P_FIELD_DATA(reg, field, data)    BCHP_FIELD_DATA(MEMC_GEN_0##_##reg, field, data)
#define BMRC_P_FIELD_ENUM(reg, field, name)    BCHP_FIELD_ENUM(MEMC_GEN_0##_##reg, field, name)
#elif BMRC_P_CHECKER_USE_MEMC_ARC_VER

#define BMRC_P_BCHP_MEMC_0_REG(reg) BCHP_MEMC_ARC_0##_##reg
#define BMRC_P_BCHP_MEMC_1_REG(reg) BCHP_MEMC_ARC_1##_##reg
#define BMRC_P_BCHP_MEMC_2_REG(reg) BCHP_MEMC_ARC_2##_##reg
#define BMRC_P_GET_FIELD_DATA(mem, reg, field) BCHP_GET_FIELD_DATA(mem, MEMC_ARC_0##_##reg, field)
#define BMRC_P_FIELD_DATA(reg, field, data)    BCHP_FIELD_DATA(MEMC_ARC_0##_##reg, field, data)
#define BMRC_P_FIELD_ENUM(reg, field, name)    BCHP_FIELD_ENUM(MEMC_ARC_0##_##reg, field, name)

#else /* BMRC_P_CHECKER_USE_MEMC_ARC_VER */
#define BMRC_P_BCHP_MEMC_0_REG(reg) BCHP_MEMC_0##_##reg
#define BMRC_P_BCHP_MEMC_1_REG(reg) BCHP_MEMC_1##_##reg
#define BMRC_P_BCHP_MEMC_2_REG(reg) BCHP_MEMC_2##_##reg
#define BMRC_P_GET_FIELD_DATA(mem, reg, field) BCHP_GET_FIELD_DATA(mem, MEMC_0##_##reg, field)
#define BMRC_P_FIELD_DATA(reg, field, data)    BCHP_FIELD_DATA(MEMC_0##_##reg, field, data)
#define BMRC_P_FIELD_ENUM(reg, field, name)    BCHP_FIELD_ENUM(MEMC_0##_##reg, field, name)

/* workaround to build on chips where these fields are not symetrical across MRCs, like 7400 */
#ifndef BCHP_MEMC_1_ARC_0_READ_RIGHTS_HIGH
#define BCHP_MEMC_1_ARC_0_READ_RIGHTS_HIGH 0
#endif
#ifndef BCHP_MEMC_1_ARC_0_WRITE_RIGHTS_HIGH
#define BCHP_MEMC_1_ARC_0_WRITE_RIGHTS_HIGH 0
#endif
#ifndef BCHP_MEMC_2_ARC_0_READ_RIGHTS_HIGH
#define BCHP_MEMC_2_ARC_0_READ_RIGHTS_HIGH 0
#endif
#ifndef BCHP_MEMC_2_ARC_0_WRITE_RIGHTS_HIGH
#define BCHP_MEMC_2_ARC_0_WRITE_RIGHTS_HIGH 0
#endif

#endif /* (BMRC_P_CHECKER_USE_MEMC_GEN_VER > 2) */

/* offsets */
#define BMRC_P_CHECKER_REG_OFFSET(hMrc) (BMRC_P_BCHP_MEMC_REG(hMrc, ARC_1_CNTRL) - BMRC_P_BCHP_MEMC_REG(hMrc, ARC_0_CNTRL))
#define BMRC_P_CHECKER_REG_IDX(hMrc, reg) ((BMRC_P_BCHP_MEMC_REG(hMrc, reg) - BMRC_P_BCHP_MEMC_REG(hMrc, ARC_0_CNTRL)) / sizeof(uint32_t))
#define BMRC_P_CHECKER_REG_SIZE  (BMRC_P_BCHP_MEMC_0_REG(ARC_1_CNTRL) - BMRC_P_BCHP_MEMC_0_REG(ARC_0_CNTRL))

#define BMRC_P_MEMC_0_REG_OFFSET 0

#if (BMRC_P_MEMC_NUM > 1)
    #define BMRC_P_MEMC_1_REG_OFFSET (BMRC_P_BCHP_MEMC_1_REG(ARC_0_CNTRL) - BMRC_P_BCHP_MEMC_0_REG(ARC_0_CNTRL))
#else
    #define BMRC_P_MEMC_1_REG_OFFSET 0
#endif

#if (BMRC_P_MEMC_NUM > 2)
    #define BMRC_P_MEMC_2_REG_OFFSET (BMRC_P_BCHP_MEMC_2_REG(ARC_0_CNTRL) - BMRC_P_BCHP_MEMC_0_REG(ARC_0_CNTRL))
#else
    #define BMRC_P_MEMC_2_REG_OFFSET 0
#endif

#if !BMRC_P_CHECKER_USE_NEW_NAME_SUFFIX
#define BMRC_P_NMBX_REG_OFFSET    (BCHP_MEMC_0_NMBX1 - BCHP_MEMC_0_NMBX0)
#endif

#define BMRC_P_Checker_Read32(hMrc, hChecker, reg)         BREG_Read32 (hMrc->hReg, BMRC_P_BCHP_MEMC_REG(hMrc, reg) + hChecker->ulRegOffset)
#define BMRC_P_Checker_Write32(hMrc, hChecker, reg, data)  if(hChecker->aulPrevRegTbl[BMRC_P_CHECKER_REG_IDX(hMrc, reg)] != data || !hChecker->PrevRegTblValid[BMRC_P_CHECKER_REG_IDX(hMrc, reg)]) { \
                                                                hChecker->PrevRegTblValid[BMRC_P_CHECKER_REG_IDX(hMrc, reg)] = true; \
                                                                hChecker->aulPrevRegTbl[BMRC_P_CHECKER_REG_IDX(hMrc, reg)] = data; \
                                                                BREG_Write32(hMrc->hReg, BMRC_P_BCHP_MEMC_REG(hMrc, reg) + hChecker->ulRegOffset, data); \
                                                            }


#define BMRC_P_MEMC_ADRS_SHIFT 3
#define BMRC_P_RANGE_ALIGNMENT_MASK ~0x00000007

#define BMRC_P_CHECKER_COUNT_MAX 8


BDBG_MODULE(BMRC);
BDBG_OBJECT_ID(BMRC);
BDBG_OBJECT_ID(BMRC_Checker);

typedef struct BMRC_P_CheckerContext
{
    BDBG_OBJECT(BMRC_Checker)
    BMRC_Handle hMrc;
    uint16_t usCheckerId;
    bool bExclusive;
    uint32_t ulRegOffset;
    uint32_t ulStart;
    uint32_t ulSize;
    BMRC_AccessType eCheckType;
    BMRC_AccessType eBlockType;
    uint32_t aulReadClients[BMRC_P_CLIENTS_ARRAY_SIZE];
    uint32_t aulWriteClients[BMRC_P_CLIENTS_ARRAY_SIZE];
    bool bEnabled;
    bool bActive;
    bool interruptEnabled;

    /* previous register values */
    uint32_t aulPrevRegTbl[BMRC_P_CHECKER_REG_SIZE];
    bool PrevRegTblValid[BMRC_P_CHECKER_REG_SIZE];

    /* callback data */
    BINT_Id InterruptName;
    BINT_CallbackHandle hCallback;
    BMRC_CallbackFunc_isr pfCbFunc;
    void *pvCbData1;
    int iCbData2;
    BMRC_CheckerInfo stCheckerInfo;

} BMRC_P_CheckerContext;

typedef struct BMRC_P_Context
{
    BDBG_OBJECT(BMRC)
    BREG_Handle           hReg;
    BINT_Handle           hInt;
    BMRC_Settings         stSettings;
    int32_t               lRegOffset;
    uint16_t              usMemcId;
    uint16_t              usMaxCheckers;
    uint16_t              usActiveCheckers;
    bool                  suspended;
    BMRC_P_CheckerContext aCheckers[BMRC_P_CHECKER_COUNT_MAX];

} BMRC_P_Context;

typedef struct BMRC_P_MemcInfo
{
    int32_t  lRegOffset;
    uint16_t usMaxCheckers;

} BMRC_P_MemcInfo;

static const BMRC_Settings s_stDefaultSettings = {
    0 /* Memc Module Id */
};


static const BINT_Id s_saIntIdTbl[][BMRC_P_CHECKER_COUNT_MAX] =
{
    /* memc 0 */
    {
        BMRC_P_MEMC_0_ARC_0_INTR,
        BMRC_P_MEMC_0_ARC_1_INTR,
        BMRC_P_MEMC_0_ARC_2_INTR,
        BMRC_P_MEMC_0_ARC_3_INTR,
        BMRC_P_MEMC_0_ARC_4_INTR,
        BMRC_P_MEMC_0_ARC_5_INTR,
        BMRC_P_MEMC_0_ARC_6_INTR,
        BMRC_P_MEMC_0_ARC_7_INTR
    },

    /* memc 1 */
    {
        BMRC_P_MEMC_1_ARC_0_INTR,
        BMRC_P_MEMC_1_ARC_1_INTR,
        BMRC_P_MEMC_1_ARC_2_INTR,
        BMRC_P_MEMC_1_ARC_3_INTR,
        BMRC_P_MEMC_1_ARC_4_INTR,
        BMRC_P_MEMC_1_ARC_5_INTR,
        BMRC_P_MEMC_1_ARC_6_INTR,
        BMRC_P_MEMC_1_ARC_7_INTR
    },

    /* memc 2 */
    {
        BMRC_P_MEMC_2_ARC_0_INTR,
        BMRC_P_MEMC_2_ARC_1_INTR,
        BMRC_P_MEMC_2_ARC_2_INTR,
        BMRC_P_MEMC_2_ARC_3_INTR,
        BMRC_P_MEMC_2_ARC_4_INTR,
        BMRC_P_MEMC_2_ARC_5_INTR,
        BMRC_P_MEMC_2_ARC_6_INTR,
        BMRC_P_MEMC_2_ARC_7_INTR
    }
};

static const BMRC_P_MemcInfo st_aMemcInfo[] = {
    /* offset,                   max checker count */
    {BMRC_P_MEMC_0_REG_OFFSET, BMRC_P_MEMC_0_CHECKER_COUNT_MAX},
    {BMRC_P_MEMC_1_REG_OFFSET, BMRC_P_MEMC_1_CHECKER_COUNT_MAX},
    {BMRC_P_MEMC_2_REG_OFFSET, BMRC_P_MEMC_2_CHECKER_COUNT_MAX}
};

static void BMRC_P_Checker_Violation_isr ( void *pvData1, int iData2 );
static BERR_Code BMRC_P_Checker_WriteRegs ( BMRC_Checker_Handle hChecker );

/***************************************************************************
 *
 */
BERR_Code
BMRC_GetDefaultSettings
    ( BMRC_Settings *pDefSettings )
{
    if (!pDefSettings)
    {
        return BERR_INVALID_PARAMETER;
    }

    *pDefSettings = s_stDefaultSettings;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Open
    ( BMRC_Handle                     *phMrc,
      BREG_Handle                      hRegister,
      BINT_Handle                      hInterrupt,
      const BMRC_Settings             *pDefSettings )
{
    BMRC_Handle hMrc = NULL;
    BERR_Code err = BERR_SUCCESS;

    BDBG_CASSERT(BMRC_P_MEMC_NUM==BCHP_P_MEMC_COUNT);

    hMrc = (BMRC_Handle) BKNI_Malloc(sizeof(BMRC_P_Context));

    if (!hMrc)
    {
        BDBG_ERR(( "Out of System Memory" ));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BDBG_OBJECT_INIT(hMrc, BMRC);

    hMrc->hReg = hRegister;
    hMrc->hInt = hInterrupt;

    /* Take in default settings. */
    hMrc->stSettings = (pDefSettings) ? *pDefSettings : s_stDefaultSettings;

    if (hMrc->stSettings.usMemcId >= BMRC_P_MEMC_NUM)
    {
        BDBG_ERR(("MemcId %d not supported on this chipset", hMrc->stSettings.usMemcId));
        err = BERR_NOT_SUPPORTED;
        goto error;
    }

    hMrc->usMemcId = hMrc->stSettings.usMemcId;

    /* BMRC_P_MRC_1_REG_OFFSET is actually negative for 3563,
       hence signed long for lRegOffset */
    hMrc->lRegOffset = st_aMemcInfo[hMrc->usMemcId].lRegOffset;

    hMrc->usMaxCheckers = st_aMemcInfo[hMrc->usMemcId].usMaxCheckers;
    hMrc->usActiveCheckers = 0;
    hMrc->suspended = false;
    BKNI_Memset(hMrc->aCheckers, 0, sizeof(BMRC_P_CheckerContext) * BMRC_P_CHECKER_COUNT_MAX);

    *phMrc = hMrc;

    return BERR_SUCCESS;

error:
    BKNI_Free(hMrc);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Close
    ( BMRC_Handle hMrc )
{
    BDBG_OBJECT_ASSERT(hMrc, BMRC);

    BDBG_OBJECT_DESTROY(hMrc, BMRC);
    BKNI_Free(hMrc);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void BMRC_GetSettings ( BMRC_Handle hMrc, BMRC_Settings *pSettings )
{
    BDBG_OBJECT_ASSERT(hMrc, BMRC);
    *pSettings = hMrc->stSettings;
    return;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_GetMaxCheckers
    ( BMRC_Handle hMrc,
      uint32_t *pulMaxChecker )
{
    BDBG_OBJECT_ASSERT(hMrc, BMRC);

    *pulMaxChecker = hMrc->usMaxCheckers;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_Create
    ( BMRC_Handle hMrc,
      BMRC_Checker_Handle *phChecker )
{
    int i = 0;
    uint32_t ulReg = 0;
    BMRC_P_CheckerContext *pCurChecker = NULL;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hMrc, BMRC);
    if(hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_EnterCriticalSection();
    if (hMrc->usActiveCheckers >= hMrc->usMaxCheckers)
    {
        BDBG_ERR(("Maximum number of checkers reached.  Cannot create additional checkers."));
        BKNI_LeaveCriticalSection();

        return BERR_TRACE(BMRC_CHECKER_ERR_ALL_USED);
    }

    for (i = 0; i < hMrc->usMaxCheckers; i++)
    {
        pCurChecker = &(hMrc->aCheckers[i]);

        if (!pCurChecker->bActive)
        {
            hMrc->usActiveCheckers++;
            pCurChecker->hMrc = hMrc;
            pCurChecker->usCheckerId = i;
#if (BMRC_P_CHECKER_USE_MEMC_GEN_VER == 2) /* hack for 7435's MEMC ARC arraingment */
            pCurChecker->ulRegOffset = (i * BMRC_P_CHECKER_REG_OFFSET(hMrc)) + ((i >= 4)? 
				                       (BCHP_MEMC_GEN_2_0_ARC_4_CNTRL - BCHP_MEMC_GEN_0_MEMC64_MBIST_TM_CNTRL): 0);
#else
            pCurChecker->ulRegOffset = i * BMRC_P_CHECKER_REG_OFFSET(hMrc);
#endif
            pCurChecker->ulStart = 0;
            pCurChecker->ulSize = 0;
            pCurChecker->bExclusive = false;
            pCurChecker->eCheckType = BMRC_AccessType_eNone;
            pCurChecker->eBlockType = BMRC_AccessType_eNone;
            pCurChecker->bEnabled = false;
            pCurChecker->bActive = true;
            pCurChecker->interruptEnabled = false;

            pCurChecker->InterruptName = s_saIntIdTbl[hMrc->usMemcId][i];
            pCurChecker->hCallback = 0;
            pCurChecker->pfCbFunc = NULL;
            pCurChecker->pvCbData1 = NULL;
            pCurChecker->iCbData2 = 0;

            BKNI_Memset(pCurChecker->aulReadClients, 0, sizeof(pCurChecker->aulReadClients));
            BKNI_Memset(pCurChecker->aulWriteClients, 0, sizeof(pCurChecker->aulWriteClients));
            BKNI_Memset(pCurChecker->aulPrevRegTbl, 0, sizeof(pCurChecker->aulPrevRegTbl));
            BKNI_Memset(pCurChecker->PrevRegTblValid, 0, sizeof(pCurChecker->PrevRegTblValid));

            *phChecker = (BMRC_Checker_Handle)pCurChecker;

            BDBG_OBJECT_SET(pCurChecker, BMRC_Checker);
            break;
        }
    }
    BKNI_LeaveCriticalSection();

    BDBG_ASSERT(*phChecker);

#if BMRC_P_CHECKER_USE_VIOLATION_INFO_CLEAR
    /* clear out any previous violation */
    ulReg = BMRC_P_FIELD_DATA(ARC_0_VIOLATION_INFO_CLEAR, WRITE_CLEAR, 1);
    BMRC_P_Checker_Write32(hMrc, pCurChecker, ARC_0_VIOLATION_INFO_CLEAR, ulReg);

    ulReg = BMRC_P_FIELD_DATA(ARC_0_VIOLATION_INFO_CLEAR, WRITE_CLEAR, 0);
    BMRC_P_Checker_Write32(hMrc, pCurChecker, ARC_0_VIOLATION_INFO_CLEAR, ulReg);
#else
    BSTD_UNUSED(ulReg);
#endif

    rc = BINT_CreateCallback(&(pCurChecker->hCallback),
        hMrc->hInt,
        pCurChecker->InterruptName,
        BMRC_P_Checker_Violation_isr,
        (void*)pCurChecker, pCurChecker->usCheckerId);
    if (rc) return BERR_TRACE(rc);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_Destroy
    ( BMRC_Checker_Handle hChecker )
{
    BMRC_Handle hMrc;

    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    hMrc = hChecker->hMrc;
    BDBG_OBJECT_ASSERT(hMrc, BMRC);

    BMRC_Checker_Disable(hChecker);
    BMRC_Checker_DisableCallback(hChecker);
    BINT_DestroyCallback(hChecker->hCallback);

    hChecker->bActive = false;
    hMrc->usActiveCheckers--;
    BDBG_OBJECT_UNSET(hChecker, BMRC_Checker);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetRange
    ( BMRC_Checker_Handle hChecker,
      BSTD_DeviceOffset ulStart,
      uint64_t ulSize )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (ulSize == 0)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ((ulSize != (ulSize & BMRC_P_RANGE_ALIGNMENT_MASK)) ||
        (ulStart != (ulStart & BMRC_P_RANGE_ALIGNMENT_MASK)))
    {
        BDBG_ERR(( "ulStart address and ulSize must be 8 byte aligned." ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    hChecker->ulStart = ulStart;
    hChecker->ulSize = ulSize;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetAccessCheck
    ( BMRC_Checker_Handle hChecker,
      BMRC_AccessType eAccessType )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    if (eAccessType == BMRC_AccessType_eNone)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hChecker->eCheckType = eAccessType;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetBlock
    ( BMRC_Checker_Handle hChecker,
      BMRC_AccessType eBlockType )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    hChecker->eBlockType = eBlockType;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetExclusive
    ( BMRC_Checker_Handle hChecker,
      bool bExclusive )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    hChecker->bExclusive = bExclusive;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetClient
    ( BMRC_Checker_Handle hChecker,
      BMRC_Client eClient,
      BMRC_AccessType eAccessType )
{
    int usClientId = BMRC_P_GET_CLIENT_ID(hChecker->hMrc->usMemcId, eClient);
    uint32_t ulClientsIdx = usClientId / BMRC_P_CLIENTS_ARRAY_ELEMENT_SIZE;
    uint32_t ulClientsShift = (usClientId % BMRC_P_CLIENTS_ARRAY_ELEMENT_SIZE);
    uint32_t ulClientsMask = 1 << ulClientsShift;

    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    if (usClientId < 0)
    {
        BDBG_ERR(( "Client %s(%d) not supported on this platform.",  BMRC_P_GET_CLIENT_NAME(eClient), (int)eClient));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(ulClientsIdx < BMRC_P_CLIENTS_ARRAY_SIZE);

    if (eAccessType == BMRC_AccessType_eRead)
    {
        hChecker->aulReadClients[ulClientsIdx] |= ulClientsMask;
        hChecker->aulWriteClients[ulClientsIdx] &= ~ulClientsMask;
    }

    if (eAccessType == BMRC_AccessType_eWrite)
	{
        hChecker->aulReadClients[ulClientsIdx] &= ~ulClientsMask;
        hChecker->aulWriteClients[ulClientsIdx] |= ulClientsMask;
	}
		
	if (eAccessType == BMRC_AccessType_eBoth)
    {
        hChecker->aulReadClients[ulClientsIdx] |= ulClientsMask;
        hChecker->aulWriteClients[ulClientsIdx] |= ulClientsMask;
    }

	if (eAccessType == BMRC_AccessType_eNone)
	{
        hChecker->aulReadClients[ulClientsIdx] &= ~ulClientsMask;
        hChecker->aulWriteClients[ulClientsIdx] &= ~ulClientsMask;
	}

#if 0
    {
        /* this snipped could be used to block MRC from disabling read/write access for certain clients, 
         * it could be usefull if internal table in MRC differes from the actial HW, 
         * and this causes fatal failure (like MRC blocks CPU from accessing memory) */
        unsigned i;
        for(i=0;i<sizeof(hChecker->aulReadClients)/sizeof(hChecker->aulReadClients[0]);i++) {
            unsigned j;
            for(j=0;j<32;j++) {
                unsigned client = i*32 + j;
                if(
                        hChecker->hMrc->stSettings.usMemcId == 0 &&
                        client == 127 ) { /* override configuration and allow client 127 on MEMC0 to access memory */
                    /* above test could be substituted with "if(1) { " which would effectively disable MRC to ever disable access to memory controllers */
                    hChecker->aulReadClients[i] |= 1<<j;
                    hChecker->aulWriteClients[i] |= 1<<j;
                }
            }
        }
    }
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_Enable
    ( BMRC_Checker_Handle hChecker )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    hChecker->bEnabled = true;
    BMRC_P_Checker_WriteRegs(hChecker);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_Disable
    ( BMRC_Checker_Handle hChecker )
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    hChecker->bEnabled = false;
    BMRC_P_Checker_WriteRegs(hChecker);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_EnableCallback
    ( BMRC_Checker_Handle hChecker )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (!hChecker->hCallback)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_NO_CALLBACK_SET);
    }

	rc = BINT_EnableCallback(hChecker->hCallback);
	if (rc) return BERR_TRACE(rc);
    hChecker->interruptEnabled = true;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_DisableCallback
    ( BMRC_Checker_Handle hChecker )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BINT_DisableCallback(hChecker->hCallback);
	if (rc) return BERR_TRACE(rc);
    hChecker->interruptEnabled = false;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_EnableCallback_isr
    ( BMRC_Checker_Handle hChecker )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (!hChecker->hCallback)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_NO_CALLBACK_SET);
    }

    rc = BINT_EnableCallback_isr(hChecker->hCallback);
    if (rc) return BERR_TRACE(rc);
    hChecker->interruptEnabled = true;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_DisableCallback_isr
    ( BMRC_Checker_Handle hChecker )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if(hChecker->hMrc->suspended) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BINT_DisableCallback_isr(hChecker->hCallback);
	if (rc) return BERR_TRACE(rc);
    hChecker->interruptEnabled = false;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_Checker_SetCallback
    ( BMRC_Checker_Handle hChecker,
      const BMRC_CallbackFunc_isr pfCbFunc,
      void *pvCbData1,
      int iCbData2)
{
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);

    if (hChecker->bEnabled)
    {
        return BERR_TRACE(BMRC_CHECKER_ERR_ENABLED_CANT_SET);
    }

    hChecker->pfCbFunc = pfCbFunc;
    hChecker->pvCbData1 = pvCbData1;
    hChecker->iCbData2 = iCbData2;

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void BMRC_P_Checker_Violation_isr
    ( void *pvData1,
      int iData2 )
{
    uint32_t ulReg = 0;
    BMRC_Checker_Handle hChecker = (BMRC_Checker_Handle)pvData1;
    BMRC_CheckerInfo *pCheckerInfo = &(hChecker->stCheckerInfo);
    BMRC_Handle hMrc = hChecker->hMrc;
    BMRC_ClientInfo stClientInfo;

    BSTD_UNUSED(iData2);
    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);
    BDBG_OBJECT_ASSERT(hMrc, BMRC);
    if(hMrc->suspended) {
        return ;
    }

    pCheckerInfo->usMemcId = hMrc->usMemcId;
    pCheckerInfo->usCheckerId = hChecker->usCheckerId;
    pCheckerInfo->ulStart = hChecker->ulStart;
    pCheckerInfo->ulSize = hChecker->ulSize;
    pCheckerInfo->bExclusive = hChecker->bExclusive;

#if !BMRC_P_CHECKER_USE_NEW_NAME_SUFFIX
    ulReg = BMRC_P_Checker_Read32(hMrc, hChecker, ARC_0_VIOLATION_INFO_LOW);
    pCheckerInfo->ulAddress = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_LOW, ADDRESS) << BMRC_P_MEMC_ADRS_SHIFT;

    ulReg = BMRC_P_Checker_Read32(hMrc, hChecker, ARC_0_VIOLATION_INFO_HIGH);
    pCheckerInfo->ulReqType = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_HIGH, REQ_TYPE);

#if BMRC_P_CHECKER_USE_NMBX_ID
    pCheckerInfo->usClientId = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_HIGH, CLIENT_ID);
    pCheckerInfo->ulNmbxId = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_HIGH, NMBX_ID);

    ulReg = BREG_Read32(hMrc->hReg, BCHP_MEMC_0_NMBX0 + (pCheckerInfo->ulNmbxId * BMRC_P_NMBX_REG_OFFSET) + hMrc->lRegOffset + hChecker->ulRegOffset);
    pCheckerInfo->ulNmbx = BMRC_P_GET_FIELD_DATA(ulReg, MEMC_0_NMBX0, NMBX);
#else
    pCheckerInfo->usClientId = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_HIGH, CLIENTID);
    pCheckerInfo->ulNmbx = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_HIGH, NMBX);
#endif

#else
    ulReg = BMRC_P_Checker_Read32(hMrc, hChecker, ARC_0_VIOLATION_INFO_START_ADDR);
    pCheckerInfo->ulAddress = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_START_ADDR, ADDRESS) << BMRC_P_MEMC_ADRS_SHIFT;

    ulReg = BMRC_P_Checker_Read32(hMrc, hChecker, ARC_0_VIOLATION_INFO_END_ADDR);
    pCheckerInfo->ulAddressEnd = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_END_ADDR, ADDRESS) << BMRC_P_MEMC_ADRS_SHIFT;

    ulReg = BMRC_P_Checker_Read32(hMrc, hChecker, ARC_0_VIOLATION_INFO_CMD);
    pCheckerInfo->ulReqType = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_CMD, REQ_TYPE);
    pCheckerInfo->usClientId = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_CMD, CLIENTID);
    pCheckerInfo->ulNmbxId = BMRC_P_GET_FIELD_DATA(ulReg, ARC_0_VIOLATION_INFO_CMD, NMB);
#endif
    BMRC_Checker_P_GetClientInfo_isrsafe(hMrc->stSettings.usMemcId, BMRC_P_GET_CLIENT_ENUM_isrsafe(hMrc->usMemcId, pCheckerInfo->usClientId), &stClientInfo);
    pCheckerInfo->pchClientName = stClientInfo.pchClientName;

#if BMRC_P_CHECKER_USE_VIOLATION_INFO_CLEAR
    /* clear out violation */
    ulReg = BMRC_P_FIELD_DATA(ARC_0_VIOLATION_INFO_CLEAR, WRITE_CLEAR, 1);
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_VIOLATION_INFO_CLEAR, ulReg);

    ulReg = BMRC_P_FIELD_DATA(ARC_0_VIOLATION_INFO_CLEAR, WRITE_CLEAR, 0);
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_VIOLATION_INFO_CLEAR, ulReg);
#endif

    if (hChecker->pfCbFunc)
    {
        hChecker->pfCbFunc(hChecker->pvCbData1, hChecker->iCbData2, pCheckerInfo);
    }
    return;
}

/***************************************************************************
 *
 */
BERR_Code BMRC_P_Checker_WriteRegs
    ( BMRC_Checker_Handle hChecker )
{
    BMRC_Handle hMrc;
    uint32_t ulReg = 0;

    BDBG_OBJECT_ASSERT(hChecker, BMRC_Checker);
    hMrc = hChecker->hMrc;

    if(hMrc->suspended) {
        return  BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (!hChecker->bEnabled)
    {
        /* disable checker and exit */
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CHECK, DISABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CHECK, DISABLED);

        BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_CNTRL, ulReg);

        return BERR_SUCCESS;
    }

    /* write clients */
#if BMRC_P_CLIENTS_MAX == 64
    ulReg = 0;
    ulReg |= hChecker->aulReadClients[0];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_LOW, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[0];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_LOW, ulReg);

	if (((hMrc->usMemcId == 0) && BCHP_MEMC_0_ARC_0_READ_RIGHTS_HIGH) ||
		((hMrc->usMemcId == 1) && BCHP_MEMC_1_ARC_0_READ_RIGHTS_HIGH) ||
		((hMrc->usMemcId == 2) && BCHP_MEMC_2_ARC_0_READ_RIGHTS_HIGH))
	{
		ulReg = 0;
		ulReg |= hChecker->aulReadClients[1];
		BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_HIGH, ulReg);

		ulReg = 0;
		ulReg |= hChecker->aulWriteClients[1];
		BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_HIGH, ulReg);
	}

#elif BMRC_P_CLIENTS_MAX >= 128
    ulReg = 0;
    ulReg |= hChecker->aulReadClients[0];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_0, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[1];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_1, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[2];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_2, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[3];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_3, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[0];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_0, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[1];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_1, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[2];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_2, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[3];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_3, ulReg);
#if BMRC_P_CLIENTS_MAX >= 256
    ulReg = 0;
    ulReg |= hChecker->aulReadClients[4];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_4, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[5];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_5, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[6];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_6, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulReadClients[7];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_READ_RIGHTS_7, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[4];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_4, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[5];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_5, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[6];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_6, ulReg);

    ulReg = 0;
    ulReg |= hChecker->aulWriteClients[7];
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_WRITE_RIGHTS_7, ulReg);
#endif
#else
#error not supported
#endif

    /* write range */
    ulReg = 0;
    ulReg |= BMRC_P_FIELD_DATA(ARC_0_ADRS_RANGE_LOW, ADDRESS,
                             hChecker->ulStart >> BMRC_P_MEMC_ADRS_SHIFT);
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_ADRS_RANGE_LOW, ulReg);

    /* subtract 1 from size to get correct end address offset */
    ulReg = 0;
    ulReg |= BMRC_P_FIELD_DATA(ARC_0_ADRS_RANGE_HIGH, ADDRESS,
                             (hChecker->ulStart + (hChecker->ulSize - 1)) >> BMRC_P_MEMC_ADRS_SHIFT);
    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_ADRS_RANGE_HIGH, ulReg);

    /* write exclusive mode */
    ulReg = 0;
    ulReg |= BMRC_P_FIELD_DATA(ARC_0_CNTRL, MODE, hChecker->bExclusive);

    /* write access check settings */
    if (hChecker->eCheckType == BMRC_AccessType_eWrite)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CHECK, ENABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CHECK, DISABLED);
    }
    else if (hChecker->eCheckType == BMRC_AccessType_eRead)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CHECK, DISABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CHECK, ENABLED);
    }
    else if (hChecker->eCheckType == BMRC_AccessType_eBoth)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CHECK, ENABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CHECK, ENABLED);
    }

    /* write block settings */
    if (hChecker->eBlockType == BMRC_AccessType_eWrite)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CMD_ABORT, ENABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CMD_ABORT, DISABLED);
    }
    else if (hChecker->eBlockType == BMRC_AccessType_eRead)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CMD_ABORT, DISABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CMD_ABORT, ENABLED);
    }
    else if (hChecker->eBlockType == BMRC_AccessType_eBoth)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CMD_ABORT, ENABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CMD_ABORT, ENABLED);
    }
    else if (hChecker->eBlockType == BMRC_AccessType_eNone)
    {
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CMD_ABORT, DISABLED);
        ulReg |= BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CMD_ABORT, DISABLED);
    }

    BMRC_P_Checker_Write32(hMrc, hChecker, ARC_0_CNTRL, ulReg);

    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_GetClientInfo(BMRC_Handle hMrc, BMRC_Client eClient, BMRC_ClientInfo *pClientInfo)
{
    BDBG_OBJECT_ASSERT(hMrc, BMRC);
    return BMRC_Checker_P_GetClientInfo_isrsafe(hMrc->stSettings.usMemcId, eClient, pClientInfo);
}

void BMRC_Standby( BMRC_Handle hMrc )
{
    unsigned i;
    BDBG_OBJECT_ASSERT(hMrc, BMRC);
    if(hMrc->suspended) {(void)BERR_TRACE(BERR_NOT_SUPPORTED);return;}
    BKNI_EnterCriticalSection(); /* set a barrier */
    hMrc->suspended = true; 
    BKNI_LeaveCriticalSection();
    for (i = 0; i < hMrc->usMaxCheckers; i++) {
        BMRC_P_CheckerContext *checker = &hMrc->aCheckers[i];
        if(checker->bActive && checker->interruptEnabled) {
            BINT_DisableCallback(checker->hCallback);
        }
    }
    return;
}

void BMRC_Resume( BMRC_Handle hMrc )
{
    unsigned i;
    BDBG_OBJECT_ASSERT(hMrc, BMRC);

    if(!hMrc->suspended) { (void)BERR_TRACE(BERR_NOT_SUPPORTED); return;}

    BKNI_EnterCriticalSection(); /* set a barrier */
    hMrc->suspended = false;
    BKNI_LeaveCriticalSection();

    for (i = 0; i < hMrc->usMaxCheckers; i++)
    {
        BMRC_P_CheckerContext *checker = &hMrc->aCheckers[i];
        if(checker->bActive) {
            BKNI_Memset(checker->aulPrevRegTbl, 0, sizeof(checker->aulPrevRegTbl));
            BKNI_Memset(checker->PrevRegTblValid, 0, sizeof(checker->PrevRegTblValid));
            BMRC_P_Checker_WriteRegs ( checker);
            if(checker->interruptEnabled) {
                BINT_EnableCallback(checker->hCallback);
            }
        }
    }
    return;
}

void BMRC_PrintBlockingArcs(BREG_Handle reg)
{
    unsigned memcIndex, arcIndex;
    for (memcIndex=0;memcIndex<BMRC_P_MEMC_NUM;memcIndex++) {
        for (arcIndex=0;arcIndex<st_aMemcInfo[memcIndex].usMaxCheckers;arcIndex++) {
            uint32_t value;
            unsigned regAddr = BMRC_P_BCHP_MEMC_0_REG(ARC_0_CNTRL);
            regAddr += st_aMemcInfo[memcIndex].lRegOffset;
            regAddr += (BMRC_P_BCHP_MEMC_0_REG(ARC_1_CNTRL) - BMRC_P_BCHP_MEMC_0_REG(ARC_0_CNTRL)) * arcIndex;
            value = BREG_Read32(reg, regAddr);
            if (value & BMRC_P_FIELD_ENUM(ARC_0_CNTRL, WRITE_CMD_ABORT, ENABLED)) {
                BDBG_WRN(("MEMC%d ARC%d is blocking writes", memcIndex, arcIndex));
            }
            if (value & BMRC_P_FIELD_ENUM(ARC_0_CNTRL, READ_CMD_ABORT, ENABLED)) {
                BDBG_WRN(("MEMC%d ARC%d is blocking reads", memcIndex, arcIndex));
            }
        }
    }
}

/* End of File */
