/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"
#include "bkir.h"
#include "bkir_priv.h"

#include "bchp_kbd1.h"
#if BKIR_N_CHANNELS > 1
    #include "bchp_kbd2.h"
#if BKIR_N_CHANNELS > 2
    #include "bchp_kbd3.h"
#if BKIR_N_CHANNELS > 3
    #include "bchp_kbd4.h"
#endif
#endif
#endif

#include "bchp_common.h"
#if defined(BCHP_UPG_MAIN_AON_IRQ_REG_START)
#include "bchp_int_id_upg_main_aon_irq.h"
#elif defined(BCHP_IRQ0_AON_REG_START)
#include "bchp_int_id_irq0_aon.h"
#else
#include "bchp_int_id_irq0.h"
#endif

#if (BCHP_CHIP == 7145) && (BCHP_VER < BCHP_VER_B0)
    #define KBD_USES_ZERO_BASED_NUMBERING
#endif

#ifdef KBD_USES_ZERO_BASED_NUMBERING
    #define INT_ID_0 BCHP_INT_ID_kbd0_irqen
    #define INT_ID_1 BCHP_INT_ID_kbd1_irqen
    #define INT_ID_2 BCHP_INT_ID_kbd2_irqen
    #define INT_ID_3 BCHP_INT_ID_kbd3_irqen
#else
    #if defined(BCHP_INT_ID_kbd1)
        #define INT_ID_0 BCHP_INT_ID_kbd1
        #define INT_ID_1 BCHP_INT_ID_kbd2
        #define INT_ID_2 BCHP_INT_ID_kbd3
        #define INT_ID_3 BCHP_INT_ID_kbd4
    #else
        #define INT_ID_0 BCHP_INT_ID_kbd1_irqen
        #define INT_ID_1 BCHP_INT_ID_kbd2_irqen
        #define INT_ID_2 BCHP_INT_ID_kbd3_irqen
        #define INT_ID_3 BCHP_INT_ID_kbd4_irqen
    #endif
#endif

#include "bchp_pm_aon.h"

#ifdef BCHP_PM_AON_CONFIG_irr1_in_UHF_RX1
#define HAS_UHF 1
#endif
#ifdef BCHP_PM_AON_CONFIG_irr3_in_UHF_RX1
#define HAS_UHF_3 1
#endif

#ifdef BCHP_PM_AON_CONFIG_irr0_in_MASK
#define BKIR_HAS_ZERO_BASE 1
#endif

#if BCHP_CHIP == 7439 /* TODO - remove when rdb updated */

#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN0      1
#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN1      0
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_12 3
#define BCHP_PM_AON_CONFIG_irr1_in_UHF_RX1     2

#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN0      3
#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN1      0
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_12 2
#define BCHP_PM_AON_CONFIG_irr2_in_UHF_RX1     1

#define BCHP_PM_AON_CONFIG_irr3_in_IR_IN0      2
#define BCHP_PM_AON_CONFIG_irr3_in_IR_IN1      3
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO_12 1
#define BCHP_PM_AON_CONFIG_irr3_in_UHF_RX1     0

#elif BCHP_CHIP == 7260
#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN0      0
#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN1      1
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_17 3

#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN0      3
#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN1      0
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_17 2

#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN0      2
#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN1      3
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_17 1

#elif BCHP_CHIP == 7268 || (BCHP_CHIP == 7271 && (BCHP_VER == A0) )

#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN0      0
#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN1      1
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_17 3

#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN0      3
#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN1      0
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_17 2

#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN0      2
#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN1      3
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_17 1

#elif (BCHP_CHIP == 7278) && (BCHP_VER == A0 )

#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN0      0
#define BCHP_PM_AON_CONFIG_irr0_in_IR_IN1      1
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_12 3

#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN0      3
#define BCHP_PM_AON_CONFIG_irr1_in_IR_IN1      0
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_12 2

#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN0      2
#define BCHP_PM_AON_CONFIG_irr2_in_IR_IN1      3
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_12 1

#endif


#ifdef BKIR_HAS_ZERO_BASE
#if (BCHP_CHIP == 7364) || (BCHP_CHIP == 7586) || (BCHP_CHIP == 7278)
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_12
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_12
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_12
#elif (BCHP_CHIP == 7250)
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_14
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_14
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_14
#else
#define BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO_17
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_17
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_17
#endif

#else

#if (BCHP_CHIP == 7445) || (BCHP_CHIP == 7439)
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_12
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_12
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO_12
#elif (BCHP_CHIP == 7228) || (BCHP_CHIP == 7360) || (BCHP_CHIP == 7362) || (BCHP_CHIP == 73625)\
   || (BCHP_CHIP == 7543) || (BCHP_CHIP == 7563)|| (BCHP_CHIP == 75635)
#define NO_IR_IN1 1
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_04
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_04
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO_04
#elif (BCHP_CHIP == 7231) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 73465)\
   || (BCHP_CHIP == 7358) || (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295)
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_GPIO_00
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_GPIO_00
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO BCHP_PM_AON_CONFIG_irr3_in_GPIO_00
#elif (BCHP_CHIP == 7552 ) || (BCHP_CHIP == 7584) || (BCHP_CHIP == 75845)
#define NO_IR_IN1 1
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_03
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_03
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO_03
#else
#define BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO_17
#define BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO_17
#define BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO_17
#endif

#endif


BDBG_MODULE(bkir);

#define BKIR_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define KBD_CMD_TWIRP_ENABLE        0x01
#define KBD_CMD_SEJIN_ENABLE        0x02
#define KBD_CMD_REMOTE_A_ENABLE     0x04
#define KBD_CMD_REMOTE_B_ENABLE     0x08
#define KBD_CMD_CIR_ENABLE          0x10

#define KBD_STATUS_DEVICE_MASK      0x1c
#define KBD_STATUS_DEVICE_SHIFTS    2

#ifndef RCMM_VARIABLE_LENGTH_SUPPORT
#define RCMM_VARIABLE_LENGTH_SUPPORT          0
#endif

/*******************************************************************************
*
*   Private Data
*
*******************************************************************************/
static const CIR_Param giParam = {
    270-1,          /* count divisor: divide by 270 for 10us period */
    { {900,0}, {450,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {900,0}, {225,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[]                              */
    2,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    1,              /* if true, pb[] matches a repeat sequence */
    50,             /* pulse tolerance */
    275,            /* T0 */
    225,            /* delta T */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {50,3},         /* data symbol fix-width pulse period, */
                    /* tolerance = pulse tolerance (above) */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and select code = 12.5% */
    16-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    50-1,           /* time-out clock divisor: divide by 50 or .5 ms */
    200,            /* frame time-out = 100 ms */
    22,             /* edge time-out = 11 ms */
    6,              /* minimum dead-time after fault = 3 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param sa_e2050Param = {
    100-1,          /*count divisor: divide by 100 for 3.70us period */
    { {910,0}, {890,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    227,            /* pulse tolerance = 0.841 ms */
    450,            /* T0 = 1.667 ms */
    450,            /* delta T = 1.667 ms */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {227,3},        /* data symbol fix-width pulse period = 0.841ms, */
                    /* tolerance = pulse tolerance (above) */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and select code = 12.5% */
    22-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    135-1,          /* time-out clock divisor: divide by 135 or .5ms */
    140,            /* frame time-out = 70 ms */
    9,              /* edge time-out = 4.5 ms */
    9,              /* minimum dead-time after fault = 4.5 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param twirpParam = {
    90-1,           /* count divisor: divide by 90 for 3.33us period */
    { {84,1}, {0,0}, {0,0}, {0,0} },    /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    1,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    84,             /* pulse tolerance = 0.280 ms */
    300,            /* T0 = 1.0000 ms */
    40+(3<<10),     /* delta T = 0.1358 ms (40.75*t) */
    1,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {84,1},         /* data symbol fix-width pulse period = 0.280, */
                    /* tolerance = 25% */
    {20, 3},        /* spacing tolerance value = .5 DeltaT, */
                    /* and select code = value */
    8-1,            /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    4-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    150-1,          /* time-out clock divisor: divide by 150 or .5ms */
    52,             /* frame time-out = 26 ms */
    8,              /* edge time-out = 4 ms */
    8,              /* minimum dead-time after fault = 4 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    972,                /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param sonyParam = {
    72-1,           /* count divisor: divide by 72 for 2.66us period */
    { {900,0}, {0,0}, {0,0}, {0,0} },   /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    1,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    225,            /* pulse tolerance = 0.500 ms */
    450,            /* T0 = 1.200 ms */
    225,            /* delta T = 0.600 ms */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {225,3},        /* data symbol fix-width pulse period = 0.600ms, */
                    /* tolerance = pulse tolerance (above) */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and select code = 12.5% */
    12-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    188-1,          /* time-out clock divisor: divide by 188 or .5ms */
    90,             /* frame time-out = 45 ms */
    7,              /* edge time-out = 3.5 ms */
    3,              /* minimum dead-time after fault = 1.5 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0x308,          /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param recs80Param = {
    226-1,          /* count divisor: divide by 226 for 8.370 us period */
    { {21,3}, {907,0}, {0,0}, {0,0} },      /* pa[], preamble A pulse sequence */
    { {21,3}, {453,0}, {21,3}, {453,0} },   /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    4,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    19,             /* pulse tolerance = 0.1590 ms */
    605,            /* T0 = 5.060 ms */
    302,            /* delta T = 2.5279 ms */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {21,3},         /* data symbol fix-width pulse period = 0.142ms, */
                    /* tolerance = pulse tolerance (above) */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and select code = 12.5% */
    10-1,           /* no. of symbols for sequence with preamble A */
    11-1,           /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    60-1,           /* time-out clock divisor: divide by 60 or .5ms */
    200,            /* frame time-out = 100ms */
    19,             /* edge time-out = 9ms */
    19,             /* minimum dead-time after fault = 9ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param rc5Param = {
    54-1,           /* count divisor: divide by 54 for 2.000 us period */
    { {445,1}, {889,1}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {889,1}, {0,0}, {0,0}, {0,0} },       /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    1,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    445,            /* pulse tolerance = value not used */
    889,            /* bit Period = 1.778 ms (PS T0) */
    0,              /* not used for bi-phase (PS delta T) */
    0,              /*  - " - (symbol pulse position) */
    0,              /*  - " - (measure spacing for complete cycle) */
    {0, 0},         /*  - " - (data symbol fix-width pulse period) */
    {0, 0},         /* bit period tolerance value = not applicable, */
                    /* and select code = 12.5% */
    12-1,           /* no. of symbols for sequence with preamble A */
    12-1,               /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    1,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    250-1,          /* time-out clock divisor: divide by 250 or .5ms */
    56,             /* frame time-out = 28 ms */
    5,              /* edge time-out = 2.5 ms */
    3,              /* minimum dead-time after fault = 1.5 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param ueiParam = {
    270-1,          /* count divisor */
    { {600,0}, {120,3}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {300,0}, {120,3}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    2,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    26,             /* pulse tolerance = value not used */
    60,             /* T0 */
    60,             /* not used for bi-phase (PS delta T) */
    0,              /*  - " - (symbol pulse position) */
    1,              /*  - " - (measure spacing for complete cycle) */
    {60, 1},        /*  - " - (data symbol fix-width pulse period) */
    {12, 3},            /* bit period tolerance value = not applicable, */
                    /* and select code */
    8-1,            /* no. of symbols for sequence with preamble A */
    8-1,            /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    1,              /* two symbols per cycle */
    1,              /* check stop symbol */
    0,              /* variable length data */
    50-1,           /* time-out clock divisor */
    74,             /* frame time-out */
    14,             /* edge time-out */
    16,             /* minimum dead-time after fault */
    {900, 0},       /* stop symbol pulse or cycle period */
    1100,           /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param RfueiParam = {
    270-1,          /* count divisor */
    { {600,1}, {120,3}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {300,1}, {120,3}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    2,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    26,             /* pulse tolerance = value not used */
    60,             /* T0 */
    60,             /* delta T */
    0,              /* not used: symbol pulse position */
    1,              /* not used: measure spacing for complete cycle */
    {60, 0},        /* not used: data symbol fix-width pulse period */
    {0, 1},         /* bit period tolerance value = not applicable, */
                    /* and select code */
    20-1,           /* no. of symbols for sequence with preamble A */
    20-1,           /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    1,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    10-1,           /* time-out clock divisor */
    940,            /* frame time-out */
    70,             /* edge time-out */
    16,             /* minimum dead-time after fault */
    {60, 0},        /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


static const CIR_Param echoDishRemoteParam = {
/* Symbol time spec. includes 400us ON pulse. */
    270-1,                      /* count divisor: divide by 135 for 10us period */
    { {40,3}, {658,0}, {0,0}, {0,0} },  /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    2,                          /* number of entries in pa[] */
    0,                          /* number of entries in pb[] */
    0,                          /* measure preamble pulse: */
                                /*   0 => even counts specifies cycle period */
                                /*   1 => even counts specifies off pulse period */
    0,                          /* if true, pb[] matches a repeat sequence */
    40,                         /* pulse tolerance = 400us */
    325,                        /* T0 = 3.25ms */
    907 | 1<<10,                /* DeltaT = -1.1675ms */
    0,                          /* false => fix symbol pulse between edges 0 & 1 */
                                /* true => fix symbol pulse between edges 1 & 2 */
    0,                          /* false => measure spacing for complete cycle */
                            /* true => measure spacing between 2 consecutive edges */
                                /*   (for stop/IWG symbol) */
    {40,3},                     /* data/stop symbol fix-width pulse period, */
                                /*   tolerance = pulse tolerace (above) */
    {0, 0},                     /* spacing tolerance value = not applicable, */
                                /*   and spacing tolerance = 12.5% */
    16-1,                       /* no. of symbols for sequence with preamble A */
    0,                          /* no. of symbols for sequence with preamble B */
    1-1,                        /* no. of data bits per symbol */
    0,                          /* most/!least significant symbol received first */
    0,                          /* left/!right adjust received data */
    0,                          /* bi-phase/!pulse-spacing coded */
    0,                          /* two sub-symbols per cycle */
    1,                          /* have inter-word gap or stop symbol */
    1,                          /* variable length data */
    50-1,                       /* time-out clock divisor: divide by 50 or .5 ms */
    148,                        /* frame time-out = 74 ms */
    16,                         /* edge time-out = 8.0 ms */
    16,                         /* minimum dead-time after fault = 8.0 ms */
    {658,0},                    /* stop/IWG off/cycle period = 6.5ms, 12.5% */
    744,                         /* data symbol time-out */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param echostarUHFParam =
{   /* Manchester */
/* Symbol time spec. includes 400us ON pulse. */
    108-1,                      /*  count unit @ 27MHz = 4us */
    { {500,0}, {250,0}, {500,0}, {250,0}},  /* pa[], preamble A pulse sequence */
    { {500,0}, {500,0}, {250,0}, {250,0}},     /* pb[], preamble B pulse sequence */
    3,                          /* number of entries in pa[] */
    3,                          /* number of entries in pb[] */
    1,                          /* measure preamble pulse: */
    /*   0 => even counts specifies cycle period */
    /*   1 => even counts specifies off pulse period */
    0,                          /* if true, pb[] matches a repeat sequence */
    0,                          /* pulse tolerance = 400us */
    250,                        /* T0 = 1ms */
    0,                          /* DeltaT = 0ms */
    0,                          /* false => fix symbol pulse between edges 0 & 1 */
    /* true => fix symbol pulse between edges 1 & 2 */
    0,                          /* false => measure spacing for complete cycle */
    /* true => measure spacing between 2 consecutive edges */
    /*   (for stop/IWG symbol) */
    {0,0},                      /* data/stop symbol fix-width pulse period, */
    /*   tolerance = pulse tolerace (above) */
    {0,0},                      /* spacing tolerance value = not applicable, */
    /*   and spacing tolerance = 12.5% */
    26-1,                       /* no. of symbols for sequence with preamble A */
    5-1,                        /* no. of symbols for sequence with preamble B */
    1-1,                        /* no. of data bits per symbol */
    1,                          /* most/!least significant symbol received first */
    0,                          /* left/!right adjust received data */
    1,                          /* bi-phase/!pulse-spacing coded */
    0,                          /* two sub-symbols per cycle */
    0,                          /* have inter-word gap or stop symbol */
    0,                          /* variable length data */
    50-1,                       /* time-out unit @ 27MHz = 200us */
    253,                        /* frame time-out = 50.5 ms */
    25,                         /* edge time-out = 5.0 ms */
    2,                          /* minimum dead-time after fault = 3.0 ms */
    {0,0},                      /* stop/IWG off/cycle period = 6.5ms, 12.5% */
    0,                          /* data symbol time-out */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param necParam = {
    270-1,          /* count divisor */
    { {900,0}, {450,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {900,0}, {225,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    2,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    1,              /* if true, pb[] matches a repeat sequence */
    50,             /* pulse tolerance */
    113,            /* bit Period = 1.778 ms (PS T0) */
    113,            /* delta T */
    0,              /*  - " - (symbol pulse position) */
    0,              /*  - " - (measure spacing for complete cycle) */
    {50, 3},        /*  - " - (data symbol fix-width pulse period) */
    {0, 0},         /* bit period tolerance value = not applicable, */
                    /* and select code = 12.5% */
    48-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    50-1,           /* time-out clock divisor */
    255,            /* frame time-out */
    22,             /* edge time-out */
    6,              /* minimum dead-time after fault = 1.5 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    600,            /* data symbol timeout */
    280,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param giSatParam = {
    270-1,          /* count divisor: divide by 270 for 10us period */
    { {500,0}, {200,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    50,             /* pulse tolerance */
    200,            /* T0 */
    200,            /* delta T */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {100,3},        /* data symbol fix-width pulse period, */
                    /* tolerance = pulse tolerance (above) */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and select code = 12.5% */
    12-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    50-1,           /* time-out clock divisor: divide by 50 or .5 ms */
    200,            /* frame time-out = 100 ms */
    22,             /* edge time-out = 11 ms */
    6,              /* minimum dead-time after fault = 3 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param directvUHFParam = {
    270-1,            /* count divisor */
    { {600,0}, {120,3}, {0,0}, {0,0} },    /* pa[], preamble A pulse sequence */
    { {300,0}, {120,3}, {0,0}, {0,0} },    /* pb[], preamble B pulse sequence */
    2,                /* number of entries in pa[] */
    2,                /* number of entries in pb[] */
    1,                /* measure preamble pulse: */
                    /*    0 => even counts specifies cycle period */
                    /*    1 => even counts specifies off pulse period */
    0,                /* if true, pb[] matches a repeat sequence */
    26,                /* pulse tolerance = value not used */
    60,                /* T0 */
    60,                /* not used for bi-phase (PS delta T) */
    0,                /*  - " - (symbol pulse position) */
    0,                /*  - " - (measure spacing for complete cycle) */
    {0, 0},        /*  - " - (data symbol fix-width pulse period) */
    {0, 0},            /* bit period tolerance value = not applicable, */
                    /* and select code */
    20-1,            /* no. of symbols for sequence with preamble A */
    20-1,            /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,                /* most/!least significant symbol received first */
    0,                /* left/!right adjust received data */
    0,                /* bi-phase/!pulse-spacing coded */
    1,                /* two symbols per cycle */
    0,                /* check stop symbol */
    0,                /* variable length data */
    5-1,            /* time-out clock divisor */
    1880,                /* frame time-out */
    140,                /* edge time-out */
    30,                /* minimum dead-time after fault */
    {600, 0},        /* stop symbol pulse or cycle period */
    0,            /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/* CIR configuration parameters for RC6 Mode 0 */
static const CIR_Param rC6Mode0Param = {
    125-1,      /* count divisor: count period = T/96 @ 27MHz, T=444.44.us */
    { {576,1}, {192,1}, {96,1}, {0,0} },
            /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },
            /* pb[], preamble B pulse sequence */
    3,      /* number of entries in pa[] */
    0,      /* number of entries in pb[] */
    1,      /* preamble pulse type: */
            /*   0 => even counts specifies cycle period */
            /*   1 => even counts specifies off pulse period */
    0,      /* if true, pb[] matches a repeat sequence */
    0,      /* not used: pulse tolerance */
    192,        /* T0 = 888.88us (2T) */
    0,      /* not used: DeltaT */
    0,      /* not used */
    0,      /* not used */
    {0,0},      /* not used: stop symbol */
    {0,1},      /* bit period tolerance = ~12.5% */
    0,      /* not used: no. of symbols for sequence with preamble A */
    0,      /* not used: no. of symbols for sequence with preamble B */
    1-1,        /* no. of data bits per symbol = 1 */
    1,      /* most/!least significant symbol received first */
    0,      /* left/!right adjust received data */
    1,      /* bi-phase/!pulse-spacing coded */
    0,      /* two sub-symbols per cycle */
    0,      /* have inter-word gap or stop symbol */
    1,      /* variable length data */
    96-1,       /* time-out clock divisor: divide by 96 for 0.444ms period */
    100,        /* frame time-out = 100T */
    8,      /* edge time-out = 8T */
    8,      /* minimum dead-time after fault = 8T */
    {0,0},      /* not used: stop/IWG off/cycle period */
    672,        /* data symbol time-out = 3.11ms = 7T */
    0,      /* not used: repeat timer timeout */
    0,      /* stop parameter unit selector: */
            /*   0: stop has count units */
            /*   1: stop has timout units */
    0,      /* data symbol timer clock tick dataSymTimeout units selector: */
            /*   0: dataSymTimeout has count units */
            /*   1: dataSymTimeout has timout units */
    0,      /* not used for biphase: ignore data symbol timer expiration */
            /*      while waiting for Edge 1: 0:false, 1:true */
    0,      /* enable data symbol time-out expiration flag to lflag */
            /*      status register: 0:false, 1:true */
    0,      /* enable havePreambleAftStop parameter for */
            /*      non-biphase decoding: 0:false, 1:true */
    0,      /* have preamble after stop symbol: 0:false, 1:true */
    1,      /* restrictive decoding enabled: 0:false, 1:true */
    1,      /* RC6 encoded: 0:false, 1:true. Requires biphaseCoded=1 */
    1,      /* don't validate RC mode bits: 0:false, 1:true */
    0,      /* RC6 mode bits (3 bits): 0 for Mode 0, 6 for Mode 6 */
    0,      /* Reserved for future use */
    0,      /* Reserved for future use */
    1,      /* don't validate control field bits: 0:false, 1:true */
    0,      /* RC6M0 control field (8 bits) or RC6M6A customer code bits (16 bits) */
    8-1,        /* number of RC6M0 control field bits: 0..15 for 1..16 bits */
    1       /* RC6 mode bits and control field pass-through control: */
            /*   0: Exclude mode bits and control field from */
            /*      received data. The nccb field determines */
            /*      the size of the control field. */
            /*   1: Exclude mode bits from the received data, but, */
            /*      include control field. */
            /*   2: Not allowed. */
            /*   3: Include both mode bits and customer code in */
            /*      the received data. */
};


/* CIR configuration parameters for RC6 Mode 6A */
static const CIR_Param rC6Mode6AParam = {
    125-1,      /* count divisor: count period = T/96 @ 27MHz, T=444.44.us */
    { {576,1}, {192,1}, {96,1}, {0,0} },
                /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },
                /* pb[], preamble B pulse sequence */
    3,          /* number of entries in pa[] */
    0,          /* number of entries in pb[] */
    1,          /* preamble pulse type: */
                /*   0 => even counts specifies cycle period */
                /*   1 => even counts specifies off pulse period */
    0,          /* if true, pb[] matches a repeat sequence */
    0,          /* not used: pulse tolerance */
    192,        /* T0 = 888.88us (2T) */
    0,          /* not used: DeltaT */
    0,          /* not used */
    0,          /* not used */
    {0,0},      /* not used: stop symbol */
    {0,1},      /* bit period tolerance = ~12.5% */
    0,          /* not used: no. of symbols for sequence with preamble A */
    0,          /* not used: no. of symbols for sequence with preamble B */
    1-1,        /* no. of data bits per symbol = 1 */
    1,          /* most/!least significant symbol received first */
    0,          /* left/!right adjust received data */
    1,          /* bi-phase/!pulse-spacing coded */
    0,          /* two sub-symbols per cycle */
    0,          /* have inter-word gap or stop symbol */
    1,          /* variable length data */
    96-1,       /* time-out clock divisor: divide by 96 for 0.444ms period */
    100,        /* frame time-out = 100T */
    8,          /* edge time-out = 8T */
    8,          /* minimum dead-time after fault = 8T */
    {0,0},      /* not used: stop/IWG off/cycle period */
    672,        /* data symbol time-out = 3.11ms = 7T */
    0,          /* not used: repeat timer timeout */
    0,          /* stop parameter unit selector: */
                /*   0: stop has count units */
                /*   1: stop has timout units */
    0,          /* data symbol timer clock tick dataSymTimeout units selector: */
                /*   0: dataSymTimeout has count units */
                /*   1: dataSymTimeout has timout units */
    0,          /* not used for biphase: ignore data symbol timer expiration */
                /*      while waiting for Edge 1: 0:false, 1:true */
    0,          /* enable data symbol time-out expiration flag to lflag */
                /*      status register: 0:false, 1:true */
    0,          /* enable havePreambleAftStop parameter for */
                /*      non-biphase decoding: 0:false, 1:true */
    0,          /* have preamble after stop symbol: 0:false, 1:true */
    1,          /* restrictive decoding enabled: 0:false, 1:true */
    1,          /* RC6 encoded: 0:false, 1:true. Requires biphaseCoded=1 */
    1,          /* don't validate RC mode bits: 0:false, 1:true */
    6,          /* RC6 mode bits (3 bits): 6 for Mode 6A */
    0,          /* Reserved: don't validate RC6 trailer: 0:false, 1:true */
    0,          /* Reserved: RC6 trailer (1 bit): 0 for Mode 6A */
    1,          /* don't validate customer code bits: 0:false, 1:true */
    0,          /* RC6 customer code bits (16 bits) */
    0,          /* number of RC6 customer code bits: 0..15 for 1..16 bits */
    3           /* RC6 mode bits and customer code pass-through control: */
                /*   0: Exclude mode bits and customer code from */
                /*      received data. The nccb field determines */
                /*      the size of the customer code. */
                /*   1: Exclude mode bits from the received data, but, */
                /*      include customer code. */
                /*   2: Not allowed. */
                /*   3: Include both mode bits and customer code in */
                /*      the received data. */
};

#if RCMM_VARIABLE_LENGTH_SUPPORT != 1
static const CIR_Param s_RCMMParam = {
/*
 * CIR configuration parameters for RCMM
 *
 * T0 = 444.44us
 * deltaT = 166.67us
 * Data 3 symbol OFF pulse width = 944.44us - 166.67us = 777.77us
 * Maximum pulse width = max(416.66, 277.77, 166.66, 777.77)
 *                     = 777.77us
 * Maximum frame duration = 416.66us + 277.77us + 166.66us
 *                          + 16 * 944.44us
 *                        = 15972.22us
 */
    30-1,       /* count divisor: divide by for 1.111us period */
    { {411,2}, {625,3}, {0,0}, {0,0} }, /* {411,2}->457us ONTIME+/- 50%, {625,3}->695us(ON+OFF) and use pulse tolerance of 54 */
                /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },
                /* pb[], preamble B pulse sequence */
    2,          /* number of entries in pa[] */
    0,          /* number of entries in pb[] */
    0,          /* measure preamble pulse: */
                /*   0 => even counts specifies cycle period, meaning 625 in preamble denote ON+OFF */
                /*   1 => even counts specifies off pulse period */
    0,          /* if true, pb[] matches a repeat sequence */
    54,         /* pulse tolerance = 58us */
    400,        /* T0 = 444.44us */
    150,        /* DeltaT = 166.67ms */
    0,          /* false => fix symbol pulse between edges 0 & 1 */
                /* true => fix symbol pulse between edges 1 & 2 */
    0,          /* false => measure spacing for complete cycle */
                /* true => measure spacing between 2
                                        consecutive edges */
    {150,2},    /* data/stop symbol fix-width pulse period = 166.6us, tolerance = 50% */
    {54, 3},    /* spacing tolerance value = 58us, !!! */
    16-1,       /* no. of symbols for sequence with preamble A */
    0,          /* no. of symbols for sequence with preamble B */
    2-1,        /* no. of data bits per symbol */
    1,          /* most/!least significant symbol received first */
    0,          /* left/!right adjust received data */
    0,          /* bi-phase/!pulse-spacing coded */
    0,          /* two sub-symbols per cycle */
    0,          /* have inter-word gap or stop symbol */
    0,          /* variable length data */
    50-1,       /* time-out clock divisor: divide by 50
                                        or 55.55us */
    328,        /* frame time-out = 18,222us */
    18,         /* edge time-out = 1000us */
    18,         /* minimum dead-time after fault = 1000us */
    {0,0},      /* not used: stop/IWG off/cycle period */
    0,            /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#else
static const CIR_Param s_RCMMParam = {
/*
 * CIR configuration parameters for RCMM
 *
 * T0 = 444.44us
 * deltaT = 166.67us
 * Data 3 symbol OFF pulse width = 944.44us - 166.67us = 777.77us
 * Maximum pulse width = max(416.66, 277.77, 166.66, 777.77)
 *                     = 777.77us
 * Maximum frame duration = 416.66us + 277.77us + 166.66us
 *                          + 16 * 944.44us
 *                        = 15972.22us
 */
    30-1,       /* count divisor: divide by for 1.111us period */
    { {411,2}, {625,3}, {0,0}, {0,0} }, /* {411,2}->457us ONTIME+/- 50%, {625,3}->695us(ON+OFF) and use pulse tolerance of 54 */
                /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },
                /* pb[], preamble B pulse sequence */
    2,          /* number of entries in pa[] */
    0,          /* number of entries in pb[] */
    0,          /* measure preamble pulse: */
                /*   0 => even counts specifies cycle period, meaning 625 in preamble denote ON+OFF */
                /*   1 => even counts specifies off pulse period */
    0,          /* if true, pb[] matches a repeat sequence */
    54,         /* pulse tolerance = 58us */
    400,        /* T0 = 444.44us */
    150,        /* DeltaT = 166.67ms */
    0,          /* false => fix symbol pulse between edges 0 & 1 */
                /* true => fix symbol pulse between edges 1 & 2 */
    0,          /* false => measure spacing for complete cycle */
                /* true => measure spacing between 2
                                        consecutive edges */
    {150,2},    /* data/stop symbol fix-width pulse period = 166.6us, tolerance = 50% */
    {54, 3},    /* spacing tolerance value = 58us, !!! */
    16-1,       /* ot used with variable length data, no. of symbols for sequence with preamble A */
    0,          /* no. of symbols for sequence with preamble B */
    2-1,        /* no. of data bits per symbol */
    1,          /* most/!least significant symbol received first */
    0,          /* left/!right adjust received data */
    0,          /* bi-phase/!pulse-spacing coded */
    0,          /* two sub-symbols per cycle */
    0,          /* have inter-word gap or stop symbol */
    1,          /* variable length data */
    50-1,       /* time-out clock divisor: divide by 50
                                        or 55.55us */
    328,        /* frame time-out = 18,222us */
    18,         /* edge time-out = 1000us */
    18,         /* minimum dead-time after fault = 1000us */
    {0,0},      /* not used: stop/IWG off/cycle period */
    906,            /* data symbol timeout: require > max Data 3 period = 850 + 54 count units (= 944.4us+60us) = 904 units */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#endif

static const CIR_Param rStepParam = {
    54-1,           /* count divisor: divide by 54 for 2.000 us period */
    { {165,3}, {0,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    1,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    67,             /* pulse tolerance = 67*2=134us; pa ul = 330+134= 464; pall = 330-134=196 */
    325,            /* bit Period = 325*2=650 us (PS T0) */
    0,              /* not used for bi-phase (PS delta T) */
    0,              /*  - " - (symbol pulse position) */
    0,              /*  - " - (measure spacing for complete cycle) */
    {0, 0},         /*  - " - (data symbol fix-width pulse period) */
    {67, 3},        /* bit period tolerance value = 53*2=106us; ul = 650+134=784; ll=650-134=516*/
    17-1,           /* no. of symbols for sequence with preamble A */
    17-1,               /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    1,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    250-1,          /* time-out clock divisor: divide by 250 or .5ms */
    24+1,           /* frame time-out = 12 ms */
    3+1,                /* edge time-out = 1.5 ms */
    3,              /* minimum dead-time after fault = 1.5 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,              /* validate a short pulse */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

#ifdef XMP2_NO_ACK
static const CIR_Param xmp2Param = {
    90-1,           /* count divisor: 90 divide by 27MHz for 3.333us period */
    { {70,2}, {0,0}, {0,0}, {0,0} },    /* pa[], 67*3.33=223us, 1=+/125% 2= +/- 50%, 3=use purse tolerance*/
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    0,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    35,             /* pulse tolerance = 29*3.33 = 166.55us. Only used if tolerance code is 3 */
    291,            /* T0 = 970us (969.98+0*136.71) */
    41,             /* delta T = 136.71us 41*3.333=136.653*/
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {70,2},         /* data symbol fix-width pulse period = 0.280, */
                    /* tolerance 2 = 50% */
    {21, 3},        /* spacing tolerance value = .5 DeltaT, */
                    /* and select code = value */
    8-1,            /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    4-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    150-1,          /* time-out clock divisor: 3.333us*150 = .5ms */
    60,             /* frame time-out = 60 ms */
    8,              /* edge time-out = 4 ms */
    8,              /* minimum dead-time after fault = 4 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,          /* data symbol timeout used with variable length data: 937*3.333us=3123us. After this time, we assume all data are captured */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#else
#ifdef XMP2_NEW_LE_TO_LE
/* This is the new XMP2 decoder.
 * This can decode a variable length IR input from leading edge to leading edge.
 * When used, this should decode XMXP-2 data and ack/nak IR input more reliably
 * than using falling edge since there are more jitter on the falling edge of IR input
 */
static const CIR_Param xmp2Param = {
    90-1,           /* count divisor: 90 divide by 27MHz for 3.333us period */
    { {0,0}, {0,0}, {0,0}, {0,0} }, /* pa[], 67*3.33=223us, 1=+/125% 2= +/- 50%, 3=use pulse tolerance*/
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    0,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    74,             /* pulse tolerance = 74*3.33 = 245us. Only used if tolerance code is 3 */
    291,            /* one cycle of data 0: 970us/3.33=291 */
    41,             /* delta T = 136.71us 41*3.333=136.653*/
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle*/
                    /* true => measure spacing between 2 consecutive edges */
    {116,3},        /* data symbol fix-width pulse period = 116*3.33=385us */
                    /* hilim = 500us+107us+23us=630us lolim = 214us-54us-20us=140 */
                    /* midpoint = (hilim+lolim)/2=385us. 385us/3.33us=116*/
                    /* tolerance 3 = use T0 tolerance value = (385-140)/3.33=74 */
    {21, 3},        /* spacing tolerance value = .5 DeltaT, */
                    /* and select code = value */
    0,              /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    4-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    150-1,          /* time-out clock divisor: 3.333us*150 = .5ms */
    60,             /* frame time-out = 60 ms */
    8,              /* edge time-out = 4 ms */
    8,              /* minimum dead-time after fault = 4 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    1090,           /* data symbol timeout used with variable length data: */
                    /* 1090*3.333us=3629us. After this time, we assume all data are captured */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#else
static const CIR_Param xmp2Param = {
    90-1,           /* count divisor: 90 divide by 27MHz for 3.333us period */
    { {70,2}, {0,0}, {0,0}, {0,0} },    /* pa[], 67*3.33=223us, 1=+/125% 2= +/- 50%, 3=use pulse tolerance*/
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    1,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    74,             /* pulse tolerance = 74*3.33 = 245us. Only used if tolerance code is 3 */
    205,            /* 205*3.33us=683us: T0 = 970us-214us=756us. Reduced it to make it work when on pulse is bigger and off pulse is smaller*/
    41,             /* delta T = 136.71us 41*3.333=136.653*/
    1,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    1,              /* false => measure spacing for complete cycle*/
                    /* true => measure spacing between 2 consecutive edges */
    {116,3},        /* data symbol fix-width pulse period = 397us */
                    /* hilim = 500us+107us+23us=630us lolim = 214us-54us-20us=140 */
                    /* midpoint = (hilim+lolim)/2=385us. 385us/3.33us=116*/
                    /* tolerance 2 = 50% 3 = use T0 tolerance value = 385-140/3.33=74 */
    {21, 3},        /* spacing tolerance value = .5 DeltaT, */
                    /* and select code = value */
    8-1,            /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    4-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    150-1,          /* time-out clock divisor: 3.333us*150 = .5ms */
    60,             /* frame time-out = 60 ms */
    8,              /* edge time-out = 4 ms */
    8,              /* minimum dead-time after fault = 4 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    1090,           /* data symbol timeout used with variable length data: */
                    /* 1090*3.333us=3629us. After this time, we assume all data are captured */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
#endif /* #ifdef XMP2_NEW_LE_TO_LE*/


#endif /* ifdef XMP2_NO_ACK */

static const CIR_Param xmp2AckParam = {
    90-1,           /* count divisor: 90 divide by 27MHz for 3.333us period */
    { {70,2}, {0,0}, {0,0}, {0,0} },    /* pa[], 67*3.33=223us, 1=+/125% 2= +/- 50%, 3=use purse tolerance*/
    { {0,0}, {0,0}, {0,0}, {0,0} },     /* pb[], preamble B pulse sequence */
    0,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    35,             /* pulse tolerance = 29*3.33 = 166.55us. Only used if tolerance code is 3 */
    291,            /* T0 = 970us (969.98+0*136.71) */
    41,             /* delta T = 136.71us 41*3.333=136.653*/
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {70,2},         /* data symbol fix-width pulse period = 0.280, */
                    /* tolerance 2 = 50% */
    {21, 3},        /* spacing tolerance value = .5 DeltaT, */
                    /* and select code = value */
    2-1,            /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    4-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    150-1,          /* time-out clock divisor: 3.333us*150 = .5ms */
    60,             /* frame time-out = 60 ms */
    8,              /* edge time-out = 4 ms */
    10,             /* minimum dead-time after fault = 5 ms. Make sure only 1 interrupt is received when 4 byte packet is received */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout used with variable length data: 972*3.333us=3239us. After this time, we assume all data are captured */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param rcaParam = {
    270-1,          /* count divisor 270/27Mhz = 10us */
    { {400,0}, {400,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    50,             /* pulse tolerance */
    150,            /* bit Period = 1.50 ms (PS T0) */
    100,                /* delta T = 1000us */
    0,              /*  - " - (symbol pulse position) */
    0,              /*  - " - (measure spacing for complete cycle) */
    {50, 1},        /*  - " - (data symbol fix-width pulse period) */
    {30, 3},        /* bit period tolerance value = not applicable, */
                    /* and select code = 12.5% */
    24-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    0,              /* variable length data */
    50-1,           /* time-out clock divisor = 10us*50 = 500us */
    200,            /* frame time-out */
    21,             /* edge time-out = 10ms*/
    13,             /* minimum dead-time after fault = 6 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    0,              /* data symbol timeout */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

static const CIR_Param toshibaTC9012Param = {
/*
 * Initial frame has 32 bits of data. Using variable length decoding, Repeat
 * code treated as frame with 1 bit of data.
 */
    270-1,      /* count divisor: divide by 270 for 10us/count */
    { {450,0}, {900,0}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    {   {0,0},   {0,0}, {0,0}, {0,0} }, /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    0,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    0,              /* pulse tolerance */
    112,            /* T0 = 1.12ms */
    113,            /* delta = 1.13ms */
    0,              /* false => fix symbol pulse between edges 0 & 1 */
                    /* true => fix symbol pulse between edges 1 & 2 */
    0,              /* false => measure spacing for complete cycle */
                    /* true => measure spacing between 2 consecutive edges */
    {56, 2},        /* data symbol fix-width pulse period and tolerance:
                       !!! TODO:adjust 50% */
    {0, 0},         /* spacing tolerance value = not applicable, */
                    /* and tolerance select code = 12.5% */
    0,              /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    0,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    0,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    0,              /* check stop symbol */
    1,              /* variable length data */
    50-1,           /* time-out clock divisor: 500us per time-out unit */
    200,            /* frame time-out: 100ms */
    16,             /* edge time-out: 8ms */
    16,             /* minimum dead-time after fault = 8 ms */
    {0, 0},         /* stop symbol pulse or cycle period */
    254,            /* data symbol timeout: T1 max + 2 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/* Tech4Home XIP protocol */
static const CIR_Param xipParam = {
    270-1,           /* count divisor: divide by 270 for 10.000 us period */
    { {34,1}, {68,1}, {0,0}, {0,0} }, /* pa[], preamble A pulse sequence */
    { {0,0}, {0,0}, {0,0}, {0,0} },       /* pb[], preamble B pulse sequence */
    2,              /* number of entries in pa[] */
    0,              /* number of entries in pb[] */
    1,              /* measure preamble pulse: */
                    /*  0 => even counts specifies cycle period */
                    /*  1 => even counts specifies off pulse period */
    0,              /* if true, pb[] matches a repeat sequence */
    29,             /* pulse tolerance = value not used */
    58,             /* bit Period = 580 us (PS T0) */
    0,              /* not used for bi-phase (PS delta T) */
    0,              /*  - " - (symbol pulse position) */
    0,              /*  - " - (measure spacing for complete cycle) */
    {0, 0},         /*  - " - (data symbol fix-width pulse period) */
    {0, 1},         /* bit period tolerance value = not applicable, */
                    /* and select code = 25% */
    26-1,           /* no. of symbols for sequence with preamble A */
    0,              /* no. of symbols for sequence with preamble B */
    1-1,            /* no. of data bits per symbol */
    1,              /* most/!least significant symbol received first */
    0,              /* left/!right adjust received data */
    1,              /* bi-phase/!pulse-spacing coded */
    0,              /* two symbols per cycle */
    1,              /* check stop symbol */
    0,              /* variable length data */
    100-1,          /* time-out clock divisor: divide by 100 for 1ms */
    20,             /* frame time-out = 20 ms */
    5,              /* edge time-out  = 5 ms */
    5,              /* minimum dead-time after fault = 5 ms */
    {71,1},         /* stop symbol pulse or cycle period */
    0,              /* Data symbol timeout */
    110,            /* Repeat timeout = 110ms. */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

BDBG_OBJECT_ID(BKIR_Handle);

typedef struct BKIR_P_Handle
{
    uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BDBG_OBJECT(BKIR_Handle)
    BCHP_Handle     hChip;
    BREG_Handle     hRegister;
    BINT_Handle     hInterrupt;
    unsigned int    numChannels;
    BKIR_ChannelHandle hKirChn[BKIR_N_CHANNELS];
} BKIR_P_Handle;

BDBG_OBJECT_ID(BKIR_ChannelHandle);

typedef struct BKIR_P_ChannelHandle
{
    uint32_t            magicId;                    /* Used to check if structure is corrupt */
    BDBG_OBJECT(BKIR_ChannelHandle)
    BKIR_Handle         hKir;
    uint32_t            chnNo;
    uint32_t            coreOffset;
    BKNI_EventHandle    hChnEvent;
    BINT_CallbackHandle hChnCallback;
    BKIR_KirPort        irPort;                     /* port select setting */
    BKIR_KirPort        irPortSelected;             /* which port was selected (auto mode) */
    bool                intMode;
    bool                repeatFlag;                 /* flag indicating remote A repeat condition */
    bool                isCirMode;
    BKIR_Callback       kirCb;                      /* callback function */
    void                *cbData;                    /* data passed to callback function */
    CIR_Param           customCirParam;
    BKIR_KirDevice      customDevice;               /* device that this custom cir is used for */
                                                    /* this flag will be used for special handling. */
    bool                cir_pa;                     /* preamble A is detected. */
    bool                cir_pb;                     /* preamble B is detected. */
} BKIR_P_ChannelHandle;



/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BKIR_ChannelSettings defKir0ChnSettings =
{
    BKIR_KirPortAuto,
    true,
    0
};

#if BKIR_N_CHANNELS > 1
static const BKIR_ChannelSettings defKir1ChnSettings =
{
    BKIR_KirPortAuto,
    true,
    0
};

#if BKIR_N_CHANNELS > 2
static const BKIR_ChannelSettings defKir2ChnSettings =
{
    BKIR_KirPortAuto,
    true,
    0
};

#if BKIR_N_CHANNELS > 3
static const BKIR_ChannelSettings defKir3ChnSettings =
{
    BKIR_KirPortAuto,
    true,
    0
};
#endif
#endif
#endif

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BKIR_Open(
    BKIR_Handle *pKir,                  /* [output] Returns handle */
    BCHP_Handle hChip,                  /* Chip handle */
    BREG_Handle hRegister,              /* Register handle */
    BINT_Handle hInterrupt,             /* Interrupt handle */
    const BKIR_Settings *pDefSettings   /* Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BKIR_Handle hDev;
    unsigned int chnIdx;

    BSTD_UNUSED(pDefSettings);

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInterrupt );

    /* Alloc memory from the system heap */
    hDev = (BKIR_Handle) BKNI_Malloc( sizeof( BKIR_P_Handle ) );
    if( hDev == NULL )
    {
        *pKir = NULL;
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BKIR_Open: BKNI_malloc() failed"));
        goto done;
    }

    BDBG_OBJECT_SET(hDev, BKIR_Handle);
    hDev->hChip     = hChip;
    hDev->hRegister = hRegister;
    hDev->hInterrupt = hInterrupt;
    hDev->numChannels  = BKIR_N_CHANNELS;
    for( chnIdx = 0; chnIdx < hDev->numChannels; chnIdx++ )
    {
        hDev->hKirChn[chnIdx] = NULL;
    }

    *pKir = hDev;

done:
    return( retCode );
}

BERR_Code BKIR_Close(
    BKIR_Handle hDev                    /* Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev, BKIR_Handle);

    BDBG_OBJECT_DESTROY(hDev, BKIR_Handle);
    BKNI_Free( (void *) hDev );

    return( retCode );
}

BERR_Code BKIR_GetDefaultSettings(
    BKIR_Settings *pDefSettings,        /* [output] Returns default setting */
    BCHP_Handle hChip                   /* Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChip);

    *pDefSettings = NULL; /*none available*/

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BKIR_GetTotalChannels(
    BKIR_Handle hDev,                   /* Device handle */
    unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev, BKIR_Handle);

    *totalChannels = hDev->numChannels;

    return( retCode );
}
#endif

BERR_Code BKIR_GetChannelDefaultSettings(
    BKIR_Handle hDev,                   /* Device handle */
    unsigned int channelNo,             /* Channel number to default setting for */
    BKIR_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

#if !BDBG_DEBUG_BUILD
    BSTD_UNUSED(hDev);
#endif
    BDBG_OBJECT_ASSERT(hDev, BKIR_Handle);

    switch (channelNo)
    {
        case 0:
            *pChnDefSettings = defKir0ChnSettings;
            break;

#if BKIR_N_CHANNELS > 1
        case 1:
            *pChnDefSettings = defKir1ChnSettings;
            break;

#if BKIR_N_CHANNELS > 2
        case 2:
            *pChnDefSettings = defKir2ChnSettings;
            break;

#if BKIR_N_CHANNELS > 3
        case 3:
            *pChnDefSettings = defKir3ChnSettings;
            break;
#endif
#endif
#endif

        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;

    }

    return( retCode );
}

BERR_Code BKIR_GetDefaultCirParam (
    BKIR_KirDevice      device,          /* device type to enable */
    CIR_Param           *pCustomCirParam /* [output] Returns default setting */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    const CIR_Param *pCirParam = NULL;

    switch (device)
    {
        case BKIR_KirDevice_eCirGI:
            pCirParam = &giParam;
            break;
        case BKIR_KirDevice_eCirSaE2050:
            pCirParam = &sa_e2050Param;
            break;
        case BKIR_KirDevice_eCirTwirp:
            pCirParam = &twirpParam;
            break;
        case BKIR_KirDevice_eCirSony:
            pCirParam = &sonyParam;
            break;
        case BKIR_KirDevice_eCirRecs80:
            pCirParam = &recs80Param;
            break;
        case BKIR_KirDevice_eCirRc5:
            pCirParam = &rc5Param;
            break;
        case BKIR_KirDevice_eCirUei:
            pCirParam = &ueiParam;
            break;
        case BKIR_KirDevice_eCirRfUei:
            pCirParam = &RfueiParam;
            break;
        case BKIR_KirDevice_eCirEchoStar:
            pCirParam = &echoDishRemoteParam;
            break;
        case BKIR_KirDevice_eCirNec:
            pCirParam = &necParam;
            break;
        case BKIR_KirDevice_eCirGISat:
            pCirParam = &giSatParam;
            break;
        case BKIR_KirDevice_eCirRC6:
            pCirParam = &rC6Mode6AParam;
            break;
        case BKIR_KirDevice_eCirDirectvUhfr:
            pCirParam = &directvUHFParam;
            break;
        case BKIR_KirDevice_eCirEchostarUhfr:
            pCirParam = &echostarUHFParam;
            break;
        case BKIR_KirDevice_eCirRcmmRcu:
            pCirParam = &s_RCMMParam;
            break;
        case BKIR_KirDevice_eCirRstep:
            pCirParam = &rStepParam;
            break;
        case BKIR_KirDevice_eCirXmp2:
            pCirParam = &xmp2Param;
            break;
        case BKIR_KirDevice_eCirXmp2Ack:
            pCirParam = &xmp2AckParam;
            break;
        case BKIR_KirDevice_eCirRC6Mode0:
            pCirParam = &rC6Mode0Param;
            break;
        case BKIR_KirDevice_eCirRca:
            pCirParam = &rcaParam;
            break;
        case BKIR_KirDevice_eCirToshibaTC9012:
            pCirParam = &toshibaTC9012Param;
            break;
        case BKIR_KirDevice_eCirXip:
            pCirParam = &xipParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }
    if (retCode == BERR_SUCCESS)
    {
        BKNI_Memcpy ((void *)pCustomCirParam, (void *)pCirParam, sizeof(CIR_Param));
    }

    return( retCode );
}

BERR_Code BKIR_GetCurrentCirParam (
    BKIR_ChannelHandle  hChn,            /* Device channel handle */
    BKIR_KirDevice      device,          /* device type to enable */
    CIR_Param           *pCustomCirParam /* [output] Returns current setting */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    const CIR_Param *pCirParam = NULL;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    if (device == BKIR_KirDevice_eCirCustom) {
        pCirParam = &(hChn->customCirParam);
        BKNI_Memcpy ((void *)pCustomCirParam, (void *)pCirParam, sizeof(CIR_Param));
    } else
        retCode = BKIR_GetDefaultCirParam(device, pCustomCirParam);

    return( retCode );
}

BERR_Code BKIR_OpenChannel(
    BKIR_Handle hDev,                   /* Device handle */
    BKIR_ChannelHandle *phChn,          /* [output] Returns channel handle */
    unsigned int channelNo,             /* Channel number to open */
    const BKIR_ChannelSettings *pChnDefSettings /* Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BKIR_ChannelHandle hChnDev;

    BDBG_OBJECT_ASSERT(hDev, BKIR_Handle);

    hChnDev = NULL;

    if( channelNo < hDev->numChannels )
    {
        if( hDev->hKirChn[channelNo] == NULL )
        {
            /* Alloc memory from the system heap */
            hChnDev = (BKIR_ChannelHandle) BKNI_Malloc( sizeof( BKIR_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                *phChn = NULL;
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BKIR_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }

            BDBG_OBJECT_INIT(hChnDev, BKIR_ChannelHandle);

            BKIR_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hChnDev->hChnEvent) ) );
            hChnDev->hKir       = hDev;
            hChnDev->chnNo      = channelNo;
            hChnDev->irPort     = pChnDefSettings->irPort;
            hChnDev->isCirMode  = false;
            hChnDev->kirCb      = NULL;
            hChnDev->cbData     = NULL;
            hChnDev->customDevice = pChnDefSettings->customDevice;
            hDev->hKirChn[channelNo] = hChnDev;

            /* Offsets are based off of BSCA */
            if (channelNo == 0)
                hChnDev->coreOffset = 0;
#if BKIR_N_CHANNELS > 1
            else if (channelNo == 1)
                hChnDev->coreOffset = BCHP_KBD2_STATUS - BCHP_KBD1_STATUS;
#if BKIR_N_CHANNELS > 2
            else if (channelNo == 2)
                hChnDev->coreOffset = BCHP_KBD3_STATUS - BCHP_KBD1_STATUS;
#if BKIR_N_CHANNELS > 3
            else if (channelNo == 3)
                hChnDev->coreOffset = BCHP_KBD4_STATUS - BCHP_KBD1_STATUS;
#endif
#endif
#endif
            /* initialize the CMD register to the default value. */
            BREG_Write32(hDev->hRegister, hChnDev->coreOffset + BCHP_KBD1_CMD, 0x07);

            /* Enable interrupt for this channel */
            hChnDev->intMode = pChnDefSettings->intMode;
            if (hChnDev->intMode == true)
            {
                static const BINT_Id IntId[BKIR_N_CHANNELS] =
                {
                        INT_ID_0
#if BKIR_N_CHANNELS > 1
                        ,INT_ID_1
#if BKIR_N_CHANNELS > 2
                        ,INT_ID_2
#if BKIR_N_CHANNELS > 3
                        ,INT_ID_3
#endif
#endif
#endif
                };

                /* Register and enable L2 interrupt.  */
                BKIR_CHK_RETCODE( retCode, BINT_CreateCallback(
                    &(hChnDev->hChnCallback), hDev->hInterrupt, IntId[channelNo],
                    BKIR_P_HandleInterrupt_Isr, (void *) hChnDev, 0x00 ) );
                BKIR_CHK_RETCODE( retCode, BINT_EnableCallback( hChnDev->hChnCallback ) );

                /* Enable KIR interrupt */
                BKNI_EnterCriticalSection();
                BKIR_P_EnableInt (hChnDev);
                BKNI_LeaveCriticalSection();
            }
            else
            {
                hChnDev->hChnCallback = NULL;

                /* Disable KIR interrupt */
                BKNI_EnterCriticalSection();
                BKIR_P_DisableInt (hChnDev);
                BKNI_LeaveCriticalSection();
            }

            *phChn = hChnDev;
        }
        else
        {
            retCode = BERR_TRACE(BKIR_ERR_NOTAVAIL_CHN_NO);
        }
    }
    else
    {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChnDev != NULL )
        {
            BKNI_DestroyEvent( hChnDev->hChnEvent );
            BKNI_Free( hChnDev );
            hDev->hKirChn[channelNo] = NULL;
            *phChn = NULL;
        }
    }
    return( retCode );
}

BERR_Code BKIR_CloseChannel(
    BKIR_ChannelHandle hChn         /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BKIR_Handle hDev;
    unsigned int chnNo;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    /* Disable interrupt for this channel */
    BKNI_EnterCriticalSection();
    BKIR_P_DisableInt (hChn);
    BKNI_LeaveCriticalSection();

    if (hChn->hChnCallback) {
        BKIR_CHK_RETCODE( retCode, BINT_DisableCallback( hChn->hChnCallback ) );
        BKIR_CHK_RETCODE( retCode, BINT_DestroyCallback( hChn->hChnCallback ) );
    }

    hChn->kirCb     = NULL;
    hChn->cbData    = NULL;
    BKNI_DestroyEvent( hChn->hChnEvent );
    chnNo = hChn->chnNo;
    BDBG_OBJECT_DESTROY(hChn, BKIR_ChannelHandle);
    BKNI_Free( hChn );

    hDev->hKirChn[chnNo] = NULL;

done:
    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BKIR_GetDevice(
    BKIR_ChannelHandle hChn,            /* Device channel handle */
    BKIR_Handle *phDev                  /* [output] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    *phDev = hChn->hKir;

    return( retCode );
}
#endif

BERR_Code BKIR_Set_PM_AON_CONFIG(
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_PM_APN_Config_InputChannel  channel_device,
    BKIR_InputDevice  input_device

    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    uint32_t field_value;
    BKIR_Handle hDev;

    BDBG_ASSERT( hChn );

    hDev = hChn->hKir;
    lval = BREG_Read32( hDev->hRegister, BCHP_PM_AON_CONFIG );

    if (input_device >= BKIR_INPUT_MAX) {
        BDBG_ERR(("BKIR_Set_PM_AON_CONFIG: Invalid input_device\n"));
        return BERR_INVALID_PARAMETER;
    }

    switch (channel_device)
    {
#ifdef BKIR_HAS_ZERO_BASE
    case BKIR_INPUT_PM_APN_CONFIG_IRR0:
        switch (input_device)
        {
        case BKIR_INPUT_AON_GPIO: field_value = BCHP_PM_AON_CONFIG_irr0_in_AON_GPIO ; break;
#ifdef HAS_UHF
        case BKIR_INPUT_UHF_RX:     field_value = BCHP_PM_AON_CONFIG_irr0_in_UHF_RX1 ; break;
#endif
        case BKIR_INPUT_IR_IN1:     field_value = BCHP_PM_AON_CONFIG_irr0_in_IR_IN1 ; break;
        case BKIR_INPUT_IR_IN0:
        default:                    field_value = BCHP_PM_AON_CONFIG_irr0_in_IR_IN0 ; break;
        }
#ifdef HAS_UHF
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr1_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr1_in, field_value);
#else
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr0_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr0_in, field_value);
#endif
        break;
#else  /* not BKIR_HAS_ZERO_BASE */
    case BKIR_INPUT_PM_APN_CONFIG_IRR3:
        switch (input_device)
        {
        case BKIR_INPUT_AON_GPIO: field_value = BCHP_PM_AON_CONFIG_irr3_in_AON_GPIO ; break;
#ifdef HAS_UHF_3
        case BKIR_INPUT_UHF_RX:     field_value = BCHP_PM_AON_CONFIG_irr3_in_UHF_RX1 ; break;
#endif
#ifndef NO_IR_IN1
        case BKIR_INPUT_IR_IN1:     field_value = BCHP_PM_AON_CONFIG_irr3_in_IR_IN1  ; break;
#endif
        case BKIR_INPUT_IR_IN0:
        default:                    field_value = BCHP_PM_AON_CONFIG_irr3_in_IR_IN0 ; break;
        }
#ifdef HAS_UHF_3
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr3_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr3_in, field_value);
#else
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr2_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr2_in, field_value);
#endif

#endif  /* end of BKIR_HAS_ZERO_BASE */

    case BKIR_INPUT_PM_APN_CONFIG_IRR1:
        switch (input_device)
        {
        case BKIR_INPUT_AON_GPIO: field_value = BCHP_PM_AON_CONFIG_irr1_in_AON_GPIO ; break;
#ifdef HAS_UHF
        case BKIR_INPUT_UHF_RX:     field_value = BCHP_PM_AON_CONFIG_irr1_in_UHF_RX1 ; break;
#endif
#ifndef NO_IR_IN1
        case BKIR_INPUT_IR_IN1:     field_value = BCHP_PM_AON_CONFIG_irr1_in_IR_IN1 ; break;
#endif
        case BKIR_INPUT_IR_IN0:
        default:                    field_value = BCHP_PM_AON_CONFIG_irr1_in_IR_IN0 ; break;
        }
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr1_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr1_in, field_value);
        break;
    case BKIR_INPUT_PM_APN_CONFIG_IRR2:
        switch (input_device)
        {
        case BKIR_INPUT_AON_GPIO: field_value = BCHP_PM_AON_CONFIG_irr2_in_AON_GPIO; break;
#ifdef HAS_UHF
        case BKIR_INPUT_UHF_RX:     field_value = BCHP_PM_AON_CONFIG_irr2_in_UHF_RX1 ; break;
#endif
#ifndef NO_IR_IN1
        case BKIR_INPUT_IR_IN1:     field_value = BCHP_PM_AON_CONFIG_irr2_in_IR_IN1 ; break;
#endif
        case BKIR_INPUT_IR_IN0:
        default:                    field_value = BCHP_PM_AON_CONFIG_irr2_in_IR_IN0 ; break;
        }
#ifdef HAS_UHF
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr2_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr2_in, field_value);
#else
        lval &= ~ BCHP_MASK(PM_AON_CONFIG, irr1_in);
        lval |= BCHP_FIELD_DATA(PM_AON_CONFIG, irr1_in, field_value);
#endif
        break;
    default:
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }
    BDBG_MSG(("ch_device=%u input_device=%u lval=0x%8x ", channel_device, input_device, lval ));
    BREG_Write32( hDev->hRegister, BCHP_PM_AON_CONFIG, lval );

done:
        return( retCode );
}

BERR_Code BKIR_EnableIrDevice (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_KirDevice      device          /* device type to enable */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;
    bool isCirDevice = false;
    BKIR_KirDevice tempDevice;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);
    switch( device )
    {
        case BKIR_KirDevice_eTwirpKbd:
            lval = lval | KBD_CMD_TWIRP_ENABLE;
            break;

        case BKIR_KirDevice_eSejin38KhzKbd:
            lval = (lval | KBD_CMD_SEJIN_ENABLE) & ~BCHP_KBD1_CMD_alt_table_MASK;
            break;

        case BKIR_KirDevice_eSejin56KhzKbd:
            lval = lval | KBD_CMD_SEJIN_ENABLE | BCHP_KBD1_CMD_alt_table_MASK;
            break;

        case BKIR_KirDevice_eRemoteA:
            lval = lval | KBD_CMD_REMOTE_A_ENABLE;
            break;

        case BKIR_KirDevice_eRemoteB:
            lval = lval | KBD_CMD_REMOTE_B_ENABLE;
            break;

        case BKIR_KirDevice_eCirGI:
        case BKIR_KirDevice_eCirSaE2050:
        case BKIR_KirDevice_eCirTwirp:
        case BKIR_KirDevice_eCirSony:
        case BKIR_KirDevice_eCirRecs80:
        case BKIR_KirDevice_eCirRc5:
        case BKIR_KirDevice_eCirUei:
        case BKIR_KirDevice_eCirRfUei:
        case BKIR_KirDevice_eCirEchoStar:
        case BKIR_KirDevice_eCirNec:
        case BKIR_KirDevice_eCirGISat:
        case BKIR_KirDevice_eCirCustom:
        case BKIR_KirDevice_eCirDirectvUhfr:
        case BKIR_KirDevice_eCirEchostarUhfr:
        case BKIR_KirDevice_eCirRC6:
        case BKIR_KirDevice_eCirRcmmRcu:
        case BKIR_KirDevice_eCirRstep:
        case BKIR_KirDevice_eCirXmp2:
        case BKIR_KirDevice_eCirXmp2Ack:
        case BKIR_KirDevice_eCirRC6Mode0:
        case BKIR_KirDevice_eCirRca:
        case BKIR_KirDevice_eCirToshibaTC9012:
        case BKIR_KirDevice_eCirXip:
            isCirDevice = true;
            if ((device == BKIR_KirDevice_eCirUei) || (device == BKIR_KirDevice_eCirDirectvUhfr) || (device == BKIR_KirDevice_eCirRfUei)
                || (device == BKIR_KirDevice_eCirNec) )
            {
                /* This flag is used to read repeat flag in ISR.
                 * This flag should apply to all CIR devices.
                 * Until, we test more CIR devices, use it for
                 * UEI and NEC device only.
                 */
                hChn->isCirMode = true;
            }
            if (device == BKIR_KirDevice_eCirCustom)
            {
                if ((hChn->customDevice == BKIR_KirDevice_eCirUei) || (hChn->customDevice == BKIR_KirDevice_eCirDirectvUhfr)
                    || (hChn->customDevice == BKIR_KirDevice_eCirRfUei) || (device == BKIR_KirDevice_eCirNec)  )
                {
                    /* This flag is used to read repeat flag in ISR.
                     * This flag should apply to all CIR devices.
                     * Until, we test more CIR devices, use it for
                     * UEI and NEC device only.
                     */
                    hChn->isCirMode = true;
                }
            }
            lval = lval | KBD_CMD_CIR_ENABLE;
            if ((device == BKIR_KirDevice_eCirRfUei) ||
                (device == BKIR_KirDevice_eCirDirectvUhfr) ||
                (device == BKIR_KirDevice_eCirEchostarUhfr) ||
                (device == BKIR_KirDevice_eCirRC6))
                lval &= 0xf0;       /* disable TWIRP, SEJIN, REMOTE A, and REMOTE B */

            if (device == BKIR_KirDevice_eCirCustom)
            {
                if ((hChn->customDevice == BKIR_KirDevice_eCirRfUei) ||
                    (hChn->customDevice == BKIR_KirDevice_eCirDirectvUhfr) ||
                    (hChn->customDevice == BKIR_KirDevice_eCirEchostarUhfr) ||
                    (hChn->customDevice == BKIR_KirDevice_eCirRC6))
                    lval &= 0xf0;       /* disable TWIRP, SEJIN, REMOTE A, and REMOTE B */
            }
            break;

        default:
            retCode = BERR_INVALID_PARAMETER;
            goto done;
    }

    BDBG_MSG(("%s Write32(%x, %x)", BSTD_FUNCTION, hChn->coreOffset + BCHP_KBD1_CMD, lval));
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);

    /* Set KBD3.FILTER1 to 0x0000007F for UHF */
    tempDevice = device; /* to do special handling */
    if (device == BKIR_KirDevice_eCirCustom)
    {
        /* Use custom device to set special filter */
        tempDevice = hChn->customDevice ;
    }
    switch ( tempDevice )
    {
        case BKIR_KirDevice_eCirDirectvUhfr:
        case BKIR_KirDevice_eCirEchostarUhfr:
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, 0x0000003F | BCHP_KBD1_FILTER1_filter_en_MASK);
            break;
        case BKIR_KirDevice_eCirUei:
            BDBG_MSG(("%s Write32(%x, %x -> %x)", BSTD_FUNCTION, hChn->coreOffset + BCHP_KBD1_FILTER1, BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1), 0x00000034 | BCHP_KBD1_FILTER1_filter_en_MASK));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, 0x00000034 | BCHP_KBD1_FILTER1_filter_en_MASK);
            break;
        case BKIR_KirDevice_eCirXmp2:
        case BKIR_KirDevice_eCirXmp2Ack:
            BDBG_MSG(("%s Write32(%x, %x -> %x)", BSTD_FUNCTION, hChn->coreOffset + BCHP_KBD1_FILTER1, BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1), 0x00000011 | BCHP_KBD1_FILTER1_filter_en_MASK));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, 0x00000011 | BCHP_KBD1_FILTER1_filter_en_MASK);
            break;
        default:
            BDBG_MSG(("%s Write32(%x, %x)", BSTD_FUNCTION, hChn->coreOffset + BCHP_KBD1_FILTER1, 0x00000000 & ~BCHP_KBD1_FILTER1_filter_en_MASK));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, 0x00000000 & ~BCHP_KBD1_FILTER1_filter_en_MASK);
            break;
   }

    /* Program CIR reg file for CIR devices */
    if (isCirDevice)
    {
        BKIR_P_ConfigCir (hChn, device);
    }

done:

    return( retCode );
}

BERR_Code BKIR_DisableIrDevice (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_KirDevice      device          /* device type to enable */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);

    switch( device )
    {
        case BKIR_KirDevice_eTwirpKbd:
            lval = lval & ~KBD_CMD_TWIRP_ENABLE;
            break;

        case BKIR_KirDevice_eSejin38KhzKbd:
        case BKIR_KirDevice_eSejin56KhzKbd:
            lval = lval & ~KBD_CMD_SEJIN_ENABLE;
            break;

        case BKIR_KirDevice_eRemoteA:
            lval = lval & ~KBD_CMD_REMOTE_A_ENABLE;
            break;

        case BKIR_KirDevice_eRemoteB:
            lval = lval & ~KBD_CMD_REMOTE_B_ENABLE;
            break;

        case BKIR_KirDevice_eCirGI:
        case BKIR_KirDevice_eCirSaE2050:
        case BKIR_KirDevice_eCirTwirp:
        case BKIR_KirDevice_eCirSony:
        case BKIR_KirDevice_eCirRecs80:
        case BKIR_KirDevice_eCirRc5:
        case BKIR_KirDevice_eCirUei:
        case BKIR_KirDevice_eCirRfUei:
        case BKIR_KirDevice_eCirEchoStar:
        case BKIR_KirDevice_eCirNec:
        case BKIR_KirDevice_eCirGISat:
        case BKIR_KirDevice_eCirCustom:
        case BKIR_KirDevice_eCirDirectvUhfr:
        case BKIR_KirDevice_eCirEchostarUhfr:
        case BKIR_KirDevice_eCirRC6:
        case BKIR_KirDevice_eCirRcmmRcu:
        case BKIR_KirDevice_eCirRstep:
        case BKIR_KirDevice_eCirXmp2:
        case BKIR_KirDevice_eCirXmp2Ack:
        case BKIR_KirDevice_eCirRC6Mode0:
        case BKIR_KirDevice_eCirRca:
        case BKIR_KirDevice_eCirToshibaTC9012:
        case BKIR_KirDevice_eCirXip:
            lval = lval & ~KBD_CMD_CIR_ENABLE;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto done;
    }

    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);
done:
    return( retCode );
}

BERR_Code BKIR_EnableDataFilter (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    uint64_t            pat0,       /* pattern0 to match (only least significant 48bits are used) */
    uint64_t            pat1,       /* pattern1 to match (only least significant 48bits are used) */
    uint64_t            mask0,      /* don't care bits in pattern0 (only least significant 48bits are used) */
    uint64_t            mask1       /* don't care bits in pattern1 (only least significant 48bits are used) */
    )
{
    #ifdef BCHP_KBD1_CMD_data_filtering_MASK
        uint32_t lval;
        BKIR_Handle hDev;

        BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

        hDev = hChn->hKir;

        #ifdef BCHP_KBD1_KBD_MASK2_kbd_mask0_MASK
            /* write 48 bit pattern to pattern and mask bits */
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_PAT0, pat0 & 0xffffffff);
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_PAT1, ((pat1 & 0xffff) << 16) | ((pat0 >> 32) & 0xffff));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_PAT2, ((pat1 >> 16) & 0xffffffff));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_MASK0, (mask0 & 0xffffffff));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_MASK1, ((mask1 & 0xffff) << 16) | ((mask0 >> 32) & 0xffff));
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_MASK2, ((mask1 >> 16) & 0xffffffff));
        #else
            BSTD_UNUSED(pat1);

            /* not all chips support the second filter, so return error if the user implies usage */
            if (~mask1)
                return( BERR_NOT_SUPPORTED );

            /* write pattern to match and mask bits */
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_PAT0, pat0 & 0xffffffff);
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_PAT1, (pat0 >> 32) & 0xffff);
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_MASK0, mask0 & 0xffffffff);
            BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_KBD_MASK1, (mask0 >> 32) & 0xffff);
        #endif

        /* enable data filtering based on pattern */
        lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);
        lval = (lval | BCHP_KBD1_CMD_data_filtering_MASK);
        BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);

        return( BERR_SUCCESS );
    #else
        BSTD_UNUSED(hChn);
        BSTD_UNUSED(pat0);
        BSTD_UNUSED(pat1);
        BSTD_UNUSED(mask0);
        BSTD_UNUSED(mask1);
        return( BERR_NOT_SUPPORTED );
    #endif
}

BERR_Code BKIR_DisableDataFilter (
    BKIR_ChannelHandle  hChn        /* Device channel handle */
    )
{
#ifdef BCHP_KBD1_CMD_data_filtering_MASK
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    /* disable data filtering */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);
    lval = (lval & ~BCHP_KBD1_CMD_data_filtering_MASK);
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);

    return( BERR_SUCCESS );
#else
    BSTD_UNUSED(hChn);
    return( BERR_NOT_SUPPORTED );
#endif
}

BERR_Code BKIR_EnableFilter1 (
    BKIR_ChannelHandle  hChn,           /* Device channel handle */
    unsigned int        filter_width    /* filter width if smaller than this to be rejected */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    /* enable filter1 and write filter width. */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1);
    lval &= ~BCHP_KBD1_FILTER1_filter_en_MASK;
    lval &= ~BCHP_KBD1_FILTER1_filter_width_MASK;
    lval = (lval | BCHP_KBD1_FILTER1_filter_en_MASK | filter_width);
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, lval);

    return( retCode );
}

BERR_Code BKIR_DisableFilter1 (
    BKIR_ChannelHandle  hChn        /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    /* disable filter1 and clear filter width. */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1);
    lval &= ~BCHP_KBD1_FILTER1_filter_en_MASK;
    lval &= ~BCHP_KBD1_FILTER1_filter_width_MASK;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_FILTER1, lval);

    return( retCode );
}

#if !B_REFSW_MINIMAL
BERR_Code BKIR_DisableAllIrDevices (
    BKIR_ChannelHandle  hChn        /* Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);

    lval &= ~(  KBD_CMD_TWIRP_ENABLE |
            KBD_CMD_SEJIN_ENABLE |
            KBD_CMD_REMOTE_A_ENABLE |
            KBD_CMD_REMOTE_B_ENABLE |
            KBD_CMD_CIR_ENABLE);

    BDBG_WRN(("%s Write32(%x, %x)", BSTD_FUNCTION, hChn->coreOffset + BCHP_KBD1_CMD, lval));
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);

    return( retCode );
}

BERR_Code BKIR_GetEventHandle(
    BKIR_ChannelHandle hChn,            /* Device channel handle */
    BKNI_EventHandle *phEvent           /* [output] Returns event handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    *phEvent = hChn->hChnEvent;

    return( retCode );
}

BERR_Code BKIR_IsDataReady (
    BKIR_ChannelHandle  hChn,           /* Device channel handle */
    bool                *dataReady      /* [output] flag to indicate if data is ready */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    /*
     * This function should only be called when polling for data ready.
     * If interrupt is enabled, the caller should pend on interrupt event
     */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS);
    *dataReady = (lval & BCHP_KBD1_STATUS_irq_MASK) ? true : false;

    return( retCode );
}
#endif

BERR_Code BKIR_Read_isr(
    BKIR_ChannelHandle      hChn,           /* Device channel handle */
    BKIR_KirInterruptDevice *pDevice,       /* [output] pointer to IR device type that generated the key */
    unsigned char           *data           /* [output] pointer to data received */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t lval;
    BKIR_Handle hDev;
    uint32_t data0;
    uint32_t data1;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS);
    *pDevice = (BKIR_KirInterruptDevice)((lval & KBD_STATUS_DEVICE_MASK) >> KBD_STATUS_DEVICE_SHIFTS);

    data0 = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_DATA0);
    data1 = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_DATA1);
    data[0] = data0       & 0xff;
    data[1] = data0 >> 8  & 0xff;
    data[2] = data0 >> 16 & 0xff;
    data[3] = data0 >> 24 & 0xff;
    data[4] = data1       & 0xff;
#if BCHP_KBD1_CMD_data_filtering_MASK
    /* 7550 can store up to 48 bit */
    data[5] = data1 >> 8  & 0xff;
#endif

    return( retCode );
}


#if !B_REFSW_MINIMAL
BERR_Code BKIR_Read(
    BKIR_ChannelHandle      hChn,           /* Device channel handle */
    BKIR_KirInterruptDevice *pDevice,       /* [output] pointer to IR device type that generated the key */
    unsigned char           *data           /* [output] pointer to data received */
    )
{
    BERR_Code rc;

    BKNI_EnterCriticalSection();
    rc = BKIR_Read_isr(hChn, pDevice, data);
    BKNI_LeaveCriticalSection();

    return rc;
}
#endif

BERR_Code BKIR_IsRepeated_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *repeatFlag     /* [out] flag to remote A repeat condition */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    *repeatFlag = hChn->repeatFlag;

    return( retCode );
}

BERR_Code BKIR_IsPreambleA_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *preambleFlag   /* [out] flag to remote A repeat condition */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    *preambleFlag = hChn->cir_pa;

    return( retCode );
}

BERR_Code BKIR_IsPreambleB_isrsafe(
    BKIR_ChannelHandle      hChn,           /* [in] Device channel handle */
    bool                    *preambleFlag   /* [out] flag to remote A repeat condition */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    *preambleFlag = hChn->cir_pb;

    return( retCode );
}

void BKIR_GetLastKey(
    BKIR_ChannelHandle hChn,  /* [in] Device channel handle */
    uint32_t *code,           /* [out] lower 32-bits of returned code */
    uint32_t *codeHigh,       /* [out] upper 32-bits of returned code */
    bool *preambleA,          /* [out] flag for preamble A */
    bool *preambleB           /* [out] flag for preamble B */
    )
{
    BKIR_Handle hDev;
    uint32_t lval;

    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hDev = hChn->hKir;

    *code     = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_DATA0);
    *codeHigh = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_DATA1);

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS);

    *preambleA = (lval & BCHP_KBD1_STATUS_cir_pa_MASK) ? true : false;
    *preambleB = (lval & BCHP_KBD1_STATUS_cir_pb_MASK) ? true : false;
}

void BKIR_SetCustomDeviceType (
    BKIR_ChannelHandle  hChn,           /* Device channel handle */
    BKIR_KirDevice      customDevice    /* device that this custom cir is used for */
)
{
    hChn->customDevice = customDevice;
}

void BKIR_SetCustomCir (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    CIR_Param           *pCirParam
)
{
    BKNI_Memcpy ((void *)&(hChn->customCirParam), (void *)pCirParam, sizeof(CIR_Param));
}

void BKIR_RegisterCallback (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_Callback       callback,   /* Callback function to register */
    void                *pData      /* Data passed to callback function */
)
{
    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hChn->kirCb = callback;
    hChn->cbData = pData;
}

void BKIR_UnregisterCallback (
    BKIR_ChannelHandle  hChn        /* Device channel handle */
)
{
    BDBG_OBJECT_ASSERT(hChn, BKIR_ChannelHandle);

    hChn->kirCb = NULL;
    hChn->cbData = NULL;
}

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
void BKIR_P_EnableInt(
    BKIR_ChannelHandle  hChn            /* Device channel handle */
    )
{
    uint32_t lval;
    BKIR_Handle hDev;

    hDev = hChn->hKir;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS, 0);  /* clear any previous interrupt */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);
    lval |= BCHP_KBD1_CMD_kbd_irqen_MASK;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);
}

void BKIR_P_DisableInt(
    BKIR_ChannelHandle  hChn            /* Device channel handle */
    )
{
    uint32_t lval;
    BKIR_Handle hDev;

    hDev = hChn->hKir;

    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD);
    lval &= ~BCHP_KBD1_CMD_kbd_irqen_MASK;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CMD, lval);

}

static void BKIR_P_HandleInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BKIR_ChannelHandle  hChn;
    BKIR_Handle hDev;
    uint32_t lval;

    BSTD_UNUSED(parm2);

    hChn = (BKIR_ChannelHandle) pParam1;
    BDBG_ASSERT( hChn );

    hDev = hChn->hKir;

    /* Clear interrupt */
    lval = BREG_Read32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS);

    /* Check if preamble is detected */
    hChn->cir_pa = false;
    hChn->cir_pb = false;

    /* If we're in Cir Mode then test the 'cir_pb' status for repeat keys. */
    if ( hChn->isCirMode )
    {
        hChn->cir_pa = (lval & BCHP_KBD1_STATUS_cir_pa_MASK) ? true : false;
        hChn->cir_pb = (lval & BCHP_KBD1_STATUS_cir_pb_MASK) ? true : false;
        hChn->repeatFlag = (lval & BCHP_KBD1_STATUS_cir_pb_MASK) ? true : false;
    }
    else
    {
        hChn->cir_pa = (lval & BCHP_KBD1_STATUS_cir_pa_MASK) ? true : false;
        hChn->cir_pb = (lval & BCHP_KBD1_STATUS_cir_pb_MASK) ? true : false;
        hChn->repeatFlag = (lval & BCHP_KBD1_STATUS_rflag_MASK) ? true : false;
    }

    lval &= ~BCHP_KBD1_STATUS_irq_MASK;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_STATUS, lval);

    /* Call any callback function if installed */
    if (hChn->kirCb)
    {
        (*hChn->kirCb)( hChn, hChn->cbData );
    }

    BKNI_SetEvent( hChn->hChnEvent );
    return;
}

void BKIR_P_WriteCirParam (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    uint32_t                addr,
    uint32_t                data
)
{
    BKIR_Handle hDev;

    hDev = hChn->hKir;
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CIR_ADDR, addr);
    /* Note: MOST of the data registers are 12-bits in length, but some are larger (don't mask) */
    BREG_Write32(hDev->hRegister, hChn->coreOffset + BCHP_KBD1_CIR_DATA, data);
}

void BKIR_P_ConfigCir (
    BKIR_ChannelHandle  hChn,       /* Device channel handle */
    BKIR_KirDevice      device      /* device type to enable */
)
{
    uint32_t ulData;
    uint32_t uli, ulj;
    const CIR_Param *pCirParam;

    switch (device)
    {
        case BKIR_KirDevice_eCirGI:
            pCirParam = &giParam;
            break;
        case BKIR_KirDevice_eCirSaE2050:
            pCirParam = &sa_e2050Param;
            break;
        case BKIR_KirDevice_eCirTwirp:
            pCirParam = &twirpParam;
            break;
        case BKIR_KirDevice_eCirSony:
            pCirParam = &sonyParam;
            break;
        case BKIR_KirDevice_eCirRecs80:
            pCirParam = &recs80Param;
            break;
        case BKIR_KirDevice_eCirRc5:
            pCirParam = &rc5Param;
            break;
        case BKIR_KirDevice_eCirUei:
            pCirParam = &ueiParam;
            break;
        case BKIR_KirDevice_eCirRfUei:
            pCirParam = &RfueiParam;
            break;
        case BKIR_KirDevice_eCirEchoStar:
            pCirParam = &echoDishRemoteParam;
            break;
        case BKIR_KirDevice_eCirNec:
            pCirParam = &necParam;
            break;
        case BKIR_KirDevice_eCirGISat:
            pCirParam = &giSatParam;
            break;
        case BKIR_KirDevice_eCirCustom:
            pCirParam = &(hChn->customCirParam);
            break;
        case BKIR_KirDevice_eCirRC6:
            pCirParam = &rC6Mode6AParam;
            break;
        case BKIR_KirDevice_eCirDirectvUhfr:
            pCirParam = &directvUHFParam;
            break;
        case BKIR_KirDevice_eCirEchostarUhfr:
            pCirParam = &echostarUHFParam;
            break;
        case BKIR_KirDevice_eCirRcmmRcu:
            pCirParam = &s_RCMMParam;
            break;
        case BKIR_KirDevice_eCirRstep:
            pCirParam = &rStepParam;
            break;
        case BKIR_KirDevice_eCirXmp2:
            pCirParam = &xmp2Param;
            break;
        case BKIR_KirDevice_eCirXmp2Ack:
            pCirParam = &xmp2AckParam;
            break;
        case BKIR_KirDevice_eCirRC6Mode0:
            pCirParam = &rC6Mode0Param;
            break;
        case BKIR_KirDevice_eCirRca:
            pCirParam = &rcaParam;
            break;
        case BKIR_KirDevice_eCirToshibaTC9012:
            pCirParam = &toshibaTC9012Param;
            break;
        case BKIR_KirDevice_eCirXip:
            pCirParam = &xipParam;
            break;
        default:
            return;
    }

    BKIR_P_WriteCirParam (hChn, 0, pCirParam->frameTimeout);

    ulData = (pCirParam->stop.tol << 10) | (pCirParam->stop.val);
    BKIR_P_WriteCirParam(hChn, 1, ulData);

    ulData = (pCirParam->pbCount << 3) | pCirParam->paCount;
    BKIR_P_WriteCirParam(hChn, 2, ulData);

    for (uli=0, ulj=3; uli<4; uli++) {
        ulData = ((pCirParam->pa)[uli].tol << 10) | (pCirParam->pa)[uli].val;
        BKIR_P_WriteCirParam(hChn, ulj++, ulData);
        ulData = ((pCirParam->pb)[uli].tol << 10) | (pCirParam->pb)[uli].val;
        BKIR_P_WriteCirParam(hChn, ulj++, ulData);
    }

    /* 6th bit needed 48 bit support for symbol A is in bit 11 for backward compatibility. */
    ulData = (pCirParam->nSymB << 5) | (pCirParam->nSymA & 0x1F) | ((pCirParam->nSymA >> 5) << 11);
    BKIR_P_WriteCirParam(hChn, 11, ulData);

    ulData = (pCirParam->symPulseWidth.tol << 10) | pCirParam->symPulseWidth.val;
    BKIR_P_WriteCirParam(hChn, 12, ulData);

    ulData = (pCirParam->spacingTol.tol << 10) | pCirParam->spacingTol.val;
    BKIR_P_WriteCirParam(hChn, 13, ulData);

    BKIR_P_WriteCirParam(hChn, 14, pCirParam->t0);
    BKIR_P_WriteCirParam(hChn, 15, pCirParam->delT);
    BKIR_P_WriteCirParam(hChn, 16, pCirParam->countDivisor);
    BKIR_P_WriteCirParam(hChn, 17, pCirParam->pulseTol);

    ulData = (pCirParam->varLenData << 11)
        | (pCirParam->chkStopSym << 10)
        | (pCirParam->twoSymPerCy << 9)
        | (pCirParam->biphaseCoded << 8)
        | (pCirParam->mostSignifSymRecvFirst << 7)
        | (pCirParam->leftAdjustRecvData << 6)
        | (pCirParam->pbRepeat << 5)
        | (pCirParam->measurePreamblePulse << 4)
        | (pCirParam->measureSymPulse << 3)
        | (pCirParam->fixSymPulseLast << 2)
        | (pCirParam->bitsPerSym & 3);
    BKIR_P_WriteCirParam(hChn, 18, ulData);

    BKIR_P_WriteCirParam(hChn, 19, pCirParam->timeoutDivisor);
    BKIR_P_WriteCirParam(hChn, 20, pCirParam->edgeTimeout);
    BKIR_P_WriteCirParam(hChn, 21, pCirParam->faultDeadTime);
    BKIR_P_WriteCirParam(hChn, 22, pCirParam->dataSymTimeout);

    BKIR_P_WriteCirParam(hChn, 23, pCirParam->repeatTimeout);

    ulData = (pCirParam->havePreambleAftStop << 5)
         | (pCirParam->enHavePreambleAftStop << 4)
         | (pCirParam->dataSymTimerExpStatEn << 3)
         | (pCirParam->ignoreDataSymTimerEdge1 << 2)
         | (pCirParam->stopParamUnit << 1)
         | (pCirParam->dataSymClkTickUnit);
    BKIR_P_WriteCirParam(hChn, 25, ulData);

    ulData = (pCirParam->restrictiveDecode << 10)
         | (pCirParam->passModeCustCodePass << 8)
         | (pCirParam->dontValidateMode << 7)
         | (pCirParam->dontValidateTrailer << 6)
         | (pCirParam->dontValidateCustCode << 5)
         | (pCirParam->modeBits << 2)
         | (pCirParam->trailer << 1)
         | (pCirParam->rc6);
    BKIR_P_WriteCirParam(hChn, 26, ulData);

    ulData = (pCirParam->nCustCodeBits << 16)
         | (pCirParam->custCode);
    BKIR_P_WriteCirParam(hChn, 27, ulData);
}
