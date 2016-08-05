/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   IEEE Std 802.15.4-2006 compatible Radio Driver implementation.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

/************************* INCLUDES ***********************************************************************************/
#include "bbHalRadio.h"
#include "bbPhySapForMac.h"

#ifdef _PHY_TEST_HOST_INTERFACE_
#include "bbPhySapIeee.h"
extern bool phyTestMode;
extern void phy_ResumeReq(void);
#endif

/************************* IMPLEMENTATION *****************************************************************************/
/* The code of the asynchronous request on processing by the Radio Driver. */
extern enum HAL_Radio__TASK_Code_t  HAL_Radio__TASK = HAL_RADIO_TASK__IDLE;

/* TX frame buffer for beacon, data, or command frame PSDU. */
extern HAL_Radio__PSDU_t  HAL_Radio_FrmBuf__TX_BDC_PSDU = {};

/* TX frame buffer for beacon, data, or command frame PHR. */
extern HAL_Radio__PHR_t  HAL_Radio_FrmBuf__TX_BDC_PHR = 0;

/* TX frame buffer for ACK frame PSDU. */
extern HAL_Radio__Octet_t  HAL_Radio_FrmBuf__TX_ACK_PSDU[HAL_aAckMPDUOverhead - HAL_aFCSSize] = {};

/* RX frame buffer. */
extern union HAL_Radio_FrmBuf__RX_PPDU_t  HAL_Radio_FrmBuf__RX_PPDU = {};

/* Combined variable for storing different parameters of the last received packet. */
extern struct HAL_Radio_FrmBuf__RX_Stuff_t  HAL_Radio_FrmBuf__RX_Stuff = {};

/* Timestamps of the last transmitted and the last received packets. */
extern struct HAL_Radio_FrmBuf__Tstamps_t  HAL_Radio_FrmBuf__TX_Tstamps = {}, HAL_Radio_FrmBuf__RX_Tstamps = {};

/* Variable for saving status/results of the last confirmed request to the Driver. */
extern union HAL_Radio_FrmBuf__Status_t  HAL_Radio_FrmBuf__Status = {};

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles confirmation of a packet transmission. */
void HAL_Radio__DATA_conf(void)
{
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        {
            PHY_DataConf(PHY__SUCCESS, HAL_Radio_FrmBuf__TX_Tstamps.start, HAL_Radio_FrmBuf__TX_Tstamps.end);
            phy_ResumeReq();
        }
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
        ;
#else
        PHY__DATA_conf(); // Normal MAC need it. And in some cases, Test mode also need it to set stack to normal status.
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles indication of a packet reception. */
void HAL_Radio__DATA_ind(void)
{
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        {
            PHY_DataInd(HAL_Radio_FrmBuf__RX_Tstamps.start, HAL_Radio_FrmBuf__RX_Tstamps.end);
            phy_ResumeReq();
        }
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
        ;
#else
        PHY__DATA_ind(); // Normal MAC need it. And in some cases, Test mode also need it to set stack to normal status.
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles confirmation of the Radio state switching. */
void HAL_Radio__STATE_conf(void)
{
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        PHY_SetTrxStateConf(PHY__SUCCESS);
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
        ;
#else
        PHY__STATE_conf();
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles confirmation of the Clear Channel Assessment (CCA) detection. */
void HAL_Radio__CCA_conf(void)
{
// XXX: This code shall be used if Host-test needs dynamic routing of PLME-CCA.confirm.
//   wzz: Yes. We do need it works for both normal case and test case.
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        PHY_CcaConf(HAL_Radio_FrmBuf__Status.ccaIdle ? PHY__IDLE : PHY__BUSY);
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
        ;
#else
        PHY__CCA_conf();
#endif

// XXX: Currently Host-test does not need dynamic routing of PLME-CCA.confirm.
//      See also HAL_Symbol_Match__PHY_Confirm().
//   wzz: In some cases, they may need to work together instead of exclusively.
//           It should be left for TEST app to decide how to use them
//#ifdef _PHY_TEST_HOST_INTERFACE_
//    PHY__CCA_conf();
//#else
//    PHY_CcaConf(HAL_Radio_FrmBuf__Status.ccaIdle ? PHY__IDLE : PHY__BUSY);
//#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles confirmation of the Energy Detection (ED) measurement. */
void HAL_Radio__ED_conf(void)
{
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        PHY_EdConf(PHY__SUCCESS, HAL_Radio_FrmBuf__Status.edLevel);
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
       ;
#else
        PHY__ED_conf();
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Handles confirmation of the channel and page switching. */
void HAL_Radio__CHANNEL_conf(void)
{
// XXX: This code shall be used if Host-test needs dynamic routing of PLME-SET-CHANNEL.confirm.
//   wzz: Yes. We do need it works for both normal case and test case.
#ifdef _PHY_TEST_HOST_INTERFACE_
    if (phyTestMode)
        PHY_SetChannelConf(PHY__SUCCESS);
    else /* if (phyTestMode) */
#endif
#ifdef _SOC_PHY_TEST_
        ;
#else
        PHY__CHANNEL_conf();
#endif

// XXX: Currently Host-test does not need dynamic routing of PLME-SET-CHANNEL.confirm.
//      See also HAL_Symbol_Match__PHY_Confirm().
//   wzz: In some cases, they may need to work together instead of exclusively.
//           It should be left for TEST app to decide how to use them
//#ifdef _PHY_TEST_HOST_INTERFACE_
//    PHY__CHANNEL_conf();
//#else
//    PHY_SetChannelConf(PHY__SUCCESS);
//#endif
}


/* eof bbHalRadio.c */
