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
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <netdb.h>
#include <string.h>
#include "bstd.h"
#include "bmemperf_lib.h"
#include "bmemperf_utils.h"
#include "astratop_utils.h"

#define CONTENT_TYPE_HTML           "Content-type: text/html\n\n"
#define NUMBER_OF_VLINES_ON_GRAPHS  66  /*Number of vertical lines on SVG Graph*/
#define GRAPH_Y_AXIS_XCOORD         70
#define GRAPH_X_AXIS_YCOORD         320
#define YAXIS_LABELS_PITCH          30
#define XAXIS_VLINES_PITCH          20
#define PLOTTING_LENGTH             1320
#define PLOTTING_HEIGHT             300
extern const char* g_task_colors[MAX_NUM_TASKS];

/*g_netStats and g_netStatsIdx are declared to get code compiled with bmemperf_lib.c*/
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;

/**
 *  Function: This function send request and reads response over Unix domain socket.
 **/
static int sendRequestReadResponse(
    struct sockaddr_un          *server,
    struct astraTopRequest    *request,
    int                          request_len,
    struct astraTopResponse   *response,
    int                          response_len
    )
{
    int                rc = 0;
    int                sd = 0;

    PRINTF( "%s: reqlen %d; reslen %d; server_path %s\n", __FUNCTION__, request_len, response_len, server->sun_path );
    /*   Create socket on which to send and receive */
    sd = socket( AF_UNIX, SOCK_STREAM, 0 );

    if (sd<0)
    {
        printf( "~ERROR~Creating AF_UNIX socket failed\n");
        return( -1 );
    }

    /* Connect to server */
    PRINTF( "%s: connect(sock %u; path %u)\n", __FUNCTION__, sd, server->sun_path );
    if (connect( sd, (struct sockaddr *) server, sizeof( *server )) < 0)
    {
        Close( sd );
        printf( "~ERROR~Connection to astratop_server failed\n");
        return( -1 );
    }

    PRINTF( "Sending request cmd (%d) to server; sizeof(request) %u\n", request->cmd, request_len );
    if (send( sd, request, request_len, 0 ) < 0)
    {
        printf( "~%s: failed to send cmd %u to socket %u\n", __FUNCTION__, request->cmd, sd );
        return( -1 );
    }

    PRINTF( "Reading response from server; sizeof(response) %u\n", response_len );
    if (( rc = recv( sd, response, response_len, 0 )) < 0)
    {
        printf( "~%s: failed to recv response from socket %u\n", __FUNCTION__, sd );
        return( -1 );
    }

    Close( sd );
    return( 0 );
}

/**
 *  Function: This function outputs the HTML content to draw the CPU % graph.
 **/
void outputSvgContent(
    char *svgId
    )
{
    int i;
    /*Arrow head at end of X-Axis and Y-Axis*/
    printf("<defs>\n");
    printf("<marker id=\"%sArrow\" markerWidth=\"10\" markerHeight=\"10\" refX=\"0\" refY=\"3\" orient=\"auto\" markerUnits=\"strokeWidth\">\n", svgId);
    printf("<path d=\"M0,0 L0,6 L9,3 z\" fill=\"gray\" />\n");
    printf("</marker></defs>");

    /*X-Axis*/
    printf("<line x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\" stroke=\"lightgray\" stroke-width=\"1\" marker-end=\"url(#%sArrow)\"/>\n",
             GRAPH_Y_AXIS_XCOORD, GRAPH_X_AXIS_YCOORD, GRAPH_Y_AXIS_XCOORD + PLOTTING_LENGTH + 10, GRAPH_X_AXIS_YCOORD, svgId);

    /*Y-Axis*/
    printf("<line x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\" stroke=\"lightgray\" stroke-width=\"1\" marker-end=\"url(#%sArrow)\"/>",
            GRAPH_Y_AXIS_XCOORD, GRAPH_X_AXIS_YCOORD, GRAPH_Y_AXIS_XCOORD, GRAPH_X_AXIS_YCOORD - PLOTTING_HEIGHT - 10, svgId);

    /*CPU % scale on Y-axis*/
    printf("<g font-size=\"10\" font-family=\"sans-serif\" fill=\"black\" stroke=\"none\" text-anchor=\"end\" dominant-baseline=\"central\">");
    for (i = 0; i <= 10; i++) {
        printf("<text x=\"%u\" y=\"%u\">%u</text>", GRAPH_Y_AXIS_XCOORD - 5, GRAPH_X_AXIS_YCOORD - (i * YAXIS_LABELS_PITCH),i * 10);
    }
    printf("</g>");

    /*Y-Axis label*/
    printf("<g font-size=\"10\" font-family=\"sans-serif\" fill=\"black\" stroke=\"none\" text-anchor=\"middle\" dominant-baseline=\"central\">\n");
    printf("<text x = \"%u\" y = \"%u\" transform=\"rotate(-90, %u, %u)\">CPU %%</text>\n</g>\n",
            GRAPH_Y_AXIS_XCOORD - 50, 10 + (PLOTTING_HEIGHT/2), GRAPH_Y_AXIS_XCOORD - 50, 10 + (PLOTTING_HEIGHT/2));

    /*X-Axis label*/
    printf("<g font-size=\"10\" font-family=\"sans-serif\" fill=\"black\" stroke=\"none\" text-anchor=\"middle\" dominant-baseline=\"hanging\">\n");
    printf("<text x = \"%u\" y = \"%u\">Time</text>\n</g>\n", GRAPH_Y_AXIS_XCOORD + (PLOTTING_LENGTH/2), GRAPH_X_AXIS_YCOORD + 20);

    /*Horizotal lines starting from CPU % caliberation points on Y-Axis*/
    printf("<g stroke=\"lightgray\" stroke-width=\"1\" fill=\"none\" stroke-dasharray=\"2 2\">\n");
    for (i = 1; i <= 10; i++) {
        printf("<path d=\"M%u %u L %u %u\" />",
            GRAPH_Y_AXIS_XCOORD, GRAPH_X_AXIS_YCOORD - (i * YAXIS_LABELS_PITCH), GRAPH_Y_AXIS_XCOORD + PLOTTING_LENGTH, GRAPH_X_AXIS_YCOORD - (i * YAXIS_LABELS_PITCH));
    }
    printf("</g>");

    printf("<g stroke=\"#F7EEEE\" stroke-width=\"1\" fill=\"none\" stroke-dasharray=\"2 2\">\n");
    /*Lighter Horizontal Lines*/
    for (i = 0; i < 10; i++) {
        printf("<path d=\"M%u %u L %u %u\" />",
            GRAPH_Y_AXIS_XCOORD, (GRAPH_X_AXIS_YCOORD - (YAXIS_LABELS_PITCH/2)) - (i * YAXIS_LABELS_PITCH), GRAPH_Y_AXIS_XCOORD + PLOTTING_LENGTH, (GRAPH_X_AXIS_YCOORD - (YAXIS_LABELS_PITCH/2)) - (i * YAXIS_LABELS_PITCH));
    }

    /*Lighter Vertical Lines at intervals at XAXIS_VLINES_PITCH. Totally there will be 1320/20 = 66 (NUMBER_OF_VLINES_ON_GRAPHS) lines. */
    for (i = 1; i <= NUMBER_OF_VLINES_ON_GRAPHS; i++) {
        printf("<path d=\"M%u %u L %u %u \"/>", GRAPH_Y_AXIS_XCOORD + (i * XAXIS_VLINES_PITCH), GRAPH_X_AXIS_YCOORD, GRAPH_Y_AXIS_XCOORD + (i * XAXIS_VLINES_PITCH), GRAPH_X_AXIS_YCOORD - PLOTTING_HEIGHT);
    }
    printf("</g>");

    /*Possible PolyLines and circle markers in a graph - one for each Task*/
    for (i = MAX_NUM_TASKS - 1; i >= 0; i--) {
        printf("<marker id=\"%sCircle%u\" markerWidth=\"6\" markerHeight=\"6\" refX=\"3\" refY=\"3\" markerUnits=\"strokeWidth\">",svgId,i);
        printf("<circle cx=\"3\" cy=\"3\" r=\"2\" fill=\"%s\"/>", g_task_colors[i]);
        printf("</marker>");
        printf("<polyline id=%sPolyline%u style=\"fill:none;stroke:%s;stroke-width:1\" stroke-dasharray=\"2 2\" marker-start=\"url(#%sCircle%u)\" marker-mid=\"url(#%sCircle%u)\" marker-end=\"url(#%sCircle%u)\"/>\n",
            svgId, i, g_task_colors[i], svgId, i, svgId, i, svgId,i);
    }
}


/**
 *  Function: This function is the entry point for the CGI application run whenever HTTP request for astratop.cgi is raised.
 **/

int main(
    int   argc,
    char *argv[]
    )
{
    char *queryString = NULL;
    int epochSeconds = 0;
    int tzOffset = 0;
    int selectedCores = 0;
    int filterRT = 0;
    int filterNRT = 0;
    int taskID = -1;
    int RTCores = 0;
    int NRTCores = 0;
    int exportTaskStats = 0;
    bmemperf_version_info versionInfo;
    struct utsname uname_info;
    int rc = -1;
    struct sockaddr_un server;
    struct astraTopRequest request;
    struct astraTopResponse response;

    memset( &versionInfo, 0, sizeof( versionInfo ));
    memset( &response, 0, sizeof( response ));
    memset( &request, 0, sizeof( request ));

    queryString   = getenv( "QUERY_STRING" );
    if (queryString && strlen( queryString )) {
        scanForInt( queryString, "datetime=", &epochSeconds );
        scanForInt( queryString, "tzoffset=", &tzOffset );
        scanForInt( queryString, "selectedCores=", &selectedCores );
        scanForInt( queryString, "filterRT=", &filterRT );
        scanForInt( queryString, "RTCores=", &RTCores );
        scanForInt( queryString, "filterNRT=", &filterNRT );
        scanForInt( queryString, "NRTCores=", &NRTCores );
        scanForInt( queryString, "taskID=", &taskID );
        scanForInt( queryString, "exportTasks=", &exportTaskStats);
    }
    else {
        printf( CONTENT_TYPE_HTML );
        printf( "~ERROR: QUERY_STRING is not defined~" );
        return( -1 );
    }
    printf( CONTENT_TYPE_HTML );

    /* if browser provided a new date/time value; this only happens once during initialization */
    if (epochSeconds) {
        struct timeval now          = {1400000000, 0};
        char          *boltVersion  = NULL;

        strncpy( versionInfo.platform, getPlatform(), sizeof( versionInfo.platform ) - 1 );
        strncpy( versionInfo.platVersion, getPlatformVersion(), sizeof( versionInfo.platVersion ) - 1 );
        versionInfo.majorVersion   = MAJOR_VERSION;
        versionInfo.minorVersion   = MINOR_VERSION;
        printf( "~PLATFORM~%s", versionInfo.platform );
        printf( "~VARIANT~%s", getProductIdStr() );
        printf( "~PLATVER~%s", versionInfo.platVersion );
        printf( "~VERSION~Ver: %u.%u", versionInfo.majorVersion, versionInfo.minorVersion );

        boltVersion = getFileContents( "/proc/device-tree/bolt/tag" );
        if ( boltVersion ) {
            printf( "~BOLTVER~%s", boltVersion );
            Bsysperf_Free( boltVersion );
        }

        uname(&uname_info);
        printf("~UNAME~%d-bit %s %s", (sizeof(char*) == 8)?64:32, uname_info.machine , uname_info.release );

        now.tv_sec = epochSeconds - ( tzOffset * 60 );
        settimeofday( &now, NULL );
        usleep( 200 );

        server.sun_family = AF_UNIX;
        strcpy(server.sun_path, SOCK_PATH);
        request.cmd = ASTRATOP_CMD_GET_CPUCORES;
        if (!(rc = sendRequestReadResponse( &server, &request, sizeof(request), &response, sizeof(response) ))) {
            uint32_t temp;
            uint8_t index = 0;
            printf("~CPUCORES~");
            printf("<table border=0 style=\"border-collapse:collapse;\">\n<tr>\n");
            temp = response.CPUCoresBitMask;
            while (temp) {
                if (1 == (temp & 1)) {
                    if (temp == 1)
                        printf("<td ><input type=checkbox id=checkboxcpu%d onclick=\"myClick(event);\" checked >CPU %d</td>\n",index, index);
                    else
                        printf("<td style=\"padding-right: 40px;\"><input type=checkbox id=checkboxcpu%d onclick=\"myClick(event);\" checked >CPU %d</td>\n",index, index);
                }
                temp /= 2;
                index++;
            }
            printf("</tr>\n</table>\n");
            printf("~CORESBITMASK~%u",response.CPUCoresBitMask);
        }
        request.cmd = ASTRATOP_CMD_GET_TASKINFO;
        request.secondaryCmd = ASTRATOP_CMD_GET_PER_CORE_TASKS_INFO;
        request.coresRequest.coresBitMask = response.CPUCoresBitMask;
        request.coresRequest.taskType = RT_TASKS | NRT_TASKS;
        if (!(rc = sendRequestReadResponse( &server, &request, sizeof(request), &response, sizeof(response) ))) {
            printf("~VLINESONGRAPH~%u", NUMBER_OF_VLINES_ON_GRAPHS);
            printf("~YAXISXCOORD~%u",GRAPH_Y_AXIS_XCOORD);
            printf("~GRAPHPOINTSPITCH~%u",XAXIS_VLINES_PITCH);

            char buf[10];
            uint8_t index;

            printf("~TASKSPERCOREINIT~%d", response.coresInfo.noOfCores);
            for (index = 0; index < response.coresInfo.noOfCores; index++) {
                printf("~core%d~",response.coresInfo.cores[index].cpuId);
                printf("<table border=0 style=\"border-collapse:collapse;\" width=100%%>\n");
                printf("<tr id = \"core%dTitle\"></tr>", response.coresInfo.cores[index].cpuId);
                printf("<tr ><td style=\"text-align:center;\">");
                printf("<svg id = \"core%dGraph\" height=\"%u\" width=\"%u\">",
                    response.coresInfo.cores[index].cpuId,
                    PLOTTING_HEIGHT + 70/*(10px for Y-Axis extension, 10px for arrowhead,  50 px for Time text with space )*/,
                    PLOTTING_LENGTH + 100/*(30px for X-Axis extension, 30 px for % numbers(0 - 100), 40 px for CPU % text)*/);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "core%d", response.coresInfo.cores[index].cpuId);
                outputSvgContent(buf);
                printf("</svg>");
                printf("</td></tr>\n");
                printf("<tr >\n");
                printf("<table align=\"center\" style=\"border-collapse:collapse; width:80%%;\" class = \"optionsContent\" id = \"core%dTable\">\n", response.coresInfo.cores[index].cpuId);
                printf("</table>\n");
                printf("</tr></table>\n");
            }

            printf("~RTTASKSINIT~");
            printf("<table border=0 style=\"border-collapse:collapse;\" width=100%%>\n");
            printf("<tr ><td style=\"font-size:16pt; font-weight:bold;\">RT Tasks</td></tr>");
            for (index = 0; index < response.coresInfo.noOfCores; index++) {
                printf("<tr>\n");
                printf("<table border=0 style=\"border-collapse:collapse;\" width=100%%>\n");
                printf("<tr id=\"core%dRTTitle\"></tr>", response.coresInfo.cores[index].cpuId);
                printf("<tr><td style=\"text-align:center;\">");
                printf("<svg id = \"core%dRTGraph\" height=\"%u\" width=\"%u\">",
                    response.coresInfo.cores[index].cpuId,
                    PLOTTING_HEIGHT + 70/*(10px for Y-Axis extension, 10px for arrowhead,  50 px for Time text with space )*/,
                    PLOTTING_LENGTH + 100/*(40px for X-Axis extension, 30 px for % numbers(0 - 100), 40 px for CPU % text)*/);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "core%dRT", response.coresInfo.cores[index].cpuId);
                outputSvgContent(buf);
                printf("</svg>");
                printf("</td></tr>\n");
                printf("<tr>\n");
                printf("<table align=\"center\" id=\"core%dRTTable\"style=\"border-collapse:collapse; width:80%%;\" class = \"optionsContent\"", response.coresInfo.cores[index].cpuId);
                printf("</table>\n");
                printf("</tr>");
                printf("</table>");
                printf("</tr>");
            }
            printf("</table>");

            printf("~NRTTASKSINIT~");
            printf("<table border=0 style=\"border-collapse:collapse;\" width=100%%>\n");
            printf("<tr ><td style=\"font-size:16pt; font-weight:bold;\">NRT Tasks</td></tr>");
            for (index = 0; index < response.coresInfo.noOfCores; index++) {
                printf("<tr>\n");
                printf("<table border=0 style=\"border-collapse:collapse;\" width=100%%>\n");
                printf("<tr id=\"core%dNRTTitle\"></tr>", response.coresInfo.cores[index].cpuId);
                printf("<tr><td style=\"text-align:center;\">");
                printf("<svg id = \"core%dNRTGraph\" height=\"%u\" width=\"%u\">",
                    response.coresInfo.cores[index].cpuId,
                    PLOTTING_HEIGHT + 70/*(10px for Y-Axis extension, 10px for arrowhead,  50 px for Time text with space )*/,
                    PLOTTING_LENGTH + 100/*(30px for X-Axis extension, 30 px for % numbers(0 - 100), 40 px for CPU % text)*/);
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "core%dNRT", response.coresInfo.cores[index].cpuId);
                outputSvgContent(buf);
                printf("</svg>");
                printf("</td></tr>\n");
                printf("<tr>\n");
                printf("<table align=\"center\" id=\"core%dNRTTable\"style=\"border-collapse:collapse; width:80%%;\" class = \"optionsContent\"", response.coresInfo.cores[index].cpuId);
                printf("</table>\n");
                printf("</tr>");
                printf("</table>");
                printf("</tr>");
            }
            printf("</table>");

            printf("~TASKIDINIT~");
            printf("<table border=0 style=\"border-collapse:collapse;\" id = \"taskIDInfo\" width=100%%>\n");
            printf("<tr id=\"taskIDTitle\"></tr>\n");
            printf("<tr><td style=\"text-align:center;\">\n");
            printf("<svg id = \"taskIDGraph\" height=\"%u\" width=\"%u\">",
                    PLOTTING_HEIGHT + 70/*(10px for Y-Axis extension, 10px for arrowhead,  50 px for Time text with space )*/,
                    PLOTTING_LENGTH + 100/*(30px for X-Axis extension, 30 px for % numbers(0 - 100), 40 px for CPU % text)*/);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "taskID");
            outputSvgContent(buf);
            printf("</svg>");
            printf("</td></tr>\n");
            printf("<tr>");
            printf("<table align=\"center\" style=\"border-collapse:collapse; width:80%%;\" class = \"optionsContent\" id = \"taskIDTable\">\n");
            printf("</table>\n");
            printf("</tr></table>\n");

            printf("~TASKSPERCORE~%d",response.coresInfo.noOfCores * 2);
            int t, start = 0;
            for (index = 0; index < response.coresInfo.noOfCores; index++) {
                printf("~core%dTitle~",response.coresInfo.cores[index].cpuId);
                printf("<td style=\"font-size:16pt; font-weight:bold;\">CPU %d : Number of Tasks: %d</td>",
                                response.coresInfo.cores[index].cpuId,response.coresInfo.cores[index].noOfTasks);
                printf("~core%dTable~", response.coresInfo.cores[index].cpuId);
                if (!response.coresInfo.cores[index].noOfTasks) {
                    continue;
                }
                printf("<colgroup>\n");
                printf("<col style=\"width: 20%%\" />");
                printf("<col style=\"width: 20%%\" />");
                printf("<col style=\"width: 20%%\" />");
                printf("<col style=\"width: 20%%\" />");
                printf("<col style=\"width: 20%%\" />");
                printf("</colgroup>\n");
                printf("<tr class=\"headingRow\"><th style=\"text-decoration: underline;\">Task ID</th>\n");
                printf("<th style=\"text-decoration: underline;\">Priority</th>\n");
                printf("<th style=\"text-decoration: underline;\">Load</th>\n");
                printf("<th style=\"text-decoration: underline;\">CPU %%</th>\n");
                printf("<th style=\"text-decoration: underline;\">Status</th></tr>\n");
                for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                    printf("<tr>\n");
                    printf("<td style=\"text-align:center;\"><svg width = \"11\" height = \"11\"><rect width=\"11\" height=\"11\" style=\"fill:%s;\" /></svg>&nbsp;%d</td>\n",
                        g_task_colors[response.tasksInfo.tasks[t].taskDetails.taskID], response.tasksInfo.tasks[t].taskDetails.taskID);
                    if (NRT_TASKS == response.tasksInfo.tasks[t].taskType) {
                        printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.priority);
                        printf("<td style=\"text-align:center;\">NA</td>\n");
                    }
                    else if (RT_TASKS == response.tasksInfo.tasks[t].taskType) {
                        printf("<td style=\"text-align:center;\">RT</td>\n");
                        printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskLoad);
                    }
                    printf("<td style=\"text-align:center;\">%f</td>\n",response.tasksInfo.tasks[t].taskDetails.CPUper);
                    printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskStatus);
                    printf("</tr>");
                }
                start = start + response.coresInfo.cores[index].noOfTasks;
            }
            start = 0;
            for (index = 0; index < response.coresInfo.noOfCores; index++) {
                printf("~UPDATEGRAPH~core%dGraph", response.coresInfo.cores[index].cpuId);
                if (response.coresInfo.cores[index].noOfTasks) {
                    uint32_t nextY, pixelsPerPercent;
                    float temp;
                    printf("~POLYLINESCOUNT~%u",response.coresInfo.cores[index].noOfTasks);
                    for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                        temp = (response.tasksInfo.tasks[t].taskDetails.CPUper > 100.0) ? 100.0 : response.tasksInfo.tasks[t].taskDetails.CPUper;
                        /*(0,0) is top left corner of SVG.Y-Coordinate of X-axis is GRAPH_X_AXIS_YCOORD.
                                        Hence Y coodinate to plot CPU % value = GRAPH_X_AXIS_YCOORD - (CPU % *PLOTTING_HEIGHT)/100*/
                        pixelsPerPercent = PLOTTING_HEIGHT/100;
                        nextY = (uint32_t)(GRAPH_X_AXIS_YCOORD - (temp * pixelsPerPercent));
                        printf("~core%dPolyline%u~%u", response.coresInfo.cores[index].cpuId, response.tasksInfo.tasks[t].taskDetails.taskID,nextY);
                    }
                    start = start + response.coresInfo.cores[index].noOfTasks;
                }
            }
        }
    }
    else if (selectedCores || RTCores || NRTCores || (taskID >= 0)) {
        request.cmd = ASTRATOP_CMD_GET_TASKINFO;
        if (selectedCores || RTCores || NRTCores) {
            request.secondaryCmd = ASTRATOP_CMD_GET_PER_CORE_TASKS_INFO;
            request.coresRequest.coresBitMask = (selectedCores ? selectedCores : (RTCores ? RTCores : NRTCores));
            request.coresRequest.taskType = RT_TASKS | NRT_TASKS;
            if (!selectedCores) {
                if (!filterRT)
                request.coresRequest.taskType ^= RT_TASKS;
                if (!filterNRT)
                request.coresRequest.taskType ^= NRT_TASKS;
            }
        }
        if (taskID >= 0) {
            request.secondaryCmd |= ASTRATOP_CMD_GET_TASKID_INFO;
            request.taskId = taskID;
        }
        server.sun_family = AF_UNIX;
        strcpy(server.sun_path, SOCK_PATH);

        if (!(rc = sendRequestReadResponse( &server, &request, sizeof(request), &response, sizeof(response) ))) {
            if (taskID >= 0) {
                uint8_t taskActive = 0; /*taskActive is assigned 1 if task 'taskID' is scheduled.*/
                int arrayLength = response.tasksInfo.noOfTasks;
                printf("~TASKID~2");
                if ((arrayLength > 0 ) && (response.tasksInfo.tasks[arrayLength - 1].taskDetails.taskID == taskID)) {
                    printf("~taskIDTitle~");
                    printf("<td style=\"font-size:16pt; font-weight:bold;\">Task ID: %d</td>",taskID);
                    printf("~taskIDTable~");
                    printf("<colgroup>\n");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<colgroup>\n");
                    printf("<tr class=\"headingRow\"><th style=\"text-decoration: underline;\">CPU ID</th>\n");
                    if (NRT_TASKS == response.tasksInfo.tasks[arrayLength - 1].taskType)
                        printf("<th style=\"text-decoration: underline;\">Priority</th>\n");
                    else if (RT_TASKS == response.tasksInfo.tasks[arrayLength - 1].taskType)
                        printf("<th style=\"text-decoration: underline;\">Load</th>\n");
                    printf("<th style=\"text-decoration: underline;\">CPU %%</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Status</th></tr>\n");
                    printf("<tr>\n");
                    printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[arrayLength - 1].taskDetails.CPUID);
                    if (NRT_TASKS == response.tasksInfo.tasks[arrayLength - 1].taskType)
                        printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[arrayLength - 1].taskDetails.priority);
                    else if (RT_TASKS == response.tasksInfo.tasks[arrayLength - 1].taskType)
                        printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[arrayLength - 1].taskDetails.taskLoad);
                    printf("<td style=\"text-align:center;\">%f</td>\n",response.tasksInfo.tasks[arrayLength - 1].taskDetails.CPUper);
                    printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[arrayLength - 1].taskDetails.taskStatus);
                    printf("</tr>\n");
                    taskActive = 1;
                }
                else {
                    printf("~taskIDTitle~");
                    printf("<td style=\"font-size:16pt; font-weight:bold;\">Task ID: %d is not scheduled</td>",taskID);
                    printf("~taskIDTable~");
                }
                printf("~UPDATEGRAPH~taskIDGraph");
                if (taskActive) {
                    uint32_t nextY, pixelsPerPercent;
                    float temp;
                    printf("~POLYLINESCOUNT~1");
                    temp = (response.tasksInfo.tasks[arrayLength - 1].taskDetails.CPUper > 100.0) ? 100.0 : response.tasksInfo.tasks[arrayLength - 1].taskDetails.CPUper;
                    /*(0,0) is top left corner of SVG. Y-Coordinate of X-axis is GRAPH_X_AXIS_YCOORD.
                                        Hence Y coodinate to plot CPU % value = GRAPH_X_AXIS_YCOORD - (CPU % *PLOTTING_HEIGHT)/100*/
                    pixelsPerPercent = PLOTTING_HEIGHT/100;
                    nextY = (uint32_t)(GRAPH_X_AXIS_YCOORD - (temp * pixelsPerPercent));
                    printf("~taskIDPolyline%u~%u", taskID,nextY);
                }
            }

            if (selectedCores) {
                uint8_t index;
                int t, start = 0;
                printf("~TASKSPERCORE~%d", response.coresInfo.noOfCores * 2);
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~core%dTitle~",response.coresInfo.cores[index].cpuId);
                    printf("<td style=\"font-size:16pt; font-weight:bold;\">CPU %d : Number of Tasks: %d</td>",
                                    response.coresInfo.cores[index].cpuId,response.coresInfo.cores[index].noOfTasks);
                    printf("~core%dTable~", response.coresInfo.cores[index].cpuId);
                    if (!response.coresInfo.cores[index].noOfTasks)
                        continue;

                    printf("<colgroup>\n");
                    printf("<col style=\"width: 20%%\" />");
                    printf("<col style=\"width: 20%%\" />");
                    printf("<col style=\"width: 20%%\" />");
                    printf("<col style=\"width: 20%%\" />");
                    printf("<col style=\"width: 20%%\" />");
                    printf("</colgroup>\n");
                    printf("<tr class=\"headingRow\"><th style=\"text-decoration: underline;\">Task ID</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Priority</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Load</th>\n");
                    printf("<th style=\"text-decoration: underline;\">CPU %%</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Status</th></tr>\n");
                    for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                        printf("<tr>\n");
                        printf("<td style=\"text-align:center;\"><svg width = \"11\" height = \"11\"><rect width=\"11\" height=\"11\" style=\"fill:%s;\" /></svg>&nbsp;%d</td>\n",g_task_colors[response.tasksInfo.tasks[t].taskDetails.taskID], response.tasksInfo.tasks[t].taskDetails.taskID);
                        if (NRT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.priority);
                            printf("<td style=\"text-align:center;\">NA</td>\n");
                        }
                        else if (RT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf("<td style=\"text-align:center;\">RT</td>\n");
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskLoad);
                        }
                        printf("<td style=\"text-align:center;\">%f</td>\n",response.tasksInfo.tasks[t].taskDetails.CPUper);
                        printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskStatus);
                        printf("</tr>");
                    }
                    start = start + response.coresInfo.cores[index].noOfTasks;
                }
                start = 0;
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~UPDATEGRAPH~core%dGraph", response.coresInfo.cores[index].cpuId);
                    if (response.coresInfo.cores[index].noOfTasks) {
                        uint32_t nextY, pixelsPerPercent;
                        float temp;
                        printf("~POLYLINESCOUNT~%u",response.coresInfo.cores[index].noOfTasks);
                        for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                            temp = (response.tasksInfo.tasks[t].taskDetails.CPUper > 100.0) ? 100.0 : response.tasksInfo.tasks[t].taskDetails.CPUper;
                            /*(0,0) is top left corner of SVG.Y-Coordinate of X-axis is GRAPH_X_AXIS_YCOORD.
                                        Hence Y coodinate to plot CPU % value = GRAPH_X_AXIS_YCOORD - (CPU % *PLOTTING_HEIGHT)/100*/
                            pixelsPerPercent = PLOTTING_HEIGHT/100;
                            nextY = (uint32_t)(GRAPH_X_AXIS_YCOORD - (temp * pixelsPerPercent));
                            printf("~core%dPolyline%u~%u", response.coresInfo.cores[index].cpuId, response.tasksInfo.tasks[t].taskDetails.taskID,nextY);
                        }
                        start = start + response.coresInfo.cores[index].noOfTasks;
                    }
                }
            }

            if (filterRT) {
                uint8_t index;
                int t, start = 0;
                printf("~RTTASKSPERCORE~%d", response.coresInfo.noOfCores * 2);
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~core%dRTTitle~",response.coresInfo.cores[index].cpuId);
                    printf("<td style=\"font-size:14pt;\">CPU %d : Number of RT Tasks: %d</td>",
                                    response.coresInfo.cores[index].cpuId,response.coresInfo.cores[index].noOfRTTasks);
                    printf("~core%dRTTable~", response.coresInfo.cores[index].cpuId);
                    if (!response.coresInfo.cores[index].noOfRTTasks)
                        continue;
                    printf("<colgroup>\n");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("</colgroup>\n");
                    printf("<tr class=\"headingRow\"><th style=\"text-decoration: underline;\">Task ID</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Load</th>\n");
                    printf("<th style=\"text-decoration: underline;\">CPU %%</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Status</th></tr>\n");
                    for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                        if (RT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf("<tr>\n");
                            printf("<td style=\"text-align:center;\"><svg width = \"11\" height = \"11\"><rect width=\"11\" height=\"11\" style=\"fill:%s;\" /></svg>&nbsp;%d</td>\n",g_task_colors[response.tasksInfo.tasks[t].taskDetails.taskID], response.tasksInfo.tasks[t].taskDetails.taskID);
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskLoad);
                            printf("<td style=\"text-align:center;\">%f</td>\n",response.tasksInfo.tasks[t].taskDetails.CPUper);
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskStatus);
                            printf("</tr>");
                        }
                    }
                    start = start + response.coresInfo.cores[index].noOfTasks;
                }
                start = 0;
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~UPDATEGRAPH~core%dRTGraph", response.coresInfo.cores[index].cpuId);
                    if (response.coresInfo.cores[index].noOfTasks) {
                        if (response.coresInfo.cores[index].noOfRTTasks) {
                            uint32_t nextY, pixelsPerPercent;
                            float temp;
                            printf("~POLYLINESCOUNT~%u",response.coresInfo.cores[index].noOfRTTasks);
                            for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                                if (RT_TASKS == response.tasksInfo.tasks[t].taskType) {
                                    temp = (response.tasksInfo.tasks[t].taskDetails.CPUper > 100.0) ? 100.0 : response.tasksInfo.tasks[t].taskDetails.CPUper;
                                    /*(0,0) is top left corner of SVG.Y-Coordinate of X-axis is GRAPH_X_AXIS_YCOORD.
                                            Hence Y coodinate to plot CPU % value = GRAPH_X_AXIS_YCOORD - (CPU % *PLOTTING_HEIGHT)/100*/
                                    pixelsPerPercent = PLOTTING_HEIGHT/100;
                                    nextY = (uint32_t)(GRAPH_X_AXIS_YCOORD - (temp * pixelsPerPercent));
                                    printf("~core%dRTPolyline%u~%u", response.coresInfo.cores[index].cpuId, response.tasksInfo.tasks[t].taskDetails.taskID,nextY);
                                }
                            }
                        }
                        start = start + response.coresInfo.cores[index].noOfTasks;
                    }
                }
            }

            if (filterNRT) {
                uint8_t index;
                int t, start = 0;
                printf("~NRTTASKSPERCORE~%d", response.coresInfo.noOfCores * 2);
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~core%dNRTTitle~",response.coresInfo.cores[index].cpuId);
                    printf("<td style=\"font-size:14pt;\">CPU %d : Number of NRT Tasks: %d</td>",
                                    response.coresInfo.cores[index].cpuId,response.coresInfo.cores[index].noOfNRTTasks);
                    printf("~core%dNRTTable~", response.coresInfo.cores[index].cpuId);
                    if (!response.coresInfo.cores[index].noOfNRTTasks)
                        continue;
                    printf("<colgroup>\n");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("<col style=\"width: 25%%\" />");
                    printf("</colgroup>\n");
                    printf("<tr class=\"headingRow\"><th style=\"text-decoration: underline;\">Task ID</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Priority</th>\n");
                    printf("<th style=\"text-decoration: underline;\">CPU %%</th>\n");
                    printf("<th style=\"text-decoration: underline;\">Status</th></tr>\n");
                    for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                        if (NRT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf("<tr>\n");
                            printf("<td style=\"text-align:center;\"><svg width = \"11\" height = \"11\"><rect width=\"11\" height=\"11\" style=\"fill:%s;\" /></svg>&nbsp;%d</td>\n",g_task_colors[response.tasksInfo.tasks[t].taskDetails.taskID], response.tasksInfo.tasks[t].taskDetails.taskID);
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.priority);
                            printf("<td style=\"text-align:center;\">%f</td>\n",response.tasksInfo.tasks[t].taskDetails.CPUper);
                            printf("<td style=\"text-align:center;\">%d</td>\n",response.tasksInfo.tasks[t].taskDetails.taskStatus);
                            printf("</tr>");
                        }
                    }
                    start = start + response.coresInfo.cores[index].noOfTasks;
                }
                start = 0;
                for (index = 0; index < response.coresInfo.noOfCores; index++) {
                    printf("~UPDATEGRAPH~core%dNRTGraph", response.coresInfo.cores[index].cpuId);
                    if (response.coresInfo.cores[index].noOfTasks) {
                        if (response.coresInfo.cores[index].noOfNRTTasks) {
                            uint32_t nextY, pixelsPerPercent;
                            float temp;
                            printf("~POLYLINESCOUNT~%u",response.coresInfo.cores[index].noOfNRTTasks);
                            for (t = start; t < (start + response.coresInfo.cores[index].noOfTasks); t++) {
                                if (NRT_TASKS == response.tasksInfo.tasks[t].taskType) {
                                    temp = (response.tasksInfo.tasks[t].taskDetails.CPUper > 100.0) ? 100.0 : response.tasksInfo.tasks[t].taskDetails.CPUper;
                                    /*(0,0) is top left corner of SVG.Y-Coordinate of X-axis is GRAPH_X_AXIS_YCOORD.
                                            Hence Y coodinate to plot CPU % value = GRAPH_X_AXIS_YCOORD - (CPU % *PLOTTING_HEIGHT)/100*/
                                    pixelsPerPercent = PLOTTING_HEIGHT/100;
                                    nextY = (uint32_t)(GRAPH_X_AXIS_YCOORD - (temp * pixelsPerPercent));
                                    printf("~core%dNRTPolyline%u~%u", response.coresInfo.cores[index].cpuId, response.tasksInfo.tasks[t].taskDetails.taskID,nextY);
                                }
                            }
                        }
                        start = start + response.coresInfo.cores[index].noOfTasks;
                    }
                }
            }

            if (exportTaskStats) {
                printf("~EXPORTTASKS~");
                if (response.tasksInfo.noOfTasks > 0) {
                    int t, end = response.tasksInfo.noOfTasks;
                    if ((taskID >= 0 ) && (response.tasksInfo.tasks[response.tasksInfo.noOfTasks - 1].taskDetails.taskID == taskID)) {
                        if (response.tasksInfo.noOfTasks > 1)
                            end--;
                    }
                    for (t = 0; t < end; t++) {
                        printf("\n%llu", response.tasksInfo.tasks[t].timeStamp);
                        printf(",%d", response.tasksInfo.tasks[t].taskDetails.taskID);
                        printf(",%d", response.tasksInfo.tasks[t].taskDetails.CPUID);
                        if (NRT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf(",%d", response.tasksInfo.tasks[t].taskDetails.priority);
                            printf(",NA");
                        }
                        else if (RT_TASKS == response.tasksInfo.tasks[t].taskType) {
                            printf(",RT");
                            printf(",%d", response.tasksInfo.tasks[t].taskDetails.taskLoad);
                        }
                        printf(",%f",response.tasksInfo.tasks[t].taskDetails.CPUper);
                        printf(",%d",response.tasksInfo.tasks[t].taskDetails.taskStatus);
                    }
                }
            }
        }
    }
    printf( "~STBTIME~%s", DayMonDateYear( 0 ));
    return( 0 );
}                                                          /* main */
