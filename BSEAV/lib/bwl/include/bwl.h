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
#include <stdio.h>
#include <stdlib.h>

#ifndef BWL_H__
#define BWL_H__

#include <stdint.h>
#include <stdbool.h>
#define BWL_VERSION_STR "0.902"

//---------------------------------------------------------------------------
// BWL specifics Errors
//---------------------------------------------------------------------------
#define BWL_ERR_SUCCESS     ( 0)
#define BWL_ERR_USAGE       (-1)
#define BWL_ERR_IOCTL       (-2)
#define BWL_ERR_PARAM       (-3)
#define BWL_ERR_CMD         (-4)
#define BWL_ERR_ALLOC       (-5)
#define BWL_ERR_CANCELED    (-6)
#define BWL_ERR_TIMEOUT     (-7)
#define BWL_ERR_PARTIAL     (-8)
#define BWL_ERR_GENERIC   (-256)
//#define BWL_CHECK_ERR(x)    if((x)) {goto BWL_EXIT;}
#define BWL_CHECK_ERR(x)    do{ if((x)) {BWL_DisplayError((x), (char*)__FUNCTION__, __FILE__, __LINE__); goto BWL_EXIT;}} while(0);


#define BWL_INVALID_PIN     0xFFFFFFFF /* some big number */
#define BWL_MAX_CHANNEL     32
#define DOT11_MAX_SSID_LEN  32
#define SIZE_64_BYTES       64
#define ETHER_TYPE_BRCM     0x886c      /* Broadcom Corp. */
#define WSEC_PSK_LEN        64
#define MAC_ADDR_LEN        18
#define BWL_MAX_ANTENNAS    4

#define CCA_THRESH_MILLI    14
#define ASSERT              assert

#define BWL_DEFAULT_SCAN_DWELL_TIME  (-1)

//#define BWL_DEBUG
#ifdef BWL_DEBUG
#ifndef PRINTF
#define PRINTF(x) printf(x)
#endif
#else
#ifndef PRINTF
#define PRINTF(x)
#endif
#endif

/* Network Operating Mode */
typedef enum eNetOpMode
{
    eNetOpModeAdHoc =   0x000,  /* Ad Hoc Mode */
    eNetOpModeInfra =   0x001   /* Infrastructure Mode */
} NetOpMode_t;

typedef enum e802_11Modes
{
    e802_11_none  =   0x000,
    e802_11_a     =   0x001,
    e802_11_b     =   0x002,
    e802_11_g     =   0x004,
    e802_11_n     =   0x008,
    e802_11_ac    =   0x010
} e802_11Modes_t;

typedef enum eWoWLMask
{
    eWoWLMask_None          = 0x00,
    eWoWLMask_Magic         = 0x01,
    eWoWLMask_Disassoc      = 0x02,
    eWoWLMask_LossOfBeacon  = 0x04,
    eWoWLMask_NetPattern    = 0x08,
} WoWLMask_t;


typedef enum eCryptoAlgo
{
    eCryptoAlgoOff,
    eCryptoAlgoWep1,
    eCryptoAlgoWep128,
    eCryptoAlgoTkip,
    eCryptoAlgoAesCcm,
    eCryptoAlgoAesOcbMsdu,
    eCryptoAlgoAesOcbMpdu,
    eCryptoAlgoNalg,
    eCryptoAlgoInvalid
} CryptoAlgo_t;


/* Bitmask of Ciphers */
typedef enum eWSec
{
    eWSecInvalid    = 0xff,
    eWSecNone       = 0x01,
    eWSecWep        = 0x02,
    eWSecTkip       = 0x04,
    eWSecAes        = 0x08,
    eWSecAll        = 0x10,
} WSec_t;

typedef enum eWpaAuth
{
    eWpaAuthDisabled    = 0x00,
    eWpaAuthNone        = 0x01,
    eWpaAuthWpaUnsp     = 0x02,
    eWpaAuthWpaPsk      = 0x04,
    eWpaAuthWpa2Unsp    = 0x08,
    eWpaAuthWpa2Psk     = 0x10,
    eWpaAuthInvalid     = 0xff
} WpaAuth_t;

typedef struct sWpaInfo
{
    WSec_t  Cipher;
    uint8_t Akm;
} WpaInfo_t;

typedef struct sWSecPmk
{
    uint16_t KeyLen;        /* octets in key material */
    uint16_t Flags;          /* key handling qualification */
    uint8_t  Key[WSEC_PSK_LEN];  /* PMK material */
} WSecPmk_t;

#define DPT_FNAME_LEN       48  /* Max length of friendly name */

/* structure for dpt friendly name */
typedef struct sDptFName
{
    uint8_t Len;                /* length of friendly name */
    uint8_t Name[DPT_FNAME_LEN];  /* friendly name */
} DptFName_t;

typedef enum eAuthType
{
    eAuthTypeOpen,
    eAuthTypeShare,
    eAuthTypeOpenShare,
    eAuthTypeInvalid
} AuthType_t;

typedef enum eWpaSup
{
    eWpaSupExternal = 0,    /* use external supplicant */
    eWpaSupInternal = 1     /* use internal supplicant */
} WpaSup_t;

typedef enum eBand
{
    eBand2G     = 0x1,
    eBand5G     = 0x2,
    eBandAuto   = 0x4,
    eBandAll    = 0x8,
    eBandInvalid = 0x0
} Band_t;


typedef enum eBandwidth
{
    eBandwidthInvalid,
    eBandwidth10MHz,
    eBandwidth20MHz,
    eBandwidth40MHz,
    eBandwidth80MHz,
    eBandwidth160MHz
} Bandwidth_t;

typedef enum eAntMode
{
    eAntModeRx,
    eAntModeTx,
} AntMode_t;

typedef enum eMimoBwCap
{
    eMimoBwCap20MHz_BothBands = 0,
    eMimoBwCap40MHz_BothBands,
    eMimoBwCap20MHzG_40MHz5G,
} MimoBwCap_t;

typedef enum eMimoTxBw
{
    eMimoTxBw_20MHz = 2,
    eMimoTxBw_20MHzUpper,
    eMimoTxBw_40MHz,
    eMimoTxBw_40MHzDup
} MimoTxBw_t;

typedef enum eSideband
{
    eSideband_None,
    eSideband_Lower,
    eSideband_Upper
} Sideband_t;

typedef enum eDfsSpec
{
    eDfsSpec_None,
    eDfsSpec_Loose,
    eDfsSpec_Strict
} DfsSpec_t;


typedef struct BWL_P_Handle *BWL_Handle;

typedef char SSID_t[DOT11_MAX_SSID_LEN + 1]; /* +1 for EOS */

typedef struct sCredential
{
    NetOpMode_t     eNetOpMode;     /* 0 == ad hoc, 1 == infrastructure */
    AuthType_t      eAuthType;      /* 0 == open, 1 == share, both */
    WpaAuth_t       eWpaAuth;       /* disable, none, wpa, wpapsk, wpa2, wpa2psk */
    WSec_t          eWSec;          /* wep, tkip, aes */
    WpaSup_t        eWpaSup;        /* 0 == external supplicant, 1 == internal supplicant */
    uint32_t        ulWepIndex;     /* wep key index, ie, primary key */
    uint32_t        ulKeyLen;       /* wep key, or pmk length */
    char            acKey[ SIZE_64_BYTES + 1 ]; /* wep key, pmk */
    char            acSSID[DOT11_MAX_SSID_LEN + 1 ]; /* +1 for EOS */
    struct ether_addr *peBSSID; /* BSSID of the AP */
} Credential_t;

#define BWL_CSPEC_LEN 20

typedef struct sScanInfo
{
    int32_t         lRSSI;
    uint32_t        ulChan;
    uint32_t        ulPrimChan;
    int32_t         lPhyNoise;
    uint32_t        ulAuthType;
    uint32_t        ul802_11Modes;

    struct ether_addr BSSID;

    Credential_t    tCredentials;
    bool            bLocked;
    bool            bWPS;
    int32_t         lRate;
    Bandwidth_t     tBandwidth;
    char            cChanSpec[BWL_CSPEC_LEN+1];
    int32_t         lSNR;
} ScanInfo_t;

typedef struct sDptSta
{
    struct ether_addr mac;

    uint8_t     FName[48];
    uint32_t    ulRssi;
    uint32_t    ulTxFailures;
    uint32_t    ulTxPkts;
    uint32_t    ulRxUcastPkts;
    uint32_t    ulTxRate;
    uint32_t    ulRxRate;
    uint32_t    ulRxDecryptSucceeds;
    uint32_t    ulRxDecryptFailures;
} DptSta_t;

typedef struct sDptList
{
    uint32_t    ulNum;
    DptSta_t    Sta[4];
} DptList_t;


typedef struct sDptCredential
{
    WpaAuth_t   eWpaAuth;   /* disable, none, wpa, wpapsk, wpa2, wpa2psk */
    WSec_t      eWSec;      /* wep, tkip, aes */
    WSecPmk_t   Pmk;        /* PMK */
    DptFName_t  FName;      /* Friendly name */
} DptCredential_t;

typedef struct sScanParams
{
    int32_t     lActiveTime;    /* -1 use default, dwell time per channel for
                                 * active scanning
                                 */
    int32_t     lPassiveTime;   /* -1 use default, dwell time per channel
                                 * for passive scanning
                                 */
    int32_t     lHomeTime;      /* -1 use default, dwell time for the home channel
                                 * between channel scans
                                 */
    char        *pcSSID;        /* The name of an AP that we want to fetch
                                 * scanned info from. This is usefull for
                                 * fetching credentials from hidden AP's
                                 */
}ScanParams_t;

typedef struct sBWLGetCcaStats
{
    unsigned int ChannelRxTimeMsec;
    unsigned int ChannelTxTimeMsec;
    unsigned int ChannelActiveTimeMsec;
    unsigned int ChannelBusyTimeMsec;
    unsigned int ChannelInUse;
} BWLGetCcaStats_t;

/*
 * Structure for passing hardware and software
 * revision info up from the driver.
 */
typedef struct sRevInfo
{
    uint32_t     ulVendorId;    /* PCI vendor id */
    uint32_t     ulDeviceId;    /* device id of chip */
    uint32_t     ulRadioRev;    /* radio revision */
    uint32_t     ulChipRev;     /* chip revision */
    uint32_t     ulCoreRev;     /* core revision */
    uint32_t     ulBoardId;     /* board identifier (usu. PCI sub-device id) */
    uint32_t     ulBoardVendor; /* board vendor (usu. PCI sub-vendor id) */
    uint32_t     ulBoardRev;    /* board revision */
    uint32_t     ulDriverRev;   /* driver version */
    uint32_t     ulUcodeRev;    /* microcode version */
    uint32_t     ulBus;         /* bus type */
    uint32_t     ulChipNum;     /* chip number */
    uint32_t     ulPhyType;     /* phy type */
    uint32_t     ulPhyRev;      /* phy revision */
    uint32_t     ulAnaRev;      /* anacore rev */
    uint32_t     ulChipPkg;     /* chip package info */
} RevInfo_t;

/* Supplicant Status for WPA */
typedef enum eSupStatus
{
    eSupStatusDisconnected  = 0,
    eSupStatuseConnecting,
    eSupStatusConnected,
    eSupStatusError,
} SupStatus_t;

/* Event API's */
typedef enum eEventId
{
    BWL_E_SET_SSID,         /* indicates status of set SSID */
    BWL_E_JOIN,             /* differentiates join IBSS from found (WLC_E_START) IBSS */
    BWL_E_START,            /* STA founded an IBSS or AP started a BSS */
    BWL_E_AUTH,             /* 802.11 AUTH request */
    BWL_E_AUTH_IND,         /* 802.11 AUTH indication */
    BWL_E_DEAUTH,           /* 802.11 DEAUTH request */
    BWL_E_DEAUTH_IND,       /* 802.11 DEAUTH indication */
    BWL_E_ASSOC,            /* 802.11 ASSOC request */
    BWL_E_ASSOC_IND,        /* 802.11 ASSOC indication */
    BWL_E_REASSOC,          /* 802.11 REASSOC request */
    BWL_E_REASSOC_IND,      /* 802.11 REASSOC indication */
    BWL_E_DISASSOC,         /* 802.11 DISASSOC request */
    BWL_E_DISASSOC_IND,     /* 802.11 DISASSOC indication */
    BWL_E_QUIET_START,      /* 802.11h Quiet period started */
    BWL_E_QUIET_END,        /* 802.11h Quiet period ended */
    BWL_E_BEACON_RX,        /* BEACONS received/lost indication */
    BWL_E_LINK,             /* generic link indication */
    BWL_E_MIC_ERROR,        /* TKIP MIC error occurred */
    BWL_E_NDIS_LINK,        /* NDIS style link indication */
    BWL_E_ROAM,             /* roam attempt occurred: indicate status & reason */
    BWL_E_TXFAIL,           /* change in dot11FailedCount (txfail) */
    BWL_E_PMKID_CACHE,      /* WPA2 pmkid cache indication */
    BWL_E_RETROGRADE_TSF,   /* current AP's TSF value went backward */
    BWL_E_PRUNE,            /* AP was pruned from join list for reason */
    BWL_E_AUTOAUTH,         /* report AutoAuth table entry match for join attempt */
    BWL_E_EAPOL_MSG,        /* Event encapsulating an EAPOL message */
    BWL_E_SCAN_COMPLETE,    /* Scan results are ready or scan was aborted */
    BWL_E_ADDTS_IND,        /* indicate to host addts fail/success */
    BWL_E_DELTS_IND,        /* indicate to host delts fail/success */
    BWL_E_BCNSENT_IND,      /* indicate to host of beacon transmit */
    BWL_E_BCNRX_MSG,        /* Send the received beacon up to the host */
    BWL_E_BCNLOST_MSG,      /* indicate to host loss of beacon */
    BWL_E_ROAM_PREP,        /* before attempting to roam */
    BWL_E_PFN_NET_FOUND,    /* PFN network found event */
    BWL_E_PFN_NET_LOST,     /* PFN network lost event */
    BWL_E_RESET_COMPLETE,
    BWL_E_JOIN_START,
    BWL_E_ROAM_START,
    BWL_E_ASSOC_START,
    BWL_E_IBSS_ASSOC,
    BWL_E_RADIO,
    BWL_E_PSM_WATCHDOG,    /* PSM microcode watchdog fired */
    BWL_E_PROBREQ_MSG,     /* probe request received */
    BWL_E_SCAN_CONFIRM_IND,
    BWL_E_PSK_SUP,         /* WPA Handshake fail */
    BWL_E_COUNTRY_CODE_CHANGED,
    BWL_E_EXCEEDED_MEDIUM_TIME, /* WMMAC excedded medium time */
    BWL_E_ICV_ERROR,       /* WEP ICV error occurred */
    BWL_E_UNICAST_DECODE_ERROR, /* Unsupported unicast encrypted frame */
    BWL_E_MULTICAST_DECODE_ERROR,  /* Unsupported multicast encrypted frame */
    BWL_E_TRACE,
    BWL_E_IF,               /* I/F change (for dongle host notification) */
    BWL_E_RSSI,             /* indicate RSSI change based on configured levels */
    BWL_E_PFN_SCAN_COMPLETE,/* PFN completed scan of network list */
    BWL_E_EXTLOG_MSG,
    BWL_E_ACTION_FRAME,
    BWL_E_PRE_ASSOC_IND,    /* assoc request received */
    BWL_E_PRE_REASSOC_IND,  /* re-assoc request received */
    BWL_E_CHANNEL_ADOPTED,  /* channel adopted */
    BWL_E_AP_STARTED,       /* AP started */
    BWL_E_DFS_AP_STOP,      /* AP stopped due to DFS */
    BWL_E_DFS_AP_RESUME,    /* AP resumed due to DFS */
    BWL_E_ESCAN_RESULT,     /* EScan results full or partial */
    BWL_E_LAST,             /* highest val + 1 for range checking */
} EventId_t;

typedef struct EventMessage_s{
    EventId_t           id;
    int32_t             result;
    struct ether_addr   addr;
    uint32_t            reason; /* If applicable */
} EventMessage_t;

typedef struct ChanSpec_s
{
    uint32_t ulChan;
    Band_t eBand;
    Bandwidth_t eBandwidth;
    Sideband_t eSideband;
} ChanSpec_t;

typedef struct PacketEngineParameters_s
{
    bool bStart; /* decides whether to start or stop the packet engine */
    bool bTxMode; /* decides whether we are in TX or RX mode */
    uint32_t ulIFG; /* inter-frame-gap in microseconds */
    uint32_t ulPacketSize; /* in bytes */
    uint32_t ulNumFrames; /* 0 will indicate continuous mode for TX */
    char acMac[MAC_ADDR_LEN];
} PacketEngineParameters_t;

typedef struct RssiPerAntenna_s
{
    uint32_t ulNumAntennas;
    int32_t lRSSI_ant[BWL_MAX_ANTENNAS];
    int32_t lRSSI;
} RssiPerAntenna_t;

void  BWL_DisplayError(int32_t lErr, const char *pcFunc, char *pcFile, int32_t lLine);
int32_t BWL_GetPresentIFaces(char *args[], uint32_t size);
int32_t BWL_IsPresent(uint32_t *pulPresent, char *pcIfName, uint32_t ulLength);
int32_t BWL_Init(BWL_Handle *phBwl, char *iface);
int32_t BWL_Uninit(BWL_Handle hBwl);
int32_t BWL_GetDriverError(BWL_Handle hBwl, int32_t *plDriverErrCode);
int32_t BWL_Up(BWL_Handle hBwl);
int32_t BWL_Down(BWL_Handle hBwl);
int32_t BWL_IsUp(BWL_Handle hBwl, uint32_t *pulUp);
int32_t BWL_Scan(BWL_Handle hBwl, ScanParams_t *pScanParams);
int32_t BWL_GetScanResults(BWL_Handle hBwl, ScanInfo_t *pData);
int32_t BWL_DisplayScanResults(BWL_Handle hBwl);
int32_t BWL_GetScannedApNum(BWL_Handle hBwl, uint32_t *pNumOfAP);
int32_t BWL_GetConnectedAp(BWL_Handle hBwl, char *pcSSID, uint32_t ulLength, int32_t *plRSSI);
int32_t BWL_GetConnectedInfo(BWL_Handle hBwl, ScanInfo_t *pScanInfo);

typedef struct {
    unsigned int txbyte;                //!< Tx Bytes
    unsigned int txframe;               //!< Tx frames
    unsigned int txretrans;             //!< Tx re-transmissions
    unsigned int txerror;               //!< Tx Errors
    unsigned int rxbyte;                //!< Rx Bytes
    unsigned int rxframe;               //!< Rx Frames
    unsigned int rxerror;               //!< Rx Errors
    unsigned int reset;
    unsigned int txnobuf;
    unsigned int txserr;
    unsigned int txphyerr;
    unsigned int rxnobuf;
    unsigned int rxnondata;
    unsigned int rxbadcm;
    unsigned int rxfragerr;
    unsigned int rxtoolate;
    unsigned int rxbadfcs;
    unsigned int rxfrmtooshrt;
    unsigned int rxf0ovfl;
    unsigned int rxf1ovfl;
    unsigned int pmqovfl;
    unsigned int rxcrc;
} WiFiCounters_t;

typedef char BWL_MAC_ADDRESS[20];

int32_t BWL_GetCounters(BWL_Handle hBwl, WiFiCounters_t *pCounters);
int32_t BWL_ResetCounter(BWL_Handle hBwl);
int32_t BWL_GetRevInfo(BWL_Handle hBwl, RevInfo_t *pRevInfo);
int32_t BWL_ScanAbort(BWL_Handle hBwl);

int32_t BWL_IsConnectedAp
(
    BWL_Handle  hBwl,
    uint32_t    *pulConnect
);

int32_t BWL_ConnectNoWep
(
    BWL_Handle    hBwl,        /* [in] BWL Handle */
    NetOpMode_t   eNetOpMode,  /* [in] infrastructure or adhoc */
    char          *pcSSID,     /* [in] SSID of the AP */
    struct ether_addr *peBSSID /* [in] BSSID of the AP */
);

int32_t BWL_ConnectWep
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    NetOpMode_t     eNetOpMode, /* [in] infrastructure or adhoc */
    char            *pcSSID,    /* [in] SSID of the AP */
    char            *pcKey,     /* [in] key string */
    uint32_t        ulKeyIndex, /* [in] 0-3 key index */
    AuthType_t      eAuthType,  /* [in] open, shared, open & shared */
    struct ether_addr *peBSSID  /* [in] BSSID of the AP */
);

int32_t BWL_ConnectWpaTkip
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    NetOpMode_t     eNetOpMode, /* [in] infrastructure or adhoc */
    char            *pcSSID,    /* [in] SSID of the AP */
    char            *pcKey,     /* [in] key string */
    struct ether_addr *peBSSID  /* [in] BSSID of the AP */
);

int32_t BWL_ConnectWpaAes
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    NetOpMode_t     eNetOpMode, /* [in] infrastructure or adhoc */
    char            *pcSSID,    /* [in] SSID of the AP */
    char            *pcKey,     /* [in] key string */
    struct ether_addr *peBSSID  /* [in] BSSID of the AP */
);

int32_t BWL_ConnectWpa2Tkip
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    NetOpMode_t     eNetOpMode, /* [in] infrastructure or adhoc */
    char            *pcSSID,    /* [in] SSID of the AP */
    char            *pcKey,     /* [in] key string */
    struct ether_addr *peBSSID  /* [in] BSSID of the AP */
);

int32_t BWL_ConnectWpa2Aes
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    NetOpMode_t     eNetOpMode, /* [in] infrastructure or adhoc */
    char            *pcSSID,    /* [in] SSID of the AP */
    char            *pcKey,     /* [in] key string */
    struct ether_addr *peBSSID  /* [in] BSSID of the AP */
);

int32_t BWL_ConnectAp
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    Credential_t    *pCred  /* [in] connection credential */
);

int32_t BWL_DisconnectAp(BWL_Handle hBwl);
int32_t BWL_SetCountry(BWL_Handle hBwl, char *pcCountry);
int32_t BWL_GetCountry(BWL_Handle hBwl, char *pcCountry, int32_t *pRev);
int32_t BWL_SetSsid(BWL_Handle hBwl, char *pcSsid, struct ether_addr *peBSSID);

int32_t BWL_GetSsid
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcSsid, /* [out] AP SSID */
    uint32_t    *pulLen  /* [out] SSID length */
);

int32_t BWL_GetCachedSsid
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcSsid, /* [out] AP SSID */
    uint32_t    *pulLen  /* [out] SSID length */
);

int32_t BWL_GetBssid(BWL_Handle hBwl, struct ether_addr *pbssid);
int32_t BWL_SetBand(BWL_Handle hBwl, Band_t eBand);
int32_t BWL_GetBand(BWL_Handle hBwl, Band_t *peBand);
int32_t BWL_SetChannel(BWL_Handle hBwl, uint32_t ulChan);
int32_t BWL_GetChannel(BWL_Handle hBwl, uint32_t *pulChan);

int32_t BWL_GetChannelsByCountry
(
    BWL_Handle  hBwl,
    char        *pcCountry,
    uint32_t    ulBand,
    uint32_t    aulChannels[],
    uint32_t    *pulChannels
);

int32_t BWL_SetInfraMode(BWL_Handle hBwl, NetOpMode_t eNetOpMode);
int32_t BWL_GetInfraMode(BWL_Handle hBwl, NetOpMode_t *peNetOpMode);
int32_t BWL_SetAuthType(BWL_Handle hBwl, AuthType_t eAuthType);
int32_t BWL_GetAuthType(BWL_Handle hBwl, AuthType_t *peAuthType);
int32_t BWL_SetWpaSup(BWL_Handle hBwl, WpaSup_t eWpaSup);
int32_t BWL_GetWpaSup(BWL_Handle hBwl, WpaSup_t *peWpaSup);
int32_t BWL_SetWpaAuth(BWL_Handle hBwl, WpaAuth_t eWpaAuth);
int32_t BWL_GetWpaAuth(BWL_Handle hBwl, WpaAuth_t *peWpaAuth);
int32_t BWL_SetWSec(BWL_Handle hBwl, WSec_t eWSec);
int32_t BWL_GetWSec(BWL_Handle hBwl, WSec_t *peWSec);
int32_t BWL_SetWSecKey(BWL_Handle hBwl, char *pcKey);
int32_t BWL_GetWSecKey(BWL_Handle hBwl, char* pcKey, uint32_t ulLength);
int32_t BWL_SetWepIndex(BWL_Handle hBwl, uint32_t ulIndex);
int32_t BWL_GetWepIndex(BWL_Handle hBwl, uint32_t *pulIndex);

int32_t BWL_AddWepKey
(
    BWL_Handle      hBwl,
    uint32_t          ulIndex,
    char            *pcKey,
    CryptoAlgo_t    eAlgoOverride, /* used for 16 bytes key */
    uint32_t          ulIsPrimary
);

int32_t BWL_WpsConnectByPb(BWL_Handle hBwl, char *pcNetIf, char *pKey, uint32_t ulKeyLength);
int32_t BWL_WpsConnectByPin
(
    BWL_Handle  hBwl,
    char        *pcNetIf,
    char        *pcSsid,
    uint32_t      ulPin,
    char        *pKey,
    uint32_t    ulKeyLength
);

int32_t BWL_GetCredential(BWL_Handle hBwl, Credential_t *pCredential);
int32_t BWL_GetWepKey
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    uint32_t      ulIndex,        /* [in] WEP index 0-3 */
    uint32_t      ulIsPrimary,    /* [in] WEP current used index */
    char        *pcKey,         /* [out] WEP key string */
    uint32_t      *pulLength      /* [out] WEP key length */
);

int32_t BWL_GetLinkStatus(BWL_Handle  hBwl, uint32_t *pulIsLinkUp);

int32_t BWL_GetRpcAgg
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulAgg  /* [out] Aggregation value */
);

int32_t BWL_SetRpcAgg
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulAgg  /* [in] Aggregation value */
);

int32_t BWL_GetHtRestrict
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] Restrict value */
);

int32_t BWL_SetHtRestrict
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] Restrict value */
);

int32_t BWL_GetSisoTx
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] SISO Value */
);

int32_t BWL_SetSisoTx
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] SISO Value */
);

int32_t BWL_GetMimoTx
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] MIMO Value */
);

int32_t BWL_SetMimoTx
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] MIMO Value */
);

int32_t BWL_SetChanspec
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    ChanSpec_t      *pChanSpec  /* [in] ChanSpec settings */
);

int32_t BWL_SetDataRate
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal,  /* [in] Data rate Kbps */
    bool bArate  /* false indicates B/G mode; true indicates A mode */
);

int32_t BWL_SetMcsIndex
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] MCS index */
);

int32_t BWL_SetTransmitPower
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    int8_t      ulVal  /* [in] Transmit power dB */
);

int32_t BWL_ConfigurePacketEngine
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    PacketEngineParameters_t    *pPktEngineParams  /* [in] Parameters for configuring packet engine */
);

int32_t BWL_GetStaRetryTime
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] Value */
);

int32_t BWL_SetStaRetryTime
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] Value */
);

int32_t BWL_GetMimoBwCap
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] Value */
);

int32_t BWL_SetMimoBwCap
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] Value */
);

int32_t BWL_GetBwCap
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    Band_t band_type, /* 2G or 5G*/
    uint32_t    *pulVal  /* [out] Value */
);

int32_t BWL_SetBwCap
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    Band_t band_type, /* 2G or 5G*/
    uint32_t    ulVal  /* [in] Value */
);

int32_t BWL_GetApBwCap
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    uint32_t      *pulBandWidth   /* [out] 0 is 20MHz or Not Connect, 1 is 40MHz */
);

int32_t BWL_GetDptCredential
(
    BWL_Handle          hBwl,  /* [in] BWL Handle */
    DptCredential_t    *pCred  /* [out] used to store credential */
);

int32_t BWL_GetDptMode
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t      *pulVal  /* [out] Value */
);

int32_t BWL_SetDptMode
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] Value */
);

int32_t BWL_GenerateDptKey
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSecPmk_t   *dptkey /* [out] DPT Key */
);

int32_t BWL_SetDptSecurity
(
    BWL_Handle  hBwl   /* [in] BWL Handle */
);

int32_t BWL_GetDptList
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    DptList_t   *data  /* [out] DPT list */
);

int32_t BWL_SetEvent(BWL_Handle hBwl, EventId_t eEvent);
int32_t BWL_ClearEvent(BWL_Handle hBwl, EventId_t eEvent);
int32_t BWL_ParseEvent(BWL_Handle hBwl, void *pBuff, uint32_t ulBufLength, EventMessage_t *pEvent);
int32_t BWL_SetOBSSCoEx(BWL_Handle hBwl, uint32_t ulCoEx);
int32_t BWL_Get802_11Modes(BWL_Handle hBwl, uint32_t *pModes);
int32_t BWL_GetWpaSupStatus(BWL_Handle hBwl, SupStatus_t *pStatus);

int32_t BWL_GetSupAuthStatus
(
    BWL_Handle  hBwl,
    uint32_t    *pulStatus
);

int32_t BWL_SetRSSIEventLevels(BWL_Handle hBwl, int32_t *plLevel, uint32_t ulNumLevels);
int32_t BWL_GetRSSI(BWL_Handle hBwl, int32_t *plRSSI);
int32_t BWL_GetRate(BWL_Handle hBwl, int32_t *plRate);

int32_t BWL_WoWLSetMask(BWL_Handle hBwl, uint32_t ulMask);
int32_t BWL_WoWLActivate(BWL_Handle hBwl);
int32_t BWL_WoWLForce(BWL_Handle hBwl);
int32_t BWL_WoWLPatternClear(BWL_Handle hBwl);
int32_t BWL_WoWLPatternAdd(BWL_Handle hBwl, uint32_t ulOffset, char *pMask, char *pValue);
int32_t BWL_EScanAbort(BWL_Handle hBwl);
int32_t BWL_EScanResults(BWL_Handle hBwl, const char *pIface, ScanParams_t *pScanParams, ScanInfo_t **pData, uint32_t *pulCount, bool *pbStop, int lTimeoutMs);
int32_t BWL_EScanResultsWlExe (BWL_Handle hBwl);
int32_t BWL_escanresults_print(void);
int32_t BWL_escanresults_print1(unsigned char *buffer, char * printBuffer, int printBufferLen, const char *redirectFilename );
const char * BWL_escanresults_get_ssid( unsigned char *buffer );
int32_t BWL_escanresults_print_linkedlist(void);
int32_t BWL_escanresults_free(void);
int32_t BWL_escanresults_count( void );
int32_t BWL_escanresults_copy( unsigned char *dest, unsigned int start_index, unsigned int max_copy );
int32_t BWL_count_lines( const char * buffer, int max_line_length );
const char *BWL_GetAmpdu( BWL_Handle hBwl );
int         BWL_ClearAmpdu( BWL_Handle hBwl );
const unsigned char *BWL_GetPhyRssiAnt( BWL_Handle hBwl );
int32_t BWL_SetBridgeMode(BWL_Handle hBwl, uint32_t ulValue);
int32_t BWL_GetAntennaCount(BWL_Handle hBwl, uint32_t *pulAntenna);
int32_t BWL_SetAntenna(BWL_Handle hBwl, AntMode_t eAntMode, uint32_t ulAntenna);
int32_t BWL_GetAntenna(BWL_Handle hBwl, AntMode_t eAntMode, uint32_t *pulAntenna);
int32_t BWL_GetRssiPerAntenna(BWL_Handle  hBwl, RssiPerAntenna_t *pRssiPerAntenna);
int32_t BWL_Disassoc(BWL_Handle hBwl);
int32_t BWL_Out(BWL_Handle hBwl);
int32_t BWL_ControlFrequencyAccuracy(BWL_Handle  hBwl, int32_t channel);
int32_t BWL_ControlPhyWatchdog(BWL_Handle hBwl, bool bEnable);
int32_t BWL_ControlPhyForceCal(BWL_Handle hBwl, bool bEnable);
int32_t BWL_SetInterferenceMitigationMode(BWL_Handle hBwl, int mode);
int32_t BWL_ControlScanSuppression(BWL_Handle hBwl, int mode);
int32_t BWL_SetBeaconInterval(BWL_Handle hBwl, int interval);
int32_t BWL_SetMimoPreamble(BWL_Handle hBwl, int value);
int32_t BWL_GetMimoPreamble(BWL_Handle hBwl, int *pValue);
int32_t BWL_SetFrameBurst(BWL_Handle hBwl, int mode);
int32_t BWL_SetMpcMode(BWL_Handle hBwl, int value);
int32_t BWL_GetMpcMode(BWL_Handle hBwl, uint32_t *pulMode);
int32_t BWL_GetTemperature(BWL_Handle hBwl, uint32_t *pTemp);
int32_t BWL_SetApMode(BWL_Handle hBwl, int value);
int32_t BWL_SetAntennaDiversity(BWL_Handle hBwl, int value);
int32_t BWL_SetTxAntenna(BWL_Handle hBwl, int value);
int32_t BWL_SetAmpDU(BWL_Handle hBwl, int value);
int32_t BWL_SetPhyTxPowerControl(BWL_Handle hBwl, int value);
int32_t BWL_SetPhyScramInit(BWL_Handle hBwl, int value);
int32_t BWL_GetNRate(BWL_Handle hBwl, uint32_t *pNRate);

bool BWL_IsP2PIface(BWL_Handle hBwl, struct ether_addr *mac);
int32_t BWL_DeleteP2PIface(BWL_Handle hBwl, struct ether_addr *mac);
int32_t BWL_GetGpioState(BWL_Handle hBwl, uint32_t *pulGpioValue);
int32_t BWL_HideSSID(BWL_Handle hBwl, bool bEnable);
int32_t BWL_SetRadar(BWL_Handle hBwl, bool bEnable);
int32_t BWL_SetDfsSpec(BWL_Handle hBwl,DfsSpec_t eDfsSpec);

int32_t BWL_GetBands(BWL_Handle hBwl, Band_t *peBand);
int32_t BWL_SetVhtFeatures(BWL_Handle hBwl, int value);
int32_t BWL_GetVhtFeatures(BWL_Handle hBwl, uint32_t *pulVhtFeatures);


extern int g_wlc_idx;
int32_t BWL_IfaceCreate(BWL_Handle hBwl, bool bAp, struct ether_addr *mac, char *pIfaceName, uint32_t ulLength, int index, uint8_t *bsscfgidx);
int32_t BWL_IfaceDelete(BWL_Handle hBwl);

int32_t BWL_SetChanspecStr(BWL_Handle hBwl, char* pChanspec, uint32_t ulLength);

typedef enum eBssMode {
    eBssModeNone,
    eBssModeUp,
    eBssModeDown,
    eBssModeAp,
    eBssModeSta
} BssMode_t;

int32_t BWL_SetBss(BWL_Handle hBwl, BssMode_t eMode, int8_t index);
int32_t BWL_SetRsdbMode(BWL_Handle hBwl, int32_t lMode);

int32_t BWL_GetApSta(BWL_Handle hBwl, uint32_t *pulApSta);
int32_t BWL_SetApSta(BWL_Handle hBwl, uint32_t ulApSta);

int32_t BWL_SetMchanAlgo(BWL_Handle hBwl, uint32_t ulApSta);
int32_t BWL_SetMchanBw(BWL_Handle hBwl, uint32_t ulBw);
int32_t BWL_SetMchanSchedMode(BWL_Handle hBwl, uint32_t ulMode);
char *  BWL_GetDriverVersion( BWL_Handle hBwl );
int     BWL_ClearCounters( BWL_Handle hBwl );
int     BWL_SendWakeOnWlan( BWL_Handle hBwl, char *pkt_len, char *destination_frame, char *macAddress, char *pkt_type );
int     BWL_GetMacAssocList( BWL_Handle hBwl, BWL_MAC_ADDRESS *outputList, int outputListLen );
int     BWL_GetCcaStats( BWL_Handle hBwl, BWLGetCcaStats_t *BWLGetCcaStats );

#endif /* BWL_H__ */
