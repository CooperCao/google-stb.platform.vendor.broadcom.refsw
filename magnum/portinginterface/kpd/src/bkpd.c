/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bstd.h"
#include "bkpd.h"
#include "bchp_ldk.h"

#if (BCHP_CHIP == 7125) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || (BCHP_CHIP == 7400) || \
    (BCHP_CHIP == 7405) || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7420) || (BCHP_CHIP == 7468) || (BCHP_CHIP == 7550)
    #include "bchp_irq0.h"
    #include "bchp_int_id_irq0.h"
#elif (BCHP_CHIP == 7271) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7260)
    #include "bchp_int_id_upg_main_aon_irq.h"
#else
    #include "bchp_int_id_irq0_aon.h"
    #include "bchp_irq0_aon.h"
#endif

BDBG_MODULE(bkpd);

#define BKPD_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define KPD_DEFAULT_PRESCALE        0x0055
#define KPD_DEFAULT_DUTYCYCLE_OFF   0x01
#define KPD_DEFAULT_DUTYCYCLE_ON    0xAA
#define KPD_DEFAULT_DEBOUNCE        0x40

#ifdef BCHP_INT_ID_ldk
    #define BCHP_INT_ID_ldk_irqen           BCHP_INT_ID_ldk
#endif

static void BKPD_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
);

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BKPD_CallbackInfo
{
    BKPD_CallbackFunc cbFunc ;
    void              *pParm1 ;
    int               parm2 ;
} BKPD_CallbackInfo;

BDBG_OBJECT_ID(BKPD);
typedef struct BKPD_P_Handle
{
    BDBG_OBJECT(BKPD)
    BCHP_Handle         hChip;
    BREG_Handle         hRegister;
    BINT_Handle         hInterrupt;
    BKNI_EventHandle    hEvent;
    BINT_CallbackHandle hCallback;
    bool                intMode;
    BKPD_CallbackInfo   stCallbackInfo;
} BKPD_P_Handle;

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BKPD_Settings defKpdSettings =
{
    KPD_DEFAULT_PRESCALE,
    KPD_DEFAULT_DEBOUNCE,
    true,
    0
};

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BKPD_Open(
    BKPD_Handle *pKpd,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BKPD_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code       retCode = BERR_SUCCESS;
    BKPD_Handle     hDev;
    uint32_t        lval;
    uint16_t        sval;
    uint32_t        prescale;

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );

    /* Alloc memory from the system heap */
    hDev = (BKPD_Handle) BKNI_Malloc( sizeof( BKPD_P_Handle ) );
    if( hDev == NULL )
    {
        *pKpd = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BKPD_Open: BKNI_malloc() failed\n"));
        goto done;
    }

    BKNI_Memset(hDev, 0, sizeof(*hDev));
    BDBG_OBJECT_SET(hDev, BKPD);
    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->intMode = pDefSettings->intMode;

    BKPD_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEvent) ) );

    prescale = pDefSettings->prescale;
    BREG_Write32 (hDev->hRegister, BCHP_LDK_PRESCHI, (uint8_t)(prescale >> 8));
    BREG_Write32 (hDev->hRegister, BCHP_LDK_PRESCLO, (uint8_t)(prescale & 0xff));

    BREG_Write32 (hDev->hRegister, BCHP_LDK_DEBOUNCE, pDefSettings->debounce);
#ifdef BCHP_LDK_KEY_MASK
    BREG_Write32 (hDev->hRegister, BCHP_LDK_KEY_MASK, pDefSettings->keyMask);
#endif

    /* Set up default duty cycle assuming 100% brightness. */
    BREG_Write32 (hDev->hRegister, BCHP_LDK_DUTYOFF, 0);
    BREG_Write32 (hDev->hRegister, BCHP_LDK_DUTYON, KPD_DEFAULT_DUTYCYCLE_ON+KPD_DEFAULT_DUTYCYCLE_OFF);

    /* Read data to clear any pending interrupt */
    BKPD_Read (hDev, &sval);

    if (hDev->intMode)
    {
        BKPD_CHK_RETCODE( retCode, BINT_CreateCallback(
            &(hDev->hCallback), hDev->hInterrupt, BCHP_INT_ID_ldk_irqen,
            BKPD_P_HandleInterrupt_Isr, (void *) hDev, 0x00 ) );
        BKPD_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hCallback ) );
    }

    BKNI_EnterCriticalSection();
    /* Enable keypad interrupt in led/keypad controller */
    lval = BREG_Read32 (hDev->hRegister, BCHP_LDK_CONTROL);
    lval |= BCHP_LDK_CONTROL_irqen_MASK;
    BREG_Write32 (hDev->hRegister, BCHP_LDK_CONTROL, lval);

    /* Clear user callback */
    hDev->stCallbackInfo.cbFunc = NULL;

    BKNI_LeaveCriticalSection();

    /* Set the VER bit.  Scan keypad only once at end of ON interval. */
    lval = BREG_Read32 (hDev->hRegister, BCHP_LDK_CONTROL);
    lval &= ~BCHP_LDK_CONTROL_swr_MASK;
    lval |=  BCHP_LDK_CONTROL_ver_MASK;
    lval &= 0x0f;                           /* make sure to zero out the upper bits */
    BREG_Write32 (hDev->hRegister, BCHP_LDK_CONTROL, lval);

    *pKpd = hDev;

done:
    if ((retCode != BERR_SUCCESS) && hDev)
    {
        BKNI_Free( (void *) hDev );
        *pKpd = NULL;
    }
    return( retCode );
}

BERR_Code BKPD_Close(
    BKPD_Handle hDev                    /* Device handle */
    )
{
    BERR_Code   retCode = BERR_SUCCESS;
    uint32_t    lval;

    BDBG_OBJECT_ASSERT(hDev, BKPD);

    if ( hDev->hCallback )
    {
        BKPD_CHK_RETCODE( retCode, BINT_DisableCallback( hDev->hCallback ) );
        BKPD_CHK_RETCODE( retCode, BINT_DestroyCallback( hDev->hCallback ) );
    }

    BKNI_EnterCriticalSection();
    /* Enable keypad interrupt in led/keypad controller */
    lval = BREG_Read32 (hDev->hRegister, BCHP_LDK_CONTROL);
    lval &= ~BCHP_LDK_CONTROL_irqen_MASK;
    BREG_Write32 (hDev->hRegister, BCHP_LDK_CONTROL, lval);
    BKNI_LeaveCriticalSection();

    BKNI_DestroyEvent( hDev->hEvent );
    BDBG_OBJECT_DESTROY(hDev, BKPD);
    BKNI_Free( (void *) hDev );

done:
    return( retCode );
}

BERR_Code BKPD_GetDefaultSettings(
    BKPD_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChip);

    *pDefSettings = defKpdSettings;

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BKPD_GetEventHandle(
    BKPD_Handle     hDev,           /* Device handle */
    BKNI_EventHandle *phEvent       /* [output] Returns event handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev, BKPD);

    *phEvent = hDev->hEvent;

    return( retCode );
}

BERR_Code BKPD_IsDataReady (
    BKPD_Handle         hKpd,           /* Device handle */
    bool                *ready          /* flag indicating key is pressed */
)
{
    uint32_t            lval;

    BDBG_OBJECT_ASSERT(hKpd, BKPD);

    #if defined(BCHP_IRQ0_AON_IRQSTAT_ldkirq_MASK)
        lval = BREG_Read32 (hKpd->hRegister, BCHP_IRQ0_AON_IRQSTAT);
        *ready = (lval & BCHP_IRQ0_AON_IRQSTAT_ldkirq_MASK) ? true : false;
    #elif defined(BCHP_IRQ0_AON_IRQSTAT_ldk_irq_MASK)
        lval = BREG_Read32 (hKpd->hRegister, BCHP_IRQ0_AON_IRQSTAT);
        *ready = (lval & BCHP_IRQ0_AON_IRQSTAT_ldk_irq_MASK) ? true : false;
    #elif defined(BCHP_IRQ0_IRQSTAT_ldk_MASK)
        lval = BREG_Read32 (hKpd->hRegister, BCHP_IRQ0_IRQSTAT);
        *ready = (lval & BCHP_IRQ0_IRQSTAT_ldk_MASK) ? true : false;
    #elif defined(BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_ldk_MASK)
        lval = BREG_Read32 (hKpd->hRegister,BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS);
        *ready = (lval & BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS_ldk_MASK) ? true : false;
    #else
        lval = BREG_Read32 (hKpd->hRegister, BCHP_IRQ0_IRQSTAT);
        *ready = (lval & BCHP_IRQ0_IRQSTAT_ldkirq_MASK) ? true : false;
    #endif

    return BERR_SUCCESS;
}
#endif

BERR_Code BKPD_Read_isr(
    BKPD_Handle         hKpd,           /* Device handle */
    uint16_t            *pData          /* pointer to data read from keypad */
)
{
    uint32_t            hi, lo;

    BDBG_OBJECT_ASSERT(hKpd, BKPD);
    hi = BREG_Read32 (hKpd->hRegister, BCHP_LDK_KEYROW32);
    lo = BREG_Read32 (hKpd->hRegister, BCHP_LDK_KEYROW10);

    *pData = (((uint16_t)hi) << 8) | ((uint16_t)lo);

    return BERR_SUCCESS;
}

BERR_Code BKPD_InstallInterruptCallback (
    BKPD_Handle         hKpd,           /* Device handle */
    BKPD_CallbackFunc   callback,       /* callback function */
    void                *pParm1,        /* application specified parameter */
    int                 parm2           /* application specified parameter */
)
{
    BDBG_OBJECT_ASSERT(hKpd, BKPD);

    BKNI_EnterCriticalSection();
    hKpd->stCallbackInfo.cbFunc = callback;
    hKpd->stCallbackInfo.pParm1 = pParm1 ;
    hKpd->stCallbackInfo.parm2 = parm2 ;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
BERR_Code BKPD_UnInstallInterruptCallback (
    BKPD_Handle         hKpd            /* Device handle */
)
{
    BDBG_OBJECT_ASSERT(hKpd, BKPD);

    BKNI_EnterCriticalSection();
    hKpd->stCallbackInfo.cbFunc = NULL;
    hKpd->stCallbackInfo.pParm1 = NULL ;
    hKpd->stCallbackInfo.parm2 = 0 ;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}
#endif

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
static void BKPD_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BKPD_Handle         hDev;
    uint16_t            data;

    BSTD_UNUSED(parm2);
    hDev = (BKPD_Handle) pParam1;
    BDBG_OBJECT_ASSERT(hDev, BKPD);

    /*
     * Check to see if user has installed a callback
     */
    if(hDev->stCallbackInfo.cbFunc)
    {
        hDev->stCallbackInfo.cbFunc (   hDev->stCallbackInfo.pParm1,
                                        hDev->stCallbackInfo.parm2,
                                        NULL) ;
    }

    /*
     * Clear keypad interrupt by reading the key registers
     */
    BKPD_Read_isr(hDev, &data);

    BKNI_SetEvent( hDev->hEvent );

    return;
}
