/******************************************************************************
* (c) 2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bstd.h"
#include "bfpga_name.h"

BDBG_MODULE(BFPGA_NAME);

static const char * const BFPGA_TsSelectNames[] = {
#if BCHP_CHIP==7400
    "MB 4500SDS",       /* 0 */
    "MB 3517VSB",       /* 1 */
    "RMX 0",            /* 2 */
    "RMX 1",            /* 3 */
    "Reserved 4",       /* 4 */
    "1394",             /* 5 */
    "Reserved 6",       /* 6 */
    "Streamer 1",       /* 7 */
    "Slot0 TS2",        /* 8 */
    "Slot0 TS3",        /* 9 */
    "POD",              /* 10 */
    "Slot1 TS4",        /* 11 */
    "Slot1 TS5",        /* 12 */
    "Unknown 13",       /* 13 */
    "Slot 2",           /* 14 */
    "Disabled"          /* 15 */
#else
    "3250 DS 1",
    "3250 OOB",
    "7038 HSX 1",
    "7038 HSX 2",
    "7041 TS 0",
    "1394",
    "Streamer 2",
    "Streamer 1",
    "3250 DS 2",
    "7041 TS 1",
    "POD",
    "VSB/SDS 1",
    "VSB/SDS 2",
    "Reserved 1",
    "MS-POD",
    "Disabled"
#endif
};

#if BCHP_CHIP == 7400
/* PKT4 is available on 97038V4 boards. We have no way to detect version of board. Customers can turn this
on if they wish. For other boards, default it on. */
#define B_HAS_PKT4 1
#endif

static const char * const BFPGA_OutputSelectNames[] = {
    "Band 0",
    "Band 1",
    "Band 2",
    "Band 3",
    "1394",
    "Test",
    "POD",
    "AVC"
#if B_HAS_PKT4
    ,"Band 4" /* 0x0B */
#endif
};

const char * BFPGA_GetTsSelectName( BFPGA_TsSelect tsSelect )
{
    if (tsSelect >= sizeof(BFPGA_TsSelectNames)/sizeof(BFPGA_TsSelectNames[0])) return "unknown";

    return BFPGA_TsSelectNames[tsSelect];
}

const char * BFPGA_GetOutputSelectName( BFPGA_OutputSelect outSelect )
{
    if (outSelect >= sizeof(BFPGA_OutputSelectNames)/sizeof(BFPGA_OutputSelectNames[0])) return "unknown";

    return BFPGA_OutputSelectNames[outSelect];
}

void BFPGA_DumpConfiguration( BFPGA_Handle hFpga )
{
#if BDBG_DEBUG_BUILD
    BFPGA_info fpgaInfo;
    unsigned i;
    BFPGA_TsSelect tsSelect;
    BFPGA_ClockPolarity inClock, outClock;
    bool softConfig;
    int rc;

    rc = BFPGA_GetInfo(
        hFpga,
        &fpgaInfo
        );
    if (rc) {BERR_TRACE(rc); return;}

    BDBG_WRN(("FPGA Version: 0x%X, Board Cfg: 0x%X, Strap: 0x%X", fpgaInfo.fpga_ver, fpgaInfo.board_cfg, fpgaInfo.strap_pins ));

    for( i = 0; i < sizeof(BFPGA_OutputSelectNames)/sizeof(BFPGA_OutputSelectNames[0]); i++ )
    {
        BFPGA_GetTsOutput( hFpga, i, &tsSelect, &softConfig );
        BFPGA_GetClockPolarity( hFpga, i, &inClock, &outClock );

        BDBG_WRN(("%-12s (clk %s) ==> %-8s (clk %s) %s",
            BFPGA_GetTsSelectName(tsSelect), inClock?"-":"+",
            BFPGA_GetOutputSelectName(i), outClock?"-":"+",
            softConfig?"custom":"default"));
    }
    return;
#else
    BSTD_UNUSED(hFpga);
#endif
}

