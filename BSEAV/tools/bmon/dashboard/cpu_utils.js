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
 *****************************************************************************/

var cpu_idle_previous = [0, 0, 0, 0]; /* needed to compute the delta between this second and last second */
var cpuUpdateInProgress = [0, 0, 0, 0]; /* during the 2 seconds it takes to deactivate/activate a cpu, do not change the button status */
var uptime_msec = "";
var uptime_msec_delta = 0;
var uptime_msec_baseline = 0;
var uptime_msec_previous = 0;
var cpu_idle_average_previous = 0;

var cpuUsageLongAverage = [0,0,0,0,0,0,0,0,0,0]; // 10-second average CPU utilization
var CPU_USAGE_LONG_AVERAGE_MAX = 5; // 10-second window
var cpuUsageLongAverageIdx = 0; // index into 10-second average CPU utilization array (cpuUsageLongAverage)
var cpuUsageLongAverageCount = 0; // number of 10-second average CPU utilization array (cpuUsageLongAverage)

function computeCpuUtilization(jsondata, jsonDatasets) {
    var cpu_utilization = "0.5";
    var cpu_idle_average = 0;
    var cpu_idle_average_delta = 0;
    var num_data_points = 0;
    var cpu_idle_all_cpus = 0;
    var cpu_active_count = 0;
    var cpuobj = 0;
    var idx = 0;

    if (jsondata == 0) {
        return cpu_utilization;
    }

    if (uptime_msec_baseline == 0) {
        uptime_msec_baseline = uptime_msec;
    }

    for (idx = 0; idx < jsondata.length; idx++) {
        if (idx == 0) /* this is the first of two structures in the object */
        {
            uptime_msec = jsondata[idx]['uptime_msec'];
            uptime_msec_delta = Number((uptime_msec - uptime_msec_previous) / 1000);
            /* if nearly a full second has not gone by, assume we had two requests within a second */
            if (uptime_msec_delta < 0.50) {
                //console.log( "computeCpuUtilization .. uptime_msec_delta " + uptime_msec_delta );
                return cpu_utilization;
            }
        } else if (idx == 1) /* this is the structure that contains an array of elements for each CPU */
        {
            var msg = "";
            var cpu_total_idx = 0;

            cpuobj = jsondata[idx].cpus;

            /* loop through each cpu and check to see if cpu is active */
            for (var cpu = 0; cpu < cpuobj.length; cpu++) {
                if (cpuobj[cpu].active) {
                    var cpu_idle = Number(cpuobj[cpu].idle);

                    cpu_idle_average_delta = Number(cpu_idle - cpu_idle_previous[cpu]);
                    cpu_utilization = Math.max(Number(100 - (cpu_idle_average_delta / uptime_msec_delta)).toFixed(0), 0);
                    //console.log( "cpu " + cpu + " - " + cpu_utilization );

                    /* when this function is called from index.html, we only want the cpu utilization ... do not update any datasets */
                    if (jsonDatasets) {
                        var cpu_total_idx = jsonDatasets.length - 1;
                        num_data_points = jsonDatasets[cpu].data.length;
                        if (num_data_points >= 10) {

                            /* drop off the left-most element in the array */
                            jsonDatasets[cpu].data.shift();

                            // also need to shift the Total line ... it is the last element in the array
                            if (cpu == 0) {
                                if (cpu_total_idx > 0) {
                                    jsonDatasets[cpu_total_idx].data.shift();
                                }
                            }
                        }

                        /* add the new element to the end of the array */
                        jsonDatasets[cpu].data.push( cpu_utilization );
                        msg += cpu_utilization + ",";
                    }

                    cpu_idle_all_cpus += cpu_idle;
                    cpu_active_count++;

                    cpu_idle_previous[cpu] = cpu_idle;
                }
            }
            if (cpuobj.length && cpu_active_count) {
                cpu_idle_average = cpu_idle_all_cpus / cpu_active_count;
            }

            cpu_idle_average_delta = Number(cpu_idle_average - cpu_idle_average_previous);
            cpu_utilization = Number(100 - (cpu_idle_average_delta / uptime_msec_delta)).toFixed(1);
            if (Number(cpu_utilization) <= 0) {
                cpu_utilization = "0.5";
            } else if (Number(cpu_utilization) >= 100) {
                /* we don't want decimal place when 100% */
                cpu_utilization = "100";
            }

            //console.log( msg + cpu_utilization);

            /* update the total utilization for all cpus */
            if (jsonDatasets) {
                jsonDatasets[cpu_total_idx].data.push(Number(cpu_utilization).toFixed(0));
            }
        }
    }

    uptime_msec_previous = uptime_msec;
    cpu_idle_average_previous = cpu_idle_average;

    msg = "";
    cpuUsageLongAverage[cpuUsageLongAverageIdx] = (cpu_utilization );
    var avg = 0;
    for(idx=0; idx<CPU_USAGE_LONG_AVERAGE_MAX ; idx++ ) {
        avg += Number( cpuUsageLongAverage[idx] );
        msg += cpuUsageLongAverage[idx] + ", ";
    }
    avg = Math.round(avg/CPU_USAGE_LONG_AVERAGE_MAX);
    msg += avg;

    //console.log( cpu_utilization + "%,     " + msg + ", " + cpuUsageLongAverageIdx );

    cpuUsageLongAverageIdx++; // increment index into array ... wrapping around to 0 if it gets too big
    if ( cpuUsageLongAverageIdx >= CPU_USAGE_LONG_AVERAGE_MAX ) cpuUsageLongAverageIdx = 0;
    cpuUsageLongAverageCount++; // Count is used to only compute average for 2 cycles if we have only been collecting values for 2 seconds

    return cpu_utilization /* avg */;
}

/*
    The state of the CPU buttons needs to change based on some external alteration of the actual CPU
    state. Someone could turn a CPU on or off from the command line or another app. When that happens,
    the button's state needs to change to match the actual CPU state.

    For example:  echo "0" > /sys/devices/system/cpu/cpu2/online
*/
function updateCpuButtons( myObj_data )
{
    var cpu = 0;
    var cpuButtonObj = 0;

    if ( myObj_data && myObj_data.length > 1 && myObj_data[1].cpus && myObj_data[1].cpus.length ) {
        var cpuState = "off";
        var isChecked = false;
        for(cpu=0; cpu<myObj_data[1].cpus.length; cpu++ ) {
            if ( myObj_data[1].cpus[cpu].active ) {
                cpuState = "on";
            } else {
                cpuState = "off";
            }
            cpuButtonObj = $('#cpu-deactivate-'+cpu);
            if ( cpuButtonObj ) {
                isChecked = cpuButtonObj.is(':checked');

                /* It takes 1-2 seconds to transition to the opposite state; do not change the button status until transition completes */
                if ( cpuUpdateInProgress[cpu] > 0 ) {
                    //console.log( "cpu " + cpu + " .. waiting .. " + cpuUpdateInProgress[cpu] );
                    cpuUpdateInProgress[cpu]--;
                } else {
                    if ( (isChecked && cpuState == "off" ) || ( !isChecked && cpuState == "on" ) ) {
                        var toggle = cpuButtonObj.data('bs.toggle');
                        if ( cpuState == "on" ) {
                            toggle.on(true);
                            //toggle.off(false);
                        } else {
                            //toggle.on(false);
                            toggle.off(true);
                        }
                    }
                }
            }
        }
    }
}
