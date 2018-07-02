 /******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "wifi_convert.h"
#include "wifi_priv.h"
#include "wifi_sa.h"
#include "bmon_utils.h"
#include "wifi_bwl_interface.h"
#include "bmon_uri.h"


#define WIFI_INTERFACE_NAME "wlan0"
const char *bmon_wifi_commands[] = {"connectedSTAs","spectrumData","transmitStatistics","receiveStatistics","transmitAMPDUStatistcs","receiveAMPDUStatistcs","ampduData","iq_samples","commandList","wifi_de_stats","currentStats","powerStats","chanimStats","all",};
const char *bmon_wifi_set_commands[] = {"triggerScan","POWSWindow",};

#define PAYLOAD_SIZE 16384*4

//#define MEM_DEBUG
#ifdef MEM_DEBUG
static unsigned int count=0;
#endif

enum bmon_wifi_command{
    CONNECTED_STA_LIST,
    SPECTRUM_ANALYSIS,
    TX_PACKETS_STATS,
    RX_PACKETS_STATS,
    TX_AMPDU_STATS,
    RX_AMPDU_STATS,
    AMPDU_CHART,
    IQ_SAMPLES,
    HELP,
    WIFI_DE_STATS,
    CURRENT_STATS,
    POWER_STATS,
    CHANIM_STATS,
    ALL,
    SPECTRUM_ANALYSIS_TRIGGER,
    POWS_WINDOW_VALUE
};

unsigned int wifi_get_connected_sta_list(unsigned int numberSTAs, CONNECTED_MAC_ADDRESS* list)
{
    unsigned int ret =0;
    ret= bmon_wifi_get_connected_stas(WIFI_INTERFACE_NAME,numberSTAs, list);
    return ret;
}

void wifi_trigger_spectrum_data(void)
{
        bmon_wifi_trigger_scan(WIFI_INTERFACE_NAME);
}

void wifi_get_spectrum_data (struct spectrum_data_t **datap,unsigned int *num)
{
    ScanInfo_t *pData;
    struct spectrum_data_t * data;

    pData=bmon_wifi_get_scanresults(WIFI_INTERFACE_NAME,num);

    if(pData) {
        data = (struct spectrum_data_t *)malloc(sizeof(struct spectrum_data_t)*(*num));
#ifdef MEM_DEBUG
        printf("MALLOC%d %s %p\n",count,"(struct spectrum_data_t *)",data);
        count++;
#endif
        if( data )
        {
            int ii=0;
            memset(data,0,sizeof(struct spectrum_data_t));
            *datap = data;

            for( ii = 0; ii < *num; ii++ )
            {
                strcpy(data[ii].SSID,pData[ii].tCredentials.acSSID);
                data[ii].octet[0]=pData[ii].BSSID.octet[0];
                data[ii].octet[1]=pData[ii].BSSID.octet[1];
                data[ii].octet[2]=pData[ii].BSSID.octet[2];
                data[ii].octet[3]=pData[ii].BSSID.octet[3];
                data[ii].octet[4]=pData[ii].BSSID.octet[4];
                data[ii].octet[5]=pData[ii].BSSID.octet[5];
                data[ii].lRSSI=pData[ii].lRSSI;
                data[ii].ulPhyNoise=pData[ii].lPhyNoise;
                data[ii].ulChan=pData[ii].ulChan;
                data[ii].ulPrimChan=pData[ii].ulPrimChan;
                data[ii].ulBandwidth=pData[ii].tBandwidth;
                data[ii].lRate=pData[ii].lRate;
                data[ii].ul802_11Modes=pData[ii].ul802_11Modes;
            }
        }
        free(pData);
    }
}

void wifi_get_tx_stats (struct wifi_tx_stats_t *data)
{
    WiFiCounters_t    tCounters;

    memset( &tCounters, 0, sizeof( tCounters ));

    bmon_wifi_get_counters( WIFI_INTERFACE_NAME, &tCounters );

    data->txframes = tCounters.txframe;
    data->txretrans = tCounters.txretrans;
    data->txerror = tCounters.txerror;
    data->txserr = tCounters.txserr;
    data->txnobuf = tCounters.txnobuf;
    data->txnoassoc = tCounters.txnoassoc;
    data->txrunt = tCounters.txrunt;
#ifdef WIFI_PLUGIN_DEBUG
    printf("txframe %d,txretrans %d, txerror %d, txserr %d, txnobuf %d, txnoassoc %d, txrunt %d\n",
        data->txframes,
        data->txretrans,
        data->txerror,
        data->txserr,
        data->txnobuf,
        data->txnoassoc,
        data->txrunt);
#endif
}

void wifi_get_rx_stats (struct wifi_rx_stats_t *data)
{
    WiFiCounters_t    tCounters;

    memset( &tCounters, 0, sizeof( tCounters ));

    bmon_wifi_get_counters( WIFI_INTERFACE_NAME, &tCounters );

    data->rxframe = tCounters.rxframe;
    data->rxerror = tCounters.rxerror;
    data->rxnobuf = tCounters.rxnobuf;
    data->rxnondata = tCounters.rxnondata;
    data->rxbadds = tCounters.rxbadds;
    data->rxbadcm = tCounters.rxbadcm;
    data->rxfragerr = tCounters.rxfragerr;
    data->rxrunt = tCounters.rxrunt;
    data->rxnoscb = tCounters.rxnoscb;
    data->rxbadproto = tCounters.rxbadproto;
    data->rxbadsrcmac = tCounters.rxbadsrcmac;
    data->rxbadda = tCounters.rxbadda;
#ifdef WIFI_PLUGIN_DEBUG
    printf("rxframe %d rxerror %d rxnobuf %d rxnondata %d rxbadds %d rxbadcm %d rxfragerr %d rxrunt %d rxnoscb %d rxbadproto %d rxbadsrcmac %d rxbadda %d\n",
        data->rxframe = tCounters.rxframe,
        data->rxerror = tCounters.rxerror,
        data->rxnobuf = tCounters.rxnobuf,
        data->rxnondata = tCounters.rxnondata,
        data->rxbadds = tCounters.rxbadds,
        data->rxbadcm = tCounters.rxbadcm,
        data->rxfragerr = tCounters.rxfragerr,
        data->rxrunt = tCounters.rxrunt,
        data->rxnoscb = tCounters.rxnoscb,
        data->rxbadproto = tCounters.rxbadproto,
        data->rxbadsrcmac = tCounters.rxbadsrcmac,
        data->rxbadda = tCounters.rxbadda);
#endif
}

void wifi_get_txampdu_stats (struct wifi_txampdu_stats_t *data)
{
    ampdu_data_t      AmpduData;
    memset( &AmpduData, 0, sizeof( AmpduData ));

    bmon_wifi_ampdu_get_report( WIFI_INTERFACE_NAME );
    bmon_wifi_read_ampdu_data( &AmpduData );
    bmon_wifi_clear_ampdu( WIFI_INTERFACE_NAME );

    data->txampdu=AmpduData.tx_ampdus;
    data->txmpdu=AmpduData.tx_mpdus;
    data->txmpduperampdu=AmpduData.tx_mpduperampdu;
    data->retry_ampdu=AmpduData.retry_ampdu;
    data->retry_mpdu=AmpduData.retry_mpdu;
    data->txbar=AmpduData.txbar;
    data->rxba=AmpduData.rxba;
}
void wifi_get_rxampdu_stats (struct wifi_rxampdu_stats_t *data)
{
    ampdu_data_t      AmpduData;
    memset( &AmpduData, 0, sizeof( AmpduData ));

    bmon_wifi_ampdu_get_report( WIFI_INTERFACE_NAME );
    bmon_wifi_read_ampdu_data( &AmpduData );
    bmon_wifi_clear_ampdu( WIFI_INTERFACE_NAME );

    data->rxampdu=AmpduData.rx_ampdus;
    data->rxmpdu=AmpduData.rx_mpdus;
    data->rxmpduperampdu=AmpduData.rx_mpduperampdu;
    data->rxbar=AmpduData.rxbar;

    data->txba=AmpduData.txba;
}

void wifi_get_power_stats(struct wifi_power_stats_t *data)
{
    bmon_wifi_get_pm2_rcv_dur(WIFI_INTERFACE_NAME,&(data->POWSWindow));
}

void wifi_set_power_stats(unsigned int val)
{
    bmon_wifi_set_pm2_rcv_dur(WIFI_INTERFACE_NAME,val);
}

void wifi_get_chanim_count(uint32_t *num)
{
    bmon_wifi_chanim_get_count(WIFI_INTERFACE_NAME, num);
}

void wifi_get_chanim_stats(struct wifi_chanim_stats_t *data, uint32_t num)
{
    int32_t ii;
    chanim_data_t*  chanimData;

    if (!data) {
#if defined(WIFI_PLUGIN_DEBUG)
        printf("error : invalid chanim_stats data ptr\n");
#endif
        return;
    }
    chanimData = (chanim_data_t*)malloc(sizeof(chanim_data_t) * num);
    bmon_wifi_chanim_get_report(WIFI_INTERFACE_NAME, chanimData);

    for (ii = 0; ii < num; ii++) {
        data[ii].bgnoise = chanimData[ii].bgnoise;
        data[ii].chanspec = chanimData[ii].chanspec;
        data[ii].channum = chanimData[ii].channum;
        data[ii].timestamp = chanimData[ii].timestamp;
        data[ii].chan_idle = chanimData[ii].chan_idle;
        data[ii].tx = chanimData[ii].tx;
        data[ii].ibss = chanimData[ii].ibss;
        data[ii].obss = chanimData[ii].obss;
        data[ii].busy = chanimData[ii].busy;
        data[ii].avail = chanimData[ii].avail;
        data[ii].nwifi = chanimData[ii].nwifi;
#if CCASTATS_EX
        data[ii].glitchcnt = chanimData[ii].glitchcnt;
        data[ii].badplcp = chanimData[ii].badplcp;
        data[ii].bphy_glitchcnt = chanimData[ii].bphy_glitchcnt;
        data[ii].bphy_badplcp = chanimData[ii].bphy_badplcp;
#endif
    }
}

void wifi_get_current_stats(struct wifi_current_stats_t *data)
{
    ScanInfo_t  tScanInfo;
    char   wifiDriverVersion[STRING_SIZE];
    RevInfo_t  tRevInfo;
    phy_rssi_ant_t pRssiAnt;
    unsigned int    pNRate;
    int ret=0;

    ret=bmon_wifi_get_connected_info(WIFI_INTERFACE_NAME,&tScanInfo);
    if(ret==0){
        snprintf (data->MACaddress, STRING_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X",
               tScanInfo.BSSID.octet[0], tScanInfo.BSSID.octet[1], tScanInfo.BSSID.octet[2],
               tScanInfo.BSSID.octet[3], tScanInfo.BSSID.octet[4], tScanInfo.BSSID.octet[5]);
        data->RSSI=tScanInfo.lRSSI;
        data->phy_noise=tScanInfo.lPhyNoise;
        data->wifi80211Modes=tScanInfo.ul802_11Modes;
        data->primaryChannel=tScanInfo.ulPrimChan;
        data->channel=tScanInfo.ulChan;
        data->locked=tScanInfo.bLocked;
        data->WPS=tScanInfo.bWPS;
        data->rate=tScanInfo.lRate;
        strcpy(data->SSID,tScanInfo.tCredentials.acSSID);
        strcpy(data->bandwidth,bmon_wifi_get_BandwidthStr( tScanInfo.tBandwidth ));
        strcpy(data->channelSpec,tScanInfo.cChanSpec);
        data->authenticationType=tScanInfo.ulAuthType;
    }
    bmon_wifi_get_driver_version( WIFI_INTERFACE_NAME, &wifiDriverVersion[0], sizeof(wifiDriverVersion) );
    strcpy(data->driverVersion,wifiDriverVersion);

    bmon_wifi_get_revs(WIFI_INTERFACE_NAME,&tRevInfo);
    data->deviceID=tRevInfo.ulDeviceId;
    data->chipNumber=tRevInfo.ulChipNum;
    data->pciVendorID=tRevInfo.ulVendorId;

    bmon_wifi_get_rssi_ant(WIFI_INTERFACE_NAME,&pRssiAnt);
    data->phy_rssi_ant[0]=pRssiAnt.rssi_ant[0];
    data->phy_rssi_ant[1]=pRssiAnt.rssi_ant[1];
    data->phy_rssi_ant[2]=pRssiAnt.rssi_ant[2];
    data->phy_rssi_ant[3]=pRssiAnt.rssi_ant[3];


    bmon_wifi_get_NRate(WIFI_INTERFACE_NAME,&pNRate);
    bmon_wifi_get_rate_print(data->NRate,pNRate);
    bmon_wifi_get_mode(WIFI_INTERFACE_NAME,&(data->apsta));
}

void wifi_get_ampdu_chart(struct wifi_ampdu_chart_t *data)
{
    ampdu_data_t      AmpduData;
    unsigned long int lAmpduTotalUsage = 0;
    memset( &AmpduData, 0, sizeof( AmpduData ));
    {
        int rxtx   = 0;
        int vhtsgi = 0;
        int ants   = 0;
        int mcs    = 0;

        bmon_wifi_ampdu_get_report( WIFI_INTERFACE_NAME );
        bmon_wifi_read_ampdu_data( &AmpduData );
        bmon_wifi_clear_ampdu( WIFI_INTERFACE_NAME );

        for (ants = 0; ants<BMON_NUM_ANTENNAS; ants++)
        {
            for (mcs = 0; mcs<BMON_NUM_MCS; mcs++)
            {
                lAmpduTotalUsage += AmpduData.antennas.rx[ants+0].level[mcs];
                lAmpduTotalUsage += AmpduData.antennas.rx[ants+4].level[mcs];
                lAmpduTotalUsage += AmpduData.antennas.tx[ants+0].level[mcs];
                lAmpduTotalUsage += AmpduData.antennas.tx[ants+4].level[mcs];
#ifdef WIFI_PLUGIN_DEBUG
                    printf("%s - AMPDU.rx[%d][%d] = %u ... %lu\n", __FUNCTION__, ants+0, mcs, AmpduData.antennas.rx[ants+0].level[mcs], lAmpduTotalUsage   );
                    printf("%s - AMPDU.rx[%d][%d] = %u ... %lu\n", __FUNCTION__, ants+4, mcs, AmpduData.antennas.rx[ants+4].level[mcs], lAmpduTotalUsage    );
                    printf("%s - AMPDU.tx[%d][%d] = %u ... %lu\n", __FUNCTION__, ants+0, mcs, AmpduData.antennas.tx[ants+0].level[mcs], lAmpduTotalUsage    );
                    printf("%s - AMPDU.tx[%d][%d] = %u ... %lu\n", __FUNCTION__, ants+4, mcs, AmpduData.antennas.tx[ants+4].level[mcs], lAmpduTotalUsage    );
#endif
            }
        }
        if (lAmpduTotalUsage)
        {
            for (ants = 0; ants<BMON_NUM_ANTENNAS; ants++)
            {
                for (mcs = 0; mcs<BMON_NUM_MCS; mcs++)
                    {
                        data->ampdu_data[rxtx+0][vhtsgi+0][ants][mcs] = AmpduData.antennas.rx[ants+0].level[mcs] * 100 / lAmpduTotalUsage;
                        data->ampdu_data[rxtx+0][vhtsgi+1][ants][mcs] = AmpduData.antennas.rx[ants+4].level[mcs] * 100 / lAmpduTotalUsage;
                        data->ampdu_data[rxtx+1][vhtsgi+0][ants][mcs] = AmpduData.antennas.tx[ants+0].level[mcs] * 100 / lAmpduTotalUsage;
                        data->ampdu_data[rxtx+1][vhtsgi+1][ants][mcs] = AmpduData.antennas.tx[ants+4].level[mcs] * 100 / lAmpduTotalUsage;
#ifdef WIFI_PLUGIN_DEBUG
                            printf("%s - ampdu[%d][%d][%d][%d] = %u\n", __FUNCTION__, rxtx+0, vhtsgi+0, ants+0, mcs, data->ampdu_data[rxtx+0][vhtsgi+0][ants][mcs] );
                            printf("%s - ampdu[%d][%d][%d][%d] = %u\n", __FUNCTION__, rxtx+0, vhtsgi+1, ants+0, mcs, data->ampdu_data[rxtx+0][vhtsgi+1][ants][mcs] );
                            printf("%s - ampdu[%d][%d][%d][%d] = %u\n", __FUNCTION__, rxtx+1, vhtsgi+0, ants+0, mcs, data->ampdu_data[rxtx+1][vhtsgi+0][ants][mcs] );
                            printf("%s - ampdu[%d][%d][%d][%d] = %u\n", __FUNCTION__, rxtx+1, vhtsgi+1, ants+0, mcs, data->ampdu_data[rxtx+1][vhtsgi+1][ants][mcs] );
#endif
                    }
            }
        }
    }
}

#ifdef WIFI_DE
void wifi_get_dataelements_stats (struct wifi_dataelements_t *data)
{
    unsigned int devitr =0;
    void *ptr=NULL;

    strcpy(data->deID,"testID");
    strcpy(data->controllerID,"ControllerID");
    data->numberOfDevices=1;
    data->deviceList=(struct DE_devices_t**)malloc((data->numberOfDevices)*sizeof(struct DE_devices_t*));
#ifdef MEM_DEBUG
    printf("MALLOC%d %s %p\n",count,"(struct DE_devices_t**)",data->deviceList);
    count++;
#endif

    if(data->deviceList==NULL)
        return;
    for(devitr=0;devitr<(data->numberOfDevices);devitr++) {
        struct DE_devices_t*    de_dev;
        unsigned int radioitr =0;

        de_dev=(struct DE_devices_t*)malloc(sizeof(struct DE_devices_t));
#ifdef MEM_DEBUG
        printf("MALLOC%d %s %p\n",count,"(struct DE_devices_t*)",de_dev);
        count++;
#endif

        if(de_dev==NULL)
            return;
        *((struct DE_devices_t**)(data->deviceList)+devitr)=de_dev;
        strcpy(de_dev->devID,"Device1");

        de_dev->MultiAPCap=0;
        de_dev->NumberOfRadios=1;
        de_dev->RadioList=(struct DE_RadioList_t**)malloc((de_dev->NumberOfRadios)*sizeof(struct DE_RadioList_t*));
#ifdef MEM_DEBUG
        printf("MALLOC%d %s %p\n",count,"(struct DE_RadioList_t**)",de_dev->RadioList);
        count++;
#endif
        if(de_dev->RadioList==NULL)
            return;
        for(radioitr=0;radioitr<(de_dev->NumberOfRadios);radioitr++) {
            struct DE_RadioList_t*          de_radiolist;
            struct DE_Capabilites_t*        de_caps;
            struct DE_BackHaulSTA_t*        de_bh_STA;
            struct DE_ScanResultList_t*     de_scanresults_list;
            unsigned int curr_oper_classes_itr=0;
            unsigned int cap_oper_classes_itr=0;
            unsigned int unassoc_sta_list_itr=0;
            unsigned int scanresults_chann_list_itr=0;
            unsigned int bsslist_itr=0;


            de_radiolist=(struct DE_RadioList_t*)malloc(sizeof(struct DE_RadioList_t));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_RadioList_t*)",de_radiolist);
            count++;
#endif
            if(de_radiolist==NULL)
                return;
            *((struct DE_RadioList_t**)(de_dev->RadioList)+radioitr)=de_radiolist;

            strcpy(de_radiolist->radioID,"Radio1");
            strcpy(de_radiolist->radioEnabled,"True");
            de_radiolist->noise=101;
            de_radiolist->utilization=22;
            de_radiolist->utilizationOther=33;

            de_radiolist->numberOfCurrentOperatingClasses=2;
            de_radiolist->currentOperatingClasses=(struct DE_CurrentOperatingClasses_t**)malloc((de_radiolist->numberOfCurrentOperatingClasses)*sizeof(struct DE_CurrentOperatingClasses_t*));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_CurrentOperatingClasses_t**)",de_radiolist->currentOperatingClasses);
            count++;
#endif
            if(de_radiolist->currentOperatingClasses==NULL)
                return;
            for(curr_oper_classes_itr=0;curr_oper_classes_itr<(de_radiolist->numberOfCurrentOperatingClasses);curr_oper_classes_itr++){
                struct DE_CurrentOperatingClasses_t*  de_currentoperatingclasses;

                de_currentoperatingclasses=(struct DE_CurrentOperatingClasses_t*)malloc(sizeof(struct DE_CurrentOperatingClasses_t));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_CurrentOperatingClasses_t*)",de_currentoperatingclasses);
                count++;
#endif
                if(de_currentoperatingclasses==NULL)
                    return;
                *((struct DE_CurrentOperatingClasses_t**)(de_radiolist->currentOperatingClasses)+curr_oper_classes_itr)=de_currentoperatingclasses;

                de_currentoperatingclasses->currentOperatingClass =115;
                de_currentoperatingclasses->channel=48;
                de_currentoperatingclasses->maxTxPower=11;
            }

            de_caps=(struct DE_Capabilites_t*)malloc(sizeof(struct DE_Capabilites_t));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_Capabilites_t*)",de_caps);
            count++;
#endif
            if(de_caps==NULL)
                return;
            memset(de_caps,0,sizeof(struct DE_Capabilites_t));
            de_radiolist->capabilities=de_caps;

            de_caps->HTCapabilities[0]=0x12;
            de_caps->VHTCapabilities[0]=0xFE;
            de_caps->VHTCapabilities[1]=0xDC;
            de_caps->VHTCapabilities[2]=0xBA;
            de_caps->VHTCapabilities[3]=0x98;
            de_caps->VHTCapabilities[4]=0x76;
            de_caps->VHTCapabilities[5]=0x54;
            de_caps->HECapabilities[0]=0x12;
            de_caps->HECapabilities[1]=0x34;
            de_caps->HECapabilities[2]=0x56;
            de_caps->HECapabilities[3]=0x78;
            de_caps->HECapabilities[4]=0x9a;
            de_caps->HECapabilities[5]=0xbc;
            de_caps->HECapabilities[6]=0xde;
            de_caps->HECapabilities[7]=0xf0;
            de_caps->HECapabilities[8]=0x12;
            de_caps->HECapabilities[9]=0x34;
            de_caps->HECapabilities[10]=0x56;
            de_caps->HECapabilities[11]=0x78;
            de_caps->HECapabilities[12]=0x9a;
            de_caps->HECapabilities[13]=0xbc;

            de_caps->numberOfOperatingClasses=2;
            de_caps->operatingClasses=(struct DE_OperatingClasses_t**)malloc((de_caps->numberOfOperatingClasses)*sizeof(struct DE_OperatingClasses_t*));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_OperatingClasses_t**)",de_caps->operatingClasses);
            count++;
#endif
            if(de_caps->operatingClasses==NULL)
                return;
            for(cap_oper_classes_itr=0;cap_oper_classes_itr<(de_caps->numberOfOperatingClasses);cap_oper_classes_itr++){
                struct DE_OperatingClasses_t*  de_capoperatingclasses;
                unsigned int    cap_nonoperable_itr=0;

                de_capoperatingclasses=(struct DE_OperatingClasses_t*)malloc(sizeof(struct DE_OperatingClasses_t));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_OperatingClasses_t*)",de_capoperatingclasses);
                count++;
#endif
                if(de_capoperatingclasses==NULL)
                    return;
                *((struct DE_OperatingClasses_t**)(de_caps->operatingClasses)+cap_oper_classes_itr)=de_capoperatingclasses;
                de_capoperatingclasses->operatingClass=0;
                de_capoperatingclasses->MaxTxPower=19;
                de_capoperatingclasses->sizeOfNonOperable=4;
                de_capoperatingclasses->NonOperable=(unsigned int*)malloc(sizeof(unsigned int*)*(de_capoperatingclasses->sizeOfNonOperable));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(unsigned int*)nonoperable",de_capoperatingclasses->NonOperable);
                count++;
#endif

                if(de_capoperatingclasses->NonOperable==NULL)
                    return;
                for(cap_nonoperable_itr=0;cap_nonoperable_itr<(de_capoperatingclasses->sizeOfNonOperable);cap_nonoperable_itr++){
                    de_capoperatingclasses->NonOperable[cap_nonoperable_itr]=0;
                }
            }
            de_bh_STA=(struct DE_BackHaulSTA_t*)malloc(sizeof(struct DE_BackHaulSTA_t));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_BackHaulSTA_t*)",de_bh_STA);
            count++;
#endif

            if(de_bh_STA==NULL)
                return;
            memset(de_bh_STA,0,sizeof(struct DE_BackHaulSTA_t));
            de_radiolist->backhaulSTA=de_bh_STA;

            de_radiolist->numberOfunassociatedSTAList=2;
            de_radiolist->unassociatedSTAList=(struct DE_UnassociatedSTAList_t**)malloc((de_radiolist->numberOfunassociatedSTAList)*sizeof(struct DE_UnassociatedSTAList_t*));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_UnassociatedSTAList_t**)",de_radiolist->unassociatedSTAList);
            count++;
#endif
            if(de_radiolist->unassociatedSTAList==NULL)
                return;
            for(unassoc_sta_list_itr=0;unassoc_sta_list_itr<(de_radiolist->numberOfunassociatedSTAList);unassoc_sta_list_itr++){
                struct DE_UnassociatedSTAList_t*  de_unassoc_sta_list;
                de_unassoc_sta_list=(struct DE_UnassociatedSTAList_t*)malloc(sizeof(struct DE_UnassociatedSTAList_t));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_UnassociatedSTAList_t*)",de_unassoc_sta_list);
                count++;
#endif
                if(de_unassoc_sta_list==NULL)
                    return;
                *((struct DE_UnassociatedSTAList_t**)(de_radiolist->unassociatedSTAList)+unassoc_sta_list_itr)=de_unassoc_sta_list;
                memset(de_unassoc_sta_list,0,sizeof(struct DE_UnassociatedSTAList_t));
            }

            de_scanresults_list=(struct DE_ScanResultList_t*)malloc(sizeof(struct DE_ScanResultList_t));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_ScanResultList_t*)",de_scanresults_list);
            count++;
#endif
            if(de_scanresults_list==NULL)
                return;
            memset(de_scanresults_list,0,sizeof(struct DE_ScanResultList_t));
            de_radiolist->scanResultList=de_scanresults_list;
            de_scanresults_list->numberOfChannels=4;
            de_scanresults_list->lastScan=0;
            de_scanresults_list->channelList=(struct DE_ChannelScan_t**)malloc((de_scanresults_list->numberOfChannels)*sizeof(struct DE_ChannelScan_t*));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_ChannelScan_t**)",de_scanresults_list->channelList);
            count++;
#endif

            if(de_scanresults_list->channelList==NULL)
                return;
            for(scanresults_chann_list_itr=0;scanresults_chann_list_itr<(de_scanresults_list->numberOfChannels);scanresults_chann_list_itr++){
                struct DE_ChannelScan_t*  de_scanresults_chan_list;
                unsigned int operatingclass_scanlist_itr=0;
                unsigned int neighborlist_itr=0;

                de_scanresults_chan_list=(struct DE_ChannelScan_t*)malloc(sizeof(struct DE_ChannelScan_t));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_ChannelScan_t*)",de_scanresults_chan_list);
                count++;
#endif
                if(de_scanresults_chan_list==NULL)
                    return;
                *((struct DE_ChannelScan_t**)(de_scanresults_list->channelList)+scanresults_chann_list_itr)=de_scanresults_chan_list;
                memset(de_scanresults_chan_list,0,sizeof(struct DE_ChannelScan_t));
                de_scanresults_chan_list->channel=44;
                de_scanresults_chan_list->utilization=33;
                de_scanresults_chan_list->noise=22;
                de_scanresults_chan_list->sizeofoperatingClass=2;
                de_scanresults_chan_list->operatingClass=(unsigned int*)malloc(sizeof(unsigned int*)*(de_scanresults_chan_list->sizeofoperatingClass));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(unsigned int*)operatingclass",de_scanresults_chan_list->operatingClass);
                count++;
#endif
                if(de_scanresults_chan_list->operatingClass==NULL)
                    return;
                for(operatingclass_scanlist_itr=0;operatingclass_scanlist_itr<(de_scanresults_chan_list->sizeofoperatingClass);operatingclass_scanlist_itr++){
                    de_scanresults_chan_list->operatingClass[operatingclass_scanlist_itr]=0;
                }
                de_scanresults_chan_list->numberOfNeighbors=2;
                de_scanresults_chan_list->neighborList=(struct DE_NeighborList_t**)malloc((de_scanresults_chan_list->numberOfNeighbors)*sizeof(struct DE_NeighborList_t*));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_NeighborList_t**)",de_scanresults_chan_list->neighborList);
                count++;
#endif

                if(de_scanresults_chan_list->neighborList==NULL)
                    return;
                for(neighborlist_itr=0;neighborlist_itr<(de_scanresults_chan_list->numberOfNeighbors);neighborlist_itr++){
                    struct DE_NeighborList_t*  de_neighborlist;
                    de_neighborlist=(struct DE_NeighborList_t*)malloc(sizeof(struct DE_NeighborList_t));
#ifdef MEM_DEBUG
                    printf("MALLOC%d %s %p\n",count,"(struct DE_NeighborList_t*)",de_neighborlist);
                    count++;
#endif
                    if(de_neighborlist==NULL)
                        return;
                    *((struct DE_NeighborList_t**)(de_scanresults_chan_list->neighborList)+neighborlist_itr)=de_neighborlist;
                    memset(de_neighborlist,0,sizeof(struct DE_NeighborList_t));
                    de_neighborlist->BSSID[0]=0xAABBCCDDEEF1 + neighborlist_itr;
                    strcpy(de_neighborlist->SSID,"TESTSSID");
                    de_neighborlist->signalStrength=0 + neighborlist_itr;
                    de_neighborlist->channelBandwidth=0 + neighborlist_itr;
                    de_neighborlist->stationCount=0;
                    de_neighborlist->channelUtilization=20 + neighborlist_itr;
                }
            }
            de_radiolist->numberOfBSS=1;
            de_radiolist->BSSList=(struct DE_BSSList_t**)malloc((de_radiolist->numberOfBSS)*sizeof(struct DE_BSSList_t*));
#ifdef MEM_DEBUG
            printf("MALLOC%d %s %p\n",count,"(struct DE_BSSList_t**)",de_radiolist->BSSList);
            count++;
#endif

            if(de_radiolist->BSSList==NULL)
                return;
            for(bsslist_itr=0;bsslist_itr<(de_radiolist->numberOfBSS);bsslist_itr++){
                struct DE_BSSList_t*  de_bss_list;
                unsigned int stalist_itr=0;
//fill in DE
                unsigned int numberSTAs = MAX_CONNECTED_STAS;
                unsigned int numberRET=0;
                wifi_connected_sta_t conn_sta_data;
//fill in DE


                de_bss_list=(struct DE_BSSList_t*)malloc(sizeof(struct DE_BSSList_t));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_BSSList_t*)",de_bss_list);
                count++;
#endif

                if(de_bss_list==NULL)
                    return;
                *((struct DE_BSSList_t**)(de_radiolist->BSSList)+bsslist_itr)=de_bss_list;
                memset(de_bss_list,0,sizeof(struct DE_BSSList_t));
                de_bss_list->BSSID[0]=0xBB;
                strcpy(de_bss_list->BSSEnabled,"True");
                strcpy(de_bss_list->SSID,"TTEESSTTSSSSIIDD");
                //fill in DE
                {
                    struct wifi_current_stats_t curr_data;
                    memset(&curr_data,0,sizeof(struct wifi_current_stats_t));
                    wifi_get_current_stats(&curr_data);
                    if(curr_data.apsta == 1){
                        strcpy(de_bss_list->BSSID,curr_data.MACaddress);
                        strcpy(de_bss_list->SSID,curr_data.SSID);
                    }
                }
                //fill in DE

                de_bss_list->lastChange=0;
                de_bss_list->unicastBytesSent=0;
                de_bss_list->unicastBytesReceived=0;
                de_bss_list->multicastBytesSent=0;
                de_bss_list->multicastBytesReceived=0;
                de_bss_list->broadcastBytesSent=0;
                de_bss_list->broadcastBytesReceived=0;
                de_bss_list->estServiceParamBE[0]=0;
                de_bss_list->estServiceParamBK[0]=0;
                de_bss_list->estServiceParamVI[0]=0;
                de_bss_list->estServiceParamVO[0]=0;
                de_bss_list->numberOfSTA=1;
//fill in DE
                conn_sta_data.address = malloc(sizeof(CONNECTED_MAC_ADDRESS) * numberSTAs);
                numberRET = wifi_get_connected_sta_list(numberSTAs,conn_sta_data.address);
                de_bss_list->numberOfSTA=numberRET;
// fill in DE
                de_bss_list->staList=(struct DE_STAList_t**)malloc((de_bss_list->numberOfSTA)*sizeof(struct DE_STAList_t*));
#ifdef MEM_DEBUG
                printf("MALLOC%d %s %p\n",count,"(struct DE_STAList_t**)",de_bss_list->staList);
                count++;
#endif

                if(de_bss_list->staList==NULL)
                    return;
                for(stalist_itr=0;stalist_itr<(de_bss_list->numberOfSTA);stalist_itr++){
                    struct DE_STAList_t*  de_sta_list;

                    de_sta_list=(struct DE_STAList_t*)malloc(sizeof(struct DE_STAList_t));
#ifdef MEM_DEBUG
                    printf("MALLOC%d %s %p\n",count,"(struct DE_STAList_t*)",de_sta_list);
                    count++;
#endif
                    if(de_sta_list==NULL)
                        return;
                    *((struct DE_STAList_t**)(de_bss_list->staList)+stalist_itr)=de_sta_list;
                    memset(de_sta_list,0,sizeof(struct DE_STAList_t));
                    de_sta_list->MACaddress[0]=0xcc;
//fill in DE
                    strcpy(de_sta_list->MACaddress,conn_sta_data.address[stalist_itr]);
//fill in DE
                    de_sta_list->HTCapabilities[0]=0x12;
                    de_sta_list->VHTCapabilities[0]=0xFE;
                    de_sta_list->VHTCapabilities[1]=0xDC;
                    de_sta_list->VHTCapabilities[2]=0xBA;
                    de_sta_list->VHTCapabilities[3]=0x98;
                    de_sta_list->VHTCapabilities[4]=0x76;
                    de_sta_list->VHTCapabilities[5]=0x54;
                    de_sta_list->HECapabilities[0]=0x12;
                    de_sta_list->HECapabilities[1]=0x34;
                    de_sta_list->HECapabilities[2]=0x56;
                    de_sta_list->HECapabilities[3]=0x78;
                    de_sta_list->HECapabilities[4]=0x9a;
                    de_sta_list->HECapabilities[5]=0xbc;
                    de_sta_list->HECapabilities[6]=0xde;
                    de_sta_list->HECapabilities[7]=0xf0;
                    de_sta_list->HECapabilities[8]=0x12;
                    de_sta_list->HECapabilities[9]=0x34;
                    de_sta_list->HECapabilities[10]=0x56;
                    de_sta_list->HECapabilities[11]=0x78;
                    de_sta_list->HECapabilities[12]=0x9a;
                    de_sta_list->HECapabilities[13]=0xbc;

                    de_sta_list->statusCode=0;
                    de_sta_list->signalStrength=0;
                    de_sta_list->lastConnectTime=0;
                    de_sta_list->lastDataDownlinkRate=0;
                    de_sta_list->lastDataUplinkRate=0;
                    de_sta_list->estMACDataRateDownlink=0;
                    de_sta_list->estMACDataRateUplink=0;
                    de_sta_list->utilizationReceive=44;
                    de_sta_list->utilizationTransmit=33;
                    de_sta_list->bytesSent=0x11;
                    de_sta_list->bytesReceived=0x22;
                    de_sta_list->packetsSent=0x33;
                    de_sta_list->packetsReceived=0x44;
                    de_sta_list->errorsReceived=0x55;
                    de_sta_list->errorsSent=0x11;
                    de_sta_list->retransCount=0x01;
                    strcpy(de_sta_list->IPv6Address,"ffee:xx");
                    strcpy(de_sta_list->IPv4Address,"192.168.1.xx");
                    strcpy(de_sta_list->hostName,"STA1HOSTNAME");
                    de_sta_list->measurement[0]=0X77;

                }
            }


        }
    }
}
#endif

#ifdef WIFI_DE
int wifi_de_memory_management(struct wifi_dataelements_t* data)
{
    if(data->deviceList){
        unsigned int devitr=0;
        struct DE_devices_t* de_dev=NULL;
        for(devitr=0;devitr<(data->numberOfDevices);devitr++) {
            de_dev = *((struct DE_devices_t**)(data->deviceList)+devitr);
            if(de_dev) {
                if(de_dev->RadioList) {

                    unsigned int radioitr=0;
                    for(radioitr=0;radioitr<(de_dev->NumberOfRadios);radioitr++) {
                        struct DE_RadioList_t*            de_radiolist;
                        struct DE_Capabilites_t*        de_caps;
                        struct DE_BackHaulSTA_t*        de_bh_STA;
                        struct DE_ScanResultList_t*     de_scanresults_list;
                        unsigned int curr_oper_classes_itr=0;
                        unsigned int cap_oper_classes_itr=0;
                        unsigned int unassoc_sta_list_itr=0;
                        unsigned int scanresults_chann_list_itr=0;
                        unsigned int bsslist_itr=0;

                        de_radiolist=*((struct DE_RadioList_t**)(de_dev->RadioList)+radioitr);
                        if(de_radiolist) {
                            if(de_radiolist->currentOperatingClasses){

                                for(curr_oper_classes_itr=0;curr_oper_classes_itr<(de_radiolist->numberOfCurrentOperatingClasses);curr_oper_classes_itr++){
                                    struct DE_CurrentOperatingClasses_t*  de_currentoperatingclasses;
                                    de_currentoperatingclasses=*((struct DE_CurrentOperatingClasses_t**)(de_radiolist->currentOperatingClasses)+curr_oper_classes_itr);
                                    if(de_currentoperatingclasses) {
#ifdef MEM_DEBUG
                                        printf("FREE%d %s %p\n",count,"struct DE_CurrentOperatingClasses_t*",de_currentoperatingclasses);
                                        count--;
#endif
                                        free(de_currentoperatingclasses);
                                    }
                                }
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"(struct DE_CurrentOperatingClasses_t**)",de_radiolist->currentOperatingClasses);
                                count--;
#endif
                                free(de_radiolist->currentOperatingClasses);
                            }
                            if(de_radiolist->capabilities){
                                de_caps=de_radiolist->capabilities;
                                if(de_caps->operatingClasses){
                                    for(cap_oper_classes_itr=0;cap_oper_classes_itr<(de_caps->numberOfOperatingClasses);cap_oper_classes_itr++){
                                        struct DE_OperatingClasses_t*  de_capoperatingclasses;
                                        unsigned int    cap_nonoperable_itr=0;
                                        de_capoperatingclasses=*((struct DE_OperatingClasses_t**)(de_caps->operatingClasses)+cap_oper_classes_itr);
                                        if(de_capoperatingclasses){
                                            if(de_capoperatingclasses->NonOperable){
#ifdef MEM_DEBUG
                                                printf("FREE%d %s %p\n",count,"de_capoperatingclasses->NonOperable",de_capoperatingclasses->NonOperable);
                                                count--;
#endif

                                                free(de_capoperatingclasses->NonOperable);
                                            }
#ifdef MEM_DEBUG
                                            printf("FREE%d %s %p\n",count,"struct DE_OperatingClasses_t*",de_capoperatingclasses);
                                            count--;
#endif
                                            free(de_capoperatingclasses);
                                        }
                                    }
#ifdef MEM_DEBUG
                                    printf("FREE%d %s %p\n",count,"struct DE_OperatingClasses_t**",de_caps->operatingClasses);
                                    count--;
#endif

                                    free(de_caps->operatingClasses);
                                }
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"struct DE_Capabilites_t*",de_radiolist->capabilities);
                                count--;
#endif
                                free(de_radiolist->capabilities);
                            }

                            if(de_radiolist->backhaulSTA){
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"struct DE_BackHaulSTA_t*",de_radiolist->backhaulSTA);
                                count--;
#endif
                                free(de_radiolist->backhaulSTA);
                            }


                            if(de_radiolist->unassociatedSTAList) {
                                struct DE_UnassociatedSTAList_t*  de_unassoc_sta_list;

                                for(unassoc_sta_list_itr=0;unassoc_sta_list_itr<(de_radiolist->numberOfunassociatedSTAList);unassoc_sta_list_itr++){
                                    de_unassoc_sta_list=*((struct DE_UnassociatedSTAList_t**)(de_radiolist->unassociatedSTAList)+unassoc_sta_list_itr);
                                    if(de_unassoc_sta_list){
#ifdef MEM_DEBUG
                                        printf("FREE%d %s %p\n",count,"struct DE_UnassociatedSTAList_t*",de_unassoc_sta_list);
                                        count--;
#endif
                                        free(de_unassoc_sta_list);
                                    }
                                }
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"struct DE_UnassociatedSTAList_t**",de_radiolist->unassociatedSTAList);
                                count--;
#endif
                                free(de_radiolist->unassociatedSTAList);
                            }

                            if(de_radiolist->scanResultList){
                                de_scanresults_list=de_radiolist->scanResultList;

                                if(de_scanresults_list->channelList){
                                    for(scanresults_chann_list_itr=0;scanresults_chann_list_itr<(de_scanresults_list->numberOfChannels);scanresults_chann_list_itr++){
                                        struct DE_ChannelScan_t*  de_scanresults_chan_list;
                                        unsigned int operatingclass_scanlist_itr=0;
                                        unsigned int neighborlist_itr=0;

                                        de_scanresults_chan_list=*((struct DE_ChannelScan_t**)(de_scanresults_list->channelList)+scanresults_chann_list_itr);
                                        if(de_scanresults_chan_list){
#ifdef MEM_DEBUG
                                            printf("FREE%d %s %p\n",count,"operatingclass",de_scanresults_chan_list->operatingClass);
                                            count--;
#endif
                                            free(de_scanresults_chan_list->operatingClass);

                                            if(de_scanresults_chan_list->neighborList){
                                                struct DE_NeighborList_t*  de_neighborlist;
                                                for(neighborlist_itr=0;neighborlist_itr<(de_scanresults_chan_list->numberOfNeighbors);neighborlist_itr++){
                                                    de_neighborlist=*((struct DE_NeighborList_t**)(de_scanresults_chan_list->neighborList)+neighborlist_itr);
                                                    if(de_neighborlist){
#ifdef MEM_DEBUG
                                                        printf("FREE%d %s %p\n",count,"struct DE_NeighborList_t*",de_neighborlist);
                                                        count--;
#endif
                                                        free(de_neighborlist);
                                                    }
                                                }
#ifdef MEM_DEBUG
                                                printf("FREE%d %s %p\n",count,"struct DE_NeighborList_t**",de_scanresults_chan_list->neighborList);
                                                count--;
#endif
                                                free(de_scanresults_chan_list->neighborList);
                                            }
#ifdef MEM_DEBUG
                                            printf("FREE%d %s %p\n",count,"(struct DE_ChannelScan_t*)",de_scanresults_chan_list);
                                            count--;
#endif
                                            free(de_scanresults_chan_list);
                                        }
                                    }
#ifdef MEM_DEBUG
                                    printf("FREE%d %s %p\n",count,"(struct DE_ChannelScan_t**)",de_scanresults_list->channelList);
                                    count--;
#endif
                                    free(de_scanresults_list->channelList);
                                }
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"struct DE_ScanResultList_t*",de_radiolist->scanResultList);
                                count--;
#endif
                                free(de_radiolist->scanResultList);
                            }

                            if(de_radiolist->BSSList){
                                for(bsslist_itr=0;bsslist_itr<(de_radiolist->numberOfBSS);bsslist_itr++){
                                    struct DE_BSSList_t*  de_bss_list;
                                    unsigned int stalist_itr=0;
                                    de_bss_list=*((struct DE_BSSList_t**)(de_radiolist->BSSList)+bsslist_itr);
                                    if(de_bss_list){
                                        if(de_bss_list->staList){

                                            for(stalist_itr=0;stalist_itr<(de_bss_list->numberOfSTA);stalist_itr++){
                                                struct DE_STAList_t*  de_sta_list;
                                                de_sta_list=*((struct DE_STAList_t**)(de_bss_list->staList)+stalist_itr);
                                                if(de_sta_list){
#ifdef MEM_DEBUG
                                                    printf("FREE%d %s %p\n",count,"(struct DE_STAList_t*)",de_sta_list);
                                                    count--;
#endif
                                                    free(de_sta_list);
                                                }
                                            }
#ifdef MEM_DEBUG
                                            printf("FREE%d %s %p\n",count,"(struct DE_STAList_t**)",de_bss_list->staList);
                                            count--;
#endif
                                            free(de_bss_list->staList);
                                        }
#ifdef MEM_DEBUG
                                        printf("FREE%d %s %p\n",count,"struct DE_BSSList_t*",de_bss_list);
                                        count--;
#endif
                                        free(de_bss_list);
                                    }
                                }
#ifdef MEM_DEBUG
                                printf("FREE%d %s %p\n",count,"struct DE_BSSList_t**",de_radiolist->BSSList);
                                count--;
#endif
                                free(de_radiolist->BSSList);
                            }
#ifdef MEM_DEBUG
                            printf("FREE%d %s %p\n",count,"struct DE_RadioList_t*",de_radiolist);
                            count--;
#endif
                            free(de_radiolist);
                        }
                    }
#ifdef MEM_DEBUG
                    printf("FREE%d %s %p\n",count,"struct DE_RadioList_t**",de_dev->RadioList);
                    count--;
#endif
                    free(de_dev->RadioList);
                }
#ifdef MEM_DEBUG
                printf("FREE%d %s %p\n",count,"struct DE_devices_t*",de_dev);
                count--;
#endif
                free(de_dev);
            }
        }
#ifdef MEM_DEBUG
        printf("FREE%d %s %p\n",count,"struct struct DE_devices_t**",data->deviceList);
        count--;
#endif
        free(data->deviceList);
    }
}
#endif
int wifi_get_data(const char * filter,char * data,size_t data_size)
{
    int            rc = 0;
    char           *payload;
    struct timeval tv = {0, 0};
    bmon_wifi_t wifi_data;
    enum bmon_wifi_command arg = ALL;
    const char * pFilter         = filter;
    int ret=-1;
    char tagval[256]={0};
    bmon_uri    uriParsed;

    payload = data;
    memset( payload, 0, data_size);
    memset( &wifi_data, 0, sizeof( wifi_data ));

    strncpy( wifi_data.plugin_version, WIFI_PLUGIN_VERSION, sizeof( wifi_data.plugin_version ) - 1 );
    bmon_uri_parse(filter,&uriParsed);
    for(int i=0;i<(sizeof(bmon_wifi_commands)/sizeof(bmon_wifi_commands[0]));i++) {
#ifdef WIFI_PLUGIN_DEBUG
        printf("%d..arg %s list %s\n",i ,uriParsed.path,bmon_wifi_commands[i]);
#endif
        if(strstr(uriParsed.path,bmon_wifi_commands[i])) {
            arg = i;
            break;
        }
    }
    gettimeofday( &tv, NULL );

    for(unsigned int j=0;j<(sizeof(bmon_wifi_set_commands)/sizeof(bmon_wifi_set_commands[0]));j++){
            ret=bmon_uri_find_tagvalue(filter,bmon_wifi_set_commands[j],tagval,sizeof(tagval));
            if (ret){
                arg=(sizeof(bmon_wifi_commands)/sizeof(bmon_wifi_commands[0]))+j;
        }
    }

     switch(arg) {
        case POWS_WINDOW_VALUE:{
            wifi_set_power_stats(atoi(tagval));
            rc = wifi_trigger_spectrum_data_convert_to_json( payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case CONNECTED_STA_LIST: {
            unsigned int numberSTAs = MAX_CONNECTED_STAS;
            unsigned int numberRET=0;
            wifi_connected_sta_t data;
            data.address = malloc(sizeof(CONNECTED_MAC_ADDRESS) * numberSTAs);
            numberRET = wifi_get_connected_sta_list(numberSTAs,data.address);
            data.numberSTAs=numberRET;
            rc = wifi_connected_sta_list_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
            free(data.address);
        }
        break;
        case SPECTRUM_ANALYSIS_TRIGGER: {
            wifi_trigger_spectrum_data();
            rc = wifi_trigger_spectrum_data_convert_to_json( payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case SPECTRUM_ANALYSIS: {
            struct spectrum_data_t *data=NULL;
            unsigned int num=0;
            wifi_get_spectrum_data(&data,&num);
            rc = wifi_spectrum_data_convert_to_json( data, num, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
#ifdef WIFI_PLUGIN_DEBUG
            for(int i=0;i<num;i++) {
                printf("[%d] SSID: \"%s\"\n\tBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n\tRSSI: %d db\n\tNoise: %d\n\t"
                        "Chan: %d\n\t PrimChan: %d\n\tBandwidth: %d\n\tRate: %d\n\tModes: %d\n", i,data[i].SSID,
                        data[i].octet[0], data[i].octet[1], data[i].octet[2],
                        data[i].octet[3], data[i].octet[4], data[i].octet[5],
                        data[i].lRSSI, data[i].ulPhyNoise, data[i].ulChan, data[i].ulPrimChan,
                        data[i].ulBandwidth, data[i].lRate, data[i].ul802_11Modes);
                sleep(2);
            }
#endif
#ifdef MEM_DEBUG
            printf("FREE%d %s %p\n",count,"struct spectrum_data_t",data);
            count--;
#endif
            if(data)
                free(data);
        }
        break;
        case TX_PACKETS_STATS: {
            struct wifi_tx_stats_t data;
            memset(&data,0,sizeof(struct wifi_tx_stats_t));
            wifi_get_tx_stats(&data);
            rc = wifi_txpkt_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case RX_PACKETS_STATS: {
            struct wifi_rx_stats_t data;
            memset(&data,0,sizeof(struct wifi_rx_stats_t));
            wifi_get_rx_stats(&data);
            rc = wifi_rxpkt_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case TX_AMPDU_STATS: {
            struct wifi_txampdu_stats_t data;
            memset(&data,0,sizeof(struct wifi_txampdu_stats_t));
            wifi_get_txampdu_stats(&data);
            rc = wifi_txampdu_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case RX_AMPDU_STATS: {
            struct wifi_rxampdu_stats_t data;
            memset(&data,0,sizeof(struct wifi_rxampdu_stats_t));
            wifi_get_rxampdu_stats(&data);
            rc = wifi_rxampdu_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case AMPDU_CHART: {
            struct wifi_ampdu_chart_t data;
            memset(&data,0,sizeof(struct wifi_ampdu_chart_t));
            wifi_get_ampdu_chart(&data);
            rc = wifi_ampdu_chart_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case IQ_SAMPLES: {
            wifi_sa_params_t params;
            wifi_psd_t data;
            memset(&params, 0, sizeof(params));
            wifi_sa_convert_url_to_params((char*)filter, &params);
            if (wifi_sa_params_validate(&params)) {
                wifi_get_sa_data(&params, &data);
                rc = wifi_sa_convert_to_json(&data, payload, sizeof(payload),
                    tv.tv_sec, tv.tv_usec);
            }
        }
        break;
        case HELP:{
            unsigned int size = (sizeof(bmon_wifi_commands)/sizeof(bmon_wifi_commands[0]));
            rc = wifi_command_list_convert_to_json( bmon_wifi_commands,size, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter);
        }
        break;
#ifdef WIFI_DE
        case WIFI_DE_STATS: {
            struct wifi_dataelements_t data;
            memset(&data,0,sizeof(struct wifi_dataelements_t));
            wifi_get_dataelements_stats(&data);
            rc = wifi_de_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0,pFilter);
            wifi_de_memory_management(&data);
        }
        break;
#endif
        case CURRENT_STATS: {
            struct wifi_current_stats_t data;
            memset(&data,0,sizeof(struct wifi_current_stats_t));
            wifi_get_current_stats(&data);
            rc = wifi_current_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case POWER_STATS: {
            struct wifi_power_stats_t data;
            memset(&data,0,sizeof(struct wifi_power_stats_t));
            wifi_get_power_stats(&data);
            rc = wifi_power_stats_convert_to_json( &data, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
        }
        break;
        case CHANIM_STATS: {
            uint32_t num;
            struct wifi_chanim_stats_t* data;
            memset(&data,0,sizeof(struct wifi_chanim_stats_t));
            wifi_get_chanim_count(&num);
            data = malloc(sizeof(wifi_chanim_stats_t) * num);
            wifi_get_chanim_stats(data, num);
            rc = wifi_chanim_stats_convert_to_json( data, num, payload, data_size, tv.tv_sec, tv.tv_usec, 0, pFilter,NULL);
            free(data);
        }
        break;
        case ALL:{
            struct spectrum_data_t *spec_data=NULL;
            unsigned int num, chanim_count = 0;
            unsigned int numberSTAs = MAX_CONNECTED_STAS;
            unsigned int numberRET=0;

            struct wifi_tx_stats_t txstat_data;
            struct wifi_rx_stats_t rxstat_data;
            struct wifi_txampdu_stats_t txampdu_data;
            struct wifi_rxampdu_stats_t rxampdu_data;
            struct wifi_ampdu_chart_t ampdu_data;
            struct wifi_current_stats_t currstat_data;
            struct wifi_power_stats_t power_data;
            struct wifi_chanim_stats_t* chanim_data= NULL;
            struct wifi_connected_sta_t connected_data;
            if(json_CheckFilter(NULL,filter,NULL,"spectrumData"))
                wifi_get_spectrum_data(&spec_data,&num);
            if(json_CheckFilter(NULL,filter,NULL,"transmitStatistics"))
                wifi_get_tx_stats(&txstat_data);
            if(json_CheckFilter(NULL,filter,NULL,"receiveStatistics"))
                wifi_get_rx_stats(&rxstat_data);
            if(json_CheckFilter(NULL,filter,NULL,"transmitAMPDUStatistcs"))
                wifi_get_txampdu_stats(&txampdu_data);
            if(json_CheckFilter(NULL,filter,NULL,"receiveAMPDUStatistcs"))
                wifi_get_rxampdu_stats(&rxampdu_data);
            if(json_CheckFilter(NULL,filter,NULL,"ampduData"))
                wifi_get_ampdu_chart(&ampdu_data);
            if(json_CheckFilter(NULL,filter,NULL,"currentStats")){
                wifi_get_current_stats(&currstat_data);
            }

            if(json_CheckFilter(NULL,filter,NULL,"powerStats")){
                wifi_get_power_stats(&power_data);
            }
            if(json_CheckFilter(NULL,filter,NULL,"connectedSTAs")) {
                if(currstat_data.apsta == 1) {
                    connected_data.address = malloc(sizeof(CONNECTED_MAC_ADDRESS) * numberSTAs);
                    numberRET = wifi_get_connected_sta_list(numberSTAs,connected_data.address);
                    connected_data.numberSTAs=numberRET;
                }
            }
            if(json_CheckFilter(NULL,filter,NULL,"chanimStats")){
                wifi_get_chanim_count(&chanim_count);
                if(chanim_count>0) {
                    chanim_data = malloc(sizeof(wifi_chanim_stats_t) * chanim_count);
                    memset(chanim_data,0,sizeof(wifi_chanim_stats_t) * chanim_count);
                    wifi_get_chanim_stats(chanim_data, chanim_count);
                }
            }
            rc = wifi_all_convert_to_json(spec_data,num,&txstat_data,&rxstat_data,&txampdu_data,&rxampdu_data,&ampdu_data, &currstat_data,
                    &power_data, chanim_data, chanim_count, &connected_data, payload, data_size, tv.tv_sec, tv.tv_usec, pFilter);
            if(chanim_data)
                free(chanim_data);
            if(spec_data)
                free(spec_data);
            if(connected_data.address)
                free(connected_data.address);
        }
        break;
    }
#ifdef cJSON_FORMAT
#else
    if (rc)
    {
        free(payload);
        return -1;
    }
#endif
error:
    return( 0 );
}
#if defined ( BMON_PLUGIN )
int main(
    int   argc,
    char *argv[]
    )
{
    int    rc              = 0;
    char   filterDefault[] = "/";
    char * pFilter         = filterDefault;
    char   payload[PAYLOAD_SIZE];
    long int       bytes = 0;
    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = wifi_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving wifi output data from Nexus", rc, error);

    /* send response back to user */
    bytes = printf( "%s\n", payload );
    fflush(stdout);
    if (bytes <= 0)
    {
        sprintf( payload, "printf(payload) Failed ... (%s) ... tried (%s)\n", strerror( errno ), payload );
        goto error;
    }
error:
    return(rc);
}

#endif /* #if defined(BMON_PLUGIN) */
