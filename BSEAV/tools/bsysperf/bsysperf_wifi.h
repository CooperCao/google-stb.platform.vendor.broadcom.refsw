/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef __BSYSPERF_WIFI_H__
#define __BSYSPERF_WIFI_H__

#if 0
#include <proto/ethernet.h>
#endif
struct ether_addr { /* CAD needed by bwl.h */
    unsigned char octet[8];
};
#include "bwl.h"

#define WIFI_INTERFACE_NAME   "wlan0"
#define ANTENNA_MAX_LEVELS    12
#define AMPDU_TX_RX_MAX       16 /*1x1,2x2,3x3,4x4 for VHT and SVG ... for TX and RX */
#define BSYSPERF_RSSI_ANT_MAX 4

typedef struct
{
    char         name[16]; /* TX VHT SGI 1x1 */
    unsigned int level[ANTENNA_MAX_LEVELS];
} ANTENNA1_T;

typedef struct
{
    ANTENNA1_T tx[8]; /* 1st 4 are TX VHT 1x1,2x2,3x3,4x4; 2nd 4 are SGI */
    ANTENNA1_T rx[8]; /* 1st 4 are RX VHT 1x1,2x2,3x3,4x4; 2nd 4 are SGI */
} ANTENNAS_T;

typedef struct
{
    ANTENNAS_T antennas;
    unsigned long int tx_mpdus;
    unsigned long int tx_ampdus;
    unsigned long int tx_mpduperampdu;
    unsigned long int rx_mpdus;
    unsigned long int rx_ampdus;
    unsigned long int rx_mpduperampdu;
} AMPDU_DATA_T;

/** RSSI per antenna */
typedef struct {
    unsigned int version;                   /**< version field */
    unsigned int count;                     /**< number of valid antenna rssi */
    signed char rssi_ant[BSYSPERF_RSSI_ANT_MAX];   /**< rssi per antenna */
} BSYSPERF_PHY_RSSI_ANT_T;

int         Bsysperf_WifiInit( const char * ifname );
int         Bsysperf_WifiUninit( void );
int         Bsysperf_WifiGetRevs( const char *ifname, RevInfo_t *tRevInfo );
uint32_t    Bsysperf_WifiScanStart( const char *ifname );
int         Bsysperf_WifiScanGetResults( const char *ifname );
int         Bsysperf_WifiGetConnectedInfo( const char *ifname, ScanInfo_t *tScanInfo );
int         Bsysperf_WifiGetRate( const char *ifname, int *pRate );
char       *Bsysperf_WifiBandwidthStr( Bandwidth_t bandwidth );
char       *Bsysperf_WifiDbStrength( int signal_strength );
int         Bsysperf_WifiGetCounters( const char *ifname, WiFiCounters_t *pCounters );
int         Bsysperf_WifiGetNRate( const char *ifname, uint32_t *nrate );
char       *wl_rate_print(char *rate_buf, uint32_t rspec);
char       *Bsysperf_WifiGetVersion( const char *ifname );
int         Bsysperf_GetScanResultsWl( void );
int         Bsysperf_WifiScanResultsFormat( ScanInfo_t *pScanInfo );
int         Bsysperf_WifiAmpdu_GetReport( const char *ifname );
int         Bsysperf_WifiClearAmpdu( const char *ifname );
int         Bsysperf_WifiReadAmpduData( AMPDU_DATA_T *pAmpduData );
int         Bsysperf_WifiOutputAntennasHtml( ANTENNAS_T *antennas );
int         Bsysperf_WifiPrintAntennas( ANTENNAS_T *antennas );
int         Bsysperf_WifiGetRssiAnt( const char *ifname, BSYSPERF_PHY_RSSI_ANT_T *pRssiAnt );
int         Bsysperf_WifiGetDriverVersion( const char *ifname, char *strDriverVersion, int iDriverVersionLen );
int         Bsysperf_WifiClearCounters( const char *ifname );

#endif /* __BSYSPERF_WIFI_H__ */
