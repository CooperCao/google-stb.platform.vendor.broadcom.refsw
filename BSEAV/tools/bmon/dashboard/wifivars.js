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

var myObj = 0;
var sceneType = 3;
var sceng0, sceng1;
var xAxis, yAxis;
var xAxisScale, yAxisScale;
var svg_chart;
var channels = [];
var heatmap;
var SSID_NUM = 10;
var chart_max_size = 490;
var chan_arr_2g_5 = [
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
];
var CHAN_NUM_2G_5M = chan_arr_2g_5.length;

var chan_arr_5g_20 = [
    36, 40, 44, 48, /* unii-1 */
    52, 56, 60, 64, /* unii-2 dfs */
    100, 104, 108, 112, 116, /* unii-2ex dfs */
    120, 124, 128, /* radar */
    132, 136, 140, 144, /* unii-2 dfs */
    149, 153, 157, 161, /* unii-3 */
    165, 169, 173, 177,
    181 /* ism */];

var chan_arr_5g_20_ind = [];
for (i = 0; i < chan_arr_5g_20; i++) {
    chan_arr_5g_20_ind[i] = i;
}

var chan_arr_5g_20_rssi = [];
for (i = 0; i < 70; i+=5) {
    chan_arr_5g_20_rssi[i] = i;
}

var CHAN_NUM_5G_20M = chan_arr_5g_20.length;
var itemSize = 30;
var cellSize = itemSize - 1;
var colorCalibration     = ['#f2f3f4', '#cdddf4', '#3f7ee2', '#e5344c', '#ffffff'];
var colorDFS             = "#cdddf4";
var colorSurveyDatasetBK = 'rgba(255, 99, 132, 0.2)';
var colorSurveyBorder    = 'rgba(255,99,132,1)';
var colorSurveyChartBK   = '#f2f3f4';
var colorAmpduBase       = 'rgba(205, 221, 244, 255)';    // corresponds for colorCalibration[1]

var margin = { top: 60, right: 60, bottom: 60, left: 160 };
var timers = [];
var chanim_counter = 0;

var reqampdu_timer;
var reqspec_timer;
var reqchan_timer;
var reqtrig_timer;
var reqstats_timer;
var reqpower_timer;
var powsDoughnutChart;
