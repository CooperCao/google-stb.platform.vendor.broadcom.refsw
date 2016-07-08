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
******************************************************************************/
#ifndef __BTHERMAL_H__
#define __BTHERMAL_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#define X_COORDINATE_BASE            25
#define THERMOMETER_COLOR_BACKGROUND "#e0e0e0"
#define THERMOMETER_COLOR_NORMAL     "#00ff00"
#define THERMOMETER_COLOR_WARNING    "#ff9333"
#define THERMOMETER_COLOR_CRITICAL   "#ff0000"
#define DEGREES_PER_PIXEL            6 /* 3 */
#define DEGREES_PER_TICK             10 /* 20 */
#define THERMOMETER_MAX_CELCIUS      131 /*266*/

typedef enum
{
    BTHERMAL_CELCIUS,
    BTHERMAL_FAHRENHEIT,
    BTHERMAL_SCALE_MAX
} BTHERMAL_SCALE;

typedef struct
{
    unsigned int degrees_per_pixel;
    unsigned int degrees_per_tick;
    unsigned int max_degrees;
    unsigned int freezing;
} BTHERMAL_SETTINGS;

static BTHERMAL_SETTINGS bthermal_settings[BTHERMAL_SCALE_MAX] = { {DEGREES_PER_PIXEL,DEGREES_PER_TICK,THERMOMETER_MAX_CELCIUS,0},{3,20,295,32}};

#define SYS_PATH "/sys/devices/virtual/thermal/"
#define BTHERMAL_TRIP_POINT_MAX 3
/* each trip_point has a hysterisus, type (passive/active), and temperature */
typedef struct
{
    int  temperature;
    char type[12];
} BTHERMAL_TRIP_POINT;
/* each thermal_zone has a mode (enabled/disabled), type (cpu-thermal), policy (user_space), temperature, and maybe some trip_points */
typedef struct
{
    int  mode;
    int  num_trip_points;
    int  temperature;
    char type[32];
    BTHERMAL_TRIP_POINT trip_point[BTHERMAL_TRIP_POINT_MAX];
} BTHERMAL_THERMAL_ZONE;
typedef struct
{
    int cur_state;
    int max_state;
    char type[32]; /* intel_powerclamp */
} BTHERMAL_COOLING_DEVICE;
#define BTHERMAL_THERMAL_ZONE_MAX 3
#define BTHERMAL_COOLING_DEVICE_MAX 3
typedef struct
{
    int                     num_thermal_zones;
    int                     num_cooling_devices;
    BTHERMAL_THERMAL_ZONE   thermal_zone[BTHERMAL_THERMAL_ZONE_MAX];
    BTHERMAL_COOLING_DEVICE cooling_device[BTHERMAL_COOLING_DEVICE_MAX];
} BTHERMAL_DEVICE;

int bthermal_output_svg_beginning( BTHERMAL_SCALE scale );
int bthermal_output_svg_ending( void );
int bthermal_output_tick_marks( BTHERMAL_SCALE scale );
int bthermal_adjust_mercury( int degrees_celcius, BTHERMAL_SCALE scale );
int bthermal_output_trip_point( int trip_point_num, int degrees_celcius, BTHERMAL_SCALE scale );
float bthermal_get_temperature( unsigned int thermal_zone );
int bthermal_convert_to_scale( int degrees_celcius, BTHERMAL_SCALE scale );
int bthermal_get_information( BTHERMAL_DEVICE *bthermal_device );

#endif /*__BTHERMAL_H__*/
