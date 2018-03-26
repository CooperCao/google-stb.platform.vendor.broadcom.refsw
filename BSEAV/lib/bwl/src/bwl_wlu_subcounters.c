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

extern wifi_counters_t wifi_counters;

/**
 * For non-legacy mode (so user specified filter options): prevents empty lines when printing
 * counter values. For legacy mode: empty lines are preserved for backwards compatibility of tools
 * that interpret wl utility output.
 * @param cnt_filters  User specified filters to apply
 * @param ppbuf        Output buffer for the sprintf
 */
void prcnt_prnl1(struct cnt_filters_s *cnt_filters)
{
    if (at_start_of_line == FALSE || cnt_filters->filters_active == FALSE) {
        printf("\n");
        at_start_of_line = TRUE;
    }
}

/** Must be called before any of the other prcnt_*() functions */
void prcnt_init()
{
    at_start_of_line = TRUE;
}

/**
 * Returns TRUE if a counter value is allowed by the filters to be printed.
 * @param cnt_filters   Specifies the filters to be applied
 * @param cntname        The name of the variable in the 'cnt' structure
 */
static bool prcnt_filter1( struct cnt_filters_s *cnt_filters, char *cntname)
{
    if (cnt_filters->filter != 0) {
        const struct cnt_properties_s *cnt = &cnt_props[0];
        while (cnt->name != NULL) {
            if (!strcmp(cntname, cnt->name)) {
                if ((cnt_filters->filter & cnt->categories) != cnt_filters->filter) {
                    // returns FALSE in non inverted case
                    return cnt_filters->invert_selection;
                }
                break;
            }
            cnt++;
        }
        if (cnt->name == NULL)
            fprintf(stderr, "could not find this entry... %s\n", cntname);
    }

    return !cnt_filters->invert_selection;
}

/**
  * Returns TRUE if a counter value is allowed by the filters to be printed.
  * @param cnt_filters   Specifies the filters to be applied
  * @param cntname       The name of the variable in the 'cnt' structure
  * @param cntvalue      The value of the counter
  */
static bool prcnt_filter(struct cnt_filters_s *cnt_filters, char *cntname, uint32 cntvalue)
{
    if (cnt_filters->nonzero == TRUE && cntvalue == 0)
        return FALSE;

    return prcnt_filter1(cnt_filters, cntname);
}

/* This is the callback function that is called for each buffer found in the counters buffer returned from the driver */
int wl_counters_cbfn(void *ctx, const unsigned char *data, unsigned short int type, unsigned short int len)
{
    int err = BCME_OK;
    wl_cnt_cbfn_info_t *cbfn_info = ctx;
    uint i;
    struct cnt_filters_s *p_cnt_filters = &cbfn_info->cnt_filters;

    PRINIT(); /* initializes printing of counter values */

    switch (type) {
        case WL_CNT_XTLV_WLC_RINIT_RSN: {
            /*printf("%s() WL_CNT_XTLV_WLC_RINIT_RSN \n", __FUNCTION__ );*/
            /* reinit reason counters */
            reinit_rsns_t *cnt = (reinit_rsns_t *)data;
            unsigned int *val = (unsigned int *)cnt->rsn;
            uint maxoffset = len/sizeof(unsigned int); /* CAD works for 64-bit??? */

            if (len > sizeof(reinit_rsns_t)) {
                printf("type %d: cntbuf length too long! %d > %d\n"
                    "May need to use up-to-date wl utility.\n",
                    type, len, (int)sizeof(reinit_rsns_t));
            }

            if (cnt->rsn[0] != INVALID_CNT_VAL) {
                PRSTR(reinitreason_counts, "reinitreason_counts: ");
                for (i = 0; i < WL_REINIT_RC_LAST && i < maxoffset; i++) {
                    PRVAL_FMTSTR2(reinitreason_counts, "%d(%d) ", (int) i, (int) val[i]);
                }
                PRNL1();
            }
            break;
        }
        case WL_CNT_XTLV_WLC: {
            /*printf("%s() WL_CNT_XTLV_WLC \n", __FUNCTION__ );*/
            /* WLC layer counters */
            wl_cnt_wlc_t *cnt = (wl_cnt_wlc_t *)data;

            if (len > sizeof(wl_cnt_wlc_t)) {
                printf("type %d: cntbuf length too long! %d > %d\n"
                    "May need to use up-to-date wl utility.\n",
                    type, len, (int)sizeof(wl_cnt_wlc_t));
            }
#if OLDWAY
            PRVALSF(reinit);


            /* Display old reinitreason counts */
            if (cnt->reinitreason[0] != INVALID_CNT_VAL) {
                PRSTR(reinitreason_counts, "reinitreason_counts: ");
                for (i = 0; i < NREINITREASONCOUNT; i++)
                    PRVAL_FMTSTR2(reinitreason_counts, "%d(%d) ",
                        i, cnt->reinitreason[i]);
                PRNL1();
            }

            PRVALSF(reset); PRVALSF(pciereset); PRVALSF(cfgrestore);
            PRVALSF(dma_hang); PRVALSF(ampdu_wds); PRNL1();

            PRVALSF(txframe); PRVALSF(txbyte); PRVALSF(txretrans); PRVALSF(txlost);
            PRVALSF(txfail); PRVALSF(txchanrej); PRNL1();
            PRVALSF(txdatamcast); PRVALSF(txdatabcast); PRNL1();
            PRVALSF(tbtt); PRVALSF(p2p_tbtt); PRVALSF(p2p_tbtt_miss); PRNL1();
            PRVALSF(rxframe); PRVALSF(rxbyte); PRVALSF(rxerror); PRNL1();
            PRVALSF(txprshort); PRVALSF(txdmawar); PRVALSF(txnobuf); PRVALSF(txnoassoc);
            PRVALSF(txchit); PRVALSF(txcmiss); PRNL1();
            PRVALSF(txserr); PRVALSF(txphyerr); PRVALSF(txphycrs); PRVALSF(txerror);
            PRNL1();
            PRVALSF_RENAME(txfrag, d11_txfrag); PRVALSF_RENAME(txmulti, d11_txmulti);
            PRVALSF_RENAME(txretry, d11_txretry);
            PRVALSF_RENAME(txretrie, d11_txretrie); PRNL1();
            PRVALSF_RENAME(txrts, d11_txrts); PRVALSF_RENAME(txnocts, d11_txnocts);
            PRVALSF_RENAME(txnoack, d11_txnoack);
            PRVALSF_RENAME(txfrmsnt, d11_txfrmsnt); PRNL1();

            PRVALSF(rxcrc); PRVALSF(rxnobuf); PRVALSF(rxnondata); PRVALSF(rxbadds);
            PRVALSF(rxbadcm); PRVALSF(rxdup); PRVALSF(rxfragerr); PRNL1();

            PRVALSF(rxrunt); PRVALSF(rxgiant); PRVALSF(rxnoscb); PRVALSF(rxbadproto);
            PRVALSF(rxbadsrcmac); PRVALSF(rxrtry); PRNL1();
            PRVALSF_RENAME(rxfrag, d11_rxfrag); PRVALSF_RENAME(rxmulti, d11_rxmulti);
            PRVALSF_RENAME(rxundec, d11_rxundec); PRNL1();
            PRVALSF(rxctl); PRVALSF(rxbadda); PRVALSF(rxfilter);
            if (cnt->rxuflo[0] != INVALID_CNT_VAL) {
                PRSTR(rxuflo, "rxuflo: ");
                for (i = 0; i < NFIFO; i++)
                    PRVAL_FMTSTR1(rxuflo, "%u ", cnt->rxuflo[i]);
                PRNL1();
            }

            /* WPA2 counters */
            PRNL1();
            PRVALSF(tkipmicfaill); PRVALSF(tkipicverr); PRVALSF(tkipcntrmsr); PRNL1();
            PRVALSF(tkipreplay); PRVALSF(ccmpfmterr); PRVALSF(ccmpreplay); PRNL1();
            PRVALSF(ccmpundec); PRVALSF(fourwayfail); PRVALSF(wepundec); PRNL1();
            PRVALSF(wepicverr); PRVALSF(decsuccess); PRVALSF(rxundec); PRNL1();
            PRNL1();

            /* per-rate receive counters */
            PRVALSF(rx1mbps); PRVALSF(rx2mbps); PRVALSF(rx5mbps5);
            PRVALSF(rx11mbps); PRNL1();
            PRVALSF(rx6mbps); PRVALSF(rx9mbps); PRVALSF(rx12mbps);
            PRVALSF(rx18mbps); PRNL1();
            PRVALSF(rx24mbps); PRVALSF(rx36mbps); PRVALSF(rx48mbps);
            PRVALSF(rx54mbps); PRNL1();

            PRVALSF(txmpdu_sgi); PRVALSF(rxmpdu_sgi); PRVALSF(txmpdu_stbc);
            PRVALSF(rxmpdu_stbc); PRVALSF(rxmpdu_mu); PRNL1();

            PRVALSF(cso_normal); PRVALSF(cso_passthrough);
            PRNL1();
            PRVALSF(chained); PRVALSF(chainedsz1); PRVALSF(unchained);
            PRVALSF(maxchainsz); PRVALSF(currchainsz); PRNL1();
            PRNL1();

            /* detailed amangement and control frame counters */
            PRVALSF(txbar); PRVALSF(txpspoll); PRVALSF(rxbar);
            PRVALSF(rxpspoll); PRNL1();
            PRVALSF(txnull); PRVALSF(txqosnull); PRVALSF(rxnull);
            PRVALSF(rxqosnull); PRNL1();
            PRVALSF(txassocreq); PRVALSF(txreassocreq); PRVALSF(txdisassoc);
            PRVALSF(txassocrsp); PRVALSF(txreassocrsp); PRNL1();
            PRVALSF(txauth); PRVALSF(txdeauth); PRVALSF(txprobereq);
            PRVALSF(txprobersp); PRVALSF(txaction); PRNL1();
            PRVALSF(rxassocreq); PRVALSF(rxreassocreq); PRVALSF(rxdisassoc);
            PRVALSF(rxassocrsp); PRVALSF(rxreassocrsp); PRNL1();
            PRVALSF(rxauth); PRVALSF(rxdeauth); PRVALSF(rxprobereq);
            PRVALSF(rxprobersp); PRVALSF(rxaction); PRNL1();
#else
            SAVALSF(reset);
            SAVALSF(txbyte);
            SAVALSF(txframe);
            SAVALSF(txretrans);
            SAVALSF(rxerror);
            SAVALSF(txnobuf);
            SAVALSF(txserr);
            SAVALSF(txphyerr);
            SAVALSF(txerror);
            SAVALSF(rxbyte);
            SAVALSF(rxframe);
            SAVALSF(rxnobuf);
            SAVALSF(rxnondata);
            SAVALSF(rxbadcm);
            SAVALSF(rxfragerr);
            SAVALSF(rxcrc);
#endif /* OLDWAY */
            break;
        }
        case WL_CNT_XTLV_CNTV_LE10_UCODE: {
            /*printf("%s() WL_CNT_XTLV_CNTV_LE10_UCODE \n", __FUNCTION__ );*/
            wl_cnt_v_le10_mcst_t *cnt = (wl_cnt_v_le10_mcst_t *)data;
            if (len != sizeof(wl_cnt_v_le10_mcst_t)) {
                printf("type %d: cnt struct length mismatch! %d != %d\n",
                    type, len, (int)sizeof(wl_cnt_v_le10_mcst_t));
            }

#if OLDWAY
            /* UCODE SHM counters */
            PRVAL(txallfrm); PRVAL(txbcnfrm); PRVAL(txrtsfrm);
            PRVAL(txctsfrm); PRVAL(txackfrm); PRVAL(txback);
            PRVAL(txdnlfrm); PRNL1();
            PRSTR(txfunfl, "txfunfl: ");
            for (i = 0; i < NFIFO; i++) {
                PRVAL_FMTSTR1(txfunfl, "%u ", cnt->txfunfl[i]);
            }
            /*PRVAL(txtplunfl); PRVAL(txphyerror); PRNL1();*/
            PRNL1();

            PRVAL(rxstrt); PRVAL(rxbadplcp); PRVAL(rxcrsglitch);
            PRVAL(rxtoolate); PRNL1();
            PRVAL(rxdrop20s); PRVAL(rxrsptmout);  PRNL1();
            PRVAL(rxbadfcs); PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt);
            PRVAL(rxinvmachdr); PRNL1();
            PRVAL(rxf0ovfl); PRVAL(rxf1ovfl); PRVAL(rxf2ovfl);
            PRVAL(txsfovfl); PRVAL(pmqovfl); PRNL1();
            PRVAL(rxcfrmucast); PRVAL(rxrtsucast); PRVAL(rxctsucast);
            PRVAL(rxackucast); PRVAL(rxback); PRNL1();
            PRVAL(rxbeaconmbss); PRVAL(rxdfrmucastmbss);
            PRVAL(rxmfrmucastmbss); PRNL1();
            PRVAL(rxbeaconobss); PRVAL(rxdfrmucastobss);
            PRVAL(rxdfrmocast); PRVAL(rxmfrmocast); PRNL1();
            PRVAL(rxcfrmocast); PRVAL(rxrtsocast); PRVAL(rxctsocast); PRNL1();
            PRVAL(rxcfrmmcast); PRVAL(rxdfrmmcast); PRVAL(rxmfrmmcast); PRNL1();
            PRNL1();

            PRVAL(rxcgprqfrm); PRVAL(rxcgprsqovfl);
            PRVAL(txcgprsfail); PRVAL(txcgprssuc); PRVAL(prs_timeout); PRNL1();
            PRVAL(pktengrxducast); PRVAL(pktengrxdmcast);
            PRVAL(bcntxcancl); PRNL1();
            PRVAL(txfbw); PRVAL(rxnack); PRVAL(frmscons);
            PRVAL(txnack); PRNL1();
            PRNL1();
#else
            SAVALSF(rxtoolate);
            SAVALSF(rxbadfcs);
            SAVALSF(rxfrmtooshrt);
            SAVALSF(rxf0ovfl);
            SAVALSF(rxf1ovfl);
            SAVALSF(pmqovfl);
#endif /* OLDWAY */
            break;
        }
        case WL_CNT_XTLV_LT40_UCODE_V1: {
            /*printf("%s() WL_CNT_XTLV_LT40_UCODE_V1 \n", __FUNCTION__ );*/
            wl_cnt_lt40mcst_v1_t *cnt = (wl_cnt_lt40mcst_v1_t *)data;
            if (len != sizeof(wl_cnt_lt40mcst_v1_t)) {
                printf("type %d: cnt struct length mismatch! %d != %d; cnt (%p)\n",
                    type, len, (int)sizeof(wl_cnt_lt40mcst_v1_t), cnt );
            }
            PRCNT_MACSTAT_TX_VER_GE11;
            /* rx start and those that do not end well */
#if 0
            PRVAL(rxstrt); PRVAL(rxbadplcp); PRVAL(rxcrsglitch);
            PRVAL(rxtoolate); PRVAL(rxnodelim); PRNL1();
            PRVAL(bphy_badplcp); PRVAL(bphy_rxcrsglitch); PRNL1();
            PRVAL(rxbadfcs); PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt);
            PRVAL(rxanyerr); PRNL1();
            PRVAL(rxf0ovfl); PRVAL(pmqovfl); PRNL1();
            PRCNT_MACSTAT_RX_VER_GE11;
            PRNL1();
            PRVAL(dbgoff46); PRVAL(dbgoff47);
            PRVAL(dbgoff48); PRVAL(phywatch); PRNL1();
            PRNL1();
#endif
            break;
        }
        case WL_CNT_XTLV_GE40_UCODE_V1: {
            /*printf("%s() WL_CNT_XTLV_GE40_UCODE_V1 \n", __FUNCTION__ );*/
            wl_cnt_ge40mcst_v1_t *cnt = (wl_cnt_ge40mcst_v1_t *)data;
            if (len != sizeof(wl_cnt_ge40mcst_v1_t)) {
                printf("type %d: cnt struct length mismatch! %d != %d; cnt (%p)\n",
                    type, len, (int)sizeof(wl_cnt_ge40mcst_v1_t), cnt );
            }
            PRCNT_MACSTAT_TX_VER_GE11;
            /* rx start and those that do not end well */
#if 0
            PRVAL(rxstrt); PRVAL(rxbadplcp); PRVAL(rxcrsglitch);
            PRVAL(rxtoolate); PRVAL(rxnodelim); PRNL1();
            PRVAL(rxdrop20s); PRVAL(bphy_badplcp); PRVAL(bphy_rxcrsglitch); PRNL1();
            PRVAL(rxbadfcs); PRVAL(rxfrmtoolong); PRVAL(rxfrmtooshrt);
            PRVAL(rxanyerr); PRNL1();
            PRVAL(rxf0ovfl); PRVAL(rxf1ovfl); PRVAL(rxhlovfl); PRVAL(pmqovfl); PRNL1();
            PRCNT_MACSTAT_RX_VER_GE11;
            PRVAL(missbcn_dbg); PRNL1();
            PRNL1();
#endif
            break;
        }
        case WL_CNT_XTLV_GE64_UCODEX_V1: {
            /*printf("%s() WL_CNT_XTLV_GE64_UCODEX_V1 \n", __FUNCTION__ );*/
            wl_cnt_ge64mcxst_v1_t *cnt = (wl_cnt_ge64mcxst_v1_t *)data;
            if (len != sizeof(wl_cnt_ge64mcxst_v1_t)) {
                printf("type %d: cnt struct length mismatch! %d != %d\n",
                    type, len, (int)sizeof(wl_cnt_ge64mcxst_v1_t));
            }
            PRVAL(macxsusp); PRVAL(m2vmsg); PRVAL(v2mmsg); PRNL1();
            PRVAL(mboxout); PRVAL(musnd); PRVAL(sfb2v);
            PRNL1();
            break;
        }
        default:
            printf("Unknown counters type %d!! You may try updating wl utility.\n",
                type);
            break;
    }

    return err;
}

int bcm_xtlv_id(const bcm_xtlv_t *elt, bcm_xtlv_opts_t opts)
{
    int id = 0;
    if (opts & BCM_XTLV_OPTION_IDU8)
        id =  *(const unsigned char *)elt;
    else
        id = ltoh16_ua((const unsigned char *)elt);

    return id;
}

/* copied from BSEAV/connectivity/wlan/STB7271_BRANCH_15_10/linux-external-stbsoc/src/shared/bcmxtlv.c */
int bcm_xtlv_size_for_data(int dlen, bcm_xtlv_opts_t opts)
{
    int hsz;

    hsz = bcm_xtlv_hdr_size(opts);
    return ((opts & BCM_XTLV_OPTION_ALIGN32) ? ALIGN_SIZE(dlen + hsz, 4) : (dlen + hsz));
}

/*
 *  unpack all xtlv records from the issue a callback
 *  to set function one call per found tlv record
 */
int bcm_unpack_xtlv_buf(void *ctx, const unsigned char *tlv_buf, unsigned short int buflen, bcm_xtlv_opts_t opts, bcm_xtlv_unpack_cbfn_t *cbfn)
{
    unsigned short int len;
    unsigned short int type;
    int res = BCME_OK;
    int size;
    const bcm_xtlv_t *ptlv;
    int sbuflen = buflen;
    const unsigned char *data;
    int hdr_size;

    assert(!buflen || tlv_buf);
    assert(!buflen || cbfn);

    hdr_size = BCM_XTLV_HDR_SIZE_EX(opts);
    while (sbuflen >= hdr_size) {
        ptlv = (const bcm_xtlv_t *)tlv_buf;

        bcm_xtlv_unpack_xtlv(ptlv, &type, &len, &data, opts);
        size = bcm_xtlv_size_for_data(len, opts);
        /*printf("%s():%u ptlv %p; len %u; size %d \n", __FUNCTION__, __LINE__, ptlv, len, size );*/

        sbuflen -= size;
        if (sbuflen < 0) /* check for buffer overrun */
            break;

        if ( type != 0 && len != 0 )
        {
            /*printf("%s():%u calling cbfn(type=%d, len=%d) \n", __FUNCTION__, __LINE__, type, len );*/
            if ((res = cbfn(ctx, data, type, len)) != BCME_OK) break;
        }
        tlv_buf += size;
    }
    return res;
}

/* copied from BSEAV/connectivity/wlan/STB7271_BRANCH_15_10/linux-external-stbsoc/src/shared/bcmxtlv.c */
int bcm_xtlv_hdr_size(bcm_xtlv_opts_t opts)
{
    int len = (int)OFFSETOF(bcm_xtlv_t, data); /* nominal */
    if (opts & BCM_XTLV_OPTION_LENU8) --len;
    if (opts & BCM_XTLV_OPTION_IDU8) --len;

    return len;
}

/* copied from BSEAV/connectivity/wlan/STB7271_BRANCH_15_10/linux-external-stbsoc/src/shared/bcmxtlv.c */
int bcm_xtlv_len(const bcm_xtlv_t *elt, bcm_xtlv_opts_t opts)
{
    const uint8 *lenp;
    int len;

    lenp = (const uint8 *)&elt->len; /* nominal */
    if (opts & BCM_XTLV_OPTION_IDU8) --lenp;

    if (opts & BCM_XTLV_OPTION_LENU8)
        len = *lenp;
    else
        len = ltoh16_ua(lenp);

    return len;
}
/* copied from BSEAV/connectivity/wlan/STB7271_BRANCH_15_10/linux-external-stbsoc/src/shared/bcmxtlv.c */
/* xtlv header is always packed in LE order */
void bcm_xtlv_unpack_xtlv(const bcm_xtlv_t *xtlv, uint16 *type, uint16 *len, const uint8 **data, bcm_xtlv_opts_t opts)
{
    if (type) *type = (uint16)bcm_xtlv_id(xtlv, opts);
    if (len) *len = (uint16)bcm_xtlv_len(xtlv, opts);
    if (data) *data = (const uint8 *)xtlv + BCM_XTLV_HDR_SIZE_EX(opts);
}
