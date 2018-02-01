/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bint_plat.h"  /* include other interrupt interface headers */
#include "bint_stats.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "blst_slist.h"
#include "bchp.h"
#include "btmr.h"

BDBG_MODULE(int);

#define BINT_REENTRANT  1
#define BINT_NON_REENTRANT  0

#ifndef BINT_REENTRANT_CONFIG
#define BINT_REENTRANT_CONFIG   BINT_REENTRANT
#endif

#ifndef BKNI_ASSERT_ISR_CONTEXT
#define BKNI_ASSERT_ISR_CONTEXT() (void)0
#endif

/* forward typedef to manage circular dependencies between structures */
typedef struct BINT_P_Callback *BINT_P_CallbackHandle;
typedef struct BINT_P_L2Register BINT_P_L2Register;

/*
Summary:
This structure defines the head element for creating
a linked list.
*/
typedef struct BINT_P_cblHead BINT_P_cblHead;
BLST_S_HEAD(BINT_P_cblHead, BINT_P_Callback);

BLST_S_HEAD(BINT_P_L2RegisterList, BINT_P_L2Register);

typedef struct BINT_P_L2Aggregator {
    BLST_S_ENTRY(BINT_P_L2Aggregator) link; /* linked list support */
    unsigned L2Shift;
    struct BINT_P_L2RegisterList L2RegList; /* list of all L2 registers for this L2 bit */
} BINT_P_L2Aggregator;


/*
Summary:
This structure defines a single element in the
table used in the InterruptInterface.

Description:
One and only instance of this structure will exist for each L2 interrupt bit.
*/
typedef struct BINT_P_L2Int
{
    BLST_S_ENTRY(BINT_P_L2Int) link; /* linked list support */
    BINT_P_L2Register *L2Reg; /* pointer back to L2Register */
    BINT_Id intId; /* Interrupt ID (contains L2 base and L2 shift) */
    int enableCount; /* Number of callbacks that are enabled for this interrupt */
    BINT_P_cblHead callbackList; /* list of callbacks associated with this interrupt */
    unsigned count;     /* number of times when L2 interrupt was fired */
} BINT_P_L2Int;

/*
Summary:
This structure defines the head element for creating
a linked list.
*/
typedef struct BINT_P_L2Head BINT_P_L2Head;
BLST_S_HEAD(BINT_P_L2Head, BINT_P_L2Int);

/*
Summary:
This structure defines the callback handle/context.  It includes
linked list support.

Description:
One of these structures exists for each callback created.  There
may be multiple callbacks assigned for a single interrupt.
*/
BDBG_OBJECT_ID(BINT_Callback);
typedef struct BINT_P_Callback
{
    BDBG_OBJECT(BINT_Callback)
    BLST_S_ENTRY(BINT_P_Callback) link; /* linked list support */
    BLST_S_ENTRY(BINT_P_Callback) allCbLink; /* links all callbacks created for traversing */
    BINT_P_L2Int *L2Handle; /* L2 handle for this interrupt */
    BINT_CallbackFunc func_isr; /* function to call when the interrupt triggers */
    void * pParm1; /* returned when callback is executed */
    int parm2; /* returned when callback is executed */
    const char *callbackName; /* callback name saved for the debug builds */

    bool enabled; /* false: callback will never be executed, true: callback will be executed when interrupt triggers */

    uint32_t ulStartTime;

    unsigned count; /* number of times when callback interrupt was fired */

    BINT_Stats_CallbackStats StatInfo;
} BINT_P_Callback;

struct BINT_P_L2Register {
    BLST_S_ENTRY(BINT_P_L2Register) link;
    BINT_P_IntMap intMapEntry; /* interrupt map entry (copy with 'correct' L1Shift)*/
    bool standard;
    bool weakMask;
    bool l3Root;
    int enableCount; /* Number of callbacks that are enabled for this register */
    unsigned count; /* Number of times when interrupt fired for this register */
    BINT_Handle intHandle; /* handle to the main InterruptInterface */
    BLST_S_HEAD(BINT_P_L2IntList, BINT_P_L2Int) intList; /* L2 Interrupts for this register */
    BLST_S_HEAD(BINT_P_L2AggregatorList, BINT_P_L2Aggregator) aggregatorList; /* List of L2(L3) registers connected to this L2 register */
};

/*
Summary:
This structure defines state per each L1 interrupt bit.
*/
typedef struct BINT_P_L1 {
    struct BINT_P_L2RegisterList L2RegList; /* list of all L2 Registers for this L1 bit */
} BINT_P_L1;

BDBG_OBJECT_ID(BINT);

typedef struct BINT_P_Context
{
    BDBG_OBJECT(BINT)
#if BINT_REENTRANT_CONFIG==BINT_REENTRANT
    BKNI_MutexHandle lock;
#define BINT_LOCK(h) BKNI_AcquireMutex((h)->lock)
#define BINT_UNLOCK(h) BKNI_ReleaseMutex((h)->lock)
#else
#define BINT_LOCK(h)
#define BINT_UNLOCK(h)
#endif
    int callbackCount; /* Number of callbacks installed in InterruptInterface */
    unsigned numInts; /* Number of L2 interrupts managed by this instance of the InterruptInterface */
    BREG_Handle regHandle; /* regHandle for accessing interrupt registers */
    BINT_P_cblHead allCbList; /* list of all callbacks registered */
    BINT_Settings settings; /* BINT_Settings */
    BTMR_TimerHandle hTimer; /* timer used for stats tracking */
    bool bStatsEnable; /* enables stats tracking */

    BINT_P_L1 L1[BINT_P_L1_SIZE]; /* Array of L1 interrupt states */
    BINT_P_L2Register *L2RegisterData; /* pointer to store data for all L2 registers */
    BINT_P_L2Aggregator *L2AggregatorData; /* pointer to store data for all L2->L3 mappings */

} BINT_P_Context;

/* Default bin configuration */
static const BINT_Stats_CallbackBin g_aDefaultBins[] =
{
    /* range min, range max, bin hit count */
    {  0,         50,        0 },
    {  51,        100,       0 },
    {  101,       200,       0 },
    {  201,       500,       0 },
    {  501,       1000,      0 },
    {  1000,      2500,      0 },
    {  2501,      10000,     0 },
    {  10001,     50000,     0 },
    {  50001,     100000,    0 },
    {  100001,    500000,    0 },
};

#define BINT_P_STATS_DEFAULT_BINS_NUM \
    (sizeof (g_aDefaultBins) / sizeof (BINT_Stats_CallbackBin))

#ifdef BINT_STATS_ENABLE
static BERR_Code BINT_P_Stats_ComputeStats_isr( BINT_CallbackHandle cbHandle, uint32_t ulStart, uint32_t ulEnd );
static uint32_t BINT_P_GetElapsedTime_isr( uint32_t ulTimerStart, uint32_t ulTimerEnd );
#endif
static void BINT_P_ClearIntNoOp_isrsafe( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BSTD_UNUSED(regHandle);
    BSTD_UNUSED(baseAddr);
    BSTD_UNUSED(shift);
    return;
}

static void BINT_P_ClearInt_isrsafe(BINT_Handle intHandle, const BINT_P_L2Register *L2Reg, unsigned L2Shift)
{
    if(L2Reg->standard) {
        BREG_Write32(intHandle->regHandle, L2Reg->intMapEntry.L2RegOffset+BINT_P_STD_CLEAR, 1<<L2Shift);
    } else {
        intHandle->settings.pClearInt( intHandle->regHandle, L2Reg->intMapEntry.L2RegOffset, L2Shift);
    }
    return;
}

static void BINT_P_SetMask_isrsafe(BINT_Handle intHandle, bool standard, uint32_t L2RegOffset, unsigned L2Shift)
{
    if(standard) {
        BREG_Write32(intHandle->regHandle, L2RegOffset+BINT_P_STD_MASK_SET, 1<<L2Shift);
    } else {
        intHandle->settings.pSetMask(intHandle->regHandle, L2RegOffset, L2Shift);
    }
    return;
}


static void BINT_P_ClearMask_isrsafe(BINT_Handle intHandle, const BINT_P_L2Register *L2, unsigned L2Shift)
{
    bool standard = L2->standard;
    uint32_t L2RegOffset = L2->intMapEntry.L2RegOffset;
    if(standard) {
        BREG_Write32( intHandle->regHandle, L2RegOffset+BINT_P_STD_MASK_CLEAR, 1<<L2Shift);
    } else {
        intHandle->settings.pClearMask(intHandle->regHandle, L2RegOffset, L2Shift);
    }
    return;
}


void BINT_GetDefaultCustomSettings( BINT_CustomSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static bool BINT_L2IsDisabled(const BINT_CustomSettings *pCustomSettings, unsigned addr)
{
    if (pCustomSettings) {
        unsigned i;
        for (i=0;i<sizeof(pCustomSettings->disabledL2Registers.address)/sizeof(pCustomSettings->disabledL2Registers.address[0]) && pCustomSettings->disabledL2Registers.address[i]; i++) {
            if (pCustomSettings->disabledL2Registers.address[i] == addr) return true;
        }
    }
    return false;
}

static BERR_Code BINT_P_VerifyL2Register(BINT_Handle intHandle, const BINT_P_IntMap *intMapEntry, unsigned last)
{
    unsigned i = last;
    unsigned j;
    int L1Shift = BINT_MAP_GET_L1SHIFT(intMapEntry);

    for(j=0;j<i;j++) {
        BINT_P_L2Register *L2RegOld = intHandle->L2RegisterData + j;
        BINT_P_IntMap *intMapEntryOld = &L2RegOld->intMapEntry;
        if(intMapEntryOld->L2RegOffset == intMapEntry->L2RegOffset) {
            if( ~(intMapEntryOld->L2InvalidMask | intMapEntry->L2InvalidMask) != 0 || intMapEntryOld->L2InvalidMask == BINT_DONT_PROCESS_L2 || intMapEntry->L2InvalidMask == BINT_DONT_PROCESS_L2) {
                BDBG_ERR(("Duplicated mask bits used for %#x (%s,%u:%s,%u), mask %#x&%#x->%#x", intMapEntry->L2RegOffset, intMapEntryOld->L2Name, intMapEntryOld->L1Shift, intMapEntry->L2Name, L1Shift, intMapEntryOld->L2InvalidMask, intMapEntry->L2InvalidMask, ~(intMapEntryOld->L2InvalidMask | intMapEntry->L2InvalidMask)));
            }
            if(intMapEntryOld->L1Shift==L1Shift) {
                BDBG_WRN(("found duplicated entry for %#x (%u:%s:%s), combining mask %#x&%#x->%#x", intMapEntry->L2RegOffset, L1Shift, intMapEntryOld->L2Name, intMapEntry->L2Name, intMapEntryOld->L2InvalidMask, intMapEntry->L2InvalidMask, intMapEntryOld->L2InvalidMask & intMapEntry->L2InvalidMask));
                intMapEntryOld->L2InvalidMask = intMapEntryOld->L2InvalidMask & intMapEntry->L2InvalidMask;
                break;
            }
        }
    }
    if(j!=i) {
        return BERR_NOT_SUPPORTED;
    }
    return BERR_SUCCESS;
}

struct BINT_P_OpenState {
    unsigned countStandard;
    unsigned countWeakMask;
    unsigned countExternal;
    unsigned countL3;
};

static BERR_Code BINT_P_InitializeL2Register(BINT_P_L2Register *L2Reg, struct BINT_P_OpenState *state, const BINT_P_IntMap *intMapEntry, const BINT_CustomSettings *pCustomSettings)
{
    int L1Shift = BINT_MAP_GET_L1SHIFT(intMapEntry);

    BDBG_MSG(("L2: %d %#x(%#x) %s %s %s %s", L1Shift, intMapEntry->L2RegOffset, intMapEntry->L2InvalidMask, intMapEntry->L2Name, (intMapEntry->L1Shift & BINT_IS_STANDARD)==BINT_IS_STANDARD?"STANDARD":"",(intMapEntry->L1Shift & BINT_P_MAP_MISC_WEAK_MASK)==BINT_P_MAP_MISC_WEAK_MASK?"WEAK_MASK":"", (intMapEntry->L1Shift & BINT_P_MAP_MISC_L3_MASK)==BINT_P_MAP_MISC_L3_MASK?"L3":"" ));
    if(BINT_MAP_IS_EXTERNAL(intMapEntry)) { /* external interrupt */
        state->countExternal ++;
        return BERR_NOT_SUPPORTED; /* no trace here */
    }
    if (BINT_L2IsDisabled(pCustomSettings, intMapEntry->L2RegOffset)) {
        return BERR_NOT_SUPPORTED; /* no trace here */
    }
    L2Reg->intMapEntry = *intMapEntry;
    L2Reg->intMapEntry.L1Shift = L1Shift; /* correct L1Shift */
    BLST_S_INIT(&L2Reg->intList);
    BLST_S_INIT(&L2Reg->aggregatorList);

    L2Reg->standard = (intMapEntry->L1Shift & BINT_IS_STANDARD)==BINT_IS_STANDARD;
    L2Reg->l3Root= (intMapEntry->L1Shift & BINT_P_MAP_MISC_L3_ROOT_MASK)==BINT_P_MAP_MISC_L3_ROOT_MASK;
    L2Reg->weakMask = (intMapEntry->L1Shift & BINT_P_MAP_MISC_WEAK_MASK)==BINT_P_MAP_MISC_WEAK_MASK;
    L2Reg->enableCount = 0;
    L2Reg->count = 0;
    if(L2Reg->standard && L2Reg->weakMask) {/* interrupt couldn't be both standard and with a 'weak' mask */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if(L2Reg->intMapEntry.L2InvalidMask == BINT_DONT_PROCESS_L2 && (L2Reg->standard || L2Reg->weakMask)) {/* interrupt couldn't be both don't process and either standard or with a 'weak' mask */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    state->countStandard += L2Reg->standard?1:0;
    state->countWeakMask += L2Reg->weakMask?1:0;
    BDBG_ASSERT(L1Shift<BINT_P_L1_SIZE);
    return BERR_SUCCESS;
}

static BERR_Code BINT_P_AddL3Interrupts(BINT_Handle intHandle, BINT_P_L2Register *L2, struct BINT_P_OpenState *state, unsigned first, unsigned count, const BINT_CustomSettings *pCustomSettings )
{
    unsigned i;
    BINT_P_L2Aggregator *L2Aggregator;

    for(i=first;i<first+count;i++) {
        const BINT_P_IntMap *intMapEntry = &intHandle->settings.pIntMap[i];
        BINT_P_L2Register *L2Reg = &intHandle->L2RegisterData[i];
        unsigned L2Shift = (intMapEntry->L1Shift >> BINT_P_MAP_L2_SHIFT) & 0x0F;
        BERR_Code rc;

        BDBG_ASSERT( (intMapEntry->L1Shift & BINT_P_MAP_MISC_L3_MASK) == BINT_P_MAP_MISC_L3_MASK); /* this entry should for L3 interrupt */
        if( (intMapEntry->L1Shift & BINT_IS_STANDARD) != BINT_IS_STANDARD) {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        rc = BINT_P_InitializeL2Register(L2Reg, state, intMapEntry, pCustomSettings);
        if(rc==BERR_NOT_SUPPORTED) {
            rc = BERR_SUCCESS;continue;
        }
        rc = BINT_P_VerifyL2Register(intHandle, intMapEntry, i);
        if(rc==BERR_NOT_SUPPORTED) {
            rc = BERR_SUCCESS;continue;
        }

        for(L2Aggregator=BLST_S_FIRST(&L2Reg->aggregatorList);L2Aggregator;L2Aggregator=BLST_S_NEXT(L2Aggregator, link)) {
            if(L2Aggregator->L2Shift==L2Shift) {
                break;
            }
        }
        if(L2Aggregator==NULL) { /* new L2 bit connected to L3 interrupt register */
            L2Aggregator = &intHandle->L2AggregatorData[state->countL3];
            L2Aggregator->L2Shift = L2Shift;
            BLST_S_INIT(&L2Aggregator->L2RegList);
            BLST_S_INSERT_HEAD(&L2->aggregatorList, L2Aggregator, link);
        }
        L2Reg->intHandle = intHandle;
        BLST_S_INSERT_HEAD(&L2Aggregator->L2RegList, L2Reg, link);
        state->countL3++;
    }
    return BERR_SUCCESS;
}

BERR_Code BINT_Open( BINT_Handle *pHandle, BREG_Handle regHandle, const BINT_Settings *pDefSettings,
    const BINT_CustomSettings *pCustomSettings )
{
    int i;
    BERR_Code rc = BERR_SUCCESS;
    BINT_Handle intHandle;
    struct BINT_P_OpenState openState;
    unsigned l3count;

    openState.countStandard=0;
    openState.countWeakMask=0;
    openState.countExternal=0;
    openState.countL3=0;

    intHandle = BKNI_Malloc( sizeof( BINT_P_Context ) );
    if( intHandle == NULL )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset( intHandle, 0, sizeof( *intHandle) );
    BDBG_OBJECT_SET(intHandle, BINT);

#if BINT_REENTRANT_CONFIG==BINT_REENTRANT
    rc = BKNI_CreateMutex(&intHandle->lock);
    if(rc!=BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto error;
    }
#endif

    intHandle->numInts = 0;
    intHandle->callbackCount = 0;
    intHandle->settings = *pDefSettings;
    intHandle->settings.pClearInt = intHandle->settings.pClearInt ? intHandle->settings.pClearInt : BINT_P_ClearIntNoOp_isrsafe;
    intHandle->L2RegisterData = NULL;
    intHandle->L2AggregatorData = NULL;
    intHandle->hTimer = NULL;
    intHandle->bStatsEnable = false;

    intHandle->regHandle = regHandle;

    if( intHandle->settings.pReadStatus == NULL ||
        intHandle->settings.pSetMask == NULL ||
        intHandle->settings.pClearMask == NULL )
    {
        BDBG_ERR(("Invalid function points passed into BINT_Open()"));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(0);
        goto error;
    }


    /* If your platform has BCHP_PWR-based power management, then the SW layer that is calling BINT_Open
       must also acquire/release power around BINT_Open. Otherwise, you'll likely hit GISB errors below,
       when registers are accessed without power to their respective 108M clocks.

       See /rockford/appframework/src/common/appframework/framework.c or nexus_core.c for examples
       on how to do this */

#ifndef BINT_OPEN_BYPASS_L2INIT
    /* Clear all L2 interrupts */
    for( i=0;;i++) {
        unsigned bit;
        const BINT_P_IntMap *L2Register = &intHandle->settings.pIntMap[i];
        if (L2Register->L1Shift<0) {
            break;
        }
        if(BINT_MAP_IS_EXTERNAL(L2Register)) {
            continue;
        }
        if (BINT_L2IsDisabled(pCustomSettings, L2Register->L2RegOffset)) {
            continue;
        }
        if(L2Register->L1Shift&BINT_P_MAP_MISC_L3_ROOT_MASK) { /* don't apply mask for L3 ROOT interrupt */
            continue;
        }
        for(bit=0; bit<32; bit++) {
            if ( (L2Register->L2InvalidMask&(1<<bit))==0) {
                BINT_P_SetMask_isrsafe(intHandle, (L2Register->L1Shift&BINT_IS_STANDARD)==BINT_IS_STANDARD, L2Register->L2RegOffset, bit);
            }
        }
    }
#endif
    for( i=0; i<BINT_P_L1_SIZE; i++ )
    {
        BLST_S_INIT(&intHandle->L1[i].L2RegList);
    }
    /* count number of L2/L3 Registers */
    for( i=0,l3count=0;;i++) {
        if(intHandle->settings.pIntMap[i].L1Shift<0) {
            break;
        }
        if(intHandle->settings.pIntMap[i].L1Shift & BINT_P_MAP_MISC_L3_MASK) {
            l3count++;
        }
    }
    intHandle->L2RegisterData = BKNI_Malloc(sizeof(*intHandle->L2RegisterData)*i);
    if(intHandle->L2RegisterData==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
    if(l3count>0) {
        intHandle->L2AggregatorData = BKNI_Malloc(sizeof(*intHandle->L2AggregatorData)*l3count);
        if(intHandle->L2AggregatorData==NULL){rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
    }

    for(i=0;;i++) {
        const BINT_P_IntMap *intMapEntry = &intHandle->settings.pIntMap[i];
        BINT_P_L2Register *L2Reg = &intHandle->L2RegisterData[i];
        int L1Shift = BINT_MAP_GET_L1SHIFT(intMapEntry);

        if(intMapEntry->L1Shift<0) {
            break;
        }
        if( (intMapEntry->L1Shift & BINT_P_MAP_MISC_L3_MASK) == BINT_P_MAP_MISC_L3_MASK) {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            continue;
        }
        rc = BINT_P_InitializeL2Register(L2Reg, &openState, intMapEntry, pCustomSettings);
        if(rc==BERR_NOT_SUPPORTED) {
            rc = BERR_SUCCESS;continue;
        }
        rc = BINT_P_VerifyL2Register(intHandle, intMapEntry, i);
        if(rc==BERR_NOT_SUPPORTED) {
            rc = BERR_SUCCESS;continue;
        }
        L2Reg->intHandle = intHandle;
        BLST_S_INSERT_HEAD(&intHandle->L1[L1Shift].L2RegList, L2Reg, link);
#ifndef BINT_OPEN_BYPASS_L2INIT
        if(L2Reg->weakMask) {
            unsigned bit;
            for(bit=0; bit<32; bit++) {
                if ( (L2Reg->intMapEntry.L2InvalidMask&(1<<bit))==0) {
                    BINT_P_ClearInt_isrsafe(intHandle, L2Reg, bit);
                }
            }
        }
#endif
        if((intMapEntry->L1Shift & BINT_P_MAP_MISC_L3_ROOT_MASK) == BINT_P_MAP_MISC_L3_ROOT_MASK) {
            for(l3count=0;;l3count++) {
                const BINT_P_IntMap *intMapEntryL3 = &intMapEntry[1+l3count];


                if(intMapEntryL3->L1Shift<0) {
                    break;
                }
                if( (intMapEntryL3->L1Shift & BINT_P_MAP_MISC_L3_MASK) != BINT_P_MAP_MISC_L3_MASK) {
                    break;
                }
                /* found L3 interrupt */;
                if( (intMapEntryL3->L1Shift & BINT_IS_STANDARD) != BINT_IS_STANDARD) {
                    (void)BERR_TRACE(BERR_NOT_SUPPORTED); /* L2 must be standard interrupt */
                    BDBG_ASSERT(0);
                }
                if(L1Shift != BINT_MAP_GET_L1SHIFT(intMapEntryL3)) {
                    (void)BERR_TRACE(BERR_NOT_SUPPORTED); /* L1 Shift should match for all L3 interrupts */
                }
            }
            BDBG_ASSERT(l3count);
            if(l3count) {
                BINT_P_AddL3Interrupts(intHandle, L2Reg, &openState, i+1, l3count, pCustomSettings);
                i+= l3count; /* skip L3 interrupts */
            }
        }
    }
    BDBG_MSG(("using BINT with %u external interrupts, %u standard interrupts, %u L3 interrupts and %u with a 'weak' mask", openState.countExternal, openState.countStandard, openState.countL3, openState.countWeakMask));

    *pHandle = intHandle;

    return rc;

error:

    if( intHandle != NULL )
    {
#if BINT_REENTRANT_CONFIG==BINT_REENTRANT
        if(intHandle->lock)
        {
            BKNI_DestroyMutex(intHandle->lock);
        }
#endif
        if(intHandle->L2AggregatorData) {
            BKNI_Free(intHandle->L2AggregatorData);
        }
        if(intHandle->L2RegisterData) {
            BKNI_Free(intHandle->L2RegisterData);
        }
        BKNI_Free( intHandle );
    }
    *pHandle = NULL;

    return rc;
}

static void BINT_P_FreeL2Regs(const struct BINT_P_L2RegisterList *L2RegList)
{
    BINT_P_L2Register *L2Reg;

    for(L2Reg = BLST_S_FIRST(L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
        BINT_P_L2Int *L2Handle;

        while(NULL!=(L2Handle=BLST_S_FIRST(&L2Reg->intList))) {
            BINT_CallbackHandle cbHandle;

            while(NULL!=(cbHandle=BLST_S_FIRST(&L2Handle->callbackList))) {
                BINT_DestroyCallback(cbHandle);
            }

            BKNI_EnterCriticalSection();
            BLST_S_REMOVE_HEAD(&L2Reg->intList, link);
            BKNI_LeaveCriticalSection();
            BKNI_Free( L2Handle );
        }
    }
    return;
}

BERR_Code BINT_Close( BINT_Handle intHandle )
{
    int L1Shift;

    BDBG_OBJECT_ASSERT(intHandle, BINT);
    BDBG_ASSERT ( intHandle->bStatsEnable == false );

    for(L1Shift=0; L1Shift<BINT_P_L1_SIZE; L1Shift++ ) {
        BINT_P_L2Register *L2Reg;
        BINT_P_FreeL2Regs(&intHandle->L1[L1Shift].L2RegList);

        for(L2Reg = BLST_S_FIRST(&intHandle->L1[L1Shift].L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
            const BINT_P_L2Aggregator *L2Aggregator;
            for(L2Aggregator = BLST_S_FIRST(&L2Reg->aggregatorList); L2Aggregator; L2Aggregator = BLST_S_NEXT(L2Aggregator, link)) {
                BINT_P_FreeL2Regs(&L2Aggregator->L2RegList);
            }
        }
    }

    if( intHandle->callbackCount != 0 ) {
        return BERR_TRACE(BERR_UNKNOWN);
    }

#if BINT_REENTRANT_CONFIG==BINT_REENTRANT
    BKNI_DestroyMutex(intHandle->lock);
#endif
    if(intHandle->L2AggregatorData) {
        BKNI_Free(intHandle->L2AggregatorData);
    }
    BKNI_Free(intHandle->L2RegisterData);
    BDBG_OBJECT_DESTROY(intHandle, BINT);
    BKNI_Free( intHandle );

    return BERR_SUCCESS;
}

static void BINT_P_ExecuteCallback_isr(BINT_Handle intHandle, BINT_P_Callback *cbHandle)
{
#ifdef BINT_STATS_ENABLE
    uint32_t ulStart, ulEnd = 0;
    bool bStatsEnable = intHandle->bStatsEnable;
    BERR_Code rc;

    if (bStatsEnable)
    {
        rc = BTMR_ReadTimer_isr(intHandle->hTimer, &ulStart);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Error reading timer for statistics."));
        }
    }

    (*cbHandle->func_isr)( cbHandle->pParm1, cbHandle->parm2 );

    if (bStatsEnable)
    {
        rc = BTMR_ReadTimer_isr(intHandle->hTimer, &ulEnd);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Error reading timer for statistics."));
        }

        BINT_P_Stats_ComputeStats_isr( cbHandle, ulStart, ulEnd );
    }
#else
    BSTD_UNUSED (intHandle);
    (*cbHandle->func_isr)( cbHandle->pParm1, cbHandle->parm2 );
#endif /* BINT_STATS_ENABLE */
    cbHandle->count++;

    return;
}

static void BINT_P_ProcessStandardL2Reg_isr(BINT_Handle intHandle, BINT_P_L2Register *L2Reg, uint32_t intStatus)
{
    uint32_t L2BaseRegister = L2Reg->intMapEntry.L2RegOffset;
    BINT_P_L2Int *L2Handle;
    BREG_Handle regHandle = intHandle->regHandle;

    L2Reg->count++;
    for(L2Handle = BLST_S_FIRST(&L2Reg->intList) ; L2Handle ; L2Handle=BLST_S_NEXT(L2Handle, link)) {
        BINT_P_Callback *cbHandle;
        unsigned L2Bit = 1<<BCHP_INT_ID_GET_SHIFT( L2Handle->intId );
        if( (intStatus & L2Bit) && L2Handle->enableCount ) {
            intStatus &= ~L2Bit; /* clear status bits */
            L2Handle->count++; /* DumpInfo accounting */
            BREG_Write32_isr(regHandle, L2BaseRegister + BINT_P_STD_CLEAR, L2Bit); /* clear status */
            /* Call all callbacks that are enabled */
            for(cbHandle=BLST_S_FIRST(&L2Handle->callbackList); cbHandle ; cbHandle=BLST_S_NEXT(cbHandle, link)) {
                if( cbHandle->enabled ) {
                    BINT_P_ExecuteCallback_isr(intHandle, cbHandle);
                }
            }
            if(L2Handle->enableCount) { /* interrupt could be disabled by the interrupt handler */
                /* Shared L1 interrupts require that the L2 be masked in bcmdriver.ko, so BINT unmasks
                here to reverse that. For unshared L1's, this is harmless. */
                BREG_Write32_isr(regHandle, L2BaseRegister + BINT_P_STD_MASK_CLEAR, L2Bit);
            }
        }
    }
    return;
}

static void BINT_P_ProcessL2Reg_isr(BINT_Handle intHandle, BINT_P_L2Register *L2Reg)
{
    BREG_Handle regHandle = intHandle->regHandle;
    uint32_t intStatus=0;
    uint32_t L2BaseRegister = L2Reg->intMapEntry.L2RegOffset;
    BINT_P_Callback *cbHandle;
    BINT_P_L2Int *L2Handle;

    L2Handle=BLST_S_FIRST(&L2Reg->intList);

    if( L2Reg->intMapEntry.L2InvalidMask!=BINT_DONT_PROCESS_L2 ) { /* non-standard interrupts */
        unsigned L2Shift;

        intStatus = intHandle->settings.pReadStatus(regHandle, L2BaseRegister);
        intStatus &= ~L2Reg->intMapEntry.L2InvalidMask; /* only handle interrupts that are known */
        if(intStatus==0) {
            return; /* short-circuit loop over L2 interrupts */
        }
        L2Reg->count++;
        for( ; L2Handle ; L2Handle=BLST_S_NEXT(L2Handle, link)) {
            unsigned L2Bit;
            L2Shift = BCHP_INT_ID_GET_SHIFT( L2Handle->intId );
            L2Bit = 1<<L2Shift;

            /* find any interrupts that are triggered and enabled */
            if( (intStatus & L2Bit) && L2Handle->enableCount ) {
                intStatus &= ~L2Bit; /* clear status bits */
                L2Handle->count++; /* DumpInfo accounting */
                /* Since L2 interrupts are edge triggered they must be cleared before processing!! */
                intHandle->settings.pClearInt( regHandle, L2BaseRegister, L2Shift);
                /* Call all callbacks that are enabled */
                for(cbHandle=BLST_S_FIRST(&(L2Handle->callbackList)); cbHandle ; cbHandle=BLST_S_NEXT(cbHandle, link)) {
                    if( cbHandle->enabled ) {
                        BINT_P_ExecuteCallback_isr(intHandle, cbHandle);
                    }
                }
                if( L2Handle->enableCount ) { /* interrupt could be disabled by the interrupt handler */
                    /* Shared L1 interrupts require that the L2 be masked in bcmdriver.ko, so BINT unmasks
                    here to reverse that. For unshared L1's, this is harmless. */
                    intHandle->settings.pClearMask(regHandle, L2BaseRegister, L2Shift);
                }
            }
        }
        if( intStatus && L2Reg->weakMask) {
            for(L2Shift=0;L2Shift<32;L2Shift++) {
                if(intStatus&(1<<L2Shift)) {
                    intHandle->settings.pClearInt( regHandle, L2BaseRegister, L2Shift);
                }
            }
        }

    } else { /*  very special interrupts */
        /*
        If the L2InvalidMask is BINT_DONT_PROCESS_L2 that means that the interrupt interface
        does not process the L2 interrupts for this L1 shift.  Instead a separate L2
        isr routine should be installed as the callback for the specified L1
        shift that will handle this interrupt.
        */
        L2Reg->count++;
        L2Handle->count++; /* DumpInfo accounting */
        cbHandle=BLST_S_FIRST(&(L2Handle->callbackList));
        if(cbHandle) {
            BINT_P_ExecuteCallback_isr(intHandle, cbHandle);
        } else {
            BDBG_ERR(("L1 interrupt for not connected 'very special' %s", L2Reg->intMapEntry.L2Name));
        }
    }
    return;
}

/**
BINT_Isr_isr is the main inner loop of the entire refsw architecture.
Optimization of every line of code matters greatly.
**/
void BINT_Isr_isr( BINT_Handle intHandle, int L1Shift )
{
    BINT_P_L2Register *L2Reg;

#if 0
/* for performance, this ASSERT is compiled out. you can temporarily enable for debug. */
    BDBG_OBJECT_ASSERT(intHandle, BINT);
    BKNI_ASSERT_ISR_CONTEXT();
#endif
    for(L2Reg = BLST_S_FIRST(&intHandle->L1[L1Shift].L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
        uint32_t intStatus;
        if(!L2Reg->l3Root) {
            if(L2Reg->enableCount==0 && !L2Reg->weakMask ) { /* short-circular register access if there is no enabled interrupt handlers and no need to explicitly clears interrupt */
                continue;
            }
            if (L2Reg->standard) {

                /* Standard registers can be handled internal to bint.c which results in dramatic performance improvement.
                Each chip must set BINT_IS_STANDARD as appropriate. */

                intStatus = BREG_Read32_isr(intHandle->regHandle, L2Reg->intMapEntry.L2RegOffset); /* read status */
                intStatus &= ~L2Reg->intMapEntry.L2InvalidMask; /* only handle interrupts that are known */
                if(intStatus==0) {
                    continue; /* short-circuit loop over L2 interrupts */
                }
                BINT_P_ProcessStandardL2Reg_isr(intHandle, L2Reg, intStatus);
            } else {
                BINT_P_ProcessL2Reg_isr(intHandle, L2Reg);
            }
        } else {
            const BINT_P_L2Aggregator *L2Aggregator = BLST_S_FIRST(&L2Reg->aggregatorList);
            intStatus = BREG_Read32_isr(intHandle->regHandle, L2Reg->intMapEntry.L2RegOffset); /* read status */
            intStatus &= ~L2Reg->intMapEntry.L2InvalidMask; /* only handle interrupts that are known */
            if(intStatus==0) {
                continue; /* short-circuit loop over L2 interrupts */
            }
            BINT_P_ProcessStandardL2Reg_isr(intHandle, L2Reg, intStatus);
            for(; L2Aggregator; L2Aggregator = BLST_S_NEXT(L2Aggregator, link)) {
                unsigned L2Bit = 1<<L2Aggregator->L2Shift;
                if(intStatus && L2Bit) {
                    BINT_P_L2Register *L3Reg;
                    for(L3Reg = BLST_S_FIRST(&L2Aggregator->L2RegList); L3Reg ; L3Reg = BLST_S_NEXT(L3Reg, link)) {
                        uint32_t intL3Status;
                        intL3Status = BREG_Read32_isr(intHandle->regHandle, L3Reg->intMapEntry.L2RegOffset); /* read status */
                        intL3Status &= ~L3Reg->intMapEntry.L2InvalidMask; /* only handle interrupts that are known */
                        if(intL3Status) {
                            BINT_P_ProcessStandardL2Reg_isr(intHandle, L3Reg, intL3Status);
                        }
                    }
                }
            }
        }
    }
    return;
}

static BINT_P_L2Register *
BINT_P_FindL2Reg(const struct BINT_P_L2RegisterList *L2RegList, BINT_Id intId)
{
    BINT_P_L2Register *L2Reg;
    uint32_t L2Shift = BCHP_INT_ID_GET_SHIFT(intId);

    for(L2Reg = BLST_S_FIRST(L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
        /*
        We must find the matching L2 register offset and ensure that the specified L2 interrupt is actually
        handled by the specified L1->L2 mapping.  This is because some wacky L2 interrupt registers actually map to
        multiple L1 interrupts (i.e. 8 bits of the L2 register map to L1=X, while the other bits map to L1=Y).
        This also properly handles multiple L2 interrupt registers that are mapped to a single L1 bit.

        Also, if the L2InvalidMask is BINT_DONT_PROCESS_L2 for this register it means that the interrupt interface
        does not handle the L2 interrupts.  Rather it should just create the first callback associated with that
        L1 interrupt shift.
        */
        if( BCHP_INT_ID_GET_REG(intId) != L2Reg->intMapEntry.L2RegOffset) {
            continue;
        }
        if(L2Reg->l3Root) {
            continue;
        }
        if((L2Reg->intMapEntry.L2InvalidMask & (1<<L2Shift)) == 0 || L2Reg->intMapEntry.L2InvalidMask == BINT_DONT_PROCESS_L2) {
            break;
        }
    }
    return L2Reg;
}

#if BDBG_DEBUG_BUILD
#undef BINT_CreateCallback
BERR_Code BINT_CreateCallback( BINT_CallbackHandle *pCbHandle, BINT_Handle intHandle, BINT_Id intId, BINT_CallbackFunc func_isr, void * pParm1, int parm2 )
{
    BDBG_WRN(("BINT_CallbackFunc shall be never called in the debug builds"));
    return BINT_P_CreateCallback_Tag(pCbHandle, intHandle, intId, func_isr, pParm1, parm2, "");
}
BERR_Code BINT_P_CreateCallback_Tag( BINT_CallbackHandle *pCbHandle, BINT_Handle intHandle, BINT_Id intId, BINT_CallbackFunc func_isr, void * pParm1, int parm2, const char *callbackName)
#else
BERR_Code BINT_P_CreateCallback_Tag( BINT_CallbackHandle *pCbHandle, BINT_Handle intHandle, BINT_Id intId, BINT_CallbackFunc func_isr, void * pParm1, int parm2)
{
    return BINT_CreateCallback(pCbHandle, intHandle, intId, func_isr, pParm1, parm2);
}
BERR_Code BINT_CreateCallback( BINT_CallbackHandle *pCbHandle, BINT_Handle intHandle, BINT_Id intId, BINT_CallbackFunc func_isr, void * pParm1, int parm2 )
#endif
{
    uint32_t L2Shift = BCHP_INT_ID_GET_SHIFT(intId);
    int L1Shift;
    BINT_P_L2Int *L2Handle;
    BERR_Code rc;
    BINT_P_L2Register *L2Reg=NULL;
    BINT_CallbackHandle cbHandle;

    BDBG_OBJECT_ASSERT(intHandle, BINT);
    BINT_LOCK(intHandle);

    for(L1Shift = 0; L1Shift<BINT_P_L1_SIZE; L1Shift++) {

        L2Reg = BINT_P_FindL2Reg( &intHandle->L1[L1Shift].L2RegList, intId);
        if(L2Reg!=NULL) {
            goto match_found;
        }

        for(L2Reg = BLST_S_FIRST(&intHandle->L1[L1Shift].L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
            const BINT_P_L2Aggregator *L2Aggregator;
            for(L2Aggregator = BLST_S_FIRST(&L2Reg->aggregatorList); L2Aggregator; L2Aggregator = BLST_S_NEXT(L2Aggregator, link)) {
                BINT_P_L2Register *L3Reg=BINT_P_FindL2Reg( &L2Aggregator->L2RegList, intId);
                if(L3Reg) {
                    L2Reg = L3Reg;
                    goto match_found;
                }
            }
        }
    }
match_found:

    if(L2Reg == NULL) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done; }

    /* Determine if we need to allocate a new L2 interrupt context */
    for( L2Handle=BLST_S_FIRST(&L2Reg->intList) ; L2Handle ; L2Handle=BLST_S_NEXT(L2Handle, link)) {
        if( L2Handle->intId == intId ) {
            break;
        }
    }
    if( L2Handle == NULL )
    {
        /* We need to create a new L2 element to manage this interrupt bit */
        L2Handle = BKNI_Malloc( sizeof(*L2Handle) );
        if( L2Handle == NULL ) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done; }

        BLST_S_INIT( &L2Handle->callbackList);
        L2Handle->enableCount = 0;
        L2Handle->intId = intId;
        L2Handle->L2Reg = L2Reg;
        L2Handle->count = 0;
        intHandle->numInts++;

        BKNI_EnterCriticalSection();
        BLST_S_INSERT_HEAD(&L2Reg->intList, L2Handle, link);
        BKNI_LeaveCriticalSection();

        /* clear previous status */
        BINT_P_ClearInt_isrsafe(intHandle, L2Reg, L2Shift);
    }
    if(L2Reg->intMapEntry.L2InvalidMask == BINT_DONT_PROCESS_L2) {
        if( BLST_S_FIRST(&L2Handle->callbackList)!=NULL) {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER); /* only one callback could be enabled for 'very special' interrupts */
            goto done;
        }
        L2Reg->enableCount = 1; /* set enable count to 1, to allow handling of such interrupt */
    }

    cbHandle = BKNI_Malloc( sizeof(*cbHandle) );
    if( cbHandle == NULL ) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done; }

    BKNI_Memset( cbHandle, 0, sizeof(*cbHandle) );
    BDBG_OBJECT_SET(cbHandle, BINT_Callback);

    cbHandle->func_isr = func_isr;
    cbHandle->pParm1 = pParm1;
    cbHandle->parm2 = parm2;
    cbHandle->L2Handle = L2Handle;

    cbHandle->count = 0;
#if BDBG_DEBUG_BUILD
    cbHandle->callbackName = callbackName;
#else
    cbHandle->callbackName = NULL;
#endif

    intHandle->callbackCount++;

    cbHandle->StatInfo.ulTimeMin = UINT32_MAX;
    cbHandle->StatInfo.ulTimeMax = 0;
    cbHandle->StatInfo.ulTimeAvg = 0;
    cbHandle->StatInfo.ulCbHitCount = 0;
    cbHandle->StatInfo.ulTimeStampStartIdx = 0;

    BKNI_Memset(cbHandle->StatInfo.aulTimeStamp, 0, sizeof(uint32_t) * BINT_P_STATS_RECENT_CB_HIT_COUNT);

    /* set up default bins */
    cbHandle->StatInfo.ulActiveBins = BINT_P_STATS_DEFAULT_BINS_NUM;
    BKNI_Memcpy(cbHandle->StatInfo.aBinInfo, g_aDefaultBins, sizeof(g_aDefaultBins));

    cbHandle->StatInfo.bDefaultBins = true;

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&L2Handle->callbackList, cbHandle, link);
    BLST_S_INSERT_HEAD(&intHandle->allCbList, cbHandle, allCbLink);
    BKNI_LeaveCriticalSection();

    *pCbHandle = cbHandle;
    rc = BERR_SUCCESS;

done:
    BINT_UNLOCK(intHandle);
    return rc;
}

BERR_Code BINT_DestroyCallback( BINT_CallbackHandle cbHandle )
{
    BINT_P_Context *intHandle;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    intHandle = cbHandle->L2Handle->L2Reg->intHandle;
    BINT_LOCK(intHandle);

    if( cbHandle->enabled == true )
    {
        BINT_DisableCallback(cbHandle );
    }

    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&cbHandle->L2Handle->callbackList, cbHandle, BINT_P_Callback, link);
    BLST_S_REMOVE(&intHandle->allCbList, cbHandle, BINT_P_Callback, allCbLink);
    BKNI_LeaveCriticalSection();


    BDBG_OBJECT_DESTROY(cbHandle, BINT_Callback);
    BKNI_Free( cbHandle );
    intHandle->callbackCount--;
    BINT_UNLOCK(intHandle);

    return BERR_SUCCESS;
}

BERR_Code BINT_EnableCallback( BINT_CallbackHandle cbHandle )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    BKNI_EnterCriticalSection();
    rc = BINT_EnableCallback_isr( cbHandle );
    BKNI_LeaveCriticalSection();

    return rc;
}

BERR_Code BINT_EnableCallback_isr( BINT_CallbackHandle cbHandle )
{
    BINT_P_Context *intHandle;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);
    BKNI_ASSERT_ISR_CONTEXT();

    /* If enabled, we are already done... */
    if( cbHandle->enabled )
    {
        return BERR_SUCCESS;
    }

    intHandle = cbHandle->L2Handle->L2Reg->intHandle;

    /* Flag callback as enabled so that we execute it if we get an interrupt immediately after it is unmasked */
    cbHandle->enabled = true;

    cbHandle->L2Handle->enableCount++;
    cbHandle->L2Handle->L2Reg->enableCount ++;
    if( cbHandle->L2Handle->enableCount == 1 )
    {
        BINT_P_ClearMask_isrsafe(intHandle, cbHandle->L2Handle->L2Reg, BCHP_INT_ID_GET_SHIFT(cbHandle->L2Handle->intId));
    }

    return BERR_SUCCESS;
}

BERR_Code BINT_DisableCallback( BINT_CallbackHandle cbHandle )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);
    BKNI_EnterCriticalSection();
    rc = BINT_DisableCallback_isr( cbHandle );
    BKNI_LeaveCriticalSection();

    return rc;
}

BERR_Code BINT_DisableCallback_isr( BINT_CallbackHandle cbHandle )
{
    BINT_P_Context *intHandle;
    BINT_P_L2Int *L2Handle;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);
    BKNI_ASSERT_ISR_CONTEXT();

    /* If not enabled, we are already done... */
    if( cbHandle->enabled == false )
    {
        return BERR_SUCCESS;
    }

    L2Handle = cbHandle->L2Handle;
    intHandle = L2Handle->L2Reg->intHandle;

    L2Handle->enableCount--;
    L2Handle->L2Reg->enableCount --;
    BDBG_ASSERT(L2Handle->enableCount >=0);
    BDBG_ASSERT(L2Handle->L2Reg->enableCount >=0);
    if( cbHandle->L2Handle->enableCount == 0 )
    {
        BINT_P_SetMask_isrsafe(intHandle, L2Handle->L2Reg->standard, BCHP_INT_ID_GET_REG(L2Handle->intId), BCHP_INT_ID_GET_SHIFT(L2Handle->intId) );
    }

    /* Flag callback as disabled only after it is masked so that we can execute if we get an interrupt */
    cbHandle->enabled = false;

    return BERR_SUCCESS;
}

BERR_Code BINT_ClearCallback( BINT_CallbackHandle cbHandle )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    BKNI_EnterCriticalSection();
    rc = BINT_ClearCallback_isr( cbHandle );
    BKNI_LeaveCriticalSection();

    return rc;
}

BERR_Code BINT_ClearCallback_isr( BINT_CallbackHandle cbHandle )
{
    BERR_Code rc = BERR_SUCCESS;
    BINT_P_Context *intHandle;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);
    BKNI_ASSERT_ISR_CONTEXT();

    intHandle = cbHandle->L2Handle->L2Reg->intHandle;

    BINT_P_ClearInt_isrsafe(intHandle, cbHandle->L2Handle->L2Reg, BCHP_INT_ID_GET_SHIFT(cbHandle->L2Handle->intId) );

    return rc;
}

BERR_Code BINT_TriggerInterruptByHandle_isrsafe( BINT_CallbackHandle cbHandle )
{
    BINT_P_Context *intHandle;
    const BINT_P_L2Register *L2Reg;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    L2Reg = cbHandle->L2Handle->L2Reg;
    intHandle = L2Reg->intHandle;

    if(L2Reg->standard) {
        BREG_Write32_isr(intHandle->regHandle, BCHP_INT_ID_GET_REG(cbHandle->L2Handle->intId)+BINT_P_STD_SET, 1<<BCHP_INT_ID_GET_SHIFT(cbHandle->L2Handle->intId) );
    } else if(intHandle->settings.pSetInt != NULL) {
        intHandle->settings.pSetInt( intHandle->regHandle, BCHP_INT_ID_GET_REG(cbHandle->L2Handle->intId), BCHP_INT_ID_GET_SHIFT(cbHandle->L2Handle->intId) );
    } else {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

void BINT_GetL1BitMask(BINT_Handle intHandle, uint32_t *BitMask)
{
    int i;


    BDBG_OBJECT_ASSERT(intHandle, BINT);

    for(i=0;i<BINT_MAX_INTC_SIZE;i++)
    {
        BitMask[i]=0;
    }

    for(i=0; i<BINT_P_L1_SIZE; i++ ) {
        const BINT_P_L2Register *L2Reg = BLST_S_FIRST(&intHandle->L1[i].L2RegList);
        unsigned L1Shift;
        if(L2Reg==NULL) {
            continue;
        }
        L1Shift = L2Reg->intMapEntry.L1Shift;
        if(L1Shift >= 128)
        {
            BitMask[4] |= 1ul<<(L1Shift-128);
        }
        else if(L1Shift >= 96 && L1Shift < 128)
        {
            BitMask[3] |= 1ul<<(L1Shift-96);
        }
        else if(L1Shift >= 64 && L1Shift < 96)
        {
            BitMask[2] |= 1ul<<(L1Shift-64);
        }
        else if( L1Shift >= 32 && L1Shift < 64 )
        {
            BitMask[1] |= 1ul<<(L1Shift-32);
        }
        else
        {
            BitMask[0] |= 1ul<<L1Shift;
        }
    }
}

BERR_Code BINT_Stats_AddBin( BINT_CallbackHandle cbHandle, uint32_t ulRangeMin, uint32_t ulRangeMax )
{
    uint32_t ulActiveBins;
    BINT_Stats_CallbackBin *pCurBinInfo = NULL;
    BINT_Stats_CallbackBin aTmpBinInfo[BINT_P_STATS_BIN_MAX];
    uint16_t i = 0;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    if ( ulRangeMin > ulRangeMax)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( cbHandle->StatInfo.bDefaultBins )
    {
        BINT_Stats_DestroyBins(cbHandle);
        cbHandle->StatInfo.bDefaultBins = false;
    }

    ulActiveBins = cbHandle->StatInfo.ulActiveBins;

    if ( ulActiveBins == BINT_P_STATS_BIN_MAX )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i = 0; i <= ulActiveBins; i++)
    {
        pCurBinInfo = &(cbHandle->StatInfo.aBinInfo[i]);

        /* check for range overlap with current bin */
        if (( ulActiveBins > 0 ) &&
            ((( ulRangeMin >= pCurBinInfo->ulBinRangeMin ) &&
              ( ulRangeMin <= pCurBinInfo->ulBinRangeMax )) ||
             (( ulRangeMax >= pCurBinInfo->ulBinRangeMin ) &&
              ( ulRangeMax <= pCurBinInfo->ulBinRangeMax ))))
        {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* check for bin insertion here */
        if ( ulRangeMax < pCurBinInfo->ulBinRangeMin )
        {
            /* shift bins for insertion */
            uint32_t ulBinCopyNum = ulActiveBins - i;
            BKNI_Memcpy(aTmpBinInfo, pCurBinInfo, sizeof(BINT_Stats_CallbackBin) * ulBinCopyNum);
            BKNI_Memcpy(pCurBinInfo + 1, aTmpBinInfo, sizeof(BINT_Stats_CallbackBin) * ulBinCopyNum);
        }

        /* set bin */
        if (( ulRangeMax < pCurBinInfo->ulBinRangeMin ) || (i == ulActiveBins))
        {
            pCurBinInfo->ulBinRangeMin = ulRangeMin;
            pCurBinInfo->ulBinRangeMax = ulRangeMax;
            pCurBinInfo->ulBinHitCount = 0;

            cbHandle->StatInfo.ulActiveBins++;
            break;
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BINT_Stats_DestroyBins( BINT_CallbackHandle cbHandle )
{
    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    cbHandle->StatInfo.ulActiveBins = 0;

    BKNI_Memset(cbHandle->StatInfo.aBinInfo, 0,
                sizeof(BINT_Stats_CallbackBin) * BINT_P_STATS_BIN_MAX);

    return BERR_SUCCESS;
}

BERR_Code BINT_Stats_Get( BINT_CallbackHandle cbHandle, BINT_Stats_CallbackStats **ppCbStats )
{
    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

#ifndef BINT_STATS_ENABLE
    BDBG_WRN(("Stats tracking not enabled in compile."));
#endif /* BINT_STATS_ENABLE */

    *ppCbStats = &(cbHandle->StatInfo);

    return BERR_SUCCESS;
}

BERR_Code BINT_Stats_Reset( BINT_CallbackHandle cbHandle )
{
    uint16_t i;
    BINT_Stats_CallbackStats *pStatInfo;

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);

    pStatInfo = &cbHandle->StatInfo;
    BKNI_EnterCriticalSection();
    pStatInfo->ulTimeMin = UINT32_MAX;
    pStatInfo->ulTimeMax = 0;
    pStatInfo->ulTimeAvg = 0;
    pStatInfo->ulCbHitCount = 0;

    for (i = 0; i < pStatInfo->ulActiveBins; i++)
    {
        pStatInfo->aBinInfo[i].ulBinHitCount = 0;
    }
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

BERR_Code BINT_Stats_Enable( BINT_Handle intHandle, BTMR_Handle hTmrHandle )
{
    BERR_Code rc = BERR_SUCCESS;
    BTMR_TimerHandle hTimer = NULL;

    BTMR_Settings stSettings = { BTMR_Type_eSharedFreeRun,
                                 NULL,
                                 NULL,
                                 0,
                                 false };

    BDBG_OBJECT_ASSERT(intHandle, BINT);

    if ( intHandle->bStatsEnable == true )
    {
        return BERR_TRACE(BINT_STATS_ERR_ALREADY_ENABLED);
    }

    rc = BTMR_CreateTimer( hTmrHandle, &hTimer, &stSettings );
    if (rc != BERR_SUCCESS)
    {
        return rc;
    }

    intHandle->hTimer = hTimer;
    intHandle->bStatsEnable = true;

    return BERR_SUCCESS;
}

BERR_Code BINT_Stats_Disable( BINT_Handle intHandle )
{
    BDBG_OBJECT_ASSERT(intHandle, BINT);

    if ( intHandle->bStatsEnable == false )
    {
        return BERR_TRACE(BINT_STATS_ERR_ALREADY_DISABLED);
    }

    BDBG_ASSERT( intHandle->hTimer != NULL );
    BTMR_DestroyTimer(intHandle->hTimer);

    intHandle->bStatsEnable = false;

    return BERR_SUCCESS;
}

static BINT_CallbackHandle BINT_P_GetCallbackByNumber_locked(BINT_Handle intHandle, unsigned number)
{
    unsigned i;
    BINT_CallbackHandle callback;

    BDBG_OBJECT_ASSERT(intHandle, BINT);
    for(callback=BLST_S_FIRST(&intHandle->allCbList),i=0;i<number;i++) {
        if(callback==NULL) {
            break;
        }
        callback = BLST_S_NEXT(callback, allCbLink);
    }
    return callback;
}

BERR_Code BINT_Stats_AddBinByNumber(BINT_Handle  intHandle, unsigned callbackNumber, uint32_t ulRangeMin, uint32_t ulRangeMax)
{
    BINT_CallbackHandle callback;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(intHandle, BINT);

    BINT_LOCK(intHandle);
    callback = BINT_P_GetCallbackByNumber_locked(intHandle, callbackNumber);
    if(callback) {
        rc = BINT_Stats_AddBin(callback, ulRangeMin, ulRangeMax);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);}
    } else {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BINT_UNLOCK(intHandle);
    return rc;
}

BERR_Code BINT_Stats_ResetByNumber( BINT_Handle  intHandle, unsigned callbackNumber)
{
    BINT_CallbackHandle callback;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(intHandle, BINT);

    BINT_LOCK(intHandle);
    callback = BINT_P_GetCallbackByNumber_locked(intHandle, callbackNumber);
    if(callback) {
        rc = BINT_Stats_Reset(callback);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);}
    } else {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BINT_UNLOCK(intHandle);
    return rc;
}

#ifdef BINT_STATS_ENABLE
/* returns elapsed time in microseconds */
static uint32_t BINT_P_GetElapsedTime_isr( uint32_t ulTimerStart, uint32_t ulTimerEnd )
{
    uint32_t ulTimerMax;
    uint32_t ulTimerElapsed;

    ulTimerMax = BTMR_ReadTimerMax_isrsafe();

    if (ulTimerEnd < ulTimerStart)
    {
        ulTimerElapsed = ((ulTimerMax - ulTimerStart) + ulTimerEnd);
    }
    else
    {
        ulTimerElapsed = (ulTimerEnd - ulTimerStart);
    }

    return ulTimerElapsed;
}

static BERR_Code BINT_P_Stats_ComputeStats_isr( BINT_CallbackHandle cbHandle, uint32_t ulStart, uint32_t ulEnd )
{
    uint16_t i;
    uint32_t ulSampleNum = BINT_P_STATS_SAMPLE_MAX;
    uint32_t ulElapsedTime = BINT_P_GetElapsedTime_isr( ulStart, ulEnd );
    BINT_Stats_CallbackStats *pStatInfo = &(cbHandle->StatInfo);

    BDBG_OBJECT_ASSERT(cbHandle, BINT_Callback);
    pStatInfo->ulCbHitCount++;


    /* calculate min, max and average times */
    if ( ulElapsedTime < pStatInfo->ulTimeMin)
    {
        pStatInfo->ulTimeMin = ulElapsedTime;
    }

    if ( ulElapsedTime > pStatInfo->ulTimeMax)
    {
        pStatInfo->ulTimeMax = ulElapsedTime;
    }

    if (ulSampleNum > pStatInfo->ulCbHitCount)
    {
        ulSampleNum = pStatInfo->ulCbHitCount;
    }

    pStatInfo->ulTimeAvg = ((pStatInfo->ulTimeAvg * (ulSampleNum - 1)) +
                                  ulElapsedTime) / ulSampleNum;
    pStatInfo->aulTimeStamp[pStatInfo->ulTimeStampStartIdx] = ulStart;

    /* check for callbacks that take too long */
    if (ulElapsedTime > BINT_P_STATS_EXECUTION_TIME_MAX_THRESHOLD)
    {
        BDBG_WRN(("BINT_Isr(%s) took %d msec", cbHandle->L2Handle->L2Reg->intMapEntry.L2Name, ulElapsedTime/1000));
    }

    /* check for runaway interrupts */
    if (ulSampleNum >= BINT_P_STATS_RECENT_CB_HIT_COUNT)
    {
/* Commenting out this code because some interrupts fire faster than this for a period of time. */
#if 0
        uint32_t ulTotalPeriod, ulAvgPeriod;
        uint32_t ulTimeStampEndIdx;

        if (pStatInfo->ulTimeStampStartIdx == BINT_P_STATS_RECENT_CB_HIT_COUNT - 1)
        {
            ulTimeStampEndIdx = 0;
        }
        else
        {
            ulTimeStampEndIdx = pStatInfo->ulTimeStampStartIdx + 1;
        }

        ulTotalPeriod = BINT_P_GetElapsedTime_isr(pStatInfo->aulTimeStamp[ulTimeStampEndIdx],
                                              pStatInfo->aulTimeStamp[pStatInfo->ulTimeStampStartIdx]);

        ulAvgPeriod = ulTotalPeriod / BINT_P_STATS_RECENT_CB_HIT_COUNT;

        if (ulAvgPeriod < BINT_P_STATS_AVG_PERIOD_MIN_THRESHOLD)
        {
            BDBG_WRN(("BINT_Isr(%s) overflow, %d msec between hits",
            cbHandle->L2Handle->intHandle->settings.pIntMap[cbHandle->L2Handle->intMapIndex].L2Name,
            ulAvgPeriod/1000));
        }
#endif

        pStatInfo->ulTimeStampStartIdx++;
        pStatInfo->ulTimeStampStartIdx = pStatInfo->ulTimeStampStartIdx % BINT_P_STATS_RECENT_CB_HIT_COUNT;
    }

    /* mark bin according to elapsed time */
    for (i = 0; i < pStatInfo->ulActiveBins; i++)
    {
        if ((ulElapsedTime >= pStatInfo->aBinInfo[i].ulBinRangeMin) &&
            (ulElapsedTime <= pStatInfo->aBinInfo[i].ulBinRangeMax))
        {
            pStatInfo->aBinInfo[i].ulBinHitCount++;
            break;
        }
    }

    return BERR_SUCCESS;
}

static  BINT_CallbackHandle BINT_P_GetCallbackFirst( BINT_Handle intHandle )
{
    return BLST_S_FIRST(&(intHandle->allCbList));
}

static BINT_CallbackHandle BINT_P_GetCallbackNext( BINT_CallbackHandle cbHandle )
{
    return BLST_S_NEXT(cbHandle, allCbLink);
}

static void BINT_P_GetInterruptId( BINT_CallbackHandle cbHandle, BINT_Id *pIntId )
{
    *pIntId = cbHandle->L2Handle->intId;
    return;
}

static void BINT_P_GetCallbackStatus( BINT_CallbackHandle cbHandle, bool *pbEnabled)
{
    *pbEnabled = cbHandle->enabled;
    return;
}


BERR_Code BINT_Stats_Dump
	( BINT_Handle          intHandle )
{
	BINT_CallbackHandle cbHandle;
	BINT_Stats_CallbackStats *pCbStats = NULL;
	BINT_Id intId = 0;
	int iCallbackNum = 1;
	int i = 0;
	bool bCallbackEnabled = false;
	bool bLastDumpStats = false;
	bool bFirstLine = true;

	if (intHandle == NULL)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	cbHandle = BINT_P_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		int iDumpBins = 0;
		bool bDumpStats = false;

		BINT_Stats_Get(cbHandle, &pCbStats);

		BINT_P_GetInterruptId(cbHandle, &intId);
		BINT_P_GetCallbackStatus(cbHandle, &bCallbackEnabled);

		if (bCallbackEnabled ||
			!pCbStats->bDefaultBins ||
			(pCbStats->ulCbHitCount != 0))
		{
			bDumpStats = true;
		}

		if (bDumpStats || bLastDumpStats || bFirstLine)
		{
			BKNI_Printf("-------------------------------------------------------------------------------\n");
			bFirstLine = false;
		}

		BKNI_Printf(" Callback %-2d %s -- IntID: 0x%08x  L2Reg: 0x%08x  L2Shift: %u\n",
			iCallbackNum++,
			(bCallbackEnabled) ? "(on) ": "(off)",
			intId,
			BCHP_INT_ID_GET_REG(intId),
			BCHP_INT_ID_GET_SHIFT(intId));

		if (bDumpStats)
		{
			BKNI_Printf("                      Min: %-6u Max: %-6u  Avg: %-6u  CbHitCount : %-6u\n",
				(pCbStats->ulTimeMin == UINT32_MAX) ? 0: pCbStats->ulTimeMin,
				pCbStats->ulTimeMax,
				pCbStats->ulTimeAvg,
				pCbStats->ulCbHitCount);
		}

		/* print bins */
		for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
		{
			if (!pCbStats->bDefaultBins ||
				(pCbStats->aBinInfo[i].ulBinHitCount != 0))
			{
				iDumpBins++;

				BKNI_Printf("  %sBin %-2d -- Range Min: %-6u  Range Max: %-6u  BinHitCount: %-6u\n",
					pCbStats->bDefaultBins ? "(Default) ": "(User)    ",
					i + 1,
					pCbStats->aBinInfo[i].ulBinRangeMin,
					pCbStats->aBinInfo[i].ulBinRangeMax,
					pCbStats->aBinInfo[i].ulBinHitCount);
			}
		}

		bLastDumpStats = bDumpStats;
		cbHandle = BINT_P_GetCallbackNext( cbHandle );
	}

	BKNI_Printf("\n");

	return BERR_SUCCESS;
}

static void BINT_P_Stats_DumpLabel
	( BINT_Stats_CallbackStats *pCbStats )
{
	int i = 0;

	BKNI_Printf("\n");
	BKNI_Printf("Callback #,Status,IntId,L2Reg,L2Shift,");
	BKNI_Printf("Min,Max,Avg,CbHitCount,");

	for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
	{
		BKNI_Printf("%d-%d us,",
			pCbStats->aBinInfo[i].ulBinRangeMin,
			pCbStats->aBinInfo[i].ulBinRangeMax);
	}

	BKNI_Printf("\n");
}

void BINT_P_Stats_DumpCbData
	( BINT_CallbackHandle cbHandle, int iCallbackNum )
{
	BINT_Stats_CallbackStats *pCbStats = NULL;
	BINT_Id intId = 0;
	bool bCallbackEnabled = false;
	int i = 0;

	BINT_Stats_Get(cbHandle, &pCbStats);
	BINT_P_GetInterruptId(cbHandle, &intId);
	BINT_P_GetCallbackStatus(cbHandle, &bCallbackEnabled);

	BKNI_Printf("Callback %d,%s,0x%08x,0x%08x,%u,",
		iCallbackNum,
		(bCallbackEnabled) ? "(on) ": "(off)",
		intId,
		BCHP_INT_ID_GET_REG(intId),
		BCHP_INT_ID_GET_SHIFT(intId));

	BKNI_Printf("%u,%u,%u,%u,",
		(pCbStats->ulTimeMin == UINT32_MAX) ? 0: pCbStats->ulTimeMin,
		pCbStats->ulTimeMax,
		pCbStats->ulTimeAvg,
		pCbStats->ulCbHitCount);

	/* print bins */
	for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
	{
		if (pCbStats->aBinInfo[i].ulBinHitCount != 0)
		{
			BKNI_Printf("%u",  pCbStats->aBinInfo[i].ulBinHitCount);
		}

		BKNI_Printf(",");
	}

	BKNI_Printf("\n");
}

BERR_Code BINT_Stats_DumpData
	( BINT_Handle          intHandle )
{
	BINT_CallbackHandle cbHandle;
	BINT_Stats_CallbackStats *pCbStats = NULL;
	int iCallbackNum = 1;
	bool bFirstDump = true;

	if (intHandle == NULL)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* print stats using default bins */
	cbHandle = BINT_P_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		BINT_Stats_Get(cbHandle, &pCbStats);

		if (pCbStats->bDefaultBins)
		{
			/* print out label */
			if (bFirstDump)
			{
				BINT_P_Stats_DumpLabel(pCbStats);
				bFirstDump = false;
			}

			BINT_P_Stats_DumpCbData(cbHandle, iCallbackNum);
		}

		iCallbackNum++;
		cbHandle = BINT_P_GetCallbackNext( cbHandle );
	}

	/* print stats using user bins */
	iCallbackNum = 1;
	cbHandle = BINT_P_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		BINT_Stats_Get(cbHandle, &pCbStats);

		if (!pCbStats->bDefaultBins)
		{
			BINT_P_Stats_DumpLabel(pCbStats);
			BINT_P_Stats_DumpCbData(cbHandle, iCallbackNum);
		}

		iCallbackNum++;
		cbHandle = BINT_P_GetCallbackNext( cbHandle );
	}

	return BERR_SUCCESS;
}
#endif /* #ifdef BINT_STATS_ENABLE */

struct BINT_P_DumpInfoState {
    bool int_head;
};

static void BINT_P_DumpInfo_L2RegList(BINT_Handle intHandle, const struct BINT_P_L2RegisterList *L2RegList, unsigned L1Shift, struct BINT_P_DumpInfoState *state)
{
    BINT_P_L2Register *L2Reg;

#if BDBG_DEBUG_BUILD
#else
    BSTD_UNUSED (intHandle);
    BSTD_UNUSED (L1Shift);
#endif

    for(L2Reg = BLST_S_FIRST(L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
        BINT_P_L2Int *L2Handle;
        bool l1_head = false;

        if(L2Reg->count==0) {
            continue;
        }
        if (!state->int_head) {
            BDBG_MSG(("------[%s dump]--------", intHandle->settings.name?intHandle->settings.name:"XXX"));
            state->int_head=true;
        }
        if(!l1_head) {
            BDBG_MSG((" %#x:%s %u", (unsigned)L1Shift, L2Reg->intMapEntry.L2Name, L2Reg->count));
            l1_head=true;
        }


        for(l1_head=false,L2Handle=BLST_S_FIRST(&L2Reg->intList); L2Handle ; L2Handle=BLST_S_NEXT(L2Handle, link)) {
            bool l2_head;
            BINT_CallbackHandle callback;

            if (!L2Handle->count) {
                continue;
            }
            for(l2_head=false,callback=BLST_S_FIRST(&L2Handle->callbackList); callback; callback=BLST_S_NEXT(callback, link)) {
                if (!callback->count) {
                    continue;
                }
                if (!l2_head) {
                    BDBG_MSG(("   %#x:%u %s[%p]:(%p,%d) %u %s", BCHP_INT_ID_GET_REG(L2Handle->intId), BCHP_INT_ID_GET_SHIFT(L2Handle->intId), callback->callbackName, (void *)(unsigned long)callback->func_isr, (void *)callback->pParm1, callback->parm2, callback->count, callback->enabled?"":"disabled"));
                    l2_head=true;
                } else {
                    BDBG_MSG(("   >>> %s[%p]:(%p,%d) %u %s", callback->callbackName, (void *)(unsigned long)callback->func_isr, callback->pParm1, callback->parm2, callback->count, callback->enabled?"":"disabled"));
                }
#if 0
#ifdef BINT_STATS_ENABLE
                if (intHandle->bStatsEnable) {
                    BDBG_MSG(("     elapsed: min %d, max %d, avg %d (usec)",
                        callback->StatInfo.ulTimeMin, callback->StatInfo.ulTimeMax, callback->StatInfo.ulTimeAvg));
                    callback->StatInfo.ulTimeMin = 0;
                    callback->StatInfo.ulTimeMax = 0;
                    callback->StatInfo.ulTimeAvg = 0;
                    callback->StatInfo.ulCbHitCount = 0;
                }
#endif
#endif
                callback->count=0;
            }
            L2Handle->count=0;
        }
        L2Reg->count = 0;
    }
}

void BINT_DumpInfo(BINT_Handle intHandle)
{
    struct BINT_P_DumpInfoState state;
    unsigned L1Shift;

    BDBG_OBJECT_ASSERT(intHandle, BINT);
    BINT_LOCK(intHandle);

    state.int_head = false;
    for(L1Shift=0;L1Shift<BINT_P_L1_SIZE;L1Shift++) {
        const BINT_P_L2Register *L2Reg;
        BINT_P_DumpInfo_L2RegList(intHandle, &intHandle->L1[L1Shift].L2RegList, L1Shift, &state);
        for(L2Reg = BLST_S_FIRST(&intHandle->L1[L1Shift].L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
            const BINT_P_L2Aggregator *L2Aggregator;
            for(L2Aggregator = BLST_S_FIRST(&L2Reg->aggregatorList); L2Aggregator; L2Aggregator = BLST_S_NEXT(L2Aggregator, link)) {
                BINT_P_DumpInfo_L2RegList(intHandle, &L2Aggregator->L2RegList, L1Shift, &state);
            }
        }
    }
    BINT_UNLOCK(intHandle);
    return;
}

static bool BINT_P_ApplyStateL2Reg_isr(BINT_Handle intHandle, const struct BINT_P_L2RegisterList *L2RegList, uint32_t L2RegOffset)
{
    const BINT_P_L2Register *L2Reg;
    bool match = false;

    for(L2Reg = BLST_S_FIRST(L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link))
    {
        BINT_P_L2Int *L2Handle;
        if(L2Reg->intMapEntry.L2RegOffset != L2RegOffset)
        {
            continue;
        }
        match = true;
        for( L2Handle=BLST_S_FIRST(&L2Reg->intList); L2Handle ; L2Handle=BLST_S_NEXT(L2Handle, link))
        {
            if(!L2Handle->enableCount)
            {
                continue;
            }
            BINT_P_ClearMask_isrsafe(intHandle, L2Reg, BCHP_INT_ID_GET_SHIFT(L2Handle->intId));
        }
        break;
    }
    return match;
}

void BINT_ApplyL2State_isr(BINT_Handle intHandle, uint32_t L2RegOffset)
{
    unsigned L1Shift;
    const BINT_P_L2Register *L2Reg;

    BDBG_OBJECT_ASSERT(intHandle, BINT);

    for(L1Shift = 0; L1Shift<BINT_P_L1_SIZE; L1Shift++)
    {
        BINT_P_ApplyStateL2Reg_isr(intHandle, &intHandle->L1[L1Shift].L2RegList, L2RegOffset);
        for(L2Reg = BLST_S_FIRST(&intHandle->L1[L1Shift].L2RegList); L2Reg ; L2Reg = BLST_S_NEXT(L2Reg, link)) {
            const BINT_P_L2Aggregator *L2Aggregator;
            for(L2Aggregator = BLST_S_FIRST(&L2Reg->aggregatorList); L2Aggregator; L2Aggregator = BLST_S_NEXT(L2Aggregator, link)) {
                if(BINT_P_ApplyStateL2Reg_isr(intHandle, &L2Aggregator->L2RegList, L2RegOffset)) {
                    BINT_P_ClearMask_isrsafe(intHandle, L2Reg, L2Aggregator->L2Shift);
                }
            }
        }
    }
    return;
}

