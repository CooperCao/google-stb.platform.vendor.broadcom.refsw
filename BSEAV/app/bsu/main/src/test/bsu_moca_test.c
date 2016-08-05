/******************************************************************************
* Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/***********************************************************************/
/* Header files                                                        */
/***********************************************************************/
#include <stdio.h>
#include <string.h>
#include "bsu_stdlib.h"
#ifndef MOCA2_SUPPORT
#include "devctl_moca.h"
#endif
typedef unsigned int uint32_t;
#include "bmocadriver.h"
#include "bsu_prompt.h"
#ifndef MOCA2_SUPPORT
#include "b_os_lib.h"
#endif
#if (BCHP_CHIP==7408)
#include "sys_handles.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clk.h"
#endif

#ifdef MOCA1_SUPPORT
int mocactl_main(int argc, char **argv);
#endif

#ifdef MOCA2_SUPPORT
int mocap_main(int argc, char **argv);
#endif

#define NUM_ARGS 32
#define MAX_STRING_SIZE 512
char arg_str[NUM_ARGS][MAX_STRING_SIZE];
char *args[NUM_ARGS];

#if (BCHP_CHIP==7408)
void bchip_moca_init(void)
{
    uint32_t val;

#ifdef BCHP_SUN_TOP_CTRL_SW_RESET
    //BDEV_WR_F_RB(SUN_TOP_CTRL_SW_RESET, moca_sw_reset, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_SUN_TOP_CTRL_SW_RESET);
    val &= ~BCHP_MASK(SUN_TOP_CTRL_SW_RESET, moca_sw_reset);
    val |= BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_RESET, moca_sw_reset, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_SUN_TOP_CTRL_SW_RESET, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_SUN_TOP_CTRL_SW_RESET);
#else
    BDEV_WR_F_RB(SUN_TOP_CTRL_SW_INIT_0_CLEAR, moca_sw_init, 1);
#endif

#if (BCHP_CHIP==7125)
    BDEV_WR_F_RB(CLKGEN_MISC_CLOCK_SELECTS, CLOCK_SEL_ENET_CG_MOCA, 0);
    BDEV_WR_F_RB(CLKGEN_MISC_CLOCK_SELECTS, CLOCK_SEL_GMII_CG_MOCA, 0);
#elif (BCHP_CHIP==7340)
    BDEV_WR_F_RB(CLKGEN_MISC_CLOCK_SELECTS, CLOCK_SEL_ENET_CG_MOCA, 1);
    BDEV_WR_F_RB(CLKGEN_MISC_CLOCK_SELECTS, CLOCK_SEL_GMII_CG_MOCA, 0);
#elif (BCHP_CHIP==7408) || (BCHP_CHIP==7420)
    //BDEV_WR_F_RB(CLK_MISC, MOCA_ENET_GMII_TX_CLK_SEL, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_MISC);
    val &= ~BCHP_MASK(CLK_MISC, MOCA_ENET_GMII_TX_CLK_SEL);
    val |= BCHP_FIELD_DATA(CLK_MISC, MOCA_ENET_GMII_TX_CLK_SEL, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_MISC, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_MISC);
#endif
}

static void bcm7408_pm_network_enable(void)
{
    uint32_t val;

/*
    if (MOCA_WOL(flags)) {
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL,
            DIS_GENET_RGMII_216M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL,
            DIS_MOCA_ENET_UNIMAC_SYS_TX_27_108M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL,
            DIS_MOCA_ENET_L2_INTR_27_108M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL,
            DIS_MOCA_ENET_GMII_TX_27_108M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL, DIS_MOCA_54M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL, DIS_216M_CLK, 0);
        BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL, DIS_108M_CLK, 0);
        BDEV_WR_F_RB(CLK_MISC, MOCA_ENET_CLK_SEL, 0);
        BDEV_WR_F_RB(CLK_MISC, MOCA_ENET_GMII_TX_CLK_SEL, 0);
        return;
    }
*/

    //BDEV_UNSET_RB(BCHP_CLK_MOCA_CLK_PM_CTRL, 0x6ab);
    val = BREG_Read32(bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL);
    val &= ~0x6ab;
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL, val);
    BREG_Read32(bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_6, DIS_CH, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_6);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_6, DIS_CH);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_5, DIS_CH, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_6, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_6);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_5, DIS_CH, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_5, EN_CMLBUF);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_5, EN_CMLBUF, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_5, EN_CMLBUF, 1);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_5, EN_CMLBUF);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_5, EN_CMLBUF, 1);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_5);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_4, DIS_CH, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_4, DIS_CH);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_4, DIS_CH, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_4, EN_CMLBUF, 1);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_4, EN_CMLBUF);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_4, EN_CMLBUF, 1);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_4);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_3, DIS_CH, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_3, DIS_CH);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_3, DIS_CH, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3);

    //BDEV_WR_F_RB(CLK_SYS_PLL_1_3, EN_CMLBUF, 1);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3);
    val &= ~BCHP_MASK(CLK_SYS_PLL_1_3, EN_CMLBUF);
    val |= BCHP_FIELD_DATA(CLK_SYS_PLL_1_3, EN_CMLBUF, 1);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_SYS_PLL_1_3);

    //BDEV_WR_F_RB(CLK_MOCA_CLK_PM_CTRL, DIS_MOCA_ENET_HFB_27_108M_CLK, 0);
    val = BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL);
    val &= ~BCHP_MASK(CLK_MOCA_CLK_PM_CTRL, DIS_MOCA_ENET_HFB_27_108M_CLK);
    val |= BCHP_FIELD_DATA(CLK_MOCA_CLK_PM_CTRL, DIS_MOCA_ENET_HFB_27_108M_CLK, 0);
    BREG_Write32(bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL, val);
    BREG_Read32 (bcmGetRegHandle(), BCHP_CLK_MOCA_CLK_PM_CTRL);
}
#endif

/*****************************************************************************
 ****************************************************************************/
void MocaInit (char *str, char *freq)
{
    tBMocaMACAddress mac_addr;
    char cmd_str[MAX_STRING_SIZE];

#ifdef MOCA2_SUPPORT
    ((void)freq);   /* unused */
#endif

#if 0 // (BCHP_CHIP==7408)
    bchip_moca_init();
    bcm7408_pm_network_enable();
#endif

    strcpy(cmd_str, str);
#ifdef MOCA1_SUPPORT
    strcat(cmd_str, "F");
    strcat(cmd_str, freq);
#endif
    mac_addr.macaddr_hi = 0;
    #if (BCHP_CHIP==7125)
        mac_addr.macaddr_lo = 7125;
    #elif (BCHP_CHIP==7340)
        mac_addr.macaddr_lo = 7340;
    #elif (BCHP_CHIP==7344)
        mac_addr.macaddr_lo = 7344;
    #elif (BCHP_CHIP==7408)
        mac_addr.macaddr_lo = 7408;
    #elif (BCHP_CHIP==7420)
        mac_addr.macaddr_lo = 7420;
    #elif (BCHP_CHIP==7425)
        mac_addr.macaddr_lo = 7425;
    #elif (BCHP_CHIP==7429)
        mac_addr.macaddr_lo = 7429;
    #elif (BCHP_CHIP==7439)
        mac_addr.macaddr_lo = 7439;
    #else
        #error unsupported chip
    #endif
    BcmMoCA_Init (&mac_addr, cmd_str);
}

#ifdef MOCA2_SUPPORT
typedef int CmsRet;
#endif

void MocaCmd (char *str)
{
    char cmd_str[MAX_STRING_SIZE];
    char *str2;
    int i;

    for (i=0; i<NUM_ARGS; i++) {
        args[i] = (char *)&arg_str[i];
    }

    i=0;
    strcpy(cmd_str, str);
    str2 = strtok(cmd_str, " ");
    strcpy(arg_str[i++], str2);
    while ((str2=strtok(NULL, " ")))
        strcpy(arg_str[i++], str2);

#ifdef MOCA1_SUPPORT
    mocactl_main(i, args);
#else
    mocap_main(i, args);
#endif
}

#ifdef MOCA1_SUPPORT
CmsRet MocaGetStatus(MoCA_STATUS *pStatus)
{
    CmsRet nRet = CMSRET_SUCCESS ;
    void * g_mocaHandle;

    g_mocaHandle = MoCACtl_Open( NULL );
    nRet = MoCACtl2_GetStatus( g_mocaHandle, pStatus);
    MoCACtl_Close( g_mocaHandle );

    return nRet;
}

CmsRet MocaGetNodeStatus (MoCA_NODE_STATUS_ENTRY *pNodeStatus)
{
    CmsRet nRet = CMSRET_SUCCESS ;
    void * g_mocaHandle;

    g_mocaHandle = MoCACtl_Open( NULL );
    nRet = MoCACtl2_GetNodeStatus( g_mocaHandle, pNodeStatus);
    MoCACtl_Close( g_mocaHandle );

    return nRet;
}
#endif

void MocaShell(void)
{
    char input[MAX_STRING_SIZE];
    while (1)
    {
        printf("# ");
        rdstr(input);
        if (strcmp(input, "")==0)
            continue;
        if ((strcmp(input, "exit")==0) || (strcmp(input, "quit")==0))
            return;
        else
            MocaCmd(input);
    }
}

#ifdef MOCA1_SUPPORT
void bsu_moca_test (void)
{
    static int init=0;
    static char freq[10];
    #if (((BCM_BOARD==97420) && defined(DIAGS_SATFE)) || (BCM_BOARD==97340))
    strcpy(freq, "675");
    #else
    strcpy(freq, "1200");
    #endif

    if (!init)
    {
        MocaInit("-wD", freq);
        /*MocaInit("-DP", freq);*/ /* enables logging */
        init=1;
    }

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  MoCA MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Enter MoCA command shell\n");
        printf("    2) Start MoCA\n");
        printf("    3) \"mocactl start\"\n");
        printf("    4) \"mocactl stop\"\n");
        printf("    5) \"mocactl restart\"\n");
        printf("    6) \"mocactl show --initparms\"\n");
        printf("    7) \"mocactl show --status\"\n");
        printf("    8) \"mocactl show --nodestatus 0\"\n");
        printf("    9) \"mocactl show --nodestatus 1\"\n");
        printf("    a) \"mocactl\"\n");
        printf("    b) \"mocactl restart --rfType bandF --beaconChannel 675 --lof 675\"\n");
        printf("    c) \"mocactl restart --rfType midhi --beaconChannel 850\"\n");
        printf("    d) \"mocactl restart --tpc off\"\n");
        printf("    e) \"mocactl restart --singleCh on\"\n");
        printf("    f) \"mocactl restart --autoScan off\"\n");
        printf("    g) \"mocactl config --rlapm off --snrMgn 0\"\n");
        printf("    h) \"mocactl config --sapm off\"\n");
        printf("    i) \"mocactl restart --maxTxPower 3\"\n");
        printf("    j) \"mocactl restart --lof 850\"\n");
        printf("    k) \"mocactl restart --constTxMode normal\"\n");
        printf("    l) \"mocactl restore_defaults trace config start\"\n");
        printf("    m) Get status API\n");
        printf("    n) Get node status 0 API\n");
        printf("    o) Get node status 1 API\n");
        printf("    p) Loop Get status API\n");
        printf("    q) Loop band F, Get status API\n");
        printf("    r) \"mocactl restart --rfType bandD\"\n");
        printf("    s) \"mocactl restart --beaconChannel 1150 --lof 1150\"\n");
        printf("    t) \"mocactl restart --rfType bandE --beaconChannel 500 --lof 500\"\n");
        printf("    u) \"mocactl show --stats\"\n");
        printf("    v) \"mocactl trace --core\"\n");
        printf("    w) \"mocactl trace --mmpdump\"\n");
        printf("    z) Toggle logging (logging=%d)\n", 0);

        switch(PromptChar())
        {
            case '0':
                return;

            case '1':
                MocaShell();
                break;

            case '2':
                MocaCmd("mocactl restore_defaults");
                MocaCmd("mocactl trace");
                MocaCmd("mocactl config");
                MocaCmd("mocactl start");
                break;

            case '3':
                MocaCmd("mocactl start");
                break;

            case '4':
                MocaCmd("mocactl stop");
                break;

            case '5':
                MocaCmd("mocactl restart");
                break;

            case '6':
                MocaCmd("mocactl show --initparms");
                break;

            case '7':
                MocaCmd("mocactl show --status");
                break;

            case '8':
                MocaCmd("mocactl show --nodestatus 0");
                break;

            case '9':
                MocaCmd("mocactl show --nodestatus 1");
                break;

            case 'a':
                MocaCmd("mocactl");
                break;

            case 'b':
                MocaCmd("mocactl restart --rfType bandF --beaconChannel 675 --lof 675");
                break;

            case 'c':
                MocaCmd("mocactl restart --rfType midhi --beaconChannel 850");
                break;

            case 'd':
                MocaCmd("mocactl restart --tpc off");
                break;

            case 'e':
                MocaCmd("mocactl restart --singleCh on");
                break;

            case 'f':
                MocaCmd("mocactl restart --autoScan off");
                break;

            case 'g':
                MocaCmd("mocactl config --rlapm off --snrMgn 0");
                break;

            case 'h':
                MocaCmd("mocactl config --sapm off");
                break;

            case 'i':
                MocaCmd("mocactl restart --maxTxPower 3");
                break;

            case 'j':
                MocaCmd("mocactl restart --lof 850");
                break;

            case 'k':
                MocaCmd("mocactl restart --constTxMode normal");
                break;

            case 'l':
                MocaCmd("mocactl restore_defaults trace config start");
                break;

            case 'm':
                {
                    MoCA_STATUS status;
                    if (MocaGetStatus(&status) == CMSRET_SUCCESS)
                        printf("nodeId=%d, connectedNodes=0x%x, linkStatus=%d\n", status.generalStatus.nodeId, status.generalStatus.connectedNodes, status.generalStatus.linkStatus);
                }
                break;

            case 'n':
                {
                    MoCA_NODE_STATUS_ENTRY nodeStatus;
                    nodeStatus.nodeId = 0;
                    if (MocaGetNodeStatus(&nodeStatus) == CMSRET_SUCCESS)
                        printf("SNR=%d\n", nodeStatus.rxBc.avgSnr);
                    else
                        printf("error returned from MocaGetNodeStatus\n");
                }
                break;

            case 'o':
                {
                    MoCA_NODE_STATUS_ENTRY nodeStatus;
                    nodeStatus.nodeId = 1;
                    if (MocaGetNodeStatus(&nodeStatus) == CMSRET_SUCCESS)
                        printf("SNR=%d\n", nodeStatus.rxBc.avgSnr);
                    else
                        printf("error returned from MocaGetNodeStatus\n");
                }
                break;

            case 'p':
                while (1)
                {
                    MoCA_STATUS status;
                    if (MocaGetStatus(&status) == CMSRET_SUCCESS)
                        printf("nodeId=%d, connectedNodes=0x%x, linkStatus=%d\n", status.generalStatus.nodeId, status.generalStatus.connectedNodes, status.generalStatus.linkStatus);
                    BKNI_Sleep(2000);
                    if (det_in_char()) break;
                }
                break;

            case 'q':
                {
                    MoCA_STATUS status;
                    MoCA_NODE_STATUS_ENTRY nodeStatus;
                    B_Time initial_time;
                    int exit_loop=0;
                    int loop_count=0;
                    int link_up_count=0;
                    B_Time_Get(&initial_time);
                    while (1)
                    {
                        int counter=0;
                        int time_ms;
                        B_Time start_time;
                        B_Time end_time;
                        int time_diff_ms;

                        B_Time time;
                        B_Time_Get(&start_time);

                        MocaCmd("mocactl restart --rfType bandF --beaconChannel 675 --lof 675");
                        while (1)
                        {
                            if (MocaGetStatus(&status) != CMSRET_SUCCESS)
                                printf("error returned from MocaGetStatus\n");

                            nodeStatus.nodeId = 0;
                            if (MocaGetNodeStatus(&nodeStatus) == CMSRET_SUCCESS)
                            {
                                /*
                                if (nodeStatus.rxBc.avgSnr)
                                    printf("SNR=%d\n", nodeStatus.rxBc.avgSnr);
                                */
                            }
                            else
                                printf("error returned from MocaGetNodeStatus 0\n");

                            nodeStatus.nodeId = 1;
                            if (MocaGetNodeStatus(&nodeStatus) == CMSRET_SUCCESS)
                            {
                                /*
                                if (nodeStatus.rxBc.avgSnr)
                                    printf("SNR=%d\n", nodeStatus.rxBc.avgSnr);
                                */
                            }
                            else
                                printf("error returned from MocaGetNodeStatus 1\n");

                            if (det_in_char())
                            {
                                exit_loop=1;
                                break;
                            }
                            B_Time_Get(&end_time);
                            if (B_Time_Diff(&end_time, &start_time) > 120000)
                            {
                                break;
                            }
                            if (status.generalStatus.linkStatus)
                            {
                                link_up_count++;
                                break;
                            }
                        }
                        loop_count++;

                        printf("loop_count=%d, link_up_count=%d, elapsed time=%d msecs\n", loop_count, link_up_count, B_Time_Diff(&end_time, &initial_time));
                        if (exit_loop) break;
                    }
                }
                break;

            case 'r':
                MocaCmd("mocactl restart --rfType bandD");
                break;

            case 's':
                MocaCmd("mocactl restart --beaconChannel 1150 --lof 1150");
                break;

            case 't':
                MocaCmd("mocactl restart --rfType bandE --beaconChannel 500 --lof 500");
                break;

            case 'u':
                MocaCmd("mocactl show --stats");
                break;

            case 'v':
                MocaCmd("mocactl trace --core");
                break;

            case 'w':
                MocaCmd("mocactl trace --mmpdump");
                break;

            case 'z':
                /*moca_logging = moca_logging ? 0 : 1;*/
                break;

            default:
                printf("Invalid selection\n");
                break;
        }

            default:
                printf("Invalid selection\n");
                break;
        }
    }
}
#else
void bsu_moca_test (void)
{
    static int init=0;
    static char freq[10];
    #if (((BCM_BOARD==97420) && defined(DIAGS_SATFE)) || (BCM_BOARD==97340))
    strcpy(freq, "675");
    #else
    strcpy(freq, "1200");
    #endif

    if (!init)
    {
        MocaInit("-wD", freq);
        /*MocaInit("-DP", freq);*/ /* enables logging */
        init=1;
    }

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  MoCA MENU  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Enter MoCA command shell\n");
        printf("    2) Start MoCA\n");
        printf("    3) \"mocap set --start\"\n");
        printf("    4) \"mocap set --stop\"\n");
        printf("    5) \"mocap set --lof 1200 --pco 0 --bandwidth 1 --restart\"\n");
        printf("    6) \"mocap get --gen_node_ext_status index 0 profile_type 7\"\n");
        printf("    7) \"mocap get --gen_node_ext_status index 1 profile_type 7\"\n");
        printf("    8) \"mocap set --start --nc_mode 0 --preferred_nc 0 --single_channel_operation 1 --rf_band 0 --beacon_channel_set 0 --lof 1150 --privacy_en 1 --password 01234567890123456 --beacon_pwr_reduction_en 0 --taboo taboo_fixed_mask_start 41 taboo_fixed_channel_mask 0x0\"\n");

        switch(PromptChar())
        {
            case '0':
                return;

            case '1':
                MocaShell();
                break;

            case '2':
                MocaCmd("mocap set --restore_defaults");
                //MocaCmd("mocap set --trace 1 --restart");
                MocaCmd("mocap set --start");
                break;

            case '3':
                MocaCmd("mocap set --start");
                break;

            case '4':
                MocaCmd("mocap set --stop");
                break;

            case '5':
                MocaCmd("mocap set --lof 1200 --pco 0 --bandwidth 1 --restart");
                break;

            case '6':
                MocaCmd("mocap get --gen_node_ext_status index 0 profile_type 7");
                break;

            case '7':
                MocaCmd("mocap get --gen_node_ext_status index 1 profile_type 7");
                break;

            case '8':
                MocaCmd("mocap set --start --nc_mode 0 --preferred_nc 0 --single_channel_operation 1 --rf_band 0 --beacon_channel_set 0 --lof 1150 --privacy_en 1 --password 01234567890123456 --beacon_pwr_reduction_en 0 --taboo taboo_fixed_mask_start 41 taboo_fixed_channel_mask 0x0");
                break;

            default:
                printf("Invalid selection\n");
                break;
        }
    }
}
#endif
