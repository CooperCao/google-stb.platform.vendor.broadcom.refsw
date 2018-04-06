/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*= Module Overview *********************************************************
Overview

To follow later

Sample Code

To follow later
***************************************************************************/

#ifndef BXPT_MPOD_H__
#define BXPT_MPOD_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Enumeration of all the output clock rates.
****************************************************************************/
#if BXPT_HAS_MPOD_OUTPUT_CLOCK_RATE
typedef enum BXPT_Mpod_OutputClockRate
{
    BXPT_Mpod_OutputClockRate_e81,              /* 81 MHz */
    BXPT_Mpod_OutputClockRate_e54,              /* 54 MHz */
    BXPT_Mpod_OutputClockRate_e40_50,           /* 40.5 MHz */
    BXPT_Mpod_OutputClockRate_e20_25,           /* 20.25 MHz */
    BXPT_Mpod_OutputClockRate_e27,              /* 27 MHz */
    BXPT_Mpod_OutputClockRate_ePll,              /* PLL clock */
    BXPT_Mpod_OutputClockRate_e108,              /* 108 MHz clock */

    /* The number of input bands varies from chip to chip. */
    BXPT_Mpod_OutputClockRate_eIb0 = 0x8,        /* Input band 0 clock */
    BXPT_Mpod_OutputClockRate_eIb1,              /* Input band 1 clock */
    BXPT_Mpod_OutputClockRate_eIb2,              /* Input band 2 clock */
    BXPT_Mpod_OutputClockRate_eIb3,              /* Input band 3 clock */
    BXPT_Mpod_OutputClockRate_eIb4,              /* Input band 4 clock */
    BXPT_Mpod_OutputClockRate_eIb5,              /* Input band 5 clock */
    BXPT_Mpod_OutputClockRate_eIb6,              /* Input band 6 clock */
    BXPT_Mpod_OutputClockRate_eIb7,              /* Input band 7 clock */
    BXPT_Mpod_OutputClockRate_eIb8,              /* Input band 8 clock */
    BXPT_Mpod_OutputClockRate_eIb9,              /* Input band 9 clock */
    BXPT_Mpod_OutputClockRate_eIb10,              /* Input band 10 clock */
    BXPT_Mpod_OutputClockRate_eIb11,              /* Input band 11 clock */
    BXPT_Mpod_OutputClockRate_eIb12,              /* Input band 12 clock */

    BXPT_Mpod_OutputClockRate_eMax              /* End of the enum list */
}
BXPT_Mpod_OutputClockRate;
#else
typedef enum BXPT_Mpod_OutputClock
{
    BXPT_Mpod_OutputClock_e81 = 0,      /* 81 MHz */
    BXPT_Mpod_OutputClock_e54 = 1,      /* 54 MHz */
    BXPT_Mpod_OutputClock_e40_5 = 2,      /* 40.5 MHz */
    BXPT_Mpod_OutputClock_e20_25 = 3,      /* 20.25 MHz */
    BXPT_Mpod_OutputClock_e27 = 4,      /* 27 MHz */
    BXPT_Mpod_OutputClock_ePLL = 5,      /* PLL MHz */
    BXPT_Mpod_OutputClock_e108 = 6,      /* 108 MHz */

    /* Input band clocks */
    BXPT_Mpod_OutputClock_eIb0 = 8,
    BXPT_Mpod_OutputClock_eIb1 = 9,
    BXPT_Mpod_OutputClock_eIb2 = 10,
    BXPT_Mpod_OutputClock_eIb3 = 11,
    BXPT_Mpod_OutputClock_eIb4 = 12,
    BXPT_Mpod_OutputClock_eIb5 = 13,
    BXPT_Mpod_OutputClock_eIb6 = 14,
    BXPT_Mpod_OutputClock_eIb7 = 15,
    BXPT_Mpod_OutputClock_eIb8 = 16
}
BXPT_Mpod_OutputClock;
#endif

/***************************************************************************
Summary
Output clock rate divider.
****************************************************************************/
typedef enum BXPT_Mpod_OutputClockDivider
{
    BXPT_Mpod_OutputClockDivider_eNone,  /* No divider */
    BXPT_Mpod_OutputClockDivider_e12,    /* Divide clock by 12 */
    BXPT_Mpod_OutputClockDivider_e9,     /* Divide clock by 9 */
    BXPT_Mpod_OutputClockDivider_e8,     /* Divide clock by 8 */
    BXPT_Mpod_OutputClockDivider_e4,     /* Divide clock by 4 */
    BXPT_Mpod_OutputClockDivider_eMax
}
BXPT_Mpod_OutputClockDivider;

/***************************************************************************
Summary:
Configuration settings to used during BXPT_Mpod_Init().
****************************************************************************/
typedef struct BXPT_Mpod_Config
{
    /* Ouput Interface Formatter Control */
    unsigned char ByteSync;
    unsigned char ClkNrun;
    unsigned char InvertClk;
    unsigned char NshiftClk;
    unsigned char OutputInvertSync;

    /* Input Interface Formatter Control */
    unsigned char InputInvertSync;

#if BXPT_HAS_MPOD_SCARD_SUPPORT
    /* S-Mode is enabled when true, M-Mode when false */
    bool SmodeEn;

    /*
    ** Value inserted into the Host_reserved field of the 12-byte
    ** pre-header when HostRsvdEn is true.
    */
    unsigned int    HostRsvd;
    bool HostRsvdEn;

    /* Output clock delay, in 9.26 nS steps */
    unsigned int    ClkDelay;

    /* Invert the VALID and CLK signals. S-Mode only. */
    bool OutputInvertValid;
    bool InputInvertValid;
    bool InputInvertClk;

    /*
    ** Parser band used for data that originates in the S-Pod. This is
    ** interpreted as an playback parser band number if PbBand is true. Otherwise,
    ** the data is identified as an input band parser. Note that BandEn
    ** must be true to enable BandNo and PbBand. This value should be programmed
    ** if the single stream transport does not originate from either the chip's
    ** InBand or Playback paths but, instead originates from the S-Card itself.
    ** In this case, an unused InBand or Playback parser path would have to be
    ** chosen for downstream PID parsing of the incoming single transport stream
    ** from the S-Card (S-Mode only).
    */
    unsigned char BandNo;
    bool PbBand;
    bool BandEn;

    /*
    ** Replace the timestamp received from the interface with a new
    ** internally generated one.
    */
    bool TimestampInsertEn;

#else
    /* Additional Output Interface Formatter Control bits */
    unsigned char OutputDataSwap;
    unsigned char OutputDataLsbFirst;

    /* Input Packet Extractor Control */
    unsigned char CrcDropPacket;

    unsigned char SyncOnPosEdge;
    unsigned char InputDataSwap;
    unsigned char InputDataLsbFirst;
#endif
    /* Loopback mode, for tests only. */
    bool Loopback;

#if BXPT_HAS_MPOD_CARDBUS_SUPPORT
    bool CardBusModeEn;
#endif
    /* Support for the parallel interface.  */
#if BXPT_HAS_PARALLEL_MPOD
    bool ParallelEn;
#endif

    /* Output interface clock rate or source */
#if BXPT_HAS_MPOD_OUTPUT_CLOCK_RATE
    BXPT_Mpod_OutputClockRate OutputClockRate;
#endif
    BXPT_Mpod_OutputClockDivider OutputClockDivider;

    /*
    ** Selects which pin is used for the Pod2Chip MCLK:
    ** false (default) - Use the POD2CHIP_MCLKI input pin.
    ** true - Use the GPIO_37 input pin. Make sure that the pin is properly configured in the sundry block.
    */
#if BXPT_HAS_MPOD_MCLK_SELECT
    bool Pod2ChipMClkMuxSelect;
#endif

    /*
    ** Some external devices are known to output partial packets or other garbage data after being configured
    ** and receiving their first block of input data. In such cases, it's necessary to delay enabling the MPOD's
    ** input formatter until device output has stabilized. The MPOD interface's input formatter is normally
    ** enabled when BXPT_Mpod_Init() is called. The SkipInputFormatter bool, when set, will not enable the
    ** formatter until BXPT_Mpod_EnableInputFormatter() is called. By default, this bool is false to maintain
    ** previous behavior.
    */
    bool SkipInputFormatter;

#if BXPT_HAS_PROGRAMMABLE_MPOD_CLOCK
    BXPT_Mpod_OutputClock PacketOutputClock;    /* Clock source for the MPOD output. */
    unsigned PacketOutputDelay;     /* Number of clocks between packets on the output. 0 specifies no delay and is the default */
#endif

    /* Delay between packets on the output interface. Specified in clocks. */
    unsigned OutputPacketDelayCount;
}
BXPT_Mpod_Config;

/***************************************************************************
Summary:
Get the default config for the MultiPod interface.

Description:
Copy the default settings for the MultiPod registers into the Config
structure, which is then used in a subsequent call to BXPT_Mpod_Init(). The
user can customize the default values as needed before calling BXPT_Mpod_Init().

Returns:
    void

See Also:
BXPT_Mpod_Init
****************************************************************************/
void BXPT_Mpod_GetDefaultConfig(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_Mpod_Config *Config
    );

/***************************************************************************
Summary:
Enable or disable the input formatter.

Description:
The MPOD interface's input formatter is normally enabled when BXPT_Mpod_Init()
is called. The SkipInputFormatter option, when used, will not enable the
formatter until BXPT_Mpod_EnableInputFormatter() is called.

Returns:
    void

See Also:
BXPT_Mpod_Init
****************************************************************************/
void BXPT_Mpod_EnableInputFormatter(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    bool Enable
    );

/***************************************************************************
Summary:
Initialize the MultiPod interface.

Description:
The initial configuration of the MultiPod interface is loaded, then the interface
is enabled. Custom configuration is supported by setting the members of the
BXPT_Mpod_Config struct. After initializing the interface, parser data can
be sent through the interface by calling BXPT_Mpod_RouteToMpod().

Returns:
    BERR_SUCCESS                - MultiPod interface initialized.

See Also:
BXPT_Mpod_Shutdown, BXPT_Mpod_RouteToMpod
****************************************************************************/
BERR_Code BXPT_Mpod_Init(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    const BXPT_Mpod_Config *Config
    );

/***************************************************************************
Summary:
Shutdown the MultiPod interface.

Description:
All parser bands are configured to bypass the MultiPod interface, after which
the interface is disabled.

Returns:
    BERR_SUCCESS                - MultiPod interface shutdown.

See Also:
BXPT_Mpod_Init
****************************************************************************/
BERR_Code BXPT_Mpod_Shutdown(
    BXPT_Handle hXpt                            /* [in] Handle for this transport */
    );

/***************************************************************************
Summary:
Get PODRES field from the incoming MultiPod headers.

Description:
Get the value of the PODRES field in the last MultiPod header received.

Returns:
    unsigned int                - The PODRES field retrieved.
****************************************************************************/
unsigned int BXPT_Mpod_GetPodRes(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    );

/***************************************************************************
Summary:
Send all PIDS on a parser band out the MPOD interface.

Description:
Data for the selected parser is routed throught the MultiPod interface.

Returns:
    BERR_SUCCESS                - Parser data routed to interface.
    BERR_INVALID_PARAMETER      - Bad parser parameter

See Also:
BXPT_Mpod_Init, BXPT_Mpod_AllPass
****************************************************************************/
BERR_Code BXPT_Mpod_RouteToMpod(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    );

/***************************************************************************
Summary:
Send PID-filtered data out the MPOD interface.

Description:
Data for the selected parser is routed throught the MultiPod interface.
Only data on enabled PID channels mapped to the parser will be sent.

Returns:
    BERR_SUCCESS                - Parser data routed to interface.
    BERR_INVALID_PARAMETER      - Bad parser parameter

See Also:
BXPT_Mpod_Init, BXPT_Mpod_AllPass
****************************************************************************/
BERR_Code BXPT_Mpod_RouteToMpodPidFiltered(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool MpodPidFilter,         /* [in] enable pid filtering prior to the MCARD */
    bool ContinuityCountCheck,  /* [in] enable CC checking after the MCARD */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    );

/***************************************************************************
Summary:
Send all parser band data out the MPOD interface.

Description:
For routing data to the MultiPod, the PID channel assignments are ignored.
All data arriving at the given parser band is routed to the interface.

Returns:
    BERR_SUCCESS                - All data routed to interface.
    BERR_INVALID_PARAMETER      - Bad parser parameter

See Also:
BXPT_Mpod_Init, BXPT_Mpod_RouteToMpod
****************************************************************************/
BERR_Code BXPT_Mpod_AllPass(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] All pass mode enabled if true, disabled if false */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_MPOD_H__ */

/* end of file */
