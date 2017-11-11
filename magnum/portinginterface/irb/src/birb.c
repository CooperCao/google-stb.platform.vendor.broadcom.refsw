/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#include "bstd.h"
#include "birb.h"
#include "bchp_common.h"
#include "bchp_irb.h"
#ifdef BCHP_UPG_MAIN_IRQ_REG_START
#include "bchp_int_id_upg_main_irq.h"
#include "bchp_upg_main_irq.h"
#else
#include "bchp_irq0.h"
#include "bchp_int_id_irq0.h"
#endif

BDBG_MODULE(birb);

#define BIRB_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define MAX_IRB_SEQUENCES           159     /* leave room for 1 start bit */
#define IRB_PAGE_SIZE               40
#define NUMSEQ_GREATER_THAN_80_BIT  0x80

/*******************************************************************************
*
*   Private Data
*
*******************************************************************************/
static const SIrbConfiguration SonyConfiguration =
{
    "sony",                     /* name; */
    MASTER_DIVISOR,             /* masterDivisor */
    CARRIER_CLOCK_DIVISOR,      /* carrierClockDivisor */
    INDEX_CLOCK_DIVISOR,        /* indexClockDivisor */
    0,                          /* noCarrier */
    12/CARRIER_PERIOD,          /* carrierHighCount */
    13/CARRIER_PERIOD,          /* carrierLowCount */
    21,                         /* numberSequences */
    {0,2},                      /* logic0IndexPair */
    {1,2},                      /* logic1IndexPair */
    {                           /* sequenceIndex[] */
        {3,2},                  /* start bit */
        {1,2}, {0,2}, {1,2}, {0,2}, /* data bit 0..3 = 5 */
        {1,2}, {0,2}, {0,2}, {0,2}, /* data bit 4..7 = 1 */
        {1,2}, {0,2}, {1,2}, {1,2}, /* data bit 8..11 = d */
        {1,2}, {0,2}, {0,2}, {1,2}, /* data bit 12..15 = 9*/
        {0,2}, {0,2}, {1,2}, {0,2}  /* data bit 16..19 = 4*/
    },
    {                           /* duration[]; */
        600/INDEX_PERIOD,       /* 0.6ms, '0' ON */
        1200/INDEX_PERIOD,      /* 1.2ms, '1' ON */
        600/INDEX_PERIOD,       /* 0.6ms,  '0', '1', OFF */
        2400/INDEX_PERIOD,      /* 2.4ms,  start bit ON */
        600/INDEX_PERIOD        /* last sequence OFF duration */
                                /* 0.6ms + filler to make 45ms frame duration */
                                /* automatically calculated when sending */
    },
    (FRAME_PERIOD/INDEX_PERIOD),/* frame period A */
    0,                          /* frame period B */
    4,                          /* lastSequenceOffIndex */
    3,                          /* repeatCount */
    0,                           /* repeatStartIndex, repeat whole sequence */
    0                           /* altModCnt */
};

/* Configuration for GI Remote A IR Code */
static const SIrbConfiguration GIConfiguration =
{
    "GI",                       /* name; */
    MASTER_DIVISOR,             /* masterDivisor */
    CARRIER_CLOCK_DIVISOR,      /* carrierClockDivisor */
    INDEX_CLOCK_DIVISOR,        /* indexClockDivisor */
    0,                          /* noCarrier */
    12/CARRIER_PERIOD,          /* carrierHighCount */
    13/CARRIER_PERIOD,          /* carrierLowCount */
    17,                         /* numberSequences */
    {2,0},                      /* logic0IndexPair */
    {2,1},                      /* logic1IndexPair */
    {                           /* sequenceIndex[] */
        {3,1},                  /* start bit */
        {1,2}, {0,2}, {1,2}, {0,2}, /* data bit 0..3 = 5 */
        {1,2}, {0,2}, {0,2}, {0,2}, /* data bit 4..7 = 1 */
        {1,2}, {0,2}, {1,2}, {1,2}, /* data bit 8..11 = d */
        {1,2}, {0,2}, {0,2}, {1,2}, /* data bit 12..15 = 9*/
        {0,2}, {0,2}, {1,2}, {0,2}  /* data bit 16..19 = 4*/
    },
    {                           /* duration[]; */
        2250/INDEX_PERIOD,      /* 2.25ms, '0' OFF */
        4500/INDEX_PERIOD,      /* 4.50ms, '1' OFF */
        500/INDEX_PERIOD,       /* 0.50ms,  '0', '1', ON */
        9000/INDEX_PERIOD,      /* 9.00ms,  start bit, preamble */
        600/INDEX_PERIOD        /* last sequence OFF duration */
                                /* 0.6ms + filler to make 45ms frame duration */
                                /* automatically calculated when sending */
    },
    (FRAME_PERIOD/INDEX_PERIOD),/* frame period A */
    0,                          /* frame period B */
    4,                          /* lastSequenceOffIndex */
    0,                          /* repeatCount */
    0,                           /* repeatStartIndex, repeat whole sequence */
    0                           /* altModCnt */
};

/* Configuration for Pioneer */
static const SIrbConfiguration PioneerConfiguration =
{
    "pioneer",                  /* name; */
    MASTER_DIVISOR,             /* masterDivisor */
    CARRIER_CLOCK_DIVISOR,      /* carrierClockDivisor */
    INDEX_CLOCK_DIVISOR,        /* indexClockDivisor */
    1,                          /* noCarrier */
    12/CARRIER_PERIOD,          /* carrierHighCount */
    13/CARRIER_PERIOD,          /* carrierLowCount */
    17,                         /* numberSequences */
    {2,0},                      /* logic0IndexPair */
    {2,1},                      /* logic1IndexPair */
    {                           /* sequenceIndex[] */
        {3,4},                  /* start bit */
        {1,2}, {0,2}, {1,2}, {0,2}, /* data bit 0..3 = 5 */
        {1,2}, {0,2}, {0,2}, {0,2}, /* data bit 4..7 = 1 */
        {1,2}, {0,2}, {1,2}, {1,2}, /* data bit 8..11 = d */
        {1,2}, {0,2}, {0,2}, {1,2}, /* data bit 12..15 = 9*/
        {0,2}, {0,2}, {1,2}, {0,2}  /* data bit 16..19 = 4*/
    },
    {                           /* duration[]; */
        /* case 1, no B data bits */
        562/INDEX_PERIOD,       /* 0.562ms, '0' OFF */
        1687/INDEX_PERIOD,      /* 1.687ms, '1' OFF */
        562/INDEX_PERIOD,       /* 0.562ms,  '0', '1', ON */
        9000/INDEX_PERIOD,      /* 9.00ms,  start bit A and B, preamble */
        4500/INDEX_PERIOD,      /* 4.50ms,  start bit A off */
        2250/INDEX_PERIOD,      /* 2.25ms,  start bit B off */
        562/INDEX_PERIOD,       /* last sequence ON duration */
        562/INDEX_PERIOD,       /* last sequence OFF duration for A */
                                /* 0.6ms + filler to make 45ms frame duration */
                                /* automatically calculated when sending */

        /* case 2, B data bits */
        527/INDEX_PERIOD,       /* 0.527ms, '0' OFF */
        1583/INDEX_PERIOD,      /* 1.583ms, '1' OFF */
        528/INDEX_PERIOD,   /* 0.528ms,  '0', '1', ON */
        8440/INDEX_PERIOD,      /* 8.44ms,  start bit A and B, preamble */
        4220/INDEX_PERIOD,      /* 4.22ms,  start bit A off, start bit B off */
        562/INDEX_PERIOD        /* last sequence OFF duration for B */

    },
    (FRAME_PERIOD_PIONEER_ABBB_1/INDEX_PERIOD),     /* frame period A */
    (FRAME_PERIOD_PIONEER_ABBB_1/INDEX_PERIOD),     /* frame period B */
    CASE2_DURATION_INDEX - 1,   /* lastSequenceOffIndex */
    0,                          /* repeatCount */
    0,                           /* repeatStartIndex, repeat whole sequence */
    0                           /* altModCnt */
};

/* Configuration for Pioneer AAAA */
static const SIrbConfiguration PioneerAAAAConfiguration =
{
    "pioneerAAAA",              /* name; */
    MASTER_DIVISOR,             /* masterDivisor */
    CARRIER_CLOCK_DIVISOR,      /* carrierClockDivisor */
    INDEX_CLOCK_DIVISOR,        /* indexClockDivisor */
    1,                          /* noCarrier */
    12/CARRIER_PERIOD,          /* carrierHighCount */
    13/CARRIER_PERIOD,          /* carrierLowCount */
    17,                         /* numberSequences */
    {2,0},                      /* logic0IndexPair */
    {2,1},                      /* logic1IndexPair */
    {                           /* sequenceIndex[] */
        {3,4},                  /* start bit */
        {1,2}, {0,2}, {1,2}, {0,2}, /* data bit 0..3 = 5 */
        {1,2}, {0,2}, {0,2}, {0,2}, /* data bit 4..7 = 1 */
        {1,2}, {0,2}, {1,2}, {1,2}, /* data bit 8..11 = d */
        {1,2}, {0,2}, {0,2}, {1,2}, /* data bit 12..15 = 9*/
        {0,2}, {0,2}, {1,2}, {0,2}  /* data bit 16..19 = 4*/
    },
    {                           /* duration[]; */
        /* case 4, no end pulse, fixed frame time */
        870/INDEX_PERIOD,       /* 0.870ms, '0' OFF, ON */
        900/INDEX_PERIOD,       /* 0.900ms, '1' OFF, ON */

        /* case 5, end pulse, variable frame time */
        305/INDEX_PERIOD,       /* 0.305ms,  '0', OFF, ON, '1' ON */
        2137/INDEX_PERIOD,      /* 2.137ms,  '1', OFF */
        0,                      /* case 4 last sequence OFF duration */
        305/INDEX_PERIOD,       /* 0.305ms, end pulse */
        26000/INDEX_PERIOD      /* case 5 last sequence OFF duration, pause time */
    },
    (FRAME_PERIOD_PIONEER_AAAA_4/INDEX_PERIOD),     /* frame period A */
    (FRAME_PERIOD_PIONEER_AAAA_4/INDEX_PERIOD),     /* frame period B */
    0,                          /* lastSequenceOffIndex */
    0,                          /* repeatCount */
    0,                           /* repeatStartIndex, repeat whole sequence */
    0                           /* altModCnt */
};

/* Configuration for XMP-2 Station with 38KHz carrier */
static const SIrbConfiguration Xmp2StationConfiuration = {
    "xmp2_station_38k",         /* name; */
    1,                          /* masterDivisor */
    2,                          /* carrierClockDivisor */
    1,                          /* indexClockDivisor */
    0,                          /* noCarrier */
    118,                        /* carrierHighCount */
    237,                        /* carrierLowCount */
    17,                         /* numberSequences */
    {0,0},                      /* not used, logic0IndexPair */
    {0,0},                      /* not used, logic1IndexPair */
    {   /* sequenceIndex[] */
        {DATA_PULSE,DATA4}, {ZERO,DELTA1}, /* 1st nibble = 5 */
        {DATA_PULSE,DATA2}, {ZERO,ZERO},   /* 2nd nibble = 2 */
        {DATA_PULSE,DATA4}, {ZERO,DELTA1}, /* 1st nibble = 5 */
        {DATA_PULSE,DATA2}, {ZERO,ZERO},   /* 2nd nibble = 2 */
        {DATA_PULSE,DATA4}, {ZERO,DELTA1}, /* 1st nibble = 5 */
        {DATA_PULSE,DATA2}, {ZERO,ZERO},   /* 2nd nibble = 2 */
        {DATA_PULSE,DATA4}, {ZERO,DELTA1}, /* 1st nibble = 5 */
        {DATA_PULSE,DATA2}, {ZERO,ZERO},   /* 2nd nibble = 2 */
        {DATA_PULSE,ZERO}               /* trailer pulse */
    },
    {                           /* duration[] */
        3690,                   /* 0: deltaT-1 */
        3691,                   /* 1: deltaT, 136,71us*/
        3692,                   /* 2: deltaT+1*/
        20509,                  /* 3: D(0), low period for data 0 */
        27892,                  /* 4: D(2), low period for data 2 */
        35274,                  /* 5: D(4), low period for data 4 */
        42656,                  /* 6: D(6), low period for data 6 */
        50039,                  /* 7: D(8), low period for data 8 */
        57421,                  /* 8: D(10), low period for data 10 */
        64803,                  /* 9: D(12), low period for data 12 */
                                /* the remain 4 are double-time values */
        72186,              /* 10: D(14), low period for data 14 */
        5680,                   /* 11: D(DataPulse), high period of data symbol */
        13500,              /* 500us. */ /* 22720,(840us) */              /* 12: D(TrailerPulse), high period of Trailer pulse */
        0                   /* 13: Zero */
    },
    0,                          /* not used, frame period A*/
    0,                          /* not used, frame period B*/
    0,                          /* not used, lastSequenceOffIndex */
    0,                          /* repeatCount */
    0,                          /* repeatStartIndex, repeat whole sequence */
    1                           /* altModCnt */
};

static const SIrbConfiguration RC6Configuration =
{
    "rc6",                      /* name; */
    75,                         /* masterDivisor */
    1,                          /* carrierClockDivisor */
    1,                          /* indexClockDivisor */
    0,                          /* noCarrier */
    3,                          /* carrierHighCount */
    7,                          /* carrierLowCount */
    21,                         /* numberSequences */
    {0,2},                      /* logic0IndexPair (not used) */
    {1,2},                      /* logic1IndexPair (not used) */
    {                           /* sequenceIndex[] */
        {3,2},                  /* start bit */
        {1,2}, {0,2}, {1,2}, {0,2}, /* data bit 0..3 = 5 */
        {1,2}, {0,2}, {0,2}, {0,2}, /* data bit 4..7 = 1 */
        {1,2}, {0,2}, {1,2}, {1,2}, /* data bit 8..11 = d */
        {1,2}, {0,2}, {0,2}, {1,2}, /* data bit 12..15 = 9*/
        {0,2}, {0,2}, {1,2}, {0,2}  /* data bit 16..19 = 4*/
    },
    {                           /* duration[]; */
        0,
        160,                    /* 1T */
        320,                    /* 2T */
        480,                    /* 3T */
        640,                    /* 4T (not used) */
        800,                    /* 5T (not used) */
        960,                    /* 6T */
        30126                   /* 83.6 ms */
    },
    (FRAME_PERIOD/INDEX_PERIOD),/* frame period A */
    0,                          /* frame period B */
    4,                          /* lastSequenceOffIndex */
    3,                          /* repeatCount */
    0,                           /* repeatStartIndex, repeat whole sequence */
    1                           /* altModCnt */
};

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/
BDBG_OBJECT_ID(BIRB_Handle);

typedef struct BIRB_P_Handle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BDBG_OBJECT(BIRB_Handle)
    BCHP_Handle         hChip;
    BREG_Handle         hRegister;
    BINT_Handle         hInterrupt;
    BKNI_EventHandle    hEvent;
    BINT_CallbackHandle hCallback;
    SIrbConfiguration   *pConfig;
    SIrbConfiguration   CustomConfiguration;

    bool                longSequenceReloadPage0;
    bool                longSequenceReloadPage1;
    bool                intMode;
} BIRB_P_Handle;

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BIRB_Settings defIrbSettings =
{
    BIRB_Device_eSony,          /* IRB device */
    true                        /* intMode */
};

/*******************************************************************************
*
*   Private Module Functions (Prototypes)
*
*******************************************************************************/
static void BIRB_P_EnableInt_isr(BIRB_Handle hDev);
static void BIRB_P_DisableInt_isr(BIRB_Handle hDev);
static void BIRB_P_ConfigDataSequence(BIRB_Handle hDev, uint32_t *pData, uint8_t bits, bool headerPulse);
static void BIRB_P_ConfigDataSequenceRC6(BIRB_Handle hDev, unsigned mode, unsigned trailer, uint32_t *pData, uint8_t bits);
static void BIRB_P_ConfigDataSequenceAB(BIRB_Handle hDev, uint32_t *pDataA, uint8_t bitsA,
	uint32_t *pDataB, uint8_t bitsB, bool headerA, bool headerB, bool fixedFlag);
static void BIRB_P_ConfigDataSequenceAA(BIRB_Handle hDev, uint32_t *pData, uint8_t bits, bool headerPulse, bool fixedFlag);
static void BIRB_P_ConfigDataSequenceXmp2(BIRB_Handle hDev, uint8_t *pData, uint8_t numByte);
static void BIRB_P_HandleInterrupt_Isr(void *pParam1, int parm2);

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BIRB_Open(
    BIRB_Handle *pIrb,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BIRB_Settings *pSettings      /* settings */
    )
{
    BERR_Code       retCode = BERR_SUCCESS;
    BIRB_Handle     hDev;
    uint32_t        lval;

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );

    /* Alloc memory from the system heap */
    hDev = (BIRB_Handle) BKNI_Malloc( sizeof( BIRB_P_Handle ) );
    if( hDev == NULL )
    {
        *pIrb = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BIRB_Open: BKNI_malloc() failed"));
        goto done;
    }

    BDBG_OBJECT_SET(hDev, BIRB_Handle);
    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;

    hDev->pConfig = &hDev->CustomConfiguration;
    BIRB_Config(hDev, pSettings->irbDev);

    BIRB_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEvent) ) );

    BIRB_Reset (hDev);

    hDev->intMode = pSettings->intMode;
    if (pSettings->intMode)
    {
        #if defined(BCHP_INT_ID_irb_irqen)
            BIRB_CHK_RETCODE( retCode, BINT_CreateCallback(
                &(hDev->hCallback), hDev->hInterrupt, BCHP_INT_ID_irb_irqen,
                BIRB_P_HandleInterrupt_Isr, (void *) hDev, 0x00 ) );
        #elif defined(BCHP_INT_ID_irb)
            BIRB_CHK_RETCODE( retCode, BINT_CreateCallback(
                &(hDev->hCallback), hDev->hInterrupt, BCHP_INT_ID_irb,
                BIRB_P_HandleInterrupt_Isr, (void *) hDev, 0x00 ) );
        #else
            #error unsupported
        #endif
        BIRB_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hCallback ) );

        BKNI_EnterCriticalSection();

        /*
         * Enable IRB interrupt in UPG
         */

        #if defined(BCHP_UPG_MAIN_IRQ_CPU_STATUS)
            /* Register is write only, no need to read modify write */
            lval = BCHP_FIELD_DATA(UPG_MAIN_IRQ_CPU_MASK_CLEAR, irb, 1);
            BREG_Write32(hDev->hRegister, BCHP_UPG_MAIN_IRQ_CPU_MASK_CLEAR, lval);
        #else
            lval = BREG_Read32(hDev->hRegister, BCHP_IRQ0_IRQEN);
            #if defined(BCHP_IRQ0_IRQEN_irb_irqen_SHIFT)
                lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, irb_irqen, 1);
            #elif defined(BCHP_IRQ0_IRQEN_irb_SHIFT)
                lval |= BCHP_FIELD_DATA(IRQ0_IRQEN, irb, 1);
            #else
                #error unsupported
            #endif

            BREG_Write32( hDev->hRegister, BCHP_IRQ0_IRQEN, lval );
        #endif

        /*
         * Enable IRB interrupt in IRB
         */
        BIRB_P_EnableInt_isr (hDev);
        BKNI_LeaveCriticalSection();
    }
    else
	{
        BKNI_EnterCriticalSection();
        BIRB_P_DisableInt_isr (hDev);
        BKNI_LeaveCriticalSection();
	}

    *pIrb = hDev;

done:
    if ((retCode != BERR_SUCCESS) && hDev)
    {
        BKNI_Free( (void *) hDev );
        *pIrb = NULL;
    }
    return( retCode );
}

BERR_Code BIRB_Close(
    BIRB_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    BKNI_EnterCriticalSection();
    BIRB_P_DisableInt_isr(hDev);
    BKNI_LeaveCriticalSection();

    BIRB_CHK_RETCODE( retCode, BINT_DisableCallback( hDev->hCallback ) );
    BIRB_CHK_RETCODE( retCode, BINT_DestroyCallback( hDev->hCallback ) );
    BKNI_DestroyEvent( hDev->hEvent );

    BDBG_OBJECT_DESTROY(hDev, BIRB_Handle);
    BKNI_Free( (void *) hDev );

done:
    return( retCode );
}

BERR_Code BIRB_GetDefaultSettings(
    BIRB_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(hChip);
    BDBG_ASSERT( pDefSettings );

    *pDefSettings = defIrbSettings;

    return( retCode );
}

BERR_Code BIRB_GetEventHandle(
    BIRB_Handle     hDev,               /* Device handle */
    BKNI_EventHandle *phEvent           /* [output] Returns event handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    *phEvent = hDev->hEvent;

    return( retCode );
}

void BIRB_ConfigCustom (
    BIRB_Handle         hDev,           /* Device handle */
    SIrbConfiguration   *pConfig        /* Pointer to custom config */
)
{
    BSTD_UNUSED(hDev);
    BKNI_Memcpy ((void *)&(hDev->CustomConfiguration), (void *)pConfig, sizeof(SIrbConfiguration));
}


BERR_Code BIRB_Config (
    BIRB_Handle         hDev,           /* Device handle */
    BIRB_Device         irbDev          /* IR device type */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	SIrbConfiguration *pConfig = NULL;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    switch (irbDev)
    {
        case BIRB_Device_eSony:
            pConfig = (SIrbConfiguration *)&SonyConfiguration;
            break;
        case BIRB_Device_eGI:
            pConfig = (SIrbConfiguration *)&GIConfiguration;
            break;
        case BIRB_Device_eCustom:
			/* Don't do this -- this is the default */
            /*pConfig = &(hDev->CustomConfiguration);*/
            break;
        case BIRB_Device_ePioneer:
            pConfig = (SIrbConfiguration *)&PioneerConfiguration;
            break;
        case BIRB_Device_ePioneerAAAA:
            pConfig = (SIrbConfiguration *)&PioneerAAAAConfiguration;
            break;
        case BIRB_Device_eXmp2:
            pConfig = (SIrbConfiguration *)&Xmp2StationConfiuration;
            break;
        case BIRB_Device_eRC6:
            pConfig = (SIrbConfiguration *)&RC6Configuration; /* For now, use same configuration as RC6 Mode 0 */
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

	if (pConfig)
		BIRB_ConfigCustom(hDev, pConfig);

    return( retCode );
}

void BIRB_Reset(
    BIRB_Handle         hDev            /* Device handle */
)
{
    uint32_t    lval;

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval |= BCHP_IRB_BLAST_CONTROL_blastRST_MASK;           /* assert reset */
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);
	/* Re-read to clear any bits cleared by reset (don't re-eanable) */
    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval &= ~BCHP_IRB_BLAST_CONTROL_blastRST_MASK;          /* de-assert reset */
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);
}

BERR_Code BIRB_Blast (
    BIRB_Handle         hDev            /* Device handle */
)
{
    BERR_Code   retCode = BERR_SUCCESS;
    uint32_t    lval;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval |= BCHP_IRB_BLAST_CONTROL_blast_MASK;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

    return( retCode );
}

bool BIRB_IsIrbFinished(
    BIRB_Handle         hDev            /* Device handle */
)
{
    uint32_t    lval;

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);

    return (lval & BCHP_IRB_BLAST_CONTROL_blast_MASK) ? false : true;
}

BERR_Code BIRB_SendWithHeaderOption (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pData,         /* pointer to data to blast */
    uint8_t             bits,           /* number of bits to blast */
    bool                headerPulse     /* header flag */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    if (bits > MAX_IRB_SEQUENCES)
    {
        retCode = BIRB_ERR_TOO_MANY_SEQ;
        goto done;
    }

    BIRB_Reset(hDev);

    BIRB_P_ConfigDataSequence(hDev, pData, bits, headerPulse);

    BIRB_ConfigRegisters(hDev);

    BIRB_CHK_RETCODE(retCode, BIRB_Blast(hDev));

done:
    return( retCode );
}

BERR_Code BIRB_SendRC6 (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pData,         /* pointer to data to blast */
    uint8_t             bits,           /* number of bits to blast */
    uint8_t             mode,           /* 3 bits */
    uint8_t             trailer         /* 0 or 1 */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    if (bits > MAX_IRB_SEQUENCES)
    {
        retCode = BIRB_ERR_TOO_MANY_SEQ;
        goto done;
    }

    BIRB_Reset(hDev);

    BIRB_P_ConfigDataSequenceRC6(hDev, mode, trailer, pData, bits);

    BIRB_ConfigRegisters(hDev);

    BIRB_CHK_RETCODE(retCode, BIRB_Blast(hDev));

done:
    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BIRB_Send (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pData,         /* pointer to data to blast */
    uint8_t             bits            /* number of bits to blast */
)
{
	return BIRB_SendWithHeaderOption(hDev, pData, bits, true);
}
#endif

BERR_Code BIRB_SendABBB (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pDataA,        /* pointer to data A to blast */
    uint8_t             bitsA,          /* number of bits in A to blast */
    uint32_t            *pDataB,        /* pointer to data B to blast */
    uint8_t             bitsB,          /* number of bits in B to blast */
    bool                headerA,        /* headerA flag */
    bool                headerB,        /* headerB flag */
    bool                fixedFlag       /* true: frame length fixed */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    if ((bitsA + bitsB) > MAX_IRB_SEQUENCES)
    {
        retCode = BIRB_ERR_TOO_MANY_SEQ;
        goto done;
    }

    BIRB_Reset (hDev);

    BIRB_P_ConfigDataSequenceAB(hDev, pDataA, bitsA, pDataB, bitsB, headerA, headerB, fixedFlag);

    BIRB_ConfigRegisters (hDev);

    BIRB_CHK_RETCODE( retCode, BIRB_Blast (hDev) );

done:
    return( retCode );
}

BERR_Code BIRB_SendAAAA (
    BIRB_Handle         hDev,           /* Device handle */
    uint32_t            *pDataA,        /* pointer to data A to blast */
    uint8_t             bitsA,          /* number of bits in A to blast */
    bool                headerPulse,    /* header flag */
    bool                fixedFlag       /* true: frame length fixed */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    if (bitsA > MAX_IRB_SEQUENCES)
    {
        retCode = BIRB_ERR_TOO_MANY_SEQ;
        goto done;
    }

    BIRB_Reset (hDev);

    BIRB_P_ConfigDataSequenceAA(hDev, pDataA, bitsA, headerPulse, fixedFlag);

    BIRB_ConfigRegisters (hDev);

    BIRB_CHK_RETCODE( retCode, BIRB_Blast (hDev) );

done:
    return( retCode );
}


BERR_Code BIRB_SendXmp2Ack (
    BIRB_Handle         hDev           /* Device handle */
)
{
    uint8_t     ackData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    /* build ack data
     * For now, owener code is 0.
     * Data structure is based on page 8 of Packet Layer Spec version 1.3,
     * which is slightly different from page 18 of XMP-2 Remore Demo v1.2.doc
     * ------------------------
     * |C1|C0|0|ACK|N3|N2|N1|N0|
     * ------------------------
     * C1-C0 10 = dual device mode
     * ACK = 1 NAK= 0
     * N3-N0 0000 = accepting all peripherals.
     */
    ackData = 0x90;

    BIRB_Reset (hDev);

    BIRB_P_ConfigDataSequenceXmp2(hDev, &ackData, 1);

    BIRB_ConfigRegisters (hDev);

    BIRB_CHK_RETCODE( retCode, BIRB_Blast (hDev) );

done:
    return( retCode );
}


BERR_Code BIRB_SendXmp2Nack (
    BIRB_Handle         hDev           /* Device handle */
)
{
    uint8_t     ackData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);

    /* build ack data
     * For now, owener code is 0.
     * Data structure is based on page 8 of Packet Layer Spec version 1.3,
     * which is slightly different from page 18 of XMP-2 Remore Demo v1.2.doc
     * ------------------------
     * |C1|C0|0|ACK|N3|N2|N1|N0|
     * ------------------------
     * C1-C0 10 = dual device mode
     * ACK = 1 NAK= 0
     * N3-N0 0000 = accepting all peripherals.
     */
    ackData = 0x80;

    BIRB_Reset (hDev);

    BIRB_P_ConfigDataSequenceXmp2(hDev, &ackData, 1);

    BIRB_ConfigRegisters (hDev);

    BIRB_CHK_RETCODE( retCode, BIRB_Blast (hDev) );

done:
    return( retCode );
}


/****************************************************************
 * This function is used to send generic one or four byte data. *
 ****************************************************************/
BERR_Code BIRB_SendXmp2Bytes (
    BIRB_Handle         hDev,           /* Device handle */
    uint8_t             *pByte,         /* date to send */
    uint8_t             numByte         /* 1 or 4 bye */
)
{
    uint8_t     ackData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev,BIRB_Handle);
    BSTD_UNUSED(ackData);

    if (numByte != 1 && numByte != 4)
    {
        BDBG_ERR(("%s/%d : unsupported bytes", __FILE__, __LINE__)); \
        return BERR_INVALID_PARAMETER;
    }
    BIRB_Reset (hDev);

    BIRB_P_ConfigDataSequenceXmp2(hDev, pByte, numByte);

    BIRB_ConfigRegisters (hDev);

    BIRB_CHK_RETCODE( retCode, BIRB_Blast (hDev) );

done:
    return( retCode );
}


void BIRB_ConfigRegisters(
    BIRB_Handle     hDev
)
{
    uint32_t i, lval, regIndex, addr, duration;
    SIrbConfiguration *pIrbConfig = hDev->pConfig;

    /* Clear interrupt status */
    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS);
    lval |= (BCHP_IRB_BLAST_INTRSTATUS_seqPage0Done_MASK |
             BCHP_IRB_BLAST_INTRSTATUS_seqPage1Done_MASK |
             BCHP_IRB_BLAST_INTRSTATUS_blastDone_MASK );
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS, lval);

    /*
     * Configure CONTROL register
     */
    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval |= (BCHP_IRB_BLAST_CONTROL_intensify_MASK | (pIrbConfig->noCarrier ? BCHP_IRB_BLAST_CONTROL_carrInh_MASK : 0));
    lval &= ~BCHP_IRB_BLAST_CONTROL_seqPage_MASK;               /* starting loading page 0 */
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

    /*
     * Configure prescaler registers
     */
    lval = pIrbConfig->masterClockDivisor - 1;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_PRIMPRE, lval);

    lval  = (pIrbConfig->carrierClockDivisor - 1) << 4;
    lval |= (pIrbConfig->indexClockDivisor - 1);
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INDXPRE, lval);

    /*
     * Configure repeat registers
     */
    lval = pIrbConfig->repeatCount;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_REPNUM, lval);

    lval = pIrbConfig->repeatStartIndex;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_REPINDX, lval);
    /*
     * Configure carrier width registers
     */
    lval = pIrbConfig->carrierHighCount - 1;
/*  if (lval & ~0xf)
        BRCM_DBG_WRN(("irblastr carrierHighCount > 16"));
*/
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CARRHI, lval);

    lval = pIrbConfig->carrierLowCount - 1;
/*  if (bval & ~0xf)
        BRCM_DBG_WRN(("irblastr carrierLowCount > 16"));
*/
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CARRLO, lval);

    /*
     * Configure Number of Sequence register
     */
    if (pIrbConfig->numberSequences > (2 * IRB_PAGE_SIZE))
    {
        lval = pIrbConfig->numberSequences - (2 * IRB_PAGE_SIZE) - 1;
        lval |= NUMSEQ_GREATER_THAN_80_BIT;
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_NUMSEQ, lval);

        /*
         * Must set longSeqMode bit
         */
        lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE);
        lval |= (BCHP_IRB_BLAST_INTRENABLE_longSeqMode_MASK |
                BCHP_IRB_BLAST_INTRENABLE_seqPage0Done_MASK);

        hDev->longSequenceReloadPage0 = true;

        if (pIrbConfig->numberSequences > (3 * IRB_PAGE_SIZE))
        {
            lval |= BCHP_IRB_BLAST_INTRENABLE_seqPage1Done_MASK;
            hDev->longSequenceReloadPage1 = true;
        }

        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE, lval);
    }
    else
    {
        lval = pIrbConfig->numberSequences - 1;
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_NUMSEQ, lval);
        hDev->longSequenceReloadPage0 = hDev->longSequenceReloadPage1 = false;
    }

    /*
     * Configure Sequence Index registers.
     */
    for (i = 0, regIndex = 0; i < pIrbConfig->numberSequences; i++, regIndex++)
    {
        /*
         * Check to see if we've loaded both pages already
         */
        if (i == (2* IRB_PAGE_SIZE))
            break;

        /*
         * Check to see if we have to switch page
         */
        if (i == IRB_PAGE_SIZE)
        {
            lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
            lval |= BCHP_IRB_BLAST_CONTROL_seqPage_MASK;
            BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

            regIndex = 0;
        }

        lval = (pIrbConfig->sequenceIndex[i].on << 4) | (pIrbConfig->sequenceIndex[i].off);
        BREG_Write32(hDev->hRegister, (BCHP_IRB_BLAST_SEQ_REGFILE00 + (regIndex * 4)), lval);
    }

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval &= ~BCHP_IRB_BLAST_CONTROL_seqPage_MASK;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

    addr = BCHP_IRB_BLAST_MOD_REGFILE00;    /* address of duration memory */
    for (i = 0; i < irb_durationMemLength; i++)
    {
        if (pIrbConfig->altModCnt == 1)
        {
            /* When altModeCnt is one, actual 0 pulse is used.
             * Do not subtract 1
             */
            duration = pIrbConfig->duration[i];
        }
        else
        {
            duration = pIrbConfig->duration[i] - 1;
        }

        /* The last 4 entries are "double-time", so we need to divide them by 2 */
        if (i >= 10)
            duration /= 2;

        lval = duration >> 8;
        BREG_Write32(hDev->hRegister, addr, lval);
        addr += 4;

        lval = duration & 0xff;
        BREG_Write32(hDev->hRegister, addr, lval);
        addr += 4;
    }

    /* Configure altModCnt bit in INTRSTATUS register. */
    if (pIrbConfig->altModCnt == 1)
    {
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE, BLAST_INTRENABLE_ALTMODCNT);
    }
    else
    {
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRDISABLE, BLAST_INTRENABLE_ALTMODCNT);
    }
}

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
static void BIRB_P_EnableInt_isr(
    BIRB_Handle     hDev
)
{
    uint32_t    lval;

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval |= BCHP_IRB_BLAST_CONTROL_irqInh_MASK;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE);
    lval |= (BCHP_IRB_BLAST_INTRENABLE_masterIntrEn_MASK | BCHP_IRB_BLAST_INTRENABLE_blastDone_MASK);
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE, lval);

}

static void BIRB_P_DisableInt_isr(
    BIRB_Handle     hDev
)
{
    uint32_t    lval;

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
    lval |= BCHP_IRB_BLAST_CONTROL_irqInh_MASK;
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE);
    lval &= ~(BCHP_IRB_BLAST_INTRENABLE_masterIntrEn_MASK | BCHP_IRB_BLAST_INTRENABLE_blastDone_MASK);
    BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE, lval);

}

static void BIRB_P_ConfigDataSequence(
    BIRB_Handle     hDev,
    uint32_t        *pData,         /* pointer to data to blast */
    uint8_t         bits,           /* number of bits to blast */
    bool            headerPulse     /* header flag */
)
{
    uint8_t     i, lastSeqIndex, noHeader;
    uint32_t    code, frameLength = 0, lastOffPeriod;
    SIrbConfiguration *pIrbConfig = hDev->pConfig;

    if (headerPulse)
    {
        noHeader = 0;
        pIrbConfig->numberSequences = bits + 1;
    }
    else
    {
        noHeader = 1;
    }

    /*
     * Start bit sequence index already at sequenceIndex[0] if there's a header.
     * In this case, fill in indices for data bits beginning at sequenceIndex[1].
     */
    lastSeqIndex = bits;
    code = *pData++;
    for (i = 1 - noHeader; i <= lastSeqIndex - noHeader; i++)
    {
        pIrbConfig->sequenceIndex[i] = (code & 1) ? pIrbConfig->logic1IndexPair
                                                  : pIrbConfig->logic0IndexPair;
        code >>= 1;
        if (((i + noHeader) % 32) == 0)
        {
            code = *pData++;
        }
    }

    /*
     * Extend OFF period of last pulse to satisfy frame duration requirment.
     */
    for (i = 0; i <= (lastSeqIndex - noHeader); i++)
    {
        irb_IndexPair indexPair = pIrbConfig->sequenceIndex[i];
        frameLength +=  pIrbConfig->duration[indexPair.on] +
                        pIrbConfig->duration[indexPair.off];
    }
    lastOffPeriod = pIrbConfig->framePeriod - frameLength +
                    pIrbConfig->duration[pIrbConfig->sequenceIndex[lastSeqIndex].off];
    pIrbConfig->duration[pIrbConfig->lastSequenceOffIndex] = lastOffPeriod;
    pIrbConfig->sequenceIndex[lastSeqIndex].off = pIrbConfig->lastSequenceOffIndex;
}

static void rc6DataToSequence(SIrbConfiguration *pConfig, int *pSeqIndex, uint32_t data, int dataSize, unsigned onIndex, unsigned offIndex)
{
    int i;
    for (i=dataSize-1; i>=0; i--) {
       unsigned dataBit = (data >> i) & 1;
       if (dataBit) {
          pConfig->sequenceIndex[*pSeqIndex].on = onIndex;
          pConfig->sequenceIndex[*pSeqIndex].off = offIndex;
          (*pSeqIndex)++;
       } else {
          pConfig->sequenceIndex[*pSeqIndex].on = 0;
          pConfig->sequenceIndex[*pSeqIndex].off = offIndex;
          (*pSeqIndex)++;
          pConfig->sequenceIndex[*pSeqIndex].on = onIndex;
          pConfig->sequenceIndex[*pSeqIndex].off = 0;
          (*pSeqIndex)++;
       }
    }
}

static void BIRB_P_ConfigDataSequenceRC6(
    BIRB_Handle     hDev,
    unsigned        mode,
    unsigned        trailer,
    uint32_t        *pData,         /* pointer to data to blast */
    uint8_t         bits            /* number of bits to blast */
)
{
    SIrbConfiguration *pIrbConfig = hDev->pConfig;

    /* Assume IRB altModCnt=1 (so that duration=0 means zero-width pulse) and duration[i] = i*t, i=0..6 */

    int i=0;  /* index to sequence memory sequenceIndex[] */

    /* Leader bit: 6t on, 2t off */
    pIrbConfig->sequenceIndex[i].on = 6;
    pIrbConfig->sequenceIndex[i].off = 2;
    i++;

    /* Start bit: 1t on, 1t off */
    pIrbConfig->sequenceIndex[i].on = 1;
    pIrbConfig->sequenceIndex[i].off = 1;
    i++;

    /* Mode bits m2,m1,m0:  regular RC6 data encoded */
    rc6DataToSequence(pIrbConfig, &i, mode,3, 1,1);

    /* Trailer bit with 2t on and 2t off instead of 1t on and 1t off of regular data bit */
    rc6DataToSequence(pIrbConfig, &i, trailer,1, 2,2);

    /* Control and Information fields :  regular RC6 data encoded */
    rc6DataToSequence(pIrbConfig, &i, *pData,bits, 1,1);

#if 0
    /* Signal-freq time: 6t off */
    pIrbConfig->sequenceIndex[i].on = 0;
    pIrbConfig->sequenceIndex[i].off = 6;
    i++;
#else
    /* 83.4 ms */
    pIrbConfig->sequenceIndex[i].on = 0;
    pIrbConfig->sequenceIndex[i].off = 7;
    i++;
#endif

    pIrbConfig->numberSequences = i;
}

static void BIRB_P_ConfigDataSequenceAA(
    BIRB_Handle     hDev,
    uint32_t        *pData,         /* pointer to data to blast */
    uint8_t         bits,           /* number of bits to blast */
    bool            headerPulse,    /* header flag */
    bool            fixedFlag       /* fixed frame flag */
)
{
    uint8_t     i;
    uint8_t     noHeader;
    uint32_t    code, frameLength = 0, lastOffPeriod;
    SIrbConfiguration *pIrbConfig = hDev->pConfig;

    /*
     * Start bit sequence index already at sequenceIndex[0].
     * Fill in indices for data bits beginning at sequenceIndex[1].
     */

    if (headerPulse) {
        noHeader = 0;
    } else {
        noHeader = 1;
    }

    code = *pData++;
    for (i = 1 - noHeader; i <= bits - noHeader; i++)
    {
        pIrbConfig->sequenceIndex[i] = (code & 1) ? pIrbConfig->logic1IndexPair
                                                  : pIrbConfig->logic0IndexPair;
        code >>= 1;
        if (((i + noHeader) % 32) == 0)
        {
            code = *pData++;
        }
    }

    if (fixedFlag) {
        for (i = 0; i <= (pIrbConfig->numberSequences - 2); i++)
        {
            irb_IndexPair indexPair = pIrbConfig->sequenceIndex[i];
            frameLength +=  pIrbConfig->duration[indexPair.on] +
                            pIrbConfig->duration[indexPair.off];
        }

        lastOffPeriod = pIrbConfig->framePeriod - (frameLength +
                        pIrbConfig->duration[pIrbConfig->sequenceIndex[pIrbConfig->numberSequences - 1].on]);
        pIrbConfig->duration[pIrbConfig->lastSequenceOffIndex] = lastOffPeriod;
        /* pIrbConfig->sequenceIndex[pIrbConfig->numberSequences - 1].off = pIrbConfig->lastSequenceOffIndex; */
    }
}


static void BIRB_P_ConfigDataSequenceAB(
    BIRB_Handle     hDev,
    uint32_t        *pDataA,        /* pointer to data A to blast */
    uint8_t         bitsA,          /* number of bits A to blast */
    uint32_t        *pDataB,        /* pointer to data B to blast */
    uint8_t         bitsB,          /* number of bits B to blast */
    bool            headerA,        /* header A flag */
    bool            headerB,        /* header B flag */
    bool            fixedFlag       /* fixed frame length */
)
{
    uint8_t     i, startSeqIndexB, noHeader;
    uint32_t    code, frameLength = 0, lastOffPeriod;
    SIrbConfiguration *pIrbConfig = hDev->pConfig;

    /****************************************/
    /* Data A                               */
    /****************************************/

    if (headerA) {
        noHeader = 0;
    } else {
        noHeader = 1;
    }

    code = *pDataA++;
    for (i = 1 - noHeader; i <= bitsA - noHeader; i++)
    {
        pIrbConfig->sequenceIndex[i] = (code & 1) ? pIrbConfig->logic1IndexPair
                                                  : pIrbConfig->logic0IndexPair;
        code >>= 1;
        if (((i + noHeader) % 32) == 0)
        {
            code = *pDataA++;
        }
    }

    if (fixedFlag)
    {
        for (i = 0; i <= (bitsA - noHeader); i++)
        {
            irb_IndexPair indexPair = pIrbConfig->sequenceIndex[i];
            frameLength +=  pIrbConfig->duration[indexPair.on] +
                            pIrbConfig->duration[indexPair.off];
        }

        lastOffPeriod = pIrbConfig->framePeriod - (frameLength +
                        pIrbConfig->duration[pIrbConfig->sequenceIndex[bitsA - noHeader + 1].on]);
        pIrbConfig->duration[pIrbConfig->sequenceIndex[bitsA - noHeader + 1].off] = lastOffPeriod;
    }

    /****************************************/
    /* Data B                               */
    /****************************************/
    /*
     * Start bit sequence index already at sequenceIndex[0].
     * Fill in indices for data bits beginning at sequenceIndex[1].
     */
    startSeqIndexB = bitsA + 2 - noHeader;

    if (headerB) {
        noHeader = 0;
    } else {
        noHeader = 1;
    }

    if (bitsB != 0) {
        code = *pDataB++;
        for (i = (startSeqIndexB + 1 - noHeader); i <= (startSeqIndexB + bitsB - noHeader); i++)
        {
            pIrbConfig->sequenceIndex[i] = (code & 1) ? pIrbConfig->logic1IndexPair
                                                      : pIrbConfig->logic0IndexPair;
            code >>= 1;
            if (((i - startSeqIndexB + noHeader) % 32) == 0)
            {
                code = *pDataB++;
            }
        }
    }

    if (fixedFlag)
    {
        /*
         * Calculate time for data and header
         */
        frameLength = 0;
        for (i = startSeqIndexB; i <= (pIrbConfig->numberSequences - 2); i++)
        {
            irb_IndexPair indexPair = pIrbConfig->sequenceIndex[i];
            frameLength +=  pIrbConfig->duration[indexPair.on] +
                            pIrbConfig->duration[indexPair.off];
        }

        /* Now calculate the remaining off time for end pulse */
        lastOffPeriod = pIrbConfig->framePeriodB - (frameLength +
                        pIrbConfig->duration[pIrbConfig->sequenceIndex[pIrbConfig->numberSequences - 1].on]);

        pIrbConfig->duration[pIrbConfig->sequenceIndex[pIrbConfig->numberSequences - 1].off] = lastOffPeriod;
    }
}

/*****************************************************************************/
/* Set Data Sequence (sequenceIndex) in configuration for XMP-2 Station 38K  */
/*****************************************************************************/
static void BIRB_P_ConfigDataSequenceXmp2(
    BIRB_Handle     hDev,
    uint8_t         *pData,             /* data */
    uint8_t         numByte             /* 1 or 4 */
    )
{
    SIrbConfiguration *pIrbConfig= hDev->pConfig;
    uint8_t i;
    uint8_t nibble0, nibble1;

    if (numByte == 4)
    {
        pIrbConfig->numberSequences = 17;
        /* sequenceIndex[4] could be set to trailer pulse.
         * set it back to data pulse
         */
        pIrbConfig->sequenceIndex[4].on = DATA_PULSE;
    }
    else
    if (numByte == 1)
    {
        pIrbConfig->numberSequences = 5;
        /* set the trailer pulse */
        pIrbConfig->sequenceIndex[4].on = TRAILER_PULSE;
        pIrbConfig->sequenceIndex[4].off = ZERO;
    }
    else
    {
        BDBG_ERR(("%s/%d : unsupported bytes", __FILE__, __LINE__)); \
        return;
    }

    for (i=0; i<numByte; i++)
    {
        nibble0 = (pData[i] >> 4) & 0x0F;   /* upper nibble */
        nibble1 = pData[i] & 0x0F;          /* lower nibble */
        /* Data High already at sequenceIndex[i].on and
         * Zero already at sequenceIndex[i+1].on,
         * need only determine sequenceIndex[i].off and
         * sequenceIndex[i+1].off from nibble.
         */
        pIrbConfig->sequenceIndex[i*4].off   = DATA0 + (nibble0>>1);
        pIrbConfig->sequenceIndex[i*4+1].off = (nibble0 & 0x01) ? DELTA1 : ZERO;
        pIrbConfig->sequenceIndex[i*4+2].off = DATA0 + (nibble1>>1);
        pIrbConfig->sequenceIndex[i*4+3].off = (nibble1 & 0x01) ? DELTA1 : ZERO;
    }
}


static void BIRB_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BIRB_Handle         hDev;
    uint32_t            lval, i, regIndex;
    SIrbConfiguration   *pIrbConfig;
    BSTD_UNUSED(parm2);

    hDev = (BIRB_Handle) pParam1;
    BDBG_ASSERT( hDev );

    pIrbConfig = hDev->pConfig;

    lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS);
    lval &= BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_INTRENABLE);

    if (lval & BCHP_IRB_BLAST_INTRSTATUS_blastDone_MASK)
    {
        /* clear interrupt */
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS, BCHP_IRB_BLAST_INTRSTATUS_blastDone_MASK);
        BKNI_SetEvent( hDev->hEvent );
        return;
    }

    if (lval & BCHP_IRB_BLAST_INTRSTATUS_seqPage0Done_MASK)
    {
        /* clear interrupt */
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRDISABLE, BCHP_IRB_BLAST_INTRSTATUS_seqPage0Done_MASK);
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS, BCHP_IRB_BLAST_INTRSTATUS_seqPage0Done_MASK);

        /*
         * Load sequence 81-120
         */
        if (hDev->longSequenceReloadPage0)
        {
            lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
            lval &= ~BCHP_IRB_BLAST_CONTROL_seqPage_MASK;               /* choose page 0 */
            BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

            for (i = 80, regIndex = 0; i < pIrbConfig->numberSequences; i++, regIndex++)
            {
                lval = (pIrbConfig->sequenceIndex[i].on << 4) | (pIrbConfig->sequenceIndex[i].off);
                BREG_Write32(hDev->hRegister, (BCHP_IRB_BLAST_SEQ_REGFILE00 + (regIndex * 4)), lval);
                if (regIndex == 39)                                     /* we've filled up this page */
                    break;
            }
            hDev->longSequenceReloadPage0 = false;
        }
        return;
    }

    if (lval & BCHP_IRB_BLAST_INTRSTATUS_seqPage1Done_MASK)
    {
        /* clear interrupt */
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRDISABLE, BCHP_IRB_BLAST_INTRSTATUS_seqPage1Done_MASK);
        BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_INTRSTATUS, BCHP_IRB_BLAST_INTRSTATUS_seqPage1Done_MASK);

        /*
         * Load sequence 121-160
         */
        if (hDev->longSequenceReloadPage1)
        {
            lval = BREG_Read32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL);
            lval |= BCHP_IRB_BLAST_CONTROL_seqPage_MASK;                /* choose page 1 */
            BREG_Write32(hDev->hRegister, BCHP_IRB_BLAST_CONTROL, lval);

            for (i = 120, regIndex = 0; i < pIrbConfig->numberSequences; i++, regIndex++)
            {
                lval = (pIrbConfig->sequenceIndex[i].on << 4) | (pIrbConfig->sequenceIndex[i].off);
                BREG_Write32(hDev->hRegister, (BCHP_IRB_BLAST_SEQ_REGFILE00 + (regIndex * 4)), lval);
            }
            hDev->longSequenceReloadPage1 = false;
        }
    }
}

