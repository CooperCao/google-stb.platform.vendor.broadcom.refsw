/******************************************************************************
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
*****************************************************************************/

/*============================= Module Overview ===============================
<verbatim>
The SAT portinginterface controls the satellite demod/FEC on Broadcom chips.
The SAT PI is used on 28nm Broadcom chips and later.  For earlier Broadcom
satellite frontends, use the AST PI instead.
</verbatim>
=============================================================================*/

#ifndef _BSAT_H_
#define _BSAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bint.h"
#include "bkni.h"
#include "bfec.h"
#include "berr_ids.h"


#define BSAT_API_VERSION 10


/******************************************************************************
Summary:
   BSAT error codes.
Desciption:
   This is a list of return error codes specific to the BSAT PI.
See Also:
   None.
******************************************************************************/
#define BSAT_ERR_UNSUPPORTED_HW   BERR_MAKE_CODE(BERR_SAT_ID, 0) /* chip does not support this function */
#define BSAT_ERR_POWERED_DOWN     BERR_MAKE_CODE(BERR_SAT_ID, 1) /* unable to access h/w block due to it being powered down */
#define BSAT_ERR_NOT_IMPLEMENTED  BERR_MAKE_CODE(BERR_SAT_ID, 2) /* function not implemented */
#define BSAT_ERR_INVALID_STATE    BERR_MAKE_CODE(BERR_SAT_ID, 3) /* invalid s/w state */
#define BSAT_ERR_POWERUP_FAILED   BERR_MAKE_CODE(BERR_SAT_ID, 4) /* unable to power up h/w */
#define BSAT_ERR_POWERDOWN_FAILED BERR_MAKE_CODE(BERR_SAT_ID, 5) /* unable to power down h/w */
#define BSAT_ERR_INVALID_STREAM_ID BERR_MAKE_CODE(BERR_SAT_ID, 6) /* stream ID not found */


/******************************************************************************
Summary:
   Handle for the BSAT device.
Desciption:
   This is an opaque handle for the BSAT device.
See Also:
   BSAT_Open()
******************************************************************************/
typedef struct BSAT_P_Handle *BSAT_Handle;


/******************************************************************************
Summary:
   Handle for the BSAT channel device.
Desciption:
   This is an opaque handle for the BSAT channel device.
See Also:
   BSAT_OpenChannel()
******************************************************************************/
typedef struct BSAT_P_ChannelHandle *BSAT_ChannelHandle;


/******************************************************************************
Summary:
   Specifies the modulation, transmission format, code rate of the signal.
Desciption:
   This enum type specifies the modulation, transmission format, and code rate
	of the signal.
See Also:
   BSAT_Acquire(), BSAT_GetChannelStatus()
******************************************************************************/
typedef enum BSAT_Mode
{
   BSAT_Mode_eDvbs_scan = 0x00,     /* 0x00 = Legacy QPSK DVB-S code rate scan */
   BSAT_Mode_eDvbs_1_2,             /* 0x01 = Legacy QPSK DVB-S code rate 1/2 */
   BSAT_Mode_eDvbs_2_3,             /* 0x02 = Legacy QPSK DVB-S code rate 2/3 */
   BSAT_Mode_eDvbs_3_4,             /* 0x03 = Legacy QPSK DVB-S code rate 3/4 */
   BSAT_Mode_eDvbs_5_6,             /* 0x04 = Legacy QPSK DVB-S code rate 5/6 */
   BSAT_Mode_eDvbs_7_8,             /* 0x05 = Legacy QPSK DVB-S code rate 7/8 */
   BSAT_Mode_eDss_scan,             /* 0x06 = Legacy QPSK DTV code rate scan */
   BSAT_Mode_eDss_1_2,              /* 0x07 = Legacy QPSK DTV code rate 1/2 */
   BSAT_Mode_eDss_2_3,              /* 0x08 = Legacy QPSK DTV code rate 2/3 */
   BSAT_Mode_eDss_6_7,              /* 0x09 = Legacy QPSK DTV code rate 6/7 */
   BSAT_Mode_eDcii_scan,            /* 0x0A = Legacy QPSK DigicipherII code rate scan */
   BSAT_Mode_eDcii_1_2,             /* 0x0B = Legacy QPSK DigicipherII code rate 1/2 */
   BSAT_Mode_eDcii_2_3,             /* 0x0C = Legacy QPSK DigicipherII code rate 2/3 */
   BSAT_Mode_eDcii_3_4,             /* 0x0D = Legacy QPSK DigicipherII code rate 3/4 */
   BSAT_Mode_eDcii_5_6,             /* 0x0E = Legacy QPSK DigicipherII code rate 5/6 */
   BSAT_Mode_eDcii_7_8,             /* 0x0F = Legacy QPSK DigicipherII code rate 7/8 */
   BSAT_Mode_eDcii_5_11,            /* 0x10 = Legacy QPSK DigicipherII code rate 5/11 */
   BSAT_Mode_eDcii_3_5,             /* 0x11 = Legacy QPSK DigicipherII code rate 3/5 */
   BSAT_Mode_eDcii_4_5,             /* 0x12 = Legacy QPSK DigicipherII code rate 4/5 */
   BSAT_Mode_eDvbs2_Qpsk_1_4,       /* 0x13 = DVB-S2 QPSK code rate 1/4 */
   BSAT_Mode_eDvbs2_Qpsk_1_3,       /* 0x14 = DVB-S2 QPSK code rate 1/3 */
   BSAT_Mode_eDvbs2_Qpsk_2_5,       /* 0x15 = DVB-S2 QPSK code rate 2/5 */
   BSAT_Mode_eDvbs2_Qpsk_1_2,       /* 0x16 = DVB-S2 QPSK code rate 1/2 */
   BSAT_Mode_eDvbs2_Qpsk_3_5,       /* 0x17 = DVB-S2 QPSK code rate 3/5 */
   BSAT_Mode_eDvbs2_Qpsk_2_3,       /* 0x18 = DVB-S2 QPSK code rate 2/3 */
   BSAT_Mode_eDvbs2_Qpsk_3_4,       /* 0x19 = DVB-S2 QPSK code rate 3/4 */
   BSAT_Mode_eDvbs2_Qpsk_4_5,       /* 0x1A = DVB-S2 QPSK code rate 4/5 */
   BSAT_Mode_eDvbs2_Qpsk_5_6,       /* 0x1B = DVB-S2 QPSK code rate 5/6 */
   BSAT_Mode_eDvbs2_Qpsk_8_9,       /* 0x1C = DVB-S2 QPSK code rate 8/9 */
   BSAT_Mode_eDvbs2_Qpsk_9_10,      /* 0x1D = DVB-S2 QPSK code rate 9/10 */
   BSAT_Mode_eDvbs2_8psk_3_5,       /* 0x1E = DVB-S2 8PSK code rate 3/5 */
   BSAT_Mode_eDvbs2_8psk_2_3,       /* 0x1F = DVB-S2 8PSK code rate 2/3 */
   BSAT_Mode_eDvbs2_8psk_3_4,       /* 0x20 = DVB-S2 8PSK code rate 3/4 */
   BSAT_Mode_eDvbs2_8psk_5_6,       /* 0x21 = DVB-S2 8PSK code rate 5/6 */
   BSAT_Mode_eDvbs2_8psk_8_9,       /* 0x22 = DVB-S2 8PSK code rate 8/9 */
   BSAT_Mode_eDvbs2_8psk_9_10,      /* 0x23 = DVB-S2 8PSK code rate 9/10 */
   BSAT_Mode_eDvbs2_16apsk_2_3,     /* 0x24 = DVB-S2 16APSK code rate 2/3 */
   BSAT_Mode_eDvbs2_16apsk_3_4,     /* 0x25 = DVB-S2 16APSK code rate 3/4 */
   BSAT_Mode_eDvbs2_16apsk_4_5,     /* 0x26 = DVB-S2 16APSK code rate 4/5 */
   BSAT_Mode_eDvbs2_16apsk_5_6,     /* 0x27 = DVB-S2 16APSK code rate 5/6 */
   BSAT_Mode_eDvbs2_16apsk_8_9,     /* 0x28 = DVB-S2 16APSK code rate 8/9 */
   BSAT_Mode_eDvbs2_16apsk_9_10,    /* 0x29 = DVB-S2 16APSK code rate 9/10 */
   BSAT_Mode_eDvbs2_32apsk_3_4,     /* 0x2A = DVB-S2 32APSK code rate 3/4 */
   BSAT_Mode_eDvbs2_32apsk_4_5,     /* 0x2B = DVB-S2 32APSK code rate 4/5 */
   BSAT_Mode_eDvbs2_32apsk_5_6,     /* 0x2C = DVB-S2 32APSK code rate 5/6 */
   BSAT_Mode_eDvbs2_32apsk_8_9,     /* 0x2D = DVB-S2 32APSK code rate 8/9 */
   BSAT_Mode_eDvbs2_32apsk_9_10,    /* 0x2E = DVB-S2 32APSK code rate 9/10 */
   BSAT_Mode_eDvbs2_scan,           /* 0x2F = DVB-S2 code rate scan */
   BSAT_Mode_eDvbs2_ACM,            /* 0x30 = DVB-S2 ACM */
   BSAT_Mode_eTurbo_scan,           /* 0x31 = Turbo code rate scan */
   BSAT_Mode_eTurbo_Qpsk_1_2,       /* 0x32 = Turbo QPSK 1/2 */
   BSAT_Mode_eTurbo_Qpsk_2_3,       /* 0x33 = Turbo QPSK 2/3 */
   BSAT_Mode_eTurbo_Qpsk_3_4,       /* 0x34 = Turbo QPSK 3/4 */
   BSAT_Mode_eTurbo_Qpsk_5_6,       /* 0x35 = Turbo QPSK 5/6 */
   BSAT_Mode_eTurbo_Qpsk_7_8,       /* 0x36 = Turbo QPSK 7/8 */
   BSAT_Mode_eTurbo_8psk_2_3,       /* 0x37 = Turbo 8PSK 2/3 */
   BSAT_Mode_eTurbo_8psk_3_4,       /* 0x38 = Turbo 8PSK 3/4 */
   BSAT_Mode_eTurbo_8psk_4_5,       /* 0x39 = Turbo 8PSK 4/5 */
   BSAT_Mode_eTurbo_8psk_5_6,       /* 0x3A = Turbo 8PSK 5/6 */
   BSAT_Mode_eTurbo_8psk_8_9,       /* 0x3B = Turbo 8PSK 8/9 */
   BSAT_Mode_eDvbs2x_Qpsk_13_45,    /* 0x3C = DVB-S2X QPSK 13/45, pls=132 */
   BSAT_Mode_eDvbs2x_Qpsk_9_20,     /* 0x3D = DVB-S2X QPSK 9/20, pls=134 */
   BSAT_Mode_eDvbs2x_Qpsk_11_20,    /* 0x3E = DVB-S2X QPSK 11/20, pls=136 */
   BSAT_Mode_eDvbs2x_8apsk_5_9_L,   /* 0x3F = DVB-S2X 8APSK 5/9-L, pls=138 */
   BSAT_Mode_eDvbs2x_8apsk_26_45_L, /* 0x40 = DVB-S2X 8APSK 26/45-L, pls=140 */
   BSAT_Mode_eDvbs2x_8psk_23_36,    /* 0x41 = DVB-S2X 8APSK 23/36, pls=142 */
   BSAT_Mode_eDvbs2x_8psk_25_36,    /* 0x42 = DVB-S2X 8APSK 25/36, pls=144 */
   BSAT_Mode_eDvbs2x_8psk_13_18,    /* 0x43 = DVB-S2X 8APSK 13/18, pls=146 */
   BSAT_Mode_eDvbs2x_16apsk_1_2_L,  /* 0x44 = DVB-S2X 16APSK 1/2-L, pls=148 */
   BSAT_Mode_eDvbs2x_16apsk_8_15_L, /* 0x45 = DVB-S2X 16APSK 8/15-L, pls=150 */
   BSAT_Mode_eDvbs2x_16apsk_5_9_L,  /* 0x46 = DVB-S2X 16APSK 5/9-L, pls=152 */
   BSAT_Mode_eDvbs2x_16apsk_26_45,  /* 0x47 = DVB-S2X 16APSK 26/45, pls=154 */
   BSAT_Mode_eDvbs2x_16apsk_3_5,    /* 0x48 = DVB-S2X 16APSK 3/5, pls=156 */
   BSAT_Mode_eDvbs2x_16apsk_3_5_L,  /* 0x49 = DVB-S2X 16APSK 3/5-L, pls=158 */
   BSAT_Mode_eDvbs2x_16apsk_28_45,  /* 0x4A = DVB-S2X 16APSK 28/45, pls=160 */
   BSAT_Mode_eDvbs2x_16apsk_23_36,  /* 0x4B = DVB-S2X 16APSK 23/36, pls=162 */
   BSAT_Mode_eDvbs2x_16apsk_2_3_L,  /* 0x4C = DVB-S2X 16APSK 2/3-L, pls=164 */
   BSAT_Mode_eDvbs2x_16apsk_25_36,  /* 0x4D = DVB-S2X 16APSK 25/36, pls=166 */
   BSAT_Mode_eDvbs2x_16apsk_13_18,  /* 0x4E = DVB-S2X 16APSK 13/18, pls=168 */
   BSAT_Mode_eDvbs2x_16apsk_7_9,    /* 0x4F = DVB-S2X 16APSK 7/9, pls=170 */
   BSAT_Mode_eDvbs2x_16apsk_77_90,  /* 0x50 = DVB-S2X 16APSK 77/90, pls=172 */
   BSAT_Mode_eDvbs2x_32apsk_2_3_L,  /* 0x51 = DVB-S2X 32APSK 2/3-L, pls=174 */
   BSAT_Mode_eDvbs2x_32apsk_32_45,  /* 0x52 = DVB-S2X 32APSK 32/45, pls=178 */
   BSAT_Mode_eDvbs2x_32apsk_11_15,  /* 0x53 = DVB-S2X 32APSK 11/15, pls=180 */
   BSAT_Mode_eDvbs2x_32apsk_7_9,    /* 0x54 = DVB-S2X 32APSK 7/9, pls=182 */
   BSAT_Mode_eBlindScan,            /* 0x55 =Blind scan */
   BSAT_Mode_eUnknown = 0xFF        /* 0xFF =invalid mode, do not use */
} BSAT_Mode;


/* common macros for filtering BSAT_Mode */
#define BSAT_MODE_IS_LEGACY_QPSK(x)    (x <= BSAT_Mode_eDcii_4_5)
#define BSAT_MODE_IS_DVBS(x)           (x <= BSAT_Mode_eDvbs_7_8)
#define BSAT_MODE_IS_DTV(x)            ((x >= BSAT_Mode_eDss_scan ) && (x <= BSAT_Mode_eDss_6_7 ))
#define BSAT_MODE_IS_DCII(x)           ((x >= BSAT_Mode_eDcii_scan) && (x <= BSAT_Mode_eDcii_4_5))
#define BSAT_MODE_IS_DVBS2(x)          ((x >= BSAT_Mode_eDvbs2_Qpsk_1_4) && (x <= BSAT_Mode_eDvbs2_ACM))
#define BSAT_MODE_IS_DVBS2_QPSK(x)     ((x >= BSAT_Mode_eDvbs2_Qpsk_1_4) && (x <= BSAT_Mode_eDvbs2_Qpsk_9_10))
#define BSAT_MODE_IS_DVBS2_8PSK(x)     ((x >= BSAT_Mode_eDvbs2_8psk_3_5) && (x <= BSAT_Mode_eDvbs2_8psk_9_10))
#define BSAT_MODE_IS_DVBS2_16APSK(x)   ((x >= BSAT_Mode_eDvbs2_16apsk_2_3) && (x <= BSAT_Mode_eDvbs2_16apsk_9_10))
#define BSAT_MODE_IS_DVBS2_32APSK(x)   ((x >= BSAT_Mode_eDvbs2_32apsk_3_4) && (x <= BSAT_Mode_eDvbs2_32apsk_9_10))
#define BSAT_MODE_IS_DVBS2_EXTENDED(x) ((x >= BSAT_Mode_eDvbs2_16apsk_2_3) && (x <= BSAT_Mode_eDvbs2_32apsk_9_10))
#define BSAT_MODE_IS_TURBO(x)          ((x >= BSAT_Mode_eTurbo_scan) && (x <= BSAT_Mode_eTurbo_8psk_8_9))
#define BSAT_MODE_IS_TURBO_QPSK(x)     ((x >= BSAT_Mode_eTurbo_Qpsk_1_2) && (x <= BSAT_Mode_eTurbo_Qpsk_7_8))
#define BSAT_MODE_IS_TURBO_8PSK(x)     ((x >= BSAT_Mode_eTurbo_8psk_2_3) && (x <= BSAT_Mode_eTurbo_8psk_8_9))
#define BSAT_MODE_IS_DVBS2X(x)         ((x >= BSAT_Mode_eDvbs2x_Qpsk_13_45) && (x <= BSAT_Mode_eDvbs2x_32apsk_7_9))
#define BSAT_MODE_IS_DVBS2X_QPSK(x)    ((x >= BSAT_Mode_eDvbs2x_Qpsk_13_45) && (x <= BSAT_Mode_eDvbs2x_Qpsk_11_20))
#define BSAT_MODE_IS_DVBS2X_8PSK(x)    ((x >= BSAT_Mode_eDvbs2x_8psk_23_36) && (x <= BSAT_Mode_eDvbs2x_8psk_13_18))
#define BSAT_MODE_IS_DVBS2X_8APSK(x)   ((x >= BSAT_Mode_eDvbs2x_8apsk_5_9_L) && (x <= BSAT_Mode_eDvbs2x_8apsk_26_45_L))
#define BSAT_MODE_IS_DVBS2X_16APSK(x)  ((x >= BSAT_Mode_eDvbs2x_16apsk_1_2_L) && (x <= BSAT_Mode_eDvbs2x_16apsk_77_90))
#define BSAT_MODE_IS_DVBS2X_32APSK(x)  ((x >= BSAT_Mode_eDvbs2x_32apsk_2_3_L) && (x <= BSAT_Mode_eDvbs2x_32apsk_7_9))


/******************************************************************************
Summary:
   Bit definitions for BSAT_AcqSettings.options.
Desciption:
   The following bit definitions specify acquisition options for BSAT_Acquire().
   BSAT_AcqSettings.options is the bitwise OR of the BSAT_ACQ_* flags below.
See Also:
   BSAT_AcqSettings()
******************************************************************************/
#define BSAT_ACQ_PILOT              0x00000001 /* DVB-S2 only: 0=no pilot, 1=pilot */
#define BSAT_ACQ_PILOT_SCAN_ENABLE  0x00000002 /* DVB-S2 only: 1=enable pilot scan, 0=pilot setting specified by BSAT_ACQ_PILOT bit */
#define BSAT_ACQ_DISABLE_SPINV_SCAN 0x00000004 /* spectral inversion scan: 0=enabled, 1=disabled (use BSAT_ACQ_SPINV) */
#define BSAT_ACQ_SPINV              0x00000008 /* if BSAT_ACQ_DISABLE_SPINV_SCAN=1: 0=normal, 1=inverted */
#define BSAT_ACQ_NYQUIST_MASK       0x00000030 /* nyquist roll-off select: 0=20%, 1=35%, 2=10%, 3=5% */
#define BSAT_ACQ_NYQUIST_35         0x00000010
#define BSAT_ACQ_NYQUIST_10         0x00000020
#define BSAT_ACQ_NYQUIST_20         0x00000000
#define BSAT_ACQ_NYQUIST_5          0x00000030
#define BSAT_ACQ_DCII_SPLIT         0x00000040 /* DCII only: 0=combined, 1=split */
#define BSAT_ACQ_DCII_SPLIT_Q       0x00000080 /* DCII split mode only: 0=split I, 1=split Q */
#define BSAT_ACQ_OQPSK              0x00000100 /* DCII only: 0=QPSK, 1=OQPSK */
#define BSAT_ACQ_DVBS2_SHORT_FRAMES 0x00000200 /* DVB-S2 only: 0=normal frames, 1=short frames */
#define BSAT_ACQ_DISABLE_REACQ      0x00000400 /* Auto Reacquisitions: 0=enable, 1=disable */
#define BSAT_ACQ_CHAN_BOND          0x00000800 /* set to 1 if this channel is being used for channel bonding */
#define BSAT_ACQ_DISABLE_TRACKING   0x20000000 /* 0=normal, 1=acquire only (don't track) */
#define BSAT_ACQ_TUNER_TEST_MODE    0x40000000 /* Tuner Test Mode: 0=disable, 1=enable */
#define BSAT_ACQ_ENABLE_BERT        0x80000000 /* internal BERT: 0=disable, 1=enable */
#define BSAT_ACQ_DEFAULT            0x00000001 /* default settings */


/******************************************************************************
Summary:
   Structure containing acquisition parameters.
Desciption:
   This structure contains commanded channel acquisition parameters.
See Also:
   BSAT_Acquire(), BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_AcqSettings
{
   BSAT_Mode mode;       /* modulation, code rate */
   uint32_t  symbolRate; /* symbol rate in sym/sec */
   uint32_t  freq;       /* center frequency in Hz */
   uint32_t  options;    /* Bitwise-OR of BSAT_ACQ_* (see above) */
   uint32_t  adcSelect;  /* ADC selection (0-based index) */
} BSAT_AcqSettings;


/******************************************************************************
Summary:
   Specifies PRBS mode for BERT.
Desciption:
   This enum specifies the mode for the internal BERT.
See Also:
   BSAT_BertSettings, BSAT_SetBertSettings(), BSAT_GetBertSettings()
******************************************************************************/
typedef enum BSAT_BertMode
{
   BSAT_BertMode_ePRBS23 = 0,
   BSAT_BertMode_ePRBS15,
   BSAT_BertMode_ePRBS31
} BSAT_BertMode;


/******************************************************************************
Summary:
   Structure containing BERT settings.
Desciption:
   This structure contains settings used to configure the internal BERT during
   channel acquisition.  The BERT is enabled and these settings are applied
	only when BSAT_AcqSettings.options bit 0 (BSAT_ACQ_ENABLE_BERT) is set.
See Also:
   BSAT_SetBertSettings(), BSAT_GetBertSettings()
******************************************************************************/
typedef struct BSAT_BertSettings
{
   BSAT_BertMode mode;    /* PRBS mode */
   bool          bInvert; /* true=PN data is inverted */
} BSAT_BertSettings;


/******************************************************************************
Summary:
   Structure containing BERT status.
Desciption:
   This structure contains BERT status information returned by
	BSAT_GetBertStatus().
See Also:
   BSAT_GetBertStatus()
******************************************************************************/
typedef struct BSAT_BertStatus
{
   bool bEnabled; /* true={BERT is enabled in the current acquisition} */
   bool bLock;    /* true={BERT is currently locked} */
   bool bSyncLost;     /* true={BERT has lost sync since the last call to BSAT_GetBertStatus() or BSAT_ResetChannelStatus()} */
   bool bSyncAcquired; /* true={BERT has acquired sync since the last call to BSAT_GetBertStatus() or BSAT_ResetChannelStatus()} */
   uint32_t bitCountHi;   /* total bit count, bits 47:32 */
   uint32_t bitCountLo;   /* total bit count, bits 31:0 */
   uint32_t errorCountHi; /* BER error count, bits 40:32 */
   uint32_t errorCountLo; /* BER error count, bits 31:0 */
} BSAT_BertStatus;


/******************************************************************************
Summary:
   Structure containing legacy QPSK channel status.
Desciption:
   This structure contains legacy QPSK channel status information.
See Also:
   BSAT_ChannelStatus, BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_QpskStatus
{
   bool      bValid;         /* true={all data in this struct is valid} */
   uint32_t  rsCorrCount;    /* accumulated number of RS correctable errors */
   uint32_t  rsUncorrCount;  /* accumulated number of RS uncorrectable errors */
   uint32_t  preVitErrCount; /* Viterbi re-encoder error count */
} BSAT_QpskStatus;


/******************************************************************************
Summary:
   Structure containing DVB-S2 channel status.
Desciption:
   This structure contains DVB-S2 channel status information.
See Also:
   BSAT_ChannelStatus, BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_Dvbs2Status
{
   bool     bValid;      /* true={all data in this struct is valid}, false={AFEC core is not powered so the rest of the data in this struct is invalid} */
   uint32_t totalBlocks; /* accumulated number of BCH blocks */
   uint32_t corrBlocks;  /* accumulated number of BCH corrected blocks */
   uint32_t badBlocks;   /* accumulated number of BCH bad blocks */
   uint32_t fecFreq;     /* FEC clock frequency in Hz */
   bool     bPilot;      /* true=pilot */
   bool     bShortFrame; /* true={short FEC frames} */
} BSAT_Dvbs2Status;


/******************************************************************************
Summary:
   Structure containing Turbo channel status.
Desciption:
   This structure contains Turbo channel status information.
See Also:
   BSAT_ChannelStatus, BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_TurboStatus
{
   bool     bValid;      /* true={all data in this struct is valid}, false={TFEC core is not powered so the rest of the data in this struct is invalid} */
   uint32_t totalBlocks; /* accumulated number of blocks */
   uint32_t corrBlocks;  /* accumulated number of corrected blocks */
   uint32_t badBlocks;   /* accumulated number of bad blocks */
   uint32_t fecFreq;     /* FEC clock frequency in Hz */
} BSAT_TurboStatus;


/******************************************************************************
Summary:
   Structure containing AGC status.
Desciption:
   This structure contains AGC register values to be used to compute an input
   power estimate.
See Also:
   BSAT_ChannelStatus, BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_AgcStatus
{
   uint32_t flags;    /* used for input power calculation */
   uint32_t value[3]; /* AGC values */
} BSAT_AgcStatus;


/* bit definitions for BSAT_ChannelStatus.notification */
#define BSAT_STATUS_NOTIFICATION_RAIN_FADE  0x01 /* rain fade event */
#define BSAT_STATUS_NOTIFICATION_FREQ_DRIFT 0x02 /* frequency drift event */
#define BSAT_STATUS_NOTIFICATION_THRESHOLD1 0x04 /* threshold1 status event */
#define BSAT_STATUS_NOTIFICATION_THRESHOLD2 0x08 /* threshold2 status event */


/******************************************************************************
Summary:
   Structure containing channel status.
Desciption:
   This structure contains channel status information.
See Also:
   BSAT_GetChannelStatus()
******************************************************************************/
typedef struct BSAT_ChannelStatus
{
   bool             bTunerLocked;    /* true={tuner is locked} */
   bool             bDemodLocked;    /* true={demod/FEC are locked} */
   BSAT_AcqSettings acqSettings;     /* commanded acquisition settings */
   BSAT_Mode        mode;            /* actual mode acquired */
   int32_t          symbolRateError; /* symbol rate error in sym/sec */
   int32_t          carrierError;    /* carrier frequency offset in Hz */
   uint32_t         outputBitrate;   /* transport output bit rate in bits/sec */
   uint32_t         mpegCount;       /* accumulated number of MPEG frames */
   uint32_t         mpegErrors;      /* accumulated number of MPEG frame errors */
   uint32_t         snr;             /* SNR estimate in units of 1/256 dB */
   uint32_t         reacqCount;      /* number of re-acquisitions performed */
   uint32_t         reacqCause;      /* reason for last reacquisition */
   uint32_t         sampleFreq;      /* sample frequency in Hz */
   uint32_t         aciBandwidth;    /* ACI filter bandwidth in Hz */
   uint32_t         notification;    /* see BSAT_STATUS_NOTIFICATION_* */
   BSAT_AgcStatus   agc;             /* raw AGC values used to compute input power estimate */
   bool             bSpinv;          /* true={spectum is inverted}, false={normal spectrum} */
   union
   {
      BSAT_QpskStatus  qpsk;  /* legacy QPSK specific status */
      BSAT_Dvbs2Status dvbs2; /* DVB-S2 specific status */
      BSAT_TurboStatus turbo; /* Turbo specific status */
   } modeStatus;
} BSAT_ChannelStatus;


/******************************************************************************
Summary:
   Specifies the service provider.
Desciption:
   This enum specifies the service provider.  The network spec is used to
   optimize acquisition settings based on network provider.
See Also:
   BSAT_SetNetworkSpec(), BSAT_GetNetworkSpec()
******************************************************************************/
typedef enum BSAT_NetworkSpec
{
   BSAT_NetworkSpec_eDefault = 0,
   BSAT_NetworkSpec_eEcho,
   BSAT_NetworkSpec_eEuro
} BSAT_NetworkSpec;


/***************************************************************************
Summary:
   Enum to specify MPEG packetizer error mode for DVB-S2 acquisitions.
Description:
   Enum to specify MPEG packetizer error mode for DVB-S2 acquisitions.
See Also:
   BSAT_OutputTransportSettings
****************************************************************************/
typedef enum BSAT_BchMpegErrorMode
{
   BSAT_BchMpegErrorMode_eBchOrCrc8, /* (BCH flag) AND (CRC8 flag) */
   BSAT_BchMpegErrorMode_eCrc8,      /* CRC8 flag */
   BSAT_BchMpegErrorMode_eBch,       /* BCH flag */
   BSAT_BchMpegErrorMode_eBchAndCrc8 /* (BCH flag) AND (CRC8 flag) */
} BSAT_BchMpegErrorMode;


/***************************************************************************
Summary:
   Structure containing MPEG transport output settings.
Description:
   Structure containing MPEG transport output settings.
See Also:
   BSAT_GetOutputTransportSettings(), BSAT_SetOutputTransportSettings()
****************************************************************************/
typedef struct BSAT_OutputTransportSettings
{
   bool bSerial;  /* false={parallel output}, true={serial output} */
   bool bDataInv; /* false={normal DATA}, true={inverted DATA} */
   bool bClkInv;  /* false={normal CLK}, true={inverted CLK} */
   bool bClkSup;  /* false={CLK runs continuously}, true={CLK is suppressed when VALID not active} */
   bool bVldInv;  /* false={VALID is active high}, true={VALID is active low} */
   bool bSyncInv; /* false={SYNC is active high}, true={SYNC is active low} */
   bool bErrInv;  /* false={ERR is active high}, true={ERR is active low} */
   bool bXbert;   /* false={no suppression of CLK based on ERR}, true={suppress CLK when ERR asserted} */
   bool bTei;     /* transport error indicator flag in MPEG2 transport: false=off, true=on */
   bool bDelay;   /* output delay: false={normal operation}, true={delay ERR, VALID, and DATA relative to CLK} */
   bool bSync1;   /* SYNC duration in serial mode: false={no effect (SYNC is either 1 byte (bHead4=false), or 4 bytes (bHead4=true)}, true={SYNC will be valid for 1 bit in serial mode.  Has no effect in parallel mode.  In parallel mode, SYNC duration is defined by bHead4.} */
   bool bHead4;   /* mpeg packet header length, defines the duration of VALID and SYNC: true={header is 4 bytes}, false={header is 1 byte} */
   bool bDelHeader;  /* true={delete MPEG header} */
   bool bOpllBypass; /* true={bypass OPLL}, false={use normal OPLL setting} */
   BSAT_BchMpegErrorMode bchMpegErrorMode; /* MPEG error mode for LDPC BCH */
} BSAT_OutputTransportSettings;


/******************************************************************************
Summary:
   Structure containing status from most recent tone detect operation.
Desciption:
   This structure contains status from most recent tone detect operation.
See Also:
   BSAT_StartToneDetect, BSAT_GetToneDetectStatus
******************************************************************************/
typedef struct BSAT_ToneDetectStatus
{
   bool bEnabled;      /* true={tone detection has started} */
   bool bScanDone;     /* true={peak scan has completed, status is valid}, false={the rest of the status in this struct is invalid} */
   BERR_Code status;   /* status of peak scan operation on completion */
   uint32_t tunerFreq; /* commanded tuner frequency (in Hz) from most recent call to BSAT_StartToneDetect() */
   uint32_t peakPower; /* peak power raw value */
   uint32_t peakBin;   /* bin number at peak power */
} BSAT_ToneDetectStatus;


/******************************************************************************
Summary:
   Structure containing status from most recent symbol rate scan operation.
Desciption:
   This structure contains status from most recent symbol rate scan operation.
See Also:
   BSAT_StartSymbolRateScan, BSAT_GetSymbolRateScanStatus
******************************************************************************/
typedef struct BSAT_SymbolRateScanStatus
{
   bool bEnabled;       /* true={symbol rate scan has started} */
   bool bScanDone;      /* true={symbol rate scan has completed, status is valid}, false={the rest of the status in this struct is invalid} */
   BERR_Code status;    /* status of the symbol rate scan operation on completion */
   uint32_t tunerFreq;  /* commanded tuner frequency (in Hz) from most recent call to BSAT_StartSymbolRateScan() */
   uint32_t minSymbolRate; /* min symbol rate from most recent call to BSAT_StartSymbolRateScan() */
   uint32_t maxSymbolRate; /* max symbol rate from most recent call to BSAT_StartSymbolRateScan() */
   uint32_t peakPower;  /* peak power raw value */
   uint32_t symbolRate; /* symbol rate estimate */
} BSAT_SymbolRateScanStatus;


/******************************************************************************
Summary:
   Structure containing status from most recent PSD scan operation.
Desciption:
   This structure contains status from most recent PSD scan operation.
See Also:
   BSAT_StartPsdScan, BSAT_GetPsdScanStatus
******************************************************************************/
typedef struct BSAT_PsdScanStatus
{
   bool bEnabled;      /* true={PSD scan has started} */
   bool bScanDone;     /* true={PSD scan has completed, status is valid}, false={the rest of the status in this struct is invalid} */
   BERR_Code status;   /* status of PSD scan operation on completion */
   uint32_t tunerFreq; /* commanded tuner frequency (in Hz) from most recent call to BSAT_StartPsdScan() */
   uint32_t power;     /* PSD raw value */
} BSAT_PsdScanStatus;


/******************************************************************************
Summary:
   Structure specifying AMC scrambling sequence used in DVB-S2.
Desciption:
   Structure specifying AMC scrambling sequence used in DVB-S2.
See Also:
   BSAT_SetAmcScramblingSeq
******************************************************************************/
typedef struct BSAT_AmcScramblingSeq
{
   uint32_t xseed;       /* physical layer scrambler seed */
   uint32_t plhdrscr1;   /* PL Header Scrambling Sequence Bit[31:0] */
   uint32_t plhdrscr2;   /* PL Header Scrambling Sequence Bit[63:32] */
   uint32_t plhdrscr3;   /* PL Header Scrambling Sequence Bit[89:64] */
} BSAT_AmcScramblingSeq;


/******************************************************************************
Summary:
   Structure used to configure the status notification event.
Desciption:
   Structure used to configure the status notification event.
See Also:
   BSAT_SetSignalNotificationSettings, BSAT_GetSignalNotificationSettings
******************************************************************************/
typedef struct BSAT_SignalNotificationSettings
{
   uint32_t rainFadeThreshold;  /* SNR drop threshold in units of 1/256 dB SNR */
   uint32_t rainFadeWindow;     /* time window for rain fade event in units of 100 msecs */
   uint32_t freqDriftThreshold; /* carrier offset threshold in Hz for frequency drift */
} BSAT_SignalNotificationSettings;


/***************************************************************************
Summary:
   Structure containing signal detect status.
Description:
   This structure contains the status on the most recent signal detect
   operation initiated by BSAT_Acquire().
See Also:
   BSAT_GetSignalDetectStatus(), BSAT_StartSignalDetect()
****************************************************************************/
typedef struct BSAT_SignalDetectStatus
{
   bool bEnabled;          /* true={signal detect mode was enabled in acquisition} */
   bool bDone;             /* true={bEnabled AND signal detection operation has completed} */
   bool bTimingLoopLocked; /* true={bDone=1 AND timing loop is locked} */
} BSAT_SignalDetectStatus;


/***************************************************************************
Summary:
   Structure containing extended acquisition settings.
Description:
   This structure contains settings typically not changed during normal
   operation.  These settings are used mainly for debugging.
See Also:
   BSAT_GetExtAcqSettings(), BSAT_SetExtAcqSettings()
****************************************************************************/
typedef struct BSAT_ExtAcqSettings
{
   bool     bForceUnlockOutsideSearchRange; /* carrier drifts outside of search range: true={force unlock}, false={do nothing} */
   bool     bBypassDft;       /* true={bypass DFT carrier search} */
   bool     bDontRetuneOnReacquire;         /* true={disable retune before reacquisition} */
   bool     bPreserveState;   /* true={preserve hardware state on acquisition failure when auto reacquisition is disabled} */
   bool     bPreserveCommandedTunerFreq;    /* true={dont recenter LO to detected carrier offset} */
   bool     bDisableNotch;    /* true={disable all DCO notch filters} */
   bool     bCheckTimingLoop; /* true={forces software to verify timing loop lock before proceeding with acquisition} */
   bool     bOverrideFiltctl; /* true={override FILTCTL setting} */
   uint32_t filtctlOverride;  /* value to set FILTCTL when bOverrideFiltctl=true */
   uint32_t maxReacqs;        /* maximum number of reacqs to attempt, 0=no reacquisition limit (default) */
   uint8_t  blindScanModes;   /* BSAT_Modes searched in blind scan acquisition: bitwise OR of BSAT_BLIND_SCAN_MODE_* */
   uint8_t  snrEstMethod;     /* SNR estimation method: 0=auto(default), 1=snre, 2=snore */
} BSAT_ExtAcqSettings;


/***************************************************************************
Summary:
   Structure containing notch filter setttings.
Description:
   This structure contains settings typically not changed during normal
   operation.  These are used mainly for troubleshooting.
See Also:
   BSAT_GetNotchSettings(), BSAT_SetNotchSettings()
****************************************************************************/
typedef struct BSAT_NotchSettings
{
   uint32_t freq;  /* frequency of the notch filter in Hz */
   uint8_t  alpha; /* depth of DC notch filter */
} BSAT_NotchSettings;


/******************************************************************************
Summary:
   Legacy QPSK acquisition settings
Description:
   These settings apply only to Legacy QPSK acquisitions.
See Also:
   BSAT_SetLegacyQpskAcqSettings(), BSAT_GetLegacyQpskAcqSettings()
******************************************************************************/
typedef struct BSAT_LegacyQpskAcqSettings
{
   bool    bRsDisable;    /* true={disable Reed Solomon}, false={normal mode} */
   uint8_t dtvScanModes;  /* code rates searched in DTV scan mode: bitwise OR of BSAT_SCAN_MODE_DTV_* */
   uint8_t dvbsScanModes; /* code rates searched in DVB-S scan mode: bitwise OR of BSAT_SCAN_MODE_DVBS_* */
   uint8_t dciiScanModes; /* code rates searched in DCII scan mode: bitwise OR of BSAT_SCAN_MODE_DCII_* */
   uint8_t stuffBytes;    /* number of null bytes to insert in each frame */
} BSAT_LegacyQpskAcqSettings;


/******************************************************************************
Summary:
   DVB-S2 acquisition settings
Description:
   These settings apply only to DVB-S2 acquisitions.
See Also:
   BSAT_SetDvbs2AcqSettings(), BSAT_GetDvbs2AcqSettings()
******************************************************************************/
typedef struct BSAT_Dvbs2AcqSettings
{
   uint32_t  scanModes;  /* modes searched in DVB-S2 scan mode: bitwise OR of BSAT_SCAN_MODE_DVBS2_* */
   uint32_t  ctl;        /* see BSAT_DVBS2_CTL_* */
} BSAT_Dvbs2AcqSettings;


/******************************************************************************
Summary:
   DVB-S2X acquisition settings
Description:
   These settings apply only to DVB-S2 acquisitions.
See Also:
   BSAT_SetDvbs2xAcqSettings(), BSAT_GetDvbs2xAcqSettings()
******************************************************************************/
typedef struct BSAT_Dvbs2xAcqSettings
{
   uint32_t  scanModes;  /* modes searched in DVB-S2X scan mode: bitwise OR of BSAT_SCAN_MODE_DVBS2X_* */
} BSAT_Dvbs2xAcqSettings;


/******************************************************************************
Summary:
   Turbo acquisition settings
Description:
   These settings apply only to Turbo acquisitions.
See Also:
   BSAT_SetTurboAcqSettings(), BSAT_GetTurboAcqSettings()
******************************************************************************/
typedef struct BSAT_TurboAcqSettings
{
   uint32_t scanModes;    /* modes searched in Turbo scan mode: bitwise OR of BSAT_SCAN_MODE_TURBO_* */
   uint32_t ctl;          /* see BSAT_TURBO_CTL_* */
   uint32_t tzsyOverride; /* TZSY value if BSAT_TURBO_CTL_OVERRIDE_TZSY bit is set in ctl */
} BSAT_TurboAcqSettings;


/******************************************************************************
Summary:
   Serial BERT output settings.
Description:
   These settings configure the serial BERT output.
See Also:
   BSAT_SetExternalBertSettings(), BSAT_GetExternalBertSettings()
******************************************************************************/
typedef struct BSAT_ExternalBertSettings
{
   bool     bEnable; /* true={enable serial BERT output}, false={disable serial BERT output} */
   bool     bClkInv; /* true={invert BERT_CLK output} */
   uint32_t channel; /* SDS channel to output (0-based index) */
} BSAT_ExternalBertSettings;


/******************************************************************************
Summary:
   Capture mode for soft decisions in ACM mode.
Description:
   Capture mode for soft decisions in ACM mode.
See Also:
   BSAT_AcmSettings
******************************************************************************/
typedef enum BSAT_AcmSoftDecisionCapture
{
   BSAT_AcmSoftDecisionCapture_eAll = 0,   /* capture data from every frame (no filtering) */
   BSAT_AcmSoftDecisionCapture_eMode,      /* capture data that match a given BSAT_Mode */
   BSAT_AcmSoftDecisionCapture_eModulation /* capture data that match a given BSAT_Modulation */
} BSAT_AcmSoftDecisionCapture;


/******************************************************************************
Summary:
   ACM SNR filtering mode
Description:
   ACM SNR filtering mode
See Also:
   BSAT_AcmSettings
******************************************************************************/
typedef enum BSAT_AcmSnrFilter
{
   BSAT_AcmSnrFilter_eNone = 0,         /* no filtering */
   BSAT_AcmSnrFilter_eMode,         /* filter by given BSAT_Mode */
   BSAT_AcmSnrFilter_eModulation,   /* filter by given modulation */
   BSAT_AcmSnrFilter_eModulationLE  /* filter by modulations less than or equal to given modulation */
} BSAT_AcmSnrFilter;


/******************************************************************************
Summary:
   Modulation types
Description:
   Modulation types
See Also:
   BSAT_AcmSettings()
******************************************************************************/
typedef enum BSAT_Modulation
{
   BSAT_Modulation_eQpsk = 0,   /* QPSK */
   BSAT_Modulation_e8psk,   /* 8PSK */
   BSAT_Modulation_e8apsk,  /* 8APSK */
   BSAT_Modulation_e16apsk, /* 16APSK */
   BSAT_Modulation_e32apsk  /* 32APSK */
} BSAT_Modulation;


/******************************************************************************
Summary:
   ACM soft decision capture settings
Description:
   Specifies how soft decisions are captured in ACM mode
See Also:
   BSAT_AcmSettings()
******************************************************************************/
typedef struct BSAT_AcmSoftDecisionCaptureSettings
{
   BSAT_AcmSoftDecisionCapture captureMode; /* soft decision capture mode */
   union {
      BSAT_Mode       mode;
      BSAT_Modulation modulation;
   } param; /* only applies when capture mode is mode or modulation */
} BSAT_AcmSoftDecisionCaptureSettings;


/******************************************************************************
Summary:
   SNR filtering settings for ACM mode
Description:
   SNR filtering settings for ACM mode
See Also:
   BSAT_AcmSettings()
******************************************************************************/
typedef struct BSAT_AcmSnrFilterSettings
{
   BSAT_AcmSnrFilter  filterMode; /* SNR filtering mode */
   union {
      BSAT_Mode       mode; /* applies when filterMode is BSAT_AcmSnrFilter_eMode */
      BSAT_Modulation modulation; /* applies when filterMode is BSAT_AcmSnrFilter_eModulation or BSAT_AcmSnrFilter_eModulationLE */
   } param;
} BSAT_AcmSnrFilterSettings;


/******************************************************************************
Summary:
   BERT settings for ACM mode
Description:
   BERT settings for ACM mode
See Also:
   BSAT_AcmSettings()
******************************************************************************/
typedef struct BSAT_AcmBertSettings
{
   bool bEnable; /* false=BERT ACM disabled, true=BERT ACM enabled */
   bool bFilterByStreamId; /* false=filter by mode, true=filter by stream ID */
   union {
      BSAT_Mode mode;     /* applies when bFilterByStreamId is false */
      uint8_t   streamId; /* applies when bFilterByStreamId is true */
   } param;
} BSAT_AcmBertSettings;


/******************************************************************************
Summary:
   ACM settings.
Description:
   These settings apply to ACM mode, which is only supported in SDS core 7.4
   and later (i.e. BCM45308-B0 and later).
See Also:
   BSAT_SetAcmSettings(), BSAT_GetAcmSettings()
******************************************************************************/
typedef struct BSAT_AcmSettings
{
   BSAT_AcmSoftDecisionCaptureSettings softDecisions;
   BSAT_AcmSnrFilterSettings           snr;
   BSAT_AcmBertSettings                bert;
} BSAT_AcmSettings;


/******************************************************************************
Summary:
   Stream status
Description:
   Stream status
See Also:
   BSAT_GetStreamStatus()
******************************************************************************/
typedef struct BSAT_StreamStatus
{
   bool      bValid;         /* true=status is valid, false=status is not valid */
   BSAT_Mode mode;           /* BSAT_Mode of the stream */
   uint32_t  mpegFrameCount; /* number of MPEG frames received in the output interface */
   uint32_t  mpegErrorCount; /* number of MPEG frames with error */
   uint32_t  iterationCount; /* iteration count */
   uint32_t  ldpcFrameCount; /* number of LDPC frames that got sent to LDPC decoder */
   uint32_t  ldpcErrorCount; /* number of LDPC frames that failed */
   uint32_t  totalBlocks;    /* number of BCH blocks decoded */
   uint32_t  corrBlocks;     /* number of corrected BCH blocks */
   uint32_t  badBlocks;      /* number of bad BCH blocks */
   uint32_t  corrBits;       /* number of corrected bits */
   uint8_t   matype;         /* MATYPE-1 for the stream */
   uint8_t   pls;            /* PLS code for the stream */
   uint8_t   streamId;       /* stream ID */
   uint8_t   lock;           /* 1=locked, 0=not locked */
} BSAT_StreamStatus;


/* bit definitions for BSAT_ExtAcqSettings.blindScanModes */
#define BSAT_BLIND_SCAN_MODE_DVBS   0x01 /* search DVB-S modes */
#define BSAT_BLIND_SCAN_MODE_TURBO  0x02 /* search Turbo modes */
#define BSAT_BLIND_SCAN_MODE_DVBS2  0x04 /* search DVB-S2 modes */
#define BSAT_BLIND_SCAN_MODE_DTV    0x08 /* search DTV modes */
#define BSAT_BLIND_SCAN_MODE_DCII   0x10 /* search Digicipher-II modes */
#define BSAT_BLIND_SCAN_MODE_MASK   0x1F /* search DVB-S, Turbo, DVB-S2, DTV, and Digicipher-II */

/* bit definitions for BSAT_LegacyQpskAcqSettings.dtvScanModes */
#define BSAT_SCAN_MODE_DTV_1_2 0x01 /* search DTV 1/2 */
#define BSAT_SCAN_MODE_DTV_2_3 0x02 /* search DTV 2/3 */
#define BSAT_SCAN_MODE_DTV_6_7 0x04 /* search DTV 6/7 */
#define BSAT_SCAN_MODE_DTV_ALL 0x07 /* search all DTV code rates */

/* bit definitions for BSAT_LegacyQpskAcqSettings.dvbsScanModes */
#define BSAT_SCAN_MODE_DVBS_1_2 0x01 /* search DVB-S 1/2 */
#define BSAT_SCAN_MODE_DVBS_2_3 0x02 /* search DVB-S 2/3 */
#define BSAT_SCAN_MODE_DVBS_3_4 0x04 /* search DVB-S 3/4 */
#define BSAT_SCAN_MODE_DVBS_5_6 0x08 /* search DVB-S 5/6 */
#define BSAT_SCAN_MODE_DVBS_7_8 0x10 /* search DVB-S 7/8 */
#define BSAT_SCAN_MODE_DVBS_ALL 0x1F /* search all DVB-S code rates */

/* bit definitions for BSAT_LegacyQpskAcqSettings.dciiScanModes */
#define BSAT_SCAN_MODE_DCII_1_2  0x01 /* search Digicipher-II 1/2 */
#define BSAT_SCAN_MODE_DCII_2_3  0x02 /* search Digicipher-II 2/3 */
#define BSAT_SCAN_MODE_DCII_3_4  0x04 /* search Digicipher-II 3/4 */
#define BSAT_SCAN_MODE_DCII_5_6  0x08 /* search Digicipher-II 5/6 */
#define BSAT_SCAN_MODE_DCII_7_8  0x10 /* search Digicipher-II 7/8 */
#define BSAT_SCAN_MODE_DCII_5_11 0x20 /* search Digicipher-II 5/11 */
#define BSAT_SCAN_MODE_DCII_3_5  0x40 /* search Digicipher-II 3/5 */
#define BSAT_SCAN_MODE_DCII_4_5  0x80 /* search Digicipher-II 4/5 */
#define BSAT_SCAN_MODE_DCII_ALL  0xFF /* search all Digicipher-II code rates */

/* bit definitions for BSAT_Dvbs2AcqSettings.scanModes */
#define BSAT_SCAN_MODE_DVBS2_QPSK_1_4    0x00000001 /* search DVB-S2 QPSK 1/4 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_1_3    0x00000002 /* search DVB-S2 QPSK 1/3 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_2_5    0x00000004 /* search DVB-S2 QPSK 2/5 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_1_2    0x00000008 /* search DVB-S2 QPSK 1/2 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_3_5    0x00000010 /* search DVB-S2 QPSK 3/5 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_2_3    0x00000020 /* search DVB-S2 QPSK 2/3 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_3_4    0x00000040 /* search DVB-S2 QPSK 3/4 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_4_5    0x00000080 /* search DVB-S2 QPSK 4/5 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_5_6    0x00000100 /* search DVB-S2 QPSK 5/6 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_8_9    0x00000200 /* search DVB-S2 QPSK 8/9 */
#define BSAT_SCAN_MODE_DVBS2_QPSK_9_10   0x00000400 /* search DVB-S2 QPSK 9/10 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_3_5    0x00000800 /* search DVB-S2 8PSK 3/5 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_2_3    0x00001000 /* search DVB-S2 8PSK 2/3 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_3_4    0x00002000 /* search DVB-S2 8PSK 3/4 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_5_6    0x00004000 /* search DVB-S2 8PSK 5/6 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_8_9    0x00008000 /* search DVB-S2 8PSK 8/9 */
#define BSAT_SCAN_MODE_DVBS2_8PSK_9_10   0x00010000 /* search DVB-S2 8PSK 9/10 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_2_3  0x00020000 /* search DVB-S2 16APSK 2/3 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_3_4  0x00040000 /* search DVB-S2 16APSK 3/4 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_4_5  0x00080000 /* search DVB-S2 16APSK 4/5 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_5_6  0x00100000 /* search DVB-S2 16APSK 5/6 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_8_9  0x00200000 /* search DVB-S2 16APSK 8/9 */
#define BSAT_SCAN_MODE_DVBS2_16APSK_9_10 0x00400000 /* search DVB-S2 16APSK 9/10 */
#define BSAT_SCAN_MODE_DVBS2_32APSK_3_4  0x00800000 /* search DVB-S2 32APSK 3/4 */
#define BSAT_SCAN_MODE_DVBS2_32APSK_4_5  0x01000000 /* search DVB-S2 32APSK 4/5 */
#define BSAT_SCAN_MODE_DVBS2_32APSK_5_6  0x02000000 /* search DVB-S2 32APSK 5/6 */
#define BSAT_SCAN_MODE_DVBS2_32APSK_8_9  0x04000000 /* search DVB-S2 32APSK 8/9 */
#define BSAT_SCAN_MODE_DVBS2_32APSK_9_10 0x08000000 /* search DVB-S2 32APSK 9/10 */
#define BSAT_SCAN_MODE_DVBS2_ALL         0x0FFFFFFF /* search all DVB-S2 modcods */

/* bit definitions for BSAT_Dvbsx2AcqSettings.scanModes */
#define BSAT_SCAN_MODE_DVBS2X_QPSK_13_45    0x00000001 /* search DVB-S2X QPSK 13/45 */
#define BSAT_SCAN_MODE_DVBS2X_QPSK_9_20     0x00000002 /* search DVB-S2X QPSK 9/20 */
#define BSAT_SCAN_MODE_DVBS2X_QPSK_11_20    0x00000004 /* search DVB-S2X QPSK 11/20 */
#define BSAT_SCAN_MODE_DVBS2X_8APSK_5_9_L   0x00000008 /* search DVB-S2X 8APSK 5/9-L8 */
#define BSAT_SCAN_MODE_DVBS2X_8APSK_26_45_L 0x00000010 /* search DVB-S2X 8APSK 26/45-L */
#define BSAT_SCAN_MODE_DVBS2X_8PSK_23_36    0x00000020 /* search DVB-S2X 8APSK 23/36 */
#define BSAT_SCAN_MODE_DVBS2X_8PSK_25_36    0x00000040 /* search DVB-S2X 8APSK 25/36 */
#define BSAT_SCAN_MODE_DVBS2X_8PSK_13_18    0x00000080 /* search DVB-S2X 8APSK 13/18 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_1_2_L  0x00000100 /* search DVB-S2X 16APSK 1/2-L */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_8_15_L 0x00000200 /* search DVB-S2X 16APSK 8/15-L */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_5_9_L  0x00000400 /* search DVB-S2X 16APSK 5/9-L */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_26_45  0x00000800 /* search DVB-S2X 16APSK 26/45 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_3_5    0x00001000 /* search DVB-S2X 16APSK 3/5 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_3_5_L  0x00002000 /* search DVB-S2X 16APSK 3/5-L */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_28_45  0x00004000 /* search DVB-S2X 16APSK 28/45 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_23_36  0x00008000 /* search DVB-S2X 16APSK 23/36 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_2_3_L  0x00010000 /* search DVB-S2X 16APSK 2/3-L */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_25_36  0x00020000 /* search DVB-S2X 16APSK 25/36 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_13_18  0x00040000 /* search DVB-S2X 16APSK 13/18 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_7_9    0x00080000 /* search DVB-S2X 16APSK 7/9 */
#define BSAT_SCAN_MODE_DVBS2X_16APSK_77_90  0x00100000 /* search DVB-S2X 16APSK 77/90 */
#define BSAT_SCAN_MODE_DVBS2X_32APSK_2_3_L  0x00200000 /* search DVB-S2X 32APSK 2/3-L */
#define BSAT_SCAN_MODE_DVBS2X_32APSK_32_45  0x00400000 /* search DVB-S2X 32APSK 32/45 */
#define BSAT_SCAN_MODE_DVBS2X_32APSK_11_15  0x00800000 /* search DVB-S2X 32APSK 11/15 */
#define BSAT_SCAN_MODE_DVBS2X_32APSK_7_9    0x01000000 /* search DVB-S2X 32APSK 7/9 */
#define BSAT_SCAN_MODE_DVBS2X_ALL           0x01FFFFFF /* search all DVB-S2 modcods */

/* bit definitions for BSAT_TurboAcqSettings.scanModes */
#define BSAT_SCAN_MODE_TURBO_QPSK_1_2 0x00000001 /* search Turbo QPSK 1/2 */
#define BSAT_SCAN_MODE_TURBO_QPSK_2_3 0x00000002 /* search Turbo QPSK 2/3 */
#define BSAT_SCAN_MODE_TURBO_QPSK_3_4 0x00000004 /* search Turbo QPSK 3/4 */
#define BSAT_SCAN_MODE_TURBO_QPSK_5_6 0x00000008 /* search Turbo QPSK 5/6 */
#define BSAT_SCAN_MODE_TURBO_QPSK_7_8 0x00000010 /* search Turbo QPSK 7/8 */
#define BSAT_SCAN_MODE_TURBO_8PSK_2_3 0x00000020 /* search Turbo 8PSK 2/3 */
#define BSAT_SCAN_MODE_TURBO_8PSK_3_4 0x00000040 /* search Turbo 8PSK 3/4 */
#define BSAT_SCAN_MODE_TURBO_8PSK_4_5 0x00000080 /* search Turbo 8PSK 4/5 */
#define BSAT_SCAN_MODE_TURBO_8PSK_5_6 0x00000100 /* search Turbo 8PSK 5/6 */
#define BSAT_SCAN_MODE_TURBO_8PSK_8_9 0x00000200 /* search Turbo 8PSK 8/9 */
#define BSAT_SCAN_MODE_TURBO_ALL      0x000003FF /* search all Turbo modes */

/* bit definitions for BSAT_Dvbs2AcqSettings.ctl */
#define BSAT_DVBS2_CTL_DISABLE_PSL           0x01 /* power saving loop: 0=enable, 1=disable */
#define BSAT_DVBS2_CTL_DISABLE_FINE_FREQ_EST 0x02 /* fine freq estimation: 0=enable, 1=disable */
#define BSAT_DVBS2_CTL_GOLD_CODE_SEEK        0x04 /* gold code seek: 0=disable, 1=enable */

/* bit definitions for BSAT_TurboAcqSettings.ctl */
#define BSAT_TURBO_CTL_OVERRIDE_TZSY 0x01  /* 0=use default setting for TZSY, 1=override TFEC_TZSY */

/* data types used by BSAT_GetTraceInfo() */
typedef enum BSAT_TraceEvent
{
   BSAT_TraceEvent_eTuneDone = 0,
   BSAT_TraceEvent_eFreqEstDone,
   BSAT_TraceEvent_eRetuneDone,
   BSAT_TraceEvent_eStartHp,
   BSAT_TraceEvent_eRcvrLocked,
   BSAT_TraceEvent_eStartTracking,
   BSAT_TraceEvent_eInitialLock,
   BSAT_TraceEvent_eStableLock,
   BSAT_TraceEvent_eReacquire,
   BSAT_TraceEvent_eMax
} BSAT_TraceEvent;
#define BSAT_TraceEvent_eStartViterbi BSAT_TraceEvent_eStartHp
#define BSAT_TraceEvent_eViterbiLocked BSAT_TraceEvent_eRcvrLocked

typedef struct BSAT_TraceInfo
{
   uint32_t eventTime[BSAT_TraceEvent_eMax]; /* in microseconds relative to start of acquisition */
} BSAT_TraceInfo;

typedef enum BSAT_FastStatusId {
   BSAT_FastStatusId_eNone = 0,
   BSAT_FastStatusId_eSymRateError,
   BSAT_FastStatusId_eCarrierError,
   BSAT_FastStatusId_eMpegCount,
   BSAT_FastStatusId_eMpegErrors,
   BSAT_FastStatusId_eSnr,
   BSAT_FastStatusId_eReacqCount,
   BSAT_FastStatusId_eAgc0,
   BSAT_FastStatusId_eAgc1,
   BSAT_FastStatusId_eTotalBlocks,
   BSAT_FastStatusId_ePreVitErrCount = BSAT_FastStatusId_eTotalBlocks,
   BSAT_FastStatusId_eCorrBlocks,
   BSAT_FastStatusId_eRsCorrCount = BSAT_FastStatusId_eCorrBlocks,
   BSAT_FastStatusId_eBadBlocks,
   BSAT_FastStatusId_eRsUncorrCount = BSAT_FastStatusId_eBadBlocks,
   BSAT_FastStatusId_max
} BSAT_FastStatusId;

typedef struct BSAT_FastStatusItem {
   BSAT_FastStatusId id;
   bool              bValid;
   union
   {
      uint32_t uint32;
      int32_t  int32;
   } value;
} BSAT_FastStatusItem;

typedef struct BSAT_FastChannelStatus {
   bool                bDemodLocked; /* true={demod/FEC are locked} */
   BSAT_FastStatusItem item[7];
   uint8_t             nItems;
} BSAT_FastChannelStatus;


struct BSAT_Settings;
struct BSAT_ChannelSettings;


/******************************************************************************
Summary:
   Structure representing API function table.
Desciption:
   This structure contains pointers to all public BSAT functions.
See Also:
   BSAT_Settings
******************************************************************************/
typedef struct BSAT_ApiFunctTable
{
   BERR_Code (*Open)(BSAT_Handle*, BCHP_Handle, void*, BINT_Handle, const struct BSAT_Settings*);
   BERR_Code (*Close)(BSAT_Handle);
   BERR_Code (*GetTotalChannels)(BSAT_Handle, uint32_t *);
   BERR_Code (*GetChannelDefaultSettings)(BSAT_Handle, uint32_t, struct BSAT_ChannelSettings*);
   BERR_Code (*OpenChannel)(BSAT_Handle, BSAT_ChannelHandle*, uint32_t, const struct BSAT_ChannelSettings*);
   BERR_Code (*CloseChannel)(BSAT_ChannelHandle);
   BERR_Code (*GetDevice)(BSAT_ChannelHandle, BSAT_Handle *);
   BERR_Code (*GetVersionInfo)(BSAT_Handle, BFEC_VersionInfo*);
   BERR_Code (*Reset)(BSAT_Handle);
   BERR_Code (*PowerDownChannel)(BSAT_ChannelHandle);
   BERR_Code (*PowerUpChannel)(BSAT_ChannelHandle);
   BERR_Code (*IsChannelOn)(BSAT_ChannelHandle, bool*);
   BERR_Code (*Acquire)(BSAT_ChannelHandle, BSAT_AcqSettings*);
   BERR_Code (*GetLockStatus)(BSAT_ChannelHandle, bool*, bool*);
   BERR_Code (*GetChannelStatus)(BSAT_ChannelHandle, BSAT_ChannelStatus*);
   BERR_Code (*ResetChannelStatus)(BSAT_ChannelHandle);
   BERR_Code (*GetSoftDecisions)(BSAT_ChannelHandle, uint32_t, int16_t*, int16_t*);
   BERR_Code (*ResetChannel)(BSAT_ChannelHandle, bool);
   BERR_Code (*SetBertSettings)(BSAT_ChannelHandle, BSAT_BertSettings*);
   BERR_Code (*GetBertSettings)(BSAT_ChannelHandle, BSAT_BertSettings*);
   BERR_Code (*GetBertStatus)(BSAT_ChannelHandle, BSAT_BertStatus*);
   BERR_Code (*SetSearchRange)(BSAT_ChannelHandle, uint32_t);
   BERR_Code (*GetSearchRange)(BSAT_ChannelHandle, uint32_t*);
   BERR_Code (*SetAmcScramblingSeq)(BSAT_ChannelHandle, BSAT_AmcScramblingSeq*);
   BERR_Code (*SetNetworkSpec)(BSAT_Handle, BSAT_NetworkSpec);
   BERR_Code (*GetNetworkSpec)(BSAT_Handle, BSAT_NetworkSpec*);
   BERR_Code (*SetOutputTransportSettings)(BSAT_ChannelHandle, BSAT_OutputTransportSettings*);
   BERR_Code (*GetOutputTransportSettings)(BSAT_ChannelHandle, BSAT_OutputTransportSettings*);
   BERR_Code (*GetInitDoneEventHandle)(BSAT_Handle, BKNI_EventHandle*);
   BERR_Code (*GetLockStateChangeEventHandle)(BSAT_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetAcqDoneEventHandle)(BSAT_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetSignalNotificationEventHandle)(BSAT_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*GetReadyEventHandle)(BSAT_ChannelHandle, BKNI_EventHandle*);
   BERR_Code (*SetAcqDoneEventSettings)(BSAT_Handle, uint8_t);
   BERR_Code (*SetSignalNotificationSettings)(BSAT_ChannelHandle, BSAT_SignalNotificationSettings*);
   BERR_Code (*GetSignalNotificationSettings)(BSAT_ChannelHandle, BSAT_SignalNotificationSettings*);
   BERR_Code (*StartToneDetect)(BSAT_ChannelHandle, uint32_t, uint32_t);
   BERR_Code (*GetToneDetectStatus)(BSAT_ChannelHandle, BSAT_ToneDetectStatus*);
   BERR_Code (*StartSymbolRateScan)(BSAT_ChannelHandle, uint32_t, uint32_t, uint32_t, uint32_t);
   BERR_Code (*GetSymbolRateScanStatus)(BSAT_ChannelHandle, BSAT_SymbolRateScanStatus*);
   BERR_Code (*StartSignalDetect)(BSAT_ChannelHandle, uint32_t, uint32_t, uint32_t);
   BERR_Code (*GetSignalDetectStatus)(BSAT_ChannelHandle, BSAT_SignalDetectStatus*);
   BERR_Code (*SetConfig)(BSAT_Handle, uint32_t, uint32_t);
   BERR_Code (*GetConfig)(BSAT_Handle, uint32_t, uint32_t*);
   BERR_Code (*SetChannelConfig)(BSAT_ChannelHandle, uint32_t, uint32_t);
   BERR_Code (*GetChannelConfig)(BSAT_ChannelHandle, uint32_t, uint32_t*);
   BERR_Code (*GetLegacyQpskAcqSettings)(BSAT_ChannelHandle, BSAT_LegacyQpskAcqSettings*);
   BERR_Code (*SetLegacyQpskAcqSettings)(BSAT_ChannelHandle, BSAT_LegacyQpskAcqSettings*);
   BERR_Code (*GetDvbs2AcqSettings)(BSAT_ChannelHandle, BSAT_Dvbs2AcqSettings*);
   BERR_Code (*SetDvbs2AcqSettings)(BSAT_ChannelHandle, BSAT_Dvbs2AcqSettings*);
   BERR_Code (*GetTurboAcqSettings)(BSAT_ChannelHandle, BSAT_TurboAcqSettings*);
   BERR_Code (*SetTurboAcqSettings)(BSAT_ChannelHandle, BSAT_TurboAcqSettings*);
   BERR_Code (*GetExtAcqSettings)(BSAT_ChannelHandle, BSAT_ExtAcqSettings*);
   BERR_Code (*SetExtAcqSettings)(BSAT_ChannelHandle, BSAT_ExtAcqSettings*);
   BERR_Code (*SetACIBandwidth)(BSAT_ChannelHandle, uint32_t);
   BERR_Code (*SetNotchSettings)(BSAT_Handle, uint32_t, BSAT_NotchSettings*);
   BERR_Code (*GetNotchSettings)(BSAT_Handle, uint32_t*, BSAT_NotchSettings*);
   BERR_Code (*GetTraceInfo)(BSAT_ChannelHandle, BSAT_TraceInfo*);
   BERR_Code (*SetCwc)(BSAT_Handle, uint32_t, uint32_t*);
   BERR_Code (*SetExternalBertSettings)(BSAT_Handle, BSAT_ExternalBertSettings*);
   BERR_Code (*GetExternalBertSettings)(BSAT_Handle, BSAT_ExternalBertSettings*);
   BERR_Code (*GetDvbs2xAcqSettings)(BSAT_ChannelHandle, BSAT_Dvbs2xAcqSettings*);
   BERR_Code (*SetDvbs2xAcqSettings)(BSAT_ChannelHandle, BSAT_Dvbs2xAcqSettings*);
   BERR_Code (*StartPsdScan)(BSAT_ChannelHandle, uint32_t, uint32_t);
   BERR_Code (*GetPsdScanStatus)(BSAT_ChannelHandle, BSAT_PsdScanStatus*);
   BERR_Code (*SetAcmSettings)(BSAT_ChannelHandle, BSAT_AcmSettings*);
   BERR_Code (*GetAcmSettings)(BSAT_ChannelHandle, BSAT_AcmSettings*);
   BERR_Code (*GetStreamList)(BSAT_ChannelHandle, int, int *, uint8_t *);
   BERR_Code (*GetStreamStatus)(BSAT_ChannelHandle, uint8_t, BSAT_StreamStatus*);
   BERR_Code (*GetFastChannelStatus)(BSAT_ChannelHandle, BSAT_FastStatusId *, uint8_t, BSAT_FastChannelStatus *);
} BSAT_ApiFunctTable;


/******************************************************************************
Summary:
   Structure containing BSAT device settings.
Desciption:
   This structure contains BSAT settings that will be passed into BSAT_Open().
See Also:
   BSAT_Open()
******************************************************************************/
typedef struct BSAT_Settings
{
   BSAT_ApiFunctTable api; /* API function table for per-chip implementation */
} BSAT_Settings;


/******************************************************************************
Summary:
   Structure containing BSAT device channel settings.
Description:
   This structure contains BSAT channel settings that will be passed into
   BSAT_OpenChannel().
See Also:
   BSAT_OpenChannel(), BSAT_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BSAT_ChannelSettings
{
   uint32_t reserved; /* TBD */
} BSAT_ChannelSettings;


/******************************************************************************
Summary:
   Initialize the BSAT PI.
Description:
   This function returns a BSAT device handle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BSAT_Handle is valid
******************************************************************************/
BERR_Code BSAT_Open(
   BSAT_Handle *h,                /* [out] BSAT handle */
   BCHP_Handle hChip,             /* [in] chip handle */
   void *pReg,                    /* [in] BREG or BHAB handle */
   BINT_Handle hInterrupt,        /* [in] BINT handle */
   const BSAT_Settings *pSettings /* [in] default settings */
);


/******************************************************************************
Summary:
   Close the BSAT PI.
Description:
   This function releases all resources allocated by the BSAT device.  The
   BSAT_Handle may not be used after this call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_Close(
   BSAT_Handle h   /* [in] BSAT handle */
);


/******************************************************************************
Summary:
   Get the total number of logical sub-devices for the BSAT device.
Description:
   This function returns the number of satellite downstream channels supported
   by this chip.
Returns:
   BERR_Code : BERR_SUCCESS = totalChannels is valid
******************************************************************************/
BERR_Code BSAT_GetTotalChannels(
   BSAT_Handle h,             /* [in] BSAT handle */
   uint32_t    *totalChannels /* [out] number of satellite downstream channels supported by this chip */
);


/******************************************************************************
Summary:
   Get default channel settings.
Description:
   This function gets the default settings for the BSAT channel device.  The
   returned settings will normally be modified (if necessary) and passed into
   BSAT_OpenChannel().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetChannelDefaultSettings(
   BSAT_Handle h,                    /* [in] BSAT handle */
   uint32_t    chan,                 /* [in] channel index (0 based) */
   BSAT_ChannelSettings *pSettings   /* [out] default channel settings */
);


/******************************************************************************
Summary:
   Initialize a BSAT channel device.
Description:
   This function initializes a BSAT channel device and returns a
   BSAT_ChannelHandle.
Returns:
   BERR_Code : BERR_SUCCESS = the returned BSAT_ChannelHandle is valid
******************************************************************************/
BERR_Code BSAT_OpenChannel(
   BSAT_Handle                h,               /* [in] BSAT handle */
   BSAT_ChannelHandle         *pChannelHandle, /* [out] BSAT channel handle */
   uint32_t                   chan,      /* [in] channel index (0 based) */
   const BSAT_ChannelSettings *pSettings /* [in] channel settings */
);


/******************************************************************************
Summary:
   Close the BSAT channel device.
Description:
   This function frees the channel handle and any memory allocated by
   BSAT_OpenChannel().  The channel handle should not be used after this call.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_CloseChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
);


/******************************************************************************
Summary:
   Returns the BSAT_Handle associated with the BSAT_ChannelHandle.
Description:
   This function returns the BSAT device handle that is associated with the
   given channel.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetDevice(
   BSAT_ChannelHandle h,     /* [in] BSAT channel handle */
   BSAT_Handle        *pDev  /* [out] BSAT handle */
);


/******************************************************************************
Summary:
   Returns SAT PI version information.
Description:
   This function returns SAT PI version.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetVersionInfo(
   BSAT_Handle h,             /* [in] BSAT handle */
   BFEC_VersionInfo *pVersion /* [out] version information */
);


/******************************************************************************
Summary:
   Initializes the BSAT PI and frontend blocks.
Description:
   This function resets all functional blocks.  This function should be called
   after all the BSAT handles have been openned.  The Initialization Done
   event will be set when initialization has completed.  All channels are
   powered down in their initial state, so use BSAT_PowerUpChannel() to power
   up the channels before acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_Reset(
   BSAT_Handle h    /* [in] BSAT handle */
);


/******************************************************************************
Summary:
   Power down the satellite downstream channel.
Description:
   This function turns off the cores associated with the specified channel.
   The Ready event will be set after the channel has completely powered down.
   BERR_Code
******************************************************************************/
BERR_Code BSAT_PowerDownChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
);


/******************************************************************************
Summary:
   Power up the satellite downstream channel.
Description:
   This function is used to power up the specified channel.  The Ready event will
   be set after the channel has completely powered up.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_PowerUpChannel(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
);


/******************************************************************************
Summary:
   Return power up state of the channel.
Description:
   This function is used determine if the channel is powered up.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_IsChannelOn(
   BSAT_ChannelHandle h,   /* [in] BSAT channel handle */
   bool               *pOn /* [out] true={channel is currently powered up} */
);


/******************************************************************************
Summary:
   Initiate channel acquisition.
Description:
   This function initiates channel acquisition.  This function will return
   immediately while channel acquisition continues in the background.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_Acquire(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_AcqSettings   *pParams   /* [in] acquisition parameters */
);


/******************************************************************************
Summary:
   Returns channel lock status.
Description:
   This function is a quick function (relative to BSAT_GetChannelStatus) that
   is used to get a full lock status.  Full lock means the demod, FEC, and
   BERT (if enabled) are all locked.  AcqDone status is also returned to
   qualify a not-locked condition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetLockStatus(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   bool               *pbLocked, /* [out] true if demod, FEC, and BERT (if enabled) are all locked */
   bool               *pbAcqDone /* [out] true if Acquisition Done has been asserted */
);


/******************************************************************************
Summary:
   Returns channel status.
Description:
   This function returns satellite downstream status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetChannelStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_ChannelStatus *pStatus /* [out] channel status */
);


/******************************************************************************
Summary:
   Reset all counters in returned status.
Description:
   This function resets the MPEG frame count, MPEG frame error count,
   reacquisition count, and all block counters returned by
   BSAT_GetChannelStatus().  Additionally, the BERT counters returned by
   BSAT_GetBertStatus() are reset.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_ResetChannelStatus(
   BSAT_ChannelHandle h  /* [in] BSAT channel handle */
);


/******************************************************************************
Summary:
   Get I/Q soft decisions.
Description:
   This function gets data for a constellation display.  This function returns
   n pairs of I/Q values.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSoftDecisions(
   BSAT_ChannelHandle h,   /* [in] BSAT channel handle */
   uint32_t           n,   /* [in] number of I/Q pairs to get */
   int16_t            *pI, /* [out] n I-values */
   int16_t            *pQ  /* [out] n Q-values */
);


/******************************************************************************
Summary:
   Aborts the current acquisition.
Description:
   This function turns off all interrupts in the firmware to halt the
   acquisition state machine.  The Acquisition Done event and the Lock State
   Change event will be disabled until the next acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_ResetChannel(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   bool               bDisableDemod /* [in] true={disable demod/oif after stopping acquisition}, false={software acquisition/tracking is stopped, but demod remains in current state} */
);


/******************************************************************************
Summary:
   Configure the BERT.
Description:
   This function is used to configure the internal BERT.  The new BERT settings
   will take affect on the next call to BSAT_Acquire() with BERT enabled.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetBertSettings(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_BertSettings  *pSettings /* [in] BERT settings */
);


/******************************************************************************
Summary:
   Get the BERT configuration.
Description:
   This function is used to get the configuration of the BERT when it is
   enabled.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetBertSettings(
   BSAT_ChannelHandle h,         /* [in] BSAT channel handle */
   BSAT_BertSettings  *pSettings /* [out] BERT settings */
);


/******************************************************************************
Summary:
   Get BERT status information.
Description:
   This function returns BERT status information.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetBertStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_BertStatus    *pStatus /* [out] BERT status */
);


/******************************************************************************
Summary:
   Set carrier search range.
Description:
   This function sets the acquisition search range for future acquisitions.
   The demod will scan for the carrier +/-searchRange relative to the tuning
   frequency specified in BSAT_Acquire().  The default search range is 5000000
   (i.e. +/-5MHz).
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetSearchRange(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   uint32_t           searchRange /* [in] search range in Hz */
);


/******************************************************************************
Summary:
   Get the carrier search range.
Description:
   This function returns the acquisition search range.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSearchRange(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   uint32_t           *pSearchRange /* [out] search range in Hz */
);


/******************************************************************************
Summary:
   Specifies the AMC scrambling sequence in DVB-S2.
Description:
   This function specifies the AMC scrambling sequence used in DVB-S2
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetAmcScramblingSeq(
   BSAT_ChannelHandle    h,     /* [in] BSAT channel handle */
   BSAT_AmcScramblingSeq *pSeq  /* [in] scrambling sequence */
);


/******************************************************************************
Summary:
   Set the network spec.
Description:
   Set the network spec.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetNetworkSpec(
   BSAT_Handle      h,          /* [in] BSAT handle */
   BSAT_NetworkSpec networkSpec /* [in] network spec */
);


/******************************************************************************
Summary:
   Get the network spec.
Description:
   Get the network spec.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetNetworkSpec(
   BSAT_Handle      h,            /* [in] BSAT handle */
   BSAT_NetworkSpec *pNetworkSpec /* [out] network spec */
);


/******************************************************************************
Summary:
   Configure the MPEG transport output settings.
Description:
   Configure the MPEG transport output settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetOutputTransportSettings(
   BSAT_ChannelHandle h,                   /* [in] BSAT channel handle */
   BSAT_OutputTransportSettings *pSettings /* [in] transport settings */
);


/******************************************************************************
Summary:
   Get the MPEG transport output settings.
Description:
   Get the MPEG transport output settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetOutputTransportSettings(
   BSAT_ChannelHandle h,                   /* [in] BSAT channel handle */
   BSAT_OutputTransportSettings *pSettings /* [in] transport settings */
);


/******************************************************************************
Summary:
   Returns the Initialization Done event handle.
Description:
   The Initialization Done event is set when the BSAT PI has finished the
   initialization process started by BSAT_Reset().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetInitDoneEventHandle(
   BSAT_Handle      h,      /* [in] BSAT handle */
   BKNI_EventHandle *hEvent /* [out] event handle */
);


/******************************************************************************
Summary:
   Returns the Lock State Change event handle.
Description:
   The Lock State Change event is set whenever the demod/FEC lock status
   transitions from locked to not-locked, or not-locked to locked.  The
   following sequence can be used to determine lock after performing an
   acquisition:
      BSAT_GetAcqDoneEventHandle(hChn, &hAcqDoneEvent);
      BSAT_ResetChannel(hChn, true); <-- stop current acquisition
      BKNI_WaitForEvent(hAcqDoneEvent, 0); <-- clear the event before acquiring
      BSAT_Acquire(hChn, ...);
      BKNI_WaitForEvent(hAcqDoneEvent, 1000);
      BSAT_GetLockStatus(hChn, &bLocked, &bAcqDone);
      if (bLocked && bAcqDone)
         <declare locked>
      else
         <declare not locked>
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetLockStateChangeEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] lock event handle */
);


/******************************************************************************
Summary:
   Returns the Acquisition Done event handle.
Description:
   The Acquisition Done event is set when any of the following conditions are
   true:
   1. The demod has not locked after n reacquisitions.  By default n=3, but n
      is configurable using BSAT_SetAcqDoneEventSettings().
   2. The demod has locked in the i^th acquisition where i<=n.  In this case,
      the Acquisition Done event will be set at the same time as the Lock State
      Change event.
   3. The signal detect operation initiated by BSAT_StartSignalDetect() has
      finished.
   4. The peak power scan initiated by BSAT_StartToneDetect() has finished.
   5. The symbol rate scan initiated by BSAT_StartSymbolRateScan() has finished.
   6. The PSD scan initiated by BSAT_StartPsdScan() has finished.
   After the Acquisition Done event is set, it will not be set again until
   after the next call to BSAT_Acquire().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetAcqDoneEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
);


/******************************************************************************
Summary:
   Returns the Status notification event handle.
Description:
   The Status notification event is set when any of the following conditions
   are true:
   1. Rain Fade: SNR drop exceeds some threshold over a time window.
   2. Frequency Drift: Carrier drift exceeds some threshold.
   3. Threshold1: SNR drops below threshold1.
   4. Threshold2: SNR drops below threshold2.
   See BSAT_SetSignalNotificationSettings() to configure the status
   notifications.  The real time status event indicator is given in
   BSAT_ChannelStatus.statusEvents.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSignalNotificationEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
);


/******************************************************************************
Summary:
   Returns the Ready event handle.
Description:
   The Ready event is used to signal when one of the following operations has
   completed.
   1. power down initiated by BSAT_PowerDownChannel()
   2. power up initiated by BSAT_PowerUpChannel()
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetReadyEventHandle(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   BKNI_EventHandle   *hEvent /* [out] event handle */
);


/******************************************************************************
Summary:
   Initiate the peak scan operation for tone detection.
Description:
   This function stops the current acquisition, tunes the tuner, and starts the
   DFT engine for the tone detect operation.  The Acquisition Done event
   signals the completion of the tone detect operation.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_StartToneDetect(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint32_t           freq,     /* [in] center frequency in Hz */
   uint32_t           adcSelect /* [in] ADC selection (0-based index) */
);


/******************************************************************************
Summary:
   Get the status of the most recent tone detect operation.
Description:
   This function returns the status from the tone detect operation initiated by
   the most recent call to BSAT_StartToneDetect().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetToneDetectStatus(
   BSAT_ChannelHandle    h,       /* [in] BSAT channel handle */
   BSAT_ToneDetectStatus *pStatus /* [out] tone detect status */
);


/******************************************************************************
Summary:
   Initiate the symbol rate scan operation.
Description:
   This function stops the current acquisition, tunes the tuner, and starts the
   DFT engine for symbol rate scan operation.  The Acquisition Done event is
   set when the symbol rate scan has completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_StartSymbolRateScan(
   BSAT_ChannelHandle h,      /* [in] BSAT channel handle */
   uint32_t           freq,   /* [in] center frequency in Hz */
   uint32_t           minSymbolRate, /* [in] minimum symbol rate in sym/sec */
   uint32_t           maxSymbolRate, /* [in] maximum symbol rate in sym/sec */
   uint32_t           adcSelect      /* [in] ADC selection (0-based index) */
);


/******************************************************************************
Summary:
   Get the status of the most recent symbol rate scan operation.
Description:
   This function returns the status from the symbol rate scan initiated by the
   most recent call to BSAT_StartSymbolRateScan().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSymbolRateScanStatus(
   BSAT_ChannelHandle h,              /* [in] BSAT channel handle */
   BSAT_SymbolRateScanStatus *pStatus /* [out] status from most recent call to BSAT_StartSymbolRateScan() */
);


/******************************************************************************
Summary:
   Initiate the PSD scan.
Description:
   This function stops the current acquisition, tunes the tuner, and starts the
   DFT engine to get the PSD.  The Acquisition Done event signals the completion
   of the PSD scan operation.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_StartPsdScan(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint32_t           freq,     /* [in] center frequency in Hz */
   uint32_t           adcSelect /* [in] ADC selection (0-based index) */
);


/******************************************************************************
Summary:
   Get the status of the most recent PSD scan operation.
Description:
   This function returns the status from the PSD operation initiated by
   the most recent call to BSAT_StartPsdScan().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetPsdScanStatus(
   BSAT_ChannelHandle h,       /* [in] BSAT channel handle */
   BSAT_PsdScanStatus *pStatus /* [out] PSD scan status */
);


/******************************************************************************
Summary:
   Change the value of a global configuration parameter.
Description:
   This function is used to change the value of a chip-specific global
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetConfig(
   BSAT_Handle h,    /* [in] BSAT device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    val   /* [in] configuration parameter value */
);


/******************************************************************************
Summary:
   Get the current value of a global configuration parameter.
Description:
   This function is used to read the value of a chip-specific global
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetConfig(
   BSAT_Handle h,    /* [in] BSAT device handle */
   uint32_t    addr, /* [in] configuration parameter address */
   uint32_t    *pVal /* [out] value of the configuration parameter */
);


/******************************************************************************
Summary:
   Change the value of a channel configuration parameter.
Description:
   This function is used to change the value of a chip-specific channel
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetChannelConfig(
   BSAT_ChannelHandle h,    /* [in] BSAT channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           val   /* [in] configuration parameter value */
);


/******************************************************************************
Summary:
   Get the current value of a channel configuration parameter.
Description:
   This function is used to read the value of a chip-specific channel
   configuration parameter.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetChannelConfig(
   BSAT_ChannelHandle h,    /* [in] BSAT channel handle */
   uint32_t           addr, /* [in] configuration parameter address */
   uint32_t           *pVal /* [out] value of the configuration parameter */
);


/******************************************************************************
Summary:
   Configure the Acquisition Done event.
Description:
   This function is used to set the conditions for triggering the Acquisition
   Done event.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetAcqDoneEventSettings(
   BSAT_Handle h,      /* [in] BSAT channel handle */
   uint8_t     reacqs  /* [in] number of failed reacquisitions before setting Acq Done event */
);


/******************************************************************************
Summary:
   Configure status notification.
Description:
   This function is used to configure the status notification events.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetSignalNotificationSettings(
   BSAT_ChannelHandle              h,         /* [in] BSAT channel handle */
   BSAT_SignalNotificationSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Get status notification configuration.
Description:
   This function returns the current status notification configuration.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSignalNotificationSettings(
   BSAT_ChannelHandle              h,         /* [in] BSAT channel handle */
   BSAT_SignalNotificationSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set the ACI filter bandwidth.
Description:
   This function sets the ACI filter.  If the bw parameter is set to 0, then
   it will be set automatically by the firmware as a function of symbol rate
   and nyquist rolloff.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetACIBandwidth(
   BSAT_ChannelHandle h,  /* [in] BSAT channel handle */
   uint32_t           bw  /* [in] filter bandwidth in Hz */
);


/******************************************************************************
Summary:
   Initiates the signal detection operation.
Description:
   This function starts a channel acquisition and attempts to lock the timing
   loop to determine if a signal is present at the given frequency.  The
   AcqDone event signals when the signal detect operation has completed.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_StartSignalDetect(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   uint32_t           symbolRate, /* [in] symbol rate in sym/sec */
   uint32_t           freq,       /* [in] center frequency in Hz */
   uint32_t           adcSelect   /* [in] ADC selection (0-based index) */
);


/******************************************************************************
Summary:
   Get the status of the most recent signal detect operation.
Description:
   The Signal detect operation is a quick acquisition to lock only the timing
   loop in order to verify a valid signal is present at the specified frequency.
   Signal detect mode is initiated by BSAT_StartSignalDetect().
   BSAT_GetSignalDetectStatus() returns the status of the most recent signal
   detect operation.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetSignalDetectStatus(
   BSAT_ChannelHandle      h,       /* [in] BSAT channel handle */
   BSAT_SignalDetectStatus *pStatus /* [out] returned status */
);


/******************************************************************************
Summary:
   Get acquisition settings for DVB-S, DTV, and DCII modes.
Description:
   Get acquisition settings for DVB-S, DTV, and DCII modes.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetLegacyQpskAcqSettings(
   BSAT_ChannelHandle         h,         /* [in] BSAT channel handle */
   BSAT_LegacyQpskAcqSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set acquisition settings for DVB-S, DTV, and DCII modes.
Description:
   This function is used to change acquisition settings for Legecy QPSK mode.
   The updated settings will take affect starting from the next legacy QPSK
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetLegacyQpskAcqSettings(
   BSAT_ChannelHandle         h,         /* [in] BSAT channel handle */
   BSAT_LegacyQpskAcqSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Get acquisition setttings for DVB-S2 mode.
Description:
   Get acquisition setttings for DVB-S2 mode.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetDvbs2AcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2AcqSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set acquisition settings for DVB-S2 mode.
Description:
   This function is used to change various acquisition settings for DVB-S2 mode.
   The updated settings will take affect starting from the next DVB-S2
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetDvbs2AcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2AcqSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Get acquisition setttings for DVB-S2X mode.
Description:
   Get acquisition setttings for DVB-S2X mode.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetDvbs2xAcqSettings(
   BSAT_ChannelHandle     h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2xAcqSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set acquisition settings for DVB-S2X mode.
Description:
   This function is used to change various acquisition settings for DVB-S2X mode.
   The updated settings will take affect starting from the next DVB-S2X
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetDvbs2xAcqSettings(
   BSAT_ChannelHandle     h,         /* [in] BSAT channel handle */
   BSAT_Dvbs2xAcqSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Get acquisition setttings for Turbo mode.
Description:
   Get acquisition setttings for Turbo mode.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetTurboAcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_TurboAcqSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set acquisition settings for Turbo mode.
Description:
   This function is used to change various acquisition settings for Turbo mode.
   The updated settings will take affect starting from the next Turbo
   acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetTurboAcqSettings(
   BSAT_ChannelHandle    h,         /* [in] BSAT channel handle */
   BSAT_TurboAcqSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Get extended acquisition setttings.
Description:
   Get extended acquisition setttings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetExtAcqSettings(
   BSAT_ChannelHandle    h,       /* [in] BSAT channel handle */
   BSAT_ExtAcqSettings *pSettings /* [out] settings */
);


/******************************************************************************
Summary:
   Set extended acquisition settings.
Description:
   This function is used to set miscellaneous acquisition settings.  The
   updated settings will take affect starting from the next call to
   BSAT_Acquire().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetExtAcqSettings(
   BSAT_ChannelHandle    h,       /* [in] BSAT channel handle */
   BSAT_ExtAcqSettings *pSettings /* [in] settings */
);


/******************************************************************************
Summary:
   Set CWC configuration.
Description:
   This function is used to set up the CW cancellers.  Up to 6 frequencies may
   be provided.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetCwc(
   BSAT_Handle        h,         /* [in] BSAT handle */
   uint32_t           n,         /* [in] number of CWCs (max=6), or 0 to disable */
   uint32_t *pFreqBuf            /* [in] list of frequencies in Hz */
);


/******************************************************************************
Summary:
   Set notch filter settings.
Description:
   This function is used to set up the DCO notch filter at n specified
   frequencies.  If n=0, then all previous notch settings are discarded.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetNotchSettings(
   BSAT_Handle        h,         /* [in] BSAT handle */
   uint32_t           n,         /* number of notches */
   BSAT_NotchSettings *pSettings /* [in] notch filter settings */
);


/******************************************************************************
Summary:
   Get current notch filter settings.
Description:
   This function returns the notch filter settings currently stored.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetNotchSettings(
   BSAT_Handle        h,         /* [in] BSAT handle */
   uint32_t           *pNum,     /* number of notches */
   BSAT_NotchSettings *pSettings /* [in] notch filter settings */
);


/******************************************************************************
Summary:
   Get acquisition trace information.
Description:
   This function returns trace information from the most recent acquisition.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetTraceInfo(
   BSAT_ChannelHandle h, /* [in] BSAT channel handle */
   BSAT_TraceInfo *pInfo /* [out] trace info */
);


/******************************************************************************
Summary:
   Configure serial BERT output.
Description:
   This function enables and configures serial BERT output to an external BERT.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetExternalBertSettings(
   BSAT_Handle               h,         /* [in] BSAT device handle */
   BSAT_ExternalBertSettings *pSettings /* [in] external BERT configuration  */
);


/******************************************************************************
Summary:
   Get external BERT output configuration settings.
Description:
   Get external BERT output configuration settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetExternalBertSettings(
   BSAT_Handle               h,         /* [in] BSAT device handle */
   BSAT_ExternalBertSettings *pSettings /* [out] external BERT configuration  */
);


/******************************************************************************
Summary:
   Configure ACM settings.
Description:
   This function sets various ACM parameters.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_SetAcmSettings(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   BSAT_AcmSettings   *pSettings  /* [in] ACM settings  */
);


/******************************************************************************
Summary:
   Get ACM settings.
Description:
   Get ACM settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetAcmSettings(
   BSAT_ChannelHandle h,          /* [in] BSAT channel handle */
   BSAT_AcmSettings   *pSettings  /* [out] ACM settings  */
);


/******************************************************************************
Summary:
   Get list of stream IDs.
Description:
   This function returns an array of 8-bit stream IDs for every stream detected.
   This feature is available only on chips with SDS core 7.4 or later where
   ACM and multistream capability is supported.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetStreamList(
   BSAT_ChannelHandle h,  /* [in] BSAT channel handle */
   int bufsize,           /* [in] size of the allocated buffer */
   int *pNumStreams,      /* [out] number of stream IDs returned in pStreamIdList */
   uint8_t *pStreamIdList /* [out] array of 8-bit stream IDs */
);


/******************************************************************************
Summary:
   Get status of a specific stream in a multistream channel.
Description:
   This function returns status information about a given stream.  This feature
   is available only on chips with SDS core 7.4 or later where ACM and
   multistream capability is supported.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetStreamStatus(
   BSAT_ChannelHandle h,        /* [in] BSAT channel handle */
   uint8_t            streamId, /* [in] stream ID */
   BSAT_StreamStatus  *pStatus  /* [out] stream status information */
);


/******************************************************************************
Summary:
   Used to quickly get a user-defined subset of channel status.
Description:
   This function is useful only on PHY chips where the host controls the
   frontend using a relatively slow interface such as I2C.  The purpose of this
   function is to get channel status as quickly as possible using I2C, as
   compared to the time taken with BSAT_GetChannelStatus().  This function does
   not use the HAB interface.  Instead, it directs the LEAP firmware to
   continuously dump a user-specified set of status (up to 7 status
   items) to spare registers on the PHY chip.  The LEAP updates the status every
   millisecond.  Once the stream of status has started, the host can directly
   read registers to pick up the status.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BSAT_GetFastChannelStatus(
   BSAT_ChannelHandle h,            /* [in] BSAT channel handle */
   BSAT_FastStatusId *pStatusIds,   /* [in] ID of the status items */
   uint8_t numItems,                /* [in] number of status items (0 to 7) */
   BSAT_FastChannelStatus *pStatus  /* [out] returned status */
);

#ifdef __cplusplus
}
#endif

#endif /* _BSAT_H_ */
