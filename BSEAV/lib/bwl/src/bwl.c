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
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <typedefs.h>
#include <epivers.h>
#include <pthread.h>
#include <proto/ethernet.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>
#include <proto/802.1d.h>
#include <proto/802.11e.h>
#include <proto/wpa.h>
#include <proto/bcmip.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#ifndef FALCON_WIFI_DRIVER
#include <bcmwifi_channels.h>
#else
#include <bcmwifi.h>
#endif
#include <bcmsrom_fmt.h>
#include <bcmsrom_tbl.h>
#include <bcmcdc.h>

#ifdef INCLUDE_WPS
/* wps includes */
#include <portability.h>
#include <wpserror.h>
#include <reg_prototlv.h>
#include <wps_enrapi.h>
#include <wps_enr.h>
#include <wps_enr_osl.h>
#include <wps_sta.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#if 0
#include <linux/if_ether.h> /* ETH_P_ALL */
#endif


/* need this for using exec */
#include <unistd.h>
#include <sys/wait.h>
#include "wlu.h"
#include "bwl_priv.h"
#include "bwl.h"
#include "bwl_wl.h"

#if defined(linux)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#elif defined(UNDER_CE) || defined(_CRT_SECURE_NO_DEPRECATE)
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

#ifdef WL_DUMP_BUF_LEN
#undef WL_DUMP_BUF_LEN
#endif
#define WL_DUMP_BUF_LEN (16 * 1024)

#define ESCAN_BSS_FIXED_SIZE 4
#define ESCAN_EVENTS_BUFFER_SIZE 2048
#define ESCAN_LOOP_TIMEOUT_MS   10

#define BWL_LOCK() pthread_mutex_lock(&hBwl->bwlMutex)
#define BWL_UNLOCK() pthread_mutex_unlock(&hBwl->bwlMutex)

char *GetFileContents( const char *filename );
#ifdef INCLUDE_WPS
wps_ap_list_info_t *create_aplist(void);
void config_init(void);
int find_pbc_ap(char * bssid, char *ssid, uint8 *wsec);
int enroll_device(char *pin, char *ssid, uint8 wsec, char *bssid, char *key, uint32_t key_len);
int display_aplist(wps_ap_list_info_t *ap);
uint32_t wps_generatePin(char c_devPwd[8], IN bool b_display);
#endif

/* WLU externs */
extern int bwl_wl_check(void *bwl);
extern int wlu_set(void *wl, int cmd, void *cmdbuf, int len);
extern int wlu_get(void *wl, int cmd, void *cmdbuf, int len);
extern void dump_bss_info(wl_bss_info_t *bi);
extern void dump_networks(char *network_buf);
extern void wl_find(char *args[], uint32_t size);
extern int wlu_iovar_set(void *wl, const char *iovar, void *param, int paramlen);
extern int wlu_iovar_get(void *wl, const char *iovar, void *param, int paramlen);
extern int wlu_iovar_getbuf(void* wl, const char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern uint8 * wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key);
extern bool wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len);
extern bool bcm_is_wps_ie(uint8_t *ie, uint8_t **tlvs, uint32_t *tlvs_len);
extern int wlu_pattern_atoh(char *src, char *dst);
extern chanspec_t bwl_chspec32_from_driver(uint32 chanspec32);
extern void wl_rsn_ie_dump(bcm_tlv_t *ie, WpaInfo_t *info);
int32_t bwl_connect_ap(BWL_Handle hBwl,Credential_t *pCred);
extern int wlu_iovar_setint(void *wl, const char *iovar, int val);
extern int wlu_iovar_getint(void *wl, const char *iovar, int *pval);

typedef struct BWL_P_Handle
{
    void            *wl;
    pthread_mutex_t bwlMutex;
    char            s_bufdata[WLC_IOCTL_MAXLEN];
    struct ifreq    ifr;
} BWL_P_Handle;

typedef struct
{
    uint32_t    eBwl;
    uint32_t    eWl;
    const char  *pName;
} BwlToWl_t;

static BwlToWl_t CryptoAlgoTable[] =
{
    { eCryptoAlgoOff,        CRYPTO_ALGO_OFF            , "eCryptoAlgoOff"          },
    { eCryptoAlgoWep1,       CRYPTO_ALGO_WEP1           , "eCryptoAlgoWep1"         },
    { eCryptoAlgoWep128,     CRYPTO_ALGO_WEP128         , "eCryptoAlgoWep128"       },
    { eCryptoAlgoTkip,       CRYPTO_ALGO_TKIP           , "eCryptoAlgoTkip"         },
    { eCryptoAlgoAesCcm,     CRYPTO_ALGO_AES_CCM        , "eCryptoAlgoAesCcm"       },
    { eCryptoAlgoAesOcbMsdu, CRYPTO_ALGO_AES_OCB_MSDU   , "eCryptoAlgoAesOcbMsdu"   },
    { eCryptoAlgoAesOcbMpdu, CRYPTO_ALGO_AES_OCB_MPDU   , "eCryptoAlgoAesOcbMpdu"   },
    { eCryptoAlgoNalg,       CRYPTO_ALGO_NALG           , "eCryptoAlgoNalg"         },
};


static BwlToWl_t WSecTable[] =
{
    { eWSecNone,    0               , "eWSecNone"   },
    { eWSecWep,     WEP_ENABLED     , "eWSecWep"    },
    { eWSecTkip,    TKIP_ENABLED    , "eWSecTkip"   },
    { eWSecAes,     AES_ENABLED     , "eWSecAes"    },
    { eWSecAll,     (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED)     , "eWSecSw"     },
};


static BwlToWl_t AuthTypeTable[] =
{
    { eAuthTypeOpen,      WL_AUTH_OPEN_SYSTEM   , "eAuthTypeOpen"       },
    { eAuthTypeShare,     WL_AUTH_SHARED_KEY    , "eAuthTypeShare"      },
    { eAuthTypeOpenShare, WL_AUTH_OPEN_SHARED   , "eAuthTypeOpenShare"  },
};

static BwlToWl_t WpaAuthTable[] =
{
    { eWpaAuthDisabled, WPA_AUTH_DISABLED   , "eWpaAuthDisabled"},
    { eWpaAuthNone,     WPA_AUTH_NONE       , "eWpaAuthNone"    },
    { eWpaAuthWpaUnsp,  WPA_AUTH_UNSPECIFIED, "eWpaAuthWpaUnsp" },
    { eWpaAuthWpaPsk,   WPA_AUTH_PSK        , "eWpaAuthWpaPsk"  },
    { eWpaAuthWpa2Unsp, WPA2_AUTH_UNSPECIFIED,"eWpaAuthWpa2Unsp"},
    { eWpaAuthWpa2Psk,  WPA2_AUTH_PSK       , "eWpaAuthWpa2Psk" }
};


static BwlToWl_t BandTable[] =
{
    { eBandAuto,        WLC_BAND_AUTO   , "eBandAuto"   },
    { eBand5G,          WLC_BAND_5G     , "eBand5G"     },
    { eBand2G,          WLC_BAND_2G     , "eBand2G"     },
    { eBandAll,         WLC_BAND_ALL    , "eBandAll"    },
};


static BwlToWl_t NetOpModeTable[] =
{
    { eNetOpModeAdHoc,  0,  "eNetOpModeAdHoc"   },
    { eNetOpModeInfra,  1,  "eNetOpModeInfra"   },
};


static BwlToWl_t WpaSupTable[] =
{
    {eWpaSupExternal,   0,  "eWpaSupExternal" },
    {eWpaSupInternal,   1,  "eWpaSupInternal" },
};


static BwlToWl_t EventMessageTable [] =
{
    { BWL_E_SET_SSID,               WLC_E_SET_SSID,                 "WLC_E_SET_SSID"},
    { BWL_E_JOIN,                   WLC_E_JOIN,                     "WLC_E_JOIN"},
    { BWL_E_START,                  WLC_E_START,                    "WLC_E_START"},
    { BWL_E_AUTH,                   WLC_E_AUTH,                     "WLC_E_AUTH"},
    { BWL_E_AUTH_IND,               WLC_E_AUTH_IND,                 "WLC_E_AUTH_IND"},
    { BWL_E_DEAUTH,                 WLC_E_DEAUTH,                   "WLC_E_DEAUTH"},
    { BWL_E_DEAUTH_IND,             WLC_E_DEAUTH_IND,               "WLC_E_DEAUTH_IND"},
    { BWL_E_ASSOC,                  WLC_E_ASSOC,                    "WLC_E_ASSOC"},
    { BWL_E_ASSOC_IND,              WLC_E_ASSOC_IND,                "WLC_E_ASSOC_IND"},
    { BWL_E_REASSOC,                WLC_E_REASSOC,                  "WLC_E_REASSOC"},
    { BWL_E_REASSOC_IND,            WLC_E_REASSOC_IND,              "WLC_E_REASSOC_IND"},
    { BWL_E_DISASSOC,               WLC_E_DISASSOC,                 "WLC_E_DISASSOC"},
    { BWL_E_DISASSOC_IND,           WLC_E_DISASSOC_IND,             "WLC_E_DISASSOC_IND"},
    { BWL_E_QUIET_START,            WLC_E_QUIET_START,              "WLC_E_QUIET_START"},
    { BWL_E_QUIET_END,              WLC_E_QUIET_END,                "WLC_E_QUIET_END"},
    { BWL_E_BEACON_RX,              WLC_E_BEACON_RX,                "WLC_E_BEACON_RX"},
    { BWL_E_LINK,                   WLC_E_LINK,                     "WLC_E_LINK"},
    { BWL_E_MIC_ERROR,              WLC_E_MIC_ERROR,                "WLC_E_MIC_ERROR"},
    { BWL_E_NDIS_LINK,              WLC_E_NDIS_LINK,                "WLC_E_NDIS_LINK"},
    { BWL_E_ROAM,                   WLC_E_ROAM,                     "WLC_E_ROAM"},
    { BWL_E_TXFAIL,                 WLC_E_TXFAIL,                   "WLC_E_TXFAIL"},
    { BWL_E_PMKID_CACHE,            WLC_E_PMKID_CACHE,              "WLC_E_PMKID_CACHE"},
    { BWL_E_RETROGRADE_TSF,         WLC_E_RETROGRADE_TSF,           "WLC_E_RETROGRADE_TSF"},
    { BWL_E_PRUNE,                  WLC_E_PRUNE,                    "WLC_E_PRUNE"},
    { BWL_E_AUTOAUTH,               WLC_E_AUTOAUTH,                 "WLC_E_AUTOAUTH"},
    { BWL_E_EAPOL_MSG,              WLC_E_EAPOL_MSG,                "WLC_E_EAPOL_MSG"},
    { BWL_E_SCAN_COMPLETE,          WLC_E_SCAN_COMPLETE,            "WLC_E_SCAN_COMPLETE"},
    { BWL_E_ADDTS_IND,              WLC_E_ADDTS_IND,                "WLC_E_ADDTS_IND"},
    { BWL_E_DELTS_IND,              WLC_E_DELTS_IND,                "WLC_E_DELTS_IND"},
    { BWL_E_BCNSENT_IND,            WLC_E_BCNSENT_IND,              "WLC_E_BCNSENT_IND"},
    { BWL_E_BCNRX_MSG,              WLC_E_BCNRX_MSG,                "WLC_E_BCNRX_MSG"},
    { BWL_E_BCNLOST_MSG,            WLC_E_BCNLOST_MSG,              "WLC_E_BCNLOST_MSG"},
    { BWL_E_ROAM_PREP,              WLC_E_ROAM_PREP,                "WLC_E_ROAM_PREP"},
    { BWL_E_PFN_NET_FOUND,          WLC_E_PFN_NET_FOUND,            "WLC_E_PFN_NET_FOUND"},
    { BWL_E_PFN_NET_LOST,           WLC_E_PFN_NET_LOST,             "WLC_E_PFN_NET_LOST"},
    { BWL_E_RESET_COMPLETE,         WLC_E_RESET_COMPLETE,           "WLC_E_RESET_COMPLETE"},
    { BWL_E_JOIN_START,             WLC_E_JOIN_START,               "WLC_E_JOIN_START"},
    { BWL_E_ROAM_START,             WLC_E_ROAM_START,               "WLC_E_ROAM_START"},
    { BWL_E_ASSOC_START,            WLC_E_ASSOC_START,              "WLC_E_ASSOC_START"},
    { BWL_E_IBSS_ASSOC,             WLC_E_IBSS_ASSOC,               "WLC_E_IBSS_ASSOC"},
    { BWL_E_RADIO,                  WLC_E_RADIO,                    "WLC_E_RADIO"},
    { BWL_E_PSM_WATCHDOG,           WLC_E_PSM_WATCHDOG,             "WLC_E_PSM_WATCHDOG"},
//  { BWL_E_CCX_ASSOC_START,        WLC_E_CCX_ASSOC_START,          "WLC_E_CCX_ASSOC_START"},
//  { BWL_E_CCX_ASSOC_ABORT,        WLC_E_CCX_ASSOC_ABORT,          "WLC_E_CCX_ASSOC_ABORT"},
    { BWL_E_PROBREQ_MSG,            WLC_E_PROBREQ_MSG,              "WLC_E_PROBREQ_MSG"},
    { BWL_E_SCAN_CONFIRM_IND,       WLC_E_SCAN_CONFIRM_IND,         "WLC_E_SCAN_CONFIRM_IND"},
    { BWL_E_PSK_SUP,                WLC_E_PSK_SUP,                  "WLC_E_PSK_SUP"},
    { BWL_E_COUNTRY_CODE_CHANGED,   WLC_E_COUNTRY_CODE_CHANGED,     "WLC_E_COUNTRY_CODE_CHANGED"},
    { BWL_E_EXCEEDED_MEDIUM_TIME,   WLC_E_EXCEEDED_MEDIUM_TIME,     "WLC_E_EXCEEDED_MEDIUM_TIME"},
    { BWL_E_ICV_ERROR,              WLC_E_ICV_ERROR,                "WLC_E_ICV_ERROR"},
    { BWL_E_UNICAST_DECODE_ERROR,   WLC_E_UNICAST_DECODE_ERROR,     "WLC_E_UNICAST_DECODE_ERROR"},
    { BWL_E_MULTICAST_DECODE_ERROR, WLC_E_MULTICAST_DECODE_ERROR,   "WLC_E_MULTICAST_DECODE_ERROR"},
    { BWL_E_TRACE,                  WLC_E_TRACE,                    "WLC_E_TRACE"},
//  { BWL_E_HCI_EVENT,              WLC_E_BTA_HCI_EVENT,            "WLC_E_BTA_HCI_EVENT"},
    { BWL_E_IF,                     WLC_E_IF,                       "WLC_E_IF"},
    { BWL_E_RSSI,                   WLC_E_RSSI,                     "WLC_E_RSSI"},
    { BWL_E_PFN_SCAN_COMPLETE,      WLC_E_PFN_SCAN_COMPLETE,        "WLC_E_PFN_SCAN_COMPLETE"},
    { BWL_E_EXTLOG_MSG,             WLC_E_EXTLOG_MSG,               "WLC_E_EXTLOG_MSG"},
//  { BWL_E_ACTION_FRAME,           WLC_E_ACTION_FRAME,             "WLC_E_ACTION_FRAME"},
    { BWL_E_PRE_ASSOC_IND,          WLC_E_PRE_ASSOC_IND,            "WLC_E_PRE_ASSOC_IND"},
    { BWL_E_PRE_REASSOC_IND,        WLC_E_PRE_REASSOC_IND,          "WLC_E_PRE_REASSOC_IND"},
    { BWL_E_CHANNEL_ADOPTED,        WLC_E_CHANNEL_ADOPTED,          "WLC_E_CHANNEL_ADOPTED"},
    { BWL_E_AP_STARTED,             WLC_E_AP_STARTED,               "WLC_E_AP_STARTED"},
    { BWL_E_DFS_AP_STOP,            WLC_E_DFS_AP_STOP,              "WLC_E_DFS_AP_STOP"},
    { BWL_E_DFS_AP_RESUME,          WLC_E_DFS_AP_RESUME,            "WLC_E_DFS_AP_RESUME"},
    { BWL_E_ESCAN_RESULT,           WLC_E_ESCAN_RESULT,             "WLC_E_ESCAN_RESULT"},
    { BWL_E_LAST,                   WLC_E_LAST,                     "WLC_E_LAST"}};

#define MCS_INDEX_COUNT 32
static int32_t PhyDataRate_40MHz[MCS_INDEX_COUNT][2] =
{
    { 27,30 },    { 54,60 },    { 81,90 },    { 108,120 },    { 162,180 },    { 216,240 },    { 243,270 },    { 270,300 },
    { 54,60 },    { 108,120 },  { 162,180 },  { 216,240 },    { 324,360 },    { 432,480 },    { 486,540 },    { 540,600 }, /*15 */
    { 81,90 },    { 162,180 },  { 243,270 },  { 324,360 },    { 486,540 },    { 648,720 },    { 728,810 },    { 810,900 },
    { 108,120 },  { 216,240 },  { 324,360 },  { 432,480 },    { 648,720 },    { 864,960 },    { 972,1080 },   { 1080,1200 },
};

static int32_t PhyDataRate_20MHz[MCS_INDEX_COUNT][2] =
{
    { 13,14},    { 26,29},    { 39,43},    { 52,58},    { 78,87},    { 104,116},    { 117,130},    { 130,144},
    { 26,29},    { 52,58},    { 78,87},    { 104,116},  { 156,173},  { 208,231},    { 234,260},    { 260,289}, /* 15 */
    { 39,43},    { 78,87},    { 117,130},  { 156,173},  { 234,260},  { 312,347},    { 351,390},    { 390,433},
    { 52,58},    { 104,116},  { 156,173},  { 208,231},  { 312,347},  { 416,462},    { 468,520},    { 520,578},
};

#ifndef _wlu_h_
struct escan_bss {
    struct escan_bss *next;
    wl_bss_info_t bss[1];
};
#endif /* _wlu_h_ */

/* dword align allocation */
static union
{
    char bufdata[WLC_IOCTL_MAXLEN];
    uint32 alignme;
} bufstruct_wlu;

wifi_counters_t wifi_counters;
static char *buf = (char*) &bufstruct_wlu.bufdata;

#define FETCH_COUNTER_VALUE(entry, name)                            \
{                                                                   \
    const char* cnt_name = name;                                    \
    if ( (pbuf = strstr (buf, cnt_name))!= NULL )                   \
    {                                                               \
        printf("%s(): Found %s\n", __FUNCTION__, cnt_name);         \
        pbuf += strlen(cnt_name)+1;                                 \
        char *pTmp = strdup(pbuf);                                  \
        pbuf = strtok (pTmp, " ");                                  \
        entry = atoi (pbuf) ;                                       \
        free(pTmp);                                                 \
        printf("\t %s: %Lu\n", cnt_name, entry);            \
    }                                                               \
}


static int
bwl_get_scan(void *wl, int opc, char *scan_buf, uint buf_len)
{
    wl_scan_results_t *list = (wl_scan_results_t*)scan_buf;
    int ret;

    list->buflen = htod32(buf_len);
    ret = wlu_get(wl, opc, scan_buf, buf_len);
    if (ret < 0)
        return ret;
    ret = 0;

    list->buflen = dtoh32(list->buflen);
    list->version = dtoh32(list->version);
    list->count = dtoh32(list->count);
    if (list->buflen == 0) {
        list->version = 0;
        list->count = 0;
    } else if (list->version != WL_BSS_INFO_VERSION &&
               list->version != LEGACY2_WL_BSS_INFO_VERSION &&
               list->version != LEGACY_WL_BSS_INFO_VERSION) {
        fprintf(stderr, "Sorry, your driver has bss_info_version %d "
            "but this program supports only version %d.\n",
            list->version, WL_BSS_INFO_VERSION);
        list->buflen = 0;
        list->count = 0;
    }

    return ret;
}

int
bwl_parse_country_spec(const char *spec, char *ccode, int *regrev)
{
    char *revstr;
    char *endptr = NULL;
    int ccode_len;
    int rev = -1;

    revstr = strchr(spec, '/');

    if (revstr) {
        rev = strtol(revstr + 1, &endptr, 10);
        if (*endptr != '\0') {
            /* not all the value string was parsed by strtol */
            fprintf(stderr,
                "Could not parse \"%s\" as a regulatory revision "
                "in the country string \"%s\"\n",
                revstr + 1, spec);
            return BWL_ERR_USAGE;
        }
    }

    if (revstr)
        ccode_len = (int)(uintptr)(revstr - spec);
    else
        ccode_len = (int)strlen(spec);

    if (ccode_len > 3) {
        fprintf(stderr,
            "Could not parse a 2-3 char country code "
            "in the country string \"%s\"\n",
            spec);
        return BWL_ERR_USAGE;
    }

    memcpy(ccode, spec, ccode_len);
    ccode[ccode_len] = '\0';
    *regrev = rev;

    return 0;
}

int32_t bwl_get_driver_error
(
    BWL_Handle  hBwl, /* [in] BWL Handle */
    int32_t     *plDriverErrCode /* [out] the driver's error code */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    err = wlu_iovar_get( wl, "bcmerror", plDriverErrCode, sizeof( int32_t ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

static int bwl_parse_bss_info(wl_bss_info_t *bi, ScanInfo_t *pScanInfo)
{
    unsigned int j;
    int mcs_idx = 0;
    if ( (bi == NULL) || (pScanInfo == NULL) )
    {
        return BWL_ERR_PARAM;
    }

    bi->chanspec                        = bwl_chspec32_from_driver(bi->chanspec);
    memset(&pScanInfo->tCredentials.acSSID, 0, sizeof(pScanInfo->tCredentials.acSSID));
    strncpy(pScanInfo->tCredentials.acSSID, (char*)bi->SSID, bi->SSID_len);
    pScanInfo->lRSSI                    = (int16)(dtoh16(bi->RSSI));
    pScanInfo->ulChan                   = CHSPEC_CHANNEL( bi->chanspec);
    pScanInfo->ulPrimChan               = wf_chspec_ctlchan( bi->chanspec);
    pScanInfo->lPhyNoise                = bi->phy_noise;
    pScanInfo->lSNR                     = bi->SNR;
    bi->capability                      = dtoh16(bi->capability);
    pScanInfo->tCredentials.eNetOpMode  = !(bi->capability & DOT11_CAP_IBSS);

    pScanInfo->BSSID       = bi->BSSID;
    pScanInfo->ul802_11Modes  = e802_11_none;
    pScanInfo->ul802_11Modes |= CHSPEC_IS2G(bi->chanspec)? e802_11_b : e802_11_none;
    pScanInfo->ul802_11Modes |= (bi->n_cap) ? e802_11_n : e802_11_none;
#ifndef FALCON_WIFI_DRIVER
    pScanInfo->ul802_11Modes |= (bi->vht_cap) ? e802_11_ac : e802_11_none;
#endif
    for (j = 0; j < MCS_INDEX_COUNT; j++)
    {
        if (isset(bi->basic_mcs, j))
        {
            mcs_idx = j;
        }
    }

    if (CHSPEC_IS5G(bi->chanspec))
    {
        pScanInfo->ul802_11Modes |= e802_11_a;
    }
    else
    {
        for (j = 0; j < dtoh32(bi->rateset.count); j++)
        {
            int r = bi->rateset.rates[j] & 0x7f;
            uint b = bi->rateset.rates[j] & 0x80;
            if (r == 0)
                break;

            if (r > pScanInfo->lRate)
            {
                pScanInfo->lRate = r;
            }
            if ( (r/2 == 24) && b)
            {
                pScanInfo->ul802_11Modes &= ~e802_11_b;
            }
            else if (r/2 == 54)
            {
                pScanInfo->ul802_11Modes |= e802_11_g;
                break;
            }
        }
    }
    wf_chspec_ntoa(bi->chanspec, pScanInfo->cChanSpec);

#ifndef FALCON_WIFI_DRIVER
    /* Bandwidth */
    if (CHSPEC_IS160(bi->chanspec))
    {
        pScanInfo->tBandwidth = eBandwidth160MHz;
    }
    else if (CHSPEC_IS80(bi->chanspec))
    {
        pScanInfo->tBandwidth = eBandwidth80MHz;
    }
    else
#endif
    if (CHSPEC_IS40(bi->chanspec))
    {
        pScanInfo->tBandwidth = eBandwidth40MHz;
    }
    else if (CHSPEC_IS20(bi->chanspec))
    {
        pScanInfo->tBandwidth = eBandwidth20MHz;
    }
    else if (CHSPEC_IS10(bi->chanspec))
    {
        pScanInfo->tBandwidth = eBandwidth10MHz;
    }


    if (bi->n_cap)
    {
        if (CHSPEC_IS40(bi->chanspec))
        {
            pScanInfo->lRate = PhyDataRate_40MHz[mcs_idx][((dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_40) == HT_CAP_SHORT_GI_40)];
        }
        else if (CHSPEC_IS20(bi->chanspec))
        {
            pScanInfo->lRate = PhyDataRate_20MHz[mcs_idx][((dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_20) == HT_CAP_SHORT_GI_20)];
        }
    }


    /* Parse credentials */
    if (dtoh32(bi->ie_length))
    {
        uint8_t     *cp = (uint8 *)(((uint8 *)bi) + dtoh16(bi->ie_offset));
        uint8_t     *parse = cp;
        uint32_t    parse_len = dtoh32(bi->ie_length);
        uint8_t     *wpaie;
        uint8_t     *rsnie;
        WpaInfo_t  wpa_info, rsn_info;

        memset(&wpa_info, 0, sizeof(WpaInfo_t));
        memset(&rsn_info, 0, sizeof(WpaInfo_t));

        while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
        {
            if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
                break;
        }


        /* Read the WPA information */
        if (wpaie)
        {
            wl_rsn_ie_dump((bcm_tlv_t*)wpaie, &wpa_info);
        }

        rsnie = wlu_parse_tlvs(cp, dtoh32(bi->ie_length), DOT11_MNG_RSN_ID);
        if (rsnie)
        {
            wl_rsn_ie_dump((bcm_tlv_t*)rsnie, &rsn_info);
        }


        /* Now figure out the supported cipher & authentication modes */
        pScanInfo->tCredentials.eWpaAuth = eWpaAuthDisabled; /* clear to start with */

        if (rsn_info.Akm == RSN_AKM_PSK)
        {
            pScanInfo->tCredentials.eWpaAuth |= eWpaAuthWpa2Psk;
        }
        if (rsn_info.Akm == RSN_AKM_UNSPECIFIED)
        {
            pScanInfo->tCredentials.eWpaAuth |= eWpaAuthWpa2Unsp;
        }
        if (wpa_info.Akm == RSN_AKM_PSK)
        {
            pScanInfo->tCredentials.eWpaAuth |= eWpaAuthWpaPsk;
        }
        if (wpa_info.Akm == RSN_AKM_UNSPECIFIED)
        {
            pScanInfo->tCredentials.eWpaAuth |= eWpaAuthWpaUnsp;
        }
        if( (rsn_info.Akm == RSN_AKM_NONE) ||
            (wpa_info.Akm == RSN_AKM_NONE) )
        {
            pScanInfo->tCredentials.eWpaAuth |= eWpaAuthNone;
        }


        /* Supported Encryption Method */
        pScanInfo->tCredentials.eWSec |= rsn_info.Cipher;
        pScanInfo->tCredentials.eWSec |= wpa_info.Cipher;

        /* Search for WPS */
        parse     = cp;
        parse_len = dtoh32(bi->ie_length);
        while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
        {
            pScanInfo->bWPS = bcm_is_wps_ie(wpaie, &parse, &parse_len);
            if (pScanInfo->bWPS)
            {
                break;
            }
        }
    }

    /* Detect if we support WEP */
    if (bi->capability & DOT11_CAP_PRIVACY)
    {
        pScanInfo->tCredentials.eWSec |= eWSecWep;
    }

    pScanInfo->bLocked = (bi->capability & DOT11_CAP_PRIVACY);

    return 0;
}

int32_t bwl_set_ssid
(
    BWL_Handle          hBwl,    /* [in] BWL Handle */
    char                *pcSsid, /* [in] AP SSID */
    struct ether_addr   *peBSSID /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    wl_join_params_t    join;

    if( pcSsid == NULL )
    {
        fprintf( stderr, "SSID arg NULL ponter\n" );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    if( strlen( pcSsid ) > DOT11_MAX_SSID_LEN )
    {
        fprintf( stderr, "SSID arg \"%s\" must be 32 chars or less\n", pcSsid );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }
    memset( &join, 0, sizeof(wl_join_params_t) );
    join.ssid.SSID_len = strlen( pcSsid );
    strcpy((char*)join.ssid.SSID, pcSsid);
    join.ssid.SSID_len = htod32( join.ssid.SSID_len );

    if( NULL == peBSSID )
    {
        PRINTF(("peBSSID NULL\n"));
        err = wlu_set( wl, WLC_SET_SSID, &join.ssid, sizeof( wlc_ssid_t ) );
    }
    else
    {
        PRINTF(("peBSSID Non NULL\n"));
        memcpy( &join.params.bssid, peBSSID, sizeof(struct ether_addr) );
        err = wlu_set( wl, WLC_SET_SSID, &join, sizeof( wl_join_params_t ) );
    }

BWL_EXIT:
    return( err );
}

int32_t bwl_get_cached_ssid
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcSsid, /* [out] AP SSID */
    uint32_t    *pulLen  /* [out] SSID length */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    wlc_ssid_t  ssid;

    if( pcSsid == NULL )
    {
        fprintf( stderr, "SSID arg NULL ponter\n" );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    err = wlu_iovar_get( wl, "ssid", &ssid, sizeof( ssid ) );
    BWL_CHECK_ERR( err );

    *pulLen = dtoh32( ssid.SSID_len );
    memcpy( pcSsid, ssid.SSID, *pulLen );

BWL_EXIT:
    return( err );
}

int32_t bwl_set_infra_mode
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    NetOpMode_t eNetOpMode  /* [in] ad-hoc or infrasture */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulMode = 0;
    uint32_t      i;

    for (i = 0; i < sizeof(NetOpModeTable)/sizeof(NetOpModeTable[0]); i++)
    {
        if (NetOpModeTable[i].eBwl == eNetOpMode)
        {
            ulMode = htod32(NetOpModeTable[i].eWl);
            break;
        }
    }

    err = wlu_set( wl, WLC_SET_INFRA, &ulMode, sizeof( ulMode ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_infra_mode
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    NetOpMode_t *peNetOpMode    /* [out] ad-hoc or infrasture */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulMode;
    uint32_t      i;

    err = wlu_get( wl, WLC_GET_INFRA, &ulMode, sizeof( ulMode ) );
    BWL_CHECK_ERR( err );


    ulMode = htod32(ulMode);
    for (i = 0; i < sizeof(NetOpModeTable)/sizeof(NetOpModeTable[0]); i++)
    {
        if (NetOpModeTable[i].eWl == ulMode)
        {
            *peNetOpMode = NetOpModeTable[i].eBwl;
            break;
        }
    }

BWL_EXIT:
    return( err );
}

int32_t bwl_set_auth_type
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    AuthType_t  eAuthType   /* [in] authentication type */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulAuthType;
    uint32_t      i;

    for (i = 0; i < sizeof(AuthTypeTable)/sizeof(AuthTypeTable[0]); i++)
    {
        if (AuthTypeTable[i].eBwl == eAuthType)
        {
            ulAuthType = htod32(AuthTypeTable[i].eWl);
            break;
        }
    }

    err = wlu_iovar_set( wl, "auth", &ulAuthType, sizeof( ulAuthType ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_auth_type
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    AuthType_t  *peAuthType     /* [out] authentication type */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulAuthType;
    uint32_t      i;

    err = wlu_iovar_get( wl, "auth", &ulAuthType, sizeof( ulAuthType ) );
    BWL_CHECK_ERR( err );

    ulAuthType = htod32( ulAuthType );
    for (i = 0; i < sizeof(AuthTypeTable)/sizeof(AuthTypeTable[0]); i++)
    {
        if (AuthTypeTable[i].eWl == ulAuthType)
        {
            *peAuthType = AuthTypeTable[i].eBwl;
            break;
        }
    }
BWL_EXIT:
    return( err );
}

int32_t bwl_set_wpa_sup
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaSup_t    eWpaSup     /* [in] driver supplicant */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulWpaSup;
    uint32_t      i;

    for (i = 0; i < sizeof(WpaSupTable)/sizeof(WpaSupTable[0]); i++)
    {
        if (WpaSupTable[i].eBwl == eWpaSup)
        {
            ulWpaSup = htod32(WpaSupTable[i].eWl);
            break;
        }
    }

    err = wlu_iovar_set( wl, "sup_wpa", &ulWpaSup, sizeof( ulWpaSup ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_wpa_sup
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaSup_t    *peWpaSup   /* [out] driver supplicant */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulWpaSup;
    uint32_t      i;

    err = wlu_iovar_get( wl, "sup_wpa", &ulWpaSup, sizeof( ulWpaSup) );
    BWL_CHECK_ERR( err );

    ulWpaSup = htod32(ulWpaSup);
    for (i = 0; i < sizeof(WpaSupTable)/sizeof(WpaSupTable[0]); i++)
    {
        if (WpaSupTable[i].eWl == ulWpaSup)
        {
            *peWpaSup = WpaSupTable[i].eBwl;
            break;
        }
    }

BWL_EXIT:
    return( err );
}

int32_t bwl_set_wpa_auth
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    WpaAuth_t   eWpaAuth /* [in] wpa authentication: none, wpa psk, wpa2 psk */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulWpaAuth;
    uint32_t      i;

    for (i = 0; i < sizeof(WpaAuthTable)/sizeof(WpaAuthTable[0]); i++)
    {
        if (WpaAuthTable[i].eBwl == eWpaAuth)
        {
            ulWpaAuth = htod32(WpaAuthTable[i].eWl);
            break;
        }
    }

    err = wlu_iovar_set( wl, "wpa_auth", &ulWpaAuth, sizeof( ulWpaAuth ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_wpa_auth
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaAuth_t   *peWpaAuth  /* [out] wpa authentication  */
)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulWpaAuth;
    uint32_t      i;

    err = wlu_iovar_get( wl, "wpa_auth", &ulWpaAuth, sizeof( ulWpaAuth ) );
    BWL_CHECK_ERR( err );

    ulWpaAuth = htod32( ulWpaAuth );
    for (i = 0; i < sizeof(WpaAuthTable)/sizeof(WpaAuthTable[0]); i++)
    {
        if (WpaAuthTable[i].eWl == ulWpaAuth)
        {
            *peWpaAuth = WpaAuthTable[i].eBwl;
            break;
        }
    }

BWL_EXIT:
    return( err );
}

int32_t bwl_set_wsec
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSec_t      eWSec   /* [in] wireless security: none, wep, tkip, aes */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulWSec = 0;
    uint32_t    i;

    for (i = 0; i < sizeof(WSecTable)/sizeof(WSecTable[0]); i++)
    {
        if (WSecTable[i].eBwl == eWSec)
        {
            ulWSec = htod32(WSecTable[i].eWl);
            break;
        }
    }

    err = wlu_set( wl, WLC_SET_WSEC, &ulWSec, sizeof( ulWSec ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_wsec
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSec_t      *peWSec /* [out] wireless security: none, wep, tkip, aes */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulWSec;
    uint32_t    i;

    err = wlu_get( wl, WLC_GET_WSEC, &ulWSec, sizeof( ulWSec ) );
    BWL_CHECK_ERR( err );

    ulWSec = htod32( ulWSec );
    for (i = 0; i < sizeof(WSecTable)/sizeof(WSecTable[0]); i++)
    {
        if (WSecTable[i].eWl == ulWSec)
        {
            *peWSec = WSecTable[i].eBwl;
            break;
        }
    }

BWL_EXIT:
    return( err );
}

int32_t bwl_set_wsec_key
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    char        *pcKey  /* [out] security passphrass/key */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    wsec_pmk_t  psk;
    size_t      key_len;

    key_len = strlen( pcKey );
    if( (key_len < WSEC_MIN_PSK_LEN) || (key_len > WSEC_MAX_PSK_LEN) )
    {
        fprintf( stderr, "passphrase must be between %d and %d characters long\n",
                 WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    psk.key_len = htod16( (ushort) key_len );
    psk.flags  = htod16( WSEC_PASSPHRASE );

    memcpy( psk.key, pcKey, key_len );

    err = wlu_set( wl, WLC_SET_WSEC_PMK, &psk, sizeof( psk ) );
    BWL_CHECK_ERR( err );


BWL_EXIT:
    return( err );
}

int32_t bwl_get_wsec_key
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcKey,  /* [out] security passphrass/key */
    uint32_t      ulLength /* [in] WSEC_MIN_PSK_LEN =< length =< WSEC_MAX_PSK_LEN */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    wsec_pmk_t  psk;
    size_t      key_len;

    if( (ulLength < WSEC_MIN_PSK_LEN) || (ulLength > WSEC_MAX_PSK_LEN) )
    {
        fprintf( stderr, "passphrase must be between %d and %d characters long\n",
                 WSEC_MIN_PSK_LEN, WSEC_MAX_PSK_LEN );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    err = wlu_iovar_get( wl, "wsec_get_pmk", &psk, sizeof(psk) );
    BWL_CHECK_ERR( err );

    key_len = (uint16) htod16( psk.key_len );
    memcpy( pcKey, psk.key, key_len );

BWL_EXIT:
    return( err );
}

int32_t bwl_set_wep_index
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    uint32_t    ulIndex     /* [in] WEP index */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    ulIndex = htod32( ulIndex );
    err = wlu_set( wl, WLC_SET_KEY_PRIMARY, &ulIndex, sizeof( ulIndex ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_wep_index
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    uint32_t    *pulIndex   /* [out] WEP index */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulIndex = 0;

    err = wlu_get( wl, WLC_GET_KEY_PRIMARY, &ulIndex, sizeof( uint32_t ) );
    BWL_CHECK_ERR( err );

    *pulIndex = htod32( ulIndex );

BWL_EXIT:
    return( err );
}

int32_t bwl_add_wep_key
(
    BWL_Handle      hBwl,           /* [in] BWL Handle */
    uint32_t          ulIndex,        /* [in] key index [0:3] */
    char            *pcKey,         /* [in] key string */
    CryptoAlgo_t    eAlgoOverride,  /* [in] used for 16 bytes key */
    uint32_t          ulIsPrimary     /* [in] type of key */
)
{
    int32_t         err = 0;
    void            *wl = hBwl->wl;
    wl_wsec_key_t   key;
    uint32_t        ulLen;
    uint32_t        ulAlgo = CRYPTO_ALGO_OFF;
    unsigned char   *data = &key.data[0];
    char            hex[] = "XX";

    memset( &key, 0, sizeof( key ) );
    ulLen = strlen( pcKey );

    switch( ulLen )
    {
    case 5:
    case 13:
    case 16:
        memcpy(data, pcKey, ulLen + 1);
        break;
    case 12:
    case 28:
    case 34:
    case 66:
        /* strip leading 0x */
        if (!strnicmp(pcKey, "0x", 2))
            pcKey += 2;
        else
            return -1;
        /* fall through */
    case 10:
    case 26:
    case 32:
    case 64:
        ulLen = ulLen / 2;
        while (*pcKey) {
            strncpy(hex, pcKey, 2);
            *data++ = (char) strtoul(hex, NULL, 16);
            pcKey += 2;
        }
        break;
    default:
        return BWL_ERR_PARAM;
    }

    switch (ulLen)
    {
    case 5:
        ulAlgo = CRYPTO_ALGO_WEP1;
        break;

    case 13:
        ulAlgo = CRYPTO_ALGO_WEP128;
        break;

    case 16:
    {
        unsigned int i;
        for (i = 0; i < sizeof(CryptoAlgoTable)/sizeof(CryptoAlgoTable[0]); i++)
        {
            if (CryptoAlgoTable[i].eBwl == eAlgoOverride)
            {
                ulAlgo = CryptoAlgoTable[i].eWl;
                break;
            }
        }
        /* sanity check to make sure the override algo is valid */
        if( (ulAlgo != CRYPTO_ALGO_AES_CCM)  &&
            (ulAlgo != CRYPTO_ALGO_AES_OCB_MPDU) )
        {
            BWL_CHECK_ERR( err = BWL_ERR_PARAM );
        }
        break;
    }
    case 32:
        ulAlgo = CRYPTO_ALGO_TKIP;
        break;

    default:
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
        break;
    }


    if( ulIsPrimary )
        key.flags = WL_PRIMARY_KEY;

    key.index  = htod32( ulIndex );
    key.len    = htod32( ulLen );
    key.algo   = htod32( ulAlgo );
    key.flags  = htod32( key.flags );

    err = wlu_set( wl, WLC_SET_KEY, &key, sizeof( key ) );
    BWL_CHECK_ERR( err );


BWL_EXIT:
    return( err );
}

int32_t bwl_connect_ap
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    Credential_t    *pCred  /* [in] connection credential */
)
{
    int32_t   err = 0;

    err = bwl_set_infra_mode( hBwl, pCred->eNetOpMode );
    BWL_CHECK_ERR( err );

    err = bwl_set_auth_type( hBwl, pCred->eAuthType );
    BWL_CHECK_ERR( err );

    err = bwl_set_wpa_auth( hBwl, pCred->eWpaAuth );
    BWL_CHECK_ERR( err );

    err = bwl_set_wsec( hBwl, pCred->eWSec );
    BWL_CHECK_ERR( err );

    err = bwl_set_wpa_sup( hBwl, pCred->eWpaSup );
    if (err)
    {
        BWL_DisplayError(err, __FUNCTION__, __FILE__, __LINE__);
    }


    switch (pCred->eWSec)
    {
    case eWSecWep:
        /* set the key and key index for wep */
        err = bwl_add_wep_key( hBwl,
                             pCred->ulWepIndex,
                             pCred->acKey,
                             eCryptoAlgoOff, /* dont' use overide */
                             1 );    /* primary key */
        BWL_CHECK_ERR( err );

        err = bwl_set_wep_index( hBwl, pCred->ulWepIndex );
        BWL_CHECK_ERR( err );
        break;

    case eWSecTkip:
    case eWSecAes:
        /* set the key for tkip or aes */
        err = bwl_set_wsec_key( hBwl, pCred->acKey );
        BWL_CHECK_ERR( err );
        break;

    default:
        break;
    }

    err = bwl_set_ssid( hBwl, pCred->acSSID, pCred->peBSSID );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    return( err );
}

int32_t bwl_get_link_status
(
    BWL_Handle  hBwl,        /* [in] BWL Handle */
    uint32_t    *pulIsLinkUp /* [out] 1 is up, 0 is down */
)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    struct ether_addr   eth_addr;

    err = wlu_get( wl, WLC_GET_BSSID, &eth_addr, sizeof(struct ether_addr) );

    if( BWL_ERR_SUCCESS != err ||
        0 == memcmp(&eth_addr,&ether_null,sizeof(struct ether_addr)) )
    {
        *pulIsLinkUp = 0;
    }
    else
    {
        *pulIsLinkUp = 1;
    }

    return BWL_ERR_SUCCESS;
}

#ifdef INCLUDE_WPS
int32_t bwl_generate_dpt_key
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSecPmk_t   *dptkey /* [out] DPT Key */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint8       key[9] = "BRCM"; /* 8 chars */

    /* Use wps to generate PIN for now */
    err = wps_generatePin( (char*)key, 0 );
    if( WPS_SUCCESS != err )
    {
        err = BWL_ERR_PARAM;
    }
    else
    {
        dptkey->KeyLen = 8;
        dptkey->Flags   = WSEC_PASSPHRASE;
        strncpy( (char*)dptkey->Key, (char*)key, dptkey->KeyLen );
        err = BWL_ERR_SUCCESS;
    }

    return( err );
}
#endif

/*******************************************************************************
*
*   Name: BWL_DisplayError()
*
*   Purpose:
*       Prints out the function that cause the error.
*
*   Returns:
*       None
*
*   See Also:
*
*******************************************************************************/
void  BWL_DisplayError
(
    int32_t         lErr,     /* [in] error type */
    const char      *pcFunc,  /* [in] the functin that cause the error */
    char            *pcFile,  /* [in] the file that the function resides */
    int32_t         lLine     /* [in] the line number where the error occur */
)
{
    char *pcErr;

    switch( lErr )
    {
        case BWL_ERR_USAGE: pcErr = "BWL_ERR_USAGE"; break;
        case BWL_ERR_IOCTL: pcErr = "BWL_ERR_IOCTL"; break;
        case BWL_ERR_PARAM: pcErr = "BWL_ERR_PARAM"; break;
        case BWL_ERR_CMD:   pcErr = "BWL_ERR_CMD";   break;
        case BWL_ERR_ALLOC: pcErr = "BWL_ERR_ALLOC"; break;
        default: pcErr = "UNKNOWN ERROR"; break;
    }
    fprintf( stderr, "err=%s in '%s()' [%s @ %d]\n", pcErr, pcFunc, pcFile, lLine );
}


int32_t BWL_SetOBSSCoEx(BWL_Handle hBwl, uint32_t ulCoEx)
{
    int32_t      err = 0;
    void         *wl = hBwl->wl;

    BWL_LOCK();

    ulCoEx = htod32(ulCoEx);
    err = wlu_iovar_set( wl, "obss_coex", &ulCoEx, sizeof( ulCoEx ) );
    BWL_CHECK_ERR( err );

    BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_IsPresent()
*
*   Purpose:
*       Checks to see if the device is connected to the system (plugged in) before
*       we initialize the BWL API
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_IsPresent
(
    uint32_t    *pulPresent,
    char        *pcIfName,
    uint32_t    ulLength
)
{
    int32_t         err = 0;
    char        *args[10];


    wl_find(args, 10);

    if (args[0] != NULL)
    {
        *pulPresent = 1;
        strncpy(pcIfName, args[0], ulLength);
    }

    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetPresentIFaces()
*
*   Purpose:
*       Return an array of interface names that exist in the system Checks to see if there are any WiFi devices connected to the system and
*       returns an array of arguments filled with the interface names.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetPresentIFaces
(
    char        *args[],
    uint32_t    size
)
{
    int32_t         err = 0;
    struct ifreq    ifr;

    memset(&ifr, 0, sizeof(struct ifreq));


    wl_find(args, size);

//    err = (strcmp(args[0], "") == 0) ? -1 : 0;

    if (err != 0)
    {
        goto BWL_EXIT;
    }

BWL_EXIT:
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Init()
*
*   Purpose:
*       Finds the wireless interface and initialize the BWL handle.  The
*       handle is used throughout the all BWL functions.  This function
*       must be called prior to accessing any other BWL function.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_Uninit()
*
*******************************************************************************/
int32_t BWL_Init
(
    BWL_Handle  *phBwl,  /* [out] the driver handle to be filled in */
    char        *iface   /* [in] interface to bind to*/
)
{
    int32_t         err        = BWL_ERR_IOCTL;
    char            tempBuffer[1024];
    FILE           *fp         = NULL;
    BWL_Handle      hBwl       = NULL;

    memset( tempBuffer, 0, sizeof(tempBuffer) );
    fp = fopen( "/proc/modules", "r" );
    if ( fp )
    {
        int num_bytes = fread( tempBuffer, 1, sizeof(tempBuffer)-1, fp);
        if ( num_bytes )
        {
            /* if the wl driver is found as one of the installed modules */
            if ( strstr( tempBuffer, "wl" ) )
            {
                err = BWL_ERR_SUCCESS;
            }
        }
        fclose( fp );
    }

    if ( err == BWL_ERR_IOCTL ) return (err);

    hBwl = (BWL_Handle) malloc( sizeof( BWL_P_Handle ) );
    if( hBwl == 0 )
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }
    memset( hBwl, 0, sizeof( BWL_P_Handle ) );

    if (iface != NULL)
    {
        strncpy(hBwl->ifr.ifr_name, iface, IFNAMSIZ);
    }
    else
    {
        char *argv[1];
        memset(argv, 0, sizeof(argv));

        /* use default interface */
        wl_find(argv, 1);
        if (argv[0] != NULL)
        {
            strncpy(hBwl->ifr.ifr_name, argv[0], IFNAMSIZ);
            free(argv[0]);
        }
    }

    /*printf("%s:%u: ifname (%s) \n", __FILE__, __LINE__, hBwl->ifr.ifr_name );*/
    err = bwl_wl_check( (void *)&hBwl->ifr );
    BWL_CHECK_ERR( err );

    pthread_mutex_init(&hBwl->bwlMutex, NULL);

    hBwl->wl = (void*)&hBwl->ifr;

    *phBwl = hBwl;

BWL_EXIT:
    if (err)
    {
        free(hBwl);
    }
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Uninit()
*
*   Purpose:
*       Frees the resources used by by BWL_Init().  Call this function to clean
*       up.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_Init()
*
*******************************************************************************/
int32_t BWL_Uninit
(
    BWL_Handle  hBwl    /* [in] BWL Handle */
)
{
    if( hBwl )
    {
        pthread_mutex_destroy(&hBwl->bwlMutex);
        free( hBwl );
    }

    return( BWL_ERR_SUCCESS );
}


/*******************************************************************************
*
*   Name: BWL_GetDriverError()
*
*   Purpose:
*       Returns the driver's error.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetDriverError
(
    BWL_Handle  hBwl, /* [in] BWL Handle */
    int32_t     *plDriverErrCode /* [out] the driver's error code */
)
{
    int32_t err = 0;

    BWL_LOCK();

    err = bwl_get_driver_error(hBwl, plDriverErrCode);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Up()
*
*   Purpose:
*       Brings the wireless device up before association.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_Up
(
    BWL_Handle  hBwl  /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    /* bring the dongle out of reset */
    err = wlu_set( wl, WLC_UP, NULL, 0 );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Down()
*
*   Purpose:
*       Brings the wireless device down.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_Down
(
    BWL_Handle  hBwl /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    /* bring the dongle out of reset */
    err = wlu_set( wl, WLC_DOWN, NULL, 0 );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_IsUp()
*
*   Purpose:
*       Checks to see if the device is up.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_IsUp
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    uint32_t    *pulUp  /* [out] 1 is up, 0 is down */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulUp = 0;

    BWL_LOCK();

    err = wlu_get( wl, WLC_GET_UP, &ulUp, sizeof( ulUp ) );
    BWL_CHECK_ERR( err );

    *pulUp = htod32( ulUp );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Scan()
*
*   Purpose:
*       Scans to find the APs.  The scan might take a few seconds to complete.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetScanResults()
*
*******************************************************************************/
int32_t BWL_Scan
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    ScanParams_t    *pScanParams
)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int32_t             params_size;
    wl_scan_params_t    *params;

    BWL_LOCK();

    params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof( uint16 );
    params      = (wl_scan_params_t*) malloc( params_size );
    if( !params )
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }
    memset( params, 0, params_size );

    /* Do a single AP scan */
    if(pScanParams->pcSSID != NULL)
    {
        params->ssid.SSID_len = strlen(pScanParams->pcSSID);
        strncpy((char*)params->ssid.SSID, pScanParams->pcSSID, sizeof(params->ssid.SSID)/sizeof(params->ssid.SSID[0]));
        params->ssid.SSID_len = htod32(params->ssid.SSID_len);
    }

    memcpy( &params->bssid, &ether_bcast, ETHER_ADDR_LEN );
    params->bss_type     = DOT11_BSSTYPE_ANY;
    params->scan_type    = 0;
    params->nprobes      = htod32(-1);
    params->active_time  = htod32(pScanParams->lActiveTime);
    params->passive_time = htod32(pScanParams->lPassiveTime);
    params->home_time    = htod32(pScanParams->lHomeTime);
    params->channel_num  = 0;

    params_size = WL_SCAN_PARAMS_FIXED_SIZE +
        dtoh32( params->channel_num ) * sizeof( uint16 );

    err = wlu_set( wl, WLC_SCAN, params, params_size );
    BWL_CHECK_ERR( err );


BWL_EXIT:
    BWL_UNLOCK();
    if( params )
        free( params );
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ScanAbort()
*
*   Purpose:
*       Abort a scan in progress.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ScanAbort()
*
*******************************************************************************/
int32_t BWL_ScanAbort
(
    BWL_Handle  hBwl       /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "scanabort", NULL, 0 );
    BWL_CHECK_ERR( err );


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetScanResults()
*
*   Purpose:
*       Gets the scan results.  Call this function after a few seconds after
*       calling BWL_Scan().
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_Scan()
*
*******************************************************************************/
int32_t BWL_GetScanResults
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    ScanInfo_t      *pData  /* [out] the scan data */
)
{
    int32_t   err = 0;
    void    *wl = hBwl->wl;
    char    *dump_buf = NULL;

    BWL_LOCK();

    dump_buf = malloc( WL_DUMP_BUF_LEN );
    if( dump_buf == NULL )
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }

    memset(dump_buf, 0, WL_DUMP_BUF_LEN);
    err = bwl_get_scan( wl, WLC_SCAN_RESULTS, dump_buf, WL_DUMP_BUF_LEN );
    BWL_CHECK_ERR( err );

    if( !err )
    {
        wl_scan_results_t   *list = (wl_scan_results_t*)dump_buf;
        wl_bss_info_t       *bi;
        uint32_t              i;

        if( list->count == 0 )
        {
            goto BWL_EXIT;
        }
        else if( list->version != WL_BSS_INFO_VERSION &&
                 list->version != LEGACY_WL_BSS_INFO_VERSION )
        {
            fprintf( stderr, "Sorry, your driver has bss_info_version %d "
                             "but this program supports only version %d.\n",
                             list->version, WL_BSS_INFO_VERSION );
            goto BWL_EXIT;
        }

        bi = list->bss_info;
        for( i = 0; i < list->count; i++,
             bi = (wl_bss_info_t*)((int8*)bi + dtoh32( bi->length )) )
        {
            err = bwl_parse_bss_info(bi, &pData[i]);
            BWL_CHECK_ERR( err );
        }
    }

BWL_EXIT:
    BWL_UNLOCK();
    if( dump_buf )
        free( dump_buf );

    return( err );
}


/*******************************************************************************
*
*   Name: BWL_DisplayScanResults()
*
*   Purpose:
*       Displays the scan results.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetScanResults()
*
*******************************************************************************/
int32_t BWL_DisplayScanResults
(
    BWL_Handle      hBwl   /* [in] BWL Handle */
)
{
    int32_t       err = 0;
    void        *wl = hBwl->wl;
    char        *dump_buf;

    BWL_LOCK();

    dump_buf = malloc( WL_DUMP_BUF_LEN );
    if( dump_buf == NULL )
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }

    err = bwl_get_scan( wl, WLC_SCAN_RESULTS, dump_buf, WL_DUMP_BUF_LEN );
    BWL_CHECK_ERR( err );

    dump_networks( dump_buf );


BWL_EXIT:
    BWL_UNLOCK();
    if( dump_buf )
        free( dump_buf );

    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetScannedApNum()
*
*   Purpose:
*       Get the number of AP in a scan.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetScanResults()
*
*******************************************************************************/
int32_t BWL_GetScannedApNum
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    uint32_t          *ulNumOfAp  /* [out] number of AP */
)
{
    int32_t       err = 0;
    void        *wl = hBwl->wl;
    char        *dump_buf;

    BWL_LOCK();

    *ulNumOfAp = 0;
    dump_buf = malloc( WL_DUMP_BUF_LEN );
    if( dump_buf == NULL )
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }

    err = bwl_get_scan( wl, WLC_SCAN_RESULTS, dump_buf, WL_DUMP_BUF_LEN );
    BWL_CHECK_ERR( err );

    if( !err )
    {
        wl_scan_results_t *list = (wl_scan_results_t*)dump_buf;
        *ulNumOfAp = list->count;
    }


BWL_EXIT:
    BWL_UNLOCK();
    if( dump_buf )
        free( dump_buf );

    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetConnectedAp()
*
*   Purpose:
*       Get information for the AP currently connected.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetConnectedAp
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    char        *pcSSID,    /* [in] the pointer to store SSID strings */
    uint32_t    ulLength,   /* [in] the length of pcSSID */
    int32_t     *plRSSI     /* [in] the pointer to store RSSI value */
)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    struct ether_addr   bssid;
    wl_bss_info_t       *bi;
    char                *pbuf;

    BWL_LOCK();

    BWL_CHECK_ERR( (pcSSID == NULL) || (plRSSI == NULL) );
    memset( &bssid, 0, sizeof( bssid ) );
    *pcSSID = '\0';
    err = wlu_get( wl, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN );

    if( err == 0 )
    {
        /* The adapter is associated. */
        #if 1
        pbuf = malloc( WLC_IOCTL_MAXLEN );
        #else
        pbuf = s_bufdata;
        #endif

        *((uint32_t*)pbuf) = htod32( WLC_IOCTL_MAXLEN );
        err = wlu_get( wl, WLC_GET_BSS_INFO, pbuf, WLC_IOCTL_MAXLEN );
        BWL_CHECK_ERR( err );

        bi = (wl_bss_info_t*)(pbuf + 4);
        if( dtoh32( bi->version ) == WL_BSS_INFO_VERSION ||
            dtoh32( bi->version ) == LEGACY_WL_BSS_INFO_VERSION )
        {
#ifdef BWL_DEBUG
            dump_bss_info(bi);
#endif
            strncpy(pcSSID, (char*)bi->SSID, ulLength);
            *plRSSI = dtoh16( bi->RSSI );
        }
        else
        {
            fprintf( stderr, "Sorry, your driver has bss_info_version %d "
                        "but this program supports only version %d.\n",
                        bi->version, WL_BSS_INFO_VERSION );
        }
        free( pbuf );
    }
    else
    {
        int32_t errcode = 0, err2 = 0;

        err2 = bwl_get_driver_error(hBwl, &errcode);
        if ( (err2 == 0) && (errcode == BCME_NOTASSOCIATED) )
        {
            err = 0;
        }

    }
BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_IsConnectedAp()
*
*   Purpose:
*       Checking to see if the stat is connnected to the AP.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetCountry()
*
*******************************************************************************/
int32_t BWL_IsConnectedAp
(
    BWL_Handle  hBwl,
    uint32_t    *pulConnect
)
{
    int32_t              err = 0;
    void                *wl = hBwl->wl;
    struct ether_addr   bssid;

    BWL_LOCK();

    memset( &bssid, 0, sizeof( bssid ) );
    err = wlu_get( wl, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN );

    /*-----------------------------------------------------------------------
     * Check to see if the return ether address is all zeros.
     * If it is all zeros, then the client is not connected to an AP.
     *-----------------------------------------------------------------------*/
    if( ETHER_ISNULLADDR(bssid.octet) )
    {
        /* not connected */
        *pulConnect = 0;
    }
    else
    {
        *pulConnect = 1;
    }

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetCountry()
*
*   Purpose:
*       Set the country code using ISO 3166 format.
*       follows ISO 3166 format
*       eg: KR/3, KP (Korea)
*           US (United States)
*           JP (Japan)
*           CN (China)
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetCountry()
*
*******************************************************************************/
int32_t BWL_SetCountry
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    char        *pcCountry  /* [in] Country string */
)
{
    int32_t         err = 0;
    void            *wl = hBwl->wl;
    wl_country_t    cspec;

    BWL_LOCK();


    /* always start out clean */
    memset( &cspec, 0, sizeof( cspec ) );
    cspec.rev = -1;

    /* parse a country spec, e.g. "US/1", or a country code.
     * cspec.rev will be -1 if not specified.
     */
    err = bwl_parse_country_spec( pcCountry, cspec.country_abbrev, &cspec.rev );

    if( err )
    {
        fprintf( stderr,
                "Argument \"%s\" could not be parsed as a country name, "
                "country code, or country code and regulatory revision.\n",
                pcCountry );
        BWL_CHECK_ERR( err = BWL_ERR_USAGE );
    }

    /* if the arg was a country spec, then fill out ccode and rev,
     * and leave country_abbrev defaulted to the ccode
     */
    if( cspec.rev != -1 )
    {
        memcpy( cspec.ccode, cspec.country_abbrev, WLC_CNTRY_BUF_SZ );
    }

    cspec.rev = htod32(cspec.rev);

    /* first try the country iovar */
    if (cspec.rev == -1 && cspec.ccode[0] == '\0')
        err = wlu_iovar_set( wl, "country", &cspec, WLC_CNTRY_BUF_SZ );
    else
        err = wlu_iovar_set( wl, "country", &cspec, sizeof( cspec ) );

    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetCountry()
*
*   Purpose:
*       Get the country code in ISO 3166 format.
*       follows ISO 3166 format
*       eg: KR/3, KP (Korea)
*           US (United States)
*           JP (Japan)
*           CN (China)
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetCountry()
*
*******************************************************************************/
int32_t BWL_GetCountry
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    char        *pcCountry, /* [out] Country string */
    int32_t     *pRev       /* [out] Revision */
)
{
    int32_t         err = 0;
    void            *wl = hBwl->wl;
    wl_country_t    cspec;

    BWL_LOCK();

    BWL_CHECK_ERR( (pcCountry == NULL) || (pRev == NULL) || (hBwl == NULL) );

    /* always start out clean */
    memset( &cspec, 0, sizeof( cspec ) );
    cspec.rev = -1;

    /* first try the country iovar */
    err = wlu_iovar_get( wl, "country", &cspec, sizeof( cspec ) );
    BWL_CHECK_ERR( err );

    memcpy( pcCountry, cspec.country_abbrev, WLC_CNTRY_BUF_SZ );
    *pRev = dtoh32(cspec.rev);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetSsid()
*
*   Purpose:
*       Set the SSID of the AP that the client want to associate.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetSsid()
*
*******************************************************************************/
int32_t BWL_SetSsid
(
    BWL_Handle          hBwl,    /* [in] BWL Handle */
    char                *pcSsid, /* [in] AP SSID */
    struct ether_addr   *peBSSID /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;

    BWL_LOCK();

    err = bwl_set_ssid(hBwl, pcSsid, peBSSID);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetSsid()
*
*   Purpose:
*       Get the SSID of the associated AP.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetSsid()
*       BWL_GetCacheSsid()
*
*******************************************************************************/
int32_t BWL_GetSsid
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcSsid, /* [out] AP SSID */
    uint32_t    *pulLen  /* [out] SSID length */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    wlc_ssid_t  ssid;

    BWL_LOCK();

    memset(&ssid,0,sizeof(wlc_ssid_t));

    if( pcSsid == NULL )
    {
        fprintf( stderr, "SSID arg NULL ponter\n" );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    err = wlu_get( wl, WLC_GET_SSID, &ssid, sizeof( ssid ) );
    *pulLen = dtoh32( ssid.SSID_len );
    memcpy( pcSsid, ssid.SSID, *pulLen );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetCachedSsid()
*
*   Purpose:
*       Get the BSSID of the previous stored SSID.  The return the SSID
*       regardless of associatively.  This is different than the BWL_GetSsid().
*       The BWL_GetSsid() returns the currently associated SSID.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetSsid()
*       BWL_GetSsid()
*
*******************************************************************************/
int32_t BWL_GetCachedSsid
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcSsid, /* [out] AP SSID */
    uint32_t    *pulLen  /* [out] SSID length */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_get_cached_ssid(hBwl, pcSsid, pulLen);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetBssid()
*
*   Purpose:
*       Get the BSSID of the associated AP.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetSsid()
*
*******************************************************************************/
int32_t BWL_GetBssid
(
    BWL_Handle          hBwl,    /* [in] BWL Handle */
    struct ether_addr   *pbssid  /* [out] AP BSSID */
)

{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    memset( pbssid, 0, sizeof( struct ether_addr ) );
    err = wlu_get( wl, WLC_GET_BSSID, pbssid, ETHER_ADDR_LEN );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetBand()
*
*   Purpose:
*       Set the Band to use.
*       Supported Bands: Auto, 5G, 2G, All bands.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetBand()
*
*******************************************************************************/
int32_t BWL_SetBand
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    Band_t      eBand   /* [in] Auto, 5G, 2G, All bands */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulBand = 0;
    uint32_t    ii;

    BWL_LOCK();

    for (ii = 0; ii < sizeof(BandTable)/sizeof(BandTable[0]); ii++)
    {
        if (BandTable[ii].eBwl == eBand)
        {
            ulBand = htod32(BandTable[ii].eWl);
            break;
        }
    }

    err = wlu_set( wl, WLC_SET_BAND, &ulBand, sizeof( ulBand ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetBand()
*
*   Purpose:
*       Get the Band associated band.
*       Supported Bands: Auto, 5G, 2G, All bands.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetBand()
*
*******************************************************************************/
int32_t BWL_GetBand
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    Band_t      *peBand /* [out] Auto, 5G, 2G, All bands */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulBand;
    uint32_t    ii;

    BWL_LOCK();

    err = wlu_get( wl, WLC_GET_BAND, &ulBand, sizeof( ulBand ) );
    BWL_CHECK_ERR( err );

    ulBand = htod32(ulBand);
    for (ii = 0; ii < sizeof(BandTable)/sizeof(BandTable[0]); ii++)
    {
        if (BandTable[ii].eWl == ulBand)
        {
            *peBand = BandTable[ii].eBwl;
            break;
        }
    }


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetBands()
*
*   Purpose:
*       Get the Band associated bands.
*       Supported Bands: Auto, 5G, 2G, All bands.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetBand()
*
*******************************************************************************/
int32_t BWL_GetBands
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    Band_t      *peBand /* [out] Auto, 5G, 2G, All bands */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    list[3];
    uint32_t    i;
    uint32_t    j = 0;

    BWL_LOCK();

    err = wlu_get(wl, WLC_GET_BANDLIST, list, sizeof(list));
    BWL_CHECK_ERR( err );

    list[0] = dtoh32(list[0]);
    list[1] = dtoh32(list[1]);
    list[2] = dtoh32(list[2]);

    for (j = 0; j < sizeof(list)/sizeof(list[0]); j++)
    {
        for (i = 0; i < sizeof(BandTable)/sizeof(BandTable[0]); i++)
        {
            if (BandTable[i].eWl == list[j])
            {
                *peBand = BandTable[i].eBwl;
                break;
            }
        }
    }

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetChannel()
*
*   Purpose:
*       Set the channel use..
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetChannel()
*
*******************************************************************************/
int32_t BWL_SetChannel
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    uint32_t    ulChan  /* [in] Channel */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    ulChan = htod32( ulChan );
    err = wlu_set( wl, WLC_SET_CHANNEL, &ulChan, sizeof( ulChan ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetChannel()
*
*   Purpose:
*       Set the channel use..
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetChannel()
*
*******************************************************************************/
int32_t BWL_GetChannel
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    uint32_t    *pulChan    /* [in] Channel */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulChan;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "chanspec", &ulChan, sizeof( ulChan ) );
    BWL_CHECK_ERR( err );

    *pulChan = CHSPEC_CHANNEL( ulChan );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetDfsSpec()
*
*   Purpose:
*       Set the Dynamic Frequency Selection(DFS) mode of operation.
*       Supported specification 11h only: None, Loose, Strict
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetDfsSpec
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    DfsSpec_t   eDfsSpec   /* [in] None, Loose, Strict */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    int         spect;

    BWL_LOCK();

    switch(eDfsSpec)
    {
    case eDfsSpec_Loose:
        spect = 1; /* Loose 11h*/
        break;
    case eDfsSpec_Strict:
        spect = 2; /* strict 11h */
        break;
    default:
        spect = 0; /* disabled 11h */
        break;
    }

    err = wlu_set( wl, WLC_SET_SPECT_MANAGMENT, &spect, sizeof( spect ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetRadar()
*
*   Purpose:
*       Enables or disables the Radar
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetRadar(BWL_Handle hBwl, bool bEnable)
{
    int32_t     err = 0;
#ifndef AARDVARK_WIFI_DRIVER
    (void)hBwl;
    (void)bEnable;
     fprintf(stderr, "%s(): Not support in this driver\n", __FUNCTION__);
#else
    void        *wl = hBwl->wl;
    int val;

    BWL_LOCK();

    val = bEnable ? 1 : 0;
    val = htod32(val);

    err = wlu_set( wl, WLC_SET_RADAR, &val, sizeof( val ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
#endif

    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetChannelsByCountry()
*
*   Purpose:
*       Get all the supported channels by the country
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetChannel()
*
*******************************************************************************/
int32_t BWL_GetChannelsByCountry
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    char        *pcCountry,     /* [in] country code in ISO 3166 format */
    Band_t      eBand,          /* [in] which band */
    uint32_t    aulChannels[],  /* [out] all channels */
    uint32_t    *pulChannels    /* [out] number of channels */
)
{
    int32_t                     err = 0;
    void                        *wl = hBwl->wl;
    wl_channels_in_country_t    *cic;
    uint32_t                    ii, len;
    uint32_t                    ulChannels;
    uint32_t                    i;

    BWL_LOCK();

    cic = (wl_channels_in_country_t *)hBwl->s_bufdata;
    cic->buflen = WLC_IOCTL_MAXLEN;
    cic->count = 0;

    /* country abbrev must follow */
    if( pcCountry == NULL )
    {
        fprintf( stderr, "missing country abbrev\n" );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    len = strlen( pcCountry );
    if ((len > 3) || (len < 2))
    {
        fprintf( stderr, "invalid country abbrev: %s\n", pcCountry );
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    strcpy( cic->country_abbrev, pcCountry );

    for (i = 0; i < sizeof(BandTable)/sizeof(BandTable[0]); i++)
    {
        if (BandTable[i].eBwl == eBand)
        {
            cic->band = htod32(BandTable[i].eWl);
            break;
        }
    }
    cic->buflen = htod32( cic->buflen );
    cic->band   = htod32( cic->band );
    cic->count  = htod32( cic->count );
    err = wlu_get( wl, WLC_GET_CHANNELS_IN_COUNTRY, hBwl->s_bufdata, WLC_IOCTL_MAXLEN );
    BWL_CHECK_ERR( err );

    ulChannels = dtoh32(cic->count);
    if( ulChannels > BWL_MAX_CHANNEL )
    {
        ulChannels = BWL_MAX_CHANNEL;
    }

    for( ii = 0; ii < ulChannels ; ii++)
    {
        aulChannels[ ii ] = dtoh32( cic->channel[ ii ] );
    }
    *pulChannels = ulChannels;


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetInfraMode()
*
*   Purpose:
*       Set the network operating mode.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetInfraMode()
*
*******************************************************************************/
int32_t BWL_SetInfraMode
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    NetOpMode_t eNetOpMode  /* [in] ad-hoc or infrasture */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_set_infra_mode(hBwl, eNetOpMode);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetInfraMode()
*
*   Purpose:
*       Set the network operating mode.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetInfraMode()
*
*******************************************************************************/
int32_t BWL_GetInfraMode
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    NetOpMode_t *peNetOpMode    /* [out] ad-hoc or infrasture */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_get_infra_mode(hBwl, peNetOpMode);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetAuthType()
*
*   Purpose:
*       Set the authentication type: open, shared, or open & shared.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetAuthType()
*
*******************************************************************************/
int32_t BWL_SetAuthType
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    AuthType_t  eAuthType   /* [in] authentication type */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_set_auth_type(hBwl, eAuthType);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetAuthType()
*
*   Purpose:
*       Set the authentication type: open, shared, or open & shared.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetAuthType()
*
*******************************************************************************/
int32_t BWL_GetAuthType
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    AuthType_t  *peAuthType     /* [out] authentication type */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_get_auth_type(hBwl, peAuthType);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_GetWpaSupStatus(BWL_Handle hBwl, SupStatus_t *pStatus)
{
    int32_t       err = 0;
    void          *wl = hBwl->wl;
    uint32_t      ulSupWpaStatus = 0;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "sup_auth_status", &ulSupWpaStatus, sizeof( ulSupWpaStatus) );
    BWL_CHECK_ERR( err );

    switch (htod32(ulSupWpaStatus))
    {
    case WLC_SUP_DISCONNECTED:
        *pStatus = eSupStatusDisconnected;
        break;

    case WLC_SUP_CONNECTING:
    case WLC_SUP_IDREQUIRED:
    case WLC_SUP_AUTHENTICATING:
    case WLC_SUP_AUTHENTICATED:
    case WLC_SUP_KEYXCHANGE:
        *pStatus = eSupStatuseConnecting;
        break;

    case WLC_SUP_KEYED:
        *pStatus = eSupStatusConnected;
        break;

    default:
        *pStatus = eSupStatusError;
        break;
    }
    BWL_EXIT:
        BWL_UNLOCK();
        return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetWpaSup()
*
*   Purpose:
*       Set WPA supplication: internal or external.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetWpaSup()
*
*******************************************************************************/
int32_t BWL_SetWpaSup
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaSup_t    eWpaSup     /* [in] driver supplicant */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_set_wpa_sup(hBwl, eWpaSup);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetWpaSup()
*
*   Purpose:
*       Get WPA supplication: internal or external.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetWpaSup()
*
*******************************************************************************/
int32_t BWL_GetWpaSup
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaSup_t    *peWpaSup   /* [out] driver supplicant */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_get_wpa_sup(hBwl, peWpaSup);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetWpaAuth()
*
*   Purpose:
*       Get WPA authentication: none, wpa psk, wpa2 psk.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetWpaAuth()
*
*******************************************************************************/
int32_t BWL_SetWpaAuth
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    WpaAuth_t   eWpaAuth /* [in] wpa authentication: none, wpa psk, wpa2 psk */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_set_wpa_auth(hBwl, eWpaAuth);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetWpaAuth()
*
*   Purpose:
*       Set WPA authentication: none, wpa psk, wpa2 psk.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetWpaAuth()
*
*******************************************************************************/
int32_t BWL_GetWpaAuth
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    WpaAuth_t   *peWpaAuth  /* [out] wpa authentication  */
)
{
    int32_t       err = 0;

    BWL_LOCK();

    err = bwl_get_wpa_auth(hBwl, peWpaAuth);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetWSec()
*
*   Purpose:
*       Set wireless security: none, wep, tkip, aes.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetWSec()
*
*******************************************************************************/
int32_t BWL_SetWSec
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSec_t      eWSec   /* [in] wireless security: none, wep, tkip, aes */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_set_wsec(hBwl, eWSec);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetWSec()
*
*   Purpose:
*       Get wireless security: none, wep, tkip, aes.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetWSec()
*
*******************************************************************************/
int32_t BWL_GetWSec
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSec_t      *peWSec /* [out] wireless security: none, wep, tkip, aes */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_get_wsec(hBwl, peWSec);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetWSecKey()
*
*   Purpose:
*       Set security passphrase/key for tkip or aes.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_SetWSecKey
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    char        *pcKey  /* [out] security passphrass/key */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_set_wsec_key(hBwl, pcKey);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetWSecKey()
*
*   Purpose:
*       Get the passphrase from the driver.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetWSecKey
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    char        *pcKey,  /* [out] security passphrass/key */
    uint32_t      ulLength /* [in] WSEC_MIN_PSK_LEN =< length =< WSEC_MAX_PSK_LEN */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_get_wsec_key(hBwl, pcKey, ulLength);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetWepIndex()
*
*   Purpose:
*       Set WEP key index.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetWepIndex()
*       BWL_AddWepKey()
*
*******************************************************************************/
int32_t BWL_SetWepIndex
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    uint32_t    ulIndex     /* [in] WEP index */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_set_wep_index(hBwl, ulIndex);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetWepIndex()
*
*   Purpose:
*       Get WEP key index.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetWepIndex()
*       BWL_AddWepKey()
*
*******************************************************************************/
int32_t BWL_GetWepIndex
(
    BWL_Handle  hBwl,       /* [in] BWL Handle */
    uint32_t    *pulIndex   /* [out] WEP index */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_get_wep_index(hBwl, pulIndex);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_AddWepKey()
*
*   Purpose:
*       Add a WEP key based on the for a key index.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetWepIndex()
*       BWL_GetWepIndex()
*
*******************************************************************************/
int32_t BWL_AddWepKey
(
    BWL_Handle      hBwl,           /* [in] BWL Handle */
    uint32_t          ulIndex,        /* [in] key index [0:3] */
    char            *pcKey,         /* [in] key string */
    CryptoAlgo_t    eAlgoOverride,  /* [in] used for 16 bytes key */
    uint32_t          ulIsPrimary     /* [in] type of key */
)
{
    int32_t         err = 0;

    BWL_LOCK();

    err = bwl_add_wep_key(hBwl, ulIndex, pcKey, eAlgoOverride, ulIsPrimary);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectNoWep()
*
*   Purpose:
*       Associate without encryption.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectNoWep
(
    BWL_Handle    hBwl,       /* [in] BWL Handle */
    NetOpMode_t   eNetOpMode, /* [in] infrastructure or adhoc */
    char          *pcSSID,     /* [in] SSID of the AP */
    struct ether_addr *peBSSID /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthTypeOpen;
    Credential.eWpaAuth   = eWpaAuthDisabled;
    Credential.eWSec      = eWSecNone;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.ulWepIndex = 0; /* doesn't need for NO WEP */
    Credential.peBSSID = peBSSID;

    /* clear the key, doesn't need it */
    memset( Credential.acKey, 0, sizeof( Credential.acKey ) );

    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential);

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectWep()
*
*   Purpose:
*       Associate using WEP key.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectWep
(
    BWL_Handle          hBwl,       /* [in] BWL Handle */
    NetOpMode_t         eNetOpMode, /* [in] infrastructure or adhoc */
    char                *pcSSID,    /* [in] SSID of the AP */
    char                *pcKey,     /* [in] key string */
    uint32_t            ulKeyIndex, /* [in] 0-3 key index */
    AuthType_t          eAuthType,  /* [in] open, shared, open & shared */
    struct ether_addr   *peBSSID    /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthType;
    Credential.eWpaAuth   = eWpaAuthDisabled;
    Credential.eWSec      = eWSecWep;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.ulWepIndex = ulKeyIndex;
    Credential.peBSSID    = peBSSID;

    memcpy( Credential.acKey, pcKey, strlen( pcKey ) );
    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential );

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectWpaTkip()
*
*   Purpose:
*       Associate using TKIP key.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectWpaTkip
(
    BWL_Handle          hBwl,       /* [in] BWL Handle */
    NetOpMode_t         eNetOpMode, /* [in] infrastructure or adhoc */
    char                *pcSSID,    /* [in] SSID of the AP */
    char                *pcKey,     /* [in] key string */
    struct ether_addr   *peBSSID    /* [in] BSSID of the AP */
)
{
    int32_t               err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthTypeOpen;
    Credential.eWpaAuth   = eWpaAuthWpaPsk;
    Credential.eWSec      = eWSecTkip;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.peBSSID    = peBSSID;

    memcpy( Credential.acKey, pcKey, strlen( pcKey ) );
    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential );

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectWpaAes()
*
*   Purpose:
*       Associate using AES key.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectWpaAes
(
    BWL_Handle          hBwl,       /* [in] BWL Handle */
    NetOpMode_t         eNetOpMode, /* [in] infrastructure or adhoc */
    char                *pcSSID,    /* [in] SSID of the AP */
    char                *pcKey,     /* [in] key string */
    struct ether_addr   *peBSSID    /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthTypeOpen;
    Credential.eWpaAuth   = eWpaAuthWpaPsk;
    Credential.eWSec      = eWSecAes;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.peBSSID    = peBSSID;

    memcpy( Credential.acKey, pcKey, strlen( pcKey ) );
    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential );

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectWpa2Tkip()
*
*   Purpose:
*       Associate using WPA2 TKIP key.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*
*******************************************************************************/
int32_t BWL_ConnectWpa2Tkip
(
    BWL_Handle          hBwl,       /* [in] BWL Handle */
    NetOpMode_t         eNetOpMode, /* [in] infrastructure or adhoc */
    char                *pcSSID,    /* [in] SSID of the AP */
    char                *pcKey,     /* [in] key string */
    struct ether_addr   *peBSSID    /* [in] BSSID of the AP */
)
{
    int32_t             err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthTypeOpen;
    Credential.eWpaAuth   = eWpaAuthWpa2Psk;
    Credential.eWSec      = eWSecTkip;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.peBSSID    = peBSSID;

    memcpy( Credential.acKey, pcKey, strlen( pcKey ) );
    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential );

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectWpa2Aes()
*
*   Purpose:
*       Associate using WPA2 AES key.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectWpa2Aes
(
    BWL_Handle          hBwl,       /* [in] BWL Handle */
    NetOpMode_t         eNetOpMode, /* [in] infrastructure or adhoc */
    char                *pcSSID,    /* [in] SSID of the AP */
    char                *pcKey,     /* [in] key string */
    struct ether_addr   *peBSSID    /* [in] BSSID of the AP */
)
{
    int32_t              err = 0;
    Credential_t        Credential;

    BWL_LOCK();

    /* good pratice, always clear the structure first */
    memset( &Credential, 0, sizeof( Credential ) );

    Credential.eNetOpMode = eNetOpMode;
    Credential.eAuthType  = eAuthTypeOpen;
    Credential.eWpaAuth   = eWpaAuthWpa2Psk;
    Credential.eWSec      = eWSecAes;
    Credential.eWpaSup    = eWpaSupInternal;
    Credential.peBSSID    = peBSSID;

    memcpy( Credential.acKey, pcKey, strlen( pcKey ) );
    memcpy( Credential.acSSID, pcSSID, strlen( pcSSID ) );

    err = bwl_connect_ap( hBwl, &Credential );

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConnectAp()
*
*   Purpose:
*       Associate to an AP using a set of credentials.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_DisconnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_ConnectAp
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    Credential_t    *pCred  /* [in] connection credential */
)
{
    int32_t   err = 0;

    BWL_LOCK();

    err = bwl_connect_ap(hBwl, pCred);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_DisconnectAp()
*
*   Purpose:
*       Disassociate from an AP.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ConnectAp()
*       BWL_ConnectNoWep()
*       BWL_ConnectWep()
*       BWL_ConnectWpaAes()
*       BWL_ConnectWpaTkip()
*       BWL_ConnectWpa2Aes()
*       BWL_ConnectWpa2Tkip()
*
*******************************************************************************/
int32_t BWL_DisconnectAp
(
    BWL_Handle  hBwl    /* [in] BWL Handle */
)
{
    int32_t       err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_set( wl, WLC_DISASSOC, NULL, 0 );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_WpsConnectByPb()
*
*   Purpose:
*       Associate to an AP using WPS push button method.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_WpsConnectByPin()
*
*******************************************************************************/
#ifdef INCLUDE_WPS
int32_t BWL_WpsConnectByPb
(
    BWL_Handle  hBwl,
    char        *pcNetIf,
    char        *pKey,
    uint32_t    ulKeyLength
)
{
    int32_t       err = 0;
    char bssid[6];
    char ssid[SIZE_SSID_LENGTH] = "broadcom\0";
    uint8 wsec = 1;
    uint band_num, active_band;
    char *bssid_ptr = bssid;
    char *pin = NULL;

    BWL_LOCK();

    wps_osl_set_ifname( pcNetIf );
    config_init();

    if( find_pbc_ap((char *)bssid, (char *)ssid, &wsec) == 0 )
    {
        PRINTF(("%s, find_pbc_ap failed\n", __FUNCTION__));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    /*
     * join. If user_bssid is specified, it might not
     * match the actual associated AP.
     * An implementation might want to make sure
     * it associates to the same bssid.
     * There might be problems with roaming.
     */
    leave_network();
    if (join_network_with_bssid(ssid, wsec, bssid_ptr)) {
        PRINTF(("Can not join [%s] network, Quit...\n", ssid));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    /* update specific RF band */
    wps_get_bands(&band_num, &active_band);
    if (active_band == WLC_BAND_5G)
        active_band = WPS_RFBAND_50GHZ;
    else if (active_band == WLC_BAND_2G)
        active_band = WPS_RFBAND_24GHZ;
    else
        active_band = WPS_RFBAND_24GHZ;
    wps_update_RFBand((uint8)active_band);

    /* If user_bssid not defined, use associated AP's */
    if( wps_get_bssid( bssid ) )
    {
        PRINTF(("Can not get [%s] BSSID, Quit....\n", ssid));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }
    bssid_ptr = bssid;

    /* setup raw 802.1X socket with "bssid" destination  */
    if( wps_osl_init(bssid) != WPS_SUCCESS )
    {
        PRINTF(("Initializing 802.1x raw socket failed. \n"));
        PRINTF(("Check PF PACKET support in kernel. \n"));
        wps_osl_deinit();
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    enroll_device(pin, ssid, wsec, bssid_ptr, pKey, ulKeyLength);


BWL_EXIT:
    BWL_UNLOCK();
    wps_cleanup();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_WpsConnectByPin()
*
*   Purpose:
*       Associate to an AP using WPS PIN method.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_WpsConnectByPb()
*
*******************************************************************************/
int32_t BWL_WpsConnectByPin
(
    BWL_Handle  hBwl,
    char        *pcNetIf,
    char        *pcSsid,
    uint32_t    ulPin,
    char        *pKey,
    uint32_t    ulKeyLength
)
{
    int32_t               err = 0;
    wps_ap_list_info_t  *wpsaplist, *ap;
    uint8               wsec;
    uint32_t              band_num, active_band;
    char                *bssid_ptr = NULL;
    char                pin[9]; /* 8 digits + EOS */
    uint32_t              ulFound;

    BWL_LOCK();


    /* Set up the network interface */
    wps_osl_set_ifname( pcNetIf );
    config_init();


    if( !wps_validateChecksum( ulPin ) )
    {
        PRINTF(( "\tInvalid PIN number parameter: %x\n", ulPin ));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    /* get all the APs by calling scan and get scan results */
    wpsaplist = create_aplist();
    if( wpsaplist == NULL )
    {
        PRINTF(( "%s, create_aplist failed\n", __FUNCTION__ ));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    /* filter out the AP that supports WPS */
    wps_get_aplist( wpsaplist, wpsaplist );


    /* find the BSSID associated with this SSID */
    ulFound = 0;
    ap      = wpsaplist;
    while( ap->used == TRUE)
    {
        if( strcmp( pcSsid, (char*)ap->ssid ) == 0 )
        {
            PRINTF(( "found %s  ", ap->ssid ));
            ulFound = 1;
            break;
        }
        ap++;
    }

    if( ulFound == 0)
    {
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }


    bssid_ptr = (char*)ap->BSSID;
    wsec      = ap->wep;
    sprintf( pin, "%08u", ulPin );
    PRINTF(( "pin =%s\n", pin ));
    PRINTF(( "bssid =%s\n", bssid_ptr ));

    leave_network();

    if( join_network_with_bssid( pcSsid, wsec, bssid_ptr ) )
    {
        PRINTF(("Can not join [%s] network, Quit...\n", pcSsid));
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    /* update specific RF band */
    wps_get_bands( &band_num, &active_band );
    if (active_band == WLC_BAND_5G)
        active_band = WPS_RFBAND_50GHZ;
    else if (active_band == WLC_BAND_2G)
        active_band = WPS_RFBAND_24GHZ;
    else
        active_band = WPS_RFBAND_24GHZ;
    wps_update_RFBand( (uint8)active_band );


    /* setup raw 802.1X socket with "bssid" destination  */
    if( wps_osl_init( bssid_ptr ) != WPS_SUCCESS )
    {
        PRINTF(("Initializing 802.1x raw socket failed. \n"));
        PRINTF(("Check PF PACKET support in kernel. \n"));
        wps_osl_deinit();
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }

    enroll_device( pin, pcSsid, wsec, bssid_ptr, pKey, ulKeyLength );

BWL_EXIT:
    BWL_UNLOCK();
    wps_cleanup();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GenerateDptKey()
*
*   Purpose:
*       Set DPT mode.
*       0 - disable
*       1 - enable
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GenerateDptKey
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    WSecPmk_t   *dptkey /* [out] DPT Key */
)
{
    int32_t     err = 0;

    BWL_LOCK();

    err = bwl_generate_dpt_key(hBwl, dptkey);

    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetDptSecurity()
*
*   Purpose:
*       Set DPT security
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_SetDptSecurity
(
    BWL_Handle  hBwl   /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    dpt_wsec = 4, sup_wpa = 1, dpt_wpa_auth = 0x200;
    wsec_pmk_t  key;

    BWL_LOCK();

    err = bwl_generate_dpt_key( hBwl, &key );
    BWL_CHECK_ERR( err );

    err = wlu_iovar_set( wl, "dpt_pmk", &key, sizeof(key));
    BWL_CHECK_ERR( err );

    err = wlu_iovar_set( wl, "dpt_wsec", &dpt_wsec, sizeof(dpt_wsec));
    BWL_CHECK_ERR( err );

    err = wlu_iovar_set( wl, "sup_wpa", &sup_wpa, sizeof(sup_wpa));
    BWL_CHECK_ERR( err );

    err = wlu_iovar_set( wl, "dpt_wpa_auth", &dpt_wpa_auth, sizeof(dpt_wpa_auth));
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );

}
#endif


/*******************************************************************************
*
*   Name: BWL_Get802_11Modes()
*
*   Purpose:
*       Get 802.11n configuration modes.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_Get802_11Modes(BWL_Handle hBwl, uint32_t *pModes)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    struct ether_addr   bssid;
    wl_bss_info_t       *bi;
    ScanInfo_t          tScanInfo;
    uint32_t*           pBuf = NULL;

    BWL_LOCK();

    BWL_CHECK_ERR(pModes == NULL);

    memset( &bssid, 0, sizeof( bssid ) );
    memset( &tScanInfo, 0, sizeof( tScanInfo ) );

    err = wlu_get( wl, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN );
    BWL_CHECK_ERR( err );

    pBuf = (uint32_t*)hBwl->s_bufdata;

    /* The adapter is associated. */
    *pBuf = htod32(WLC_IOCTL_MAXLEN);
    err = wlu_get( wl, WLC_GET_BSS_INFO, hBwl->s_bufdata, WLC_IOCTL_MAXLEN );
    BWL_CHECK_ERR( err );

    bi = (wl_bss_info_t*)(hBwl->s_bufdata + 4);
    err = bwl_parse_bss_info(bi, &tScanInfo);
    BWL_CHECK_ERR( err );

    *pModes = tScanInfo.ul802_11Modes;

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetCredential()
*
*   Purpose:
*       Get the stored credential from the driver.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetCredential
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    Credential_t    *pCred  /* [out] used to store credential */
)
{
    int32_t       err = 0;
    uint32_t      ulLen;

    BWL_LOCK();

    PRINTF(( "--> BWL_GetCredential\n" ));
    err = bwl_get_infra_mode( hBwl, &(pCred->eNetOpMode) );
    BWL_CHECK_ERR( err );
    PRINTF(( "eNetOpMode = %d\n", pCred->eNetOpMode ));

    err = bwl_get_auth_type( hBwl, &(pCred->eAuthType) );
    BWL_CHECK_ERR( err );
    PRINTF(( "eAuthType = %d\n", pCred->eAuthType ));

    err = bwl_get_wsec( hBwl, &(pCred->eWSec) );
    BWL_CHECK_ERR( err );
    PRINTF(( "eWsec = %d\n", pCred->eWSec ));

    err = bwl_get_wpa_auth( hBwl, &(pCred->eWpaAuth) );
    BWL_CHECK_ERR( err );
    PRINTF(( "eWpaAuth = %d\n", pCred->eWpaAuth ));

    err = bwl_get_wpa_sup( hBwl, &(pCred->eWpaSup) );
    BWL_CHECK_ERR( err );
    PRINTF(( "eWpaSup = %d\n", pCred->eWpaSup ));

    if( pCred->eWSec == eWSecWep )
    {
        err = bwl_get_wep_index( hBwl, &(pCred->ulWepIndex) );
        BWL_CHECK_ERR( err );
        PRINTF(( "ulWepIndex = %d\n", pCred->ulWepIndex ));
    }

    err = bwl_get_wsec_key( hBwl, pCred->acKey, sizeof(pCred->acKey) );
    BWL_CHECK_ERR( err  );
    PRINTF(( "acKey = %s\n", pCred->acKey ));

    err = bwl_get_cached_ssid( hBwl, pCred->acSSID, &ulLen );
    BWL_CHECK_ERR( err );


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetConnectedInfo()
*
*   Purpose:
*       Fetch the BSS info and fill the ScanInfo_t structure
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetConnectedInfo(BWL_Handle hBwl, ScanInfo_t *pScanInfo)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    struct ether_addr   bssid;
    wl_bss_info_t       *bi;
    uint32_t            *pBuf = NULL;

    BWL_LOCK();

    BWL_CHECK_ERR(pScanInfo == NULL);

    memset( &bssid, 0, sizeof( bssid ) );

    err = wlu_get( wl, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN );
    BWL_CHECK_ERR( err );

    /* The adapter is associated. */
    pBuf = (uint32_t*)hBwl->s_bufdata;

    *pBuf = htod32( WLC_IOCTL_MAXLEN );
    err = wlu_get( wl, WLC_GET_BSS_INFO, hBwl->s_bufdata, WLC_IOCTL_MAXLEN );
    BWL_CHECK_ERR( err );

    bi = (wl_bss_info_t*)(hBwl->s_bufdata + 4);
    err = bwl_parse_bss_info(bi, pScanInfo);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );

}


/*******************************************************************************
*
*   Name: BWL_GetWepKey()
*
*   Purpose:
*       Get the stored WEP key from the driver.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetWepKey
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    uint32_t    ulIndex,        /* [in] WEP index 0-3 */
    uint32_t    ulIsPrimary,    /* [in] WEP current used index */
    char        *pcKey,         /* [out] WEP key */
    uint32_t    *pulLength      /* [in]/[out] WEP key length must be >= DOT11_MAX_KEY_SIZE */
)
{
    int32_t         err = 0;
    void            *wl = hBwl->wl;
    wl_wsec_key_t   key;
    uint32_t        ulLen;

    BWL_LOCK();

    memset( &key, 0, sizeof( key ) );

    if( (NULL == pcKey) || (NULL == pulLength) ||
        (*pulLength < DOT11_MAX_KEY_SIZE) )
    {
        err = BWL_ERR_PARAM;
        BWL_CHECK_ERR( err );
    }

    if( ulIsPrimary )
        key.flags = WL_PRIMARY_KEY;

    key.index  = htod32( ulIndex );

    err = wlu_get( wl, WLC_GET_KEY, &key, sizeof( key ) );
    BWL_CHECK_ERR( err );

    ulLen = htod32( key.len );

    /* make sure that there is enough buffer to store the key */
    if( *pulLength < ulLen )
    {
        err = BWL_ERR_PARAM;
        BWL_CHECK_ERR( err );
    }

    /* update the key length and key */
    strncpy( pcKey, (char*)key.data, ulLen );
    *pulLength = ulLen;

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetEvent()
*
*   Purpose:
*       Set driver event message.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_ClearEvent()
*
*******************************************************************************/
int32_t BWL_SetEvent
(
    BWL_Handle      hBwl,
    EventId_t       eEvent
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    i;
    uint8_t     event_inds_mask[ WL_EVENTING_MASK_LEN ]; /* 128-bit mask */

    BWL_LOCK();

    err = wlu_iovar_get( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN );
    BWL_CHECK_ERR( err );

    for (i = 0; i < sizeof(EventMessageTable)/sizeof(EventMessageTable[0]); i++)
    {
        if (EventMessageTable[i].eBwl == eEvent)
        {
            event_inds_mask[EventMessageTable[i].eWl / 8] |= 1 << (EventMessageTable[i].eWl % 8);

            err = wlu_iovar_set( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN );
            BWL_CHECK_ERR( err );
            break;
        }
    }

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ClearEvent()
*
*   Purpose:
*       Set driver's event message.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetEvent()
*       BWL_ParseEvent()
*
*******************************************************************************/
int32_t BWL_ClearEvent
(
    BWL_Handle      hBwl,
    EventId_t       eEvent
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    i;
    uint8_t     event_inds_mask[ WL_EVENTING_MASK_LEN ]; /* 128-bit mask */

    BWL_LOCK();

    err = wlu_iovar_get( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN );
    BWL_CHECK_ERR( err );

    for (i = 0; i < sizeof(EventMessageTable)/sizeof(EventMessageTable[0]); i++)
    {
        if (EventMessageTable[i].eBwl == eEvent)
        {
            event_inds_mask[EventMessageTable[i].eWl / 8] &= ~(1 << (EventMessageTable[i].eWl % 8));
            err = wlu_iovar_set( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN );
            BWL_CHECK_ERR( err );
            break;
        }
    }

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ParseEvent()
*
*   Purpose:
*       Parse the driver's event message.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetEvent()
*       BWL_ClearEvent()
*
*******************************************************************************/
int32_t BWL_ParseEvent
(
    BWL_Handle      hBwl,
    void            *pBuff,
    uint32_t        ulBufLength,
    EventMessage_t  *pEvent
)
{
    int32_t         err = BWL_ERR_USAGE;
    void            *wl = hBwl->wl;
    uint32_t        i;
    bcm_event_t     *event;
    uint32_t        event_type;
    EventMessage_t  bwlEvent;
    uint32          result = 0;

    BWL_LOCK();

    if( wl == NULL )
        goto BWL_EXIT;

    if ( (pBuff == NULL) || (pEvent == NULL) || (ulBufLength < sizeof(bcm_event_t)) )
    {
        BWL_CHECK_ERR(err = BWL_ERR_PARAM);
    }

    event           = (bcm_event_t *)pBuff;
    event_type      = bwl_g_swap ? event->event.event_type : bcmswap32(event->event.event_type);
    result          = bwl_g_swap ? event->event.status : bcmswap32(event->event.status);
    bwlEvent.reason = bwl_g_swap ? event->event.reason : bcmswap32(event->event.reason);
    bwlEvent.addr   = event->event.addr;

    for (i = 0; i < sizeof(EventMessageTable)/sizeof(EventMessageTable[0]); i++)
    {
        if (EventMessageTable[i].eWl == event_type)
        {
            bwlEvent.id = EventMessageTable[i].eBwl;

            switch(result)
            {
            case WLC_E_STATUS_SUCCESS:
                bwlEvent.result = BWL_ERR_SUCCESS;
                break;
            case WLC_E_STATUS_TIMEOUT:
                bwlEvent.result = BWL_ERR_TIMEOUT;
                break;
            case WLC_E_STATUS_ABORT:
                bwlEvent.result = BWL_ERR_CANCELED;
                break;
            case WLC_E_STATUS_PARTIAL:
                bwlEvent.result = BWL_ERR_PARTIAL;
                break;
            default:
                bwlEvent.result = BWL_ERR_GENERIC;
                break;
            }
            err = BWL_ERR_SUCCESS;
            break;
        }
    }

    *pEvent = bwlEvent;

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetSupAuthStatus()
*
*   Purpose:
*       This function is to get WPA authentication status.
*           (ie 4 way handshake status)
*       WLC_SUP_KEYD(6) is the status for authentication is completed.
*       Below shows the ioctl which this function is used
*       "sup_auth_status"(iovar):
*       get WPA authentication status
*       - WLC_SUP_DISCONNECTED(0): Not connected
*       - WLC_SUP_AUTHENTICATED(3): In authentication sequence
*       - WLC_SUP_KEYXCHANGE(5): In key exchange sequence
*       - WLC_SUP_KEYED(6): authentication completed.
*
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetSupAuthStatus
(
    BWL_Handle  hBwl,
    uint32_t    *pulStatus
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    uStatus;

    BWL_LOCK();

    *pulStatus = 0;

    err = wlu_iovar_get( wl, "sup_auth_status", &uStatus, sizeof( uStatus ) );
    BWL_CHECK_ERR( err );

   *pulStatus = (uint32_t) htod32( uStatus );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetLinkStatus()
*
*   Purpose:
*       Returns the link status.  Return 1 if STA is connected (link is up), 0 if STA is
*       disconnected (link is down).
*
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetLinkStatus
(
    BWL_Handle  hBwl,        /* [in] BWL Handle */
    uint32_t    *pulIsLinkUp /* [out] 1 is up, 0 is down */
)
{
    BWL_LOCK();

    bwl_get_link_status(hBwl, pulIsLinkUp);

    BWL_UNLOCK();
    return BWL_ERR_SUCCESS;
}


/*******************************************************************************
*
*   Name: BWL_GetRpcAgg()
*
*   Purpose:
*       Get RPC aggregation.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetRpcAgg()
*
*******************************************************************************/
int32_t BWL_GetRpcAgg
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulAgg  /* [out] Aggregation value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulAgg;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "rpc_agg", &ulAgg, sizeof( ulAgg ) );
    BWL_CHECK_ERR( err );

    *pulAgg = htod32( ulAgg );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetRpcAgg()
*
*   Purpose:
*       Set RPC aggregation.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetRpcAgg()
*
*******************************************************************************/
int32_t BWL_SetRpcAgg
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulAgg  /* [in] Aggregation value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "rpc_agg", &ulAgg, sizeof( ulAgg ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetHtRestrict()
*
*   Purpose:
*       Get HT Rate restrict value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetHtRestrict()
*
*******************************************************************************/
int32_t BWL_GetHtRestrict
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal   /* [out] Restrict value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "ht_wsec_restrict", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetHtRestrict()
*
*   Purpose:
*       Set RPC aggregation.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetHtRestrict()
*
*******************************************************************************/
int32_t BWL_SetHtRestrict
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] Restrict value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "ht_wsec_restrict", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetSisoTx()
*
*   Purpose:
*       Get SISO TX value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetSisoTx()
*
*******************************************************************************/
int32_t BWL_GetSisoTx
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal  /* [out] SISO Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "siso_tx", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetSisoTx()
*
*   Purpose:
*       Set SISO TX value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetSisoTx()
*
*******************************************************************************/
int32_t BWL_SetSisoTx
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] SISO Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "siso_tx", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetMimoTx()
*
*   Purpose:
*       Get MIMO TX value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetMimoTx()
*
*******************************************************************************/
int32_t BWL_GetMimoTx
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal  /* [out] MIMO Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "mimo_txbw", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetMimoTx()
*
*   Purpose:
*       Set MIMO TX value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetMimoTx()
*
*******************************************************************************/
int32_t BWL_SetMimoTx
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] MIMO Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    ulVal = htod32(ulVal);
    err = wlu_iovar_set( wl, "mimo_txbw", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetChanspec()
*
*   Purpose:
*       Set chanspec parameters.
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetChanspec
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    ChanSpec_t      *pChanSpec  /* [in] ChanSpec settings */
)
{
    int32_t     err = 0;
    uint32_t    chanSpec = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    chanSpec |= pChanSpec->ulChan;
    if (eBand2G == pChanSpec->eBand)
    {
        chanSpec |= WL_CHANSPEC_BAND_2G;
    }
    else if (eBand5G == pChanSpec->eBand)
    {
        chanSpec |= WL_CHANSPEC_BAND_5G;
    }
    if (eBandwidth20MHz == pChanSpec->eBandwidth)
    {
        chanSpec |= WL_CHANSPEC_BW_20;
    }
    else if (eBandwidth40MHz == pChanSpec->eBandwidth)
    {
        chanSpec |= WL_CHANSPEC_BW_40;
    }
    if (eSideband_Lower == pChanSpec->eSideband)
    {
        chanSpec |= WL_CHANSPEC_CTL_SB_LOWER;
    }
    else if (eSideband_Upper == pChanSpec->eSideband)
    {
        chanSpec |= WL_CHANSPEC_CTL_SB_UPPER;
    }
#ifdef WL_CHANSPEC_CTL_SB_NONE
    else if (eSideband_None == pChanSpec->eSideband)
    {
        chanSpec |= WL_CHANSPEC_CTL_SB_NONE;
    }
#endif

    err = wlu_iovar_setint( wl, "chanspec", (int)chanSpec);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetDataRate()
*
*   Purpose:
*       Set the data rate for either 802.11A or 80211B/G (used for transmission/receive test).
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetDataRate
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal,  /* [in] Data rate Kbps*/
    bool bArate  /* false indicates B/G mode; true indicates A mode */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    char *p;
    int    val = 0;

    BWL_LOCK();

    /* convert to internally usable value (which needs to be in 500 Kbps units) */
    val = htod32(((ulVal * 2)/1000));

    /* construct an iovar */
    if (bArate)
    {
        strcpy(buf, "a_rate");
    }
    else
    {
        strcpy(buf, "bg_rate");
    }
    p = &buf[strlen(buf) + 1];
    memcpy(p, &val, sizeof(uint));
    p += sizeof(uint);

    err = wlu_set(wl, WLC_SET_VAR, &buf[0], (p - buf));

    BWL_CHECK_ERR( err );
BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetMcsIndex()
*
*   Purpose:
*       Set the MCS index for 802.11N (used for transmission/receive test).
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetMcsIndex
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t      ulVal  /* [in] MCS index */
)
{
    int32_t     err = 0;

    BWL_LOCK();

#if defined(NRATE_RATE_MASK) && defined(NRATE_MCS_INUSE) && defined(NRATE_STF_SHIFT) && defined(NRATE_STF_MASK)
    void        *wl = hBwl->wl;
    uint32_t    nrate = 0;
    int stream;
    nrate |= ulVal & NRATE_RATE_MASK;
    nrate |= NRATE_MCS_INUSE;

    if ((ulVal >=0) && (ulVal <= 7))
    {
        stream = 0; /* SISO */
    }
    else
    {
        stream = 3; /* SDM */
    }
    nrate |= (stream << NRATE_STF_SHIFT) & NRATE_STF_MASK;

    err = wlu_iovar_setint(wl, "nrate", (int)nrate);
#else
    (void)ulVal;
    err = -1;
#endif
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetTransmitPower()
*
*   Purpose:
*       Set the transmit power (used for transmission test).
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetTransmitPower
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    int8_t      ulVal  /* [in] Transmit power dB */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    int new_val = 0;
    bool neg_pwr = false;

    BWL_LOCK();

#if defined(WL_TXPWR_OVERRIDE) && defined(WL_TXPWR_NEG)
    if (ulVal <= 0)
    {
        neg_pwr = true;
        ulVal = -ulVal;
    }

    new_val = ulVal * 4;
    new_val |= WL_TXPWR_OVERRIDE;

    if (neg_pwr)
    {
        new_val |= WL_TXPWR_NEG;
        new_val = htod32(new_val);
        err = wlu_iovar_set( wl, "qtxpowerneg", &new_val, sizeof( new_val ) );
    }
    else
    {
        new_val = htod32(new_val);
        err = wlu_iovar_set( wl, "qtxpower", &new_val, sizeof( new_val ) );
    }
#else
    err = -1;
#endif
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ConfigurePacketEngine()
*
*   Purpose:
*       Configures the packet engine (used for transmit/recieve tests).
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_ConfigurePacketEngine
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    PacketEngineParameters_t    *pPktEngineParams  /* [in] Parameters for configuring packet engine */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    wl_pkteng_t pkteng;

    BWL_LOCK();

    memset(&pkteng, 0, sizeof(pkteng));

    if (false == pPktEngineParams->bStart)
    {
        if (true == pPktEngineParams->bTxMode)
        {
            pkteng.flags = WL_PKTENG_PER_TX_STOP;
        }
        else
        {
            pkteng.flags = WL_PKTENG_PER_RX_STOP;
        }
    }
    else
    {
        if (true == pPktEngineParams->bTxMode)
        {
            pkteng.flags = WL_PKTENG_PER_TX_START;
            pkteng.delay = pPktEngineParams->ulIFG;
            pkteng.nframes = pPktEngineParams->ulNumFrames;
            pkteng.length = pPktEngineParams->ulPacketSize;
        }
        else
        {
            pkteng.flags = WL_PKTENG_PER_RX_START;
            pkteng.flags &= ~WL_PKTENG_SYNCHRONOUS;
        }

        if (!wl_ether_atoe(pPktEngineParams->acMac, (struct ether_addr *)&pkteng.dest))
        {
            err = BWL_ERR_PARAM;
            BWL_CHECK_ERR( err );
        }
    }

    pkteng.flags = htod32(pkteng.flags);
    pkteng.delay = htod32(pkteng.delay);
    pkteng.nframes = htod32(pkteng.nframes);
    pkteng.length = htod32(pkteng.length);
    err = wlu_iovar_set( wl, "pkteng", &pkteng, sizeof( pkteng ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetStaRetryTime()
*
*   Purpose:
*       Get STA retry time.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetStaRetryTime()
*
*******************************************************************************/
int32_t BWL_GetStaRetryTime
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal  /* [out] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "sta_retry_time", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetStaRetryTime()
*
*   Purpose:
*       Set STA retry value.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetStaRetryTime()
*
*******************************************************************************/
int32_t BWL_SetStaRetryTime
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "sta_retry_time", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetMimoBwCap()
*
*   Purpose:
*       Get the mimo bandwidth cap.
*       0 - 20 Mhz only
*       1 - 40 Mhz
*       2 - 20 Mhz in 2.4G, 40Mhz in 5G
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetMimoBwCap()
*
*******************************************************************************/
int32_t BWL_GetMimoBwCap
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal  /* [out] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "mimo_bw_cap", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetMimoBwCap()
*
*   Purpose:
*       Set the mimo bandwidth cap.
*       0 - 20 Mhz only
*       1 - 40 Mhz
*       2 - 20 Mhz in 2.4G, 40Mhz in 5G
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetMimoBwCap()
*
*******************************************************************************/
int32_t BWL_SetMimoBwCap
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    if( ulVal > WLC_N_BW_20IN2G_40IN5G )
    {
        BWL_CHECK_ERR( err = BWL_ERR_PARAM );
    }
    else
    {
        ulVal = htod32( ulVal );
        err = wlu_iovar_set( wl, "mimo_bw_cap", &ulVal, sizeof( ulVal ) );
        BWL_CHECK_ERR( err );
    }


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetBwCap()
*
*   Purpose:
*       Get the bandwidth cap for either 2G or 5G band.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetBwCap()
*
*******************************************************************************/
int32_t BWL_GetBwCap
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    Band_t band_type,  /* 2G or 5G*/
    uint32_t    *pulVal   /* [out] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    struct {
        int32_t band;
        int32_t bw_cap;
    } param = {0, 0};
    uint32_t *pTmpVal; /*Temp pointer to avoid compiler warning*/

    char pbuf[WLC_IOCTL_SMLEN];

    BWL_LOCK();

    if (band_type == eBand2G)
         param.band = WLC_BAND_2G;
    else
         param.band = WLC_BAND_5G;

    err = wlu_iovar_getbuf( wl, "bw_cap", (void *)&param, sizeof(param), (void *)pbuf, sizeof(pbuf));

    BWL_CHECK_ERR( err );
    pTmpVal = (uint32_t *)pbuf;
    *pulVal = htod32( *pTmpVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetBwCap()
*
*   Purpose:
*       Set the bandwidth cap for either 2G or 5G band.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetBwCap()
*
*******************************************************************************/
int32_t BWL_SetBwCap
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    Band_t band_type, /* 2G or 5G*/
    uint32_t    ulVal  /* [in] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    struct {
        int32_t band;
        int32_t bw_cap;
    } param = {0, 0};

    BWL_LOCK();

    param.bw_cap = htod32( ulVal );
    if (band_type == eBand2G)
         param.band = WLC_BAND_2G;
    else
         param.band = WLC_BAND_5G;

    err = wlu_iovar_set( wl, "bw_cap", (void *)&param, sizeof(param));

    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetApBwCap()
*
*   Purpose:
*       Set the mimo bandwidth cap.
*       0 - 20 Mhz only
*       1 - 40 Mhz
*       2 - 20 Mhz in 2.4G, 40Mhz in 5G
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetApBwCap
(
    BWL_Handle  hBwl,           /* [in] BWL Handle */
    uint32_t    *pulBandWidth   /* [out] 0 is 20MHz or Not Connect, 1 is 40MHz */
)
{
    int32_t         err = 0;
    void            *wl = hBwl->wl;
    uint32_t        ulIsLinkUp=0;
    uint32_t        ulBandWidth=0;
    wl_bss_info_t   *bi;
    char            *pbuf=NULL;

    BWL_LOCK();

    err = bwl_get_link_status( hBwl, &ulIsLinkUp);
    BWL_CHECK_ERR( err );

    if( 0 == ulIsLinkUp )
    {  /* link is down */
        /* do nothing */
    }
    else
    {
        /* link is up */
        /* Get AP's Capability */
        pbuf = malloc( WLC_IOCTL_MAXLEN );

        if( NULL == pbuf )
        {
            BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
        }

        *((uint32_t*)pbuf) = htod32( WLC_IOCTL_MAXLEN );
        err = wlu_get( wl, WLC_GET_BSS_INFO, pbuf, WLC_IOCTL_MAXLEN );
        BWL_CHECK_ERR( err );
        bi = (wl_bss_info_t*)(pbuf + 4);

        if( 0 == memcmp(&bi->BSSID,&ether_null,sizeof(struct ether_addr)) )
        {
        }
        else
        {
            if (dtoh32(bi->nbss_cap) & HT_CAP_40MHZ)
            {
                ulBandWidth = 1;
            }
        }
    }

    *pulBandWidth = htod32( ulBandWidth );

BWL_EXIT:
    BWL_UNLOCK();
    if( pbuf )
    {
        free( pbuf );
    }
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetDptCredential()
*
*   Purpose:
*       Get the stored DPT credential from the driver.
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetDptCredential
(
    BWL_Handle          hBwl,  /* [in] BWL Handle */
    DptCredential_t    *pCred  /* [out] used to store credential */
)
{
    int32_t       err = 0;
    void        *wl = hBwl->wl;
    uint32_t      ulWpaAuth;
    uint32_t      ulWSec;
    uint32_t      ii;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "dpt_wsec", &ulWSec, sizeof( ulWSec ) );
    BWL_CHECK_ERR( err );
    pCred->eWSec = (WSec_t)(htod32( ulWSec ));
    PRINTF(( "eWSec = %d\n", pCred->eWSec ));

    err = wlu_iovar_get( wl, "dpt_wpa_auth", &ulWpaAuth, sizeof( ulWpaAuth ) );
    BWL_CHECK_ERR( err );
    pCred->eWpaAuth = (WpaAuth_t)htod32( ulWpaAuth );
    PRINTF(( "eWpaAuth = %d\n", pCred->eWpaAuth ));

    err = wlu_iovar_get( wl, "dpt_pmk", &(pCred->Pmk), sizeof(pCred->Pmk) );
    BWL_CHECK_ERR( err );

    PRINTF(("key == "));
    for( ii = 0; ii < pCred->Pmk.KeyLen; ii++ )
    {
        PRINTF(( "%c", pCred->Pmk.Key[ii] ));
    }
    PRINTF(("\n"));

    err = wlu_iovar_get( wl, "dpt_fname", &(pCred->FName), sizeof( DptFName_t ) );
    BWL_CHECK_ERR( err );

    PRINTF(("friendly name == "));
    for( ii = 0; ii < pCred->FName.Len; ii++ )
    {
        PRINTF(( "%c", pCred->FName.name[ii] ));
    }
    PRINTF(("\n"));

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetDptMode()
*
*   Purpose:
*       Get DPT mode.
*       0 - disable
*       1 - enable
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_SetDptMode()
*
*******************************************************************************/
int32_t BWL_GetDptMode
(
    BWL_Handle  hBwl,    /* [in] BWL Handle */
    uint32_t    *pulVal  /* [out] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;
    uint32_t    ulVal;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "dpt", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

    *pulVal = htod32( ulVal );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetDptMode()
*
*   Purpose:
*       Set DPT mode.
*       0 - disable
*       1 - enable
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetDptMode()
*
*******************************************************************************/
int32_t BWL_SetDptMode
(
    BWL_Handle  hBwl,  /* [in] BWL Handle */
    uint32_t    ulVal  /* [in] Value */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set( wl, "dpt", &ulVal, sizeof( ulVal ) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetDptList()
*
*   Purpose:
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetDptList
(
    BWL_Handle  hBwl,   /* [in] BWL Handle */
    DptList_t   *data
)
{
    int32_t         err = 0;
#if defined(DHD_WIFI_DRIVER) || defined(EAGLE_WIFI_DRIVER)
    (void)hBwl;
    (void)data;
    fprintf(stderr, "%s(): Not support in this driver\n", __FUNCTION__);
    err = -1;
#else
    void            *wl = hBwl->wl;
    dpt_list_t      *list;
    unsigned char   buf[1024];

    BWL_LOCK();

    err = wlu_iovar_get( wl, "dpt_list", buf, sizeof(DptList_t) );
    BWL_CHECK_ERR( err );

    {
        uint32_t i;
        list = (dpt_list_t *)buf;
        data->ulNum = list->num;
        for(i=0; i < list->num; i++)
        {
            memcpy(data->Sta[i].mac.octet, list->status[i].sta.ea.octet, 6);
            strncpy((char*)data->Sta[i].FName, (char*)list->status[i].name, list->status[i].fnlen);

            data->Sta[i].ulRssi = list->status[i].rssi;

            data->Sta[i].ulTxFailures   = list->status[i].sta.tx_failures;
            data->Sta[i].ulTxPkts       = list->status[i].sta.tx_pkts;
            data->Sta[i].ulRxUcastPkts = list->status[i].sta.rx_ucast_pkts;
            data->Sta[i].ulTxRate       = list->status[i].sta.tx_rate;
            data->Sta[i].ulRxRate       = list->status[i].sta.rx_rate;

            data->Sta[i].ulRxDecryptSucceeds = list->status[i].sta.rx_decrypt_succeeds;
            data->Sta[i].ulRxDecryptFailures = list->status[i].sta.rx_decrypt_failures;
        }
    }
BWL_EXIT:
#endif
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetCounters()
*
*   Purpose:
*        Return all the counters we care about
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetCounters
(
    BWL_Handle      hBwl,    /* [in] BWL Handle */
    WiFiCounters_t *pCounters /* Counters structure */
)
{
    int32_t         err = 0;
#if defined (AARDVARK_WIFI_DRIVER)
    (void)hBwl;
    (void)pCounters;
    err =BWL_ERR_GENERIC;
#else
    void            *wl = hBwl->wl;
    wl_cnt_info_t   *cntinfo = NULL;
    char            *buf = NULL;
    uint16          ver;
#if defined(EAGLE_WIFI_DRIVER)
    uint8           cntdata[sizeof(wl_cnt_ver_11_t)];
    wl_cnt_cbfn_info_t cbfn_info;
#else
    uint8           cntdata[WL_CNTBUF_MAX_SIZE];
#endif
    BWL_LOCK();

    UNUSED_PARAMETER(at_start_of_line);

    buf = (char*)malloc(WLC_IOCTL_MAXLEN);
    if (buf == NULL)
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }
    memset(buf, 0, WLC_IOCTL_MAXLEN);

    if ((err = wlu_iovar_get (wl, "counters", buf, WLC_IOCTL_MEDLEN)) )
    {
        BWL_CHECK_ERR( err = BWL_ERR_IOCTL );
    }

    cntinfo = (wl_cnt_info_t *)buf;
    cntinfo->version = dtoh16(cntinfo->version);
    cntinfo->datalen = dtoh16(cntinfo->datalen);

    ver = cntinfo->version;
    if (ver > WL_CNT_T_VERSION) {
        printf("\tIncorrect version of counters struct: expected %d; got %d\n",
               WL_CNT_T_VERSION, ver);
        BWL_CHECK_ERR(err = BCME_ERROR);
    }

    /* CAD 2016-05-19 - datalen is 1024 but sizeof(cntdata) is 988 ... overflow ... segfault */
    memcpy(cntdata, (cntinfo->data+BCM_XTLV_HDR_SIZE), MIN(cntinfo->datalen,sizeof(cntdata)) );

    memset( &wifi_counters, 0, sizeof(wifi_counters) );

    /* bcm_unpack_xtlv_buf() ... prints 8 lines to console */
    /* txallfrm 16691 txback 825110829 txdnlfrm 220
       txinrtstxop 775159808
       txfunfl: 3069534208 129152 3197926892 219404 txtplunfl 3197950596 txphyerror 82424
       rxbadplcp 3197950932 rxcrsglitch 1
       bphy_rxcrsglitch 48
       rxfrmtoolong 1
       rxback 808594993
       pktengrxducast 3197927312 pktengrxdmcast 1076494336
    */

    if ((err = bcm_unpack_xtlv_buf(&cbfn_info, cntdata, cntinfo->datalen, BCM_XTLV_OPTION_ALIGN32, wl_counters_cbfn))) {
        printf("error %d\n", err);
    }

    /* Similar fields are set in bwl_wl_counters.c */
    SAVPCOUNTERS(reset);
    SAVPCOUNTERS(txbyte);
    SAVPCOUNTERS(txframe);
    SAVPCOUNTERS(txretrans);
    SAVPCOUNTERS(rxerror);
    SAVPCOUNTERS(txnobuf);
    SAVPCOUNTERS(txserr);
    SAVPCOUNTERS(txphyerr);
    SAVPCOUNTERS(txerror);
    SAVPCOUNTERS(rxbyte);
    SAVPCOUNTERS(rxframe);
    SAVPCOUNTERS(rxnobuf);
    SAVPCOUNTERS(rxnondata);
    SAVPCOUNTERS(rxbadcm);
    SAVPCOUNTERS(rxfragerr);
    SAVPCOUNTERS(rxtoolate);
    SAVPCOUNTERS(rxbadfcs);
    SAVPCOUNTERS(rxfrmtooshrt);
    SAVPCOUNTERS(rxf0ovfl);
    SAVPCOUNTERS(rxf1ovfl);
    SAVPCOUNTERS(pmqovfl);
    SAVPCOUNTERS(rxcrc);
    SAVPCOUNTERS(txnobuf);
    SAVPCOUNTERS(txnoassoc);
    SAVPCOUNTERS(txrunt);
    SAVPCOUNTERS(rxbadds);
    SAVPCOUNTERS(rxrunt);
    SAVPCOUNTERS(rxnoscb);
    SAVPCOUNTERS(rxbadproto);
    SAVPCOUNTERS(rxbadsrcmac);
    SAVPCOUNTERS(rxbadda);

BWL_EXIT:
    if (buf != NULL)
    {
        free(buf);
    }
    BWL_UNLOCK();
#endif
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetSamples()
*
*   Purpose:
*        Return all the samples based on input type
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetSamples
(
    BWL_Handle     hBwl,    /* [in] BWL Handle */
    BWL_SampleType nType,   /* [in] WL Sample Types */
    WiFiSamples_t *pSamples /* Samples structure */
)
{
    int ret;
    int32_t err = 0;
    void *wl = hBwl->wl;
    wl_samplecollect_args_t arg;
    wlc_rev_info_t revinfo;
    uint32 phytype;
    uint32 phyrev;

    uint16 nbytes, tag;
    wl_sampledata_t *sample_collect;
    wl_sampledata_t sample_data, *psample;
    uint32 flag, *header, sync;
    int sampledata_version = htol16(WL_SAMPLEDATA_T_VERSION);

    BWL_LOCK();

    memset(&revinfo, 0, sizeof(revinfo));

    if ((ret = wlu_get(wl, WLC_GET_REVINFO, &revinfo, sizeof(revinfo))) < 0)
	    return ret;

    phytype = dtoh32(revinfo.phytype);
    phyrev = dtoh32(revinfo.phyrev);

    arg.coll_us = 60;
    arg.cores = -1;
    arg.bitStart = -1;
    /* extended settings */
    arg.trigger = TRIGGER_NOW;
    arg.mode = nType;
    arg.post_dur = 500;
    arg.pre_dur = 0;
    arg.gpio_sel = 0;
    arg.downsamp = FALSE;
    arg.be_deaf = FALSE;
    arg.timeout = 1000;
    arg.agc = FALSE;
    arg.filter = FALSE;
    arg.trigger_state = 0;
    arg.module_sel1 = 2;
    arg.module_sel2 = 6;
    arg.nsamps = 2048;
    arg.version = WL_SAMPLECOLLECT_T_VERSION;
    arg.length = sizeof(wl_samplecollect_args_t);

    UNUSED_PARAMETER(at_start_of_line);
    UNUSED_PARAMETER(phyrev);

    if ((phytype == WLC_PHY_TYPE_HT) || (phytype == WLC_PHY_TYPE_AC)) {
	    if ((err = wlu_iovar_getbuf (wl, "sample_collect", &arg, sizeof(arg),
					    pSamples->buff, WLC_SAMPLECOLLECT_MAXLEN))) {
		    BWL_CHECK_ERR( err = BWL_ERR_IOCTL );
	    }
    } else {
	    fprintf(stderr, "Unsupported phytype=%x phyrev=%x\n", phytype, phyrev);
    }

    sample_collect = (wl_sampledata_t *)pSamples->buff;
    header = (uint32 *)&sample_collect[1];
    tag = ltoh16_ua(&sample_collect->tag);
    if (tag != WL_SAMPLEDATA_HEADER_TYPE) {
	    fprintf(stderr, "Expect SampleData Header type %d, receive type %d\n",
			    WL_SAMPLEDATA_HEADER_TYPE, tag);
	    return -1;
    }

    nbytes = ltoh16_ua(&sample_collect->length);
    flag = ltoh32_ua(&sample_collect->flag);
    sync = ltoh32_ua(&header[0]);
    if (sync != 0xACDC2009) {
	    fprintf(stderr, "Header sync word mismatch (0x%08x)\n", sync);
	    return -1;
    }

    memset(&sample_data, 0, sizeof(wl_sampledata_t));
    sample_data.version = sampledata_version;
    sample_data.size = htol16(sizeof(wl_sampledata_t));
    flag = 0;

    /* new format, used in htphy */
    do {
	    sample_data.tag = htol16(WL_SAMPLEDATA_TYPE);
	    sample_data.length = htol16(WLC_SAMPLECOLLECT_MAXLEN);

	    /* mask seq# */
	    sample_data.flag = htol32((flag & 0xff));

	    err = wlu_iovar_getbuf(wl, "sample_data", &sample_data, sizeof(wl_sampledata_t),
			    pSamples->buff, WLC_SAMPLECOLLECT_MAXLEN);
	    if (err) {
		    fprintf(stderr, "Error reading back sample collected data\n");
		    err = -1;
		    break;
	    }

	    // ptr = (uint8 *)pSamples->buff + sizeof(wl_sampledata_t);
	    psample = (wl_sampledata_t *)pSamples->buff;
	    tag = ltoh16_ua(&psample->tag);
	    nbytes = ltoh16_ua(&psample->length);
	    flag = ltoh32_ua(&psample->flag);
	    if (tag != WL_SAMPLEDATA_TYPE) {
		    fprintf(stderr, "Expect SampleData type %d, receive type %d\n",
				    WL_SAMPLEDATA_TYPE, tag);
		    err = -1;
		    break;
	    }
	    if (nbytes == 0) {
		    fprintf(stderr, "Done retrieving sample data\n");
		    err = -1;
		    break;
	    }

	    pSamples->count += nbytes;

    } while (flag & WL_SAMPLEDATA_MORE_DATA);

BWL_EXIT:
    BWL_UNLOCK();

    return( err );
}

/*******************************************************************************
*
*   Name: BWL_Get_RevInfo()
*
*   Purpose:
*       Get revision info
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetRevInfo
(
    BWL_Handle      hBwl,       /* [in] BWL Handle */
    RevInfo_t      *pRevInfo    /* [out] RevInfo  */
)
{
    int32_t   err = 0;
    void     *wl = hBwl->wl;
    wlc_rev_info_t wlc_rev_info;

    BWL_LOCK();

    if( pRevInfo == NULL )
        return( err );

    memset(pRevInfo, 0, sizeof(RevInfo_t));
    memset(&wlc_rev_info, 0, sizeof(wlc_rev_info_t));

    err = wlu_get(wl, WLC_GET_REVINFO, &wlc_rev_info, sizeof(wlc_rev_info_t));
    if (err < 0) {
        return (err);
    }

    pRevInfo->ulVendorId    = dtoh32(wlc_rev_info.vendorid);
    pRevInfo->ulDeviceId    = dtoh32(wlc_rev_info.deviceid);
    pRevInfo->ulRadioRev    = dtoh32(wlc_rev_info.radiorev);
    pRevInfo->ulChipNum     = dtoh32(wlc_rev_info.chipnum);
    pRevInfo->ulChipRev     = dtoh32(wlc_rev_info.chiprev);
    pRevInfo->ulChipPkg     = dtoh32(wlc_rev_info.chippkg);
    pRevInfo->ulCoreRev     = dtoh32(wlc_rev_info.corerev);
    pRevInfo->ulBoardId     = dtoh32(wlc_rev_info.boardid);
    pRevInfo->ulBoardVendor = dtoh32(wlc_rev_info.boardvendor);
    pRevInfo->ulBoardRev    = dtoh32(wlc_rev_info.boardrev);
    pRevInfo->ulDriverRev   = dtoh32(wlc_rev_info.driverrev);
    pRevInfo->ulUcodeRev    = dtoh32(wlc_rev_info.ucoderev);
    pRevInfo->ulBus         = dtoh32(wlc_rev_info.bus);
    pRevInfo->ulPhyType     = dtoh32(wlc_rev_info.phytype);
    pRevInfo->ulPhyRev      = dtoh32(wlc_rev_info.phyrev);
    pRevInfo->ulAnaRev      = dtoh32(wlc_rev_info.anarev);

    BWL_UNLOCK();
    return 0 ;

}


/*******************************************************************************
*
*   Name: BWL_SetRSSIEventLevels()
*
*   Purpose:
*       Set the level to which we will receive event notifications for signal
*       strengh changes
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_SetRSSIEventLevels(
    BWL_Handle      hBwl,
    int32_t         *plLevel,
    uint32_t        ulNumLevels)
{
    wl_rssi_event_t rssi;
    unsigned int     i;
    int32_t err = 0;
    void    *wl = hBwl->wl;

    BWL_LOCK();

    memset(&rssi, 0, sizeof(wl_rssi_event_t));

    rssi.rate_limit_msec = dtoh32((plLevel == NULL) ? 0 : 500);
    rssi.num_rssi_levels = ulNumLevels;

    for (i = 0; i < ulNumLevels; i++)
    {
        rssi.rssi_levels[i] = plLevel[i];
    }

    err = wlu_iovar_set(wl, "rssi_event", &rssi, sizeof(wl_rssi_event_t));
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetRSSI()
*
*   Purpose:
*       Fetch the RSSI for a current connected AP
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetRSSI(BWL_Handle hBwl, int32_t *plRSSI)
{
    int32_t err = 0;
    int32_t lRSSI = 0;
    void    *wl = hBwl->wl;

    BWL_LOCK();

    if( plRSSI == NULL )
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    err = wlu_get(wl, WLC_GET_RSSI, &lRSSI, sizeof(lRSSI));
    BWL_CHECK_ERR( err );
    *plRSSI = dtoh32(lRSSI);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetRate()
*
*   Purpose:
*       Return the current rate/speed in 500 Kbits/s units
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*
*******************************************************************************/
int32_t BWL_GetRate(BWL_Handle hBwl, int32_t *plRate)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;

    BWL_LOCK();

    if ( (plRate == NULL) || (hBwl == NULL) )
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    err = wlu_get(wl, WLC_GET_RATE, plRate, sizeof(int32_t));

    *plRate = dtoh32(*plRate);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_WoWLSetMask(BWL_Handle hBwl, uint32_t ulMask)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;
    int32_t ulWLMask = 0;

    BWL_LOCK();

    ulWLMask |= (ulMask & eWoWLMask_Magic)          ? WL_WOWL_MAGIC : 0;
    ulWLMask |= (ulMask & eWoWLMask_Disassoc)       ? WL_WOWL_DIS   : 0;
    ulWLMask |= (ulMask & eWoWLMask_LossOfBeacon)   ? WL_WOWL_BCN   : 0;
    ulWLMask |= (ulMask & eWoWLMask_NetPattern)     ? WL_WOWL_NET   : 0;

    if (hBwl == NULL)
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    err = wlu_iovar_set(wl, "wowl", &ulWLMask, sizeof(ulWLMask));

    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_WoWLActivate(BWL_Handle hBwl)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;
    int32_t ulValue = 1;

    BWL_LOCK();

    if (hBwl == NULL)
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    /* Need to also call wowl_dngldown to disable USB core before we go into WoWL mode */
    err = wlu_iovar_set(wl, "wowl_dngldown", &ulValue, sizeof(ulValue));
    BWL_CHECK_ERR( err );

    err = wlu_iovar_get(wl, "wowl_activate", &ulValue, sizeof(ulValue));

    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

int32_t BWL_WoWLForce(BWL_Handle hBwl)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;
    int32_t ulValue = 1;
    BWL_LOCK();
    if (hBwl == NULL)
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }
    err = wlu_iovar_set(wl, "wowl_force", &ulValue, sizeof(ulValue));
    BWL_CHECK_ERR( err );
BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

int32_t BWL_WoWLPatternAdd(BWL_Handle hBwl, uint32_t ulOffset, char *pMask, char *pValue)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;
    wl_wowl_pattern_t *wl_pattern;
    char *arg = hBwl->s_bufdata;
    const char *str;
    char *dst;
    uint tot = 0;

    BWL_LOCK();

    if ( (hBwl == NULL) || (pValue == NULL) )
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    memset(hBwl->s_bufdata, 0, sizeof(hBwl->s_bufdata));

    str = "wowl_pattern";
    strncpy(arg, str, strlen(str));
    arg[strlen(str)] = '\0';
    dst = arg + strlen(str) + 1;
    tot += strlen(str) + 1;

    str = "add";
    strncpy(dst, str, strlen(str));
    tot += strlen(str) + 1;

    wl_pattern = (wl_wowl_pattern_t *)(dst + strlen(str) + 1);
    dst = (char*)wl_pattern + sizeof(wl_wowl_pattern_t);

    /* Set the offset */
    wl_pattern->offset = ulOffset;


    /* Parse the mask */
    str = pMask;
    wl_pattern->masksize = htod32(wlu_pattern_atoh((char *)(uintptr)str, dst));
    if (wl_pattern->masksize == (uint)-1)
        return -1;

    dst += wl_pattern->masksize;
    wl_pattern->patternoffset = htod32((sizeof(wl_wowl_pattern_t) +
                                        wl_pattern->masksize));

    /* Parse the value */
    str = pValue;
    wl_pattern->patternsize =
            htod32(wlu_pattern_atoh((char *)(uintptr)str, dst));
    if (wl_pattern->patternsize == (uint)-1)
        return -1;
    tot += sizeof(wl_wowl_pattern_t) + wl_pattern->patternsize +
            wl_pattern->masksize;

    err = wlu_set(wl, WLC_SET_VAR, arg, tot);

    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );

}


/*******************************************************************************
*
*   Name: BWL_EScanAbort()
*
*   Purpose:
*       Abort an active escan
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetScanResults()
*
*******************************************************************************/
int32_t BWL_EScanAbort
(
    BWL_Handle      hBwl
)
{
    void *wl = hBwl->wl;
    char *p;
    int params_size = (WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params)) +
        (WL_NUMCHANNELS * sizeof(uint16));
    wl_escan_params_t *params;
    int err = 0;
    params_size += WL_SCAN_PARAMS_SSID_MAX * sizeof(wlc_ssid_t);
    params = (wl_escan_params_t*)malloc(params_size);
    if (params == NULL) {
        fprintf(stderr, "Error allocating %d bytes for scan params\n", params_size);
        return -1;
    }

    BWL_LOCK();
    memset(params, 0, params_size);

    memcpy(&params->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
    params->params.bss_type    = DOT11_BSSTYPE_ANY;
    params->params.scan_type   = 0;
    params->params.nprobes     = htod32(-1);
    params->params.active_time = htod32(-1);
    params->params.passive_time= htod32(-1);
    params->params.home_time   = htod32(-1);
    params->version     = htod32(ESCAN_REQ_VERSION);
    params->action      = htod16(WL_SCAN_ACTION_ABORT);
    params->params.channel_num = 0;

    p = (char*)params->params.channel_list;

    params_size = p - (char*)&params->params;


#if defined(linux)
    srand((unsigned)time(NULL));
    params->sync_id = htod16(rand() & 0xffff);
#else
    params->sync_id = htod16(4321);
#endif /* #if defined(linux) */

    params_size += OFFSETOF(wl_escan_params_t, params);
    err = wlu_iovar_set(wl, "escan", params, params_size);

    if (params)
    {
        free(params);
    }

    BWL_UNLOCK();
    return err;
}


/*******************************************************************************
*
*   Name: BWL_EScanResults()
*
*   Purpose:
*       The New EScan API which receives scan results in chunks.
*       YOU MUST FREE THE POINTER pData WHEN YOU ARE FINISHED WITH IT
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*       BWL_GetScanResults()
*
*******************************************************************************/
int32_t BWL_EScanResults
(
    BWL_Handle      hBwl,   /* [in] BWL Handle */
    const char      *pIface,
    ScanParams_t    *pScanParams,
    ScanInfo_t      **pData,
    uint32_t        *pulCount,
    bool            *pbStop,
    int             lTimeoutMs /* -1 wait for ever */
)
{
    void *wl = NULL;
    int fd, err=0, octets;
    struct sockaddr_ll sll;
    struct ifreq ifr;
    bcm_event_t *event;
    uint32 status;
    char *data;
    wl_escan_result_t *escan_data;
    struct escan_bss *escan_bss_head = NULL;
    struct escan_bss *escan_bss_tail = NULL;
    struct escan_bss *result;
    int event_type;
    int params_size = (WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params)) +
        (WL_NUMCHANNELS * sizeof(uint16));
    wl_escan_params_t *params;
    char *p;
    ScanInfo_t      *pScanData = NULL;
    int count = 0;
    int timeCount = 0;

    BWL_LOCK();

    if ( (hBwl == NULL) || (pIface == NULL) || (pData == NULL) || (pulCount == NULL) )
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }
    wl = hBwl->wl;





    strncpy(ifr.ifr_name, pIface, (IFNAMSIZ - 1));

    fd = socket(PF_PACKET, SOCK_RAW, hton16(ETHER_TYPE_BRCM));
    if (fd < 0) {
        printf("Cannot create socket %d\n", fd);
        err = -1;
        goto exit2;
    }

    err = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (err < 0) {
        printf("Cannot get index %d\n", err);
        goto exit2;
    }

    /* bind the socket first before starting escan so we won't miss any event */
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = hton16(ETHER_TYPE_BRCM);
    sll.sll_ifindex = ifr.ifr_ifindex;
    err = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    if (err < 0) {
        printf("Cannot bind %d\n", err);
        goto exit2;
    }

    data = (char*)malloc(ESCAN_EVENTS_BUFFER_SIZE);

    if (data == NULL) {
        printf("Cannot not allocate %d bytes for events receive buffer\n",
            ESCAN_EVENTS_BUFFER_SIZE);
        err = -1;
        goto exit2;
    }

    params_size += WL_SCAN_PARAMS_SSID_MAX * sizeof(wlc_ssid_t);
    params = (wl_escan_params_t*)malloc(params_size);
    if (params == NULL) {
        fprintf(stderr, "Error allocating %d bytes for scan params\n", params_size);
        BWL_UNLOCK();
        return -1;
    }
    memset(params, 0, params_size);

    memcpy(&params->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
    params->params.bss_type    = DOT11_BSSTYPE_ANY;
    params->params.scan_type   = 0;
    params->params.nprobes     = htod32(-1);
    params->params.active_time = htod32(pScanParams->lActiveTime);
    params->params.passive_time= htod32(pScanParams->lPassiveTime);
    params->params.home_time   = htod32(pScanParams->lHomeTime);
    params->version     = htod32(ESCAN_REQ_VERSION);
    params->action      = htod16(WL_SCAN_ACTION_START);
    params->params.channel_num = 0;

    if(pScanParams->pcSSID != NULL)
    {
        params->params.ssid.SSID_len = htod32(strlen(pScanParams->pcSSID));
        strncpy((char*)params->params.ssid.SSID, pScanParams->pcSSID, sizeof(params->params.ssid.SSID)/sizeof(params->params.ssid.SSID[0]));
    }

    p = (char*)params->params.channel_list;

    params_size = p - (char*)&params->params;


#if defined(linux)
    srand((unsigned)time(NULL));
    params->sync_id = htod16(rand() & 0xffff);
#else
    params->sync_id = htod16(4321);
#endif /* #if defined(linux) */

    params_size += OFFSETOF(wl_escan_params_t, params);
    err = wlu_iovar_set(wl, "escan", params, params_size);

    if (params)
    {
        free(params);
    }

    /* receive scan result */
    while (1) {

        if (pbStop != NULL)
        {
            fd_set tFDSet;
            struct timeval tm;
            tm.tv_sec  = 0;
            tm.tv_usec = ESCAN_LOOP_TIMEOUT_MS * 1000;
            int result = 0;

            FD_ZERO(&tFDSet);
            FD_SET(fd, &tFDSet);

            result =  select(fd + 1, &tFDSet, NULL, NULL, &tm);

            if (*pbStop)
            {
                status = WLC_E_STATUS_ABORT;
                break;
            }

            /* Check if we timed out */
            if (lTimeoutMs > 0)
            {
                timeCount += ESCAN_LOOP_TIMEOUT_MS;
                if (timeCount >= lTimeoutMs)
                {
                    status = WLC_E_STATUS_TIMEOUT;
                    break;
                }
            }

            if (result <= 0)
            {
                continue;
            }

            if (!FD_ISSET(fd, &tFDSet))
            {
                continue;
            }
        }

        octets = recv(fd, data, ESCAN_EVENTS_BUFFER_SIZE, 0);
    if (octets > 0) { /* Process the event only if a meaningful data has been received.*/
        event = (bcm_event_t *)data;
        event_type = ntoh32(event->event.event_type);

        if (event_type == WLC_E_ESCAN_RESULT) {
            escan_data = (wl_escan_result_t*)&data[sizeof(bcm_event_t)];
            status = ntoh32(event->event.status);
            if (status == WLC_E_STATUS_PARTIAL) {
                wl_bss_info_t *bi = &escan_data->bss_info[0];
                wl_bss_info_t *bss;

                /* check if we've received info of same BSSID */
                for (result = escan_bss_head; result; result = result->next) {
                    bss = result->bss;

#define WLC_BSS_RSSI_ON_CHANNEL 0x0002 /* Copied from wlc.h. Is there a better way to do this? */

                    if(!memcmp(bi->BSSID.octet, bss->BSSID.octet, ETHER_ADDR_LEN)&&
                        CHSPEC_BAND(bi->chanspec) ==
                        CHSPEC_BAND(bss->chanspec) &&
                        bi->SSID_len == bss->SSID_len &&
                        !memcmp(bi->SSID, bss->SSID, bi->SSID_len))
                        break;
                }

                if (!result) {
                    /* New BSS. Allocate memory and save it */
                    struct escan_bss *ebss = malloc(ESCAN_BSS_FIXED_SIZE
                        + dtoh32(bi->length));

                    if (!ebss) {
                        perror("can't allocate memory for bss");
                        goto exit1;
                    }

                    ebss->next = NULL;
                    memcpy(&ebss->bss, bi, dtoh32(bi->length));
                    if (escan_bss_tail) {
                        escan_bss_tail->next = ebss;
                    }
                    else {
                        escan_bss_head = ebss;
                    }
                    escan_bss_tail = ebss;
                }
                else {
                    /* We've got this BSS. Update rssi if necessary */
                    if ((bss->flags & WLC_BSS_RSSI_ON_CHANNEL) ==
                        (bi->flags & WLC_BSS_RSSI_ON_CHANNEL)) {
                        /* preserve max RSSI if the measurements are
                         * both on-channel or both off-channel
                         */
                        bss->RSSI = (dtoh16(bss->RSSI) > dtoh16(bi->RSSI)) ? bss->RSSI : bi->RSSI;
                    } else if ((bss->flags & WLC_BSS_RSSI_ON_CHANNEL) &&
                        (bi->flags & WLC_BSS_RSSI_ON_CHANNEL) == 0) {
                        /* preserve the on-channel rssi measurement
                         * if the new measurement is off channel
                        */
                        bss->RSSI = bi->RSSI;
                        bss->flags |= WLC_BSS_RSSI_ON_CHANNEL;
                    }
                }
            }
            else if (status == WLC_E_STATUS_SUCCESS) {
                /* Escan finished. Let's go dump the results. */
                break;
            }
            else {
                printf("sync_id: %d, status:%d, misc. error/abort\n",
                    dtoh16(escan_data->sync_id), status);
                goto exit1;
            } /*end if status */
        } /*end if WLC_E_ESCAN_RESULT*/
    } /*end if octets > 0*/
    } /*end while*/

    /* print scan results */
    for (result = escan_bss_head; result; result = result->next) {
#ifdef BWL_DEBUG
        dump_bss_info(result->bss);
#endif
        count++;
    }


    if ( (count > 0) && (status != WLC_E_STATUS_ABORT) )
    {
        pScanData = (ScanInfo_t*)malloc(sizeof(ScanInfo_t) * count);
        if (pScanData != NULL)
        {
            int i = 0;
            memset(pScanData, 0, sizeof(ScanInfo_t) * count);
            for (result = escan_bss_head; result; result = result->next, i++) {
                bwl_parse_bss_info(result->bss, &pScanData[i]);
            #ifdef BWL_DEBUG
                    dump_bss_info(result->bss);
            #endif
            }
        }
        *pData = pScanData;
    }
    *pulCount = count;

exit1:
    /* free scan results */
    result = escan_bss_head;
    while (result) {
        struct escan_bss *tmp = result->next;
        free(result);
        result = tmp;
    }

    switch(status)
    {
        case WLC_E_STATUS_SUCCESS:
            err = BWL_ERR_SUCCESS;
            break;
        case WLC_E_STATUS_ABORT:
            err = BWL_ERR_CANCELED;
            break;
        case WLC_E_STATUS_TIMEOUT:
            err = BWL_ERR_TIMEOUT;
            break;
        default:
            err = BWL_ERR_GENERIC;
            break;
    }
    free(data);
    close(fd);
exit2:

BWL_EXIT:
    BWL_UNLOCK();
    return err;
}


/*******************************************************************************
*
*   Name: BWL_SetBridgeMode()
*
*   Purpose:
*       Enable/disable the WET and promisc settings in the driver
*
*******************************************************************************/
int32_t BWL_SetBridgeMode(BWL_Handle hBwl, uint32_t ulValue)
{
    int32_t err = 0;
    void    *wl = hBwl->wl;

    BWL_LOCK();

    if (hBwl == NULL)
    {
        BWL_CHECK_ERR(BWL_ERR_PARAM);
    }

    err = wlu_set(wl, WLC_SET_WET, &ulValue, sizeof(ulValue));
    //BWL_CHECK_ERR( err );

    err = wlu_set(wl, WLC_SET_PROMISC, &ulValue, sizeof(ulValue));
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetAntennaCount()
*
*   Purpose:
*       Fetch the number of antennas in the system
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetAntennaCount(BWL_Handle hBwl, uint32_t *pulAntenna)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(pulAntenna == NULL);

    err = wlu_iovar_get( wl, "antennas", pulAntenna, sizeof( uint32_t ) );
    BWL_CHECK_ERR( err );

    *pulAntenna = htod32(*pulAntenna);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetAntenna()
*
*   Purpose:
*       Select which antenna(s) need to be active for the specified mode (rx or tx)
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_SetAntenna(BWL_Handle hBwl, AntMode_t eAntMode, uint32_t ulAntenna)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(eAntMode > eAntModeTx);

    ulAntenna = htod32(ulAntenna);
    if (eAntModeRx == eAntMode)
    {
        err = wlu_iovar_set( wl, "rxchain", &ulAntenna, sizeof( uint32_t ) );
    }
    else if (eAntModeTx == eAntMode)
    {
        err = wlu_iovar_set( wl, "txchain", &ulAntenna, sizeof( uint32_t ) );
    }
    BWL_CHECK_ERR( err );


BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetAntenna()
*
*   Purpose:
*       Query which antenna(s) are active for the specified mode (rx or tx)
*
*   Returns:
*       BWL_ERR_xxx
*
*   See Also:
*
*******************************************************************************/
int32_t BWL_GetAntenna(BWL_Handle hBwl, AntMode_t eAntMode, uint32_t *pulAntenna)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(pulAntenna == NULL);
    BWL_CHECK_ERR(eAntMode > eAntModeTx);

    if (eAntModeRx == eAntMode)
    {
        err = wlu_iovar_get( wl, "rxchain", pulAntenna, sizeof( uint32_t ) );
    }
    else if (eAntModeTx == eAntMode)
    {
        err = wlu_iovar_get( wl, "txchain", pulAntenna, sizeof( uint32_t ) );
    }
    BWL_CHECK_ERR( err );

    *pulAntenna = htod32(*pulAntenna);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetRssiPerAntenna()
*
*   Purpose:
*       Retrieve RSSI value for each antenna
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetRssiPerAntenna(BWL_Handle  hBwl, RssiPerAntenna_t *pRssiPerAntenna)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    unsigned int i;
    wl_rssi_ant_t rssi;

    BWL_LOCK();

    BWL_CHECK_ERR(pRssiPerAntenna == NULL);

    memset(&rssi, 0, sizeof(wl_rssi_ant_t));

    err = wlu_iovar_get( wl, "phy_rssi_ant", &rssi, sizeof(wl_rssi_ant_t) );
    BWL_CHECK_ERR( err );

    rssi.count = htod32(rssi.count);
    pRssiPerAntenna->ulNumAntennas = rssi.count;
    if (rssi.count > 0)
    {
        for (i=0; i<rssi.count; i++)
        {
            pRssiPerAntenna->lRSSI_ant[i] = rssi.rssi_ant[i];
        }
    }

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Disassoc()
*
*   Purpose:
*       Disassociates from the current BSS or IBSS
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_Disassoc
(
    BWL_Handle  hBwl  /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_set( wl, WLC_DISASSOC, NULL, 0 );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_Out()
*
*   Purpose:
*       Indicates that the interface is down (that is, disabled or non-operational) without resetting the interface
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_Out
(
    BWL_Handle  hBwl  /* [in] BWL Handle */
)
{
    int32_t     err = 0;
    void        *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_set( wl, WLC_OUT, NULL, 0 );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ControlFrequencyAccuracy()
*
*   Purpose:
*       Sets the frequency accuracy mode for manufacturing test purposes
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_ControlFrequencyAccuracy(BWL_Handle  hBwl, int32_t channel)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    channel = htod32(channel);
    err = wlu_set( wl, WLC_FREQ_ACCURACY, &channel, sizeof(int32_t) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ControlPhyWatchdog()
*
*   Purpose:
*       Enables or disables the PHY watchdog
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_ControlPhyWatchdog(BWL_Handle hBwl, bool bEnable)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int val;

    BWL_LOCK();

    val = bEnable ? 1 : 0;
    val = htod32(val);
    err = wlu_iovar_set( wl, "phy_watchdog", &val, sizeof( int) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ControlPhyForceCal()
*
*   Purpose:
*       Enables or disables the PHY watchdog
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_ControlPhyForceCal(BWL_Handle hBwl, bool bEnable)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int val;

    BWL_LOCK();

    val = bEnable ? 1 : 0;
    val = htod32(val);
    err = wlu_iovar_set( wl, "phy_forcecal", &val, sizeof( int) );
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetInterferenceMitigationMode()
*
*   Purpose:
*       Controls the interference mitigation mode
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetInterferenceMitigationMode(BWL_Handle hBwl, int mode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    mode = htod32(mode);
    err = wlu_set(wl, WLC_SET_INTERFERENCE_MODE, &mode, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_ControlScanSuppression()
*
*   Purpose:
*       Controls the scan suppression
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_ControlScanSuppression(BWL_Handle hBwl, int mode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    mode = htod32(mode);
    err = wlu_set(wl, WLC_SET_SCANSUPPRESS, &mode, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetBeaconInterval()
*
*   Purpose:
*       Controls the beacon period
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetBeaconInterval(BWL_Handle hBwl, int interval)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    interval = htod32(interval);
    err = wlu_set(wl, WLC_SET_BCNPRD, &interval, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetMimoPreamble()
*
*   Purpose:
*       Sets the MIMO preamble value
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetMimoPreamble(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    value = htod32(value);
    err = wlu_iovar_set( wl, "mimo_preamble", &value, sizeof( int) );
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetMimoPreamble()
*
*   Purpose:
*       Sets the MIMO preamble value
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetMimoPreamble(BWL_Handle hBwl, int *pValue)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_get( wl, "mimo_preamble", pValue, sizeof(int) );
    BWL_CHECK_ERR(err);
    *pValue = htod32(*pValue );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetFrameBurst()
*
*   Purpose:
*       Controls the frame burst mode
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetFrameBurst(BWL_Handle hBwl, int mode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    mode = htod32(mode);
    err = wlu_set(wl, WLC_SET_FAKEFRAG, &mode, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetMpcMode()
*
*   Purpose:
*       Sets the MPC value
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetMpcMode(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    value = htod32(value);
    err = wlu_iovar_set( wl, "mpc", &value, sizeof( int) );
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetMpcMode()
*
*   Purpose:
*       Gets the MPC value
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetMpcMode(BWL_Handle hBwl, uint32_t *pulMode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(pulMode == NULL);
    err = wlu_iovar_get( wl, "mpc", pulMode, sizeof( uint32_t ) );
    BWL_CHECK_ERR( err );

    *pulMode = htod32(*pulMode);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}



/*******************************************************************************
*
*   Name: BWL_GetTemperature()
*
*   Purpose:
*       Gets the PHY temperature value
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetTemperature(BWL_Handle hBwl, uint32_t *pTemp)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(pTemp == NULL);
    err = wlu_iovar_getint( wl, "phy_tempsense", (int32_t*)pTemp);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetApMode()
*
*   Purpose:
*       Configures for either AP (value=1) or STA (value = 0)
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetApMode(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    value = htod32(value);
    err = wlu_set(wl, WLC_SET_AP, &value, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetAntennaDiversity()
*
*   Purpose:
*       Set antenna diversity for rx
*       Values:
*           0 - force use of antenna 0
*           1 - force use of antenna 1
*           3 - automatic selection of antenna diversity
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetAntennaDiversity(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    value = htod32(value);
    err = wlu_set(wl, WLC_SET_ANTDIV, &value, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetTxAntenna()
*
*   Purpose:
*       Set the transmit antenna
*       Values:
*           0 - force use of antenna 0
*           1 - force use of antenna 1
*           3 - use the RX antenna selection that was in force during the most recently received good PLCP header
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetTxAntenna(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    value = htod32(value);
    err = wlu_set(wl, WLC_SET_TXANT, &value, sizeof(int));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetAmpDU()
*
*   Purpose:
*       Set value for ampdu
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetAmpDU(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_setint(wl, "ampdu", value);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetPhyTxPowerControl()
*
*   Purpose:
*       Set value for phy_txpwrctrl
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetPhyTxPowerControl(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_setint(wl, "phy_txpwrctrl", value);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetPhyScramInit()
*
*   Purpose:
*       Set value for phy_scraminit
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetPhyScramInit(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_setint(wl, "phy_scraminit", value);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_IsP2PIface()
*
*   Purpose:
*
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
bool BWL_IsP2PIface(BWL_Handle hBwl, struct ether_addr *mac)
{
    void                *wl = hBwl->wl;
    bool                result = false;
#ifndef FALCON_WIFI_DRIVER
    wl_p2p_ifq_t        info;
    BWL_LOCK();

    result = ( (wlu_iovar_getbuf(wl, "p2p_if", mac, sizeof(struct ether_addr), &info, sizeof(wl_p2p_ifq_t)) == 0) &&
               (info.bsscfgidx > 0) );

    BWL_UNLOCK();
#endif
    return( result );
}


/*******************************************************************************
*
*   Name: BWL_DeleteP2PIface()
*
*   Purpose:
*
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_DeleteP2PIface(BWL_Handle hBwl, struct ether_addr *mac)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    err = wlu_iovar_set(wl, "p2p_ifdel", mac, sizeof(struct ether_addr));

    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetGpioState()
*
*   Purpose:
*
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetGpioState(BWL_Handle hBwl, uint32_t *pulGpioValue)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_get(wl, "ccgpioin", pulGpioValue, sizeof(uint32_t));

    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_HideSSID()
*
*   Purpose:
*       Hide or un-hide the SSID when in SoftAp mode
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_HideSSID(BWL_Handle hBwl, bool bEnable)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int32_t            lValue = bEnable;
    BWL_LOCK();

    lValue = htod32(lValue);
    err = wlu_set( wl, WLC_SET_CLOSED, &lValue, sizeof(lValue ) );
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetVhtFeatures()
*
*   Purpose:
*       Set the Vht mode
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetVhtFeatures(BWL_Handle hBwl, int value)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int32_t            lValue = value;
    BWL_LOCK();

    lValue = htod32(lValue);
    err = wlu_iovar_setint(wl, "vht_features", lValue);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

int g_wlc_idx = 0;
/*******************************************************************************
*
*   Name: BWL_CreateIface()
*
*   Purpose:
*       Create a new virtual interface
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_IfaceCreate(BWL_Handle hBwl, bool bAp, struct ether_addr *mac, char *pIfaceName, uint32_t ulLength, int index, uint8_t *bsscfgidx)
{
    int32_t             err = 0;
#ifdef DHD_WIFI_DRIVER
    void                *wl = hBwl->wl;

    wl_interface_create_t wlif;
    wl_interface_info_t *pwlif_info;
    void                *pbuf = NULL;
    BWL_LOCK();

    memset(&wlif, 0, sizeof(wlif));
    pbuf  = malloc( WLC_IOCTL_MAXLEN );
    if (pbuf == NULL)
    {
        BWL_CHECK_ERR( err = BWL_ERR_ALLOC );
    }
    memset(pbuf, 0, WLC_IOCTL_MAXLEN);

    if (index > 0) {
        g_wlc_idx = index;
    }
    wlif.ver        = WL_INTERFACE_CREATE_VER;
    wlif.flags      = bAp ? WL_INTERFACE_CREATE_AP : WL_INTERFACE_CREATE_STA;
    wlif.flags      |= WL_INTERFACE_MAC_USE;

    memcpy(&wlif.mac_addr, mac, sizeof(struct ether_addr));


    err = wlu_iovar_getbuf(wl, "interface_create", &wlif, sizeof(wlif), pbuf, WLC_IOCTL_MAXLEN);
    if (err < 0)
    {
        printf("%s(): wlu_var_getbuf failed %d(%s)\r\n", __FUNCTION__, err, strerror(err));
    } else {
        pwlif_info = (wl_interface_info_t *)pbuf;
        printf("%s(): ifname: %s bsscfgidx: %d mac_addr %s\r\n",
                    __FUNCTION__, pwlif_info->ifname, pwlif_info->bsscfgidx,
                    wl_ether_etoa(&pwlif_info->mac_addr));
        strncpy(pIfaceName, pwlif_info->ifname, ulLength);
        *bsscfgidx = pwlif_info->bsscfgidx;
    }

    g_wlc_idx = 0;

BWL_EXIT:
    if (pbuf)
        free(pbuf);
    BWL_UNLOCK();
#else
    (void)hBwl;
    (void)bAp;
    (void)mac;
    (void)pIfaceName;
    (void)ulLength;
    (void)index;
    (void)bsscfgidx;
#endif
    return( err );
}
/*******************************************************************************
*
*   Name: BWL_DeleteIface()
*
*   Purpose:
*       Delete an interface
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_IfaceDelete(BWL_Handle hBwl)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_set(wl, "interface_remove", NULL, 0);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetChanspecStr()
*
*   Purpose:
*       Set the channel spec as a string
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetChanspecStr(BWL_Handle hBwl, char* pChanspec, uint32_t ulLength)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    chanspec_t          chanspec;
    uint32_t            val;
    (void)ulLength;
    BWL_LOCK();

    chanspec = wf_chspec_aton(pChanspec);
    val = htod32((uint32)chanspec);
    err = wlu_iovar_set(wl, "chanspec", &val, sizeof(val));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_SetBss()
*
*   Purpose:
*       Set Bss configurations
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetBss(BWL_Handle hBwl, BssMode_t eMode, int8_t index)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    struct {
        int cfg;
        int val;
    } bss_setbuf = {0,0};

    BWL_LOCK();

    switch (eMode)
    {
    case eBssModeUp:
        bss_setbuf.val = 1;
        break;
    case eBssModeDown:
        bss_setbuf.val = 0;
        break;
    case eBssModeAp:
        bss_setbuf.val = 3;
        break;
    case eBssModeSta:
        bss_setbuf.val = 2;
        break;
    default:
        BWL_CHECK_ERR(-1);
        break;
    }


    bss_setbuf.cfg = htod32(index);
    bss_setbuf.val = htod32(bss_setbuf.val);

    err =  wlu_iovar_set(wl, "bss", &bss_setbuf, sizeof(bss_setbuf));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetRsdbMode()
*
*   Purpose:
*       Set RSDB Mode 1==RSDB, 0==MIMO, -1==Auto
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetRsdbMode(BWL_Handle hBwl, int32_t lMode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    wl_config_t cfg;
    cfg.config = htod32(lMode);

    BWL_LOCK();

    err =  wlu_iovar_set(wl, "rsdb_mode", &cfg, sizeof(cfg));
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetVhtFeatures()
*
*   Purpose:
*       Get the Vht mode
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetVhtFeatures(BWL_Handle hBwl, uint32_t *pulVhtFeatures)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_getint(wl, "vht_features", (int32_t*)pulVhtFeatures);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_GetApSta()
*
*   Purpose:
*       Get the state of the apsta level
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetApSta(BWL_Handle hBwl, uint32_t *pulApSta)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_getint(wl, "apsta", (int32_t*)pulApSta);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

/*******************************************************************************
*
*   Name: BWL_SetApSta()
*
*   Purpose:
*       Get the state of the apsta level
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_SetApSta(BWL_Handle hBwl, uint32_t ulApSta)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_setint(wl, "apsta", (int32_t)ulApSta);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_SetMchanAlgo(BWL_Handle hBwl, uint32_t ulMchanAlgo)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_setint(wl, "mchan_algo", (int32_t)ulMchanAlgo);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_SetMchanBw(BWL_Handle hBwl, uint32_t ulBw)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_setint(wl, "mchan_bw", (int32_t)ulBw);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_SetMchanSchedMode(BWL_Handle hBwl, uint32_t ulMode)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    BWL_LOCK();

    err = wlu_iovar_setint(wl, "mchan_sched_mode", (int32_t)ulMode);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


int32_t BWL_ResetCounter(BWL_Handle hBwl)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;
    int val;
    BWL_LOCK();

    err = wlu_iovar_getint(wl, "reset_cnts", &val);
    BWL_CHECK_ERR(err);

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}


/*******************************************************************************
*
*   Name: BWL_GetNRate()
*
*   Purpose:
*       Gets the PHY nrate
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int32_t BWL_GetNRate(BWL_Handle hBwl, uint32_t *pNRate)
{
    int32_t             err = 0;
    void                *wl = hBwl->wl;

    BWL_LOCK();

    BWL_CHECK_ERR(pNRate == NULL);
    err = wlu_iovar_getint( wl, "nrate", (int32_t*)pNRate);
    BWL_CHECK_ERR( err );

BWL_EXIT:
    BWL_UNLOCK();
    return( err );
}

static struct escan_bss  *g_escan_bss_head = NULL;
int wl_escanresults( void *wl, const char *cmd, char **argv ); /* forward prototype */
cmd_t wl_cmds[] = {
    { "escanresults", (cmd_func_t *) wl_escanresults, -1, WLC_SET_VAR, "Start escan and display results.\n" SCAN_USAGE }
};
/* CAD 2016-05-26 - copied from wlu_common.c */
/*
 * format an iovar buffer
 * iovar name is converted to lower case
 */
static uint
wl_iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf, uint buflen, int *perr)
{
    uint iovar_len;
    char *p;

    iovar_len = strlen(name) + 1;

    /* check for overflow */
    if ((iovar_len + datalen) > buflen) {
        *perr = BCME_BUFTOOSHORT;
        return 0;
    }

    /* copy data to the buffer past the end of the iovar name string */
    if (datalen > 0)
        memmove(&iovar_buf[iovar_len], data, datalen);

    /* copy the name to the beginning of the buffer */
    strcpy(iovar_buf, name);

    /* wl command line automatically converts iovar names to lower case for
     * ease of use
     */
    p = iovar_buf;
    while (*p != '\0') {
        *p = tolower((int)*p);
        p++;
    }

    *perr = 0;
    return (iovar_len + datalen);
}

/*
 * set named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
static int
wlu_iovar_setbuf(void* wl, const char *iovar,
    void *param, int paramlen, void *bufptr, int buflen)
{
    int err;
    int iolen;

    iolen = wl_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
    if (err)
        return err;

    return wlu_set(wl, WLC_SET_VAR, bufptr, iolen);
}

/* CAD copied from wlu.c 2016-06-22 */
int wlu_var_setbuf( void *wl, const char *iovar, void *param, int param_len)
{
    int len;

    memset( buf, 0, WLC_IOCTL_MAXLEN );
    strcpy( buf, iovar );

    /* include the null */
    len = strlen( iovar ) + 1;

    if (param_len)
    {
        memcpy( &buf[len], param, param_len );
    }

    len += param_len;

    return( wlu_set( wl, WLC_SET_VAR, &buf[0], len ));
} /* wlu_var_setbuf */

/* CAD copied from wlu.c 2016-05-26 */
int
wl_parse_chanspec_list(char *list_str, chanspec_t *chanspec_list, int chanspec_num)
{
    int num = 0;
    chanspec_t chanspec;
    char *next, str[8];
    size_t len;

    if ((next = list_str) == NULL)
        return BCME_ERROR;

    while ((len = strcspn(next, " ,")) > 0) {
        if (len >= sizeof(str)) {
            fprintf(stderr, "string \"%s\" before ',' or ' ' is too long\n", next);
            return BCME_ERROR;
        }
        strncpy(str, next, len);
        str[len] = 0;
        chanspec = wf_chspec_aton(str);
        if (chanspec == 0) {
            fprintf(stderr, "could not parse chanspec starting at "
                    "\"%s\" in list:\n%s\n", str, list_str);
            return BCME_ERROR;
        }
        if (num == chanspec_num) {
            fprintf(stderr, "too many chanspecs (more than %d) in chanspec list:\n%s\n",
                chanspec_num, list_str);
            return BCME_ERROR;
        }
        chanspec_list[num++] = chanspec;
        next += len;
        next += strspn(next, " ,");
    }

    return num;
}

/* CAD copied from wlu.c 2016-05-26 */
int
wl_scan_prep(void *wl, cmd_t *cmd, char **argv, wl_scan_params_t *params, int *params_size)
{
    #if 0
    int val = 0;
    char key[64];
    int keylen;
    char *eq, *valstr, *endptr = NULL;
    char opt;
    bool positional_param;
    bool good_int;
    bool opt_end;
    #endif
    char *p = NULL;
    int err = 0;
    int i;

    int nchan = 0;
    int nssid = 0;
    wlc_ssid_t ssids[WL_SCAN_PARAMS_SSID_MAX];

    UNUSED_PARAMETER(wl);
    UNUSED_PARAMETER(cmd);

    memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
    params->bss_type = DOT11_BSSTYPE_ANY;
    params->scan_type = 0;
    params->nprobes = -1;
    params->active_time = -1;
    params->passive_time = -1;
    params->home_time = -1;
    params->channel_num = 0;
    memset(ssids, 0, WL_SCAN_PARAMS_SSID_MAX * sizeof(wlc_ssid_t));

    /* skip the command name */
    argv++;

#if 0
    opt_end = FALSE;
    while ((p = *argv) != NULL) {
        argv++;
        positional_param = FALSE;
        memset(key, 0, sizeof(key));
        opt = '\0';
        valstr = NULL;
        good_int = FALSE;

        if (opt_end) {
            positional_param = TRUE;
            valstr = p;
        }
        else if (!strcmp(p, "--")) {
            opt_end = TRUE;
            continue;
        }
        else if (!strncmp(p, "--", 2)) {
            eq = strchr(p, '=');
            if (eq == NULL) {
                fprintf(stderr,
                "wl_scan: missing \" = \" in long param \"%s\"\n", p);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
            keylen = eq - (p + 2);
            if (keylen > 63)
                keylen = 63;
            memcpy(key, p + 2, keylen);

            valstr = eq + 1;
            if (*valstr == '\0') {
                fprintf(stderr,
                "wl_scan: missing value after \" = \" in long param \"%s\"\n", p);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
        }
        else if (!strncmp(p, "-", 1)) {
            opt = p[1];
            if (strlen(p) > 2) {
                fprintf(stderr,
                "wl_scan: only single char options, error on param \"%s\"\n", p);
                err = BCME_BADARG;
                goto exit;
            }
            if (*argv == NULL) {
                fprintf(stderr,
                "wl_scan: missing value parameter after \"%s\"\n", p);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
            valstr = *argv;
            argv++;
        } else {
            positional_param = TRUE;
            valstr = p;
        }

        /* parse valstr as int just in case */
        val = (int)strtol(valstr, &endptr, 0);
        if (*endptr == '\0') {
            /* not all the value string was parsed by strtol */
            good_int = TRUE;
        }

        if (opt == 's' || !strcmp(key, "ssid") || positional_param) {
            nssid = wl_parse_ssid_list(valstr, ssids, nssid, WL_SCAN_PARAMS_SSID_MAX);
            if (nssid < 0) {
                err = BCME_BADARG;
                goto exit;
            }
        }

        /* scan_type is a bitmap value and can have multiple options */
        if (opt == 't' || !strcmp(key, "scan_type")) {
            if (!strcmp(valstr, "active")) {
                /* do nothing - scan_type is initialized outside of while loop */
            } else if (!strcmp(valstr, "passive")) {
                params->scan_type |= WL_SCANFLAGS_PASSIVE;
            } else if (!strcmp(valstr, "prohibit")) {
                params->scan_type |= WL_SCANFLAGS_PROHIBITED;
            } else if (!strcmp(valstr, "offchan")) {
                params->scan_type |= WL_SCANFLAGS_OFFCHAN;
            } else if (!strcmp(valstr, "hotspot")) {
                params->scan_type |= WL_SCANFLAGS_HOTSPOT;
            } else {
                fprintf(stderr,
                "scan_type value should be \"active\", "
                "\"passive\", \"prohibit\", \"offchan\" "
                "or \"hotspot\", but got \"%s\"\n", valstr);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
        }
        if (!strcmp(key, "bss_type")) {
            if (!strcmp(valstr, "bss") || !strcmp(valstr, "infra")) {
                params->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
            } else if (!strcmp(valstr, "ibss") || !strcmp(valstr, "adhoc")) {
                params->bss_type = DOT11_BSSTYPE_INDEPENDENT;
            } else if (!strcmp(valstr, "any")) {
                params->bss_type = DOT11_BSSTYPE_ANY;
            } else {
                fprintf(stderr,
                "bss_type value should be "
                "\"bss\", \"ibss\", or \"any\", but got \"%s\"\n", valstr);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
        }
        if (opt == 'b' || !strcmp(key, "bssid")) {
            if (!wl_ether_atoe(valstr, &params->bssid)) {
                fprintf(stderr,
                "could not parse \"%s\" as an ethernet MAC address\n", valstr);
                err = BCME_USAGE_ERROR;
                goto exit;
            }
        }
        if (opt == 'n' || !strcmp(key, "nprobes")) {
            if (!good_int) {
                fprintf(stderr,
                "could not parse \"%s\" as an int for value nprobes\n", valstr);
                err = BCME_BADARG;
                goto exit;
            }
            params->nprobes = val;
        }
        if (opt == 'a' || !strcmp(key, "active")) {
            if (!good_int) {
                fprintf(stderr,
                "could not parse \"%s\" as an int for active dwell time\n",
                    valstr);
                err = BCME_BADARG;
                goto exit;
            }
            params->active_time = val;
        }
        if (opt == 'p' || !strcmp(key, "passive")) {
            if (!good_int) {
                fprintf(stderr,
                "could not parse \"%s\" as an int for passive dwell time\n",
                    valstr);
                err = BCME_BADARG;
                goto exit;
            }
            params->passive_time = val;
        }
        if (opt == 'h' || !strcmp(key, "home")) {
            if (!good_int) {
                fprintf(stderr,
                "could not parse \"%s\" as an int for home channel dwell time\n",
                    valstr);
                err = BCME_BADARG;
                goto exit;
            }
            params->home_time = val;
        }
        if (opt == 'c' || !strcmp(key, "chanspecs")) {
            nchan = wl_parse_chanspec_list(valstr, params->channel_list,
                                          WL_NUMCHANNELS);
            if (nchan == -1) {
                fprintf(stderr, "error parsing chanspec list arg\n");
                err = BCME_BADARG;
                goto exit;
            }
        }
    }
#endif

    if (nssid > WL_SCAN_PARAMS_SSID_MAX) {
        fprintf(stderr, "ssid count %d exceeds max of %d\n",
                nssid, WL_SCAN_PARAMS_SSID_MAX);
        err = BCME_BADARG;
        goto exit;
    }

    params->nprobes = htod32(params->nprobes);
    params->active_time = htod32(params->active_time);
    params->passive_time = htod32(params->passive_time);
    params->home_time = htod32(params->home_time);

    for (i = 0; i < nchan; i++) {
        params->channel_list[i] = htodchanspec(params->channel_list[i]);
    }

    for (i = 0; i < nssid; i++) {
        ssids[i].SSID_len = htod32(ssids[i].SSID_len);
    }

    /* For a single ssid, use the single fixed field */
    if (nssid == 1) {
        nssid = 0;
        memcpy(&params->ssid, &ssids[0], sizeof(ssids[0]));
    }

    /* Copy ssid array if applicable */
    if (nssid > 0) {
        i = OFFSETOF(wl_scan_params_t, channel_list) + nchan * sizeof(uint16);
        i = ROUNDUP(i, sizeof(uint32));
        if (i + nssid * sizeof(wlc_ssid_t) > (uint)*params_size) {
            fprintf(stderr, "additional ssids exceed params_size\n");
            err = BCME_BADARG;
            goto exit;
        }

        p = (char*)params + i;
        memcpy(p, ssids, nssid * sizeof(wlc_ssid_t));
        p += nssid * sizeof(wlc_ssid_t);
    } else {
        p = (char*)params->channel_list + nchan * sizeof(uint16);
    }

    params->channel_num = htod32((nssid << WL_SCAN_PARAMS_NSSID_SHIFT) |
                                 (nchan & WL_SCAN_PARAMS_COUNT_MASK));
    *params_size = p - (char*)params + nssid * sizeof(wlc_ssid_t);
exit:
    return err;
}

/*
   This function is used for debugging purposes. It prints out the current state of the scanned APs linked list.
*/
int32_t BWL_escanresults_print_linkedlist()
{
    struct escan_bss  *result = NULL;

    printf( "%s:%u \n", __FILE__, __LINE__ );
    for (result = g_escan_bss_head; result; result = result->next) {
        printf( "\tresult (%p) ... result->bss (%p) \n", (void*) result, (void*) result->bss );
    }

    return(0);
} /* wl_escanresults_print_linkedlist */

int32_t BWL_EScanResultsWlExe(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int32_t  err = 0;
    char * argv[2];

    if (hBwl == NULL)
    {
        printf("%s:%u: hBwl cannot be NULL \n", __FILE__, __LINE__ );
        return BWL_ERR_USAGE;
    }

    argv[1] = malloc(128);
    memset(argv[1], 0, 128);

    err = wl_escanresults(hBwl->wl, (char*) &wl_cmds[0], argv );

    return ( err );
}

/* return the SSID of the entry */
const char * BWL_escanresults_get_ssid( unsigned char *buffer )
{
    wl_bss_info_t *bss = (wl_bss_info_t *) buffer;
    if (bss)
    {
        return ( (const char *) bss[0].SSID );
    }

    return( "SSID UNKNOWN" );
}

static int fd;
static fpos_t pos;
static void switchStdout(const char *newStream)
{
  fflush(stdout);
  fgetpos(stdout, &pos);
  fd = dup(fileno(stdout));
  if ( freopen(newStream, "w", stdout) == NULL ) fprintf( stderr, "Could not freopen(stdout)\n" );
}

static void revertStdout()
{
  fflush(stdout);
  dup2(fd, fileno(stdout));
  close(fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);
}

/* print just one bss entry */
int32_t BWL_escanresults_print1(unsigned char *buffer, char * printBuffer, int printBufferLen, const char *redirectFilename )
{
    wl_bss_info_t *bss = (wl_bss_info_t *) buffer;
    if (bss)
    {
        char *tempBuffer = NULL;

        /*printf("%s:%u: escan_bss (%p) \n", __FILE__, __LINE__, bss );*/

        /* redirect STDOUT to a temporary file */
        switchStdout( redirectFilename );

        /* dump bss info to new temporary file */
        dump_bss_info( bss );

        /* revert STDOUT to original file stream */
        revertStdout();

        /* get the contents of the temporary file */
        tempBuffer = GetFileContents( redirectFilename );
        if (tempBuffer)
        {
            /* copy the contents of temporary file to user's buffer */
            memcpy( printBuffer, tempBuffer, printBufferLen );
            free(tempBuffer);
        }
    }

    return(0);
}

int32_t BWL_escanresults_count( void )
{
    int                copy_count   = 0;
    struct escan_bss  *result       = NULL;

    /* walk through the linked list and count how many entries are in it */
    for (result = g_escan_bss_head; result; result = result->next)
    {
        copy_count++;
    }

    return( copy_count );
}

/*
 * If the line length exceeds the max_line_length, count the length as two or three lines.
*/
int32_t BWL_count_lines( const char * buffer, int max_line_length )
{
    int   line_count   = 0;
    const char *temp = buffer;
    const char *pos  = NULL;

    do
    {
        pos = strchr(temp, '\n' );
        line_count++;
        if (pos)
        {
            int line_length = pos - temp;
            int num_lines = (line_length + max_line_length - 1) / max_line_length;

            if (num_lines > 1)
            {
                line_count += (num_lines - 1 );
            }
            pos++;
        }
        temp = pos;
    } while (temp);
    return( line_count );
}

int32_t BWL_escanresults_copy( unsigned char *dest, unsigned int start_index, unsigned int max_copy )
{
    unsigned int       idx          = 0;
    int                copy_count   = 0;
    struct escan_bss  *result       = NULL;

    /*printf("%s:%u start %u; max %u; size of each bss %u; \n", __FILE__, __LINE__, start_index, max_copy, sizeof(result->bss) );*/
    for (result = g_escan_bss_head, idx=0; result; result = result->next,idx++)
    {
        /*printf("%s:%u idx %d ... bss (%p) ... bss->bss (%p) \n", __FILE__, __LINE__, idx, (void*) result, (void*) result->bss );*/
        if ( (start_index <= idx) && (idx < (start_index+max_copy) ) )
        {
            /*printf("%s:%u copying idx %u; bss->bss (%p) ... bytes %d \n", __FILE__, __LINE__, idx, (void*) result->bss, sizeof(result->bss) );*/
            memcpy(dest, (unsigned char*) result->bss, sizeof(result->bss) );
            /*wl_escanresults_print1(dest);*/
            /*printf("%s:%u copied  idx %u; from (%p) to pResponse (%p) done \n", __FILE__, __LINE__, idx, (void*) result->bss, (void*) dest );*/
            dest += sizeof( result->bss );
            copy_count++;
        }
    }

    return( copy_count );
}

/* removed from the end of wl_escanresults() */
int32_t BWL_escanresults_free()
{
    struct escan_bss  *result = NULL;

    /* free scan results */
    result = g_escan_bss_head;
    while (result) {
        struct escan_bss *tmp = result->next;
        /*printf("%s:%u: free(%p) \n", __FUNCTION__, __LINE__, (void*) result );*/
        free( result );
        result = tmp;
    }

    g_escan_bss_head = NULL;

    return(0);
}

/* removed from the end of wl_escanresults() */
int32_t BWL_escanresults_print()
{
    unsigned int bssCount = 0;
    struct escan_bss  *result = NULL;

    /* print scan results */
    for (result = g_escan_bss_head; result; result = result->next) {
        printf( "%s:%u \n\ncount (%d) ... bss (%p) each %d ... bss->bss (%p) each %d \n", __FILE__, __LINE__, bssCount,
           (void*) result, (int) sizeof(*result), (void*) result->bss, (int) sizeof(result->bss) );
        bssCount++;
        dump_bss_info( &result->bss[0] );
    }
    printf( "%s scan results ... bssCount (%d) ... total bytes (%d) \n", __FUNCTION__, bssCount, (int) ((int)bssCount* (int)sizeof(result->bss)) );

    return(0);
}

/* CAD copied from wlu.c 2016-05-26 */
/*******************************************************************************
*
*   Name: wl_escanresults()
*
*   Purpose:
*       Tells the Wifi driver to begin scanning for known access points (bss)
*       and piece by piece, reads the results from the driver using a raw
*       socket.
*
*   Returns:
*       BWL_ERR_xxx
*
*******************************************************************************/
int wl_escanresults(
    void       *wl,
    const char *cmd,
    char      **argv
    )
{
    int params_size = ( WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF( wl_escan_params_t, params )) +
        ( WL_NUMCHANNELS * sizeof( uint16 ));
    wl_escan_params_t *params;
    int                fd, err, octets;
    int                err2 = BCME_OK;
    struct sockaddr_ll sll;
    struct ifreq       ifr;
    bcm_event_t       *event;
    uint32             status;
    char              *data;
    int                event_type;
    uint8              event_inds_mask[WL_EVENTING_MASK_LEN]; /* event bit mask */
    bool               revert_event_bit = FALSE;
    wl_escan_result_t *escan_data;
    struct escan_bss  *escan_bss_tail = NULL;
    struct escan_bss  *result;

    fd_set         rfds;
    struct timeval tv;
    int            retval;

    /* if we are re-scanning after having done so earlier, free up the earlier buffers. */
    BWL_escanresults_free();

    params_size += WL_SCAN_PARAMS_SSID_MAX * sizeof( wlc_ssid_t );
    params       = (wl_escan_params_t *)malloc( params_size );

    if (params == NULL)
    {
        fprintf( stderr, "Error allocating %d bytes for scan params\n", params_size );
        return( BCME_NOMEM );
    }
    memset( params, 0, params_size );

    err = wl_scan_prep( wl, (cmd_t*) cmd, argv, &params->params, &params_size );
    if (err)
    {
        goto exit2;
    }

    memset( &ifr, 0, sizeof( ifr ));
    strncpy( ifr.ifr_name, ((struct ifreq *)wl )->ifr_name, ( IFNAMSIZ - 1 ));

    memset( event_inds_mask, '\0', WL_EVENTING_MASK_LEN );

    /* Read the event mask from driver and unmask the event WLC_E_ESCAN_RESULT */
    if (( err = wlu_iovar_get( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN )))
    {
        goto exit2;
    }

    if (isclr( event_inds_mask, WLC_E_ESCAN_RESULT ))
    {
        setbit( event_inds_mask, WLC_E_ESCAN_RESULT );
        if (( err = wlu_iovar_set( wl, "event_msgs",
                      &event_inds_mask, WL_EVENTING_MASK_LEN )))
        {
            goto exit2;
        }
        revert_event_bit = TRUE;
    }

    fd = socket( PF_PACKET, SOCK_RAW, hton16( ETHER_TYPE_BRCM ));
    if (fd < 0)
    {
        printf( "Cannot create socket %d\n", fd );
        err = -1;
        goto exit2;
    }

    FD_ZERO( &rfds );
    FD_SET( fd, &rfds );

    err = ioctl( fd, SIOCGIFINDEX, &ifr );
    if (err < 0)
    {
        printf( "Cannot get index %d\n", err );
        goto exit2fd;
    }

    /* bind the socket first before starting escan so we won't miss any event */
    memset( &sll, 0, sizeof( sll ));
    sll.sll_family   = AF_PACKET;
    sll.sll_protocol = hton16( ETHER_TYPE_BRCM );
    sll.sll_ifindex  = ifr.ifr_ifindex;

    err = bind( fd, (struct sockaddr *)&sll, sizeof( sll ));
    if (err < 0)
    {
        printf( "Cannot bind %d\n", err );
        goto exit2fd;
    }

    params->version = htod32( ESCAN_REQ_VERSION );
    params->action  = htod16( WL_SCAN_ACTION_START );

    srand((unsigned)time( NULL ));
    params->sync_id = htod16( rand() & 0xffff );

    params_size += OFFSETOF( wl_escan_params_t, params );
    err = wlu_iovar_setbuf( wl, "escan", params, params_size, buf, WLC_IOCTL_MAXLEN );
    if (err != 0)
    {
        goto exit2fd;
    }

    data = (char *)malloc( ESCAN_EVENTS_BUFFER_SIZE );

    if (data == NULL)
    {
        printf( "Cannot not allocate %d bytes for events receive buffer\n",
            ESCAN_EVENTS_BUFFER_SIZE );
        err = BCME_NOMEM;
        goto exit2fd;
    }

    /* Set scan timeout */
    tv.tv_sec  = WL_EVENT_TIMEOUT;
    tv.tv_usec = 0;

    /* receive scan result */
    while (( retval = select( fd+1, &rfds, NULL, NULL, &tv )) > 0) {
        octets = recv( fd, data, ESCAN_EVENTS_BUFFER_SIZE, 0 );
        event      = (bcm_event_t *)data;
        event_type = ntoh32( event->event.event_type );

        if (( event_type == WLC_E_ESCAN_RESULT ) && ( octets > 0 ))
        {
            escan_data = (wl_escan_result_t *)&data[sizeof( bcm_event_t )];
            status     = ntoh32( event->event.status );

            if (status == WLC_E_STATUS_PARTIAL)
            {
                wl_bss_info_t *bi = &escan_data->bss_info[0];
                wl_bss_info_t *bss;

                /* check if we've received info of same BSSID */
                for (result = g_escan_bss_head; result; result = result->next) {
                    bss = result->bss;

                    if (memcmp( bi->BSSID.octet, bss->BSSID.octet, ETHER_ADDR_LEN ) ||
                        ( CHSPEC_BAND( bi->chanspec ) != CHSPEC_BAND( bss->chanspec )) ||
                        ( bi->SSID_len != bss->SSID_len ) ||
                        memcmp( bi->SSID, bss->SSID, bi->SSID_len ))
                    {
                        continue;
                    }

                    /* We've already got this BSS. Update RSSI if necessary */
                    /* Prefer valid RSSI */
                    if (bi->RSSI == WLC_RSSI_INVALID)
                    {
                        break;
                    }
                    else if (bss->RSSI == WLC_RSSI_INVALID)
                    {
                        goto escan_update;
                    }

                    /* Prefer on-channel RSSI */
                    if (!( bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL ) &&
                        ( bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL ))
                    {
                        break;
                    }
                    else if (( bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL ) &&
                             !( bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL ))
                    {
                        goto escan_update;
                    }

                    /* Prefer probe response RSSI */
                    if (( bi->flags & WL_BSS_FLAGS_FROM_BEACON ) &&
                        !( bss->flags & WL_BSS_FLAGS_FROM_BEACON ))
                    {
                        break;
                    }
                    else if (!( bi->flags & WL_BSS_FLAGS_FROM_BEACON ) &&
                             ( bss->flags & WL_BSS_FLAGS_FROM_BEACON ))
                    {
                        goto escan_update;
                    }

                    /* Prefer better RSSI */
                    if (bi->RSSI <= bss->RSSI)
                    {
                        break;
                    }

escan_update:                                              /* Update known entry */
                    bss->RSSI      = bi->RSSI;
                    bss->SNR       = bi->SNR;
                    bss->phy_noise = bi->phy_noise;
                    bss->flags     = bi->flags;
                    break;
                }

                if (!result)
                {
                    /* New BSS. Allocate memory and save it */
                    struct escan_bss *ebss = malloc(
                            OFFSETOF( struct escan_bss, bss ) + bi->length );

                    if (!ebss)
                    {
                        perror( "can't allocate memory for bss" );
                        goto exit1;
                    }

                    ebss->next = NULL;
                    memcpy( &ebss->bss, bi, bi->length );
                    if (escan_bss_tail)
                    {
                        escan_bss_tail->next = ebss;
                    }
                    else
                    {
                        g_escan_bss_head = ebss;
                    }
                    escan_bss_tail = ebss;
                }
            }
            else if (status == WLC_E_STATUS_SUCCESS)
            {
                /* Escan finished. Let's go dump the results. */
                break;
            }
            else
            {
                printf( "sync_id: %d, status:%d, misc. error/abort\n",
                    escan_data->sync_id, status );
                goto exit1;
            }
        }   /* if WLC_E_ESCAN_RESULT */
    }   /* end while select() */

    if (retval > 0)
    {
        #if 0
        /* CAD DEBUG ... add some extra access points to the linked list to test the logic when the list exceeds the bsysperf_server message size */
        {
            int idx = 0;

            /* add 8 temporary APs to force the number to be greater than the number the server can send to the client in one second */
            for (idx=0; idx<8; idx++)
            {
                struct escan_bss *ebss = malloc( OFFSETOF( struct escan_bss, bss ) + sizeof(ebss->bss) );
                ebss->next = NULL;
                memset( &ebss->bss, 0, sizeof(ebss->bss) );
                sprintf( (char*) ebss->bss[0].SSID, (char*) "TEST_AP_%02d", idx+1 );
                ebss->bss[0].SSID_len = strlen((char*) ebss->bss[0].SSID);
                if (escan_bss_tail)
                {
                    escan_bss_tail->next = ebss;
                }
                else
                {
                    g_escan_bss_head = ebss;
                }
                escan_bss_tail = ebss;
            }
        }
        #endif
    }
    else if (retval == 0)
    {
        printf( " Scan timeout! \n" );
    }
    else
    {
        printf( " Receive scan results failed!\n" );
    }

exit1:
    /*BWl_escanresults_free();*/

    free( data );
exit2fd:
    close( fd );
exit2:
    free( params );

    /* Revert the event bit if appropriate */
    if (revert_event_bit &&
        !( err2 = wlu_iovar_get( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN )))
    {
        clrbit( event_inds_mask, WLC_E_ESCAN_RESULT );
        err2 = wlu_iovar_set( wl, "event_msgs", &event_inds_mask, WL_EVENTING_MASK_LEN );
    }

    if (err2)
    {
        fprintf( stderr, "Failed to revert event mask, error %d\n", err2 );
    }
    return( err ? err : err2 );
} /* wl_escanresults */

#define BWL_DUMP_BUF_LEN 4096
const char * BWL_GetAmpdu(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int   ret      = 0;
    void *wl       = hBwl->wl;
    char *dump_buf = NULL;

    dump_buf = malloc(WL_DUMP_BUF_LEN);
    if (dump_buf == NULL)
    {
        fprintf(stderr, "Failed to allocate dump buffer of %d bytes\n", BWL_DUMP_BUF_LEN);
        return NULL;
    }
    memset(dump_buf, 0, BWL_DUMP_BUF_LEN);
    strcpy(dump_buf, "ampdu ");

    ret = wlu_iovar_getbuf(wl, "dump", dump_buf, strlen(dump_buf), dump_buf, BWL_DUMP_BUF_LEN);
    if (ret >= BCME_OK) {
        ret = BCME_OK;
    }

    return ( dump_buf );
}

int BWL_ClearAmpdu(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int   ret      = 0;
    void *wl       = hBwl->wl;

    ret = wlu_iovar_setbuf(wl, "dump_clear", "ampdu", strlen("ampdu"), buf, WLC_IOCTL_MAXLEN);
    if (ret >= BCME_OK) {
        ret = BCME_OK;
    }

    return ( 0 );
}

const unsigned char * BWL_GetPhyRssiAnt(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int   ret      = 0;
    void *wl       = hBwl->wl;
    char *dump_buf = NULL;

    dump_buf = malloc(WL_DUMP_BUF_LEN);
    if (dump_buf == NULL)
    {
        fprintf(stderr, "Failed to allocate dump buffer of %d bytes\n", BWL_DUMP_BUF_LEN);
        return NULL;
    }
    memset(dump_buf, 0, BWL_DUMP_BUF_LEN);
    strcpy((char*) dump_buf, "phy_rssi_ant ");

    ret = wlu_iovar_getbuf(wl, "phy_rssi_ant", dump_buf, strlen(dump_buf), dump_buf, BWL_DUMP_BUF_LEN);
    if (ret >= BCME_OK) {
        ret = BCME_OK;
    }

    return ( (unsigned char*) dump_buf );
}
char * BWL_GetDriverVersion(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int   ret      = 0;
    void *wl       = hBwl->wl;
    char *dump_buf = NULL;
    char *p        = NULL;

    dump_buf = malloc(WLC_IOCTL_SMLEN);
    if (dump_buf == NULL)
    {
        fprintf(stderr, "Failed to allocate dump buffer of %d bytes\n", WLC_IOCTL_SMLEN);
        return NULL;
    }
    memset(dump_buf, 0, WLC_IOCTL_SMLEN);

    /* query for 'ver' to get version info */
    ret = wlu_iovar_get(wl, "ver", dump_buf, WLC_IOCTL_SMLEN);

    if (ret) {
        fprintf(stderr, "Error %d on query of driver dump\n", (int)ret);
        free(dump_buf);
        return NULL;
    }

    /* keep only the first line from the dump buf output */
    p = strchr(dump_buf, '\n');
    if (p)
    {
        *p = '\0';
    }
    return ( dump_buf );
}

/* copied from BSEAV/connectivity/wlan/STB7271_BRANCH_15_10/linux-external-stbsoc/src/wl/exe/wlu_subcounters.c */
int BWL_ClearCounters(
    BWL_Handle      hBwl   /* [in] BWL Handle */
    )
{
    int err  = 0;
    int val  = 0;
    void *wl = hBwl->wl;
#ifdef WL_NAN
    wl_nan_ioc_t*nanioc;
    uint16 iocsz = sizeof(wl_nan_ioc_t) + WL_NAN_IOC_BUFSZ;
#endif /* WL_NAN */

#define RESET_CMD "reset_cnts"
    if ((err = wlu_iovar_getint(wl, RESET_CMD, &val)))
    {
        fprintf(stderr, "%s: wlu_iovar_getint(%s) failed ... err %d \n", __FUNCTION__, RESET_CMD, (int)err );
        return (err);
    }

#ifdef WL_NAN
    /*    alloc mem for ioctl headr +  tlv data  */
    nanioc = calloc(1, iocsz);
    if (nanioc == NULL)
    {
        fprintf(stderr, "%s: calloc(%d) failed \n", __FUNCTION__, (int) iocsz );
        return BCME_NOMEM;
    }

    /* make up nan cmd ioctl header */
    nanioc->version = htod16(WL_NAN_IOCTL_VERSION);
    nanioc->id = WL_NAN_CMD_CFG_CLEARCOUNT;
    nanioc->len = WL_NAN_IOC_BUFSZ;

    /*fprintf(stderr, "%s: wl_nan_do_ioctl(%d) trying; version 0x%x; id %d; len %d \n", __FUNCTION__, (int) iocsz, nanioc->version, nanioc->id, nanioc->len );*/
    wl_nan_do_ioctl(wl, nanioc, iocsz, FALSE);
    free(nanioc);
#endif /* WL_NAN */

    return (0);
}

/* sample command ... wl -i wlan0 wowl_pkt 104 ucast 00:10:18:D5:C3:10 magic */
int BWL_SendWakeOnWlan(
    BWL_Handle      hBwl,  /* [in] BWL Handle */
    char           *pkt_len,
    char           *destination_frame,
    char           *macAddress,
    char           *pkt_type
    )
{
    char *argv[6];
    void *wl = hBwl->wl;

    memset( argv, 0, sizeof(argv) );

    /* create argv array like it would be like if user entered commands from the command line */
    argv[0] = "wowl_pkt";
    argv[1] = pkt_len;
    argv[2] = destination_frame;
    argv[3] = macAddress;
    argv[4] = pkt_type;

    wl_wowl_pkt( wl, NULL, &argv[0] );

    return 0;
}

/* Caller is responsible for allocating space for the outputList array */
int BWL_GetMacAssocList(
    BWL_Handle       hBwl,  /* [in] BWL Handle */
    BWL_MAC_ADDRESS *outputList,
    int              outputListLen
    )
{
    int num_addresses = 0;
    void *wl = hBwl->wl;

    num_addresses = wl_maclist_2( wl, outputList, outputListLen );

    return ( num_addresses );
}

/* sample command ... wl cca_get_stats */
int BWL_GetCcaStats(
    BWL_Handle        hBwl,  /* [in] BWL Handle */
    BWLGetCcaStats_t *BWLGetCcaStats
    )
{
    char *argv[2];
    void *wl = hBwl->wl;

    memset( argv, 0, sizeof(argv) );

    /* create argv array like it would be like if user entered commands from the command line */
    argv[0] = "cca_get_stats";
    argv[1] = NULL;

    wl_cca_get_stats( wl, argv[0], argv, BWLGetCcaStats );

    return 0;
}
