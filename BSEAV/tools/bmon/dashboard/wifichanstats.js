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

var color = Chart.helpers.color;
var csChart = {
    type: 'line',
    data : [],
    options: {
        title: {
            text: 'Channel Idle Time Scale'
        },
        scales: {
            xAxes: [{
                ticks: {
                    maxTicksLimit: 10,
                    max : 10,
                    min : 0
                },
                scaleLabel: {
                    display: true,
                    labelString: 'Seconds'
                }
            }],
            yAxes: [{
                scaleLabel: {
                    display: true,
                    labelString: 'Medium Idle %'
                },
                ticks: {
                    maxTicksLimit: 100,
                    max: 100,
                    min: 0
                }
            }]
        },
    }
};

window.onload = function() {
    var ctx = document.getElementById('chanstats').getContext('2d');
    window.myLine = new Chart(ctx, csChart);

    var ctxTxRx = document.getElementById("statsTxRx").getContext('2d');
    window.chartTxRx = new Chart(ctxTxRx, {
        type: 'pie',
        data: {
            labels: ["TX", "RX"],
            datasets: [{
                backgroundColor: [
                    colorCalibration[1],
                    colorCalibration[3],
                ],
                data: [10, 10]
            }]
        }
    });

    var ctxNonWifi = document.getElementById("statsNonWifi").getContext('2d');
    window.chartNonWifi = new Chart(ctxNonWifi, {
        type: 'pie',
        data: {
            labels: ["WiFi", "NonWifi", "OBSS"],
            datasets: [{
                backgroundColor: [
                    colorCalibration[3],
                    colorCalibration[1],
                    colorCalibration[2],
                ],
                data: [10, 10, 10]
            }]
        }
    });
};

function parseChanim(chanim) {
    var i;
    var chanim_idle = [];

    for (i = 0; i < chanim.length; i++) {
        chanim_idle[i] = chanim[i].chanItem.mediumIdle;
    }

    if (csChart.data.datasets == 0) {
        chanim_counter = 0;
        for (i = 0; i < chanim.length; i++) {
            var newColor = "rgb(255," + (255 - chanim[i].chanItem.channelNum) + "," + chanim[i].chanItem.channelNum + ")";
            var newDataset = {
                label: 'Ch' + chanim[i].chanItem.channelNum,
                borderColor: newColor,
                fill : false,
                backgroundColor: color(newColor).alpha(0.5).rgbString() ,
                data: [],
            };

            csChart.data.datasets.push(newDataset);
            window.myLine.update();
        }
    }

    if (csChart.data.datasets.length > 0) {

        if (chanim_counter >= 10) {
            csChart.data.labels.splice(0, 1);
            for (var index = 0; index < csChart.data.datasets.length; index++) {
                console.log("removing: " + csChart.data.datasets[index].data.length);
                csChart.data.datasets[index].data.splice(0,1);
            }
        }

        csChart.data.labels.push(chanim_counter);
        for (var index = 0; index < csChart.data.datasets.length; index++) {
                console.log("pushing dataset:" + index + " len=" + csChart.data.datasets[index].data.length +
                    "  object with x=" + (csChart.data.datasets[index].data.length) +
                    " y=" + chanim_idle[index] + " chan=" + chanim[index].chanItem.channelNum);
                csChart.data.datasets[index].data.push(
                    {
                        x: chanim_counter,
                        y: chanim_idle[index]
                    }
                );
        }

        // "bgNoise": 0, "channelSpec": 57755, "channelNum": 155, "txDuration": 1, "rxInBss": 1, "rxOutBss": 1, "mediumBusy": 3, "mediumIdle": 97, "mediumAvail": 61, "mediumNonWifi": 0
        index = chanim.length - 1; // pick 155 for now
        var rx = chanim[index].chanItem.rxInBss /* + chanim[index].chanItem.rxOutBss */;
        var tx = chanim[index].chanItem.txDuration;
        // console.log("updating data to " + "tx=" + tx + " rx=" + rx + " chan=" + chanim[index].chanItem.channelNum);
        window.chartTxRx.data.datasets[0].data[0] = rx;
        window.chartTxRx.data.datasets[0].data[1] = tx;
        window.chartTxRx.update();

        var nwifi = chanim[index].chanItem.mediumNonWifi;
        var wifi = chanim[index].chanItem.rxInBss + chanim[index].chanItem.txDuration;
        window.chartNonWifi.data.datasets[0].data[0] = wifi;
        window.chartNonWifi.data.datasets[0].data[1] = nwifi;
        window.chartNonWifi.data.datasets[0].data[2] = chanim[index].chanItem.rxOutBss;
        window.chartNonWifi.update();

        chanim_counter++;
        window.myLine.update();
    }
}
