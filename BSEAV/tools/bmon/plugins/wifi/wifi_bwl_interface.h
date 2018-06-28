/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef BMON_WIFI_H
#define BMON_WIFI_H

#ifdef BWL_SUPPORT
struct ether_addr { /* needed by bwl.h */
    unsigned char octet[8];
};
#include "bwl.h"
#include "wifi.h"

#define WIFI_INTERFACE_NAME   "wlan0"
#define FILENAME_WIFI_RECORD  "wifi_record.txt"

/* WL_RSPEC defines for rate information */
#define WL_RSPEC_RATE_MASK     0x000000FF                  /* rate or HT MCS value */
#define WL_RSPEC_VHT_MCS_MASK  0x0000000F                  /* VHT MCS value */
#define WL_RSPEC_VHT_NSS_MASK  0x000000F0                  /* VHT Nss value */
#define WL_RSPEC_VHT_NSS_SHIFT 4                           /* VHT Nss value shift */
#define WL_RSPEC_TXEXP_MASK    0x00000300
#define WL_RSPEC_TXEXP_SHIFT   8
#define WL_RSPEC_BW_MASK       0x00070000                  /* bandwidth mask */
#define WL_RSPEC_BW_SHIFT      16                          /* bandwidth shift */
#define WL_RSPEC_STBC          0x00100000                  /* STBC encoding, Nsts = 2 x Nss */
#define WL_RSPEC_TXBF          0x00200000                  /* bit indicates TXBF mode */
#define WL_RSPEC_LDPC          0x00400000                  /* bit indicates adv coding in use */
#define WL_RSPEC_SGI           0x00800000                  /* Short GI mode */
#define WL_RSPEC_ENCODING_MASK 0x03000000                  /* Encoding of Rate/MCS field */
#define WL_RSPEC_OVERRIDE_RATE 0x40000000                  /* bit indicate to override mcs only */
#define WL_RSPEC_OVERRIDE_MODE 0x80000000                  /* bit indicates override both rate & mode */

/* WL_RSPEC_ENCODING field defs */
#define WL_RSPEC_ENCODE_RATE 0x00000000                    /* Legacy rate is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_HT   0x01000000                    /* HT MCS is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_VHT  0x02000000                    /* VHT MCS and Nss is stored in RSPEC_RATE_MASK */

/* WL_RSPEC_BW field defs */
#define WL_RSPEC_BW_UNSPECIFIED 0
#define WL_RSPEC_BW_20MHZ       0x00010000
#define WL_RSPEC_BW_40MHZ       0x00020000
#define WL_RSPEC_BW_80MHZ       0x00030000
#define WL_RSPEC_BW_160MHZ      0x00040000
#define WL_RSPEC_BW_10MHZ       0x00050000
#define WL_RSPEC_BW_5MHZ        0x00060000
#define WL_RSPEC_BW_2P5MHZ      0x00070000


#define MAX_CONNECTED_STAS 10


/* trellis/connectivity/netapp/netapp/include/netapp.h */
typedef enum
{
    NETAPP_SUCCESS = 0,                                    //!< Success.
    NETAPP_FAILURE,                                        //!< General failure.
    NETAPP_INVALID_PARAMETER,                              //!< Invalid parameter.
    NETAPP_NULL_PTR,                                       //!< Null handle detected or invalid state.
    NETAPP_OUT_OF_MEMORY,                                  //!< Malloc has failed.
    NETAPP_NOT_IMPLEMENTED,                                //!< Function not implemented.
    NETAPP_NETWORK_UNREACHABLE,                            //!< Unable to reach destination network.
    NETAPP_SOCKET_ERROR,                                   //!< Error creating the Linux socket.
    NETAPP_TIMEOUT,                                        //!< Timeout error occurred.
    NETAPP_DHCP_FAILURE,                                   //!< Failure to fetch DHCPD address.
    NETAPP_HOST_NOT_FOUND,                                 //!< Not able to find host in DNS server.
    NETAPP_CANCELED,                                       //!< The function was canceled.
    NETAPP_INCORRECT_PASSWORD,                             //!< Incorrect password provided.
    NETAPP_INVALID_PIN,                                    //!< Invalid WPS pin used.
    NETAPP_NOT_FOUND,                                      //!< Requested item was not found.
    NETAPP_NOT_SUPPORTED,                                  //!< Requesting an API or function that was
                                                           //!< not supported/compiled in.
    NETAPP_WPS_MULTIPLE_AP_FOUND,                          //!< Found more than one AP in WPS PBC (overlap).
    NETAPP_SCAN_EMPTY,                                     //!< Scan was complete and no access points found.
    NETAPP_INVALID_STATE,                                  //!< Calling the API when the system is in an invalid state.
    NETAPP_WPS_2_ERR_INCOMPATIBLE,                         //!< WPS detected an AP that supports a WPS 1.0
                                                           //!< depreciated setting that is not supported in
                                                           //!< WPS 2.0. The application should restart WPS
                                                           //!< with NETAPP_SETTINGS.bWPS2_0 set to false.
    NETAPP_WFD_SESSION_NOT_AVAILABLE,                      //!< WFD Session is not available, reported from a
                                                           //!< P2P connection attempt that failed because the
                                                           //!< peer is not ready to connect.
    NETAPP_DATABASE_ERROR                                  //!< There was a problem reading or writing to the
                                                           //!< backend database, file could be corrupted.
} NETAPP_RETCODE;

/* from trellis/connectivity/netapp/netapp/src/netapp_bwl.h */
#define BWL_API_CHECK( api, tRetCode )                                              \
    {                                                                               \
        int result = api;                                                           \
        switch (result)                                                             \
        {                                                                           \
            case 0:                                                                 \
                tRetCode = NETAPP_SUCCESS;                                          \
                break;                                                              \
            case BWL_ERR_PARAM:                                                     \
                tRetCode = NETAPP_INVALID_PARAMETER;                                \
                break;                                                              \
            case BWL_ERR_ALLOC:                                                     \
                tRetCode = NETAPP_OUT_OF_MEMORY;                                    \
                break;                                                              \
            case BWL_ERR_CANCELED:                                                  \
                tRetCode = NETAPP_CANCELED;                                         \
                break;                                                              \
            default:                                                                \
                printf( "%s BWL API failed, Line=%d! \n", __FUNCTION__, __LINE__ ); \
                tRetCode = NETAPP_FAILURE;                                          \
                break;                                                              \
        }                                                                           \
        if (tRetCode != NETAPP_SUCCESS)                                             \
        {                                                                           \
            goto err_out;                                                           \
        }                                                                           \
    }

typedef struct
{
    char         name[16]; /* TX VHT SGI 1x1 */
    unsigned int level[BMON_NUM_MCS];
} antenna1_t;

typedef struct
{
    antenna1_t tx[8]; /* 1st 4 are TX VHT 1x1,2x2,3x3,4x4; 2nd 4 are SGI */
    antenna1_t rx[8]; /* 1st 4 are RX VHT 1x1,2x2,3x3,4x4; 2nd 4 are SGI */
} antennas_t;

typedef struct
{
    antennas_t antennas;
    unsigned long int tx_mpdus;
    unsigned long int tx_ampdus;
    unsigned long int tx_mpduperampdu;
    unsigned long int retry_ampdu;
    unsigned long int retry_mpdu;
    unsigned long int txbar;
    unsigned long int rxba;
    unsigned long int rx_mpdus;
    unsigned long int rx_ampdus;
    unsigned long int rx_mpduperampdu;
    unsigned long int rxbar;
    unsigned long int txba;
} ampdu_data_t;

typedef struct
{
    uint32_t glitchcnt;         /**< normalized as per second count */
    uint32_t badplcp;           /**< normalized as per second count */
    uint8_t* ccastats;          /**< normalized as 0-100 */
    uint32_t ccasize;           /**< size of the cca buffer */
    int8_t   bgnoise;           /**< background noise level (in dBm) */
    uint16_t chanspec;          /**< ctrl chanspec of the interface */
    uint16_t channum;           /**< translated chan num of the interface */
    uint32_t timestamp;         /**< time stamp at which the stats are collected */
    uint32_t bphy_glitchcnt;    /**< normalized as per second count */
    uint32_t bphy_badplcp;      /**< normalized as per second count */
    uint8_t  chan_idle;         /**< normalized as 0~100 */
    uint32_t tx;                /**< transmit duration */
    uint32_t ibss, obss;        /**< rx from within/outside bss */
    uint32_t busy;              /**< inbss + obss + goodtx + badtx + obss + nocat + nopkt */
    uint32_t avail;             /**< inbss + goodtx + badtx + txop */
    uint32_t nwifi;             /* non-wifi receive (nocat + nopkt) */
} chanim_data_t;

/** RSSI per antenna */
typedef struct {
    unsigned int version;                   /**< version field */
    unsigned int count;                     /**< number of valid antenna rssi */
    signed char rssi_ant[BMON_NUM_ANTENNAS];   /**< rssi per antenna */
} phy_rssi_ant_t;

int bmon_wifi_get_counters( const char *ifname, WiFiCounters_t *pCounters /* Counters structure */ );
int bmon_wifi_get_connected_info( const char *ifname, ScanInfo_t *tScanInfo );
int bmon_wifi_get_rssi_ant( const char *ifname, phy_rssi_ant_t *pRssiAnt );
int bmon_wifi_read_ampdu_data( ampdu_data_t *pAmpduData );
int bmon_wifi_clear_ampdu( const char *ifname );
int bmon_wifi_ampdu_get_report( const char *ifname );
int bmon_wifi_chanim_get_report( const char *ifname, chanim_data_t *data );
int bmon_wifi_chanim_get_count( const char *ifname, unsigned int *num);
int bmon_get_proc_net_tcp_bytes ( unsigned int *TxBytes, unsigned int *RxBytes );
int bmon_wifi_ampdu_write_to_file( const char *buffer, const char *filename );
char *bmon_wifi_ampdu_parse_for_totals( const char *buffer, const char *filename, int RxOrTx );
char *bmon_wifi_ampdu_parse_vmt( const char *buffer, const char *keyword, char *values, char *percentages, const char *filename );
int bmon_wifi_get_cca_stats( const char *ifname, BWLGetCcaStats_t *lBWLGetCcaStats );
ScanInfo_t* bmon_wifi_get_scanresults(const char     *ifname,unsigned int *num);
void bmon_wifi_trigger_scan(const char     *ifname);
int bmon_wifi_get_connected_stas(const char     *ifname,unsigned int   outputListLen,BWL_MAC_ADDRESS *outputList);
int bmon_wifi_get_apsta(const char     *ifname,unsigned int   pulApSta);
int bmon_wifi_get_mode(const char     *ifname,unsigned int   *pulApSta);

int bmon_wifi_get_samples( const char *ifname, unsigned int tSampleType, WiFiSamples_t *pSamples /* Samples structure */ );
int bmon_wifi_get_driver_version(const char *ifname,char *strDriverVersion,int   iDriverVersionLen);
int bmon_wifi_get_revs(const char *ifname,RevInfo_t  *tRevInfo);
char *bmon_wifi_get_BandwidthStr(Bandwidth_t bandwidth);
int bmon_wifi_get_NRate(const char *ifname,uint32_t   *pNRate);
char*  bmon_wifi_get_rate_print(char    *rate_buf,uint32_t rspec);
int bmon_wifi_get_pm2_rcv_dur(const char *ifname,uint32_t   *pm2_rcv_dur);
int bmon_wifi_set_pm2_rcv_dur(const char *ifname,uint32_t   pm2_rcv_dur);



#endif /* BWL_SUPPORT */

#endif /* BMON_WIFI_H */
