/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "buhf.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE (uhf);


#define ABSOLUTE_VALUE(x)    ((x) < 0 ? -(x) : (x))


/* If you want to the Frequency adjustment algo to print values,
 * use
 *      #define UHFR_DEBUG_FREQ_ADJUSTMENT printf x
 * else use
 *      #define UHFR_DEBUG_FREQ_ADJUSTMENT  */

#define UHFR_DEBUG_FREQ_ADJUSTMENT(x) BDBG_MSG(x)

/***************************************************************************
Summary:  Maintains status values of different fields required for the Freq
offset adjustment calculcations
****************************************************************************/
typedef struct BUHF_P_MovingAvg {
    unsigned int    logCount;       /* Current no. of entries in the log */
    unsigned char   overflowFlag;   /* Whether log has overflown */
    int             prDcOffset[BUHF_MAX_LOG_SIZE];    /* Preamble DC offset:
                                                         UHFR_COR13.dc_level */
    int             prDcOffsetSum;  /* Sum of DC offset values */
    int             prDcOffsetAvg;  /* Running Average of DC offset values */
} BUHF_P_MovingAvg;


/* Maintains the configuration of the UHFR device */
typedef struct BUHF_P_Device {
    BCHP_Handle         hChip;
    BREG_Handle         hRegister;
    BINT_Handle         hInterrupt;
    BKNI_EventHandle    hDataReadyEvent; /* Event to be set by the UHF module when data is ready */
    BINT_CallbackHandle hCallback1; /* BINT callbacks */
    BINT_CallbackHandle hCallback2;
    unsigned int        uiIndex;    /* UHFR device 0 or 1 */
    uint32_t            ui32Offset; /* Register offset of this UHFR device base
                                       address from UHFR 0 base address */
    BUHF_Settings       settings;   /* Current settings of the UHFR device */

    /* The following are used ONLY IF freqAdjust is TRUE */
    unsigned int        origFcw;        /* Original FCW in first iteration of
                                           Freq Adjust algo */
    unsigned int        adjustedFcw;    /* New FCW in final iteration of Freq
                                           Adjust algo ie after the freq has
                                           converged.*/
    BUHF_P_MovingAvg    *pMovingAvg;    /* Data structure used to calculate
                                           moving average of the DC level */

    /* Used internally */
    volatile bool       intFlag;        /* 1= Data waiting to be processed. */
    BUHF_Data           data;           /* value in the COR12 register and the preamble type */

    /* Temporary: only for diagnostic purposes. Remove later */
    unsigned int        totalPkt;       /* Number of packets entering the system */
    unsigned int        errPkt;         /* Number of incoming packets that have BCH error */
    unsigned int        correctedErrPkt;    /* Number of packets with BCH error
                                               that have been corrected */
    uint32_t            ui32IntStatus;  /* Value of UHFR_INTR2_CPU_STATUS is saved into it */
    bool                bStandbyState;  /* True if in Standby mode, false if in normal mode */
    bool                bFullPowerdown; /* True if the device is completely powered down */

    BUHF_Callback       uhfCb;          /* registered callback function */
    void                *pData;         /* data passed to callback function */
} BUHF_P_Device;



/* The interrupt handler checks which UHFR interrupt has been received and services
 * it accordingly. If an input key has been received, it calls the fsk input
 * handler.
 */
static void BUHF_P_Isr(void *hUhfr, int iParam2);
static void BUHF_P_Cust2Isr1(void *hUhfr, int iParam2);
static void BUHF_P_Cust2Isr2(void *hUhfr, int iParam2);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BUHF_Settings defUhfrSettings =
{
    BUHF_Mode_eAdvanced,     /* mode = Advanced */
    BUHF_ChanNum_eChan1,     /* channel = BUHF_ChanNum_eChan1; */
    BUHF_InjType_eLow,       /* injection = Low side */
    BUHF_eLPF,               /* filter = BUHF_eLPF */
    false,                   /* BCH error correction is disabled */
    false,                   /* Frequency adjustment is disabled */
    false,                   /* Shift IF is disabled */
    0                        /* No shift */
};

static const BUHF_StandbySettings defStandbySettings =
{
    true,       /* bEnableWakeup */
    true,       /* bHwKeyFiltering */
    0x17700BB8  /* ui32KeyPatternValue */
};


/* Low pass filter coeffs for 107 taps for Cust2 ie Channel 9 */
static const unsigned int lowPassFilterCoeffCust2[BUHF_NUM_FILTER_COEFF] = {
           0x00010000,
           0xFFFF0000,
           0x00030000,
           0xFFFB0000,
           0x00080000,
           0xFFF30000,
           0x00130000,
           0xFFE50000,
           0x00250000,
           0xFFCF0000,
           0x00420000,
           0xFFAB0000,
           0x006E0000,
           0xFF740000,
           0x00B00000,
           0xFF240000,
           0x01110000,
           0xFEAF0000,
           0x01A00000,
           0xFDFD0000,
           0x02810000,
           0xFCD80000,
           0x04110000,
           0xFA8B0000,
           0x07E20000,
           0xF2940000,
           0x28B04000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000
};

/* Low pass filter coeffs for 107 taps for Cust1 ie Channel 1-8 */
static const unsigned int lowPassFilterCoeffCust1[BUHF_NUM_FILTER_COEFF] = {
           0xFFFE0004,
           0x00050000,
           0xFFF9FFFA,
           0x0007000E,
           0x0000FFEC,
           0xFFF20010,
           0x001F0000,
           0xFFD7FFE3,
           0x0021003D,
           0x0000FFB2,
           0xFFCA003C,
           0x006D0000,
           0xFF7AFFA4,
           0x006500B6,
           0x0000FF23,
           0xFF6900A5,
           0x01270000,
           0xFE9DFF0E,
           0x010901D9,
           0x0000FDC0,
           0xFE7401B6,
           0x031B0000,
           0xFC0BFD33,
           0x033F0639,
           0x0000F679,
           0xF8170BEB,
           0x26B53334,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000,
           0x00000000
};

/* Tables with values of various registers depending on Frequency
 ANA 1 values for different frequencies = 0x40402204 for all freq*/

   #define BUHF_ANA0_VALUE 0x0000003f
   #define BUHF_ANA1_VALUE 0x00040000
   #define BUHF_ANA3_VALUE 0x01ba0000
   #define BUHF_ANA4_VALUE 0x00800000
   #define BUHF_ANADIR_VALUE 0x00002789

/* ANA 2 values for different frequencies */ /*change for the cust2 pll setting*/
static const unsigned int ana2Table[2][BUHF_NUM_CHANNELS] = {
                             /* Values for low side injection */
                             {0x01a93E94, /* 358.8 Mhz VCO 1*/
                              0x01ab2408 , /* 360.4 Mhz VCO 2*/
                              0x01b01E57 , /* 364.6 Mhz VCO 3*/
                              0x01b203cb , /* 366.2 Mhz VCO 4*/
                              0x01bF86a4 , /* 377.6 Mhz VCO 5*/
                              0x01c3518a , /* 380.8 Mhz VCO 6*/
                              0x01c6a315 , /* 383.6 Mhz VCO 7*/
                              0x01c88889 , /* 385.2 Mhz VCO 8*/
                              0x01f59815   /* 423.22 Mhz : for
Cust2*/
                              },
                             {0x01c29B7F , /* 380.2 Mhz VCO 1*/  /* Values for high side injection */
                              0x01c480F2 , /* 381.8 Mhz VCO 2*/
                              0x01c97B42 , /* 386.0 Mhz VCO 3*/
                              0x01cB60B6 , /* 387.6 Mhz VCO 4*/
                              0x01d8E38E , /* 399.0 Mhz VCO 5*/
                              0x01dCAE75 , /* 402.2 Mhz VCO 6*/
                              0x01e00000 , /* 405.0 Mhz VCO 7*/
                              0x01e1E573 , /* 406.6 Mhz VCO 8*/
                              0x020EF500   /* 444.62 Mhz : For Cust2 */
                             }
                           };

/* BCH Error Correction related macros and arrays */
#define GENERATOR_CODE (0x00000769)
#define CRC_MASK (0x000003ff)
#define CRC_SIZE (10)
#define KEY_DATA_MASK (0x03FFFC00)
#define KEY_DATA_SIZE (16)

static const unsigned long syndrome_table[] = {
0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004,
0x00000005, 0x00000006, 0x00000000, 0x00000008, 0x00000009,
0x0000000a, 0x00000000, 0x0000000c, 0x00000000, 0x00000000,
0x00000000, 0x00000010, 0x00000011, 0x00000012, 0x00480000,
0x00000014, 0x00000000, 0x00000000, 0x00000000, 0x00000018,
0x01000800, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00001400, 0x00000020, 0x00000021, 0x00000022,
0x00000000, 0x00000024, 0x00000000, 0x00900000, 0x01002000,
0x00000028, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x02000400, 0x00000000, 0x00000000, 0x00000030, 0x00000000,
0x02001000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00002800, 0x00000000, 0x00000040,
0x00000041, 0x00000042, 0x00000000, 0x00000044, 0x00000000,
0x00000000, 0x00000000, 0x00000048, 0x00000000, 0x00000000,
0x00000000, 0x01200000, 0x00000000, 0x02004000, 0x00000000,
0x00000050, 0x00800080, 0x00000000, 0x00040200, 0x00000000,
0x00200800, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000060, 0x00000000, 0x00000000, 0x00004400,
0x00000000, 0x00000000, 0x00000000, 0x00030000, 0x00000000,
0x00000000, 0x00000000, 0x00202000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00100080,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00005000,
0x00008100, 0x00000000, 0x00000000, 0x00000080, 0x00000081,
0x00000082, 0x00000000, 0x00000084, 0x00002100, 0x00000000,
0x00000000, 0x00000088, 0x00000000, 0x00000000, 0x02080000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000090,
0x00800040, 0x00000000, 0x00208000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x02400000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00020200, 0x00000000, 0x00000000,
0x000000a0, 0x00000000, 0x01000100, 0x00000000, 0x00000000,
0x00000000, 0x00080400, 0x00000000, 0x00000000, 0x00050000,
0x00401000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00400400, 0x00000000, 0x00100040, 0x00000000,
0x00081000, 0x00000000, 0x00000900, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x000000c0, 0x00800010, 0x00000000,
0x00000000, 0x00000000, 0x00084000, 0x00008800, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00060000, 0x00000000, 0x00800001, 0x00800000,
0x00000000, 0x00800002, 0x00000000, 0x00800004, 0x00404000,
0x00100020, 0x00000000, 0x00800008, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x01008000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00100010, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00200100, 0x00000000,
0x00000000, 0x00800020, 0x00000000, 0x00100004, 0x00000000,
0x00100002, 0x00100001, 0x00100000, 0x0000a000, 0x00000000,
0x00010200, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00100008, 0x00000100, 0x00000101, 0x00000102, 0x00000000,
0x00000104, 0x00002080, 0x00000000, 0x00000000, 0x00000108,
0x00000000, 0x00004200, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000110, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02040000,
0x00000000, 0x00300000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000120, 0x00000000,
0x01000080, 0x00000000, 0x00000000, 0x00041000, 0x00410000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00090000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00040400,
0x00000880, 0x00000000, 0x00008040, 0x00000000, 0x00a00000,
0x00000140, 0x00420000, 0x00000000, 0x00000000, 0x02000200,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00100800, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x000a0000, 0x00000000,
0x00802000, 0x01100000, 0x00000000, 0x00000000, 0x00000000,
0x00044000, 0x00000000, 0x00000000, 0x00000000, 0x00008020,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000600, 0x00800800, 0x00000000, 0x00000000,
0x00008010, 0x00200080, 0x00000000, 0x00000000, 0x00000000,
0x00102000, 0x01800000, 0x00000000, 0x00008008, 0x00001200,
0x00000000, 0x00000000, 0x00008004, 0x00000000, 0x00000000,
0x00008001, 0x00008000, 0x00000000, 0x00008002, 0x00000180,
0x00002004, 0x01000020, 0x00000000, 0x00002001, 0x00002000,
0x00000000, 0x00002002, 0x00000000, 0x00000000, 0x00108000,
0x00000000, 0x00011000, 0x00002008, 0x00000000, 0x00440000,
0x00000000, 0x00000000, 0x00000000, 0x00010400, 0x00000000,
0x00002010, 0x00000000, 0x00024000, 0x00000000, 0x00000000,
0x00000000, 0x00000820, 0x000c0000, 0x00000000, 0x00000000,
0x00000000, 0x01000002, 0x00000000, 0x01000000, 0x01000001,
0x00000000, 0x00002020, 0x01000004, 0x00000000, 0x00000000,
0x00000000, 0x01000008, 0x00000810, 0x00808000, 0x00000000,
0x00200040, 0x00000000, 0x00000000, 0x00000000, 0x01000010,
0x00000808, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000802, 0x00000801, 0x00000800, 0x00000000,
0x00000000, 0x02010000, 0x00000804, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00002040, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00200020, 0x00080200, 0x00000000,
0x00800100, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x02020000, 0x00000000,
0x00000000, 0x00400200, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x01000040, 0x00000000, 0x00000000,
0x00000000, 0x00200008, 0x00000000, 0x00000000, 0x00000000,
0x00200004, 0x00021000, 0x00200002, 0x00000000, 0x00200000,
0x00200001, 0x00014000, 0x00000000, 0x00000000, 0x00000000,
0x00020400, 0x00000000, 0x00000000, 0x00100100, 0x00000000,
0x00000000, 0x00000000, 0x00000840, 0x00000000, 0x00008080,
0x00200010, 0x00000000, 0x00000200, 0x00000201, 0x00000202,
0x00000000, 0x00000204, 0x00000000, 0x00000000, 0x00000000,
0x00000208, 0x00000000, 0x00004100, 0x00009000, 0x00000000,
0x00110000, 0x00000000, 0x00000000, 0x00000210, 0x00000000,
0x00000000, 0x00040040, 0x00008400, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00020080, 0x00000000, 0x00000000, 0x00000220,
0x00280000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00810000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00600000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02008000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000240, 0x00000000, 0x00000000, 0x00040010,
0x02000100, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00082000, 0x00000000, 0x00820000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00040002, 0x00040001,
0x00040000, 0x00000000, 0x00000000, 0x00000000, 0x00040004,
0x00000000, 0x00402000, 0x00000000, 0x00040008, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00400800, 0x00000000, 0x00000500, 0x00120000, 0x00000000,
0x00000000, 0x01080000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00040020, 0x00080800, 0x00000000,
0x00001100, 0x0000c000, 0x00000000, 0x00000000, 0x00010080,
0x00000000, 0x00000000, 0x00000000, 0x01400000, 0x00000000,
0x00000280, 0x00000000, 0x00840000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00200400, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00020010, 0x00000000,
0x00006000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00020008, 0x00000000, 0x00000000, 0x00201000,
0x00020004, 0x00000000, 0x00000000, 0x00020001, 0x00020000,
0x00000000, 0x00020002, 0x00000000, 0x00408000, 0x00000000,
0x00000000, 0x00140000, 0x00000000, 0x00000000, 0x00000000,
0x01004000, 0x00000000, 0x02200000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004800,
0x00088000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00010040, 0x00000000,
0x00000000, 0x00020020, 0x00000000, 0x00000000, 0x00000000,
0x02002000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x01000400, 0x00000000, 0x00001800, 0x00000000, 0x00080100,
0x00000000, 0x00800200, 0x00000c00, 0x00040080, 0x01001000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00010020, 0x00000000, 0x00400100, 0x00020040, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00204000, 0x00000000, 0x03000000, 0x00000000, 0x00000000,
0x00000000, 0x00010010, 0x00000000, 0x00002400, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010008,
0x00003000, 0x00000000, 0x00000000, 0x00000000, 0x00100200,
0x00010002, 0x00000000, 0x00010000, 0x00010001, 0x00000000,
0x00000000, 0x00010004, 0x02000800, 0x00000300, 0x00000000,
0x00004008, 0x00000000, 0x02000040, 0x00000000, 0x00000000,
0x00000000, 0x00004002, 0x00000000, 0x00004000, 0x00004001,
0x00000000, 0x00c00000, 0x00004004, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00210000, 0x00000000,
0x00000000, 0x00000000, 0x00022000, 0x00000000, 0x00004010,
0x00000000, 0x00000000, 0x00000000, 0x00880000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00020800, 0x00000000, 0x00000000, 0x00000440,
0x00004020, 0x00500000, 0x00000000, 0x00000000, 0x00048000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00001040, 0x00000000, 0x00180000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x01020000, 0x02000004, 0x00010800, 0x00000000,
0x00000000, 0x02000000, 0x02000001, 0x02000002, 0x00000000,
0x00000000, 0x00000420, 0x00004040, 0x00000000, 0x02000008,
0x00000000, 0x00000000, 0x00080080, 0x00000000, 0x00000000,
0x00000000, 0x00040100, 0x02000010, 0x00000000, 0x00001020,
0x00000000, 0x01010000, 0x00000000, 0x00000000, 0x00000000,
0x00400080, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000408, 0x00000000, 0x00000000, 0x02000020, 0x00000000,
0x00001010, 0x00000000, 0x00000401, 0x00000400, 0x00000000,
0x00000402, 0x00000000, 0x00000404, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00001004, 0x00220000, 0x00001002,
0x00000000, 0x00001000, 0x00001001, 0x00000000, 0x00000410,
0x00000000, 0x00000000, 0x00000000, 0x00008200, 0x00001008,
0x00012000, 0x00000000, 0x00101000, 0x00000000, 0x00000000,
0x00000000, 0x00002200, 0x00000000, 0x00018000, 0x00000000,
0x00000000, 0x00004080, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00080040, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x02800000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00400040,
0x00020100, 0x00100400, 0x00000000, 0x00000000, 0x00000000,
0x01000200, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00801000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x02100000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00800400, 0x00000000, 0x00000000,
0x00000a00, 0x00000000, 0x00240000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x02000080,
0x00000000, 0x00000000, 0x00080008, 0x00000000, 0x00000000,
0x00000000, 0x00080004, 0x00400010, 0x00080002, 0x00080001,
0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00400008, 0x00000000, 0x00042000, 0x00000000, 0x00400004,
0x00000000, 0x00000000, 0x00804000, 0x00400000, 0x00400001,
0x00400002, 0x00080010, 0x00028000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00040800, 0x00000480, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00200200, 0x00080020, 0x00000000, 0x01040000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00001080,
0x00000000, 0x00000000, 0x00000000, 0x00010100, 0x00000000,
0x00400020, 0x00104000};

#define SYNDROME_TABLE_SIZE (sizeof(syndrome_table) / 4)

static BERR_Code BUHF_P_BchErrorCorrection (BUHF_Handle hUhfr, unsigned int * packet);

/* BCH Error Correction related additions - end */

/* Static functions ie local functions not visible to user */
static BERR_Code BUHF_P_InitReg (BUHF_Handle hUhfr);
static BERR_Code BUHF_P_SetFilterCoeff (BUHF_Handle hUhfr, BUHF_Filter filter);

/******************************************************************************
 * Summary:
 *      Sets the filter coefficients.
 *
 * Input Parmameters:
 *      BUHF_Handle hUhfr : Pointer to UHFR device
 *      BUHF_Filter filter   : Which filter to select.
 *                             Values = BUHF_eLPF or BUHF_eBPF
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error value on failure.
 *
 * Implementation:
 *      Filter coefficients for Low Pass and High Pass filter
 *      are taken from the array filterCoeff[][].
 *
 *****************************************************************************/
static BERR_Code BUHF_P_SetFilterCoeff
(
    BUHF_Handle hUhfr,
    BUHF_Filter filter
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t val;
    unsigned int i;

    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr);

    hUhfr->settings.filter = filter;

    /* Enable auto increment */
    val = (BCHP_UHFR_LPCOEFADDR_LPF_COEF_ADDR_MEM_ACC_SEL_MASK | BCHP_UHFR_LPCOEFADDR_LPF_COEF_ADDR_INC_EN_MASK);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_LPCOEFADDR + hUhfr->ui32Offset), val);

    if (filter == BUHF_eLPF)
    {
        if (hUhfr->settings.channel == BUHF_ChanNum_eChan9)
        { /* Load LPF coeffs for Cust 2 */
            for (i=0; i<BUHF_NUM_FILTER_COEFF; i++)
            {
                val = lowPassFilterCoeffCust2[i];
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COEF + hUhfr->ui32Offset), val);
            }
        }
        else
        {   /* Load LPF coeffs for Channels 1-8 ie Cust 1 */
            for (i=0; i<BUHF_NUM_FILTER_COEFF; i++)
            {
                val = lowPassFilterCoeffCust1[i];
                BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COEF + hUhfr->ui32Offset), val);
            }
        }
    }
    else if (filter == BUHF_eBPF)
    { /* For Band Pass Filter */
        BDBG_ERR(("BandPass Filter is not supported"));
    }

    val = BREG_Read32(hUhfr->hRegister, (BCHP_UHFR_LPCOEFADDR + hUhfr->ui32Offset));
    val &= ~(BCHP_UHFR_LPCOEFADDR_LPF_COEF_ADDR_MEM_ACC_SEL_MASK | BCHP_UHFR_LPCOEFADDR_LPF_COEF_ADDR_INC_EN_MASK);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_LPCOEFADDR + hUhfr->ui32Offset), val);
    return ret;
}

/******************************************************************************
 * static BERR_Code BUHF_P_InitReg (BUHF_Handle hUhfr, BUHF_Mode mode)
 *
 * Function:
 *      Initializes all UHFR registers according to mode of
 *      operation (Legacy or Advanced)
 *
 * Input Parmameters:
 *      SBcmDevice *pDevice: Pointer to UHFR device
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error on failure
 *
 * Implementation:
 *      ANA3 is not set here. To set it, BUHF_SelectRfChan() has to be called
 *      explictly by the user.
 *      In Legacy mode, disable all UHFR intrpts.
 *      In Advanced mode, enable only BCHP_UHFR_INTR2_CPU_STATUS_CORR_BCH_ERR_MASK,
 *      CORR_DECODE_PR1_END_MASK and CORR_DECODE_PR2_END_MASK
 *
 *****************************************************************************/
static BERR_Code BUHF_P_InitReg (
        BUHF_Handle hUhfr
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int val;
    unsigned int offset = 0,uiRegval=0;
    offset = hUhfr->ui32Offset;

    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr);

    BDBG_MSG(("BUHF_P_InitReg"));

    /* always program BCHP_UHFR_AUTOPD1 for UHF1 or UHF2 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD1), 0x00000000);

    /* reset all blocks */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_RST + offset), 0xffffffff);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_RST + offset), 0);

    /* 7420 are decimated by 108 */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FILT + offset), 0x006a0104);
    val = (hUhfr->settings.channel == BUHF_ChanNum_eChan9)? 0x00cd462a: 0x0032b9d6; /* ~(0x0032b9d6)+1 = FFCD462A, but programming 0x00CD462A*/

    /* If SHIFT IF is selected, use different FCW values for UHF1 and UHF 2 */
    if ((hUhfr->settings.bShiftIf==true) && (hUhfr->settings.channel != BUHF_ChanNum_eChan9))
    {
        if (hUhfr->uiIndex == 0)
        {
            val = 0x00322833; /*in 81 MHZ domain its value is 0x00217022 And 81*(0x00217022/2^24)=10.58MHz .
                                        So in 54Mhz domain (10.58/54)*2^24=0x00322833*/
        }
        else if (hUhfr->uiIndex == 1)
        {
            val = 0x00334B78; /*in 81 MHZ domain its value is 0x00223251 And 81*(0x00223251/2^24)=10.58MHz .
                                        So in 54Mhz domain (10.58/54)*2^24=0x00334B78*/
        }
    }
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FCW + offset), val);

    BUHF_P_SetFilterCoeff(hUhfr, hUhfr->settings.filter);

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00010a04: 0x00060a05;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_IRPH + offset), val);

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_IRDC + offset), 0x260b0f0a);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FRZMM + offset), 0x02020202);

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00100100: 0x00000100;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FRZDC + offset), val);

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_DCINT+ offset), 0x00000000);

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0xf2010b0a: 0xf2010c0a;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_PDAS + offset), val);

    val = (hUhfr->settings.mode == BUHF_Mode_eLegacy)? 0x00048000: 0x00010000;
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_THRAS + offset), val);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_THHYS + offset), 0x00000020);

    if ((hUhfr->settings.channel == BUHF_ChanNum_eChan9))
    {
        /* For Cust2 in Legacy/Advanced Mode*/
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ALTCFG + offset), 0x20f40160);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ALTCFG2 + offset), 0xff10785f);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR1 + offset), 0x005b0c8e);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR4 + offset), 0x00000d6a);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR5 + offset), 0x0001ff6b);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR6 + offset), 0x0001ff6b);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR7 + offset), 0x003FD879);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR8 + offset), 0);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR9 + offset), 0);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR10 + offset), 0x00001919);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR16 + offset), 0x000FFFFF);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR11 + offset), 0x0708010F);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR3 + offset), 0x0c001f00);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR2 + offset), 0x00140801);

        uiRegval = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS + offset));
        BDBG_MSG(("BCHP_UHFR_INTR2_CPU_MASK_STATUS=%d",uiRegval));
    }

    else if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)
    {
        /* For Cust1 in Advanced mode */
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR1 + offset), 0x00000D8e);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR4 + offset), 0x003FD861);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR5 + offset), 0x0000FF7B);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR6 + offset), 0x0000FFE7);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR7 + offset), 0x003FD879);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR8 + offset), 0x00007fdf);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR9 + offset), 0x00003FFF);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR10 + offset), 0x19191919);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR16 + offset), 0x000FFFFF);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR11 + offset), 0x0708010F);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR3 + offset), 0x09401f00);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR2 + offset), 0x00151400);

        uiRegval = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS + offset));
        BDBG_MSG(("BCHP_UHFR_INTR2_CPU_MASK_STATUS=%d",uiRegval));
    }

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_CTL1 + offset), 0x00000401);

    /* Issue Soft Reset */
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_RST + offset), 0x00010000);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_RST + offset), 0);

    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        /* For Cust1/Cust2 in legacy mode */
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR5 + offset), 0x003FFF7B);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR8 + offset), 0x000FFFFF);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR3 + offset), 0x26000E07);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_SET + offset), 0xFFFFFFFF);
        uiRegval = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS + offset));
        BDBG_MSG(("BCHP_UHFR_INTR2_CPU_MASK_STATUS=%d",uiRegval));
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_CTL1 + offset), 0x40000401);
    }

    if(hUhfr->settings.channel == BUHF_ChanNum_eChan9)
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD4),0x1E240107);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD3),0x20D01068);
    }
    else
    {
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD4),0x0880DCC0);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD3),0x17700BB8);
    }
    val = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5));
    val &= ~((BCHP_MASK (UHFR_AUTOPD5, REPEAT_PKT_TIMEOUT)));
    val |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, REPEAT_PKT_TIMEOUT,0xFA0));
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5), val);

    return ret;
}

/* For BCH error Correction.  */
/*
 *  This function strips the transmitted 10 bit
 *  crc off the 26 bit packet and replaces the crc with zeros.
 *  The new crc is calculated and compared with the transmitted crc.
 *  If the crc's don't match, bit errors have occured.  Using the
 *  difference between the crc's and the syndrome look up table, up
 *  to two bit errors can be corrected.  After an attempt to correct
 *  errors has been made, the crc will be calculated again to verify
 *  the packet has been repaired properly.
 *
 *
 *  \param input - 26 bit fsk packet
 *
 *  \return  eDeviceExtError for error, BERR_SUCCESS for OK
 */
static BERR_Code BUHF_P_BchErrorCorrection
(
    BUHF_Handle hUhfr,
    unsigned int *packet
)
{
    unsigned long calc_crc;
    unsigned long syndrome;
    unsigned long generator = GENERATOR_CODE << KEY_DATA_SIZE;
    int the_index = KEY_DATA_SIZE + CRC_SIZE;
    unsigned int temp_pkt;
    unsigned int offset=0, mask=0, regVal=0, shift=0;

    BDBG_ASSERT(hUhfr);
    offset = hUhfr->ui32Offset;

    temp_pkt = *packet;

    /* Get syndrome from COR14 */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR14+offset));
    mask = BCHP_UHFR_COR14_bch_synd_MASK;
    shift = BCHP_UHFR_COR14_bch_synd_SHIFT;
    syndrome = (regVal & mask) >> shift;

    if (syndrome) /* error detected */
    {

        hUhfr->errPkt++;

        if (syndrome < SYNDROME_TABLE_SIZE)
        {
            /* Right shift packet by 6 */
            temp_pkt = ((temp_pkt) >> 6);

            temp_pkt ^= syndrome_table[syndrome]; /* attempt repair */

            the_index = KEY_DATA_SIZE + CRC_SIZE;
            generator = GENERATOR_CODE << KEY_DATA_SIZE;
            calc_crc = temp_pkt;    /* Don't strip off transmitted crc */

            /* re - calculate crc - result should be zero if packet is fixed */
            while (the_index >= CRC_SIZE)
            {
                if (calc_crc & (1 << the_index))
                {
                    calc_crc ^= generator;
                }
                generator >>= 1;
                the_index--;
            }

            if (calc_crc &= CRC_MASK)
            {
                BDBG_ERR(("BUHF_P_BchErrorCorrection: BCH error corrections failed."));
                return BERR_TRACE(BERR_UNKNOWN);
            }
            else
            {
                /* Left shift packet by 6 */
                temp_pkt = ((temp_pkt) << 6);

                /* Copy back to orignal packet memory */
                *packet = temp_pkt;

                hUhfr->correctedErrPkt++;

                BDBG_MSG(("BUHF_P_BchErrorCorrection: BCH error corrected."));
                return BERR_SUCCESS;
            }

        }
        BDBG_ERR(("BUHF_P_BchErrorCorrection: BCH error corrections failed. syndrome > SYNDROME_TABLE_SIZE"));
        return BERR_TRACE(BERR_UNKNOWN);
   }
   return BERR_SUCCESS;
}

BERR_Code BUHF_Open(
    BUHF_Handle *phUhfr,                /* [out] Returns Device handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle */
    unsigned int devNum,                /* [in] UHFR no. 0 or 1 */
    const BUHF_Settings *pSettings      /* [in] Device settings */
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i;
    BUHF_Handle hUhfr;

    /* Check parameters passed to this function */
    BDBG_ASSERT(phUhfr);
    BDBG_ASSERT(hChip);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hInterrupt);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(("devNum=%d", devNum));
    BDBG_MSG(("mode=%d", pSettings->mode));
    BDBG_MSG(("channel=%d", pSettings->channel));
    BDBG_MSG(("injection=%d", pSettings->injection));
    BDBG_MSG(("filter=%d", pSettings->filter));
    BDBG_MSG(("bchErrorCorrection=%d", pSettings->bchErrorCorrection));
    BDBG_MSG(("bFreqAdjust=%d", pSettings->bFreqAdjust));
    BDBG_MSG(("bShiftIf=%d", pSettings->bShiftIf));
    BDBG_MSG(("uiFreqShift=%d", pSettings->uiFreqShift));

    /* Make sure device number is valid, and get offset */
    if (devNum >= BUHF_MAX_DEVICES)
    {
        BDBG_ERR(("BUHF_Open: Invalid Device number."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Shift IF can be enabled only for 65nm, Dual UHFR chips. */
#if !((BUHF_MAX_DEVICES==2))
    if (pSettings->bShiftIf == true)
    {
        BDBG_ERR(("%d is not a 65nm chip with 2 UHF devices. Please disable shift IF. ", BCHP_CHIP));
        return BERR_INVALID_PARAMETER;
    }
#endif
    if ((pSettings->bShiftIf == true) && (pSettings->channel==BUHF_ChanNum_eChan9))
    {
        BDBG_ERR(("Shift IF is not valid for Channel9. Please disable shift IF. "));
        return BERR_INVALID_PARAMETER;
    }

    /* Allocate the memory for the Device handle */
    hUhfr = (BUHF_Handle) BKNI_Malloc( sizeof(BUHF_P_Device));
    if(hUhfr == NULL)
    {
        ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return ret;
    }
    /* Reset the entire structure */
    BKNI_Memset (hUhfr, 0, sizeof(BUHF_P_Device));
    *phUhfr = hUhfr;

    /* Initialise UHFR Handle */
    hUhfr->hChip = hChip;
    hUhfr->hRegister = hRegister;
    hUhfr->hInterrupt = hInterrupt;
    hUhfr->settings = *pSettings;
    hUhfr->uiIndex = devNum;
    hUhfr->bStandbyState = false;
    hUhfr->bFullPowerdown = false;

#if (BUHF_MAX_DEVICES > 1)
    /* If there are more than 1 UHF devices, get correct offset for all */
    hUhfr->ui32Offset = devNum *(BCHP_UHFR_2_RST - BCHP_UHFR_RST);
#else
    hUhfr->ui32Offset = 0; /* Since only 1 UHFR device*/
#endif

    hUhfr->origFcw = 0;
    hUhfr->adjustedFcw = 0;
    hUhfr->intFlag = 0;
    hUhfr->data.value = 0;
    hUhfr->data.prType = BUHF_PrType_eNone;
    hUhfr->totalPkt = 0;
    hUhfr->errPkt = 0;
    hUhfr->correctedErrPkt = 0;

    /* Create event handler */
    ret = BKNI_CreateEvent( &(hUhfr->hDataReadyEvent));
    if (ret != BERR_SUCCESS)
    {
         BDBG_ERR(("Could not install event handler"));
         BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here */
         return BERR_TRACE(ret);
    }

#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
    BCHP_PWR_AcquireResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif

    /* Install ISR */

    if (hUhfr->settings.mode == BUHF_Mode_eAdvanced)
    {
        if (hUhfr->settings.channel == BUHF_ChanNum_eChan9)
        {
            ret = BINT_CreateCallback (
                &hUhfr->hCallback1,
                hUhfr->hInterrupt,
                BCHP_INT_ID_ALT_PACKET_END,
                BUHF_P_Cust2Isr1,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback1);

            ret = BINT_CreateCallback (
                &hUhfr->hCallback2,
                hUhfr->hInterrupt,
                BCHP_INT_ID_ALT_SFR,
                BUHF_P_Cust2Isr2,
                (void*)hUhfr,
                0 /* Not used */);
            ret = BINT_EnableCallback (hUhfr->hCallback2);
        }
        else
        {
            ret = BINT_CreateCallback (
                &hUhfr->hCallback1,
                hUhfr->hInterrupt,
                BCHP_INT_ID_CORR_DECODE_PR1_END,
                BUHF_P_Isr,
                (void*)hUhfr,
                BCHP_INT_ID_CORR_DECODE_PR1_END);
            ret = BINT_EnableCallback (hUhfr->hCallback1);

            ret = BINT_CreateCallback (
                &hUhfr->hCallback2,
                hUhfr->hInterrupt,
                BCHP_INT_ID_CORR_DECODE_PR2_END,
                BUHF_P_Isr,
                (void*)hUhfr,
                BCHP_INT_ID_CORR_DECODE_PR2_END);
            ret = BINT_EnableCallback (hUhfr->hCallback2);
        }
    }

    BDBG_MSG(("UHF interrupt callbacks installed for UHF device %d", hUhfr->uiIndex));

    hUhfr->pMovingAvg =  (BUHF_P_MovingAvg *)BKNI_Malloc (sizeof(BUHF_P_MovingAvg));
    if (hUhfr->pMovingAvg == NULL)
    {
        BDBG_ERR(("BUHF_Open: Memory allocation for UHFR Moving Avg data structure failed."));
        ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here*/
        return ret;
    }

    hUhfr->pMovingAvg->logCount = 0;
    hUhfr->pMovingAvg->overflowFlag = 0;
    hUhfr->pMovingAvg->prDcOffsetAvg = 0;
    hUhfr->pMovingAvg->prDcOffsetSum = 0;
    for (i=0; i < BUHF_MAX_LOG_SIZE; i++)
    {
        hUhfr->pMovingAvg->prDcOffset[i] = 0;
    }

    /* Initialise UHFR Registers */
    ret = BUHF_P_InitReg (hUhfr);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BUHF_Open: Failed to initialise UHFR registers correctly."));
        BKNI_Free(hUhfr->pMovingAvg);
        BKNI_Free(hUhfr); /* free the memory allocated for handle before returning here*/
        return BERR_TRACE(ret);
    }

    hUhfr->uhfCb = NULL; /* no callback until they register one */
    hUhfr->pData = NULL;

    BUHF_SelectRfChan (hUhfr);

    return ret;
}

BERR_Code BUHF_Close
(
    BUHF_Handle hUhfr
)
{
    /* Check parameters passed to this function */
    BDBG_ASSERT(hUhfr);

    if (hUhfr->uhfCb)
        BUHF_UnregisterCallback(hUhfr);

    if (hUhfr->hCallback1 != 0)
    {
        BINT_DisableCallback (hUhfr->hCallback1);
        BINT_DestroyCallback (hUhfr->hCallback1);
    }
    if (hUhfr->hCallback2 != 0)
    {
        BINT_DisableCallback (hUhfr->hCallback2);
        BINT_DestroyCallback (hUhfr->hCallback2);
    }

#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
        BCHP_PWR_ReleaseResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif

    BKNI_DestroyEvent(hUhfr->hDataReadyEvent);

    BKNI_Free(hUhfr->pMovingAvg);
    BKNI_Free(hUhfr);

    return BERR_SUCCESS;
}

BERR_Code BUHF_GetEventHandle(
    BUHF_Handle hUhfr,          /* [in] Device  handle */
    BKNI_EventHandle *phEvent   /* [out] Returns event handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(phEvent);

    *phEvent = hUhfr->hDataReadyEvent;

    return( retCode );
}

void BUHF_RegisterCallback (
    BUHF_Handle hUhfr,      /* [in] UHFR Device handle */
    BUHF_Callback callback, /* [in] Callback function to register */
    void *pData             /* [in] Data that will be passed to callback function */
)
{
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(callback);

    hUhfr->uhfCb = callback;
    hUhfr->pData = pData;

    return;
}

void BUHF_UnregisterCallback (
    BUHF_Handle hUhfr       /* [in] UHFR Device handle */
)
{
    BDBG_ASSERT(hUhfr);

    hUhfr->uhfCb = NULL;
    hUhfr->pData = 0;

    return;
}


/******************************************************************************
 *
 * Function:
 *      The interrupt handler checks which UHFR interrupt has been received
 *      and services it accordingly. The interrupt bit is cleared by writing
 *      a 1 to it.
 *
 * Returns:
 *      BERR_SUCCESS on Success
 *      Error value on other errors.
 *
 * Implementation:
 *      When a eUhfrIntCorrDecodePr1End or eUhfrIntCorrDecodePr2End
 *      UHFR interrupt has occurred => a packet has been recieved.
 *      On recieving a packet, the interrupt flag in UHFR configuration
 *      data structure is set to indicate that there is data pending to be read;
 *      this data is saved in pUhfrConfig->data to prevent loss on reset (soft
 *      resets are done by the frequency offset adjustment algorithm), and the
 *      frequency offset adjustment algorithm is invoked.
 *
 *****************************************************************************/
/* TODO: need to clean up this ISR */
static void BUHF_P_Isr
(
    void *pUhfr,     /* [in] Handle to UHFR device */
    int iParam2      /* [in] the interrupt we're waiting for */
)
{
    unsigned int irq_status, mask;
    BUHF_Handle hUhfr;
    unsigned int offset;
    unsigned int bchError=0;

    BDBG_ASSERT(pUhfr);

    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;

    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_STATUS+offset));
    hUhfr->ui32IntStatus = irq_status;
    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS+offset));
    BDBG_MSG(("UHFR_INTR2_CPU_STATUS=0x%08x, BCHP_UHFR_INTR2_CPU_MASK_STATUS=0x%08x", irq_status, mask));

    /* If the interrupt is enabled and is set in the interrupt status register then set the flag */

    if (iParam2 == BCHP_INT_ID_CORR_DECODE_PR1_END)
    {
        BDBG_MSG(("bcmUhfrIsr: CORR_DECODE_PR1_END interrupt."));
        hUhfr->intFlag = 1;
        hUhfr->data.prType = BUHF_PrType_e1;
    }

    if (iParam2 == BCHP_INT_ID_CORR_DECODE_PR2_END)
    {
        BDBG_MSG(("bcmUhfrIsr: CORR_DECODE_PR2_END interrupt."));
        hUhfr->intFlag = 1;
        hUhfr->data.prType = BUHF_PrType_e2;
    }

    if (hUhfr->intFlag == 1)
    {
        /* The PLL workaround and the freq offset adjustment algo etc require a soft
         * reset. Soft reset also clears out COR12 => data is lost on a soft reset.
         * Therefore, in the ISR, we save the data into hUhfr->data and read
         * it from there always. */

        /* Save the packet */
        hUhfr->data.value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR12+offset));
        BDBG_MSG(("data.value = %x",hUhfr->data.value));

        /* Increment pkt count */
        hUhfr->totalPkt++;

        bchError = (irq_status & BCHP_UHFR_INTR2_CPU_STATUS_CORR_BCH_ERR_MASK) >> BCHP_UHFR_INTR2_CPU_STATUS_CORR_BCH_ERR_SHIFT;

        if (bchError == 1)
        {
            if (hUhfr->settings.bchErrorCorrection == 1)
            {
               /* If enabled do BCH error correction on the recieved data*/
               BUHF_P_BchErrorCorrection(hUhfr, &(hUhfr->data.value));
            }
            else
            {
                if (hUhfr->data.prType == BUHF_PrType_e1) {
                    BDBG_MSG(("BCH error detected, but error correction has not been enabled"));
                    hUhfr->data.value = 0xFFFFFFFF; /* if we have bad data and no correction give them noticable junk */
                }
            }
        }

        /* Call back to the registered callback function */
        if (hUhfr->uhfCb)
            hUhfr->uhfCb(hUhfr, hUhfr->pData);

        BKNI_SetEvent (hUhfr->hDataReadyEvent);
    }

    return;
}

static void BUHF_P_Cust2Isr1
(
    void *pUhfr,     /* [in] Handle to UHFR device */
    int iParam2      /* Not used */
)
{
    unsigned int irq_status, mask;
    BUHF_Handle hUhfr;
    unsigned int offset;
    bool bError=false;

    BDBG_ASSERT(pUhfr);
    BSTD_UNUSED(iParam2);

    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;

    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_STATUS+offset));
    hUhfr->ui32IntStatus = irq_status | BCHP_UHFR_INTR2_CPU_STATUS_ALT_PACKET_END_MASK;
    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS+offset));
    BDBG_MSG(("UHFR_INTR2_CPU_STATUS=0x%08x, BCHP_UHFR_INTR2_CPU_MASK_STATUS=0x%08x", irq_status, mask));

    /* If the interrupt is enabled and is set in the interrupt status register then set the flag */

    if (irq_status & BCHP_UHFR_INTR2_CPU_STATUS_CORR_ALT_CHSM_ERR_MASK)
    {
        bError = true;
        BDBG_MSG(("BUHF_P_Cust2Isr1: Checksum error interrupt occured."));
    }

    if(bError == false)
    {
        hUhfr->intFlag = 1;
        hUhfr->data.value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_ALTDTM+offset));
        hUhfr->data.prType = BUHF_PrType_eNone;
        BDBG_MSG(("hUhfr->data.value = %x",hUhfr->data.value));

        /* Call back to the registered callback function */
        if (hUhfr->uhfCb)
            hUhfr->uhfCb(hUhfr, hUhfr->pData);

        BKNI_SetEvent (hUhfr->hDataReadyEvent);
	}
}

static void BUHF_P_Cust2Isr2
(
    void *pUhfr,     /* [in] Handle to UHFR device */
    int iParam2      /* Not used */
)
{
    unsigned int irq_status;
    BUHF_Handle hUhfr;
    unsigned int offset;

    BDBG_ASSERT(pUhfr);
    BSTD_UNUSED(iParam2);

    hUhfr = (BUHF_Handle) pUhfr;
    offset = hUhfr->ui32Offset;

    irq_status = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_STATUS+offset));
    hUhfr->ui32IntStatus = irq_status | BCHP_UHFR_INTR2_CPU_STATUS_ALT_SFR_MASK;
    BDBG_MSG(("BUHF_P_Cust2Isr2: ALT_SFR interrupt occured. Decode aborted early. Data not valid."));
}

BERR_Code BUHF_IsDataReady
(
    BUHF_Handle hUhfr,               /* [in] UHFR Device handle */
    unsigned char *pFlag             /* [out] flag indicating data is available */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    unsigned int offset = 0;

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pFlag);

    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BUHF_IsDataReady: Checking if there's data to be read ..."));

    offset = hUhfr->ui32Offset;
    *pFlag = 0; /* Assume no data is ready */

    if (hUhfr->intFlag == 1)
    {
        *pFlag = 1;
    }
    return ret;
}


BERR_Code BUHF_Read_isrsafe
(
    BUHF_Handle hUhfr,              /* [in] UHFR Device handle */
    BUHF_Data   *pData              /* [out] ptr to where data is to be saved */
)
{
    BERR_Code   ret = BERR_SUCCESS;

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pData);

    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("bcmUhfrRead: Reading Data..."));

    /* The PLL workaround and the freq offset adjustment algo etc require a soft
     * reset. Soft reset also clears out COR12 => data is lost on a soft reset.
     * Therefore, in the ISR, we save the data into pUhfrConfig->data and read
     * it from there always. */
    *pData = hUhfr->data;

    /* Clear the intrpt flag and data */
    hUhfr->intFlag = 0;
    hUhfr->data.value = 0;
    hUhfr->data.prType = BUHF_PrType_eNone;

    return ret;
}

BERR_Code BUHF_GetDefaultSettings (
    BUHF_Settings *pDefSettings     /* [out] Returns default settings */
)
{
    *pDefSettings = defUhfrSettings;
    return BERR_SUCCESS;
}

BERR_Code BUHF_GetCurrentSettings (
    BUHF_Handle     hUhfr,          /* [in] UHFR Device handle */
    BUHF_Settings   *pSettings      /* [out] Returns current settings */
)
{
    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pSettings);

    *pSettings = hUhfr->settings;
    return BERR_SUCCESS;
}


BERR_Code BUHF_GetStatus
(
    BUHF_Handle   hUhfr,    /* [in] UHFR Device handle */
    BUHF_Status   *pStatus  /* [out] ptr to where Status is to be saved */
)
{
    BERR_Code   ret = BERR_SUCCESS;
    unsigned int offset=0, mask=0, regVal=0, shift=0;

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pStatus);

    offset = hUhfr->ui32Offset;

    /* This function is available only in advanced mode */
    if (hUhfr->settings.mode == BUHF_Mode_eLegacy)
    {
        BDBG_ERR(("BUHF_IsDataReady: not Available in UHFR legacy mode."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BUHF_GetStatus: Reading Status..."));

    /* Get BCH error status bit */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_STATUS+offset));
    mask = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_INTR2_CPU_MASK_STATUS+offset));

    regVal = regVal & ~mask; /* Read the bit only if this interrupt has been unmasked */
    mask = BCHP_UHFR_INTR2_CPU_STATUS_CORR_BCH_ERR_MASK;
    shift = BCHP_UHFR_INTR2_CPU_STATUS_CORR_BCH_ERR_SHIFT;
    pStatus->bchError = (regVal & mask) >> shift;

    /* Read COR 13 */
    regVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR13+offset));

    /* Get Preamble Peak */
    mask = BCHP_UHFR_COR13_pr_corr_peak_MASK;
    shift = BCHP_UHFR_COR13_pr_corr_peak_SHIFT;
    pStatus->prCorrPeak = (regVal & mask) >> shift;

    /* Get Preamble DC offset */
    mask = BCHP_UHFR_COR13_dc_level_MASK;
    shift = BCHP_UHFR_COR13_dc_level_SHIFT;
    pStatus->dcLevel = (int) ((short int) (regVal & mask) >> shift); /* Sign extend */

    BDBG_MSG(("BCH Error=%d, Preamble Peak=0x%x, DC Level=%d, Slow RSSI out=%d", pStatus->bchError, pStatus->prCorrPeak, pStatus->dcLevel, pStatus->slowRssiOut));

    pStatus->totalPkt = hUhfr->totalPkt;
    pStatus->errPkt = hUhfr->errPkt;
    pStatus->correctedErrPkt = hUhfr->correctedErrPkt;
    pStatus->ui32IntStatus = hUhfr->ui32IntStatus;

    return ret;
}

/******************************************************************************
Summary:
      Configures to one of the 9 RF channels defined .

 Description:
      Depending on the channel selected, we set the values for UHFR Analog
      Control Register 3 ie fRF, fLO, fVCO, fPI, vco_div, div2, rot_dir,
      divN[7:0] and fcw[13:0].

 Returns:
      BERR_SUCCESS on Success
      eDeviceBadDevice if invalid device number is given
      eDeviceBadParam if invalid channel number or injection value is given
      Error value on other errors.

Returns:
    BERR_SUCCESS - if open is successful.
    Error value  - if not successful. *
*****************************************************************************/


BERR_Code BUHF_SelectRfChan
(
    BUHF_Handle     hUhfr           /* [in] UHFR Device handle */
)
{
    unsigned int offset=0;
    uint32_t value, fcw, ana2_val;

    BDBG_ASSERT(hUhfr);
    offset = hUhfr->ui32Offset;

    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANA0 + offset), BUHF_ANA0_VALUE);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANA1 + offset), BUHF_ANA1_VALUE);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANA3 + offset), BUHF_ANA3_VALUE);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANA4 + offset), BUHF_ANA4_VALUE);
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANADIR + offset), BUHF_ANADIR_VALUE);
    ana2_val = ana2Table[hUhfr->settings.injection][hUhfr->settings.channel];
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_ANA2 + offset), ana2_val);

    fcw = 0x032b9d5;
    if (hUhfr->settings.channel == BUHF_ChanNum_eChan9) {
      fcw = ~fcw + 1;
    }
    BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FCW + offset), fcw);

    BKNI_Delay(20); /* TODO: get correct value */

    if (hUhfr->settings.injection == BUHF_InjType_eHigh)
    {
        if (hUhfr->settings.mode == BUHF_Mode_eLegacy) /* legacy mode regardless of Cust1 or Cust2 */
        {
            value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_FCW + offset));
            value = ~(value) + 1;
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_FCW + offset), value);
        }
        /* COR1, COR4, and COR7 are changed in advanced mode regardless of packet type;
           these 3 registers do not affect legacy mode*/
        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR1 + offset));
        value ^= 0x00000100;
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR1 + offset), value);

        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR4 + offset));
        value = ~(value);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR4 + offset), value);

        value = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_COR7 + offset));
        value = ~(value);
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_COR7 + offset), value);
    }

    return BERR_SUCCESS;
}

BERR_Code BUHF_GetDefaultStandbySettings(
    BUHF_Handle hUhf,
    BUHF_StandbySettings *pSettings
    )
{
    BDBG_ASSERT(hUhf);
    BDBG_ASSERT(pSettings);

    if(hUhf->settings.channel == BUHF_ChanNum_eChan9)
    {
        *pSettings = defStandbySettings;
        pSettings->ui32KeyPatternValue = 0x20D01068;
    }
    else
    {
        *pSettings = defStandbySettings;
    }
    return BERR_SUCCESS;
}

BERR_Code BUHF_Standby(
    BUHF_Handle hUhfr,
    const BUHF_StandbySettings *pSettings
    )
{
    BERR_Code   ret = BERR_SUCCESS;
    uint32_t    ui32RegVal;

    BDBG_ENTER(BUHF_Standby);

    BDBG_ASSERT(hUhfr);
    BDBG_ASSERT(pSettings);

    if(hUhfr->bStandbyState == true)
    {
        BDBG_WRN(("Already in Standby State. Doing nothing"));
        ret = BERR_SUCCESS;
        goto end;
    }

    if (!pSettings->bEnableWakeup) {
        /* power down everything */
        hUhfr->bFullPowerdown = true;
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
        BCHP_PWR_ReleaseResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif
    }
    else {
        /* otherwise, we must leave everything running, but lower the power duty cycle */
        hUhfr->bFullPowerdown = false;

        /* Enable automatic analog power down and long duty cycle power down for standby with wakeup
           Note, this is not a separate BCHP_PWR node because analog power is still on, but managed automatically by the HW */
        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD1));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_AUTOPD1, AUTO_PD_EN))
                       | (BCHP_MASK (UHFR_AUTOPD1, LDC_EN_AUTO_PD)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD1, AUTO_PD_EN, 1));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD1, LDC_EN_AUTO_PD, 1));
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD1), ui32RegVal );

        if (pSettings->bHwKeyFiltering == true)
        {
            ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5));
            ui32RegVal &= ~( (BCHP_MASK (UHFR_AUTOPD5, DIS_PKT_FILTER))
                                        |(BCHP_MASK (UHFR_AUTOPD5, EN_REPEAT_EN_INT)));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, DIS_PKT_FILTER, 0));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, EN_REPEAT_EN_INT, 0));
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5), ui32RegVal );

            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD4), pSettings->ui32KeyPatternValue);
        }
        else
        {
            ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5));
            ui32RegVal &= ~( (BCHP_MASK (UHFR_AUTOPD5, DIS_PKT_FILTER))
                            |(BCHP_MASK (UHFR_AUTOPD5, EN_REPEAT_EN_INT)));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, DIS_PKT_FILTER, 1));
            ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, EN_REPEAT_EN_INT, 0));
            BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5), ui32RegVal );
        }
    }

    hUhfr->bStandbyState = true;
end:
    BDBG_LEAVE(BUHF_Standby);
    return ret;
}

BERR_Code BUHF_Resume(BUHF_Handle hUhfr)
{
    BERR_Code   ret = BERR_SUCCESS;
    uint32_t    ui32RegVal;

    BDBG_ENTER(BUHF_Resume);

    BDBG_ASSERT(hUhfr);

    if(hUhfr->bStandbyState == false)
    {
        BDBG_WRN(("Already in Resume State. Doing nothing"));
        ret = BERR_SUCCESS;
        goto end;
    }

    if (hUhfr->bFullPowerdown) {
        /* power up everything */
        hUhfr->bFullPowerdown = false;
#ifdef BCHP_PWR_RESOURCE_UHF_INPUT
        BCHP_PWR_AcquireResource(hUhfr->hChip, BCHP_PWR_RESOURCE_UHF_INPUT);
#endif
    }
    else {
        /* otherwise, just turn off automatic analog power down, which restores full power state */
        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD1));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_AUTOPD1, AUTO_PD_EN))
                       | (BCHP_MASK (UHFR_AUTOPD1, LDC_EN_AUTO_PD)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD1, AUTO_PD_EN, 0));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD1, LDC_EN_AUTO_PD, 0));
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD1), ui32RegVal );

        ui32RegVal = BREG_Read32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5));
        ui32RegVal &= ~( (BCHP_MASK (UHFR_AUTOPD5, DIS_PKT_FILTER))
                            |(BCHP_MASK (UHFR_AUTOPD5, EN_REPEAT_EN_INT)));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, DIS_PKT_FILTER, 1));
        ui32RegVal |= (BCHP_FIELD_DATA (UHFR_AUTOPD5, EN_REPEAT_EN_INT, 1));
        BREG_Write32 (hUhfr->hRegister, (BCHP_UHFR_AUTOPD5), ui32RegVal );
    }

    hUhfr->bStandbyState = false;
end:
    BDBG_LEAVE(BUHF_Resume);
    return ret;
}
