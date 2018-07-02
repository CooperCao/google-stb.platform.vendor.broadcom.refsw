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
#ifndef WIFI_H
#define WIFI_H

#define WIFI_DE
#define cJSON_FORMAT

#define WIFI_PLUGIN_VERSION       "0.2"
#define WIFI_PLUGIN_NAME          "wifi"
#define WIFI_PLUGIN_DESCRIPTION   "Wireless 802.11 plugin"
#define WIFI_INTERFACE_NAME "wlan0"
//#define WIFI_PLUGIN_DEBUG

#define BMON_NUM_ANTENNAS 4                             /* 1x1, 2x2, 3x3, 4x4 */
#define BMON_NUM_MCS      12                            /* 0 thru 11 */
#define BMON_VHT_VARIANTS 2                             /* RX VHT and RX VHT SGI */
#define BMON_RX_TX        2
#define STRING_SIZE       256
typedef char CONNECTED_MAC_ADDRESS[20];


typedef struct bmon_wifi_t
{
    char               plugin_version[16];
    unsigned char      active;                             /* 1 if the wifi is actively transferring data */
    unsigned char      dir;                                /* Enc Value : bit 1->stream-out, bit 0->stream-in */
    unsigned int       tx_instant_rate;                    /* number of bytes transferred so far (see ifconfig) */
    unsigned int       rx_instant_rate;                    /* number of bytes received    so far (see ifconfig) */
    short int          rssi;                               /* Received Signal Strength Indicator */
    short int          snr;                                /* Signal To Noise ratio */
    unsigned int       per;                                /* Packet Error Rate */
    unsigned char      ch_utiliziation;                    /* This needs to be derived from new WLAN PHY parameters */
    unsigned short int retrans_count;                      /* WiFi Retx */
    unsigned int       tx_queue;                           /* summation of active tx values from /proc/net/tcp */
    unsigned int       rx_queue;                           /* summation of active rx values from /proc/net/tcp */
    unsigned char      ampdu_data[BMON_RX_TX][BMON_VHT_VARIANTS][BMON_NUM_ANTENNAS][BMON_NUM_MCS];
    unsigned short int channel_active_time_msec;
} bmon_wifi_t;

typedef struct wifi_tx_stats_t
{
    unsigned int txframes;
    unsigned int txretrans;
    unsigned int txerror;
    unsigned int txserr;
    unsigned int txnobuf;
    unsigned int txnoassoc;
    unsigned int txrunt;
} wifi_tx_stats_t;

typedef struct wifi_rx_stats_t
{
    unsigned int rxframe;
    unsigned int rxerror;
    unsigned int rxnobuf;
    unsigned int rxnondata;
    unsigned int rxbadds;
    unsigned int rxbadcm;
    unsigned int rxfragerr;
    unsigned int rxrunt;
    unsigned int rxnoscb;
    unsigned int rxbadproto;
    unsigned int rxbadsrcmac;
    unsigned int rxbadda;
} wifi_rx_stats_t;

typedef struct wifi_txampdu_stats_t
{
    unsigned int txampdu;
    unsigned int txmpdu;
    unsigned int txmpduperampdu;
    unsigned int retry_ampdu;
    unsigned int retry_mpdu;
    unsigned int txbar;
    unsigned int rxba;
} wifi_txampdu_stats_t;

typedef struct wifi_rxampdu_stats_t
{
    unsigned int rxampdu;
    unsigned int rxmpdu;
    unsigned int rxmpduperampdu;
    unsigned int rxbar;
    unsigned int txba;
} wifi_rxampdu_stats_t;

typedef struct wifi_ampdu_chart_t
{
    unsigned int      ampdu_data[BMON_RX_TX][BMON_VHT_VARIANTS][BMON_NUM_ANTENNAS][BMON_NUM_MCS];
} wifi_ampdu_chart_t;

typedef struct spectrum_data_t
{
    char               SSID[33];
    unsigned char      octet[8];
    int                lRSSI;
    int                ulPhyNoise;
    unsigned int       ulChan;
    unsigned int       ulPrimChan;
    unsigned int       ulBandwidth;
    int                lRate;
    unsigned int       ul802_11Modes;
} spectrum_data_t;
typedef struct wifi_connected_sta_t
{
    CONNECTED_MAC_ADDRESS *address;
    unsigned char      numberSTAs;
} wifi_connected_sta_t;

#ifdef WIFI_DE
typedef struct DE_CurrentOperatingClasses_t{
    unsigned int        currentOperatingClass;
    unsigned int        channel;
    int                 maxTxPower;
}DE_CurrentOperatingClasses_t;

typedef struct DE_STAList_t{
    unsigned char                  MACaddress[20];
    unsigned char                  HTCapabilities[1]; // size needs to change
    unsigned char                  VHTCapabilities[6]; // size needs to change
    unsigned char                  HECapabilities[14]; // size needs to change
    unsigned int                   statusCode;
    unsigned int                   reasonCode;
    unsigned int                   lastDataDownlinkRate;
    unsigned int                   lastDataUplinkRate;
    unsigned int                   utilizationReceive;
    unsigned int                   utilizationTransmit;
    unsigned int                   estMACDataRateDownlink;
    unsigned int                   estMACDataRateUplink;
    unsigned int                   signalStrength;
    unsigned int                   lastConnectTime;
    unsigned int                   bytesSent;
    unsigned int                   bytesReceived;
    unsigned int                   packetsSent;
    unsigned int                   packetsReceived;
    unsigned int                   errorsSent;
    unsigned int                   errorsReceived;
    unsigned int                   retransCount;
    unsigned int                   measurement[2]; // size needs to change
    char                           IPv4Address[STRING_SIZE];
    char                           IPv6Address[STRING_SIZE];
    char                           hostName[STRING_SIZE];
}DE_STAList_t;

typedef struct DE_BSSList_t{
    unsigned char       BSSID[20];
    char                SSID[STRING_SIZE];
    char                BSSEnabled[6];
    unsigned int        lastChange;
    unsigned int        unicastBytesSent;
    unsigned int        unicastBytesReceived;
    unsigned int        multicastBytesSent;
    unsigned int        multicastBytesReceived;
    unsigned int        broadcastBytesSent;
    unsigned int        broadcastBytesReceived;
    unsigned int        numberOfSTA;
    struct DE_STAList_t **staList;
    unsigned char       estServiceParamBE[3];
    unsigned char       estServiceParamBK[3];
    unsigned char       estServiceParamVI[3];
    unsigned char       estServiceParamVO[3];
}DE_BSSList_t;

typedef struct DE_OperatingClasses_t{
    unsigned int                                operatingClass;
    int                                         MaxTxPower;
    unsigned int                                sizeOfNonOperable;
    unsigned int                                *NonOperable;
}DE_OperatingClasses_t;

typedef struct DE_Capabilites_t{
    unsigned char                   HTCapabilities[1]; // size needs to change
    unsigned char                   VHTCapabilities[6]; // size needs to change
    unsigned char                   HECapabilities[14]; // size needs to change
    unsigned char                   numberOfOperatingClasses;
    struct DE_OperatingClasses_t    **operatingClasses;
}DE_Capabilites_t;

typedef struct DE_BackHaulSTA_t{
    unsigned char       backHaulSTA[6];
}DE_BackHaulSTA_t;

typedef struct DE_NeighborList_t{
    unsigned char       BSSID[8];
    char                SSID[STRING_SIZE];
    unsigned int        signalStrength;
    unsigned int        channelBandwidth;
    unsigned int        channelUtilization;
    unsigned int        stationCount;

}DE_NeighborList_t;

typedef struct DE_ChannelScan_t{
    unsigned int                channel;
    unsigned int                utilization;
    unsigned int                sizeofoperatingClass;
    unsigned int                *operatingClass; // size needs to change
    unsigned int                noise;
    unsigned int                numberOfNeighbors;
    struct DE_NeighborList_t    **neighborList;
}DE_ChannelScan_t;

typedef struct DE_ScanResultList_t{
    unsigned int                numberOfChannels;
    unsigned int                lastScan;
    struct DE_ChannelScan_t     **channelList;
}DE_ScanResultList_t;

typedef struct DE_UnassociatedSTAList_t{
    unsigned char       MACAddress[8];
    unsigned int        signalStrength;
}DE_UnassociatedSTAList_t;

typedef struct DE_RadioList_t{
    char                                    radioID[STRING_SIZE];
    char                                    radioEnabled[6];
    unsigned int                            noise;
    unsigned int                            utilization;
    unsigned int                            utilizationOther;
    unsigned int                            numberOfCurrentOperatingClasses;
    struct DE_CurrentOperatingClasses_t     **currentOperatingClasses;
    struct DE_Capabilites_t                 *capabilities;
    struct DE_BackHaulSTA_t                 *backhaulSTA;
    struct DE_UnassociatedSTAList_t         **unassociatedSTAList;

    unsigned int                            numberOfBSS;
    struct DE_BSSList_t                     **BSSList;


    struct DE_ScanResultList_t              *scanResultList;
    unsigned int                            numberOfunassociatedSTAList;

}DE_RadioList_t;

typedef struct DE_devices_t{
    char                    devID[STRING_SIZE];
    unsigned int            MultiAPCap;
    unsigned int            NumberOfRadios;
    struct DE_RadioList_t     **RadioList;
}DE_devices_t;

typedef struct wifi_dataelements_t{
    char                        deID[STRING_SIZE];
    char                        controllerID[STRING_SIZE];
    unsigned int                numberOfDevices;
    struct DE_devices_t         **deviceList;
}wifi_dataelements_t;

#endif
typedef struct wifi_current_stats_t{
    char            SSID[STRING_SIZE];
    char            driverVersion[STRING_SIZE];
    unsigned char   MACaddress[STRING_SIZE];
    unsigned int    deviceID;
    int             RSSI;
    int             phy_rssi_ant[4];
    int             phy_noise;
    unsigned int    wifi80211Modes;
    unsigned int    channel;
    unsigned int    primaryChannel;
    unsigned int    locked;
    unsigned int    WPS;
    unsigned int    rate;
    char            bandwidth[STRING_SIZE];
    char            channelSpec[STRING_SIZE];
    char            NRate[STRING_SIZE];
    unsigned int    chipNumber;
    unsigned int    pciVendorID;
    unsigned int    phyRate;
    unsigned int    authenticationType;
    unsigned int    apsta;
}wifi_current_stats_t;
typedef struct wifi_power_stats_t{
    unsigned int    POWSWindow;
}wifi_power_stats_t;

#define BMON_CHANIM_STATS_V2 2
#define BMON_CCASTATS_V2_MAX 9

typedef struct wifi_chanim_stats_t{
    char bgnoise;                   /**< background noise level (in dBm) */
    unsigned short chanspec;        /**< ctrl chanspec of the interface */
    unsigned short channum;         /**< ctrl chan num of the interface */
    unsigned int timestamp;         /**< time stamp at which the stats are collected */
    unsigned char chan_idle;        /**< normalized as 0~255 */
    unsigned int tx;
    unsigned int ibss;
    unsigned int obss;
    unsigned int busy;
    unsigned int avail;
    unsigned int nwifi;
}wifi_chanim_stats_t;

#endif /* ifndef WIFI_H */
