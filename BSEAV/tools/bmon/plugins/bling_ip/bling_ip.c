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
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bling_ip.h"
#include "plugin_header.h"
#include "bmon_utils.h"

#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_convert_macros.h"

#if NEXUS_HAS_CEC
#include "nexus_cec.h"

STRING_MAP_START(blingStatusNameMap)
STRING_MAP_ENTRY(BLING_IP_OKAY, "Success")
STRING_MAP_ENTRY(BLING_IP_ERROR_NXCLIENT_JOIN, "NxClient_Join() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_OPEN, "NEXUS_Cec_Open() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_HDMIOUTPUT_OPEN, "NEXUS_HdmiOutput_Open() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_HDMIOUTPUT_GETSTATUS, "NEXUS_HdmiOutput_GetStatus() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_SETSETTINGS, "NEXUS_Cec_SetSettings() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_GETSETTINGS, "NEXUS_Cec_GetSettings() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_GETSTATUS, "NEXUS_Cec_GetStatus() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_RECEIVEMESSAGE, "NEXUS_Cec_ReceiveMessage() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_NEXUS_CEC_TRANSMITMESSAGE, "NEXUS_Cec_TransmitMessage() failed")
STRING_MAP_ENTRY(BLING_IP_ERROR_MAX, "Unknown error code")
STRING_MAP_END()

ENUM_TO_STRING_FUNCTION(blingStatusToString, bling_status_t, blingStatusNameMap)
STRING_TO_ENUM_FUNCTION(stringToBlingStatus, bling_status_t, blingStatusNameMap)

static NEXUS_CecHandle hCec;
static bool        deviceReady     = false;
static bool        messageReceived = false;
static int         counter         = 0;
static char        bling_ipHolder[15];
static char        requestString[10];
static NEXUS_Error g_Nexus_Status    = NEXUS_SUCCESS;
static bool        g_Event_Destroyed = true;

/**
 *  Function: This function will make sure if the box is ready with the physical address to get the CEC communication done for the bling Ip address exchange.
 **/
static void deviceReady_callback(
        void * context,
        int    param
        )
{
    NEXUS_CecStatus status;
    NEXUS_Error     rc = NEXUS_SUCCESS;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    rc = NEXUS_Cec_GetStatus(hCec, &status);

    if ((status.physicalAddress[0] != 0xFF) && (status.physicalAddress[1] != 0xFF))
    {
        deviceReady = true;
    }
    /*fprintf(stderr, "%s - deviceReady %u \n", __FUNCTION__, deviceReady );*/
}

/**
 *  Function: This function will deal with receiving the bling box ip address.
 **/
static void msgReceived_callback(
        void * context,
        int    param
        )
{
    NEXUS_CecStatus          status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error              rc = NEXUS_SUCCESS;
    uint8_t                  i, j;
    char                     msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);

    do
    {
        rc = NEXUS_Cec_GetStatus(hCec, &status);
        if (rc != NEXUS_SUCCESS)
        {
            rc = BLING_IP_ERROR_NEXUS_CEC_GETSTATUS;
            break;
        }

        rc = NEXUS_Cec_ReceiveMessage(hCec, &receivedMessage);
        if (rc != NEXUS_SUCCESS)
        {
            rc = BLING_IP_ERROR_NEXUS_CEC_RECEIVEMESSAGE;
            break;
        }

        if (counter == 0)
        {
            counter++;
        }
        else
        {
            for (i = 0, j = 0; i <= receivedMessage.data.length && j < (sizeof(msgBuffer)-1); i++)
            {
                j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%c", receivedMessage.data.buffer[i]);
            }
            strcpy(bling_ipHolder, msgBuffer);
            counter++;
        }
    }
    while (0);

    /* set a global variable so that the get_data() API can know what happened in the callback */
    g_Nexus_Status = rc;

    /*fprintf(stderr, "%s - rc %d \n", __FUNCTION__, g_Nexus_Status );*/
    return;
} /* msgReceived_callback */

static void msgTransmitted_callback(
        void * context,
        int    param
        )
{
    NEXUS_CecStatus          status;
    NEXUS_Error              rc = NEXUS_SUCCESS;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_CecMessage         message;
    char                     ip_address[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);

    /* There is a scenario where the main get_data() function starts to shutdown while the callbacks are still
     * being triggered. Check to make sure the inter-communication event has not been destroyed. */
    if (g_Event_Destroyed == true) { return; }

    BKNI_SetEvent((BKNI_EventHandle)context);
    rc = NEXUS_Cec_GetStatus(hCec, &status);
    if (rc != NEXUS_SUCCESS)
    {
        g_Nexus_Status = BLING_IP_ERROR_NEXUS_CEC_GETSTATUS;
    }

    strcpy(requestString, "ip");
    sscanf(requestString, "%s", &message.buffer[0]);
    message.length          = 3;
    message.destinationAddr = receivedMessage.data.initiatorAddr;
    rc                      = NEXUS_Cec_TransmitMessage(hCec, &message);
    if (rc != NEXUS_SUCCESS)
    {
        /* set a global variable so that the get_data() API can know what happened in the callback */
        g_Nexus_Status = BLING_IP_ERROR_NEXUS_CEC_TRANSMITMESSAGE;
    }
    if (counter == 0) { counter++; }
    /*fprintf(stderr, "%s - end ... param %d \n", __FUNCTION__, param );*/
} /* msgTransmitted_callback */

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int bling_ip_get_data(
        const char * filter,
        char *       json_output,
        unsigned int json_output_size
        )
{
    cJSON *                objectRoot = NULL;
    cJSON *                objectData = NULL;
    bling_status_t         status     = BLING_IP_OKAY;
    NEXUS_Error            rc         = NEXUS_SUCCESS;
    BKNI_EventHandle       event      = NULL;
    unsigned               loops      = 0;
    NEXUS_HdmiOutputHandle hdmiOutput = NULL;
    NEXUS_HdmiOutputStatus hdmiOutputStatus;
    NEXUS_CecSettings      cecSettings;

    assert(NULL != json_output);
    assert(0 < json_output_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, done);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, BLING_IP_PLUGIN_NAME, BLING_IP_PLUGIN_DESCRIPTION, NULL, BLING_IP_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, done);

    BKNI_CreateEvent(&event);
    g_Event_Destroyed = false;

    /* nxserver does not open CEC, so a client may. */
    NEXUS_Cec_GetDefaultSettings(&cecSettings); /* is a void function */

    hCec = NEXUS_Cec_Open(0, &cecSettings);
    CHECK_PTR_ERROR_GOTO("failure opening cec", hCec, status, BLING_IP_ERROR_NEXUS_CEC_OPEN, done);

    /* nxserver opens HDMI output, so a client can only open as a read-only alias. */
    hdmiOutput = NEXUS_HdmiOutput_Open(0 + NEXUS_ALIAS_ID, NULL);
    CHECK_PTR_ERROR_GOTO("failure opening hdmi output", hdmiOutput, status, BLING_IP_ERROR_NEXUS_HDMIOUTPUT_OPEN, done);

    rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus);
    if (rc != NEXUS_SUCCESS)
    {
        status = BLING_IP_ERROR_NEXUS_HDMIOUTPUT_GETSTATUS;
        goto done;
    }

    NEXUS_Cec_GetSettings(hCec, &cecSettings);
    cecSettings.messageReceivedCallback.callback        = msgReceived_callback;
    cecSettings.messageReceivedCallback.context         = event;
    cecSettings.messageTransmittedCallback.callback     = msgTransmitted_callback;
    cecSettings.messageTransmittedCallback.context      = event;
    cecSettings.logicalAddressAcquiredCallback.callback = deviceReady_callback;
    cecSettings.logicalAddressAcquiredCallback.context  = event;
    cecSettings.physicalAddress[0]                      = (hdmiOutputStatus.physicalAddressA << 4) | hdmiOutputStatus.physicalAddressB;
    cecSettings.physicalAddress[1]                      = (hdmiOutputStatus.physicalAddressC << 4) | hdmiOutputStatus.physicalAddressD;

    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    if (rc != NEXUS_SUCCESS)
    {
        status = BLING_IP_ERROR_NEXUS_CEC_SETSETTINGS;
        goto done;
    }

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
    cecSettings.enabled = true;
    rc                  = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    if (rc != NEXUS_SUCCESS)
    {
        status = BLING_IP_ERROR_NEXUS_CEC_SETSETTINGS;
        goto done;
    }

    for (loops = 0; loops < 1; loops++)
    {
        if (deviceReady)
        {
            break;
        }
        BKNI_Sleep(1000);
    }

    if (false == deviceReady)
    {
        goto done;
    }

    loops = 3;
    while ((!strcmp(bling_ipHolder, "")))
    {
        BKNI_WaitForEvent(event, 3 * 1000);
        loops--;
        if (loops == 0)
        {
            break;
        }
    }

    {
        cJSON * objectItem = NULL;

        /* create object to be an array item which contains multiple objects */
        objectItem = json_AddArrayElement(objectData);
        CHECK_PTR_GOTO(objectItem, skipData);

        json_AddString(objectItem, NO_FILTER, objectData, "blingIP", bling_ipHolder);

        /* if an error happened in one of the callback functions */
        if ((status == NEXUS_SUCCESS) && g_Nexus_Status) { status = g_Nexus_Status; }

        json_AddString(objectItem, NO_FILTER, objectData, "status", blingStatusToString(status));
    }
skipData:

    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, json_output, json_output_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

done:
    BKNI_Sleep(1 * 1000);
    g_Event_Destroyed = true;
    BKNI_DestroyEvent(event);
    event = NULL;
    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(json_output);
    }
    return(rc);
} /* bling_ip_get_data */

#else /* CEC SUPPORT */
int bling_ip_get_data(
        const char * filter,
        char *       json_output,
        unsigned int json_output_size
        )
{
    int     rc         = 0;
    cJSON * objectRoot = NULL;
    cJSON * objectData = NULL;

    assert(NULL != json_output);
    assert(0 < json_output_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, BLING_IP_PLUGIN_NAME, BLING_IP_PLUGIN_DESCRIPTION, NULL, BLING_IP_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

    json_AddString(objectData, NO_FILTER, objectData, "status", "CEC support not enabled");

error:
    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, json_output, json_output_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(json_output);
    }
    return(rc);
} /* bling_ip_get_data */

#endif /* if NEXUS_HAS_CEC */

#if defined (BMON_PLUGIN)
/**
 *  Function: This function will coordinate collecting bling_ip data and once that is done,
 *            it will convert the bling_ip data to a JSON format and send the JSON data back
 *            to the browser or curl or wget.
 **/
#define PAYLOAD_SIZE  2048
int main(
        int    argc,
        char * argv[]
        )
{
    int  rc = 0;
    char payload[PAYLOAD_SIZE];

#if NEXUS_HAS_CEC
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }
#endif /* if NEXUS_HAS_CEC */

    rc = bling_ip_get_data(argv[1], payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving bling ip data", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:

#if NEXUS_HAS_CEC
    NxClient_Uninit();
errorNxClient:
#endif

    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */