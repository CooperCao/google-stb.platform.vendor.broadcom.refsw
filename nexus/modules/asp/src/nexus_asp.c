/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 **************************************************************************/
#include <stdio.h>
#include "nexus_asp_module.h"
#include "nexus_core_utils.h"
#include "nexus_client_resources.h"
#if NEXUS_HAS_SECURITY
#include "bhsm.h"
#include "bhsm_keyslot.h"
#include "priv/nexus_security_priv.h"
#include "nexus_security.h"
#endif
#include "basp_fw_api.h"
#include "basp.h"
#include "priv/nexus_core.h"
#include "bchp_xpt_rave.h"
#include "bxpt_capabilities.h"
#include "bchp_asp_mcpb.h"
#include "bchp_asp_mcpb_ch0.h"
#include "bchp_asp_mcpb_ch1.h"
#include "bchp_asp_edpkt_core.h"

BDBG_MODULE(nexus_asp);

NEXUS_ModuleHandle g_NEXUS_aspModule;
static bool g_enableTcpRetrans = true;
static bool g_enableTcpCongestionControl = true;
static bool g_enableTcpTimestamps = true;
static bool g_enableTcpSack = false;
static bool g_enableAch = false;

/* Setting this to non-zero will read ASP messages by polling
 * instead of using BASP interrupt callbacks. */
#define  USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS   0

#define NEXUS_NUM_ASP_CHANNELS BASP_MAX_NUMBER_OF_CHANNEL
                                    /* TODO: move this to a chip specific header file such as: */
                                    /* platforms/97272/include/nexus_platform_features.h */
                                    /* Doesn't belong in platforms/common/include/nexus_platform_generic_features_priv.h */
#define NEXUS_ASP_BLOCK_SIZE 192
#define NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK 512

typedef struct NEXUS_AspBuffer
{
    BMMA_Block_Handle   hBlock;
    void                *pBuffer;
    BMMA_DeviceOffset   offset;
    BMMA_Heap_Handle    hMmaHeap;      /* if non-NULL, then the block is internally-allocated */
    NEXUS_HeapHandle    hNexusHeap;
    unsigned            size;
} NEXUS_AspBuffer;

/* Per ASP Channel Structure. */
typedef struct NEXUS_AspChannel
{
    NEXUS_OBJECT(NEXUS_AspChannel);
    int                                 channelNum;
    NEXUS_AspChannelCreateSettings      createSettings;
    NEXUS_AspChannelSettings            settings;
    NEXUS_AspChannelStartSettings       startSettings;
    NEXUS_AspChannelState               state;
    NEXUS_AspBuffer                     writeFifo;
    NEXUS_AspBuffer                     reTransmitFifo;
    NEXUS_AspBuffer                     reAssembledFifo;
    NEXUS_AspBuffer                     receiveFifo;
    unsigned char                       *pRcvdPayload; /* Starting position of the last received payload */
    unsigned                            rcvdPayloadLength;
    NEXUS_AspBuffer                     miscBuffer;
    NEXUS_AspBuffer                     m2mDmaDescBuffer;
    BASP_ChannelHandle                  hChannel;
    NEXUS_AspChannelStatus              initialStatus;
    NEXUS_AspChannelStatus              currentStatus;
    NEXUS_IsrCallbackHandle             hStateChangedCallback;
    NEXUS_IsrCallbackHandle             hDataReadyCallback;
    struct
    {
        unsigned                        finalSendSequenceNumber;
        unsigned                        finalRecvSequenceNumber;
    } tcpState;
    bool                                gotStartResponse;
    bool                                gotStopResponse;
    bool                                gotPayloadConsumedResponse;
    NEXUS_AspChannelDtcpIpSettings      dtcpIpSettings;
#ifdef NEXUS_HAS_SECURITY
    BHSM_KeyslotHandle                  hKeySlot;
    unsigned                            extKeyTableSlotIndex;
#endif
    unsigned                            writeFifoLength;            /* # of bytes filled-into the write buffer. */
} NEXUS_AspChannel;

/* Global ASP State. */
static struct
{
    NEXUS_AspModuleSettings             settings;
    BASP_Handle                         hAspBaseModule;
    unsigned                            createdChannelCount;
    struct NEXUS_AspChannel             *hAspChannelList[BASP_MAX_NUMBER_OF_CHANNEL];
    NEXUS_AspBuffer                     rxPacketHeadersBuffer;      /* Buffer to hold up incoming packet headers before firmware gets to process them. */
    BASP_ContextHandle                  hContext;
    NEXUS_TimerHandle                   hTimer;
    unsigned                            timerIntervalInMs;
    NEXUS_AspBuffer                     statusBuffer;
} g_NEXUS_asp;

#if NEXUS_HAS_SECURITY
#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_asp.settings.modules.security)
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_asp.settings.modules.security)
#endif

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
static void NEXUS_ProcessMsgFromFwCallbackByTimer(void *pContext);
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
static void NEXUS_ProcessMsgFromFwCallback_isr(void *pContext, int param);

static void checkRegContent(
    BREG_Handle hReg,
    const char *pRegName,
    uint32_t regAddr,
    uint32_t regExpected
    )
{
    uint32_t regActual;

    regActual = BREG_Read32(hReg, regAddr);
    BDBG_LOG(("%s: Reg=%s: 0x%08x  Expected 0x%08x,  Found 0x%08x",
                BSTD_FUNCTION, pRegName, regAddr, regExpected, regActual));
}

#if 0
static void checkRegContent64(
    BREG_Handle hReg,
    const char *pRegName,
    uint32_t regAddr,
    uint64_t regExpected
    )
{
    uint64_t regActual;

    regActual = BREG_Read64(hReg, regAddr);
    BDBG_LOG(("%s: Reg=%s: 0x%08x  Expected 0x%08llx,  Found 0x%08llx",
                BSTD_FUNCTION, pRegName, regAddr, regExpected, regActual));
}
#endif

#define APP_TS_PKT_LEN              188
#define APP_TS_PKTS_IN_IP_PKT       7
#define APP_NUM_IP_PKTS             10
#define APP_TCP_START_SEQ_NUM       1000
#define APP_TCP_START_ACK_NUM       512

/* Basic switch configuration for unmanaged mode. */
static void configSwitch(
    BREG_Handle hReg
    )
{
#ifdef B_REFSW_OS_linuxemu
    /* Enable IMP (P8), P7 (ASP), P0 Switch Ports. */
    {
        /* switch_core.CTL_IMP.set(32'h00000000);*/
        {
            BREG_Write32(hReg, 0xf00040, 0);
            checkRegContent(hReg, "SWITCH_CORE_CtlImp", 0xf00040, 0x0);
        }

        /* switch_core.G_PCTL_P0.set(32'h00000000); */
        BREG_Write32(hReg, 0xf00000, 0);
        checkRegContent(hReg, "SWITCH_CORE_PCTL_P0", 0xf00000, 0x0);

        /* Port 7 Config */
        /* switch_core.CTL_P7.set(32'h00000000); */
        BREG_Write32(hReg, 0xf00038, 0);
        checkRegContent(hReg, "SWITCH_CORE_CtlP7", 0xf00038, 0x0);
    }
#endif

    /* Configure switch in unmanaged mode. */
#ifdef B_REFSW_OS_linuxemu
    {
        BREG_Write32(hReg, 0xf00058, 0x6);
        checkRegContent(hReg, "SWITCH_CORE_SwMode", 0xf00058, 0x6);

        BREG_Write32(hReg, 0xf00110, 0x40);
        checkRegContent(hReg, "SWITCH_CORE_Ctrl", 0xf00110, 0x40);
    }
#endif

    /* Disable Learning on P7 */
    {
        BREG_Write32(hReg, 0xf001e0, 0x80);
        checkRegContent(hReg, "SWITCH_CORE_DisLearn", 0xf001e0, 0x80);
    }

#ifdef B_REFSW_OS_linuxemu
    /* Set port speed, duplex, link status for enabled ports */
    {
        /* switch_core.STS_OVERRIDE_GMII_P0.set(32'h0000007b); */
        BREG_Write32(hReg, 0xf72000, 0x4b);
        checkRegContent(hReg, "STS_OVERRIDE_GMII_P0", 0xf72000, 0x4b);

        /* switch_core.STS_OVERRIDE_GMII_P7.set(32'h000000fb); */
        BREG_Write32(hReg, 0xf72070, 0x4b);
        checkRegContent(hReg, "STS_OVERRIDE_GMII_P7", 0xf72070, 0x4b);

        /* switch_core.STS_OVERRIDE_IMP.set(32'h000000fb); */
        BREG_Write32(hReg, 0xf72080, 0x4b);
        checkRegContent(hReg, "STS_OVERRIDE_IMP", 0xf72080, 0x4b);
    }
#endif

    /* Enable Broadcom Tag for ASP Port 7. */
    {
#ifdef B_REFSW_OS_linuxemu
        BREG_Write32(hReg, 0xf01018, 0x4); /* Enable Brcm Tag only on Port7. */
        checkRegContent(hReg, "BrcmHdrCtrl", 0xf01018, 0x4);
        BREG_Write32(hReg, 0xf01018, 0x5); /* Enable Brcm Tag on Port7 & IMP Port. */
        checkRegContent(hReg, "BrcmHdrCtrl", 0xf01018, 0x5);
#endif
    }

    /* Config ASP UNIMAC for interfacing w/ Switch at 1Gbps. */
    /* TODO: this also needs to be done by ASP FW. */
#if (BCHP_CHIP==7278 && BCHP_VER==A0)
    {
        BREG_Write32(hReg, 0x184c008, 0x010000db);
        checkRegContent(hReg, "ASP_UMAC_CMD", 0x184c008, 0x010000db);

        /* Program dummy MAC for now. */
        BREG_Write32(hReg, 0x184c00c, 0x00010203);
        checkRegContent(hReg, "ASP_UMAC_MAC0", 0x184c00c, 0x00010203);

        /* Program dummy MAC for now. */
        BREG_Write32(hReg, 0x184c010, 0x00000405);
        checkRegContent(hReg, "ASP_UMAC_MAC1", 0x184c010, 0x00000405);
    }
#endif

    /* TODO: Disable ACH logic in Switch until we add the full ACH support. */
    if (g_enableAch == false)
    {
        uint32_t value;
        value = BREG_Read32(hReg, 0xf80800);
        value = value & ~1;
        BDBG_WRN(("Disabling ACH logic in switch: value = 0x%x", value));
        BREG_Write32(hReg, 0xf80800, value);
    }
}

#if 0
static void validateRunStateOld(
    BREG_Handle                         hReg
    )
{
    uint32_t regAddr;
    uint32_t regExpected;

    BDBG_LOG(("Printing runtime state of various h/w block..........."));

    /* XPT Rave stats. */
    {
        uint64_t validOffset, readOffset, baseOffset, endOffset;
        uint64_t cdbSize, cdbDepth;

        readOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR);
        validOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR);
        baseOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR);
        endOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR);
        cdbSize = endOffset - baseOffset;
        if (validOffset > readOffset)
        {
            cdbDepth = validOffset - readOffset;
        }
        else
        {
            cdbDepth = validOffset + cdbSize - readOffset;
        }


#if 0
        BDBG_LOG(("%s: >>>> RAVE FIFO: size=%llu depth=%llu", BSTD_FUNCTION, cdbSize, cdbDepth));
        checkRegContent(hReg, "RaveWrPtrLo", BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR, 0);
        checkRegContent(hReg, "RaveVaPtrLo", BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR, 0);
        checkRegContent(hReg, "RaveRdPtrLo", BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR, 0);
#endif
    }

    /* Now validate key registers to reflect the state that streamer is running. */
    {
        /* EPKT Seq # */
        regAddr = 0x18440e0;    /* ASP_EPKT_CORE_CH00_CH_SEQ_NUM */
        regExpected = APP_TCP_START_SEQ_NUM + (APP_TS_PKT_LEN * APP_TS_PKTS_IN_IP_PKT * (APP_NUM_IP_PKTS-1)); /* -1 as MCPB will hold last pkt until we insert the EOF BTP. */
        checkRegContent(hReg, "AspEpktSeqNum", regAddr, regExpected);

        regAddr = 0x18440e4;    /* ASP_EPKT_CORE_CH00_CH_RETX_SEQ_NUM */
        regExpected = APP_TCP_START_SEQ_NUM + (APP_TS_PKT_LEN * APP_TS_PKTS_IN_IP_PKT * APP_NUM_IP_PKTS);
        checkRegContent(hReg, "AspEpktRetxSeqNum", regAddr, regExpected);

        /* EPKT PKT Counter:  # */
        regAddr = 0x1851220;
        regExpected = (APP_TS_PKTS_IN_IP_PKT * APP_NUM_IP_PKTS);
        checkRegContent(hReg, "AspMcpbPktCounter", regAddr, regExpected);
#if 0
        {
            uint32_t currentTsPktCount;
            currentTsPktCount = BREG_Read32(hReg, 0x1851220);
            if (
        }
#endif

#if 0
        /* EPKT ReTx related stats. */
        checkRegContent(hReg, "RetxBufLevel", 0x18440e8, 0);
        checkRegContent(hReg, "RetxBufSeq", 0x18440ec, 0);
        checkRegContent64(hReg, "RetxBase", 0x18440f0, 0);
        checkRegContent64(hReg, "RetxEnd", 0x18440f8, 0);
        checkRegContent64(hReg, "RetxWrite", 0x1844100, 0);
        checkRegContent64(hReg, "RetxValid", 0x1844108, 0);
        checkRegContent64(hReg, "RetxRead", 0x1844110, 0);
#endif

#if 0
        /* Dump the first 10 on CHIP descriptors that are used to feed data from Rave context to MCPB. */
        {
            int i;
            regAddr = 0x1859000;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
            regExpected = 0;
            for (i=0; i<10; i++)
            {
                regAddr = 0x1859000 + i*4;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
                checkRegContent(hReg, "AspMcpbOnChipDesc", regAddr, regExpected);
            }

            regAddr = 0x1851030;  /* ASP_MCPB_CH0_DMA_BUFF_WR_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DmaWrAddr", regAddr, regExpected);

            /* Read DCPM_DATA_ADDR: it should be set to Valid pointer. */
            regAddr = 0x1851238; /* ASP_MCPB_CH0_DCPM_DATA_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DpcmDataAddr", regAddr, regExpected);
        }
#endif
    }

    /* EPKT Unimac stats. */
    {
#if 0
        checkRegContent(hReg, "ASP_EDPKT_UNMAC_MIB_GTPKT", 0x18414a8, 0);
        checkRegContent(hReg, "ASP_EDPKT_UNMAC_MIB_GTFCS", 0x18414bc, 0);
        checkRegContent(hReg, "ASP_EDPKT_UNMAC_MIB_GTBYT: Good Byte Count", 0x18414e8, 0);
        checkRegContent(hReg, "ASP_EDPKT_UNMAC_MIB_GTPOK: Good Pkt Count", 0x18414ec, 0);
#endif
        checkRegContent(hReg, "ASP_EDPKT_UNMAC_MIB_GTUC: Unicast Pkt Count", 0x18414f0, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */
    }
    /* Check Switch Tx Stats. */
    {
#if 0
        checkRegContent(hReg, "STS_OVERRIDE_GMII_P7", 0xf72070, 0x4b);

        checkRegContent(hReg, "SWITCH_CORE_TxOctets_P7", 0xf13800, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxDrops_P7", 0xf13840, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxUnicastPkts_P7", 0xf138c0, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxPkts1024toMaxPktOctets_P7", 0xf13f20, 0);
#endif
#if 0
        checkRegContent(hReg, "SWITCH_CORE_RxOctets_P7", 0xf13a80, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxUndresizePkts_P7", 0xf13ac0, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxOversizePkts_P7", 0xf13bc0, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxGoodOctets_P7", 0xf13c40, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxDropPkts_P7", 0xf13c80, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxDiscard_P7", 0xf13e00, 0);
#endif
        checkRegContent(hReg, "SWITCH_CORE_RxUnicastPkts_P7", 0xf13ca0, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */
        checkRegContent(hReg, "SWITCH_CORE_TxUnicastPkts_P7", 0xf138c0, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */

#if 0
        checkRegContent(hReg, "SWITCH_CORE_TxOctets_P0", 0xf10000, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxDrops_P0", 0xf10040, 0);
#endif
        checkRegContent(hReg, "SWITCH_CORE_TxUnicastPkts_P0", 0xf100c0, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */
#if 0
        checkRegContent(hReg, "SWITCH_CORE_InRangeErrCount_P0", 0xf10580, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxFrameInDisc_P7", 0xf139a0, 0);
        checkRegContent(hReg, "SWITCH_CORE_RxDiscard_P0", 0xf10600, 0);
#endif

#if 0
        checkRegContent(hReg, "SWITCH_CORE_TxOctets_IMP", 0xf14000, 0);
        checkRegContent(hReg, "SWITCH_CORE_TxDrops_IMP",  0xf14040, 0);
        checkRegContent(hReg, "SWITCH_CORE_LinkStatus", 0xf00800, 0);
#endif
        checkRegContent(hReg, "SWITCH_CORE_TxUnicastPkts_IMP", 0xf140c0, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */
    }

    /* Check SystemPort Stats. */
    {
#if 0
        checkRegContent(hReg, "SysPortRxChkCsumBadDiscard", 0x130034c, 0);
        checkRegContent(hReg, "SysPortRxChkOtherDiscard", 0x1300350, 0);
        checkRegContent(hReg, "SysPort0RdmaStatus", 0x1303008, 0);
        checkRegContent(hReg, "SysPortRdmaOverflow", 0x130301c, 0);
        checkRegContent(hReg, "SysPortTopCntrlMiscCntl", 0x130000c, 0);
        checkRegContent(hReg, "RdmaControl", 0x1303000, 0x00000007);
        checkRegContent(hReg, "RdmaTest", 0x130304c, 0);
        checkRegContent(hReg, "GibRcvdErrCnt", 0x1301010, 0x00000000);
        checkRegContent(hReg, "GibControl", 0x1301000, 0x00000000);
        checkRegContent(hReg, "GibStatus", 0x1301004, 0x00000000);
        checkRegContent(hReg, "BigMacDa0", 0x130100c, 0x00000000);
        checkRegContent(hReg, "RbufErrPktCount", 0x1300410, 0x00000000);
        checkRegContent(hReg, "RbufOverflowPktDiscardCount", 0x130040c, 0x00000000);
        checkRegContent(hReg, "SysPortRdmaProConIndex", 0x1303020, APP_NUM_IP_PKTS-1); /* -1 as HW will need a BPT to flush out the last packet. */
#endif
    }
}
#endif

#if 0
static bool tcpRetransmissionActive(
    BREG_Handle                         hReg
    )
{
    bool tcpRetransmissionActive = false;
    BSTD_UNUSED(hReg);

    if (g_enableTcpRetrans == false)
    {
        /* We didn't disable TCP retransmissions, so check its status from h/w. */
        tcpRetransmissionActive = true;
    }
    return (tcpRetransmissionActive);
}
#endif

#if 0
static void determineAspHwPipeStatus(
    NEXUS_AspChannelHandle              hAspChannel,
    BREG_Handle                         hReg,
    NEXUS_AspChannelStats               *pCurrentStats,
    NEXUS_AspChannelStats               *pNewStats
    )
{

    /* Check if all of the h/w blocks are configured correctly & are up! */

    /* Check if ASP has sent any packets at all!. */

    /* Check if ASP has received any packets back (TCP ACKs). */
    /* If no packets are received back, check if ASP's switch port is up. */
    if (BREG_Read(hReg, 0xf00038) != 0x0) /* "SWITCH_CORE_CtlP7" */
    {
        BDBG_ERR(("%s: ASP Port(#7) is NOT configured! Look into switch configuration by Linux. "));
        checkRegContent(hReg, "STS_OVERRIDE_GMII_P7", 0xf72070, 0x4b);
        BREG_Write32(hReg, 0xf01018, 0x5); /* Enable Brcm Tag on Port7 & IMP Port. */
    }

    /* Check if current pipe is stalled. */
    /* then MCPB packet counter should have changed between the current & new stats. */

    /* If retransmissions are either disabled or not currently happening, */
    /* then MCPB packet counter should have changed between the current & new stats. */
    {
#if 0
        /* EPKT Seq # */
        regAddr = 0x18440e0;    /* ASP_EPKT_CORE_CH00_CH_SEQ_NUM */
        regAddr = 0x18440e4;    /* ASP_EPKT_CORE_CH00_CH_RETX_SEQ_NUM */
        /* EPKT ReTx related stats. */
        checkRegContent(hReg, "RetxBufLevel", 0x18440e8, 0);
        checkRegContent(hReg, "RetxBufSeq", 0x18440ec, 0);
        checkRegContent64(hReg, "RetxBase", 0x18440f0, 0);
        checkRegContent64(hReg, "RetxEnd", 0x18440f8, 0);
        checkRegContent64(hReg, "RetxWrite", 0x1844100, 0);
        checkRegContent64(hReg, "RetxValid", 0x1844108, 0);
        checkRegContent64(hReg, "RetxRead", 0x1844110, 0);
#endif
    }

#if 0
        /* Dump the first 10 on CHIP descriptors that are used to feed data from Rave context to MCPB. */
        {
            int i;
            regAddr = 0x1859000;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
            regExpected = 0;
            for (i=0; i<10; i++)
            {
                regAddr = 0x1859000 + i*4;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
                checkRegContent(hReg, "AspMcpbOnChipDesc", regAddr, regExpected);
            }

            regAddr = 0x1851030;  /* ASP_MCPB_CH0_DMA_BUFF_WR_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DmaWrAddr", regAddr, regExpected);

            /* Read DCPM_DATA_ADDR: it should be set to Valid pointer. */
            regAddr = 0x1851238; /* ASP_MCPB_CH0_DCPM_DATA_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DpcmDataAddr", regAddr, regExpected);
        }
#endif

    {
        pStats->mcpbConsumedInTsPkts = BREG_Read32(hReg, 0x1851220);
        pStats->mcpbConsumedInBytes = pStats->mcpbConsumedInTsPkts * 188; /* Consider w/ timestamps. */
        pStats->mcpbConsumedInIpPkts = pStats->mcpbConsumedInTsPkts / 7; /* ASP Sends 7 TS packets in 1 IP packet. */
        pStats->unimacTxUnicastIpPkts = BREG_Read32(hReg, 0x18414f0);
        pStats->unimacTxUnicastIpBytes = BREG_Read32(hReg, 0x18414e8); /* TODO: consider 32bit wrap. */
        /* Switch stats are cummulative. */
        pStats->nwSwRxFmAspInUnicastIpPkts = BREG_Read32(hReg, 0xf13ca0) - (pInitialStats ? pInitialStats->stats.nwSwRxFmAspInUnicastIpPkts : 0);
        pStats->nwSwTxToP0InUnicastIpPkts = BREG_Read32(hReg, 0xf100c0)- (pInitialStats ? pInitialStats->stats.nwSwTxToP0InUnicastIpPkts : 0);
        pStats->nwSwTxToHostInUnicastIpPkts = BREG_Read32(hReg, 0xf140c0)- (pInitialStats ? pInitialStats->stats.nwSwTxToHostInUnicastIpPkts : 0);

        pStats->nwSwTxToAspInUnicastIpPkts = BREG_Read32(hReg, 0xf138c0);
    }
}
#endif

int oneTimeFlag = 1;
static void updateStats(
    NEXUS_AspChannelHandle              hAspChannel,
    BREG_Handle                         hReg,
    int                                 channelNumber,
    NEXUS_AspChannelStatus               *pInitialStats,
    NEXUS_AspChannelStatus               *pStatus
    )
{
    /* XPT Rave stats. */
    {
        uint64_t validOffset, readOffset, baseOffset, endOffset;
        unsigned cdbSize, cdbDepth;

        readOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR);
        validOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR);
        baseOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR);
        endOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR);
        cdbSize = endOffset - baseOffset;
        if (validOffset > readOffset)
        {
            cdbDepth = validOffset - readOffset;
        }
        else
        {
            cdbDepth = validOffset + cdbSize - readOffset;
        }
        pStatus->stats.raveCtxDepthInBytes = cdbDepth;
        pStatus->stats.raveCtxSizeInBytes = cdbSize;
    }

    {
#if 0
        /* EPKT Seq # */
        regAddr = 0x18440e0;    /* ASP_EPKT_CORE_CH00_CH_SEQ_NUM */
        regAddr = 0x18440e4;    /* ASP_EPKT_CORE_CH00_CH_RETX_SEQ_NUM */
        /* EPKT ReTx related stats. */
        checkRegContent(hReg, "RetxBufLevel", 0x18440e8, 0);
        checkRegContent(hReg, "RetxBufSeq", 0x18440ec, 0);
        checkRegContent64(hReg, "RetxBase", 0x18440f0, 0);
        checkRegContent64(hReg, "RetxEnd", 0x18440f8, 0);
        checkRegContent64(hReg, "RetxWrite", 0x1844100, 0);
        checkRegContent64(hReg, "RetxValid", 0x1844108, 0);
        checkRegContent64(hReg, "RetxRead", 0x1844110, 0);
#endif
    }

#if 0
        /* Dump the first 10 on CHIP descriptors that are used to feed data from Rave context to MCPB. */
        {
            int i;
            regAddr = 0x1859000;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
            regExpected = 0;
            for (i=0; i<10; i++)
            {
                regAddr = 0x1859000 + i*4;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
                checkRegContent(hReg, "AspMcpbOnChipDesc", regAddr, regExpected);
            }

            regAddr = 0x1851030;  /* ASP_MCPB_CH0_DMA_BUFF_WR_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DmaWrAddr", regAddr, regExpected);

            /* Read DCPM_DATA_ADDR: it should be set to Valid pointer. */
            regAddr = 0x1851238; /* ASP_MCPB_CH0_DCPM_DATA_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DpcmDataAddr", regAddr, regExpected);
        }
#endif

    /* MCPB Stats. */
    {
        uint32_t mcpbChFieldOffset;
        uint32_t mcpbChannelSize = BCHP_ASP_MCPB_CH1_DMA_DESC_ADDR - BCHP_ASP_MCPB_CH0_DMA_DESC_ADDR;
        uint32_t newValue, curValue;

        /* Find offset for the ASP MCPB channel being used. */
        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (channelNumber * mcpbChannelSize);
        newValue = BREG_Read32(hReg, mcpbChFieldOffset);
        curValue = (uint32_t)(pStatus->stats.mcpbConsumedInTsPkts & 0xFFFFFFFF);
        pStatus->stats.mcpbConsumedInTsPkts += newValue - curValue;
        BDBG_MSG(("MCPBPktCounter value: new=%u cur=%u hi=%u lo=%u", newValue, curValue, (uint32_t)(pStatus->stats.mcpbConsumedInTsPkts >> 32), (uint32_t)(pStatus->stats.mcpbConsumedInTsPkts&0xFFFFFFFF)));

        if (hAspChannel->startSettings.mode == NEXUS_AspStreamingMode_eOut)
        {
            pStatus->stats.mcpbConsumedInBytes = pStatus->stats.mcpbConsumedInTsPkts * 188; /* Consider w/ timestamps. */
            pStatus->stats.mcpbConsumedInIpPkts = pStatus->stats.mcpbConsumedInTsPkts / 7; /* ASP Sends 7 TS packets in 1 IP packet. */
        }
        else
        {
            pStatus->stats.mcpbConsumedInBytes = pStatus->stats.mcpbConsumedInTsPkts;
            pStatus->stats.mcpbConsumedInIpPkts = pStatus->stats.mcpbConsumedInTsPkts; /* Each MCPB local counter accounts for 1 ACK packet. */
        }

        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_ESCD_TCPIP_MAX_SEQUENCE_NUMBER + (channelNumber * mcpbChannelSize);
        pStatus->stats.mcpbSendWindow = BREG_Read32(hReg, mcpbChFieldOffset) - BREG_Read32(hReg, mcpbChFieldOffset+4); /* max - cur seq # */

        pStatus->stats.mcpbChEnabled = BREG_Read32(hReg, BCHP_ASP_MCPB_RUN_STATUS_0_31) & (1<<channelNumber);
        pStatus->stats.mcpbDescFifoEmpty = BREG_Read32(hReg, BCHP_ASP_MCPB_ON_CHIP_DESC_FIFO_EMPTY_0_31) & (1<<channelNumber);

        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_DMA_DATA_BUFF_DEPTH_MONITOR + (channelNumber * mcpbChannelSize);
        pStatus->stats.mcpbPendingBufferDepth = BREG_Read32(hReg, mcpbChFieldOffset);

        /* TODO: Add Retx related counters: ASP_MCPB_CH0_DMA_RETRANS_BUFF* */
        curValue = BREG_Read32(hReg, BCHP_ASP_MCPB_RETRANS_AV_PAUSE_STATUS_0_31);
        pStatus->stats.mcpbAvPaused = curValue & (1<<channelNumber) ? true:false;

        curValue = BREG_Read32(hReg, BCHP_ASP_MCPB_DEBUG_14);
        pStatus->stats.mcpbStalled = curValue >> 10 & 0x1;
    }

    /* EPKT Stats. */
    {
        /* ASP_EPKT_CORE_CH00_CH_CONFIG_MISC */
    }

    /* EDPKT Stats. */
    {
        uint32_t eDpktPendingPktsOffset;
        uint32_t value;
#if (BCHP_VER == A0)
        pStatus->stats.eDpktRxIpPkts = BREG_Read32(hReg, BCHP_ASP_EDPKT_CORE_DEBUG_STATUS_REG2);
#else
        pStatus->stats.eDpktRxIpPkts = BREG_Read32(hReg, BCHP_ASP_EDPKT_CORE_RX_PKT_CNT);
#endif

        eDpktPendingPktsOffset = BCHP_ASP_EDPKT_CORE_CHANNEL_01_00_HEADER_COUNT_REG + ((channelNumber/2)*4);
        value = BREG_Read32(hReg, eDpktPendingPktsOffset);
        pStatus->stats.eDpktPendingPkts = (channelNumber&0x1) ? (value >> 16) : (value & 0xffff);
    }


    /* Switch stats are common for all channels & are cummulative. */
    {
        pStatus->stats.unimacTxUnicastIpPkts = BREG_Read32(hReg, 0x18414f0);
        pStatus->stats.unimacTxUnicastIpBytes = BREG_Read32(hReg, 0x18414e8); /* TODO: consider 32bit wrap. */
        pStatus->stats.nwSwRxFmAspInUnicastIpPkts = BREG_Read32(hReg, 0xf13ca0) - (pInitialStats ? pInitialStats->stats.nwSwRxFmAspInUnicastIpPkts : 0);
        pStatus->stats.nwSwTxToP0InUnicastIpPkts = BREG_Read32(hReg, 0xf100c0) - (pInitialStats ? pInitialStats->stats.nwSwTxToP0InUnicastIpPkts : 0);
        pStatus->stats.nwSwTxToHostInUnicastIpPkts = BREG_Read32(hReg, 0xf140c0) - (pInitialStats ? pInitialStats->stats.nwSwTxToHostInUnicastIpPkts : 0);

        pStatus->stats.nwSwTxToAspInUnicastIpPkts = BREG_Read32(hReg, 0xf138c0) - (pInitialStats ? pInitialStats->stats.nwSwTxToAspInUnicastIpPkts : 0);
        pStatus->stats.nwSwRxP0InUnicastIpPkts = BREG_Read32(hReg, 0xf104a0) - (pInitialStats ? pInitialStats->stats.nwSwRxP0InUnicastIpPkts : 0);
        pStatus->stats.unimacRxUnicastIpPkts = BREG_Read32(hReg, 0x1841468);
        pStatus->stats.nwSwRxP8InUnicastIpPkts = BREG_Read32(hReg, 0xf144a0) - (pInitialStats ? pInitialStats->stats.nwSwRxP8InUnicastIpPkts : 0);
        pStatus->stats.nwSwRxP0InDiscards = BREG_Read32(hReg, 0xf10600) - (pInitialStats ? pInitialStats->stats.nwSwRxP0InDiscards : 0);
    }
}

/************** Module level functions ***************/
void NEXUS_AspModule_GetDefaultSettings(
    NEXUS_AspModuleSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static void unLockAndFreeBuffer(
    NEXUS_AspBuffer                         *pBuffer
    )
{
    BDBG_ASSERT(pBuffer);

    if (pBuffer->pBuffer) BMMA_Unlock(pBuffer->hBlock, pBuffer->pBuffer);
    BMMA_UnlockOffset(pBuffer->hBlock, pBuffer->offset);
    BDBG_MSG(("%s: hNexusHeap=%p hMmaHeap=%p pBuffer=%p offset hi:lo=0x%x:0x%x",
                BSTD_FUNCTION, (void *)pBuffer->hNexusHeap, (void *)pBuffer->hMmaHeap, (void *)pBuffer->pBuffer, (uint32_t)(pBuffer->offset>>32), (uint32_t)pBuffer->offset));

    BMMA_Free(pBuffer->hBlock);
} /* unLockAndFreeBuffer */

static NEXUS_Error allocateAndLockBuffer(
    const NEXUS_AspChannelCreateBufferSettings  *pBufferSettings,
    bool                                        needCpuAccess,
    unsigned                                    alignmentInBytes,
    NEXUS_AspBuffer                             *pBuffer
    )
{
    NEXUS_Error             rc;
    NEXUS_HeapHandle        hHeap;

    BDBG_ASSERT(pBufferSettings);
    BDBG_ASSERT(pBuffer);

    BKNI_Memset(pBuffer, 0, sizeof(*pBuffer));

    /* TODO: Add logic to allocate from user provided context. */
    if (pBufferSettings->memory) BDBG_WRN(("%s: pBufferSettings->memory is not yet supported!", BSTD_FUNCTION));
    BDBG_ASSERT(pBufferSettings->memory == NULL);

    /* Select the heap to use for buffer allocation. */
    hHeap = NEXUS_P_DefaultHeap(pBufferSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (!hHeap) hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;

    pBuffer->hNexusHeap = hHeap;
    pBuffer->hMmaHeap = NEXUS_Heap_GetMmaHandle(hHeap);
    BDBG_ASSERT(pBuffer->hMmaHeap);

    /* Now allocate the buffer. */
    pBuffer->hBlock = BMMA_Alloc(pBuffer->hMmaHeap, pBufferSettings->size, alignmentInBytes, NULL);
    if (!pBuffer->hBlock) { rc=BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }

    /* Lock the buffer for CPU & Device access. */
    if (needCpuAccess)
    {
        pBuffer->pBuffer = BMMA_Lock(pBuffer->hBlock);
        BDBG_ASSERT(pBuffer->pBuffer);
    }

    pBuffer->offset = BMMA_LockOffset(pBuffer->hBlock);
    BDBG_ASSERT(pBuffer->offset);
    pBuffer->size = pBufferSettings->size;

    BDBG_MSG(("%s: size=%u needCpuAccess=%s hNexusHeap=%p hMmaHeap=%p pBuffer=%p offset hi:lo=0x%x:0x%x",
                BSTD_FUNCTION, pBufferSettings->size, needCpuAccess?"Y":"N", (void *)pBuffer->hNexusHeap, (void *)pBuffer->hMmaHeap, (void *)pBuffer->pBuffer, (uint32_t)(pBuffer->offset>>32), (uint32_t)pBuffer->offset));
    rc = NEXUS_SUCCESS;
error:
    return (rc);

} /* allocateAndLockBuffer */

#include <stdlib.h>
NEXUS_ModuleHandle NEXUS_AspModule_Init(
    const NEXUS_AspModuleSettings *pSettings
    )
{
    BERR_Code rc;
    BASP_OpenSettings aspOpenSettings;

    BDBG_ASSERT(!g_NEXUS_aspModule);

    g_NEXUS_aspModule = NEXUS_Module_Create("asp", NULL);
    if (pSettings)
    {
        g_NEXUS_asp.settings = *pSettings;
#ifdef NEXUS_HAS_SECURITY
        BDBG_MSG(("hSecurityModule=%p", (void *)pSettings->modules.security));
        BDBG_ASSERT(pSettings->modules.security);
#endif
    }
    else
    {
        NEXUS_AspModule_GetDefaultSettings(&g_NEXUS_asp.settings);
    }

    {
        NEXUS_AspChannelCreateBufferSettings    bufferCreateSettings;

        BKNI_Memset(&bufferCreateSettings, 0, sizeof(bufferCreateSettings));
        bufferCreateSettings.size = sizeof(BASP_FwStatusInfo);
        rc = allocateAndLockBuffer(&bufferCreateSettings, true /*needCpuAccess*/, BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES, &g_NEXUS_asp.statusBuffer);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        BDBG_MSG(("%s: Allocated %d bytes of status Buffer!", BSTD_FUNCTION, bufferCreateSettings.size));
        BKNI_Memset((char *)g_NEXUS_asp.statusBuffer.pBuffer, 0, g_NEXUS_asp.statusBuffer.size);
    }

#if 0
    {
        /* Reset ASP HW Blocks */
        BDBG_LOG(("!!! TODO (more this to FW): Reset ASP HW blocks!!.."));
        /* Can't reset the ASP HW blocks like this as this causes random stalls while streaming out. */
        /* This was done to reset EDPKT block upon a stall otherwise, it requires a system boot. */
        BREG_Write32(g_pCoreHandles->reg, 0x1870000, 0x1f);
        BREG_Write32(g_pCoreHandles->reg, 0x1870000, 0x0);
    }
#endif

    /* Initialize BASP basemodule. */
    {
        BASP_GetDefaultOpenSettings(&aspOpenSettings);
        aspOpenSettings.ui64StatusBufferDeviceOffset = g_NEXUS_asp.statusBuffer.offset;
        aspOpenSettings.ui32StatusBufferSize = g_NEXUS_asp.statusBuffer.size;
        BDBG_MSG(("Calling BASP_Open()..."));
        rc = BASP_Open(
                &g_NEXUS_asp.hAspBaseModule,
                g_pCoreHandles->chp,
                g_pCoreHandles->reg,
                NEXUS_Heap_GetMmaHandle(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus),
                g_pCoreHandles->bint,
                g_pCoreHandles->tmr,
                &aspOpenSettings
                );
        if (rc) {rc=BERR_TRACE(rc); goto error;}
        BDBG_MSG(("BASP_Open() successful, hAspBaseModule=%p", (void *)g_NEXUS_asp.hAspBaseModule));
    }

    /* Create a Streaming Context. */
    {
        BASP_ContextCreateSettings createSettings;

        BDBG_MSG(("Calling BASP_Context_Create() for Streaming type Context..."));
        createSettings.type = BASP_ContextType_eStreaming;
        rc = BASP_Context_Create(g_NEXUS_asp.hAspBaseModule, &createSettings, &g_NEXUS_asp.hContext);
        if (rc) {rc=BERR_TRACE(rc); goto error;}
        BDBG_MSG(("BASP_Context_Create() was successful. hContext=%p", (void *)g_NEXUS_asp.hContext));
    }

    /* Do Network Switch & System Port (gphy) setup to enable receiving of ASP Streamed packets. */
#if 0
    {
        checkRegContent64(g_pCoreHandles->reg, ">>>>>>>>>>>>>>>>>>> EDPKT_HdrRngBufWr", 0x1841110, 0);
        checkRegContent64(g_pCoreHandles->reg, ">>>>>>>>>>>>>>>>>>> EDPKT_HdrRngBufValid", 0x1841118, 0);
        checkRegContent64(g_pCoreHandles->reg, ">>>>>>>>>>>>>>>>>>> EDPKT_HdrRngBufRead", 0x1841120, 0);
    }
#endif

#if (BCHP_CHIP==7278 && BCHP_VER==A0)
    /* Disabling this workaround as it is now being set via the HW init info file. */
    {
        unsigned value;
        unsigned offset;
        int i;

        /* A0 Work around for ASP MuxController hang due to the backpressure from Security. */
        /* We have to set all NS pipe registers from 0 to 5. */
        /* e.g. set PIPE_ORDER bit (17) in BCHP_XPT_SECURITY_NS_PIPE0_CTRL. */
        offset = 0x3b4200;
        for (i=0; i<6; i++)
        {
            value = BREG_Read32(g_pCoreHandles->reg, offset+i*4);
            value |= (1<<17); /* Set PIPE_ORDER bit */
            BREG_Write32(g_pCoreHandles->reg, offset+i*4, value);
        }
    }
#endif

#if 0
    /* Temp code to disable MCPB Mux Controller. */
    {
        BREG_Write32(g_pCoreHandles->reg, 0x186bc10 /* ASP_MCPB_ASP_MCPB_GLOBAL_ASP_MCPB_GLOBAL_MUX_CTRL1 */, 1);
    }
#endif

    /* TODO: temp code. */
    {
        const char *pEnvVar;
        int envVarValue;

        if ( (pEnvVar = NEXUS_GetEnv("basp_enableTcpRetx")) != NULL)
        {
            envVarValue = NEXUS_atoi(pEnvVar);
            g_enableTcpRetrans = envVarValue ? true:false;
        }
        if ( (pEnvVar = NEXUS_GetEnv("basp_enableTcpCc")) != NULL)
        {
            envVarValue = NEXUS_atoi(pEnvVar);
            g_enableTcpCongestionControl = envVarValue ? true:false;
        }
        if ( (pEnvVar = NEXUS_GetEnv("basp_enableTcpTs")) != NULL)
        {
            envVarValue = NEXUS_atoi(pEnvVar);
            g_enableTcpTimestamps = envVarValue ? true:false;
        }
        if ( (pEnvVar = NEXUS_GetEnv("basp_enableTcpSack")) != NULL)
        {
            envVarValue = NEXUS_atoi(pEnvVar);
            g_enableTcpSack = envVarValue ? true:false;
            if (g_enableTcpSack) system("echo 1 > /proc/sys/net/ipv4/tcp_sack"); else system("echo 0 > /proc/sys/net/ipv4/tcp_sack");
        }
        if ( (pEnvVar = NEXUS_GetEnv("basp_enableAch")) != NULL)
        {
            envVarValue = NEXUS_atoi(pEnvVar);
            g_enableAch = envVarValue ? true:false;
        }

        BDBG_WRN(("******************************************************"));
        BDBG_WRN(("******************************************************"));
        BDBG_WRN(("Note: export following environment variables for enabling different TCP features"));
        BDBG_WRN(("TCP Retransmissions:         export basp_enableTcpRetx=1"));
        BDBG_WRN(("TCP Congestions Control:     export basp_enableTcpCc=1"));
        BDBG_WRN(("TCP Timestamps:              export basp_enableTcpTs=1"));
        BDBG_WRN(("TCP Selective ACKs:          export basp_enableTcpSack=1"));
        BDBG_WRN(("TCP Features: Timestamps=%s CongestionControl=%s Retx=%s, Sacks=%s",
                    g_enableTcpTimestamps?"Y":"N",
                    g_enableTcpCongestionControl?"Y":"N",
                    g_enableTcpRetrans?"Y":"N",
                    g_enableTcpSack?"Y":"N"
                 ));
        BDBG_WRN(("******************************************************"));
        BDBG_WRN(("******************************************************"));
    }

    {
        /* Configure Switch */
        BDBG_LOG(("Configuring Network Switch.."));
        configSwitch(g_pCoreHandles->reg);
    }

#if 0
    /* Test code: take it out after SAGE ASP Module is brought up. */
    {
        int lengthInDwords = 2030;
        unsigned offset;
        unsigned value;
        int i;

        offset = 0x18f00a4;
        BREG_Write32(g_pCoreHandles->reg, offset, 0x1);

        offset = 0x18f9040;
#define TEST_PATTERN 0x1fffff00
        BDBG_WRN((">>>>>>>>>>>>>>>>>>>>>>>>>> Writing test patter=%x to SAGE->ASP Msg Buffer", TEST_PATTERN));
        for (i=0; i<lengthInDwords; i++)
        {
            BDBG_WRN(("i=%d offset=0x%x pattern=0x%x", i, offset+i*4, TEST_PATTERN));
            BREG_Write32(g_pCoreHandles->reg, offset+i*4, TEST_PATTERN);
        }
        BDBG_WRN((">>>>>>>>>>>>>> Reading test patter=%x to SAGE->ASP Msg Buffer", TEST_PATTERN));
        for (i=0; i<lengthInDwords; i++)
        {
            value = BREG_Read32(g_pCoreHandles->reg, offset+i*4);
            if (value != TEST_PATTERN) BDBG_WRN((">>>>>>>>>>>>>>>> Mismatch at offset=%X, value=0x%x", offset+i*4, value));
        }
        BDBG_WRN((">>>>>>>>>>>>>>>>>>>>>>>>>> Verified test patter=%x to SAGE->ASP Msg Buffer", TEST_PATTERN));
    }

    {
        int lengthInDwords = 2010;
        unsigned offset;
        unsigned value;
        int i;

        offset = 0x18f00a4;
        BREG_Write32(g_pCoreHandles->reg, offset, 0x1);

        offset = 0x18fd080;
        BDBG_WRN((">>>>>>>>>>>>>>>>>>>>>>>>>> Writing test patter=%x to ASP->SAGE Msg Buffer", TEST_PATTERN));
        for (i=0; i<lengthInDwords; i++)
        {
            BDBG_WRN(("i=%d offset=0x%x pattern=0x%x", i, offset+i*4, TEST_PATTERN));
            BREG_Write32(g_pCoreHandles->reg, offset+i*4, TEST_PATTERN);
        }
        BDBG_WRN((">>>>>>>>>>>>>> Reading test patter=%x to ASP->SAGE Msg Buffer", TEST_PATTERN));
        for (i=0; i<lengthInDwords; i++)
        {
            value = BREG_Read32(g_pCoreHandles->reg, offset+i*4);
            if (value != TEST_PATTERN) BDBG_WRN((">>>>>>>>>>>>>>>> Mismatch at offset=%X, value=0x%x", offset+i*4, value));
        }
        BDBG_WRN((">>>>>>>>>>>>>>>>>>>>>>>>>> Verified test patter=%x to ASP->SAGE Msg Buffer", TEST_PATTERN));
    }
#endif

    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
    return g_NEXUS_aspModule;
error:
    NEXUS_AspModule_Uninit();
    return NULL;
}

void NEXUS_AspModule_Uninit()
{
    if (g_NEXUS_asp.hContext)
    {
        BASP_Context_Destroy(g_NEXUS_asp.hContext);
        g_NEXUS_asp.hContext = NULL;
    }

    if (g_NEXUS_asp.hAspBaseModule)
    {
        BASP_Close(g_NEXUS_asp.hAspBaseModule);
        g_NEXUS_asp.hAspBaseModule = NULL;
    }
    if (g_NEXUS_asp.statusBuffer.offset)
    {
        unLockAndFreeBuffer(&g_NEXUS_asp.statusBuffer);
        g_NEXUS_asp.statusBuffer.offset = 0;
    }

    NEXUS_Module_Destroy(g_NEXUS_aspModule);
    g_NEXUS_aspModule = NULL;
}

/******************** ASP Channel Specific API functions ***************/


void NEXUS_AspChannel_GetDefaultCreateSettings(NEXUS_AspChannelCreateSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->reTransmitFifo.size = 1 * 1024 * 1024;   /* 1MB of Default Retransmission Queue. */
    pSettings->receiveFifo.size = 1024 * 1024/2;        /* 1/2MB of Default Read Queue. */
    pSettings->m2mDmaDescBuffer.size = 64*256;          /* 256 Descriptors each of 64B in size. */
    pSettings->reassembledFifo.size = 64 * 1024;        /* 1/2MB of Default Re-assembled Queue. */
    pSettings->writeFifo.size = 4 * 1024;               /* 4KB of Default Write Queue. */
    pSettings->miscBuffer.size = 4 * 1024;              /* 4KB of Default Write Queue. */
}

NEXUS_AspChannelHandle NEXUS_AspChannel_Create(
    const NEXUS_AspChannelCreateSettings *pSettings
    )
{
    int                     channelNum;
    NEXUS_Error             rc;
    NEXUS_AspChannelHandle  hAspChannel = NULL;

    /* Check if we have a free ASP Channel, return error otherwise. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspChannelList[channelNum] == NULL)
        {
            break;
        }
    }
    if (channelNum == BASP_MAX_NUMBER_OF_CHANNEL)
    {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        BDBG_WRN(("%s: No free ASP Channel available, max=%d", BSTD_FUNCTION, BASP_MAX_NUMBER_OF_CHANNEL));
        return NULL;
    }
#if 0
    /* TODO: work around for channel change issue for DTCP/IP. Currently, FW is not properly closing the channel for A0. */
    {
        static int chNum=0;
        channelNum = chNum;
        chNum++;
        if (chNum >= BASP_MAX_NUMBER_OF_CHANNEL) chNum = 0;
    }
#endif

#if 0
    /* TODO: need to add this when adding Multi-Process support. */
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(asp, Count, NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }
#endif

    hAspChannel = BKNI_Malloc(sizeof(*hAspChannel));
    if (!hAspChannel)
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_AspChannel, hAspChannel);
    g_NEXUS_asp.hAspChannelList[channelNum] = hAspChannel;
    hAspChannel->channelNum = channelNum;

    if (!pSettings)
    {
        NEXUS_AspChannel_GetDefaultCreateSettings(&hAspChannel->createSettings);
    }
    else
    {
        hAspChannel->createSettings = *pSettings;
    }

    /* Allocate Per Channel Context Memory. All of these memory blocks are for ASP FW/HW usage. */
    {

        rc = allocateAndLockBuffer(&pSettings->reTransmitFifo, false /*needCpuAccess*/, BASP_EPKT_RETX_BUFFER_ALIGNMENT_IN_BYTES, &hAspChannel->reTransmitFifo);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        rc = allocateAndLockBuffer(&pSettings->miscBuffer, false /*needCpuAccess*/, BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES, &hAspChannel->miscBuffer);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        rc = allocateAndLockBuffer(&pSettings->receiveFifo, true /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES, &hAspChannel->receiveFifo); /* TODO: see if we can use a different rcv buffer for receiving HTTP Response as this wastes the virtual memory for all of the Rx buffer where as we only need it for the initial Response part. */
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        rc = allocateAndLockBuffer(&pSettings->writeFifo, true /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES, &hAspChannel->writeFifo);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        rc = allocateAndLockBuffer(&pSettings->m2mDmaDescBuffer, false /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES, &hAspChannel->m2mDmaDescBuffer);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
    }

    /* Create a Streaming Channel. */
    {
        BASP_ChannelCreateSettings createSettings;

        BASP_Channel_GetDefaultCreateSettings(&createSettings);
        BDBG_MSG(("Calling BASP_Channel_Create()..."));
        createSettings.channelNumber = channelNum;
        rc = BASP_Channel_Create(g_NEXUS_asp.hContext, &createSettings, &hAspChannel->hChannel);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        BDBG_MSG(("BASP_Channel_Create() was successful. hChannel=%p", (void *)hAspChannel->hChannel));
    }

    hAspChannel->state = NEXUS_AspChannelState_eIdle;
    BDBG_MSG(("%s: hAspChannel=%p BASP hChannel=%p channelNumber=%d", BSTD_FUNCTION, (void *)hAspChannel, (void *)hAspChannel->hChannel, hAspChannel->channelNum));

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    BDBG_MSG(("%s : %d Using NEXUS TIMER instead of interrupts", BSTD_FUNCTION, __LINE__ ));

    if (!g_NEXUS_asp.hTimer)
    {
        g_NEXUS_asp.timerIntervalInMs = 100;
        BDBG_MSG(("%s : %d Starting NEXUS TIMER for %u ms\n", BSTD_FUNCTION, __LINE__, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hTimer);
    }
#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    BDBG_MSG(("%s : %d Using BASP INTERRUPTS\n", BSTD_FUNCTION, __LINE__ ));
    /* Create a Streaming Channel. */
    {
        BASP_ChannelCallbacks callbacks;

        BASP_Channel_GetCallbacks(hAspChannel->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NEXUS_ProcessMsgFromFwCallback_isr;
        callbacks.messageReady.pParam1 = hAspChannel;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspChannel->hChannel, &callbacks);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    hAspChannel->hStateChangedCallback = NEXUS_IsrCallback_Create(hAspChannel, NULL);
    BDBG_ASSERT(hAspChannel->hStateChangedCallback);
    NEXUS_CallbackDesc_Init(&hAspChannel->settings.stateChanged);

    hAspChannel->hDataReadyCallback = NEXUS_IsrCallback_Create(hAspChannel, NULL);
    BDBG_ASSERT(hAspChannel->hDataReadyCallback);
    NEXUS_CallbackDesc_Init(&hAspChannel->settings.dataReady);

#if 0
#include "bchp_xpt_rsbuff.h"
    /* Test only: Configure XPT to disable some thruput throttling bits so that we can push thru 1Gpbs thru it. */
    {
        /* XAC Disable. */
        /* This setting has adverse affect on outgoing thruput when you use mix of wget & media player clients. */
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_PBP_XAC_EN, 0);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_IBP_XAC_EN, 0);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_CTRL_PAUSE_EN_PBP, 0);
#define BCHP_XPT_OCXC_TOP_BUF_CONFIG 0x2207000
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_OCXC_TOP_BUF_CONFIG, 0);

        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_0_31, 0xffffffff);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_32_47, 0xffffffff);
    }
#endif
    BKNI_Memset(&hAspChannel->dtcpIpSettings, 0, sizeof(hAspChannel->dtcpIpSettings));
    hAspChannel->dtcpIpSettings.pcpPayloadSize = NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK*NEXUS_ASP_BLOCK_SIZE;
    return hAspChannel;
#if 0
error:
    NEXUS_AspChannel_Destroy(hAspChannel);
    return NULL;
#endif
}

static void NEXUS_AspChannel_P_Finalizer(
    NEXUS_AspChannelHandle hAspChannel
    )
{
    BDBG_ASSERT(hAspChannel);

    NEXUS_OBJECT_ASSERT(NEXUS_AspChannel, hAspChannel);

    if (hAspChannel->hStateChangedCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspChannel->hStateChangedCallback);
        hAspChannel->hStateChangedCallback = NULL;
    }
    if (hAspChannel->hDataReadyCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspChannel->hDataReadyCallback);
        hAspChannel->hDataReadyCallback = NULL;
    }

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    /* Only cancel the timer if no other channels are active. */
    {
        int channelNum;

        for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
        {
            if (g_NEXUS_asp.hAspChannelList[channelNum]) { break; }
        }
        if (channelNum == BASP_MAX_NUMBER_OF_CHANNEL)
        {
            BDBG_WRN(("%s: No active ASP Channels, canceling the timer!", BSTD_FUNCTION));
            BDBG_ASSERT(g_NEXUS_asp.hTimer);
            NEXUS_CancelTimer(g_NEXUS_asp.hTimer);
            g_NEXUS_asp.hTimer = NULL;
        }
    }

#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    /* Disable the messageReady callbacks for this channel. */
    {
        BERR_Code               rc;
        BASP_ChannelCallbacks   callbacks;

        BDBG_MSG(("%s: Disabling BASP messageReady callbacks for channel %p!", BSTD_FUNCTION, (void*)hAspChannel->hChannel));

        BASP_Channel_GetCallbacks(hAspChannel->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NULL;
        callbacks.messageReady.pParam1 = NULL;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspChannel->hChannel, &callbacks);

        BDBG_ASSERT(rc == NEXUS_SUCCESS);
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    BASP_Channel_Destroy(hAspChannel->hChannel);
    unLockAndFreeBuffer(&hAspChannel->reTransmitFifo);
    unLockAndFreeBuffer(&hAspChannel->receiveFifo);
    unLockAndFreeBuffer(&hAspChannel->miscBuffer);
    unLockAndFreeBuffer(&hAspChannel->m2mDmaDescBuffer);
    unLockAndFreeBuffer(&hAspChannel->writeFifo);
    g_NEXUS_asp.hAspChannelList[hAspChannel->channelNum] = NULL;
    NEXUS_OBJECT_DESTROY(NEXUS_AspChannel, hAspChannel);
    BKNI_Free(hAspChannel);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AspChannel, NEXUS_AspChannel_Destroy);

/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspChannel_GetDefaultStartSettings(
    NEXUS_AspStreamingProtocol          streamingProtocol,
    NEXUS_AspChannelStartSettings       *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->protocol = streamingProtocol;
    if (streamingProtocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pSettings->protocolSettings.http.version.major = 1;
        pSettings->protocolSettings.http.version.minor = 1;
        pSettings->protocolSettings.http.enableChunkTransferEncoding = false;
        pSettings->protocolSettings.http.chunkSize = NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK*NEXUS_ASP_BLOCK_SIZE;
    }
    else
    {
        /* Not yet supported. */
        BDBG_ASSERT(NULL);
    }
    /* TODO: initialize remaining non-zero members. */
    pSettings->mediaInfoSettings.maxBitRate = 40*1024*1024;
}


static void printStartSettings(
    const NEXUS_AspChannelStartSettings *pSettings
    )
{
    const NEXUS_AspIpSettings *pIp;

    BDBG_MSG(("%s: startSettings -->", BSTD_FUNCTION));
    BDBG_MSG(("protocol=%s transportType=%d maxBitRate=%u autoStartStreaming=%s drmType=%d",
                pSettings->protocol == NEXUS_AspStreamingProtocol_eHttp ? "HTTP" : "UDP/RTP",
                pSettings->mediaInfoSettings.transportType,
                pSettings->mediaInfoSettings.maxBitRate,
                pSettings->autoStartStreaming ? "Y":"N",
                pSettings->drmType
                ));

    if (pSettings->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        BDBG_MSG(("HTTP: version=%d.%d enableChunkTransferEncoding=%s chunkSize=%u " ,
                    pSettings->protocolSettings.http.version.major,
                    pSettings->protocolSettings.http.version.minor,
                    pSettings->protocolSettings.http.enableChunkTransferEncoding?"Y":"N",
                    pSettings->protocolSettings.http.chunkSize
                 ));
        BDBG_MSG(("TCP: localPort=%d remotePort=%d initialSendSequenceNumber=0x%x initialRecvSequenceNumber0x%x currentAckedNumber=0x%x",
                    pSettings->protocolSettings.http.tcp.localPort,
                    pSettings->protocolSettings.http.tcp.remotePort,
                    pSettings->protocolSettings.http.tcp.initialSendSequenceNumber,
                    pSettings->protocolSettings.http.tcp.initialRecvSequenceNumber,
                    pSettings->protocolSettings.http.tcp.currentAckedNumber
                 ));
        BDBG_MSG(("TCP: maxSegmentSize=%u windowScaleValue local=%u remote=%u remoteWindowSize=%u enableTimeStamps=%s timestampEchoValue=%u enableSack=%s",
                    pSettings->protocolSettings.http.tcp.maxSegmentSize,
                    pSettings->protocolSettings.http.tcp.localWindowScaleValue,
                    pSettings->protocolSettings.http.tcp.remoteWindowScaleValue,
                    pSettings->protocolSettings.http.tcp.remoteWindowSize,
                    pSettings->protocolSettings.http.tcp.enableTimeStamps?"Y":"N",
                    pSettings->protocolSettings.http.tcp.timestampEchoValue,
                    pSettings->protocolSettings.http.tcp.enableSack?"Y":"N"
                 ));
    }
    if (pSettings->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pIp = &pSettings->protocolSettings.http.tcp.ip;
    }
    else if (pSettings->protocol == NEXUS_AspStreamingProtocol_eUdp)
    {
        pIp = &pSettings->protocolSettings.udp.ip;
    }
    else if (pSettings->protocol == NEXUS_AspStreamingProtocol_eRtp)
    {
        pIp = &pSettings->protocolSettings.rtp.udp.ip;
    }
    else
    {
        pIp = &pSettings->protocolSettings.tcp.ip;
    }

    if (pIp->ipVersion == NEXUS_AspIpVersion_e4)
    {
        BDBG_MSG(("IP: localIpAddr=%d.%d.%d.%d remoteIpAddr=%d.%d.%d.%d dscp=%d ecn=%u initialIdentification=%u timeToLive=%u",
                    pIp->ver.v4.localIpAddr>>24,
                    pIp->ver.v4.localIpAddr>>16 & 0xff,
                    pIp->ver.v4.localIpAddr>>8 & 0xff,
                    pIp->ver.v4.localIpAddr & 0xff,
                    pIp->ver.v4.remoteIpAddr>>24,
                    pIp->ver.v4.remoteIpAddr>>16 & 0xff,
                    pIp->ver.v4.remoteIpAddr>>8 & 0xff,
                    pIp->ver.v4.remoteIpAddr & 0xff,
                    pIp->ver.v4.dscp,
                    pIp->ver.v4.ecn,
                    pIp->ver.v4.initialIdentification,
                    pIp->ver.v4.timeToLive
                 ));
    }
    {
        BDBG_MSG(("Eth: localMacAddress=%2x:%2x:%2x:%2x:%2x:%2x remoteMacAddress=%2x:%2x:%2x:%2x:%2x:%2x etherType=0x%x, vlanTag=0x%x",
                    pIp->eth.localMacAddress[0],
                    pIp->eth.localMacAddress[1],
                    pIp->eth.localMacAddress[2],
                    pIp->eth.localMacAddress[3],
                    pIp->eth.localMacAddress[4],
                    pIp->eth.localMacAddress[5],
                    pIp->eth.remoteMacAddress[0],
                    pIp->eth.remoteMacAddress[1],
                    pIp->eth.remoteMacAddress[2],
                    pIp->eth.remoteMacAddress[3],
                    pIp->eth.remoteMacAddress[4],
                    pIp->eth.remoteMacAddress[5],
                    pIp->eth.etherType, pIp->eth.vlanTag
                    ));

        BDBG_MSG(("Switch: queueNumber=%u ingressBrcmTag=0x%x egressClassId=%u",
                    pIp->eth.networkSwitch.queueNumber,
                    pIp->eth.networkSwitch.ingressBrcmTag,
                    pIp->eth.networkSwitch.egressClassId
                 ));
    }
} /* printStartSettings */

#ifdef NEXUS_HAS_SECURITY
static BHSM_KeyslotHandle allocAndConfigKeySlot(
    NEXUS_AspChannelHandle  hAspChannel
    )
{
    BERR_Code rc;
    BHSM_KeyslotAllocateSettings keySlotSettings;
    BHSM_KeyslotHandle hKeySlot;
    BHSM_KeyslotSettings hsmKeyslotSettings;
    BHSM_KeyslotEntrySettings hsmEntrySettings;
    BHSM_KeyslotExternalKeyData extKeyTableData;

    {
        BHSM_Handle hHsm;

        LOCK_SECURITY();
        NEXUS_Security_GetHsm_priv(&hHsm);
        BHSM_Keyslot_GetDefaultAllocateSettings(&keySlotSettings);
        keySlotSettings.owner = BHSM_SecurityCpuContext_eHost;
        keySlotSettings.slotType = BHSM_KeyslotType_eIvPerBlock;
        keySlotSettings.useWithDma = false;
        hKeySlot = BHSM_Keyslot_Allocate( hHsm, &keySlotSettings );
        BDBG_ASSERT(hKeySlot);
        UNLOCK_SECURITY();
    }

#define ASP_PID_CHANNEL_BASE 1024
    rc = BHSM_Keyslot_AddPidChannel( hKeySlot, ASP_PID_CHANNEL_BASE + hAspChannel->channelNum );
    BDBG_ASSERT(rc == 0);

    BKNI_Memset( &hsmKeyslotSettings, 0, sizeof(hsmKeyslotSettings) );
    BHSM_Keyslot_GetSettings( hKeySlot, &hsmKeyslotSettings);
    hsmKeyslotSettings.regions.source[0] = true;
    hsmKeyslotSettings.regions.destinationRPipe[0] = true;
    hsmKeyslotSettings.regions.destinationGPipe[0] = true;
    rc = BHSM_Keyslot_SetSettings( hKeySlot, &hsmKeyslotSettings);
    BDBG_ASSERT(rc == 0);

    BKNI_Memset( &hsmEntrySettings, 0, sizeof(hsmEntrySettings) );
    BHSM_Keyslot_GetEntrySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &hsmEntrySettings);
    hsmEntrySettings.algorithm         = BHSM_CryptographicAlgorithm_eAes128;
    hsmEntrySettings.algorithmMode     = BHSM_CryptographicAlgorithmMode_eCbc;
    hsmEntrySettings.terminationMode   = BHSM_Keyslot_TerminationMode_eClear;
    hsmEntrySettings.external.key      = true;
    hsmEntrySettings.external.iv       = true;
    hsmEntrySettings.rPipeEnable       = true;
    hsmEntrySettings.gPipeEnable       = true;
    rc = BHSM_Keyslot_SetEntrySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &hsmEntrySettings);
    BDBG_ASSERT(rc == 0);

    rc = BHSM_Keyslot_GetEntryExternalKeySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &extKeyTableData);
    BDBG_ASSERT(rc == 0);
    hAspChannel->extKeyTableSlotIndex = extKeyTableData.slotIndex;

    BDBG_LOG(("%s: Done: extKeyTableSlotIndex=%u", BSTD_FUNCTION, hAspChannel->extKeyTableSlotIndex));
    return (hKeySlot);
} /* allocAndConfigKeySlot */
#endif

/**
Summary:
Start a AspChannel.
Note: it will only starts streaming if NEXUS_AspChannelStartSettings.autoStartStreaming flag is set to true!
**/
NEXUS_Error NEXUS_AspChannel_Start(
    NEXUS_AspChannelHandle                  hAspChannel,
    const NEXUS_AspChannelStartSettings     *pSettings
    )
{
    NEXUS_Error                             nrc;
    BASP_Pi2Fw_Message                      msg;
    BASP_ChannelStartStreamOutMessage       *pStreamOut = NULL;
    BASP_ChannelStartStreamInMessage        *pStreamIn = NULL;
    BASP_ConnectionControlBlock             *pCcb = NULL;
    const NEXUS_AspIpSettings               *pIp = NULL;
    const NEXUS_AspTcpSettings              *pTcp = NULL;
    const NEXUS_AspEthernetSettings         *pEth = NULL;
    static bool                             fwTimestampSynced = false;

    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pSettings);

    printStartSettings(pSettings);
    hAspChannel->startSettings = *pSettings;

    BKNI_Memset(&msg, 0, sizeof(msg));

    /* TODO: check for if (pSettings->autoStartStreaming) */

    /* Build Start Message. */
    if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
    {
        /* Build message header fields, others are filled-in by the ASP basemodules. */
        msg.MessageHeader.MessageType = BASP_MessageType_ePi2FwChannelStartStreamOut;
        msg.MessageHeader.ResponseType = BASP_ResponseType_eAckRespRequired;

        /* Build message payload. */
        pStreamOut = &msg.MessagePayload.ChannelStartStreamOut;

        BDBG_MSG(("%s: !!!!!! TCP Retransmissions is %s!", BSTD_FUNCTION, g_enableTcpRetrans?"enabled":"disabled"));
        pStreamOut->ui32RetransmissionEnable = g_enableTcpRetrans;
        pStreamOut->ReTransmissionBuffer.ui32BaseAddrLo = (uint32_t)(hAspChannel->reTransmitFifo.offset & 0xFFFFFFFF);
        pStreamOut->ReTransmissionBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->reTransmitFifo.offset>>32);
        pStreamOut->ReTransmissionBuffer.ui32Size = (uint32_t)hAspChannel->reTransmitFifo.size;

        pStreamOut->EthernetHeaderBuffer.ui32BaseAddrLo = (uint32_t)hAspChannel->miscBuffer.offset;
        pStreamOut->EthernetHeaderBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->miscBuffer.offset>>32);
        pStreamOut->EthernetHeaderBuffer.ui32Size = (uint32_t)hAspChannel->miscBuffer.size;

        pStreamOut->ReceivePayloadBuffer.ui32BaseAddrLo = (uint32_t)(hAspChannel->receiveFifo.offset & 0xFFFFFFFF);
        pStreamOut->ReceivePayloadBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->receiveFifo.offset>>32);
        pStreamOut->ReceivePayloadBuffer.ui32Size = (uint32_t)hAspChannel->receiveFifo.size;

#if 0
        pStreamOut->HttpResponseBuffer.ui32BaseAddrLo =
        pStreamOut->HttpResponseBuffer.ui32BaseAddrHi =
        pStreamOut->HttpResponseBuffer.ui32Size =
#endif
        if (pSettings->protocolSettings.http.enableChunkTransferEncoding)
        {
            pStreamOut->ui32HttpType = BASP_HttpConnectionType_e11WithChunking;
            pStreamOut->ui32ChunkSize = pSettings->protocolSettings.http.chunkSize/NEXUS_ASP_BLOCK_SIZE;
        }
        else
        {
            pStreamOut->ui32HttpType = BASP_HttpConnectionType_e11WithoutChunking;
        }
        pStreamOut->ui32CongestionFlowControlEnable = 1 << BASP_DEBUG_HOOK_DUPLICATE_ACK_COND_BIT;
#if 1
        if (g_enableTcpCongestionControl)
        {
            BDBG_MSG(("%s: !!!!!! Enabling TCP Congestion Control Algorithm: Slow Start & Congestion Avoidance!", BSTD_FUNCTION));
            pStreamOut->ui32CongestionFlowControlEnable |= 1 << BASP_DEBUG_HOOK_CONGESTION_CONTROL_ENABLE_BIT;
        }
#endif
        pCcb = &pStreamOut->ConnectionControlBlock;

        /* Get RAVE CDB offset corresponding to the Recpump associated with this AspChannel (passed by the caller!). */
        {
            NEXUS_RecpumpStatus     status;
            uint64_t raveContextBaseAddress;

            BDBG_ASSERT(pSettings->modeSettings.streamOut.recpump);
            nrc = NEXUS_Recpump_GetStatus(pSettings->modeSettings.streamOut.recpump, &status);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);

            raveContextBaseAddress = BASP_CVT_TO_ASP_REG_ADDR( BCHP_PHYSICAL_OFFSET + BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR +
                                          ((BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR) * status.rave.index));

            pStreamOut->ui32RaveContextBaseAddressLo = (uint32_t) (raveContextBaseAddress & 0xFFFFFFFF);
            pStreamOut->ui32RaveContextBaseAddressHi = (uint32_t) (raveContextBaseAddress >> 32);
            BDBG_MSG(("%s: hAspChannel=%p hRecpump=%p rave_ctx#=%d, baseAddr low=0x%x high=0x%x ", BSTD_FUNCTION,
                        (void *)hAspChannel, (void *)pSettings->modeSettings.streamOut.recpump, status.rave.index,
                        pStreamOut->ui32RaveContextBaseAddressLo, pStreamOut->ui32RaveContextBaseAddressHi
                        ));
        }

        /* Fill-in the ASP MCPB related defaults for this Streaming out channel. */
        {
            BASP_McpbStreamOutConfig    *pMcpb = &pStreamOut->McpbStreamOutConfig;

            /* TODO: ATS: need to reconfigure bunch of these settings when 188 + 4 byte of Timestamps are present in the stream. */
            pMcpb->ASPPids.ui32NumASPPids = 1;              /* PID filtering is done at the XPT MCPB Config level, ASP MCPB will pass-thru all AV data from RAVE. Thus the value of 1. */
            pMcpb->ASPPids.aPidList[0].ui32ProgramType = 0;
            pMcpb->ASPPids.aPidList[0].ui32PidChannel = BXPT_P_PID_TABLE_SIZE + hAspChannel->channelNum;
            pMcpb->ASPPids.aPidList[0].ui32PidValue = 0;    /* All pass case. PID filtering is done by the XPT MCPB. */
            pMcpb->ui32PacingType = 0;  /* Pacing should happen at the XPT MCPB level, so this is just for debugging. */
            pMcpb->ui32SrcTimestampType = BASP_TransportTimeStampType_e302uMod300;
            /* TODO: this is a good option to enable if we have to see output timestamps. */
            pMcpb->ui32ForceRestamping = 1;
            pMcpb->ui32PcrPacingPid = 0;        /* N/A, again happens at the XPT MCPB level. */
            pMcpb->ui32ParityCheckDisable = 0;  /* N/A, again happens at the XPT MCPB level. */
            pMcpb->ui32AvgStreamBitrate = pSettings->mediaInfoSettings.maxBitRate;
            pMcpb->ui32TimebaseIndex = 0;
            pMcpb->ui32ParserAllPassControl = 1;
#if 0
            pMcpb->ui32ParserStreamType = BASP_ParserStreamType_eMpeg;
            pMcpb->ui32PacketLen = 188;
#else
            /* Note: switching to Block mode for all container formats as DTCP/IP requires it. */
            pMcpb->ui32ParserStreamType = BASP_ParserStreamType_eBlock;
            pMcpb->ui32PacketLen = 192;
#endif
            pMcpb->ui32McpbTmeuErrorBoundLate = 0;
            /* TODO: Missing fields: outputAtsMode. */
        }
    }
    else if (pSettings->mode == NEXUS_AspStreamingMode_eIn)
    {
        /* Build message header fields, others are filled-in by the ASP basemodules. */
        msg.MessageHeader.MessageType = BASP_MessageType_ePi2FwChannelStartStreamIn;
        msg.MessageHeader.ResponseType = BASP_ResponseType_eAckRespRequired;

        /* Build message payload. */
        pStreamIn = &msg.MessagePayload.ChannelStartStreamIn;

        pStreamIn->ui32SendRstPktOnRetransTimeOutEnable = 0;
        pStreamIn->ui32KeepAliveTimerEnable = 1;
        pStreamIn->ui32DrmEnabled = 0;

        {
            NEXUS_PlaypumpStatus     status;

            BDBG_ASSERT(pSettings->modeSettings.streamIn.playpump);
            nrc = NEXUS_Playpump_GetStatus(pSettings->modeSettings.streamIn.playpump, &status);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);

            pStreamIn->ui32PlaybackChannelNumber = status.index;
        }
#define NEXUS_ASP_BASE_PID_CHANNEL_NUMBER 1024
        pStreamIn->ui32PidChannel = NEXUS_ASP_BASE_PID_CHANNEL_NUMBER + hAspChannel->channelNum;

        BDBG_MSG(("%s:%p !!!!!! Setting up stream-in channel: pbChannel#=%u pidCh#=%u", BSTD_FUNCTION,
                    (void *)hAspChannel, pStreamIn->ui32PlaybackChannelNumber, pStreamIn->ui32PidChannel));
        /* Setup initial data: HTTP Get Request for StreamIn case. */
        pStreamIn->HttpRequestBuffer.ui32BaseAddrLo = (uint32_t) (hAspChannel->writeFifo.offset & 0xFFFFFFFF);
        pStreamIn->HttpRequestBuffer.ui32BaseAddrHi = (uint32_t) (hAspChannel->writeFifo.offset >> 32);
        pStreamIn->HttpRequestBuffer.ui32Size = hAspChannel->writeFifoLength;

        /* TODO: take the retransmit buffer out. */
        pStreamIn->ReTransmissionBuffer.ui32BaseAddrLo = (uint32_t)(hAspChannel->reTransmitFifo.offset & 0xFFFFFFFF);
        pStreamIn->ReTransmissionBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->reTransmitFifo.offset>>32);
        pStreamIn->ReTransmissionBuffer.ui32Size = (uint32_t)hAspChannel->reTransmitFifo.size;

        pStreamIn->EthernetHeaderBuffer.ui32BaseAddrLo = (uint32_t)hAspChannel->miscBuffer.offset;
        pStreamIn->EthernetHeaderBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->miscBuffer.offset>>32);
        pStreamIn->EthernetHeaderBuffer.ui32Size = (uint32_t)hAspChannel->miscBuffer.size;

        pStreamIn->ReceivePayloadBuffer.ui32BaseAddrLo = (uint32_t)(hAspChannel->receiveFifo.offset & 0xFFFFFFFF);
        pStreamIn->ReceivePayloadBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->receiveFifo.offset>>32);
        pStreamIn->ReceivePayloadBuffer.ui32Size = (uint32_t)hAspChannel->receiveFifo.size;

        pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrLo = (uint32_t)(hAspChannel->m2mDmaDescBuffer.offset & 0xFFFFFFFF);
        pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrHi = (uint32_t)(hAspChannel->m2mDmaDescBuffer.offset>>32);
        pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32Size = (uint32_t)hAspChannel->m2mDmaDescBuffer.size;

        pStreamIn->ui32AvgStreamBitrate = pSettings->mediaInfoSettings.maxBitRate;
        pCcb = &pStreamIn->ConnectionControlBlock;
    }
    else
    {
        BDBG_WRN(("%s: non-HTTP protocols are not yet supported!!!", BSTD_FUNCTION));
        BDBG_ASSERT(NULL);
    }

    /* Fill-in connection control block. */
    {
        /* Determine the pointers to various headers so that we can use them independent of the protocol. */
        if (pSettings->protocol == NEXUS_AspStreamingProtocol_eHttp)
        {
            pEth = &pSettings->protocolSettings.http.tcp.ip.eth;
            pIp = &pSettings->protocolSettings.http.tcp.ip;
            pTcp = &pSettings->protocolSettings.http.tcp;
        }
        else
        {
            BDBG_WRN(("%s: non-HTTP protocols are not yet supported!!!", BSTD_FUNCTION));
            BDBG_ASSERT(NULL);
        }

        /* Ethernet & Network Switch related settings: needed for all protocols. */
        {
            if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
            {
                BKNI_Memcpy(pCcb->aui8SrcMacAddr, pEth->localMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
                BKNI_Memcpy(pCcb->aui8DestMacAddr, pEth->remoteMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
            }
            else
            {
                BKNI_Memcpy(pCcb->aui8DestMacAddr, pEth->localMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
                BKNI_Memcpy(pCcb->aui8SrcMacAddr, pEth->remoteMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
            }
            pCcb->ui32EtherType = pEth->etherType;
            pCcb->ui32IngressBrcmTag = pEth->networkSwitch.ingressBrcmTag;
            pCcb->ui32EgressClassId = pEth->networkSwitch.egressClassId;
        }

        /* IP header related settings: needed for all protocols. */
        {
            pCcb->ui32IpVersion = pIp->ipVersion == NEXUS_AspIpVersion_e6 ? 6 : 4;
            if (pIp->ipVersion == NEXUS_AspIpVersion_e4)
            {
                pCcb->ui32Dscp = pIp->ver.v4.dscp;
                pCcb->ui32TimeToLive = pIp->ver.v4.timeToLive;
#if TODO
                /* FW currently doesn't define & use the protocol type & defaults it to TCP only. */
                /* Update this when this enum gets defined. */
                pCcb->ui32ProtocolType = pSettings->protocol;
#endif
                if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
                {
                    pCcb->ai32SrcIpAddr[0] = pIp->ver.v4.localIpAddr;
                    pCcb->ai32DestIpAddr[0] = pIp->ver.v4.remoteIpAddr;
                }
                else
                {
                    pCcb->ai32DestIpAddr[0] = pIp->ver.v4.localIpAddr;
                    pCcb->ai32SrcIpAddr[0] = pIp->ver.v4.remoteIpAddr;
                }
            }
        }

        /* Set TCP header related settings if being used. */
        if (pTcp)
        {
            if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
            {
                pCcb->ui32SrcPort = pTcp->localPort;
                pCcb->ui32DestPort = pTcp->remotePort;
            }
            else
            {
                pCcb->ui32DestPort = pTcp->localPort;
                pCcb->ui32SrcPort = pTcp->remotePort;
            }
            pCcb->ui32InitialSendSeqNumber = pTcp->initialSendSequenceNumber;
            pCcb->ui32InitialReceivedSeqNumber = pTcp->initialRecvSequenceNumber;
            pCcb->ui32CurrentAckedNumber = pTcp->currentAckedNumber;
            pCcb->ui32RemoteWindowSize = pTcp->remoteWindowSize;
            pCcb->ui32WindowAdvConst = 0x1000; /* TODO: hardcoded as per HW team, need to further lookinto on this value. */
            pCcb->ui32RemoteWindowScaleValue = pTcp->remoteWindowScaleValue;
            pCcb->ui32LocalWindowScaleValue = pTcp->localWindowScaleValue;
            if (g_enableTcpSack) pCcb->ui32SackEnable = pTcp->enableSack;
            pCcb->ui32TimeStampEnable = pTcp->enableTimeStamps;
            BDBG_MSG(("%s: !!!!!! TCP Timestamps are %s!", BSTD_FUNCTION, g_enableTcpTimestamps? "enabled":"disabled"));
            if (g_enableTcpTimestamps)
            {
                pCcb->ui32TimeStampEnable = 1;
                pCcb->ui32TimeStampEchoValue = pTcp->senderTimestamp;
            }
            else
            {
                pCcb->ui32TimeStampEnable = 0;
            }
            pCcb->ui32MaxSegmentSize = pTcp->maxSegmentSize;

            pCcb->ui32KaTimeout = 7200000;
            pCcb->ui32KaInterval = 75000;
            pCcb->ui32KaMaxProbes = 9;
            pCcb->ui32RetxTimeout = 1000;
            pCcb->ui32RetxMaxRetries = 13;
        }
        else
        {
            BDBG_WRN(("%s: non-TCP protocols are not yet supported!!!", BSTD_FUNCTION));
            BDBG_ASSERT(NULL);
        }
    }

    if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
    {
#if 0 /* ******************** Temporary **********************/
        pStreamOut->ui32SwitchQueueNumber = pEth->networkSwitch.queueNumber; /* TODO: enable the loss-less behavior in switch & test this. */
#else /* ******************** Temporary **********************/
        pStreamOut->ui32SwitchQueueNumber = 0;  /* Currently, ASP will output packets to the queue 0 of its port. */
#endif /* ******************** Temporary **********************/
        BDBG_MSG(("%s: !!!!!! TODO: Overwriting the ui32SwitchQueueNumber(=%d) to 0 until ACH support is enabled in ASP & Network Switch", BSTD_FUNCTION, pStreamOut->ui32SwitchQueueNumber));
        pStreamOut->ui32SwitchSlotsPerEthernetPacket = 6; /* TODO get this number from FW/HW team & also make sure it gets programmed on each switch port. */
    }
    else if (pSettings->mode == NEXUS_AspStreamingMode_eIn)
    {
#if 0 /* ******************** Temporary **********************/
        pStreamIn->ui32SwitchQueueNumber = pEth->networkSwitch.queueNumber; /* TODO: enable the loss-less behavior in switch & test this. */
#else /* ******************** Temporary **********************/
        pStreamIn->ui32SwitchQueueNumber = 0;  /* Currently, ASP will output packets to the queue 0 of its port. */
#endif /* ******************** Temporary **********************/
        BDBG_MSG(("%s: !!!!!! TODO: Overwriting the ui32SwitchQueueNumber(=%d) to 0 until ACH support is enabled in ASP & Network Switch", BSTD_FUNCTION, pStreamIn->ui32SwitchQueueNumber));
        pStreamIn->ui32SwitchSlotsPerEthernetPacket = 6; /* TODO get this number from FW/HW team & also make sure it gets programmed on each switch port. */
    }
#if 0
    BASP_Msgqueue_Log(NULL, "StartMessage", &msg);
#endif

    /* TODO:
     * FW needs to program EDPKT's initial timestamp value that Linux had used for the connection.
     * Otherwise, receiving client will drop the incoming data packets as their timestamps will not match.
     * Until FW adds this logic, we are doing this in the software.
     */
    if (pTcp->enableTimeStamps && !fwTimestampSynced)
    {
        uint64_t timestampValue;
        /* Set bit in EPKT Debug register to enable setting of the initial timestamp value. */
        BREG_Write32(g_pCoreHandles->reg, 0x18464b0, 0x2);

        /* Linux provides 32 bit timestamp value. This value needs to be programmed in the upper 32bits of the 48bit register. */
        timestampValue = (uint64_t)pTcp->senderTimestamp <<16;
        BDBG_MSG(("!!!!! TODO (move this to FW): Program starting timestamp value to EPKT: linux ts=0x%x 64bit shifted value: hi:lo=0x%x:0x%x ....",
                    pTcp->senderTimestamp, (uint32_t)(timestampValue>>32), (uint32_t)(timestampValue & 0xFFFFFFFF)));

        BREG_Write64(g_pCoreHandles->reg, 0x1846480, timestampValue ); /* Debug register to enable setting of the initial timestamp value. */
        BREG_Write32(g_pCoreHandles->reg, 0x18464b0, 0x0); /* Debug register to enable setting of the initial timestamp value. */

        /* Read the value back. */
        timestampValue = BREG_Read64(g_pCoreHandles->reg, 0x1846480); /* Debug register to enable setting of the initial timestamp value. */
        BDBG_MSG(("timestampValue hi:lo=0x%x:0x%x read back ....", (uint32_t)(timestampValue>>32), (uint32_t)(timestampValue & 0xFFFFFFFF) ));
        fwTimestampSynced = true;
    }

#ifdef NEXUS_HAS_SECURITY
    /* DTCP/IP related info. */
    if (pSettings->drmType == NEXUS_AspChannelDrmType_eDtcpIp)
    {
        if (pSettings->mode == NEXUS_AspStreamingMode_eOut)
        {
            hAspChannel->hKeySlot = allocAndConfigKeySlot( hAspChannel );
            pStreamOut->ui32DrmEnabled = true;
            pStreamOut->ui32PcpPayloadSize = hAspChannel->dtcpIpSettings.pcpPayloadSize/NEXUS_ASP_BLOCK_SIZE;
            if ( pSettings->protocolSettings.http.enableChunkTransferEncoding &&
                (pStreamOut->ui32ChunkSize < pStreamOut->ui32PcpPayloadSize ||
                pStreamOut->ui32ChunkSize % pStreamOut->ui32PcpPayloadSize) )
            {
                /* Chunk doesn't contain integral # of PCPs. FW doesn't support this right now! */
                BDBG_WRN(("!!! pcpPayloadSize=%u is NOT integral multiple of httpChunkSize=%u, making them equal as FW only supports this mode!!",
                        pStreamOut->ui32PcpPayloadSize, pStreamOut->ui32ChunkSize));
                pStreamOut->ui32PcpPayloadSize = pStreamOut->ui32ChunkSize;
            }
            else {
                BDBG_MSG(("!!! pcpPayloadSize=%u httpChunkSize=%u!!", pStreamOut->ui32PcpPayloadSize, pStreamOut->ui32ChunkSize));
            }
            pStreamOut->DtcpIpInfo.ui32EmiModes = hAspChannel->dtcpIpSettings.emiModes;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeyLabel = hAspChannel->dtcpIpSettings.exchKeyLabel;

            /* Kx[0:1:2:3] -> Key[b3:b2:b1:b0] */
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[0] =
                (hAspChannel->dtcpIpSettings.exchKey[0] << 24) |
                (hAspChannel->dtcpIpSettings.exchKey[1] << 16) |
                (hAspChannel->dtcpIpSettings.exchKey[2] <<  8) |
                (hAspChannel->dtcpIpSettings.exchKey[3] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[1] =
                (hAspChannel->dtcpIpSettings.exchKey[4] << 24) |
                (hAspChannel->dtcpIpSettings.exchKey[5] << 16) |
                (hAspChannel->dtcpIpSettings.exchKey[6] <<  8) |
                (hAspChannel->dtcpIpSettings.exchKey[7] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[2] =
                (hAspChannel->dtcpIpSettings.exchKey[8] << 24) |
                (hAspChannel->dtcpIpSettings.exchKey[9] << 16) |
                (hAspChannel->dtcpIpSettings.exchKey[10] << 8) |
                (hAspChannel->dtcpIpSettings.exchKey[11] << 0) ;

            pStreamOut->DtcpIpInfo.ui32C_A2 = 0;

            /* Nonce[0:1:2:3] -> Nc[1][b0:b1:b2:b3], Nonce[4:5:6:7] -> Nc[1][b3:b2:b1:b0] */
            pStreamOut->DtcpIpInfo.ui32Nc[0] = /* Upper 32bits of the 64bit Nonce. */
                (hAspChannel->dtcpIpSettings.initialNonce[7] << 24) |
                (hAspChannel->dtcpIpSettings.initialNonce[6] << 16) |
                (hAspChannel->dtcpIpSettings.initialNonce[5] <<  8) |
                (hAspChannel->dtcpIpSettings.initialNonce[4] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32Nc[1] = /* Lower 32bits of the 64bit Nonce. */
                (hAspChannel->dtcpIpSettings.initialNonce[3] << 24) |
                (hAspChannel->dtcpIpSettings.initialNonce[2] << 16) |
                (hAspChannel->dtcpIpSettings.initialNonce[1] <<  8) |
                (hAspChannel->dtcpIpSettings.initialNonce[0] <<  0) ;

            pStreamOut->DtcpIpInfo.ui32ASPKeySlot = hAspChannel->extKeyTableSlotIndex + 0x2; /* +2 offset for keySlot. */
        }
        else if (pSettings->mode == NEXUS_AspStreamingMode_eIn)
        {
            BDBG_ERR(("!!! FIXME: Ignoring DTCP/IP handling for Stream-in!!"));
        }
    }
#endif

#if 0
    {
        /* TODO: debug code to reset EDPKT stats upon every channel change. */
        BDBG_MSG(("%s: hAspChannel=%p Resetting the EDPKT stats...", BSTD_FUNCTION, (void *)hAspChannel));
        BREG_Write32(g_pCoreHandles->reg, BCHP_ASP_EDPKT_CORE_TEST_REG0, 0x7c);
    }
#endif

    /* Send StreamOut message to ASP. */
    {
        BERR_Code rc;
        BDBG_MSG(("%s: hAspChannel=%p Sending StartStreamOut Msg...", BSTD_FUNCTION, (void *)hAspChannel));
        rc = BASP_Channel_SendMessage( hAspChannel->hChannel,
                                        pSettings->mode == NEXUS_AspStreamingMode_eOut ? BASP_MessageType_ePi2FwChannelStartStreamOut: BASP_MessageType_ePi2FwChannelStartStreamIn,
                                        BASP_ResponseType_eAckRespRequired,
                                        &msg);
        BDBG_ASSERT(rc == BERR_SUCCESS);
        BDBG_MSG(("%s: hAspChannel=%p StartStreamOut Msg Sent ...", BSTD_FUNCTION, (void *)hAspChannel));
    }

    /* Wait for the response to the StartStreamOut message. */
    {
        for (;;)
        {
            if (hAspChannel->gotStartResponse) { break; }
            NEXUS_UnlockModule();
            BKNI_Sleep(10);
            NEXUS_LockModule();
        }
        BDBG_MSG(("%s: hAspChannel=%p StartStreamOut Msg Response Rcvd ...", BSTD_FUNCTION, (void *)hAspChannel));
    }

    /* Save initial stats as some of the stats are commulative & we need the initial values to calculate the current stats. */
    {
        updateStats(hAspChannel, g_pCoreHandles->reg, hAspChannel->channelNum, NULL, &hAspChannel->initialStatus);
        hAspChannel->currentStatus = hAspChannel->initialStatus;
    }

    /* TODO: Disable ACH logic in MCPB. */
    {
        unsigned value;
        value = BREG_Read32(g_pCoreHandles->reg, BCHP_ASP_MCPB_DIS_QUEUE_CHECK_IN_EPKT_MODE);
        value |= (1<<hAspChannel->channelNum);
        BREG_Write32(g_pCoreHandles->reg, BCHP_ASP_MCPB_DIS_QUEUE_CHECK_IN_EPKT_MODE, value);
    }

#if 0
    /* Disable TCP retransmissions: for debugging only! */
    {
        checkRegContent(g_pCoreHandles->reg, ">>>>>>>>>>>>>>>>>>> EPKT_ASP_EPKT_CORE_CH00_CH_CONFIG_MISC", 0x1844000, 0);
        checkRegContent(g_pCoreHandles->reg, ">>>>> SWITCH_CORE_TxFrameInDisc_P7", 0xf139a0, 0);
        BREG_Write32(g_pCoreHandles->reg, 0x1844000, 0x1);
    }
#endif

#if 0
    /* Configure EDPKT to receive packets w/ any BRCM tag: for debugging only! */
    {
        int i;
        for (i=0; i<256; i+=4)
        {
            BREG_Write32(g_pCoreHandles->reg, 0x1840200+i, 0);
        }
        /* Also configure ch0 to store all packet headers in the payload field. */
        {
            BREG_Write32(g_pCoreHandles->reg, 0x1840300, 0x1c6);
        }
        BDBG_WRN(("%s: !!!!!! Receiving all packets from Switch & recording them......", BSTD_FUNCTION));
    }
#endif

#if 0
    /* Temporary code to save the start message. */
    {
        FILE *fp;
        size_t bytesWritten;

        fp = fopen("./startMsg.txt", "w+b");
        BDBG_ASSERT(fp);
        bytesWritten = fwrite((void *)&msg, 1, sizeof(msg), fp);
        BDBG_ASSERT(bytesWritten == sizeof(msg));
        fflush(fp);
        fclose(fp);
    }
    {
        unsigned i;
        FILE *fp;
        size_t bytesWritten;
        uint32_t *pDword;
        unsigned numDwords;

        numDwords = sizeof(msg) / sizeof(numDwords) + (sizeof(msg) % sizeof(numDwords) ? 1 : 0);
        BDBG_WRN(("numDwords=%u", numDwords));

        fp = fopen("./streamOutMsg.txt", "w+b");
        BDBG_ASSERT(fp);
        for (i=0, pDword = (uint32_t *)&msg; i<numDwords; i++, pDword++)
        {
            fprintf(fp, "dword[%3u]=0x%x", i, *pDword);
        }
        fflush(fp);
        fclose(fp);
    }
#endif
    hAspChannel->state = NEXUS_AspChannelState_eStartedStreaming;
    return (NEXUS_SUCCESS);
}

/**
Summary:
Start data flow to Playpump or from Recpump for an AspChannel.

If NEXUS_AspChannelStartSettings.mode == NEXUS_AspChannelStreamingMode_eIn, data flows from
    Network -> NEXUS_AspCh (XPT ASP Ch) -> [NEXUS_Dma (XPT M2M DMA)] -> Playpump (XPT Playback Ch).

If NEXUS_AspChannelStartSettings.mode == NEXUS_AspChannelStreamingMode_eOut, data flows from
    NEXUS_Recpump (RAVE Ctx) ->  NEXUS_AspCh (XPT ASP Ch) -> Network.

**/
NEXUS_Error NEXUS_AspChannel_StartStreaming(
    NEXUS_AspChannelHandle              hAspChannel
    )
{
    hAspChannel->state = NEXUS_AspChannelState_eStartedStreaming;
    return (NEXUS_SUCCESS);
}



/**
Summary:
Stop Streaming flow for an AspChannel.

ASP FW will stop feeding AV Stream into XPT Playback or outof XPT Rave pipe.
**/
void NEXUS_AspChannel_StopStreaming(
    NEXUS_AspChannelHandle              hAspChannel
    )
{
    hAspChannel->state = NEXUS_AspChannelState_eStoppedStreaming;
}


/**
Summary:
Get Default StopSettings.
**/
void NEXUS_AspChannel_GetDefaultStopSettings(
    NEXUS_AspChannelStopSettings        *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}


/**
Summary:
Stop a AspChannel.

Description:
This API sends the Abort message to FW to immediately Stop NEXUS_AspChannel.
FW will NOT synchronize the protocol state (such as for TCP) in this case.
For that, caller should use NEXUS_AspChannel_Finish()
**/
void NEXUS_AspChannel_Stop(
    NEXUS_AspChannelHandle              hAspChannel
    )
{
    BERR_Code rc;
    BDBG_ASSERT(hAspChannel);

    {
        BASP_Pi2Fw_Message pi2FwMessage;
        BASP_Fw2Pi_Message fw2PiMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        rc = BASP_Channel_SendMessage(
                hAspChannel->hChannel,
                BASP_MessageType_ePi2FwChannelAbort,
                BASP_ResponseType_eAckRespRequired,
                &pi2FwMessage);
        BDBG_ASSERT(rc == BERR_SUCCESS);

        /* Wait for Abort Response. */
        for (;;)
        {
            if (hAspChannel->gotStopResponse) { break; }

            NEXUS_UnlockModule();
            BKNI_Sleep(10);
            NEXUS_LockModule();
        }

        if (hAspChannel->startSettings.protocol == NEXUS_AspStreamingProtocol_eHttp)
        {
            hAspChannel->tcpState.finalSendSequenceNumber = fw2PiMessage.Message.ResponsePayload.AbortResponse.ui32CurrentSeqNumber;
            hAspChannel->tcpState.finalRecvSequenceNumber = fw2PiMessage.Message.ResponsePayload.AbortResponse.ui32CurrentAckedNumber;
            BDBG_MSG(("%s: hAspChannel=%p Abort Response Rcvd, seq#: final=%u initial=%u bytesSent=%u, ack#: final=%u initial=%u bytesRcvd=%u", BSTD_FUNCTION,
                        (void *)hAspChannel,
                        hAspChannel->tcpState.finalSendSequenceNumber, hAspChannel->startSettings.protocolSettings.tcp.initialSendSequenceNumber,
                        hAspChannel->tcpState.finalSendSequenceNumber - hAspChannel->startSettings.protocolSettings.tcp.initialSendSequenceNumber,
                        hAspChannel->tcpState.finalRecvSequenceNumber, hAspChannel->startSettings.protocolSettings.tcp.initialRecvSequenceNumber,
                        hAspChannel->tcpState.finalRecvSequenceNumber - hAspChannel->startSettings.protocolSettings.tcp.initialRecvSequenceNumber
                     ));
        }
    }
#ifdef NEXUS_HAS_SECURITY
    if (hAspChannel->startSettings.drmType == NEXUS_AspChannelDrmType_eDtcpIp)
    {
        if (hAspChannel->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspChannel->hKeySlot );
            UNLOCK_SECURITY();
        }
        hAspChannel->hKeySlot = NULL;
    }
#endif
    hAspChannel->state = NEXUS_AspChannelState_eIdle;
}

/**
Summary:
Finish a AspChannel.

Description:
This API initiates the normal finishing of ASP Channel.
FW will synchronize the protocol state (such as for TCP) in this case.
This may involve waiting for TCP ACKs for any pending data and thus may take time.
However, the API returns after sending the message to the FW.
Its completion will be notified via the stateChangedCallback.

For immediate aborting, caller should use NEXUS_AspChannel_Stop().
**/
void NEXUS_AspChannel_Finish(
    NEXUS_AspChannelHandle              hAspChannel
    )
{
    BERR_Code rc;
    BDBG_ASSERT(hAspChannel);

    {
        BASP_Pi2Fw_Message pi2FwMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        rc = BASP_Channel_SendMessage(
                hAspChannel->hChannel,
                BASP_MessageType_ePi2FwChannelStop,
                BASP_ResponseType_eAckRespRequired,
                &pi2FwMessage);
        BDBG_ASSERT(rc == BERR_SUCCESS);

        /* Wait for Stop Response. */
        for (;;)
        {
            if (hAspChannel->gotStopResponse) { break; }

            NEXUS_UnlockModule();
            BKNI_Sleep(10);
            NEXUS_LockModule();
        }
        BDBG_MSG(("%s: hAspChannel=%p Stop Msg Response Rcvd ...", BSTD_FUNCTION, (void *)hAspChannel));
    }
#ifdef NEXUS_HAS_SECURITY
    if (hAspChannel->startSettings.drmType == NEXUS_AspChannelDrmType_eDtcpIp)
    {
        if (hAspChannel->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspChannel->hKeySlot );
            UNLOCK_SECURITY();
        }
        hAspChannel->hKeySlot = NULL;
    }
#endif
    hAspChannel->state = NEXUS_AspChannelState_eIdle;
}



/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspChannel_GetStatus(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelStatus              *pStatus     /* [out] */
    )
{
    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pStatus);

    hAspChannel->currentStatus.state = hAspChannel->state;
    updateStats(hAspChannel, g_pCoreHandles->reg, hAspChannel->channelNum, &hAspChannel->initialStatus, &hAspChannel->currentStatus);
#if 0
    determineAspHwPipeStatus(hAspChannel, g_pCoreHandles->reg, &hAspChannel->currentStatus, &pStatus->stats);
#endif
    *pStatus = hAspChannel->currentStatus;
    pStatus->aspChannelIndex = hAspChannel->channelNum;
    if (hAspChannel->startSettings.protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pStatus->tcpState.finalSendSequenceNumber = hAspChannel->tcpState.finalSendSequenceNumber;
#if 0 /* ******************** Temporary **********************/
        pStatus->tcpState.finalRecvSequenceNumber = hAspChannel->tcpState.finalRecvSequenceNumber;
#else /* ******************** Temporary **********************/
        /* TODO: until FW is fixed to return the correct Recv Seq #, we use the original value. */
        pStatus->tcpState.finalRecvSequenceNumber = hAspChannel->startSettings.protocolSettings.tcp.initialRecvSequenceNumber;
#endif /* ******************** Temporary **********************/
    }

    /* ASP FW Stats: FW periodically updates the status buffer in DRAM. */
    {
        uint32_t newValue, curValue;
        BASP_FwStatusInfo *pFwStatusInfo = g_NEXUS_asp.statusBuffer.pBuffer;
        BASP_FwChannelInfo *pFwChannelInfo;

        pFwChannelInfo = &(pFwStatusInfo->stChannelInfo[hAspChannel->channelNum]);

        pStatus->stats.fwStats.congestionWindow = pFwChannelInfo->ui32CongestionWindow;
        pStatus->stats.fwStats.receiveWindow = pFwChannelInfo->ui32ReceivedWindowSize;
        if (hAspChannel->startSettings.mode == NEXUS_AspStreamingMode_eOut)
        {
            pStatus->stats.fwStats.sendWindow = pFwChannelInfo->ui32MaxSequenceNum - pFwChannelInfo->ui32SequenceNum;
        }
        else
        {
            pStatus->stats.fwStats.sendWindow = pFwChannelInfo->ui32SendWindow;
        }
        pStatus->stats.fwStats.sendSequenceNumber = pFwChannelInfo->ui32ReceivedAckSequenceNumber;
        pStatus->stats.fwStats.rcvdAckNumber = pFwChannelInfo->ui32AckSequenceNumber;

        newValue = pFwChannelInfo->ui32NumOfIpPktsSent;
        curValue = (uint32_t)(pStatus->stats.fwStats.pktsSent & 0xFFFFFFFF);
        pStatus->stats.fwStats.pktsSent += (newValue - curValue);

        newValue = pFwChannelInfo->ui32NumReceivedPkts;
        curValue = (uint32_t)(pStatus->stats.fwStats.pktsRcvd & 0xFFFFFFFF);
        pStatus->stats.fwStats.pktsRcvd += (newValue - curValue);
        pStatus->stats.fwStats.pktsDropped = pFwChannelInfo->ui32NumPktDropped;
        pStatus->stats.fwStats.dataPktsDropped = pFwChannelInfo->ui32NumDataPktDropped;

        pStatus->stats.fwStats.pktsRetx = pFwChannelInfo->ui32NumOfTotalRetx;
        pStatus->stats.fwStats.retxSequenceNumber = pFwChannelInfo->ui32RetxSequenceNum;
    }

    pStatus->statsValid = true;
    return (NEXUS_SUCCESS);
}


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspChannel_GetDtcpIpSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelDtcpIpSettings      *pSettings /* [out] */
    )
{
    BSTD_UNUSED(hAspChannel);

    BDBG_ASSERT(pSettings);
    *pSettings = hAspChannel->dtcpIpSettings;
}

/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspChannel_Start().
**/
NEXUS_Error NEXUS_AspChannel_SetDtcpIpSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    const NEXUS_AspChannelDtcpIpSettings *pSettings
    )
{
    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pSettings);

    hAspChannel->dtcpIpSettings = *pSettings;
    {
        BDBG_MSG(("DTCP/IP Settings: pcpPayloadSize=%u exchKeyLabel=%u emiModes=%u ",
                    pSettings->pcpPayloadSize,
                    pSettings->exchKeyLabel,
                    pSettings->emiModes
                 ));
    }

    return (NEXUS_SUCCESS);
}

/**
Summary:
API to provide data to the firmware so that it can transmit it out on the network.

Note: one usage of this API is to allow caller to send HTTP Request or HTTP Response to the remote.
**/
NEXUS_Error NEXUS_AspChannel_GetWriteBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    )
{
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pAmount2);

    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pAmount);

    *pBuffer = hAspChannel->writeFifo.pBuffer;
    *pAmount = hAspChannel->writeFifo.size;
    BDBG_MSG(("%s: hAspChannel=%p: pBuffer=%p amount=%u", BSTD_FUNCTION, (void *)hAspChannel, *pBuffer, *pAmount));
    return (NEXUS_SUCCESS);
}



NEXUS_Error NEXUS_AspChannel_WriteComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
    )
{
    BDBG_ASSERT(hAspChannel);

    hAspChannel->writeFifoLength = amount;
    NEXUS_FlushCache((void *)hAspChannel->writeFifo.pBuffer, amount);
    BDBG_MSG(("%s: hAspChannel=%p: amount=%u", BSTD_FUNCTION, (void *)hAspChannel, amount));
    return (NEXUS_SUCCESS);
}



/**
Summary:
API to receive network data (from firmware) for host access.

Note: one usage of this API is to allow caller to receive HTTP Response from the remote.
**/
NEXUS_Error NEXUS_AspChannel_GetReadBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    const void                          **pBuffer,   /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pAmount,    /* [out] number of bytes available in the data buffer pBuffer. */
    const void                          **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} pointer to buffer after wrap containing data read from network. */
    unsigned                            *pAmount2    /* [out] number of bytes available in the data buffer pBuffer2. */
    )
{
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pAmount2);

    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pAmount);

    if (hAspChannel->pRcvdPayload)
    {
        *pBuffer = hAspChannel->pRcvdPayload;
        *pAmount = hAspChannel->rcvdPayloadLength;
        *pBuffer2 = NULL;
        *pAmount2 = 0;
        BDBG_MSG(("%s: hAspChannel=%p: pBuffer=%p amount=%u", BSTD_FUNCTION, (void *)hAspChannel, *pBuffer, *pAmount));
        return (NEXUS_SUCCESS);
    }
    else
    {
        return (NEXUS_NOT_AVAILABLE);
    }
}

NEXUS_Error NEXUS_AspChannel_ReadComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            bytesRead    /* number of bytes read/consumed by the caller. */
    )
{
    BERR_Code rc;
    BASP_Pi2Fw_Message pi2FwMessage;

    BDBG_ASSERT(hAspChannel);
    BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));

    hAspChannel->pRcvdPayload = NULL;
    pi2FwMessage.MessagePayload.PayloadConsumed.ui32NumberofBytesToSkip = bytesRead;

    rc = BASP_Channel_SendMessage(
            hAspChannel->hChannel,
            BASP_MessageType_ePi2FwPayloadConsumed,
            BASP_ResponseType_eAckRespRequired,
            &pi2FwMessage);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    /* Wait for PayloadConsumed Response. */
    for (;;)
    {
        if (hAspChannel->gotPayloadConsumedResponse) { break; }

        NEXUS_UnlockModule();
        BKNI_Sleep(10);
        NEXUS_LockModule();
    }
    BDBG_MSG(("%s: hAspChannel=%p PayloadConsumed Msg Response Rcvd ...", BSTD_FUNCTION, (void *)hAspChannel));
    return (NEXUS_SUCCESS);
}


/**
Summary:
API to directly write Reassembled IP Packets to ASP FW for processing.

Host will receive the reassembled IP Packets via the OS Network Stack and then feed to the ASP FW via this API.
**/
NEXUS_Error NEXUS_AspChannel_GetReassembledWriteBufferWithWrap(
    NEXUS_AspChannelHandle              hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap which can be consumed */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    )
{
    BSTD_UNUSED(hAspChannel);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pAmount);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pAmount2);
    return (NEXUS_SUCCESS);
}


NEXUS_Error NEXUS_AspChannel_ReassembledWriteComplete(
    NEXUS_AspChannelHandle              hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
    )
{
    BSTD_UNUSED(hAspChannel);
    BSTD_UNUSED(amount);
    return (NEXUS_SUCCESS);
}



/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspChannel_GetSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    NEXUS_AspChannelSettings            *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspChannel, NEXUS_AspChannel);
    BDBG_ASSERT(pSettings);
    *pSettings = hAspChannel->settings;
}


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspChannel_SetSettings(
    NEXUS_AspChannelHandle              hAspChannel,
    const NEXUS_AspChannelSettings      *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspChannel, NEXUS_AspChannel);
    BDBG_ASSERT(pSettings);
    hAspChannel->settings = *pSettings;
    NEXUS_IsrCallback_Set(hAspChannel->hStateChangedCallback, &hAspChannel->settings.stateChanged);
    NEXUS_IsrCallback_Set(hAspChannel->hDataReadyCallback, &hAspChannel->settings.dataReady);

    return (NEXUS_SUCCESS);
}

static void processMessage_isr(
    NEXUS_AspChannelHandle              hAspChannel,
    BASP_MessageType                    messageType,
    BASP_Fw2Pi_Message                  *pFw2PiMessage
    )
{
    BDBG_MSG(("%s: hAspChannel=%p msgType=%d pMessage=%p", BSTD_FUNCTION, (void *)hAspChannel, messageType, (void *)pFw2PiMessage));
    switch (messageType)
    {
        case BASP_MessageType_eFw2PiRstNotify:
        case BASP_MessageType_eFw2PiFinNotify:  /* TODO: FIN handling has to be different. For now, just making it same as RST handling. */
        {
            hAspChannel->state = NEXUS_AspChannelState_eNetworkError;
            NEXUS_IsrCallback_Fire_isr(hAspChannel->hStateChangedCallback);
            BDBG_WRN(("%s: hAspChannel=%p Received %s: Network Peer sent TCP Rst, scheduled stateChangedCallback!", BSTD_FUNCTION, (void *)hAspChannel,
                    messageType==BASP_MessageType_eFw2PiRstNotify? "RstNotify":"FinNotify"));
            break;
        }
        case BASP_MessageType_eFw2PiPayloadNotify:  /* TODO: FIN handling has to be different. For now, just making it same as RST handling. */
        {
            uint64_t offset;
            uint32_t relativeOffset;

            /* The PayloadNotify msg contains the offset into the receiveFifo. */
            offset = pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrLo;
            offset |= (uint64_t)(pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrHi) << 32;
            /* Now determine the relative offset into the receiveFifo. */
            relativeOffset = offset - hAspChannel->receiveFifo.offset;
            hAspChannel->pRcvdPayload = ((uint8_t *)hAspChannel->receiveFifo.pBuffer) + relativeOffset;
            hAspChannel->rcvdPayloadLength = pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32Size;
            BDBG_WRN(("%s: hAspChannel=%p Rcvd PayloadNotify msg: offset hi:lo=0x%x:0x%x relativeOffset=%u pRcvdPayload=%p Length=%u issuing dataReadyCallback!", BSTD_FUNCTION,
                        (void *)hAspChannel, (uint32_t)(offset>>32), (uint32_t)offset, relativeOffset, (void *)hAspChannel->pRcvdPayload, hAspChannel->rcvdPayloadLength ));

            NEXUS_IsrCallback_Fire_isr(hAspChannel->hDataReadyCallback);
            break;
        }
        case BASP_MessageType_eFw2PiChannelAbortResp:
        {
            BDBG_WRN(("%s: hAspChannel=%p Processing Abort message from Network Peer!", BSTD_FUNCTION, (void *)hAspChannel));
            hAspChannel->tcpState.finalSendSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentSeqNumber;
            hAspChannel->tcpState.finalRecvSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentAckedNumber;
            hAspChannel->gotStopResponse = true;
            break;
        }
        case BASP_MessageType_eFw2PiChannelStartStreamOutResp:
            BDBG_WRN(("%s: hAspChannel=%p Received StartStreamOut response message from ASP!", BSTD_FUNCTION, (void *)hAspChannel));
            hAspChannel->gotStartResponse = true;
            break;

        case BASP_MessageType_eFw2PiChannelStartStreamInResp:
            BDBG_WRN(("%s: hAspChannel=%p Received StartStreamIn response message from ASP!", BSTD_FUNCTION, (void *)hAspChannel));
            hAspChannel->gotStartResponse = true;
            break;

        case BASP_MessageType_eFw2PiChannelStopResp:
            BDBG_WRN(("%s: hAspChannel=%p Received Stop response message from ASP!", BSTD_FUNCTION, (void *)hAspChannel));
            hAspChannel->gotStopResponse = true;
            break;

        case BASP_MessageType_eFw2PiPayloadConsumedResp:
            BDBG_WRN(("%s: hAspChannel=%p Received PayloadConsumed response message from ASP!", BSTD_FUNCTION, (void *)hAspChannel));
            hAspChannel->gotPayloadConsumedResponse = true;
            break;

        case BASP_MessageType_eFw2PiChannelAbortWithRstResp:
        case BASP_MessageType_eFw2PiPerformanceGatheringResp:
        case BASP_MessageType_eFw2PiGenericSgTableFeedResp:
        case BASP_MessageType_eFw2PiFrameConsumedResp:
        case BASP_MessageType_eFw2PiGetDrmConstResp:

        case BASP_MessageType_eFw2PiFrameAvailable:
        case BASP_MessageType_eFw2PiFinComplete:
        case BASP_MessageType_eFw2PiGenericSgTableFeedCompletion:
        case BASP_MessageType_eFw2PiRtoNotify:
        default:
        {
            BDBG_WRN(("%s: !!!!!! TODO: hAspChannel=%p msgType=%d is NOT YET Handled by Nexus!!!", BSTD_FUNCTION, (void *)hAspChannel, messageType));
            break;
        }
    }
}


#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS

static void NEXUS_ProcessMsgFromFwCallbackByTimer(void *pContext)
{

    int                     channelNum;
    bool                    foundChannel = false;

    BSTD_UNUSED(pContext);

    BDBG_MSG(("%s : %d : Entered by timer...", BSTD_FUNCTION, __LINE__ ));

    g_NEXUS_asp.hTimer = NULL;

    /* Loop through each channel. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspChannelList[channelNum] == NULL) { continue; }  /* This channel not active, try next one. */

        foundChannel = true;

        pContext = g_NEXUS_asp.hAspChannelList[channelNum];

        BKNI_EnterCriticalSection();
        NEXUS_ProcessMsgFromFwCallback_isr(pContext, 0);
        BKNI_LeaveCriticalSection();
    }

    if (foundChannel)
    {
        BDBG_MSG((" %s : %d Starting NEXUS TIMER for %u ms\n", BSTD_FUNCTION, __LINE__, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hTimer);
    }
}
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

static void NEXUS_ProcessMsgFromFwCallback_isr(void *pContext, int param)
{
    NEXUS_Error             rc;
    NEXUS_AspChannelHandle  hAspChannel = pContext;

    BSTD_UNUSED(param);


    /* Check if there is a response message or network generated event type message from ASP firmware. */
    while (true)
    {
        BASP_Fw2Pi_Message      fw2PiMessage;
        BASP_MessageType        msgType;
        unsigned                msgLen;

        BKNI_Memset_isr(&fw2PiMessage, 0, sizeof(fw2PiMessage));
        rc = BASP_Channel_ReadMessage_isr(hAspChannel->hChannel, &msgType, &fw2PiMessage, &msgLen);
        if (rc == BERR_NOT_AVAILABLE) {break;}
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s : %d : BASP_Channel_ReadMessage_isr() for hAspChannel=%p failed! status=%u", BSTD_FUNCTION, __LINE__, (void*) hAspChannel, rc ));
            BERR_TRACE(rc);
            return;
        }

        /* Make sure the message belongs to the channel that we asked for. */
        BDBG_ASSERT((unsigned)hAspChannel->channelNum == fw2PiMessage.MessageHeader.ui32ChannelIndex);

        processMessage_isr(hAspChannel, fw2PiMessage.MessageHeader.MessageType, &fw2PiMessage);
    }

    return;
}
