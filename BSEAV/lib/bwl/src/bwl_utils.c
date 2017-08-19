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
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <typedefs.h>
#include <epivers.h>
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
extern chanspec_t wl_chspec_to_legacy(chanspec_t chspec);
#else
#include <bcmwifi.h>
#endif
#include <bcmsrom_fmt.h>
#include <bcmsrom_tbl.h>
#include <bcmcdc.h>

/* wps includes */
#ifdef INCLUDE_WPS
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
#include <linux/if_ether.h> /* ETH_P_ALL */

/* need this for using exec */
#include <unistd.h>
#include <sys/wait.h>

#include "bwl.h"
#include "bwl_priv.h"

bool bwl_g_swap = FALSE;

#if defined(linux)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#elif   defined(UNDER_CE) || defined(_CRT_SECURE_NO_DEPRECATE)
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

#define WL_DUMP_BUF_LEN (127 * 1024)

/* buffer length needed for wl_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL
 */
#define SSID_FMT_BUF_LEN ((32 * 4) + 1)
#define USAGE_ERROR  -1     /* Error code for Usage */

static unsigned char buf[1024];
int wl_pattern_atoh(char *src, char *dst);

/* 802.11i/WPA RSN IE parsing utilities */
typedef struct {
    uint16 version;
    wpa_suite_mcast_t *mcast;
    wpa_suite_ucast_t *ucast;
    wpa_suite_auth_key_mgmt_t *akm;
    uint8 *capabilities;
} rsn_parse_info_t;

static int ioctl_version = -1;

extern int
wl_get(void *wl, int cmd, void *buf, int len);
extern int
wl_set(void *wl, int cmd, void *buf, int len);


/* Wrapper function that converts -W option in to "wlc:" prefix
 * (1) It converts an existing iovar to the following format
 * wlc:<iovar_name>\0<wlc_idx><param>
 * (2) It converts an existing ioctl to the following format
 * wlc:ioc\0<wlc_idx><ioct_cmd_id><param>
 * NOTE: (2) requires new iovar named "ioc" in driver
*/
static int
wlu_wlc_wrapper(void *wl, bool get, int* cmd, void *cmdbuf, int len, void **outbuf, int *outlen)
{
    void *param = cmdbuf;
    int paramlen = len;
    int wlc_idx = g_wlc_idx;
    char *name = NULL;
    BCM_REFERENCE(wl);
    /* Wrap only if we find a valid WLC index and iovar name */
    if (wlc_idx >= 0) {
        int cmdlen = 0;
        int prefix_len = 0;
        char *lbuf = NULL;
        char *buf = NULL;
        bool ioctl_wrap = FALSE;
        if ((*cmd == WLC_GET_VAR) || (*cmd == WLC_SET_VAR)) {
            /* incoming command is an iovar */
            /* pull out name\0param */
            name = cmdbuf;
            cmdlen = strlen(name);
            param = ((char*)cmdbuf) + cmdlen + 1;
            paramlen = len - cmdlen - 1;
        } else {
            /* we are an ioctl, invoke the common "ioc" iovar and wrap the cmd */
            name = "ioc";
            cmdlen = strlen(name);
            /* additional 4 bytes for storing IOCTL_CMD_ID */
            prefix_len = sizeof(int);
            ioctl_wrap = TRUE;
        }
        prefix_len += strlen("wlc:") + 1 +  cmdlen + sizeof(int);
        /* now create wlc:<name>\0<wlc_idx><param> */
        buf = lbuf = malloc(prefix_len + paramlen);
        if (buf == NULL) {
            printf("%s:malloc(%d) failed\n", __FUNCTION__, prefix_len + paramlen);
            return BCME_NOMEM;
        }
        memcpy(buf, "wlc:", 4); buf += 4;
        strcpy(buf, name); buf += (cmdlen+1);
        wlc_idx = htod32(wlc_idx);
        memcpy(buf, &wlc_idx, sizeof(int32)); buf += sizeof(int32);
        if (ioctl_wrap) {
            /* For IOCTL wlc:ioc\0<wlc_idx><ioctl_id><param> */
            int32 ioctl_cmd = htod32(*cmd);
            memcpy(buf, &ioctl_cmd, sizeof(int32)); buf += sizeof(int32);
        }
        memcpy(buf, param, paramlen);
        *cmd = (get) ? WLC_GET_VAR : WLC_SET_VAR;
        param = lbuf;
        paramlen += prefix_len;
    }
    *outlen = paramlen;
    *outbuf = param;
    return BCME_OK;
}


/* now IOCTL GET commands shall call wlu_get() instead of wl_get() so that the commands
 * can be batched when needed
 */
int
wlu_get(void *wl, int cmd, void *cmdbuf, int len)
{
    void *outbuf = NULL;
    int outlen;
    int err = 0;

    if (g_wlc_idx > 0)
    {
        err = wlu_wlc_wrapper(wl, TRUE, &cmd, cmdbuf, len, &outbuf, &outlen);
    }
    else
    {
        outbuf = cmdbuf;
        outlen = len;
    }
    err = wl_get(wl, cmd, outbuf, outlen);

    if (outbuf != cmdbuf) {
        memcpy(cmdbuf, outbuf, len);
        free(outbuf);
    }
    return err;

}
/* now IOCTL SET commands shall call wlu_set() instead of wl_set() so that the commands
 * can be batched when needed
 */
int
wlu_set(void *wl, int cmd, void *cmdbuf, int len)
{
    void *outbuf = NULL;
    int outlen;
    int err = 0;

    if (g_wlc_idx > 0) {
        err = wlu_wlc_wrapper(wl, FALSE, &cmd, cmdbuf, len, &outbuf, &outlen);
        if (err != BCME_OK) return err;
    }
    else
    {
        outbuf = cmdbuf;
        outlen = len;
    }
    err = wl_set(wl, cmd, outbuf, outlen);

    if (outbuf != cmdbuf) {
        memcpy(cmdbuf, outbuf, len);
        free(outbuf);
    }

    return err;
}
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
 * get named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
int
wlu_iovar_getbuf(void* wl, const char *iovar,
    void *param, int paramlen, void *bufptr, int buflen)
{
    int err;

    wl_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
    if (err)
        return err;

    return wlu_get(wl, WLC_GET_VAR, bufptr, buflen);
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

/*
 * get named iovar without parameters into a given buffer
 * iovar name is converted to lower case
 */
int
wlu_iovar_get(void *wl, const char *iovar, void *outbuf, int len)
{
    char smbuf[WLC_IOCTL_SMLEN];
    int err;

    /* use the return buffer if it is bigger than what we have on the stack */
    if (len > (int)sizeof(smbuf)) {
        err = wlu_iovar_getbuf(wl, iovar, NULL, 0, outbuf, len);
    } else {
        memset(smbuf, 0, sizeof(smbuf));
        err = wlu_iovar_getbuf(wl, iovar, NULL, 0, smbuf, sizeof(smbuf));
        if (err == 0)
            memcpy(outbuf, smbuf, len);
    }

    return err;
}
/*
 * set named iovar given the parameter buffer
 * iovar name is converted to lower case
 */
int
wlu_iovar_set(void *wl, const char *iovar, void *param, int paramlen)
{
    char smbuf[WLC_IOCTL_SMLEN*2];

    memset(smbuf, 0, sizeof(smbuf));

    return wlu_iovar_setbuf(wl, iovar, param, paramlen, smbuf, sizeof(smbuf));
}
/*
 * get named iovar as an integer value
 * iovar name is converted to lower case
 */
int
wlu_iovar_getint(void *wl, const char *iovar, int *pval)
{
    int ret;

    ret = wlu_iovar_get(wl, iovar, pval, sizeof(int));
    if (ret >= 0)
    {
        *pval = dtoh32(*pval);
    }
    return ret;
}
/*
 * set named iovar given an integer parameter
 * iovar name is converted to lower case
 */
int
wlu_iovar_setint(void *wl, const char *iovar, int val)
{
    val = htod32(val);
    return wlu_iovar_set(wl, iovar, &val, sizeof(int));
}
/*
 * format a bsscfg indexed iovar buffer
 */

/* Helper routine to print the infrastructure mode while pretty printing the BSS list */
static const char *
capmode2str(uint16 capability)
{
    capability &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);

    if (capability == DOT11_CAP_ESS)
        return "Managed";
    else if (capability == DOT11_CAP_IBSS)
        return "Ad Hoc";
    else
        return "<unknown>";
}
char *
wl_ether_etoa(const struct ether_addr *n)
{
    static char etoa_buf[ETHER_ADDR_LEN * 3];
    char *c = etoa_buf;
    int i;

    for (i = 0; i < ETHER_ADDR_LEN; i++) {
        if (i)
            *c++ = ':';
        c += sprintf(c, "%02X", n->octet[i] & 0xff);
    }
    return etoa_buf;
}
/*
 * Traverse a string of 1-byte tag/1-byte length/variable-length value
 * triples, returning a pointer to the substring whose first element
 * matches tag
 */
uint8 *
wlu_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
    uint8 *cp;
    int totlen;

    cp = tlv_buf;
    totlen = buflen;

    /* find tagged parameter */
    while (totlen >= 2) {
        uint tag;
        int len;

        tag = *cp;
        len = *(cp +1);

        /* validate remaining totlen */
        if ((tag == key) && (totlen >= (len + 2)))
            return (cp);

        cp += (len + 2);
        totlen -= (len + 2);
    }

    return NULL;
}
int
wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len)
{
    int i, c;
    char *p = ssid_buf;

    if (ssid_len > 32) ssid_len = 32;

    for (i = 0; i < ssid_len; i++) {
        c = (int)ssid[i];
        if (c == '\\') {
            *p++ = '\\';
            *p++ = '\\';
        } else if (isprint((uchar)c)) {
            *p++ = (char)c;
        } else {
            p += sprintf(p, "\\x%02X", c);
        }
    }
    *p = '\0';

    return p - ssid_buf;
}
/* Validates and parses the RSN or WPA IE contents into a rsn_parse_info_t structure
 * Returns 0 on success, or 1 if the information in the buffer is not consistant with
 * an RSN IE or WPA IE.
 * The buf pointer passed in should be pointing at the version field in either an RSN IE
 * or WPA IE.
 */
static int
wl_rsn_ie_parse_info(uint8* rsn_buf, uint len, rsn_parse_info_t *rsn)
{
    uint16 count;

    memset(rsn, 0, sizeof(rsn_parse_info_t));

    /* version */
    if (len < sizeof(uint16))
        return 1;

    rsn->version = ltoh16_ua(rsn_buf);
    len -= sizeof(uint16);
    rsn_buf += sizeof(uint16);

    /* Multicast Suite */
    if (len < sizeof(wpa_suite_mcast_t))
        return 0;

    rsn->mcast = (wpa_suite_mcast_t*)rsn_buf;
    len -= sizeof(wpa_suite_mcast_t);
    rsn_buf += sizeof(wpa_suite_mcast_t);

    /* Unicast Suite */
    if (len < sizeof(uint16))
        return 0;

    count = ltoh16_ua(rsn_buf);

    if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
        return 1;

    rsn->ucast = (wpa_suite_ucast_t*)rsn_buf;
    len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
    rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

    /* AKM Suite */
    if (len < sizeof(uint16))
        return 0;

    count = ltoh16_ua(rsn_buf);

    if (len < (sizeof(uint16) + count * sizeof(wpa_suite_t)))
        return 1;

    rsn->akm = (wpa_suite_auth_key_mgmt_t*)rsn_buf;
    len -= (sizeof(uint16) + count * sizeof(wpa_suite_t));
    rsn_buf += (sizeof(uint16) + count * sizeof(wpa_suite_t));

    /* Capabilites */
    if (len < sizeof(uint16))
        return 0;

    rsn->capabilities = rsn_buf;

    return 0;
}
static uint
wl_rsn_ie_decode_cntrs(uint cntr_field)
{
    uint cntrs;

    switch (cntr_field) {
    case RSN_CAP_1_REPLAY_CNTR:
        cntrs = 1;
        break;
    case RSN_CAP_2_REPLAY_CNTRS:
        cntrs = 2;
        break;
    case RSN_CAP_4_REPLAY_CNTRS:
        cntrs = 4;
        break;
    case RSN_CAP_16_REPLAY_CNTRS:
        cntrs = 16;
        break;
    default:
        cntrs = 0;
        break;
    }

    return cntrs;
}
static int
wlu_bcmp(const void *b1, const void *b2, int len)
{
    return (memcmp(b1, b2, len));
}
/* Is this body of this tlvs entry a WPA entry? If */
/* not update the tlvs buffer pointer/length */
bool
wlu_is_wpa_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
{
    uint8 *ie = *wpaie;

    /* If the contents match the WPA_OUI and type=1 */
    if ((ie[1] >= 6) && !wlu_bcmp(&ie[2], WPA_OUI "\x01", 4)) {
        return TRUE;
    }

    /* point to the next ie */
    ie += ie[1] + 2;
    /* calculate the length of the rest of the buffer */
    *tlvs_len -= (int)(ie - *tlvs);
    /* update the pointer to the start of the buffer */
    *tlvs = ie;

    return FALSE;
}

void
wl_rsn_ie_dump(bcm_tlv_t *ie, WpaInfo_t *info)
{
    int i;
    int rsn;
    wpa_ie_fixed_t *wpa = NULL;
    rsn_parse_info_t rsn_info;
    wpa_suite_t *suite;
    uint8 std_oui[3];
    int unicast_count = 0;
    int akm_count = 0;
    uint16 capabilities;
    int err;

    if (ie->id == DOT11_MNG_RSN_ID) {
        rsn = TRUE;
        memcpy(std_oui, WPA2_OUI, WPA_OUI_LEN);
        err = wl_rsn_ie_parse_info(ie->data, ie->len, &rsn_info);
    } else {
        rsn = FALSE;
        memcpy(std_oui, WPA_OUI, WPA_OUI_LEN);
        wpa = (wpa_ie_fixed_t*)ie;
        err = wl_rsn_ie_parse_info((uint8*)&wpa->version, wpa->length - WPA_IE_OUITYPE_LEN,
                                   &rsn_info);
    }
    if (err || rsn_info.version != WPA_VERSION)
        return;

    if (rsn)
    {
        PRINTF(("RSN:\n"));
    }
    else
    {
        PRINTF(("WPA:\n"));
    }

    /* Check for multicast suite */
    if (rsn_info.mcast) {
        PRINTF(("\tmulticast cipher: "));
        if (!wlu_bcmp(rsn_info.mcast->oui, std_oui, 3)) {
            switch (rsn_info.mcast->type) {
            case WPA_CIPHER_NONE:
                PRINTF(("NONE\n"));
                info->Cipher |= eWSecNone;
                break;
            case WPA_CIPHER_WEP_40:
                PRINTF(("WEP64\n"));
                info->Cipher |= eWSecWep;
                break;
            case WPA_CIPHER_WEP_104:
                PRINTF(("WEP128\n"));
                info->Cipher |= eWSecWep;
                break;
            case WPA_CIPHER_TKIP:
                PRINTF(("TKIP\n"));
                info->Cipher |= eWSecTkip;
                break;
            case WPA_CIPHER_AES_OCB:
                PRINTF(("AES-OCB\n"));
                info->Cipher |= eWSecAes;
                break;
            case WPA_CIPHER_AES_CCM:
                PRINTF(("AES-CCMP\n"));
                info->Cipher |= eWSecAes;
                break;
            default:
                PRINTF(("Unknown-%s(#%d)\n", rsn ? "RSN" : "WPA",
                       rsn_info.mcast->type));
                break;
            }
        }
        else {
            PRINTF(("Unknown-%02X:%02X:%02X(#%d) ",
                   rsn_info.mcast->oui[0], rsn_info.mcast->oui[1],
                   rsn_info.mcast->oui[2], rsn_info.mcast->type));
        }
    }

    /* Check for unicast suite(s) */
    if (rsn_info.ucast) {
        unicast_count = ltoh16_ua(&rsn_info.ucast->count);
        PRINTF(("\tunicast ciphers(%d): ", unicast_count));
        for (i = 0; i < unicast_count; i++) {
            suite = &rsn_info.ucast->list[i];
            if (!wlu_bcmp(suite->oui, std_oui, 3)) {
                switch (suite->type) {
                case WPA_CIPHER_NONE:
                    PRINTF(("NONE "));
                    info->Cipher |= eWSecNone;
                    break;
                case WPA_CIPHER_WEP_40:
                    PRINTF(("WEP64 "));
                    info->Cipher |= eWSecWep;
                    break;
                case WPA_CIPHER_WEP_104:
                    PRINTF(("WEP128 "));
                    info->Cipher |= eWSecWep;
                    break;
                case WPA_CIPHER_TKIP:
                    PRINTF(("TKIP "));
                    info->Cipher |= eWSecTkip;
                    break;
                case WPA_CIPHER_AES_OCB:
                    PRINTF(("AES-OCB "));
                    info->Cipher |= eWSecAes;
                    break;
                case WPA_CIPHER_AES_CCM:
                    PRINTF(("AES-CCMP "));
                    info->Cipher |= eWSecAes;
                    break;
                default:
                    PRINTF(("WPA-Unknown-%s(#%d) ", rsn ? "RSN" : "WPA",
                           suite->type));
                    break;
                }
            }
            else {
                PRINTF(("Unknown-%02X:%02X:%02X(#%d) ",
                    suite->oui[0], suite->oui[1], suite->oui[2],
                    suite->type));
            }
        }
        PRINTF(("\n"));
    }
    /* Authentication Key Management */
    if (rsn_info.akm) {
        akm_count = ltoh16_ua(&rsn_info.akm->count);
        PRINTF(("\tAKM Suites(%d): ", akm_count));
        for (i = 0; i < akm_count; i++) {
            suite = &rsn_info.akm->list[i];
            if (!wlu_bcmp(suite->oui, std_oui, 3)) {
                switch (suite->type) {
                case RSN_AKM_NONE:
                    PRINTF(("None "));
                    info->Akm |= RSN_AKM_NONE;
                    break;
                case RSN_AKM_UNSPECIFIED:
                    PRINTF(("WPA "));
                    info->Akm |= RSN_AKM_UNSPECIFIED;
                    break;
                case RSN_AKM_PSK:
                case RSN_AKM_MFP_PSK:      /*to support MFP */
                    PRINTF(("WPA-PSK "));
                    info->Akm |= RSN_AKM_PSK;
                    break;
                default:
                    PRINTF(("Unknown-%s(#%d)  ",
                           rsn ? "RSN" : "WPA", suite->type));
                    break;
                }
            }
            else {
                PRINTF(("Unknown-%02X:%02X:%02X(#%d)  ",
                    suite->oui[0], suite->oui[1], suite->oui[2],
                    suite->type));
            }
        }
        PRINTF(("\n"));
    }

    /* Capabilities */
    if (rsn_info.capabilities) {
        capabilities = ltoh16_ua(rsn_info.capabilities);
        PRINTF(("\tCapabilities(0x%04x): ", capabilities));
        if (rsn)
        {
            PRINTF(("%sPre-Auth, ", (capabilities & RSN_CAP_PREAUTH) ? "" : "No "));
        }

        PRINTF(("%sPairwise, ", (capabilities & RSN_CAP_NOPAIRWISE) ? "No " : ""));

        wl_rsn_ie_decode_cntrs((capabilities & RSN_CAP_PTK_REPLAY_CNTR_MASK) >>
                                       RSN_CAP_PTK_REPLAY_CNTR_SHIFT);

        PRINTF(("%d PTK Replay Ctr%s", cntrs, (cntrs > 1)?"s":""));

        if (rsn) {
             wl_rsn_ie_decode_cntrs(
                (capabilities & RSN_CAP_GTK_REPLAY_CNTR_MASK) >>
                RSN_CAP_GTK_REPLAY_CNTR_SHIFT);

            PRINTF(("%d GTK Replay Ctr%s\n", cntrs, (cntrs > 1)?"s":""));
        } else {
            PRINTF(("\n"));
        }
    } else {
        PRINTF(("\tNo %s Capabilities advertised\n", rsn ? "RSN" : "WPA"));
    }

}

void
wl_dump_wpa_rsn_ies(uint8* cp, uint len)
{
    uint8 *parse = cp;
    uint parse_len = len;
    uint8 *wpaie;
    uint8 *rsnie;
    WpaInfo_t info;

    while ((wpaie = wlu_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
        if (wlu_is_wpa_ie(&wpaie, &parse, &parse_len))
            break;
    if (wpaie)
        wl_rsn_ie_dump((bcm_tlv_t*)wpaie, &info);

    rsnie = wlu_parse_tlvs(cp, len, DOT11_MNG_RSN_ID);
    if (rsnie)
        wl_rsn_ie_dump((bcm_tlv_t*)rsnie, &info);

    return;
}
static void
dump_rateset(uint8 *rates, uint count)
{
    uint i;
    uint r;
    bool b;

    printf("[ ");
    for (i = 0; i < count; i++) {
        r = rates[i] & 0x7f;
        b = rates[i] & 0x80;
        if (r == 0)
            break;
        printf("%d%s%s ", (r / 2), (r % 2)?".5":"", b?"(b)":"");
    }
    printf("]");
}
void
dump_bss_info(wl_bss_info_t *bi)
{
    char ssidbuf[SSID_FMT_BUF_LEN];
    char chspec_str[CHANSPEC_STR_LEN];
    wl_bss_info_107_t *old_bi;
    int mcs_idx = 0;

    /* Convert version 107 to 109 */
    if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
        old_bi = (wl_bss_info_107_t *)bi;
        bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
        bi->ie_length = old_bi->ie_length;
        bi->ie_offset = sizeof(wl_bss_info_107_t);
    }

    wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);

    printf("SSID: \"%s\"\n", ssidbuf);

    printf("Mode: %s\t", capmode2str(dtoh16(bi->capability)));
    printf("RSSI: %d dBm\t", (int16)(dtoh16(bi->RSSI)));

    /*
     * SNR has valid value in only 109 version.
     * So print SNR for 109 version only.
     */
    if (dtoh32(bi->version) == WL_BSS_INFO_VERSION) {
        printf("SNR: %d dB\t", (int16)(dtoh16(bi->SNR)));
    }

    printf("noise: %d dBm\t", bi->phy_noise);
    if (bi->flags) {
        bi->flags = dtoh16(bi->flags);
        printf("Flags: ");
        if (bi->flags & WL_BSS_FLAGS_FROM_BEACON) printf("FromBcn ");
        if (bi->flags & WL_BSS_FLAGS_FROM_CACHE) printf("Cached ");
        printf("\t");
    }
    printf("Channel: %s\n", wf_chspec_ntoa(dtohchanspec(bi->chanspec), chspec_str));

    printf("BSSID: %s\t", wl_ether_etoa(&bi->BSSID));

    printf("Capability: ");
    bi->capability = dtoh16(bi->capability);
    if (bi->capability & DOT11_CAP_ESS) printf("ESS ");
    if (bi->capability & DOT11_CAP_IBSS) printf("IBSS ");
    if (bi->capability & DOT11_CAP_POLLABLE) printf("Pollable ");
    if (bi->capability & DOT11_CAP_POLL_RQ) printf("PollReq ");
    if (bi->capability & DOT11_CAP_PRIVACY) printf("WEP ");
    if (bi->capability & DOT11_CAP_SHORT) printf("ShortPre ");
    if (bi->capability & DOT11_CAP_PBCC) printf("PBCC ");
    if (bi->capability & DOT11_CAP_AGILITY) printf("Agility ");
    if (bi->capability & DOT11_CAP_SHORTSLOT) printf("ShortSlot ");
    if (bi->capability & DOT11_CAP_CCK_OFDM) printf("CCK-OFDM ");
    printf("\n");

    printf("Supported Rates: ");
    dump_rateset(bi->rateset.rates, dtoh32(bi->rateset.count));
    printf("\n");
    if (dtoh32(bi->ie_length))
        wl_dump_wpa_rsn_ies((uint8 *)(((uint8 *)bi) + dtoh16(bi->ie_offset)),
                            dtoh32(bi->ie_length));

    if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
        printf("802.11N Capable:\n");
        bi->chanspec = dtohchanspec(bi->chanspec);
        printf("\tChanspec: %sGHz channel %d %dMHz (0x%x)\n",
            CHSPEC_IS2G(bi->chanspec)?"2.4":"5", CHSPEC_CHANNEL(bi->chanspec),
            CHSPEC_IS40(bi->chanspec) ? 40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10),
            bi->chanspec);
        printf("\tControl channel: %d\n", bi->ctl_ch);
        printf("\t802.11N Capabilities: ");
        if (dtoh32(bi->nbss_cap) & HT_CAP_40MHZ)
            printf("40Mhz ");
        if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_20)
            printf("SGI20 ");
        if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_40)
            printf("SGI40 ");
        printf("\n\tSupported MCS : [ ");
        for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++)
            if (isset(bi->basic_mcs, mcs_idx))
                printf("%d ", mcs_idx);
        printf("]\n");
    }

    printf("\n");
}
/* Pretty print the BSS list */
void
dump_networks(char *network_buf)
{
    wl_scan_results_t *list = (wl_scan_results_t*)network_buf;
    wl_bss_info_t *bi;
    uint i;

    if (list->count == 0)
        return;
    else if (list->version != WL_BSS_INFO_VERSION &&
             list->version != LEGACY2_WL_BSS_INFO_VERSION &&
             list->version != LEGACY_WL_BSS_INFO_VERSION) {
        fprintf(stderr, "Sorry, your driver has bss_info_version %d "
            "but this program supports only version %d.\n",
            list->version, WL_BSS_INFO_VERSION);
        return;
    }

    bi = list->bss_info;
    for (i = 0; i < list->count; i++, bi = (wl_bss_info_t*)((int8*)bi + dtoh32(bi->length))) {
        dump_bss_info(bi);
    }
}
/* The below macros handle endian mis-matches between wl utility and wl driver. */
int
bwl_wl_check(void *wl)
{
    int ret;
    int val;

    if ((ret = wlu_get(wl, WLC_GET_MAGIC, &val, sizeof(int)) < 0))
        return ret;

    /* Detect if IOCTL swapping is necessary */
    if (val == (int)bcmswap32(WLC_IOCTL_MAGIC))
    {
        val = bcmswap32(val);
        bwl_g_swap = TRUE;
    }
    if (val != WLC_IOCTL_MAGIC)
        return -1;


    if ((ret = wlu_get(wl, WLC_GET_VERSION, &val, sizeof(int)) < 0))
        return ret;
    ioctl_version = dtoh32(val);
    if (ioctl_version != WLC_IOCTL_VERSION &&
        ioctl_version != 1) {
        fprintf(stderr, "Version mismatch, please upgrade. Got %d, expected %d or 1\n",
                ioctl_version, WLC_IOCTL_VERSION);
        return -1;
    }

    return 0;
}

int
wl_ether_atoe(const char *a, struct ether_addr *n)
{
    char *c = NULL;
    int i = 0;

    memset(n, 0, ETHER_ADDR_LEN);
    for (;;) {
        n->octet[i++] = (uint8)strtoul(a, &c, 16);
        if (!*c++ || i == ETHER_ADDR_LEN)
            break;
        a = c;
    }
    return (i == ETHER_ADDR_LEN);
}

/* Is this body of this tlvs entry a WPS entry? If */
/* not update the tlvs buffer pointer/length */
bool
bcm_is_wps_ie(uint8_t *ie, uint8_t **tlvs, uint32_t *tlvs_len)
{
    /* If the contents match the WPA_OUI and type=1 */
    if ((ie[TLV_LEN_OFF] > (WPA_OUI_LEN+1)) &&
        !bcmp(&ie[TLV_BODY_OFF], WPA_OUI "\x04", WPA_OUI_LEN + 1)) {
        return TRUE;
    }

    /* point to the next ie */
    ie += ie[TLV_LEN_OFF] + TLV_HDR_LEN;
    /* calculate the length of the rest of the buffer */
    *tlvs_len -= (int)(ie - *tlvs);
    /* update the pointer to the start of the buffer */
    *tlvs = ie;

    return FALSE;
}


int
wlu_pattern_atoh(char *src, char *dst)
{
    int i;
    if (strncmp(src, "0x", 2) != 0 &&
        strncmp(src, "0X", 2) != 0) {
        printf("Mask invalid format. Needs to start with 0x\n");
        return -1;
    }
    src = src + 2; /* Skip past 0x */
    if (strlen(src) % 2 != 0) {
        printf("Mask invalid format. Needs to be of even length\n");
        return -1;
    }
    for (i = 0; *src != '\0'; i++) {
        char num[3];
        strncpy(num, src, 2);
        num[2] = '\0';
        dst[i] = (uint8)strtoul(num, NULL, 16);
        src += 2;
    }
    return i;
}

/* Return a new chanspec given a legacy chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_from_legacy(chanspec_t legacy_chspec)
{
    chanspec_t chspec;
#ifndef FALCON_WIFI_DRIVER

    /* get the channel number */
    chspec = LCHSPEC_CHANNEL(legacy_chspec);

    /* convert the band */
    if (LCHSPEC_IS2G(legacy_chspec)) {
        chspec |= WL_CHANSPEC_BAND_2G;
    } else {
        chspec |= WL_CHANSPEC_BAND_5G;
    }

    /* convert the bw and sideband */
    if (LCHSPEC_IS20(legacy_chspec)) {
        chspec |= WL_CHANSPEC_BW_20;
    } else {
        chspec |= WL_CHANSPEC_BW_40;
        if (LCHSPEC_CTL_SB(legacy_chspec) == WL_LCHANSPEC_CTL_SB_LOWER) {
            chspec |= WL_CHANSPEC_CTL_SB_L;
        } else {
            chspec |= WL_CHANSPEC_CTL_SB_U;
        }
    }

    if (wf_chspec_malformed(chspec)) {
        fprintf(stderr, "wl_chspec_from_legacy: output chanspec (0x%04X) malformed\n",
                chspec);
        return INVCHANSPEC;
    }
#else
    chspec = legacy_chspec;
#endif

    return chspec;
}

/* given a chanspec value from the driver in a 32 bit integer, do the endian and
 * chanspec version conversion to a chanspec_t value
 * Returns INVCHANSPEC on error
 */
chanspec_t
bwl_chspec32_from_driver(uint32 chanspec32)
{
    chanspec_t chanspec;

    chanspec = (chanspec_t)dtohchanspec(chanspec32);

    if (ioctl_version == 1) {
        chanspec = wl_chspec_from_legacy(chanspec);
    }

    return chanspec;
}

/* Convert user's input in hex pattern to byte-size mask ... copied from wluc_wowl.c 2017-04-19 */
int
wl_pattern_atoh(char *src, char *dst)
{
	int i;
	if (strncmp(src, "0x", 2) != 0 &&
	    strncmp(src, "0X", 2) != 0) {
		printf("Data invalid format. Needs to start with 0x\n");
		return -1;
	}
	src = src + 2; /* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		printf("Data invalid format. Needs to be of even length\n");
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (uint8)strtoul(num, NULL, 16);
		src += 2;
	}
	return i;
}

/* Send a wakeup frame to sta in WAKE mode ... copied from wluc_wowl.c 2017-04-19 */
int
wl_wowl_pkt(void *wl, void *cmd, char **argv)
{
	char *arg = (char*) buf;
	const char *str;
	char *dst;
	uint tot = 0;
	uint16 type, pkt_len;
	int dst_ea = 0; /* 0 == manual, 1 == bcast, 2 == ucast */
	char *ea[ETHER_ADDR_LEN];
	if (!*++argv)
		return BCME_USAGE_ERROR;

	UNUSED_PARAMETER(cmd);

    memset( buf, 0, sizeof(buf) );

	str = "wowl_pkt";
	strncpy(arg, str, strlen(str));
    /*printf("%s:%u arg 1 (%s) ... offset 0x%x\n", __FUNCTION__, __LINE__, arg, (int)((char*)arg-(char*)buf) );*/
	arg[strlen(str)] = '\0';
	dst = arg + strlen(str) + 1;
	tot += strlen(str) + 1;

	pkt_len = (uint16)htod32(strtoul(*argv, NULL, 0));

	*((uint16*)dst) = pkt_len;
    /*printf("%s:%u arg 2 (%s) ... offset 0x%x\n", __FUNCTION__, __LINE__, arg, (int)((char*)dst-(char*)buf) );*/

	dst += sizeof(pkt_len);
	tot += sizeof(pkt_len);

	if (!*++argv) {
		printf("Dest of the packet needs to be provided\n");
		return BCME_USAGE_ERROR;
	}

	/* Dest of the frame */
	if (!strcmp(*argv, "bcast")) {
		dst_ea = 1;
		if (!wl_ether_atoe("ff:ff:ff:ff:ff:ff", (struct ether_addr *)dst))
			return BCME_USAGE_ERROR;
	} else if (!strcmp(*argv, "ucast")) {
		dst_ea = 2;
		if (!*++argv) {
			printf("EA of ucast dest of the packet needs to be provided\n");
			return BCME_USAGE_ERROR;
		}
		if (!wl_ether_atoe(*argv, (struct ether_addr *)dst))
			return BCME_USAGE_ERROR;
		/* Store it */
		memcpy(ea, dst, ETHER_ADDR_LEN);
	} else if (!wl_ether_atoe(*argv, (struct ether_addr *)dst))
		return BCME_USAGE_ERROR;

	dst += ETHER_ADDR_LEN;
	tot += ETHER_ADDR_LEN;

	if (!*++argv) {
		printf("type - magic/net needs to be provided\n");
		return BCME_USAGE_ERROR;
	}

	if (strncmp(*argv, "magic", strlen("magic")) == 0)
		type = WL_WOWL_MAGIC;
	else if (strncmp(*argv, "net", strlen("net")) == 0)
		type = WL_WOWL_NET;
	else if (strncmp(*argv, "eapid", strlen("eapid")) == 0)
		type = WL_WOWL_EAPID;
	else
		return BCME_USAGE_ERROR;

	*((uint16*)dst) = type;
	dst += sizeof(type);
	tot += sizeof(type);

	if (type == WL_WOWL_MAGIC) {
		if (pkt_len < MAGIC_PKT_MINLEN)
			return BCME_BADARG;

		if (dst_ea == 2)
			memcpy(dst, ea, ETHER_ADDR_LEN);
		else {
			if (!*++argv)
				return BCME_USAGE_ERROR;

			if (!wl_ether_atoe(*argv, (struct ether_addr *)dst))
				return BCME_USAGE_ERROR;
		}
		tot += ETHER_ADDR_LEN;
    } else if (type == WL_WOWL_NET) {
        wl_wowl_pattern_t *wl_pattern;
        wl_pattern = (wl_wowl_pattern_t *)dst;

        if (!*++argv) {
            printf("Starting offset not provided\n");
            return BCME_USAGE_ERROR;
        }

        wl_pattern->offset = (uint)htod32(strtoul(*argv, NULL, 0));

        wl_pattern->masksize = 0;

        wl_pattern->patternoffset = (uint)htod32(sizeof(wl_wowl_pattern_t));

        dst += sizeof(wl_wowl_pattern_t);

        if (!*++argv) {
            printf("pattern not provided\n");
            return BCME_USAGE_ERROR;
        }

        wl_pattern->patternsize =
                (uint)htod32(wl_pattern_atoh((char *)(uintptr)*argv, dst));
        dst += wl_pattern->patternsize;
        tot += sizeof(wl_wowl_pattern_t) + wl_pattern->patternsize;

        wl_pattern->reasonsize = 0;
        if (*++argv) {
            wl_pattern->reasonsize =
                (uint)htod32(wl_pattern_atoh((char *)(uintptr)*argv, dst));
            tot += wl_pattern->reasonsize;
        }
    } else {    /* eapid */
        if (!*++argv) {
            printf("EAPOL identity string not provided\n");
            return BCME_USAGE_ERROR;
        }

        *dst++ = strlen(*argv);
        strncpy(dst, *argv, strlen(*argv));
        tot += 1 + strlen(*argv);
    }
    /*printf( "%s: arg (%s) ... tot (%u)\n", __FUNCTION__, arg, tot );*/
    return (wlu_set(wl, WLC_SET_VAR, arg, tot));
}

/* Create list of attached MAC addresses ... copied wl_maclist() from wlu.c 2017-05-26 */
int
wl_maclist_2(void *wl, BWL_MAC_ADDRESS *outputList, int outputListLen )
{
    int ret = 0;
    struct maclist *maclist = (struct maclist *) buf;
    struct ether_addr *ea = NULL;
    uint i=0, max = (WLC_IOCTL_MEDLEN - sizeof(int)) / ETHER_ADDR_LEN;

    maclist->count = htod32(max); /* this is the maximum number of addresses that we have space for */
    if ((ret = wlu_get(wl, WLC_GET_ASSOCLIST, maclist, WLC_IOCTL_MEDLEN)) < 0)
        return ret;
    maclist->count = dtoh32(maclist->count); /* this is the number of MAC addresses that were found */
    for (i = 0, ea = maclist->ea; (i < maclist->count) && (i < max) && (i < outputListLen); i++, ea++)
    {
        strncpy( (char*) &(outputList[i]), wl_ether_etoa(ea), sizeof( BWL_MAC_ADDRESS ) - 1 );
    }
    return ( i );
}
