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

/* Coomon file to be shared by plugin and dms parser*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "wifi_convert.h"
#include "bmon_utils.h"

int wifi_convert_to_xml(
    const char     *filter,
    bmon_wifi_t *wifi_out,
    char           *output,
    unsigned int    output_size
    )
{
    return 0;
}
int wifi_timestamp_convert_json(
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec)
{
    time_t currentDateTime;
    char  *pctime = NULL;
    char   one_line[128];

    snprintf( one_line, sizeof( one_line ), "{\"plugin_name\":\"wifi\",\"timestamp\":\"%ld.%06ld\"",
        (unsigned long int) tv_sec, (unsigned long int) tv_usec );
    strncat( payload, one_line, payload_size );

    currentDateTime = (time_t) tv_sec;
    pctime          = ctime( &currentDateTime );
    bmon_trim_line( pctime );
    snprintf( one_line, sizeof( one_line ), ",\"datetime\":\"%s\",\"plugin_version\":\"%s\",\"data\":{", pctime, WIFI_PLUGIN_VERSION );
    strncat( payload, one_line, payload_size );
    return 0;
}
#ifdef cJSON_FORMAT
int wifi_chanim_stats_convert_to_json(
    struct wifi_chanim_stats_t *data,
    unsigned int    count,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char *    filter,
    cJSON *         parentobj
)
{
    int         ii, rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;
    cJSON *     object_chanimStats = NULL;
    cJSON *     object_chanItem = NULL;

    if(inc == 0)    {
        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);
        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_chanimStats = json_AddArray(objectData, NO_FILTER, objectRoot, "chanimStats");
        CHECK_PTR_ERROR_GOTO("Failure adding chanimStats Output JSON object", object_chanimStats, rc, -1, error);
    }
    else {
        object_chanimStats = json_AddArray(parentobj, filter, objectRoot, "chanimStats");
        CHECK_PTR_ERROR_GOTO("Failure adding chanimStats Output JSON object", object_chanimStats, rc, -1, error);
    }
    if(data){
        for (ii = 0; ii < count; ii++) {
            object_chanItem = json_AddObject(object_chanimStats, filter, objectData, "chanItem");
            CHECK_PTR_ERROR_GOTO("Failure adding chanItem Output JSON object", object_chanimStats, rc, -1, error);
            json_AddNumber(object_chanItem, filter, objectData, "bgNoise", data[ii].bgnoise);
            json_AddNumber(object_chanItem, filter, objectData, "channelSpec", data[ii].chanspec);
            json_AddNumber(object_chanItem, filter, objectData, "channelNum", data[ii].channum);
            json_AddNumber(object_chanItem, filter, objectData, "txDuration", data[ii].tx);
            json_AddNumber(object_chanItem, filter, objectData, "rxInBss", data[ii].ibss);
            json_AddNumber(object_chanItem, filter, objectData, "rxOutBss", data[ii].obss);
            json_AddNumber(object_chanItem, filter, objectData, "mediumBusy", data[ii].busy);
            json_AddNumber(object_chanItem, filter, objectData, "mediumIdle", data[ii].chan_idle);
            json_AddNumber(object_chanItem, filter, objectData, "mediumAvail", data[ii].avail);
            json_AddNumber(object_chanItem, filter, objectData, "mediumNonWifi", data[ii].nwifi);
#if CCASTATS_EX
            json_AddNumber(object_chanItem, filter, objectData, "bphyBadPlcp", data[ii].bphy_badplcp);
            json_AddNumber(object_chanItem, filter, objectData, "bphyGlitchCount", data[ii].bphy_glitchcnt);
            json_AddNumber(object_chanItem, filter, objectData, "glitchCount", data[ii].glitcht);
            json_AddNumber(object_chanItem, filter, objectData, "badPlcp", data[ii].badplcp);
#endif
        }
    }
error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_chanim_stats_convert_to_json(
    struct wifi_chanim_stats_t *data,
    unsigned int    chanim_count,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
)
{
    struct wifi_chanim_stats_t* pItem = data;
    char   one_line[128];
    int   num_bytes = 0, ii;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }

    strncat( payload, "\"chanimStats\":[", payload_size );
    for (ii = 0; ii < chanim_count; ii++) {
            strncat( payload, "{ ", payload_size );
            snprintf( one_line, sizeof( one_line ), "\"bgNoise\":\"%d\"",pItem[ii].bgnoise);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelSpec\":\"%d\"",pItem[ii].chanspec);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelNum\":\"%d\"",pItem[ii].channum);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelIdle\":\"%d\"",pItem[ii].chan_idle);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"txDuration\":\"%d\"",pItem[ii].tx);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"rxInBss\":\"%d\"",pItem[ii].ibss);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"rxOutBss\":\"%d\"",pItem[ii].obss);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"mediumBusy\":\"%d\"",pItem[ii].busy);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"mediumIdle\":\"%d\"",pItem[ii].chan_idle);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"mediumNonWifi\":\"%d\"",pItem[ii].nwifi);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"mediumAvail\":\"%d\"",pItem[ii].avail);
            strncat( payload, one_line, payload_size );
#if CCASTATS_EX
            snprintf( one_line, sizeof( one_line ), ",\"bphyBadPlcp\":\"%d\"",pItem[ii].bphy_badplcp);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"bphyGlitchCount\":\"%d\"",pItem[ii].bphy_glitchcnt);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), "\"glitchCount\":\"%d\"",pItem[ii].glitchcnt);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"badPlcp\":\"%d\"",pItem[ii].badplcp);
            strncat( payload, one_line, payload_size );
#endif
            strncat( payload, " }", payload_size );
            if(ii < chanim_count-1) {
                strncat( payload, " ,", payload_size );
            }
    }
    strncat( payload, " ]", payload_size );

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}

#endif

#ifdef cJSON_FORMAT
int wifi_connected_sta_list_convert_to_json(
    wifi_connected_sta_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_connectedSta = NULL;
    cJSON *     object_Macaddr = NULL;

    if(inc == 0)    {

        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_connectedSta = json_AddObject(objectData, NO_FILTER, objectData, "connectedSTAs");
        CHECK_PTR_ERROR_GOTO("Failure adding connectedSTAs Output JSON object", object_connectedSta, rc, -1, error);
    }
    else {
        object_connectedSta = json_AddObject(objectData, filter, objectData, "connectedSTAs");
        CHECK_PTR_ERROR_GOTO("Failure adding connectedSTAs Output JSON object", object_connectedSta, rc, -1, error);
    }

    object_Macaddr = json_AddArray(object_connectedSta, filter, objectData, "macAddr");
    CHECK_PTR_ERROR_GOTO("Failure adding macAddr Output JSON object", object_Macaddr, rc, -1, error);
    for(int i=0;i<data->numberSTAs;i++)
    {
       json_AddString(object_Macaddr, filter, objectData, NULL, data->address[i]);
    }


error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_connected_sta_list_convert_to_json(
    wifi_connected_sta_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    char   rate[40];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    strncat( payload, "\"connectedSTAs\":[", payload_size );
    if(data) {
        for(int i = 0;i<data->numberSTAs;i++) {
            if(i!=0)
                strncat( payload, ", ", payload_size );
            snprintf( one_line, sizeof( one_line ), "\"%s\"",data->address[i]);
            strncat( payload, one_line, payload_size );
        }

    }
    strncat( payload, " ]", payload_size );

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}


#endif
#ifdef cJSON_FORMAT
int wifi_power_stats_convert_to_json(
    struct wifi_power_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char *    filter,
    cJSON *         parentobj
)
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_POWSWindow = NULL;
    cJSON *     object_test = NULL;

    if(inc == 0)    {

        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_POWSWindow = json_AddObject(objectData, filter, objectData, "powerStats");
        CHECK_PTR_ERROR_GOTO("Failure adding powerStats Output JSON object", object_POWSWindow, rc, -1, error);
    } else {
        object_POWSWindow = json_AddObject(objectData, filter, objectData, "powerStats");
        CHECK_PTR_ERROR_GOTO("Failure adding powerStats Output JSON object", object_POWSWindow, rc, -1, error);
    }
    json_AddNumber(object_POWSWindow, filter, objectData, "POWSWindow", data->POWSWindow);

error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_power_stats_convert_to_json(
    struct wifi_power_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
)
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(data) {
        strncat( payload, "\"powerStats\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"POWSWindow\":\"%d\"",data->POWSWindow);
        strncat( payload, one_line, payload_size );

        strncat( payload, " }", payload_size );

    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}

#endif

#ifdef cJSON_FORMAT
int wifi_current_stats_convert_to_json(
    struct wifi_current_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char *    filter,
    cJSON *         parentobj
)
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_currentstats_stats = NULL;
    cJSON *     object_rssi_ant = NULL;
    char        one_line[128];

    if(inc == 0)    {

        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_currentstats_stats = json_AddObject(objectData, NO_FILTER, objectData, "currentStats");
        CHECK_PTR_ERROR_GOTO("Failure adding transmitStatistics Output JSON object", object_currentstats_stats, rc, -1, error);
    }
    else {
        object_currentstats_stats = json_AddObject(objectData, filter, objectData, "currentStats");
        CHECK_PTR_ERROR_GOTO("Failure adding current_stats Output JSON object", object_currentstats_stats, rc, -1, error);
    }


    json_AddString(object_currentstats_stats, filter, objectData, "SSID", data->SSID);
    json_AddString(object_currentstats_stats, filter, objectData, "driverVersion", data->driverVersion);
    json_AddString(object_currentstats_stats, filter, objectData, "MACaddress", data->MACaddress);
    json_AddNumber(object_currentstats_stats, filter, objectData, "deviceID", data->deviceID);
    json_AddNumber(object_currentstats_stats, filter, objectData, "RSSI", data->RSSI);

    object_rssi_ant = json_AddArray(object_currentstats_stats, filter, objectData, "phy_rssi_ant");
    CHECK_PTR_ERROR_GOTO("Failure adding current_stats Output JSON object", object_rssi_ant, rc, -1, error);
    if(data->phy_rssi_ant){
        for(int i=0;i<4;i++)
        {
            json_AddNumber(object_rssi_ant, filter, objectData, NULL, data->phy_rssi_ant[i]);
        }
    }
    json_AddNumber(object_currentstats_stats, filter, objectData, "phy_noise", data->phy_noise);
    json_AddNumber(object_currentstats_stats, filter, objectData, "wifi80211Modes", data->wifi80211Modes);

    json_AddNumber(object_currentstats_stats, filter, objectData, "channel", data->channel);
    json_AddNumber(object_currentstats_stats, filter, objectData, "primaryChannel", data->primaryChannel);
    json_AddNumber(object_currentstats_stats, filter, objectData, "locked", data->locked);
    json_AddNumber(object_currentstats_stats, filter, objectData, "WPS", data->WPS);
    json_AddNumber(object_currentstats_stats, filter, objectData, "rate", data->rate);
    json_AddString(object_currentstats_stats, filter, objectData, "bandwidth", data->bandwidth);
    json_AddString(object_currentstats_stats, filter, objectData, "channelSpec", data->channelSpec);
    json_AddString(object_currentstats_stats, filter, objectData, "NRate", data->NRate);
    json_AddNumber(object_currentstats_stats, filter, objectData, "chipNumber", data->chipNumber);
    json_AddNumber(object_currentstats_stats, filter, objectData, "pciVendorID", data->pciVendorID);
    json_AddNumber(object_currentstats_stats, filter, objectData, "phyRate", data->phyRate);
    json_AddNumber(object_currentstats_stats, filter, objectData, "authenticationType", data->authenticationType);
    if(data->apsta ==0)
        json_AddString(object_currentstats_stats, filter, objectData, "deviceType", "STA");
    else
        json_AddString(object_currentstats_stats, filter, objectData, "deviceType", "AP");
error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_current_stats_convert_to_json(
    struct wifi_current_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
)
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(data) {
        strncat( payload, "\"currentStats\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"SSID\":\"%s\"",data->SSID);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"driverVersion\":\"%s\"",data->driverVersion);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"MACaddress\":\"%s\"",data->MACaddress);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"deviceID\":\"%d\"",data->deviceID);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"RSSI\":\"%d\"",data->RSSI);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"phy_rssi_ant\":\"[%d %d %d %d]\"",data->phy_rssi_ant[0],data->phy_rssi_ant[1],data->phy_rssi_ant[2],data->phy_rssi_ant[3] );
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"phy_noise\":\"%d\"",data->phy_noise);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"wifi80211Modes\":\"%d\"",data->wifi80211Modes);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"channel\":\"%d\"",data->channel);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"primaryChannel\":\"%d\"",data->primaryChannel);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"locked\":\"%d\"",data->locked);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"WPS\":\"%d\"",data->WPS);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"rate\":\"%d\"",data->rate);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"bandwidth\":\"%s\"",data->bandwidth);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"channelSpec\":\"%s\"",data->channelSpec);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"NRate\":\"%s\"",data->NRate);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"chipNumber\":\"%d\"",data->chipNumber);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"pciVendorID\":\"%d\"",data->pciVendorID);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"phyRate\":\"%d\"",data->phyRate);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"authenticationType\":\"%d\"",data->authenticationType);
        strncat( payload, one_line, payload_size );
        if(data->apsta == 0){
            snprintf( one_line, sizeof( one_line ), ",\"deviceType\":\"%s\"","STA");
            strncat( payload, one_line, payload_size );
        }
        else {
            snprintf( one_line, sizeof( one_line ), ",\"deviceType\":\"%s\"","AP");
            strncat( payload, one_line, payload_size );
        }
        strncat( payload, " }", payload_size );

    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}


#endif


#ifdef cJSON_FORMAT
int wifi_txpkt_stats_convert_to_json(
    struct wifi_tx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char *    filter,
    cJSON *         parentobj

    )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_txpkt_stats = NULL;

    if(inc == 0)    {

        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_txpkt_stats = json_AddObject(objectData, NO_FILTER, objectData, "transmitStatistics");
        CHECK_PTR_ERROR_GOTO("Failure adding transmitStatistics Output JSON object", object_txpkt_stats, rc, -1, error);
    }
    else {
        object_txpkt_stats = json_AddObject(objectData, filter, objectData, "transmitStatistics");
        CHECK_PTR_ERROR_GOTO("Failure adding transmitStatistics Output JSON object", object_txpkt_stats, rc, -1, error);
    }
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitFrames", data->txframes);
    json_AddNumber(object_txpkt_stats, filter, objectData, "retransmissions", data->txretrans);
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitErrors", data->txerror);
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitStatusErrors", data->txserr);
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitNoBuffer", data->txnobuf);
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitNotAssociated", data->txnoassoc);
    json_AddNumber(object_txpkt_stats, filter, objectData, "transmitRuntFrames", data->txrunt);

error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_txpkt_stats_convert_to_json(
    struct wifi_tx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(data) {
        strncat( payload, "\"transmitStatistics\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"transmitFrames\":\"%d\"",data->txframes);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"retransmissions\":\"%d\"",data->txretrans);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitErrors\":\"%d\"",data->txerror);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitStatusErrors\":\"%d\"",data->txserr);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitNoBuffer\":\"%d\"",data->txnobuf);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitNotAssociated\":\"%d\"",data->txnoassoc);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitRuntFrames\":\"%d\"",data->txrunt);
        strncat( payload, one_line, payload_size );
        strncat( payload, " }", payload_size );
    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
#ifdef cJSON_FORMAT
int wifi_rxpkt_stats_convert_to_json(
    struct wifi_rx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
        int         rc         = 0;
        cJSON *     objectRoot = NULL;
        cJSON *     objectData = parentobj;
        cJSON *     object_rxpkt_stats = NULL;

        if(inc == 0)    {
            objectRoot = json_Initialize();
            CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

            objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
            CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

            object_rxpkt_stats = json_AddObject(objectData, NO_FILTER, objectData, "receiveStatistics");
            CHECK_PTR_ERROR_GOTO("Failure adding receiveStatistics Output JSON object", object_rxpkt_stats, rc, -1, error);
        }
        else {
            object_rxpkt_stats = json_AddObject(objectData, filter, objectData, "receiveStatistics");
            CHECK_PTR_ERROR_GOTO("Failure adding receiveStatistics Output JSON object", object_rxpkt_stats, rc, -1, error);
        }
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveFrames", data->rxframe);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveErrors", data->rxerror);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveOutofBuffers", data->rxnobuf);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveNonData", data->rxnondata);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveBadDSError", data->rxbadds);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveBadControlManagementFrame", data->rxbadcm);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveFragmentationErrors", data->rxfragerr);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveRuntFrames", data->rxrunt);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveNoSCBError", data->rxnoscb);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveInvalidFrames", data->rxbadproto);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveInvalidSourceMAC", data->rxbadsrcmac);
        json_AddNumber(object_rxpkt_stats, filter, objectData, "receiveInvalidDA", data->rxbadda);

    error:
        if(inc == 0){
            rc = json_Print(objectRoot, payload, payload_size);
            CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

            json_Uninitialize(&objectRoot);

            if (0 <= rc)
                rc = strlen(payload);
        return(rc);
        }

}

#else
int wifi_rxpkt_stats_convert_to_json(
    struct wifi_rx_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }

    if(data) {
        strncat( payload, "\"receiveStatistics\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"receiveFrames\":\"%d\"",data->rxframe);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveErrors\":\"%d\"",data->rxerror);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveOutofBuffers\":\"%d\"",data->rxnobuf);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveNonData\":\"%d\"",data->rxnondata);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveBadDSError\":\"%d\"",data->rxbadds);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveBadControlManagementFrame\":\"%d\"",data->rxbadcm);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveFragmentationErrors\":\"%d\"",data->rxfragerr);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveRuntFrames\":\"%d\"",data->rxrunt);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveNoSCBError\":\"%d\"",data->rxnoscb);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveInvalidFrames\":\"%d\"",data->rxbadproto);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveInvalidSourceMAC\":\"%d\"",data->rxbadsrcmac);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveInvalidDA\":\"%d\"",data->rxbadda);
        strncat( payload, one_line, payload_size );
        strncat( payload, "}", payload_size );
    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
#ifdef cJSON_FORMAT
int wifi_rxampdu_stats_convert_to_json(
    struct wifi_rxampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
        int         rc         = 0;
        cJSON *     objectRoot = NULL;
        cJSON *     objectData = parentobj;
        cJSON *     object_rxampdu_stats = NULL;

        if(inc == 0)    {
            objectRoot = json_Initialize();
            CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

            objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
            CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

            object_rxampdu_stats = json_AddObject(objectData, NO_FILTER, objectData, "receiveAMPDUStatistcs");
            CHECK_PTR_ERROR_GOTO("Failure adding receiveAMPDUStatistcs Output JSON object", object_rxampdu_stats, rc, -1, error);
        }
        else {
            object_rxampdu_stats = json_AddObject(objectData, filter, objectData, "receiveAMPDUStatistcs");
            CHECK_PTR_ERROR_GOTO("Failure adding receiveAMPDUStatistcs Output JSON object", object_rxampdu_stats, rc, -1, error);
        }
        json_AddNumber(object_rxampdu_stats, filter, objectData, "receiveAMPDUFrames", data->rxampdu);
        json_AddNumber(object_rxampdu_stats, filter, objectData, "receiveMPDUFrames", data->rxmpdu);
        json_AddNumber(object_rxampdu_stats, filter, objectData, "receiveMPDUperAMPDU", data->rxmpduperampdu);
        json_AddNumber(object_rxampdu_stats, filter, objectData, "receiveBAR", data->rxbar);
        json_AddNumber(object_rxampdu_stats, filter, objectData, "transmitBA", data->txba);

    error:
        if(inc == 0){
            rc = json_Print(objectRoot, payload, payload_size);
            CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

            json_Uninitialize(&objectRoot);

            if (0 <= rc)
                rc = strlen(payload);
        return(rc);
        }
}

#else
int wifi_rxampdu_stats_convert_to_json(
    struct wifi_rxampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(data) {
        strncat( payload, "\"receiveAMPDUStatistcs\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"receiveAMPDUFrames\":\"%d\"",data->rxampdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveMPDUFrames\":\"%d\"",data->rxmpdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveMPDUperAMPDU\":\"%d\"",data->rxmpduperampdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveBAR\":\"%d\"",data->rxbar);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitBA\":\"%d\"",data->txba);
        strncat( payload, one_line, payload_size );
        strncat( payload, "}", payload_size );
    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
#ifdef cJSON_FORMAT
int wifi_txampdu_stats_convert_to_json(
    struct wifi_txampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
        int         rc         = 0;
        cJSON *     objectRoot = NULL;
        cJSON *     objectData = parentobj;
        cJSON *     object_txampdu_stats = NULL;

        if(inc == 0)    {
            objectRoot = json_Initialize();
            CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

            objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
            CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

            object_txampdu_stats = json_AddObject(objectData, NO_FILTER, objectData, "transmitAMPDUStatistcs");
            CHECK_PTR_ERROR_GOTO("Failure adding transmitAMPDUStatistcs Output JSON object", object_txampdu_stats, rc, -1, error);
        }
        else {
            object_txampdu_stats = json_AddObject(objectData, filter, objectData, "transmitAMPDUStatistcs");
            CHECK_PTR_ERROR_GOTO("Failure adding transmitAMPDUStatistcs Output JSON object", object_txampdu_stats, rc, -1, error);
        }
        json_AddNumber(object_txampdu_stats, filter, objectData, "transmitAMPDUFrames", data->txampdu);
        json_AddNumber(object_txampdu_stats, filter, objectData, "transmitMPDUFrames", data->txmpdu);
        json_AddNumber(object_txampdu_stats, filter, objectData, "transmitMPDUperAMPDU", data->txmpduperampdu);
        json_AddNumber(object_txampdu_stats, filter, objectData, "retryAMDPU", data->retry_ampdu);
        json_AddNumber(object_txampdu_stats, filter, objectData, "retryMPDU", data->retry_mpdu);
        json_AddNumber(object_txampdu_stats, filter, objectData, "trasmitBAR", data->txbar);
        json_AddNumber(object_txampdu_stats, filter, objectData, "receiveBA", data->rxba);

    error:
        if(inc == 0){
            rc = json_Print(objectRoot, payload, payload_size);
            CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

            json_Uninitialize(&objectRoot);

            if (0 <= rc)
                rc = strlen(payload);
        return(rc);
        }
}

#else
int wifi_txampdu_stats_convert_to_json(
    struct wifi_txampdu_stats_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(data) {
        strncat( payload, "\"transmitAMPDUStatistcs\":{", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"transmitAMPDUFrames\":\"%d\"",data->txampdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitMPDUFrames\":\"%d\"",data->txmpdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"transmitMPDUperAMPDU\":\"%d\"",data->txmpduperampdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"retryAMDPU\":\"%d\"",data->retry_ampdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"retryMPDU\":\"%d\"",data->retry_mpdu);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"trasmitBAR\":\"%d\"",data->txbar);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"receiveBA\":\"%d\"",data->rxba);
        strncat( payload, one_line, payload_size );
        strncat( payload, "}", payload_size );
    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
#ifdef cJSON_FORMAT
int wifi_ampdu_chart_convert_to_json(
    struct wifi_ampdu_chart_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_ampdu_chart = NULL;

    if(inc == 0)    {
        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);
        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_ampdu_chart = json_AddObject(objectData, NO_FILTER, objectData, "ampduData");
        CHECK_PTR_ERROR_GOTO("Failure adding ampduData Output JSON object", object_ampdu_chart, rc, -1, error);
    }
    else {
        object_ampdu_chart = json_AddObject(objectData, filter, objectData, "ampduData");
        CHECK_PTR_ERROR_GOTO("Failure adding ampduData Output JSON object", object_ampdu_chart, rc, -1, error);
    }

    if(data){
        int rxtx   = 0;
        int vhtsgi = 0;
        int ants   = 0;
        int mcs    = 0;
        char   one_line[128];

        for (rxtx = 0; rxtx<BMON_RX_TX; rxtx++)
        {
            cJSON * rxtxobj = NULL;
            char   rxtxitr[128];
            if (rxtx==0)
                snprintf( rxtxitr, sizeof( rxtxitr ), "tx",rxtx);
            else
                snprintf( rxtxitr, sizeof( rxtxitr ), "rx",rxtx);

            rxtxobj = json_AddArray(object_ampdu_chart, filter, objectData, rxtxitr);

            for (vhtsgi = 0; vhtsgi<BMON_VHT_VARIANTS; vhtsgi++)
            {
                cJSON * vhtsgiobj= NULL;
                char   vhtsgiitr[128];
                if(vhtsgi==0)
                    snprintf( vhtsgiitr, sizeof( vhtsgiitr ), "vht",vhtsgi);
                else
                    snprintf( vhtsgiitr, sizeof( vhtsgiitr ), "sgi",vhtsgi);
                vhtsgiobj = json_AddArray(rxtxobj, filter, objectData, vhtsgiitr);


                for (ants = 0; ants<BMON_NUM_ANTENNAS; ants++)
                {
                    cJSON * antsobj= NULL;
                    char   antsitr[128];
                    snprintf( antsitr, sizeof( antsitr ), "ant",ants);
                    antsobj = json_AddArray(vhtsgiobj, filter, objectData, antsitr);

                    for (mcs = 0; mcs<BMON_NUM_MCS; mcs++)
                    {
                        json_AddNumber(antsobj, filter, objectData, NULL, data->ampdu_data[rxtx][vhtsgi][ants][mcs]);

                    }
                }
            }
        }
    }
    error:
        if(inc == 0){
            rc = json_Print(objectRoot, payload, payload_size);
            CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

            json_Uninitialize(&objectRoot);

            if (0 <= rc)
                rc = strlen(payload);
        return(rc);
        }
}

#else
int wifi_ampdu_chart_convert_to_json(
    struct wifi_ampdu_chart_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }

    if(data) {
        int rxtx   = 0;
        int vhtsgi = 0;
        int ants   = 0;
        int mcs    = 0;

        strncat( payload, "\"ampduData\":[", payload_size );
        for (rxtx = 0; rxtx<BMON_RX_TX; rxtx++)
        {
            if (rxtx)
                strncat( payload, ",[", payload_size );
            else
                strncat( payload, "[", payload_size );

            for (vhtsgi = 0; vhtsgi<BMON_VHT_VARIANTS; vhtsgi++)
            {
                if (vhtsgi)
                    strncat( payload, ",", payload_size );
                strncat( payload, "[", payload_size );
                for (ants = 0; ants<BMON_NUM_ANTENNAS; ants++)
                {
                    if (ants)
                        strncat( payload, ",", payload_size );
                    if (ants == 0)
                        strncat( payload, "[ ", payload_size );
                    for (mcs = 0; mcs<BMON_NUM_MCS; mcs++)
                    {
                        if (mcs == 0)
                            strncat( payload, "[ ", payload_size );
                        if (mcs>0)
                            strncat( payload, ",", payload_size );
                        sprintf( one_line, "%d", data->ampdu_data[rxtx][vhtsgi][ants][mcs] );
                        strncat( payload, one_line, payload_size );
                        if (mcs == ( BMON_NUM_MCS-1 ))
                            strncat( payload, " ]", payload_size );
                    }
                    if (ants == 3)
                        strncat( payload, " ]", payload_size );
                }
                strncat( payload, " ]", payload_size );
            }
            strncat( payload, " ]", payload_size );
        }
    strncat( payload, " ]", payload_size );
    }

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
int wifi_command_list_convert_to_json(
    const char     *data[],
    unsigned int    num,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter
    )
{
    char   one_line[128];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }

    strncat( payload, "\"commandList\":{", payload_size );

    for(int i=0;i<num;i++){
        if (i!=0)
                strncat( payload, ",", payload_size );
        snprintf( one_line, sizeof( one_line ), "\"%d\":\"%s\"",i,data[i]);
        strncat( payload, one_line, payload_size );
    }
    strncat( payload, "}", payload_size );

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#ifdef cJSON_FORMAT
int wifi_trigger_spectrum_data_convert_to_json(
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = parentobj;
    cJSON *     object_default = NULL;

    if(inc == 0)    {

        objectRoot = json_Initialize();
        CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

        objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
        CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);
        object_default = json_AddObject(objectData, NO_FILTER, objectData, "default");
        CHECK_PTR_ERROR_GOTO("Failure adding default Output JSON object", object_default, rc, -1, error);
    }
    else {
        object_default = json_AddObject(objectData, filter, objectData, "default");
        CHECK_PTR_ERROR_GOTO("Failure adding default Output JSON object", object_default, rc, -1, error);
    }
error:
    if(inc == 0){
        rc = json_Print(objectRoot, payload, payload_size);
        CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

        json_Uninitialize(&objectRoot);

        if (0 <= rc)
            rc = strlen(payload);
        return(rc);
    }
}

#else
int wifi_trigger_spectrum_data_convert_to_json(
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    char   rate[40];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}

#endif

#ifdef cJSON_FORMAT
int wifi_spectrum_data_convert_to_json(
    struct spectrum_data_t *data,
    unsigned int    num,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{

        int         rc         = 0;
        cJSON *     objectRoot = NULL;
        cJSON *     objectData = parentobj;
        cJSON *     object_spectrum_data = NULL;
        cJSON *     spectrum_data_array = NULL;
        char        one_line[128];
        int         num_bytes = 0;

        if(inc == 0)    {
            objectRoot = json_Initialize();
            CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

            objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
            CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

            object_spectrum_data = json_AddArray(objectData, filter, objectData, "spectrumData");
            CHECK_PTR_ERROR_GOTO("Failure adding spectrumData Output JSON object", object_spectrum_data, rc, -1, error);
        }
        else {
            object_spectrum_data = json_AddArray(objectData, filter, objectData, "spectrumData");
            CHECK_PTR_ERROR_GOTO("Failure adding spectrumData Output JSON object", object_spectrum_data, rc, -1, error);
        }

        if(data){
                for(int i = 0;i<num;i++) {
                    char   stanum[128]={0};
                    char        rate[40]={0};

                    snprintf( stanum, sizeof( stanum ), "ap");

                    spectrum_data_array = json_AddObject(object_spectrum_data, filter, objectData, stanum);
                    CHECK_PTR_ERROR_GOTO("Failure adding object_spectrum_data Output JSON object", object_spectrum_data, rc, -1, error);

                    json_AddString(spectrum_data_array, filter,objectData, "SSID", data[i].SSID);
                    snprintf( one_line, sizeof( one_line ), "%02x:%02x:%02x:%02x:%02x:%02x",data[i].octet[0], data[i].octet[1], data[i].octet[2],
                            data[i].octet[3], data[i].octet[4], data[i].octet[5]);
                    strncat( payload, one_line, payload_size );
                    json_AddString(spectrum_data_array, filter, objectData, "BSSID", one_line);
                    json_AddNumber(spectrum_data_array, filter, objectData, "RSSI", data[i].lRSSI);
                    json_AddNumber(spectrum_data_array, filter, objectData, "phyNoise",data[i].ulPhyNoise);
                    json_AddNumber(spectrum_data_array, filter, objectData, "channelNum", data[i].ulChan);
                    json_AddNumber(spectrum_data_array, filter, objectData, "channelPrimNum", data[i].ulPrimChan);
                    json_AddNumber(spectrum_data_array, filter, objectData, "channelBandwidth", data[i].ulBandwidth);
                    json_AddNumber(spectrum_data_array, filter, objectData, "phyRate", data[i].lRate);

                    if(data[i].ul802_11Modes & wifi_ac)
                        strncat( rate, "ac/", sizeof(rate));
                    if(data[i].ul802_11Modes & wifi_n)
                        strncat( rate, "n/", sizeof(rate));
                    if(data[i].ul802_11Modes & wifi_g)
                        strncat( rate, "g/", sizeof(rate));
                    if(data[i].ul802_11Modes & wifi_b)
                        strncat( rate, "b/", sizeof(rate));
                    if(data[i].ul802_11Modes & wifi_a)
                        strncat( rate, "a/", sizeof(rate));
                    json_AddString(spectrum_data_array, filter, objectData, "operatingMode", rate);
            }
        }
    error:
        if(inc == 0){
            rc = json_Print(objectRoot, payload, payload_size);
            CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

            json_Uninitialize(&objectRoot);

            if (0 <= rc)
                rc = strlen(payload);
        return(rc);
        }
}

#else
int wifi_spectrum_data_convert_to_json(
    struct spectrum_data_t *data,
    unsigned int    num,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter,
    cJSON *         parentobj
    )
{
    char   one_line[128];
    char   rate[40];
    int   num_bytes = 0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }
    strncat( payload, "\"spectrumData\":[", payload_size );
    if(data) {
        for(int i = 0;i<num;i++) {
            strncat( payload, "{ ", payload_size );
            snprintf( one_line, sizeof( one_line ), "\"SSID\":\"%s\"",data[i].SSID);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"BSSID\":\"%02x:%02x:%02x:%02x:%02x:%02x\"",data[i].octet[0], data[i].octet[1], data[i].octet[2],
                    data[i].octet[3], data[i].octet[4], data[i].octet[5]);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"RSSI\":\"%d\"",data[i].lRSSI);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"phyNoise\":\"%d\"",data[i].ulPhyNoise);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelNum\":\"%d\"",data[i].ulChan);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelPrimNum\":\"%d\"",data[i].ulPrimChan);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"channelBandwidth\":\"%d\"",data[i].ulBandwidth);
            strncat( payload, one_line, payload_size );
            snprintf( one_line, sizeof( one_line ), ",\"phyRate\":\"%d\"",data[i].lRate);
            strncat( payload, one_line, payload_size );

            memset( rate, 0, sizeof( rate ));
            if(data[i].ul802_11Modes & wifi_ac)
                strncat( rate, "ac/", sizeof(rate));
            if(data[i].ul802_11Modes & wifi_n)
                strncat( rate, "n/", sizeof(rate));
            if(data[i].ul802_11Modes & wifi_g)
                strncat( rate, "g/", sizeof(rate));
            if(data[i].ul802_11Modes & wifi_b)
                strncat( rate, "b/", sizeof(rate));
            if(data[i].ul802_11Modes & wifi_a)
                strncat( rate, "a/", sizeof(rate));


            snprintf( one_line, sizeof( one_line ), ",\"operatingMode\":\"%s\"",rate);
            strncat( payload, one_line, payload_size );
            strncat( payload, " }", payload_size );
            if(i<num-1)
                strncat( payload, " ,", payload_size );
            }
    }
    strncat( payload, " ]", payload_size );

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}
#endif
#ifdef WIFI_DE
int wifi_de_convert_to_json(
    struct wifi_dataelements_t *data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    unsigned int    inc,
    const char*     filter
    )
{
    char   one_line[128];
    char   rate[40];
    int   num_bytes = 0;
    int     devitr=0;
    int     radioitr=0;
    int     curr_operating_class_itr=0;

    if(inc == 0) {
#if LOCAL_HEADER
        wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    }

    strncat( payload, "\"wfa-dataelements:Network\":{", payload_size );
    if(data) {
        snprintf( one_line, sizeof( one_line ), "\"ID\":\"%s\"",data->deID);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"ControllerID\":\"%s\"",data->controllerID);
        strncat( payload, one_line, payload_size );
        snprintf( one_line, sizeof( one_line ), ",\"NumberOfDevices\":\"%d\"",data->numberOfDevices);
        strncat( payload, one_line, payload_size );
        if (data->numberOfDevices>0) {
            if (data->deviceList) {
                snprintf( one_line, sizeof( one_line ), ",\"DeviceList\":[");
                strncat( payload, one_line, payload_size );
                for(int devitr = 0;devitr<(data->numberOfDevices);devitr++) {
                    struct DE_devices_t*    de_dev;
                    de_dev=*((struct DE_devices_t**)(data->deviceList)+devitr);
                    if (de_dev) {
                        strncat( payload, "{ ", payload_size );
                        snprintf( one_line, sizeof( one_line ), "\"ID\":\"%s\"",de_dev->devID);
                        strncat( payload, one_line, payload_size );
                        snprintf( one_line, sizeof( one_line ), ",\"NumberOfRadios\":\"%d\"",de_dev->NumberOfRadios);
                        strncat( payload, one_line, payload_size );
                        snprintf( one_line, sizeof( one_line ), ",\"MultiAPCapabilities\":\"%d\"",de_dev->MultiAPCap);
                        strncat( payload, one_line, payload_size );
                        if (de_dev->NumberOfRadios>0) {
                            if(de_dev->RadioList) {
                                snprintf( one_line, sizeof( one_line ), ",\"RadioList\":[");
                                strncat( payload, one_line, payload_size );
                                for(int radioitr = 0;radioitr<(de_dev->NumberOfRadios);radioitr++) {
                                    struct DE_RadioList_t*  de_radiolist;
                                    de_radiolist=*((struct DE_RadioList_t**)(de_dev->RadioList)+radioitr);
                                    if(de_radiolist) {
                                        strncat( payload, "{ ", payload_size );
                                        snprintf( one_line, sizeof( one_line ), "\"ID\":\"%s\"",de_radiolist->radioID);
                                        strncat( payload, one_line, payload_size );
                                        snprintf( one_line, sizeof( one_line ), ",\"Enabled\":\"%s\"",de_radiolist->radioEnabled);
                                        strncat( payload, one_line, payload_size );
                                        snprintf( one_line, sizeof( one_line ), ",\"Noise\":\"%d\"",de_radiolist->noise);
                                        strncat( payload, one_line, payload_size );
                                        snprintf( one_line, sizeof( one_line ), ",\"Utilization\":\"%d\"",de_radiolist->utilization);
                                        strncat( payload, one_line, payload_size );
                                        snprintf( one_line, sizeof( one_line ), ",\"UtilizationOther\":\"%d\"",de_radiolist->utilizationOther);
                                        strncat( payload, one_line, payload_size );

                                        if (de_radiolist->numberOfCurrentOperatingClasses>0) {
                                            if(de_radiolist->currentOperatingClasses) {
                                                snprintf( one_line, sizeof( one_line ), ",\"CurrentOperatingClasses\":[");
                                                strncat( payload, one_line, payload_size );
                                                for(int curr_operating_class_itr = 0;curr_operating_class_itr<(de_radiolist->numberOfCurrentOperatingClasses);curr_operating_class_itr++) {
                                                    struct DE_CurrentOperatingClasses_t*  de_curr_operating_class;
                                                    de_curr_operating_class=*((struct DE_CurrentOperatingClasses_t**)(de_radiolist->currentOperatingClasses)+curr_operating_class_itr);
                                                    if(curr_operating_class_itr>0)
                                                        strncat( payload, ",", payload_size );

                                                    strncat( payload, "{ ", payload_size );
                                                    snprintf( one_line, sizeof( one_line ), "\"Class\":\"%d\"",de_curr_operating_class->currentOperatingClass);
                                                    strncat( payload, one_line, payload_size );
                                                    snprintf( one_line, sizeof( one_line ), ",\"TxPower\":\"%d\"",de_curr_operating_class->channel);
                                                    strncat( payload, one_line, payload_size );
                                                    snprintf( one_line, sizeof( one_line ), ",\"Channel\":\"%d\"",de_curr_operating_class->maxTxPower);
                                                    strncat( payload, one_line, payload_size );
                                                    strncat( payload, " }", payload_size );
                                                }
                                                strncat( payload, " ]", payload_size );
                                            }
                                        }

                                        if (de_radiolist->capabilities) {
                                            struct DE_Capabilites_t* de_cap;
                                            unsigned int de_oper_class_itr=0;

                                            de_cap=((struct DE_Capabilites_t*)(de_radiolist->capabilities));
                                            snprintf( one_line, sizeof( one_line ), ",\"Capabilities\":{");
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), "\"HTCapabilities\":\"0x%x\"",de_cap->HTCapabilities[0]);
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), ",\"VHTCapabilities\":\"0x%x%x%x%x%x%x\"",de_cap->VHTCapabilities[0],
                                            de_cap->VHTCapabilities[1],de_cap->VHTCapabilities[2],de_cap->VHTCapabilities[3],
                                            de_cap->VHTCapabilities[4],de_cap->VHTCapabilities[5]);
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), ",\"HECapabilities\":\"0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\"",de_cap->HECapabilities[0],
                                                de_cap->HECapabilities[1],de_cap->HECapabilities[2],de_cap->HECapabilities[3],
                                                de_cap->HECapabilities[4],de_cap->HECapabilities[5],de_cap->HECapabilities[6],
                                                de_cap->HECapabilities[7],de_cap->HECapabilities[8],de_cap->HECapabilities[9],
                                                de_cap->HECapabilities[10],de_cap->HECapabilities[11],de_cap->HECapabilities[12],
                                                de_cap->HECapabilities[13]);
                                            strncat( payload, one_line, payload_size );

                                            if (de_cap->numberOfOperatingClasses>0) {
                                                if(de_cap->operatingClasses) {
                                                    snprintf( one_line, sizeof( one_line ), ",\"OperatingClasses\":[");
                                                    strncat( payload, one_line, payload_size );
                                                    for(int de_oper_class_itr = 0;de_oper_class_itr<(de_cap->numberOfOperatingClasses);de_oper_class_itr++) {
                                                        struct DE_OperatingClasses_t*  de_oper_class;
                                                        de_oper_class=*((struct DE_OperatingClasses_t**)(de_cap->operatingClasses)+de_oper_class_itr);
                                                        if(de_oper_class) {
                                                            if(de_oper_class_itr>0)
                                                                strncat( payload, ",", payload_size );
                                                            strncat( payload, "{ ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "\"Class\":\"%d\"",de_oper_class->operatingClass);
                                                            strncat( payload, one_line, payload_size );
                                                            snprintf( one_line, sizeof( one_line ), ",\"MaxTxPower\":\"%d\"",de_oper_class->MaxTxPower);
                                                            strncat( payload, one_line, payload_size );
                                                            snprintf( one_line, sizeof( one_line ), ",\"NonOperable\":");
                                                            strncat( payload, one_line, payload_size );

                                                            strncat( payload, "[", payload_size );
                                                                for(int i=0;i<de_oper_class->sizeOfNonOperable;i++){
                                                                if (i>0)
                                                                    strncat( payload, ", ", payload_size );
                                                                snprintf( one_line, sizeof( one_line ), "%d",de_oper_class->NonOperable[i]);
                                                                strncat( payload, one_line, payload_size );
                                                            }
                                                            strncat( payload, "]", payload_size );
                                                            strncat( payload, " }", payload_size );
                                                        }
                                                    }
                                                    strncat( payload, " ]", payload_size );
                                                }
                                            }
                                            strncat( payload, " }", payload_size );
                                        }
                                        if(de_radiolist->backhaulSTA)
                                        {
                                            struct DE_BackHaulSTA_t* de_bhsta;
                                            de_bhsta=((struct DE_BackHaulSTA_t*)(de_radiolist->backhaulSTA));
                                            snprintf( one_line, sizeof( one_line ), ",\"BackhaulSta\":{");
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), "\"MACAddress\":\"%x:%x:%x:%x:%x:%x\"",de_bhsta->backHaulSTA[0],
                                                de_bhsta->backHaulSTA[1],de_bhsta->backHaulSTA[2],de_bhsta->backHaulSTA[3],
                                                de_bhsta->backHaulSTA[4],de_bhsta->backHaulSTA[5]);
                                            strncat( payload, one_line, payload_size );
                                            strncat( payload, " }", payload_size );
                                        }

                                        if (de_radiolist->numberOfunassociatedSTAList>0) {
                                            if(de_radiolist->unassociatedSTAList) {
                                                int de_unassoc_sta_list_itr=0;
                                                snprintf( one_line, sizeof( one_line ), ",\"UnassociatedStaList\":[");
                                                strncat( payload, one_line, payload_size );

                                                for(int de_unassoc_sta_list_itr = 0;de_unassoc_sta_list_itr<(de_radiolist->numberOfunassociatedSTAList);de_unassoc_sta_list_itr++) {
                                                    struct DE_UnassociatedSTAList_t*  de_unassoc_sta_list;
                                                    de_unassoc_sta_list=*((struct DE_UnassociatedSTAList_t**)(de_radiolist->unassociatedSTAList)+de_unassoc_sta_list_itr);
                                                    if(de_unassoc_sta_list) {
                                                        if(de_unassoc_sta_list_itr>0)
                                                            strncat( payload, ",", payload_size );
                                                        strncat( payload, "{ ", payload_size );
                                                        snprintf( one_line, sizeof( one_line ), "\"MACAddress\":\"%x:%x:%x:%x:%x:%x\"",de_unassoc_sta_list->MACAddress[0],
                                                        de_unassoc_sta_list->MACAddress[1],de_unassoc_sta_list->MACAddress[2],de_unassoc_sta_list->MACAddress[3],
                                                        de_unassoc_sta_list->MACAddress[4],de_unassoc_sta_list->MACAddress[5]);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"SignalStrength\":\"%d\"",de_unassoc_sta_list->signalStrength);
                                                        strncat( payload, one_line, payload_size );
                                                        strncat( payload, " }", payload_size );
                                                    }
                                                }
                                                strncat( payload, " ]", payload_size );
                                            }
                                        }

                                        if(de_radiolist->scanResultList!=NULL)
                                        {
                                            struct DE_ScanResultList_t* de_scanresultslist;
                                            de_scanresultslist=((struct DE_ScanResultList_t*)(de_radiolist->scanResultList));
                                            snprintf( one_line, sizeof( one_line ), ",\"ScanResultList\":{");
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), "\"NumberOfChannels\":\"%d\"",de_scanresultslist->numberOfChannels);
                                            strncat( payload, one_line, payload_size );
                                            snprintf( one_line, sizeof( one_line ), ",\"LastScan\":\"%d\"",de_scanresultslist->lastScan);
                                            strncat( payload, one_line, payload_size );

                                            if (de_scanresultslist->numberOfChannels>0) {
                                                int de_scanresultschannellist=0;
                                                if(de_scanresultslist->channelList) {
                                                    snprintf( one_line, sizeof( one_line ), ",\"ChannelList\":[");
                                                    strncat( payload, one_line, payload_size );

                                                    for(int de_scanresultschannellist = 0;de_scanresultschannellist<(de_scanresultslist->numberOfChannels);de_scanresultschannellist++) {
                                                        struct DE_ChannelScan_t*  de_channelscan;
                                                        de_channelscan=*((struct DE_ChannelScan_t**)(de_scanresultslist->channelList)+de_scanresultschannellist);
                                                        if(de_channelscan) {
                                                            if(de_scanresultschannellist>0)
                                                                strncat( payload, ",", payload_size );
                                                            strncat( payload, "{ ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "\"Channel\":\"%d\"",de_channelscan->channel);
                                                            strncat( payload, one_line, payload_size );
                                                            snprintf( one_line, sizeof( one_line ), ",\"Utilization\":\"%d\"",de_channelscan->utilization);
                                                            strncat( payload, one_line, payload_size );
                                                            snprintf( one_line, sizeof( one_line ), ",\"OperatingClass\":");
                                                            strncat( payload, one_line, payload_size );
                                                            strncat( payload, "[", payload_size );
                                                            for(int i=0;i<de_channelscan->sizeofoperatingClass;i++){
                                                                if (i>0)
                                                                    strncat( payload, ", ", payload_size );
                                                                snprintf( one_line, sizeof( one_line ), "%d",de_channelscan->operatingClass[i]);
                                                                strncat( payload, one_line, payload_size );
                                                            }
                                                            strncat( payload, "]", payload_size );

                                                            snprintf( one_line, sizeof( one_line ), ",\"Noise\":\"%d\"",de_channelscan->noise);
                                                            strncat( payload, one_line, payload_size );
                                                            snprintf( one_line, sizeof( one_line ), ",\"NumberOfNeighbors\":\"%d\"",de_channelscan->numberOfNeighbors);
                                                            strncat( payload, one_line, payload_size );

                                                            if (de_channelscan->numberOfNeighbors>0) {
                                                                int de_scanresults_neighborlist=0;
                                                                if(de_channelscan->neighborList) {
                                                                    snprintf( one_line, sizeof( one_line ), ",\"NeighborList\":[");
                                                                    strncat( payload, one_line, payload_size );

                                                                    for(int de_scanresults_neighborlist = 0;de_scanresults_neighborlist<(de_channelscan->numberOfNeighbors);de_scanresults_neighborlist++) {
                                                                        struct DE_NeighborList_t*  de_neighbor_list;
                                                                        de_neighbor_list=*((struct DE_NeighborList_t**)(de_channelscan->neighborList)+de_scanresults_neighborlist);
                                                                        if(de_neighbor_list) {
                                                                            if(de_scanresults_neighborlist>0)
                                                                                strncat( payload, ",", payload_size );
                                                                            strncat( payload, "{ ", payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), "\"BSSID\":\"%x:%x:%x:%x:%x:%x\"",de_neighbor_list->BSSID[0],
                                                                                de_neighbor_list->BSSID[1],de_neighbor_list->BSSID[2],de_neighbor_list->BSSID[3],
                                                                                de_neighbor_list->BSSID[4],de_neighbor_list->BSSID[5]);
                                                                            strncat( payload, one_line, payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), ",\"SSID\":\"%s\"",de_neighbor_list->SSID);
                                                                            strncat( payload, one_line, payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), ",\"ChannelBandwidth\":\"%d\"",de_neighbor_list->signalStrength);
                                                                            strncat( payload, one_line, payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), ",\"StationCount\":\"%d\"",de_neighbor_list->channelBandwidth);
                                                                            strncat( payload, one_line, payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), ",\"SignalStrength\":\"%d\"",de_neighbor_list->stationCount);
                                                                            strncat( payload, one_line, payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), ",\"ChannelUtilization\":\"%d\"",de_neighbor_list->channelUtilization);
                                                                            strncat( payload, one_line, payload_size );
                                                                            strncat( payload, " }", payload_size );
                                                                        }
                                                                    }
                                                                    strncat( payload, " ]", payload_size );
                                                                }
                                                            }
                                                            strncat( payload, " }", payload_size );
                                                        }
                                                    }
                                                    strncat( payload, " ]", payload_size );
                                                }
                                            }
                                            strncat( payload, " }", payload_size );
                                        }
                                        snprintf( one_line, sizeof( one_line ), ",\"NumberOfBSS\":\"%d\"",de_radiolist->numberOfBSS);
                                        strncat( payload, one_line, payload_size );

                                        if (de_radiolist->numberOfBSS>0)
                                        {
                                            if(de_radiolist->BSSList) {
                                                int de_bsslist_itr=0;
                                                snprintf( one_line, sizeof( one_line ), ",\"BSSList\":[");
                                                strncat( payload, one_line, payload_size );

                                                for(int de_bsslist_itr = 0;de_bsslist_itr<(de_radiolist->numberOfBSS);de_bsslist_itr++) {
                                                    struct DE_BSSList_t*  de_bss_list;
                                                    de_bss_list=*((struct DE_BSSList_t**)(de_radiolist->BSSList)+de_bsslist_itr);
                                                    if(de_bss_list) {
                                                        if(de_bsslist_itr>0)
                                                            strncat( payload, ",", payload_size );
                                                        strncat( payload, "{ ", payload_size );
//                                                      snprintf( one_line, sizeof( one_line ), "\"BSSID\":\"%x:%x:%x:%x:%x:%x\"",de_bss_list->BSSID[0],
//                                                          de_bss_list->BSSID[1],de_bss_list->BSSID[2],de_bss_list->BSSID[3],
//                                                          de_bss_list->BSSID[4],de_bss_list->BSSID[5]);
                                                        snprintf( one_line, sizeof( one_line ), "\"BSSID\":\"%s\"",de_bss_list->BSSID);

                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"Enabled\":\"%s\"",de_bss_list->BSSEnabled);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"SSID\":\"%s\"",de_bss_list->SSID);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"LastChange\":\"%d\"",de_bss_list->lastChange);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"UnicastBytesSent\":\"%d\"",de_bss_list->unicastBytesSent);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"UnicastBytesReceived\":\"%d\"",de_bss_list->unicastBytesReceived);
                                                        strncat( payload, one_line, payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"MulticastBytesSent\":\"%d\"",de_bss_list->multicastBytesSent);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"MulticastBytesReceived\":\"%d\"",de_bss_list->multicastBytesReceived);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"BroadcastBytesSent\":\"%d\"",de_bss_list->broadcastBytesSent);
                                                        strncat( payload, one_line, payload_size );
                                                        snprintf( one_line, sizeof( one_line ), ",\"BroadcastBytesReceived\":\"%d\"",de_bss_list->broadcastBytesReceived);
                                                        strncat( payload, one_line, payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"EstServiceParametersBE\":");
                                                        strncat( payload, one_line, payload_size );
                                                        strncat( payload, "[", payload_size );
                                                        for(int i=0;i<4;i++){
                                                            if (i>0)
                                                                strncat( payload, ", ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "%d",de_bss_list->estServiceParamBE[i]);
                                                            strncat( payload, one_line, payload_size );
                                                        }
                                                        strncat( payload, "]", payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"EstServiceParametersBK\":");
                                                        strncat( payload, one_line, payload_size );
                                                        strncat( payload, "[", payload_size );
                                                        for(int i=0;i<4;i++){
                                                            if (i>0)
                                                                strncat( payload, ", ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "%d",de_bss_list->estServiceParamBK[i]);
                                                            strncat( payload, one_line, payload_size );
                                                        }
                                                        strncat( payload, "]", payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"EstServiceParametersVI\":");
                                                        strncat( payload, one_line, payload_size );
                                                        strncat( payload, "[", payload_size );
                                                        for(int i=0;i<4;i++){
                                                            if (i>0)
                                                                strncat( payload, ", ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "%d",de_bss_list->estServiceParamVI[i]);
                                                            strncat( payload, one_line, payload_size );
                                                        }
                                                        strncat( payload, "]", payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"EstServiceParametersVO\":");
                                                        strncat( payload, one_line, payload_size );
                                                        strncat( payload, "[", payload_size );
                                                        for(int i=0;i<4;i++){
                                                            if (i>0)
                                                                strncat( payload, ", ", payload_size );
                                                            snprintf( one_line, sizeof( one_line ), "%d",de_bss_list->estServiceParamVO[i]);
                                                            strncat( payload, one_line, payload_size );
                                                        }
                                                        strncat( payload, "]", payload_size );

                                                        snprintf( one_line, sizeof( one_line ), ",\"NumberOfSTA\":\"%d\"",de_bss_list->numberOfSTA);
                                                        strncat( payload, one_line, payload_size );

                                                        if (de_bss_list->numberOfSTA>0) {
                                                            int de_bss_sta_list_itr=0;
                                                            if(de_bss_list->staList) {
                                                                snprintf( one_line, sizeof( one_line ), ",\"STAList\":[");
                                                                strncat( payload, one_line, payload_size );

                                                                for(int de_bss_sta_list_itr = 0;de_bss_sta_list_itr<(de_bss_list->numberOfSTA);de_bss_sta_list_itr++) {
                                                                    struct DE_STAList_t*  de_bss_sta_list;
                                                                    de_bss_sta_list=*((struct DE_STAList_t**)(de_bss_list->staList)+de_bss_sta_list_itr);
                                                                    if(de_bss_sta_list){
                                                                        if(de_bss_sta_list_itr>0)
                                                                            strncat( payload, ",", payload_size );
                                                                        strncat( payload, "{ ", payload_size );

//                                                                        snprintf( one_line, sizeof( one_line ), "\"MACAddress\":\"%x:%x:%x:%x:%x:%x\"",de_bss_sta_list->MACaddress[0],
//                                                                            de_bss_sta_list->MACaddress[1],de_bss_sta_list->MACaddress[2],de_bss_sta_list->MACaddress[3],
//                                                                            de_bss_sta_list->MACaddress[4],de_bss_sta_list->MACaddress[5]);
                                                                        snprintf( one_line, sizeof( one_line ), "\"MACAddress\":\"%s\"",de_bss_sta_list->MACaddress);

                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"HTCapabilities\":\"0x%x\"",de_bss_sta_list->HTCapabilities[0]);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"VHTCapabilities\":\"0x%x%x%x%x%x%x\"",de_bss_sta_list->VHTCapabilities[0],
                                                                            de_bss_sta_list->VHTCapabilities[1],de_bss_sta_list->VHTCapabilities[2],de_bss_sta_list->VHTCapabilities[3],
                                                                            de_bss_sta_list->VHTCapabilities[4],de_bss_sta_list->VHTCapabilities[5]);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"HECapabilities\":\"0x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\"",de_bss_sta_list->HECapabilities[0],
                                                                            de_bss_sta_list->HECapabilities[1],de_bss_sta_list->HECapabilities[2],de_bss_sta_list->HECapabilities[3],
                                                                            de_bss_sta_list->HECapabilities[4],de_bss_sta_list->HECapabilities[5],de_bss_sta_list->HECapabilities[6],
                                                                            de_bss_sta_list->HECapabilities[7],de_bss_sta_list->HECapabilities[8],de_bss_sta_list->HECapabilities[9],
                                                                            de_bss_sta_list->HECapabilities[10],de_bss_sta_list->HECapabilities[11],de_bss_sta_list->HECapabilities[12],
                                                                            de_bss_sta_list->HECapabilities[13]);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"StatusCode\":\"%d\"",de_bss_sta_list->statusCode);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"SignalStrength\":\"%d\"",de_bss_sta_list->signalStrength);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"LastConnectTime\":\"%d\"",de_bss_sta_list->lastConnectTime);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"LastDataDownlinkRate\":\"%d\"",de_bss_sta_list->lastDataDownlinkRate);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"LastDataUplinkRate\":\"%d\"",de_bss_sta_list->lastDataUplinkRate);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"EstMACDataRateDownlink\":\"%d\"",de_bss_sta_list->estMACDataRateDownlink);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"EstMACDataRateUplink\":\"%d\"",de_bss_sta_list->estMACDataRateUplink);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"UtilizationReceive\":\"%d\"",de_bss_sta_list->utilizationReceive);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"UtilizationTransmit\":\"%d\"",de_bss_sta_list->utilizationTransmit);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"BytesSent\":\"%d\"",de_bss_sta_list->bytesSent);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"BytesReceived\":\"%d\"",de_bss_sta_list->bytesReceived);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"PacketsSent\":\"%d\"",de_bss_sta_list->packetsSent);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"PacketsReceived\":\"%d\"",de_bss_sta_list->packetsReceived);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"ErrorsReceived\":\"%d\"",de_bss_sta_list->errorsReceived);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"ErrorsSent\":\"%d\"",de_bss_sta_list->errorsSent);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"RetransCount\":\"%d\"",de_bss_sta_list->retransCount);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"IPV6Address\":\"%s\"",de_bss_sta_list->IPv6Address);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"IPV4Address\":\"%s\"",de_bss_sta_list->IPv4Address);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"Hostname\":\"%s\"",de_bss_sta_list->hostName);
                                                                        strncat( payload, one_line, payload_size );
                                                                        snprintf( one_line, sizeof( one_line ), ",\"MeasurementReport\":");
                                                                        strncat( payload, one_line, payload_size );
                                                                        strncat( payload, "[", payload_size );
                                                                        for(int i=0;i<2;i++){
                                                                            if (i>0)
                                                                                strncat( payload, ", ", payload_size );
                                                                            snprintf( one_line, sizeof( one_line ), "%d",de_bss_sta_list->measurement[i]);
                                                                            strncat( payload, one_line, payload_size );
                                                                        }
                                                                        strncat( payload, "]", payload_size );
                                                                        strncat( payload, " }", payload_size );
                                                                    }
                                                                }
                                                                strncat( payload, " ]", payload_size );
                                                            }
                                                        }
                                                        strncat( payload, " }", payload_size );
                                                    }
                                                   }
                                                strncat( payload, " ]", payload_size );
                                            }
                                        }
                                        strncat( payload, " }", payload_size );
                                    }
                                }
                                strncat( payload, " ]", payload_size );
                            }
                        }
                        strncat( payload, " }", payload_size );
                    }
                }
                strncat( payload, " ]", payload_size );
               }
        }
    }
    strncat( payload, " }", payload_size );

    if(inc == 0) {
#if LOCAL_HEADER
        strncat( payload, " } }", payload_size );
#else
        strncat( payload, "}", payload_size );
        num_bytes = append_plugin_footer( payload, payload_size );
        if (num_bytes <= 0)
            return( -1 );
#endif
    }
    return 0;
}

#endif


#ifdef cJSON_FORMAT
int wifi_all_convert_to_json(
    struct spectrum_data_t *spec_data,
    unsigned int    num,
    struct wifi_tx_stats_t *txstats_data,
    struct wifi_rx_stats_t *rxstats_data,
    struct wifi_txampdu_stats_t *txampdu_data,
    struct wifi_rxampdu_stats_t *rxampdu_data,
    struct wifi_ampdu_chart_t *ampdu_data,
    struct wifi_current_stats_t *currstat_data,
    struct wifi_power_stats_t *power_data,
    struct wifi_chanim_stats_t *chanim_data,
    unsigned int    chanim_count,
    struct wifi_connected_sta_t *connected_data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    const char*     filter
    )
{
    int         rc         = 0;
    cJSON *     objectRoot = NULL;
    cJSON *     objectData = NULL;
    cJSON *     object_rxpkt_stats = NULL;

    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);
    objectData = json_GenerateHeader(objectRoot, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION, NULL, WIFI_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    if(json_CheckFilter(NULL,filter,NULL,"spectrumData"))
        wifi_spectrum_data_convert_to_json( spec_data, num, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"transmitStatistics"))
        wifi_txpkt_stats_convert_to_json( txstats_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"receiveStatistics"))
       wifi_rxpkt_stats_convert_to_json( rxstats_data, payload,payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"transmitAMPDUStatistcs"))
        wifi_txampdu_stats_convert_to_json( txampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"receiveAMPDUStatistcs"))
        wifi_rxampdu_stats_convert_to_json( rxampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"ampduData"))
        wifi_ampdu_chart_convert_to_json( ampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"currentStats"))
        wifi_current_stats_convert_to_json( currstat_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"powerStats")){
        wifi_power_stats_convert_to_json( power_data, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    }
    if(json_CheckFilter(NULL,filter,NULL,"chanimStats"))
        wifi_chanim_stats_convert_to_json( chanim_data, chanim_count, payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    if(json_CheckFilter(NULL,filter,NULL,"connectedSTAs")){
        if(currstat_data->apsta == 1)
            wifi_connected_sta_list_convert_to_json(connected_data,payload, payload_size, tv_sec, tv_usec, 1, filter,objectData);
    }
error:
    rc = json_Print(objectRoot, payload, payload_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
        rc = strlen(payload);
    return(rc);
}

#else
int wifi_all_convert_to_json(
    struct spectrum_data_t *spec_data,
    unsigned int    num,
    struct wifi_tx_stats_t *txstats_data,
    struct wifi_rx_stats_t *rxstats_data,
    struct wifi_txampdu_stats_t *txampdu_data,
    struct wifi_rxampdu_stats_t *rxampdu_data,
    struct wifi_ampdu_chart_t *ampdu_data,
    struct wifi_current_stats_t *currstat_data,
    struct wifi_power_stats_t *power_data,
    struct wifi_chanim_stats_t *chanim_data,
    unsigned int    chanim_count,
    struct wifi_connected_sta_t *connected_data,
    char           *payload,
    int             payload_size,
    unsigned int    tv_sec,
    unsigned int    tv_usec,
    const char*     filter
    )
{
    char   one_line[128];
    int   num_bytes = 0;
#if LOCAL_HEADER
    wifi_timestamp_convert_json(payload,payload_size,tv_sec,tv_usec);
#else
        num_bytes = append_plugin_header( payload, payload_size, WIFI_PLUGIN_VERSION, WIFI_PLUGIN_NAME, WIFI_PLUGIN_DESCRIPTION );
        if (num_bytes <= 0)
            return( -1 );
        strncat( payload, "{ ", payload_size );
#endif
    wifi_spectrum_data_convert_to_json( spec_data, num, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_txpkt_stats_convert_to_json( txstats_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_rxpkt_stats_convert_to_json( rxstats_data, payload,payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_txampdu_stats_convert_to_json( txampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_rxampdu_stats_convert_to_json( rxampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_ampdu_chart_convert_to_json( ampdu_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_current_stats_convert_to_json( currstat_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_power_stats_convert_to_json( power_data, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    strncat( payload, " ,", payload_size );
    wifi_chanim_stats_convert_to_json( chanim_data, chanim_count, payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    if(currstat_data->apsta == 1) {
        strncat( payload, " ,", payload_size );
        wifi_connected_sta_list_convert_to_json(connected_data,payload, payload_size, tv_sec, tv_usec, 1, filter,NULL);
    }

#if LOCAL_HEADER
    strncat( payload, " } }", payload_size );
#else
    strncat( payload, "}", payload_size );
    num_bytes = append_plugin_footer( payload, payload_size );
    if (num_bytes <= 0)
        return( -1 );
#endif

    return 0;
}
#endif
