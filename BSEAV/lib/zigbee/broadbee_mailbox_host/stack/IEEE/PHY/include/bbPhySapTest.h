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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesStart.h $
*
* DESCRIPTION:
*   MLME-START service data types definition.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_SAP_TEST_H
#define _BB_PHY_SAP_TEST_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */

//#ifdef _PHY_TEST_HOST_INTERFACE_
typedef struct _Phy_Test_Get_Caps_ConfParams_t
{
    uint8_t channelMin;
    uint8_t channelMax;
    int8_t  powerMin;
    int8_t  powerMax;
} Phy_Test_Get_Caps_ConfParams_t;

typedef struct _Phy_Test_Get_Caps_ReqDescr_t Phy_Test_Get_Caps_ReqDescr_t;

typedef struct _Phy_Test_Get_Caps_ReqDescr_t{
    void (*callback)(Phy_Test_Get_Caps_ReqDescr_t *, Phy_Test_Get_Caps_ConfParams_t *);
}Phy_Test_Get_Caps_ReqDescr_t;



typedef struct _Phy_Test_Set_Channel_ReqParams_t{
    uint8_t     channel;
}Phy_Test_Set_Channel_ReqParams_t;

typedef struct _Phy_Test_Set_Channel_ConfParams_t{
    uint8_t     status;
}Phy_Test_Set_Channel_ConfParams_t;

typedef struct _Phy_Test_Set_Channel_ReqDescr_t Phy_Test_Set_Channel_ReqDescr_t;

typedef struct _Phy_Test_Set_Channel_ReqDescr_t{
    Phy_Test_Set_Channel_ReqParams_t  params;
    void (*callback)(Phy_Test_Set_Channel_ReqDescr_t *, Phy_Test_Set_Channel_ConfParams_t *);
}Phy_Test_Set_Channel_ReqDescr_t;

typedef enum{
    RF4CE_CTRL_ANTENNA_1 = 1,
    RF4CE_CTRL_ANTENNA_2 = 2,
    RF4CE_CTRL_ANTENNA_1_2 = 3
}RF4CE_CTRL_ANTENNA;


typedef struct _Phy_Test_Select_Antenna_ReqParams_t{
    RF4CE_CTRL_ANTENNA     antenna;
}Phy_Test_Select_Antenna_ReqParams_t;

typedef struct _Phy_Test_Select_Antenna_ConfParams_t{
    uint8_t     status;
}Phy_Test_Select_Antenna_ConfParams_t;

typedef struct _Phy_Test_Select_Antenna_ReqDescr_t Phy_Test_Select_Antenna_ReqDescr_t;

typedef struct _Phy_Test_Select_Antenna_ReqDescr_t{
    Phy_Test_Select_Antenna_ReqParams_t  params;
    void (*callback)(Phy_Test_Select_Antenna_ReqDescr_t *, Phy_Test_Select_Antenna_ConfParams_t *);
}Phy_Test_Select_Antenna_ReqDescr_t;


typedef enum _RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE{
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE_UNMODULATED = 0x01,
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE_MODULATED   = 0x02
}RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE;

typedef enum _RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STARTSTOP{
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_START = 0x01,
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STOP  = 0x02
}RF4CE_CTRL_TEST_CONTINUOUS_WAVE_STARTSTOP;

typedef struct _Phy_Test_Continuous_Wave_Start_ReqParams_t{
    RF4CE_CTRL_TEST_CONTINUOUS_WAVE_MODE mode;
}Phy_Test_Continuous_Wave_Start_ReqParams_t;

typedef struct _Phy_Test_Continuous_Wave_StartStop_ConfParams_t{
    uint8_t status;
}Phy_Test_Continuous_Wave_StartStop_ConfParams_t;

typedef struct _Phy_Test_Continuous_Wave_Start_ReqDescr_t Phy_Test_Continuous_Wave_Start_ReqDescr_t;

typedef struct _Phy_Test_Continuous_Wave_Start_ReqDescr_t{
    Phy_Test_Continuous_Wave_Start_ReqParams_t params;
    void (*callback)(Phy_Test_Continuous_Wave_Start_ReqDescr_t *, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *);
}Phy_Test_Continuous_Wave_Start_ReqDescr_t;

typedef struct _Phy_Test_Continuous_Wave_Stop_ReqDescr_t Phy_Test_Continuous_Wave_Stop_ReqDescr_t;

typedef struct _Phy_Test_Continuous_Wave_Stop_ReqDescr_t{
    void (*callback)(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *, Phy_Test_Continuous_Wave_StartStop_ConfParams_t *);
}Phy_Test_Continuous_Wave_Stop_ReqDescr_t;



#define RF4CE_CTRL_TEST_MAX_LENGTH_PACKET   (125U)
typedef struct _Phy_Test_Transmit_Start_ReqParams_t{
    uint32_t packetCount;
    uint16_t intervalMs;
    uint8_t payload[RF4CE_CTRL_TEST_MAX_LENGTH_PACKET];
    uint8_t payloadLength;
}Phy_Test_Transmit_Start_ReqParams_t;

typedef struct _Phy_Test_Transmit_StartStop_ConfParams_t{
    uint8_t status;
}Phy_Test_Transmit_StartStop_ConfParams_t;

typedef struct _Phy_Test_Transmit_Start_ReqDescr_t Phy_Test_Transmit_Start_ReqDescr_t;

typedef struct _Phy_Test_Transmit_Start_ReqDescr_t{
    Phy_Test_Transmit_Start_ReqParams_t params;
    void (*callback)(Phy_Test_Transmit_Start_ReqDescr_t *, Phy_Test_Transmit_StartStop_ConfParams_t *);
}Phy_Test_Transmit_Start_ReqDescr_t;

typedef struct _Phy_Test_Transmit_Stop_ReqDescr_t Phy_Test_Transmit_Stop_ReqDescr_t;

typedef struct _Phy_Test_Transmit_Stop_ReqDescr_t{
    void (*callback)(Phy_Test_Transmit_Stop_ReqDescr_t *, Phy_Test_Transmit_StartStop_ConfParams_t *);
}Phy_Test_Transmit_Stop_ReqDescr_t;



typedef struct _Phy_Test_Receive_StartStop_ConfParams_t{
    uint8_t status;
}Phy_Test_Receive_StartStop_ConfParams_t;

typedef struct _Phy_Test_Receive_Start_ReqParams_t{
    uint32_t filter;
    uint16_t shortAddressFrom;
    uint16_t shortAddressTo;
    MAC_Addr64bit_t longAddressFrom;
    MAC_Addr64bit_t longAddressTo;
}Phy_Test_Receive_Start_ReqParams_t;

typedef struct _Phy_Test_Receive_Start_ReqDescr_t Phy_Test_Receive_Start_ReqDescr_t;

typedef struct _Phy_Test_Receive_Start_ReqDescr_t{
    Phy_Test_Receive_Start_ReqParams_t  params;
    void (*callback)(Phy_Test_Receive_Start_ReqDescr_t *, Phy_Test_Receive_StartStop_ConfParams_t *);
}Phy_Test_Receive_Start_ReqDescr_t;

typedef struct _Phy_Test_Receive_Stop_ReqDescr_t Phy_Test_Receive_Stop_ReqDescr_t;

typedef struct _Phy_Test_Receive_Stop_ReqDescr_t{
    void (*callback)(Phy_Test_Receive_Stop_ReqDescr_t *, Phy_Test_Receive_StartStop_ConfParams_t *);
}Phy_Test_Receive_Stop_ReqDescr_t;


typedef struct _Phy_Test_Echo_StartStop_ConfParams_t{
    uint8_t status;
}Phy_Test_Echo_StartStop_ConfParams_t;

typedef struct _Phy_Test_Echo_Start_ReqDescr_t Phy_Test_Echo_Start_ReqDescr_t;
typedef struct _Phy_Test_Echo_Start_ReqDescr_t{
    void (*callback)(Phy_Test_Echo_Start_ReqDescr_t *, Phy_Test_Echo_StartStop_ConfParams_t *);
}Phy_Test_Echo_Start_ReqDescr_t;

typedef struct _Phy_Test_Echo_Stop_ReqDescr_t Phy_Test_Echo_Stop_ReqDescr_t;
typedef struct _Phy_Test_Echo_Stop_ReqDescr_t{
    void (*callback)(Phy_Test_Echo_Stop_ReqDescr_t *, Phy_Test_Echo_StartStop_ConfParams_t *);
}Phy_Test_Echo_Stop_ReqDescr_t;


typedef struct _Phy_Sap_RF4CE_EnergyDetectionScanResults
{
    uint8_t energy;
}Phy_Sap_RF4CE_EnergyDetectionScanResults;

typedef struct _Phy_Test_Energy_Detect_Scan_ReqParams_t{
    uint8_t numberOfScans;
    uint8_t inervalInMs;
}Phy_Test_Energy_Detect_Scan_ReqParams_t;

typedef struct _Phy_Test_Energy_Detect_Scan_ConfParams_t{
    uint8_t             status;
    SYS_DataPointer_t   payload;  /* the array of the RF4CE_EnergyDetectionScanResults, the number of elements is numberOfScans */
}Phy_Test_Energy_Detect_Scan_ConfParams_t;

typedef struct _Phy_Test_Energy_Detect_Scan_ReqDescr_t Phy_Test_Energy_Detect_Scan_ReqDescr_t;
typedef struct _Phy_Test_Energy_Detect_Scan_ReqDescr_t{
    Phy_Test_Energy_Detect_Scan_ReqParams_t    params;
    void (*callback)(Phy_Test_Energy_Detect_Scan_ReqDescr_t *, Phy_Test_Energy_Detect_Scan_ConfParams_t *);
}Phy_Test_Energy_Detect_Scan_ReqDescr_t;


typedef struct _Phy_Test_Get_Stats_ConfParams_t{
    uint32_t packetsReceived;
    uint32_t packetsOverflow;
    uint32_t packetsSentOK;
    uint32_t packetsSentError;
}Phy_Test_Get_Stats_ConfParams_t;

typedef struct _Phy_Test_Get_Stats_ReqDescr_t Phy_Test_Get_Stats_ReqDescr_t;
typedef struct _Phy_Test_Get_Stats_ReqDescr_t{
    void (*callback)(Phy_Test_Get_Stats_ReqDescr_t *, Phy_Test_Get_Stats_ConfParams_t *);
}Phy_Test_Get_Stats_ReqDescr_t;


typedef struct _Phy_Test_Reset_Stats_ConfParams_t{
    uint8_t status;
}Phy_Test_Reset_Stats_ConfParams_t;

typedef struct _Phy_Test_Reset_Stats_ReqDescr_t Phy_Test_Reset_Stats_ReqDescr_t;

typedef struct _Phy_Test_Reset_Stats_ReqDescr_t{
    void (*callback)(Phy_Test_Reset_Stats_ReqDescr_t *, Phy_Test_Reset_Stats_ConfParams_t *);
}Phy_Test_Reset_Stats_ReqDescr_t;


typedef struct _Phy_Test_Set_TX_Power_ConfParams_t{
    uint8_t status;
}Phy_Test_Set_TX_Power_ConfParams_t;

typedef struct _Phy_Test_Set_TX_Power_ReqParams_t{
    int  power;
}Phy_Test_Set_TX_Power_ReqParams_t;

typedef struct _Phy_Test_Set_TX_Power_ReqDescr_t Phy_Test_Set_TX_Power_ReqDescr_t;

typedef struct _Phy_Test_Set_TX_Power_ReqDescr_t{
    Phy_Test_Set_TX_Power_ReqParams_t  params;
    void (*callback)(Phy_Test_Set_TX_Power_ReqDescr_t *, Phy_Test_Set_TX_Power_ConfParams_t *);
}Phy_Test_Set_TX_Power_ReqDescr_t;


void Phy_Test_Get_Caps_Req(Phy_Test_Get_Caps_ReqDescr_t *req);

void Phy_Test_Set_Channel_Req(Phy_Test_Set_Channel_ReqDescr_t *req);

void Phy_Test_SelectAntenna_Req(Phy_Test_Select_Antenna_ReqDescr_t *req);

void Phy_Test_Continuous_Wave_Start_Req(Phy_Test_Continuous_Wave_Start_ReqDescr_t *req);

void Phy_Test_Continuous_Wave_Stop_Req(Phy_Test_Continuous_Wave_Stop_ReqDescr_t *req);

void Phy_Test_Transmit_Start_Req(Phy_Test_Transmit_Start_ReqDescr_t *req);

void Phy_Test_Transmit_Stop_Req(Phy_Test_Transmit_Stop_ReqDescr_t *req);

void Phy_Test_Receive_Start_Req(Phy_Test_Receive_Start_ReqDescr_t *req);

void Phy_Test_Receive_Stop_Req(Phy_Test_Receive_Stop_ReqDescr_t *req);

void Phy_Test_Echo_Start_Req(Phy_Test_Echo_Start_ReqDescr_t *req);

void Phy_Test_Echo_Stop_Req(Phy_Test_Echo_Stop_ReqDescr_t *req);

void Phy_Test_Energy_Detect_Scan_Req(Phy_Test_Energy_Detect_Scan_ReqDescr_t *req);

void Phy_Test_Get_Stats_Req(Phy_Test_Get_Stats_ReqDescr_t *req);

void Phy_Test_Reset_Stats_Req(Phy_Test_Reset_Stats_ReqDescr_t *req);

void Phy_Test_Set_TX_Power_Req(Phy_Test_Set_TX_Power_ReqDescr_t *req);

//#endif
#endif /* _BB_PHY_SAP_TEST_H */
