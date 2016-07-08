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
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "bthermal.h"

/* copied from tmon.c */
/**
 *  Function: This function will read an "signed long" integer value from the
 *  specified filename that is prepended with the specified path.
 **/
static int sysfs_get_dlong(
    char *path,
    char *filename,
    long *p_dlong
    )
{
    FILE *fd;
    int   ret = -1;
    char  filepath[256];

    snprintf( filepath, 256, "%s%s", path, filename );

    fd = fopen( filepath, "r" );
    /*printf("%s: filepath (%s); fd %p\n", __FUNCTION__, filepath, (void*)fd );*/
    if (!fd)
    {
        return( ret );
    }
    ret = fscanf( fd, "%ld", p_dlong );
    fclose( fd );

    return( 0 );
}                                                          /* sysfs_get_dlong */

/* copied from tmon.c */
/**
 *  Function: This function will read a "string" from the
 *  specified filename that is prepended with the specified path.
 **/
static int sysfs_get_string(
    char *path,
    char *filename,
    char *str
    )
{
    FILE *fd;
    int   ret = -1;
    char  filepath[256];

    snprintf( filepath, 256, "%s/%s", path, filename );

    fd = fopen( filepath, "r" );
    if (!fd)
    {
        return( ret );
    }
    ret = fscanf( fd, "%256s", str );
    fclose( fd );

    return( ret );
}                                                          /* sysfs_get_string */

/**
 *  Function: This function will read the temperature from the specified thermal zone.
 *  Thermal zones are generally numbered from 0 to 1.
 **/
float bthermal_get_temperature(
    unsigned int thermal_zone
    )
{
    int   rc           = 0;
    long  iTemperature = 0;
    float fTemperature = -999.0;
    char  thermal_zone_fn[64];
    char  thermal_mode[16];

    memset( thermal_mode, 0, sizeof( thermal_mode ));

    sprintf( thermal_zone_fn, "thermal_zone%u/mode", thermal_zone );
    rc = sysfs_get_string( SYS_PATH, thermal_zone_fn, thermal_mode );
    /*printf("%s: get_string returned (%d); mode (%s)\n", __FUNCTION__, rc, thermal_mode );*/
    if (rc != -1)
    {
        /* if the zone is enabled and active */
        if (strcmp( thermal_mode, "enabled" ) == 0)
        {
            sprintf( thermal_zone_fn, "thermal_zone%u/temp", thermal_zone );
            rc = sysfs_get_dlong( SYS_PATH, thermal_zone_fn, &iTemperature );
            if (rc != -1)
            {
                fTemperature = (float)( iTemperature/1000 ) + ((float)( iTemperature%1000 ))/1000.;
                /*printf("iTemperature %ld; fTemperature is %5.1f; tz_fn (%s/%s) \n", iTemperature, fTemperature, SYS_PATH, thermal_zone_fn );*/
            }
        }
    }

    return( fTemperature /* to test trip_point colors ... + 56. + 15.*/ );
}                                                          /* bthermal_get_temperature */

/**
 *  Function: This function will read all of the important values from all of the discoverable
 *  thermal zones and cooling devices. Each thermal zone could have zero or more trip points.
 *  The 7271 has three trip points. Each trip point will have a hysterisus, a type, and a
 *  temperature. A thermal zone could be disabled in which case the temperature value could
 *  either come back as -1 or you could get an error reading the "temp" file.
 **/
int bthermal_get_information(
    BTHERMAL_DEVICE *bthermal_devices
    )
{
    int          rc             = 0;
    unsigned int trip_point     = 0;
    unsigned int thermal_zone   = 0;
    unsigned int cooling_device = 0;
    long         iTemperature   = 0;
    /*float        fTemperature   = 0.0;*/
    char         filename[64];
    char         thermal_zone_fn[64];
    char         thermal_zone_type[64];
    char         thermal_zone_mode[64];
    char         cooling_device_fn[64];
    char         cooling_device_type[64];
    char         trip_point_type[64];

    if (bthermal_devices == NULL)
    {
        return( -1 );
    }

    for (cooling_device = 0; cooling_device<10; cooling_device++)
    {
        /* /sys/devices/virtual/thermal/cooling_device0/ */
        sprintf( cooling_device_fn, "%scooling_device%u/", SYS_PATH, cooling_device );
        rc = sysfs_get_string( cooling_device_fn, "type", cooling_device_type );
        /*printf("sysfs_get_string(%s/type) ... rc %d \n", cooling_device_fn, rc );*/
        if (( rc != -1 ) && ( bthermal_devices->num_cooling_devices < BTHERMAL_COOLING_DEVICE_MAX ))
        {
            int cd = bthermal_devices->num_cooling_devices;
            bthermal_devices->num_cooling_devices++;

            strncpy( bthermal_devices->cooling_device[cd].type, cooling_device_type, sizeof( bthermal_devices->cooling_device[cd].type ) - 1 );
            /*printf( "%s - %s", cooling_device_fn, cooling_device_type );*/
            sysfs_get_dlong( cooling_device_fn, "cur_state", &iTemperature );
            /*printf( " ... cur_state %ld", iTemperature );*/
            bthermal_devices->cooling_device[cd].cur_state = iTemperature;
            sysfs_get_dlong( cooling_device_fn, "max_state", &iTemperature );
            /*printf( " ... max_state %ld \n", iTemperature );*/
            bthermal_devices->cooling_device[cd].max_state = iTemperature;
        }
    }

    for (thermal_zone = 0; thermal_zone<10; thermal_zone++)
    {
        sprintf( thermal_zone_fn, "thermal_zone%u/temp", thermal_zone );
        rc = sysfs_get_dlong( SYS_PATH, thermal_zone_fn, &iTemperature );
        if (( rc != -1 ) && ( bthermal_devices->num_thermal_zones < BTHERMAL_THERMAL_ZONE_MAX ))
        {
            int tz = bthermal_devices->num_thermal_zones;
            bthermal_devices->num_thermal_zones++;

            bthermal_devices->thermal_zone[tz].temperature = iTemperature;
            #if 0
            fTemperature = (float)( iTemperature/1000 ) + ((float)( iTemperature%1000 ))/1000.;
            printf("iTemperature is %ld; fTemperature is %5.1f \n", iTemperature, fTemperature );
            printf( "fTemperature is %5.1f \n", fTemperature );
            #endif

            /* determine if this thermal_zone has any trip_points */
            for (trip_point = 0; trip_point<10; trip_point++)
            {
                sprintf( thermal_zone_fn, "%sthermal_zone%u/", SYS_PATH, thermal_zone );
                if (trip_point == 0)
                {
                    sysfs_get_string( thermal_zone_fn, "mode", thermal_zone_mode );
                    sysfs_get_string( thermal_zone_fn, "type", thermal_zone_type );
                    /*printf( "%s ... type = %s ... mode = %s \n", thermal_zone_fn, thermal_zone_type, thermal_zone_mode );*/
                }

                sprintf( filename, "trip_point_%u_temp", trip_point );
                /*printf("checking (%s/%s) \n", thermal_zone_fn, filename );*/
                rc = sysfs_get_dlong( thermal_zone_fn, filename, &iTemperature );
                if (( rc == 0 ) && ( bthermal_devices->thermal_zone[tz].num_trip_points < BTHERMAL_TRIP_POINT_MAX ))
                {
                    int tp = bthermal_devices->thermal_zone[tz].num_trip_points;
                    bthermal_devices->thermal_zone[tz].num_trip_points++;

                    /*printf( "%s ... (%s) = %6ld ... type = ", thermal_zone_fn, filename, iTemperature );*/
                    sprintf( filename, "trip_point_%u_type", trip_point );
                    rc = sysfs_get_string( thermal_zone_fn, filename, trip_point_type );
                    /*printf( "%s ... num %d \n", trip_point_type, bthermal_devices->thermal_zone[tz].num_trip_points );*/
                    strncpy( bthermal_devices->thermal_zone[tz].trip_point[tp].type, trip_point_type,
                        sizeof( bthermal_devices->thermal_zone[tz].trip_point[tp].type ) - 1 );
                    bthermal_devices->thermal_zone[tz].trip_point[tp].temperature = iTemperature;
                }
            }
        }
    }

    return( 0 );
}                                                          /* bthermal_get_information */

/**
 *  Function: This function will output the beginning part of the SVG block of code that will
 *  eventually contain the thermometer readings.
 **/
int bthermal_output_svg_beginning(
    BTHERMAL_SCALE scale
    )
{
    printf( "<svg version=\"1.1\" id=\"main_svg\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n" );
    printf( "    width=\"900px\" height=\"80px\" xml:space=\"preserve\">\n" );
    printf( "    <defs>\n" );
    printf( "       <marker id=\"markerArrow\" markerWidth=\"13\" markerHeight=\"13\" refX=\"2\" refY=\"6\" orient=\"auto\">\n" );
    printf( "          <path d=\"M2,2 L2,11 L10,6 L2,2\" style=\"fill: #000000;\" />\n" );
    printf( "       </marker>\n" );
    printf( "       <marker id=\"Arrow\"  markerWidth=\"6\" markerHeight=\"8\" viewBox=\"-6 -6 12 12\"  refX=\"0\" refY=\"0\"  markerUnits=\"strokeWidth\"  orient=\"auto\" >\n" );
    printf( "          <polygon points=\"-2,0 -5,5 5,0 -5,-5\" fill=\"#0070C0\" stroke=\"#0070C0\" stroke-width=\"1px\" />\n" );
    printf( "       </marker>\n" );
    printf( "    </defs>\n" );
    printf( "    <path d=\"M10,10 L%d,10 L%d,20 L10,20 z\" style=\"fill:%s;fill-opacity:1; stroke:%s; stroke-width:0\" />\n",
        (( bthermal_settings[scale].max_degrees - bthermal_settings[scale].freezing ) * bthermal_settings[scale].degrees_per_pixel ) + X_COORDINATE_BASE + 2,
        (( bthermal_settings[scale].max_degrees - bthermal_settings[scale].freezing ) * bthermal_settings[scale].degrees_per_pixel ) + X_COORDINATE_BASE + 2,
        THERMOMETER_COLOR_BACKGROUND, THERMOMETER_COLOR_BACKGROUND );

    return( 0 );
}                                                          /* bthermal_output_svg_beginning */

/**
 *  Function: This function will output the ending part of the SVG block of code that contains
 *  the thermometer readings.
 **/
int bthermal_output_svg_ending(
    void
    )
{
    printf( "<rect x=\"0px\" y=\"0px\" width=\"900\" height=\"80\" stroke=\"black\" style=\"fill:none;\" >" ); /* border around svg element */
    printf( "</svg>" );

    return( 0 );
}

/**
 *  Function: This function will output the SVG elements that make up the tick marks on the side
 *  of the thermometer. In addition to the tick marks, the SVG elements to label the tick marks
 *  will also be output.
 **/
int bthermal_output_tick_marks(
    BTHERMAL_SCALE scale
    )
{
    int idx      = 0;
    int currentX = 0;
    int textX    = 0;

    /* output tick marks every 10 degrees */
    currentX = X_COORDINATE_BASE;
    textX    = X_COORDINATE_BASE - 3;                      /* text is left-shifted a bit so that it aligns better with the tick */
    printf( "    <path style=\"fill:black;fill-opacity:1; stroke:black; stroke-width:2\" d=\"" );
    for (idx = 0; idx<( bthermal_settings[scale].max_degrees - bthermal_settings[scale].freezing ); idx += bthermal_settings[scale].degrees_per_tick)
    {
        printf( "M%d,30 L%d,20 ", currentX, currentX );
        currentX += bthermal_settings[scale].degrees_per_pixel * bthermal_settings[scale].degrees_per_tick; /* tick marks are this many pixels apart */
    }
    printf( "\" />\n" );

    /* output the text for Celcius ... also used to show DISABLED */
    printf( "    <text x=\"%d\"  y=\"65\" id=\"celcius_mode\" style=\"font-family:Helvetica;\" onclick=\"SvgClick(event);\" >Celcius</text>\n", textX );
    for (idx = bthermal_settings[scale].freezing; idx<bthermal_settings[scale].max_degrees; idx += bthermal_settings[scale].degrees_per_tick)
    {
        if (( idx == 10 ) || ( idx == 32 ))                /* if tick mark is for a two-digit number */
        {
            textX -= 4;                                    /* shift the text a bit more to the left to accommodate 2-digit degrees ... 10, 20, etc. */
        }
        else if (( idx == 100 ) || ( idx == 112 ))         /* if tick mark is for a three-digit number */
        {
            textX -= 8;                                    /* shift the text a bit more to the left to accommodate 3-digit degrees ... 100, 110, etc. */
        }
        printf( "    <text x=\"%d\"  y=\"45\" style=\"font-family:Helvetica;\" >%d</text>\n", textX, idx );
        textX += bthermal_settings[scale].degrees_per_pixel * bthermal_settings[scale].degrees_per_tick; /* tick marks are this many pixels apart */
    }
    printf( "\n" );

    return( 0 );
}                                                          /* bthermal_output_tick_marks */

/**
 *  Function: This function will convert a celcius temperature to fahrenheit if the scale is set
 *  to FAHRENHEIT. If the currently selected "scale" is Celcius, the temperature will be returned
 *  unaltered.
 **/
int bthermal_convert_to_scale(
    int            degrees_celcius,
    BTHERMAL_SCALE scale
    )
{
    int degrees = degrees_celcius;

    /*printf("%s: degrees_celcius %d \n", __FUNCTION__, degrees_celcius );*/
    if (scale == BTHERMAL_FAHRENHEIT)
    {
        degrees = ( degrees * 9 / 5 ) + 32;
    }

    return( degrees - bthermal_settings[scale].freezing );
}

/**
 *  Function: This function will output the SVG elements that represent the "red" mercury in our
 *  digital thermometer. The "mercury" is represented by a circle at the bottom of the thermometer
 *  and a "mercury" colored rectangle that extends out from the mercury circle.
 **/
int bthermal_adjust_mercury(
    int            degrees_celcius,
    BTHERMAL_SCALE scale
    )
{
    int currentX = X_COORDINATE_BASE + ( bthermal_convert_to_scale( degrees_celcius, scale ) * bthermal_settings[scale].degrees_per_pixel ) + 1;

    printf( "<rect id=\"mercury_rect\" x=\"10px\" y=\"10px\" width=\"%d\" height=\"10\" stroke=\"none\" style=\"fill:%s;\" />\n", currentX, THERMOMETER_COLOR_NORMAL );
    printf( "    <circle id=\"mercury_circle\" r=\"10\" cy=\"15\" cx=\"15\" stroke-linecap=\"null\" stroke-linejoin=\"null\" stroke-width=\"0\" stroke=\"#000000\" fill=\"%s\" onclick=\"MyClick();\" />\n",
        THERMOMETER_COLOR_NORMAL );
    return( 0 );
}

/**
 *  Function: This function will output the SVG elements that we use to identify one of the trip
 *  points. The trip point is represented by a "blue" line with an arrow pointing to the temperature
 *  associated with the trip point. A text label (like TP0) will also help identify which trip point
 *  we are dealing with.
 **/
int bthermal_output_trip_point(
    int            trip_point_num,
    int            degrees_celcius,
    BTHERMAL_SCALE scale
    )
{
    int currentX = X_COORDINATE_BASE + ( bthermal_convert_to_scale( degrees_celcius, scale ) * bthermal_settings[scale].degrees_per_pixel );

    /*printf( "<!-- tpnum %d; degrees %d; scale %d -->\n", trip_point_num, degrees_celcius, scale );*/

    printf( "    <line x1=\"%d\" y1=\"50\" x2=\"%d\" y2=\"25\" style=\"stroke:#0070C0; stroke-width:2; marker-end:url(#Arrow)\" /><!-- trip_point_0 -->\n",
        currentX, currentX );
    printf( "    <text x=\"%d\" y=\"65\" style=\"font-family:Helvetica;stroke:#0070C0; stroke-width:2; \" >TP%d</text>\n", currentX - 15, trip_point_num );
    return( 0 );
}
