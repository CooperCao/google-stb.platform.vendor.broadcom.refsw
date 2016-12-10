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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "bsysperf_wifi.h"
#include "bmemperf_lib.h"
#include "bmemperf_utils.h"

#define FILENAME_WIFI_RECORD "wifi_record.txt"
#ifndef MAX
#define MAX( a, b ) (( a>b ) ? a : b )
#define MIN( a, b ) (( a<b ) ? a : b )
#endif

/* trellis/connectivity/netapp/netapp/include/netapp.h */
typedef enum
{
    NETAPP_SUCCESS = 0,                                    //!< Success.
    NETAPP_FAILURE,                                        //!< General failure.
    NETAPP_INVALID_PARAMETER,                              //!< Invalid parameter.
    NETAPP_NULL_PTR,                                       //!< Null handle detected or invalid state.
    NETAPP_OUT_OF_MEMORY,                                  //!< Malloc has failed.
    NETAPP_NOT_IMPLEMENTED,                                //!< Function not implemented.
    NETAPP_NETWORK_UNREACHABLE,                            //!< Unable to reach destination network.
    NETAPP_SOCKET_ERROR,                                   //!< Error creating the Linux socket.
    NETAPP_TIMEOUT,                                        //!< Timeout error occurred.
    NETAPP_DHCP_FAILURE,                                   //!< Failure to fetch DHCPD address.
    NETAPP_HOST_NOT_FOUND,                                 //!< Not able to find host in DNS server.
    NETAPP_CANCELED,                                       //!< The function was canceled.
    NETAPP_INCORRECT_PASSWORD,                             //!< Incorrect password provided.
    NETAPP_INVALID_PIN,                                    //!< Invalid WPS pin used.
    NETAPP_NOT_FOUND,                                      //!< Requested item was not found.
    NETAPP_NOT_SUPPORTED,                                  //!< Requesting an API or function that was
                                                           //!< not supported/compiled in.
    NETAPP_WPS_MULTIPLE_AP_FOUND,                          //!< Found more than one AP in WPS PBC (overlap).
    NETAPP_SCAN_EMPTY,                                     //!< Scan was complete and no access points found.
    NETAPP_INVALID_STATE,                                  //!< Calling the API when the system is in an invalid state.
    NETAPP_WPS_2_ERR_INCOMPATIBLE,                         //!< WPS detected an AP that supports a WPS 1.0
                                                           //!< depreciated setting that is not supported in
                                                           //!< WPS 2.0. The application should restart WPS
                                                           //!< with NETAPP_SETTINGS.bWPS2_0 set to false.
    NETAPP_WFD_SESSION_NOT_AVAILABLE,                      //!< WFD Session is not available, reported from a
                                                           //!< P2P connection attempt that failed because the
                                                           //!< peer is not ready to connect.
    NETAPP_DATABASE_ERROR                                  //!< There was a problem reading or writing to the
                                                           //!< backend database, file could be corrupted.
} NETAPP_RETCODE;

/* from trellis/connectivity/netapp/netapp/src/netapp_bwl.h */
#define BWL_API_CHECK( api, tRetCode )                                              \
    {                                                                               \
        int result = api;                                                           \
        switch (result)                                                             \
        {                                                                           \
            case 0:                                                                 \
                tRetCode = NETAPP_SUCCESS;                                          \
                break;                                                              \
            case BWL_ERR_PARAM:                                                     \
                tRetCode = NETAPP_INVALID_PARAMETER;                                \
                break;                                                              \
            case BWL_ERR_ALLOC:                                                     \
                tRetCode = NETAPP_OUT_OF_MEMORY;                                    \
                break;                                                              \
            case BWL_ERR_CANCELED:                                                  \
                tRetCode = NETAPP_CANCELED;                                         \
                break;                                                              \
            default:                                                                \
                printf( "%s BWL API failed, Line=%d! \n", __FUNCTION__, __LINE__ ); \
                tRetCode = NETAPP_FAILURE;                                          \
                break;                                                              \
        }                                                                           \
        if (tRetCode != NETAPP_SUCCESS)                                             \
        {                                                                           \
            goto err_out;                                                           \
        }                                                                           \
    }

#define BSYSPERF_WIFI_INIT_RETRY 5

static BWL_Handle hBwl = NULL;

typedef enum
{
    NETAPP_WIFI_RSSI_NONE = 0,                             //!< No signal (0 bar)
    NETAPP_WIFI_RSSI_POOR,                                 //!< Poor (1 bar)
    NETAPP_WIFI_RSSI_FAIR,                                 //!< Fair (2 bars)
    NETAPP_WIFI_RSSI_GOOD,                                 //!< Good (3 bars)
    NETAPP_WIFI_RSSI_EXCELLENT,                            //!< Excellent (4 bars)
    NETAPP_WIFI_RSSI_MAX
} NETAPP_WIFI_RSSI;

typedef enum {
    NETAPP_WIFI_DRIVER_FALCON = 0,
    NETAPP_WIFI_DRIVER_KIRIN,
    NETAPP_WIFI_DRIVER_AARDVARK,
    NETAPP_WIFI_DRIVER_PHOENIX2,
    NETAPP_WIFI_DRIVER_DHD,
    NETAPP_WIFI_DRIVER_EAGLE,
    NETAPP_WIFI_DRIVER_BISON,
    NEXUS_WIFI_DRIVER_MAX
} NETAPP_WIFI_DRIVER;

#if 0
/* RSSI Levels */
static int32_t lRSSILevels[NEXUS_WIFI_DRIVER_MAX][NETAPP_WIFI_RSSI_MAX] =
{
    /* NONE,    POOR,    FAIR,   GOOD,  EXCELLENT */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_FALCON */
    {-90, -70, -60, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_KIRIN */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_AARDVARK */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_PHOENIX2 */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_DHD */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_EAGLE */
    {-90, -80, -70, -50,      -40     },                   /* NETAPP_WIFI_DRIVER_BISON */
};
#endif /* if 0 */

int Bsysperf_WifiInit(
    const char *ifname
    )
{
    int i = 0;

    /* Try to init the BWL interface */
    for (i = 0; i < BSYSPERF_WIFI_INIT_RETRY && hBwl == NULL; i++)
    {
        if (BWL_Init( &hBwl, (char *) ifname ) == BWL_ERR_SUCCESS)
        {
            break;
        }
        usleep( 1000000/10 );                              /* sleep a tenth of a second */
    }

    return( 0 );
}

int Bsysperf_WifiUninit(
    void
    )
{
    #if 0
    BWL_Uninit( hBwl );
    hBwl = NULL;
    #endif

    return( 0 );
}

int Bsysperf_WifiGetRevs(
    const char *ifname,
    RevInfo_t  *tRevInfo
    )
{
    int i        = 0;
    int tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            #if 0
            RevInfo_t tRevInfo;
            #endif

            noprintf( "BWL_Init() succeeded after %d tries \n", i+1 );

            memset( tRevInfo, 0, sizeof( RevInfo_t ));
#if 0
            BWL_API_CHECK( BWL_SetRSSIEventLevels( hBwl, lRSSILevels[hNetAppWiFi->tDriverName],
                    sizeof( lRSSILevels[hNetAppWiFi->tDriverName] )/sizeof( lRSSILevels[hNetAppWiFi->tDriverName][0] )), tRetCode );
#endif
            BWL_API_CHECK( BWL_SetEvent( hBwl, BWL_E_RSSI ), tRetCode );
            BWL_API_CHECK( BWL_SetEvent( hBwl, BWL_E_ASSOC ), tRetCode );
            BWL_API_CHECK( BWL_SetEvent( hBwl, BWL_E_DISASSOC ), tRetCode );
            BWL_API_CHECK( BWL_SetEvent( hBwl, BWL_E_LINK ), tRetCode );
            BWL_SetOBSSCoEx( hBwl, true );

            BWL_API_CHECK( BWL_GetRevInfo( hBwl, tRevInfo ), tRetCode );
            #if 0
            printf( "******************************* \n" );
            printf( "WiFi Driver Version Info: \n" );
            printf( "Chip Number:       %d (%x) \n", tRevInfo.ulChipNum, tRevInfo.ulChipNum );
            printf( "Driver Version:    %d.%d.%d.%d \n",
                (uint8_t)( tRevInfo.ulDriverRev>>24 ),
                (uint8_t)( tRevInfo.ulDriverRev>>16 ),
                (uint8_t)( tRevInfo.ulDriverRev>>8 ),
                (uint8_t)tRevInfo.ulDriverRev );
            printf( "PCI vendor id:     0x%08x \n", tRevInfo.ulVendorId );
            printf( "Device id of chip: 0x%08x \n", tRevInfo.ulDeviceId );
            printf( "Radio Revision:    0x%08x \n", tRevInfo.ulRadioRev );
            printf( "Chip Revision:     0x%08x \n", tRevInfo.ulChipRev );
            printf( "Core Revision:     0x%08x \n", tRevInfo.ulCoreRev );
            printf( "Board Identifier:  0x%08x \n", tRevInfo.ulBoardId );
            printf( "Board Vendor:      0x%08x \n", tRevInfo.ulBoardVendor );
            printf( "Board Revision:    0x%08x \n", tRevInfo.ulBoardRev );
            printf( "Microcode Version: 0x%08x \n", tRevInfo.ulUcodeRev );
            printf( "Bus Type:          0x%08x \n", tRevInfo.ulBus );
            printf( "Phy Type:          0x%08x \n", tRevInfo.ulPhyType );
            printf( "Phy Revision:      0x%08x \n", tRevInfo.ulPhyRev );
            printf( "Anacore Rev:       0x%08x \n", tRevInfo.ulAnaRev );
            printf( "Chip Package Info: 0x%08x \n", tRevInfo.ulChipPkg );
            #endif /* if 0 */
        }
        Bsysperf_WifiUninit();
    } while (0);

err_out:
    return( tRetCode );
}                                                          /* Bsysperf_WifiGetRevs*/

#define BWL_WIFI_SCAN_WAIT_DURATION 1
#define BWL_WIFI_SCAN_TIMEOUT       20000
uint32_t Bsysperf_WifiScanStart(
    const char *ifname
    )
{
    int      tRetCode = -1;
    uint32_t ulCount  = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            unsigned int uRetryCount = 0;
            uint32_t     ulScanType  = BWL_E_SCAN_COMPLETE /*BWL_E_SCAN_CONFIRM_IND   BWL_E_PFN_SCAN_COMPLETE   */;
            ScanParams_t tScanParam;

            do
            {
                uRetryCount++;

                memset( &tScanParam, 0, sizeof( tScanParam ));

                BWL_API_CHECK( BWL_SetEvent( hBwl, ulScanType ), tRetCode );
                BWL_API_CHECK( BWL_Scan( hBwl, &tScanParam ), tRetCode );
                noprintf( "%s:%u: sleep(%d) \n", __FILE__, __LINE__, BWL_WIFI_SCAN_WAIT_DURATION );
                sleep( BWL_WIFI_SCAN_WAIT_DURATION );
                #if 0
                {
                    unsigned char  evBuffer[1024];
                    EventMessage_t wlEvent;
                    BWL_API_CHECK( BWL_ParseEvent( hBwl, evBuffer, sizeof( evBuffer ), &wlEvent ), tRetCode );
                    if (tRetCode != 0)
                    {
                        printf( "%s:%u: BWL_ParseEvent() failed with rc %d \n", __FILE__, __LINE__, tRetCode );
                        break;
                    }
                }
                #endif /* if 0 */

                BWL_API_CHECK( BWL_GetScannedApNum( hBwl, &ulCount ), tRetCode );
                if (ulCount <= 0)
                {
                    ulCount = 0;
                    printf( "%s:%u: Could not find any scanned devices; retryCount %d \n", __FILE__, __LINE__, uRetryCount );
                    if (uRetryCount > 9)
                    {
                        printf( "scanned devices retryCount %d max'ed out.\n", uRetryCount );
                        break;
                    }
                }
                else
                {
                    printf( "%s:%u: BWL_GetScannedApNum() returned ulCount %lu \n", __FILE__, __LINE__, (unsigned long int) ulCount );
                    break;
                }
            } while (uRetryCount < 9);
        }
        Bsysperf_WifiUninit();
    } while (0);

err_out:
    return( ulCount );
}                                                          /* Bsysperf_WifiScanStart */

int Bsysperf_WifiScanGetResults(
    const char *ifname
    )
{
    int tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            BWL_API_CHECK( BWL_EScanResultsWlExe( hBwl ), tRetCode );
        }
        Bsysperf_WifiUninit();
    } while (0);

err_out:
    return( tRetCode );
}                                                          /* Bsysperf_WifiGetScanResults */

int Bsysperf_WifiGetConnectedInfo(
    const char *ifname,
    ScanInfo_t *tScanInfo
    )
{
    int i        = 0;
    int tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s:%u: Failed to Init BWL! \n", __FILE__, __LINE__ );
            tRetCode = -1;
            break;
        }
        else
        {
            noprintf( "BWL_Init() succeeded after %d tries \n", i+1 );

            if (( tRetCode = BWL_GetConnectedInfo( hBwl, tScanInfo )) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
                /*printf("~FATAL~BWL_GetConnectedInfo() failed with error %d~", tRetCode );*/
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}                                                          /* Bsysperf_WifiGetConnectedInfo */

int Bsysperf_WifiGetRate(
    const char *ifname,
    int        *pRate
    )
{
    int tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetRate( hBwl, pRate ) == BWL_ERR_SUCCESS)
            {
                /*printf("rate is (%d) \n", *pRate );*/
            }
            else
            {
                /*printf("~FATAL~BWL_GetRate() failed with error %d~", tRetCode );*/
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}                                                          /* Bsysperf_WifiGetRate */

static char *strBandwidth[6] = {"INVALID", "10MHz", "20MHz", "40MHz", "80MHz", "160MHz"};
char *Bsysperf_WifiBandwidthStr(
    Bandwidth_t bandwidth
    )
{
    return( strBandwidth[bandwidth] );
}

#if 0
/* NONE,    POOR,    FAIR,   GOOD,  EXCELLENT */
{-90,      -80,      -70,    -50,  -40},                   /* NETAPP_WIFI_DRIVER_EAGLE */
#endif
char *Bsysperf_WifiDbStrength(
    int signal_strength
    )
{
    if (( signal_strength < -90 ) || ( signal_strength == 0 ))
    {
        return( "None" );
    }
    else if (( -90 <= signal_strength ) && ( signal_strength < -80 ))
    {
        return( "Poor" );
    }
    else if (( -80 <= signal_strength ) && ( signal_strength < -70 ))
    {
        return( "Fair" );
    }
    else if (( -70 <= signal_strength ) && ( signal_strength < -50 ))
    {
        return( "Good" );
    }

    return( "Excellent" );
}                                                          /* Bsysperf_WifiDbStrength */

int Bsysperf_WifiGetCounters(
    const char     *ifname,
    WiFiCounters_t *pCounters                              /* Counters structure */
    )
{
    int tRetCode = 0;

    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetCounters( hBwl, pCounters ) == BWL_ERR_SUCCESS)
            {
            }
            else
            {
                printf( "%s:%u: BWL_GetCounters() failed with error %d~", __FILE__, __LINE__, tRetCode );
            }
        }

        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}                                                          /* Bsysperf_WifiGetCounters */

int Bsysperf_WifiGetNRate(
    const char *ifname,
    uint32_t   *pNRate
    )
{
    int tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (BWL_GetNRate( hBwl, pNRate ) == BWL_ERR_SUCCESS)
            {
                /*printf("rate is (%d) \n", *pNRate );*/
            }
            else
            {
                /*printf("~FATAL~BWL_GetRate() failed with error %d~", tRetCode );*/
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}                                                          /* Bsysperf_WifiGetNRate */

/* WL_RSPEC defines for rate information */
#define WL_RSPEC_RATE_MASK     0x000000FF                  /* rate or HT MCS value */
#define WL_RSPEC_VHT_MCS_MASK  0x0000000F                  /* VHT MCS value */
#define WL_RSPEC_VHT_NSS_MASK  0x000000F0                  /* VHT Nss value */
#define WL_RSPEC_VHT_NSS_SHIFT 4                           /* VHT Nss value shift */
#define WL_RSPEC_TXEXP_MASK    0x00000300
#define WL_RSPEC_TXEXP_SHIFT   8
#define WL_RSPEC_BW_MASK       0x00070000                  /* bandwidth mask */
#define WL_RSPEC_BW_SHIFT      16                          /* bandwidth shift */
#define WL_RSPEC_STBC          0x00100000                  /* STBC encoding, Nsts = 2 x Nss */
#define WL_RSPEC_TXBF          0x00200000                  /* bit indicates TXBF mode */
#define WL_RSPEC_LDPC          0x00400000                  /* bit indicates adv coding in use */
#define WL_RSPEC_SGI           0x00800000                  /* Short GI mode */
#define WL_RSPEC_ENCODING_MASK 0x03000000                  /* Encoding of Rate/MCS field */
#define WL_RSPEC_OVERRIDE_RATE 0x40000000                  /* bit indicate to override mcs only */
#define WL_RSPEC_OVERRIDE_MODE 0x80000000                  /* bit indicates override both rate & mode */

/* WL_RSPEC_ENCODING field defs */
#define WL_RSPEC_ENCODE_RATE 0x00000000                    /* Legacy rate is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_HT   0x01000000                    /* HT MCS is stored in RSPEC_RATE_MASK */
#define WL_RSPEC_ENCODE_VHT  0x02000000                    /* VHT MCS and Nss is stored in RSPEC_RATE_MASK */

/* WL_RSPEC_BW field defs */
#define WL_RSPEC_BW_UNSPECIFIED 0
#define WL_RSPEC_BW_20MHZ       0x00010000
#define WL_RSPEC_BW_40MHZ       0x00020000
#define WL_RSPEC_BW_80MHZ       0x00030000
#define WL_RSPEC_BW_160MHZ      0x00040000
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WL_RSPEC_BW_10MHZ  0x00050000
#define WL_RSPEC_BW_5MHZ   0x00060000
#define WL_RSPEC_BW_2P5MHZ 0x00070000
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* Legacy defines for the nrate iovar */
#define OLD_NRATE_MCS_INUSE         0x00000080             /* MSC in use,indicates b0-6 holds an mcs */
#define OLD_NRATE_RATE_MASK         0x0000007f             /* rate/mcs value */
#define OLD_NRATE_STF_MASK          0x0000ff00             /* stf mode mask: siso, cdd, stbc, sdm */
#define OLD_NRATE_STF_SHIFT         8                      /* stf mode shift */
#define OLD_NRATE_OVERRIDE          0x80000000             /* bit indicates override both rate & mode */
#define OLD_NRATE_OVERRIDE_MCS_ONLY 0x40000000             /* bit indicate to override mcs only */
#define OLD_NRATE_SGI               0x00800000             /* sgi mode */
#define OLD_NRATE_LDPC_CODING       0x00400000             /* bit indicates adv coding in use */

#define OLD_NRATE_STF_SISO 0                               /* stf mode SISO */
#define OLD_NRATE_STF_CDD  1                               /* stf mode CDD */
#define OLD_NRATE_STF_STBC 2                               /* stf mode STBC */
#define OLD_NRATE_STF_SDM  3                               /* stf mode SDM */

/*
 * Format a ratespec for output of any of the wl_rate() iovars
 * Creates a string like this: vht mcs 9 Nss 3 Tx Exp 0 BW 20 sgi
 */
char *wl_rate_print(
    char    *rate_buf,
    uint32_t rspec
    )
{
    uint        encode, rate, txexp, bw_val;
    const char *stbc;
    const char *ldpc;
    const char *sgi;
    const char *bw;

    encode = ( rspec & WL_RSPEC_ENCODING_MASK );
    rate   = ( rspec & WL_RSPEC_RATE_MASK );
    txexp  = ( rspec & WL_RSPEC_TXEXP_MASK ) >> WL_RSPEC_TXEXP_SHIFT;
    bw_val = ( rspec & WL_RSPEC_BW_MASK );
    stbc   = (( rspec & WL_RSPEC_STBC ) != 0 ) ? " stbc" : "";
    ldpc   = (( rspec & WL_RSPEC_LDPC ) != 0 ) ? " ldpc" : "";
    sgi    = (( rspec & WL_RSPEC_SGI )  != 0 ) ? " sgi"  : "";

    if (bw_val == WL_RSPEC_BW_UNSPECIFIED)
    {
        bw = "auto";
    }
    else if (bw_val == WL_RSPEC_BW_20MHZ)
    {
        bw = "20";
    }
    else if (bw_val == WL_RSPEC_BW_40MHZ)
    {
        bw = "40";
    }
    else if (bw_val == WL_RSPEC_BW_80MHZ)
    {
        bw = "80";
    }
    else if (bw_val == WL_RSPEC_BW_160MHZ)
    {
        bw = "160";
    }
    else if (bw_val == WL_RSPEC_BW_10MHZ)
    {
        bw = "10";
    }
    else if (bw_val == WL_RSPEC_BW_5MHZ)
    {
        bw = "5";
    }
    else if (bw_val == WL_RSPEC_BW_2P5MHZ)
    {
        bw = "2.5";
    }
    else
    {
        bw = "???";
    }

    if (( rspec & ~WL_RSPEC_TXEXP_MASK ) == 0)             /* Ignore TxExpansion for NULL rspec check */
    {
        sprintf( rate_buf, "auto" );
    }
    else if (encode == WL_RSPEC_ENCODE_RATE)
    {
        sprintf( rate_buf, "rate %d%s Mbps Tx Exp %d",
            rate/2, ( rate % 2 ) ? ".5" : "", txexp );
    }
    else if (encode == WL_RSPEC_ENCODE_HT)
    {
        sprintf( rate_buf, "ht mcs %d Tx Exp %d BW %s%s%s%s",
            rate, txexp, bw, stbc, ldpc, sgi );
    }
    else if (encode == WL_RSPEC_ENCODE_VHT)
    {
        uint mcs = ( rspec & WL_RSPEC_VHT_MCS_MASK );
        uint Nss = ( rspec & WL_RSPEC_VHT_NSS_MASK ) >> WL_RSPEC_VHT_NSS_SHIFT;
        sprintf( rate_buf, "vht mcs %d Nss %d Tx Exp %d BW %s%s%s%s",
            mcs, Nss, txexp, bw, stbc, ldpc, sgi );
    }
    else
    {
        sprintf( rate_buf, "<unknown encoding for ratespec 0x%08X>", rspec );
    }

    if (0)
    {
        int          tRetCode = 0;
        ScanParams_t tScanParams;
        ScanInfo_t   tScanInfo;

        memset( &tScanParams, 0, sizeof( tScanParams ));
        memset( &tScanInfo, 0, sizeof( tScanInfo ));
        tRetCode = BWL_Scan( hBwl, &tScanParams );
        printf( "%s: BWL_Scan() rc %d \n", __FUNCTION__, tRetCode );

        if (tRetCode == 0)
        {
            int sec = 0;
            for (sec = 0; sec<10; sec++)
            {
                uint32_t ulNumOfAp = 0;

                sleep( 1 );

                tRetCode = BWL_GetScannedApNum( hBwl, &ulNumOfAp );
                printf( "%s: BWL_GetScannedApNum() rc %d; ulNumOfAp %d \n", __FUNCTION__, tRetCode, ulNumOfAp );

                if (ulNumOfAp > 0)
                {
                    ScanInfo_t *pData = NULL;

                    pData = (ScanInfo_t *)malloc( ulNumOfAp * sizeof( ScanInfo_t ));

                    if (pData == NULL)
                    {
                        printf( "%s Out of memory! \n", __FUNCTION__ );
                        tRetCode = -1;
                    }
                    else
                    {
                        memset( pData, 0x0, ( ulNumOfAp * sizeof( ScanInfo_t )));

                        tRetCode = BWL_GetScanResults( hBwl, pData );
                        printf( "%s: BWL_GetScanResults() rc %d \n", __FUNCTION__, tRetCode );

                        if (tRetCode == 0)
                        {
                            int  i = 0;
                            char mac_addr[32];

                            for (i = 0; i<ulNumOfAp; i++)
                            {
                                memset( mac_addr, 0, sizeof( mac_addr ));
                                snprintf( mac_addr, sizeof( mac_addr ), "%02X:%02X:%02X:%02X:%02X:%02X",
                                    pData[i].BSSID.octet[0], pData[i].BSSID.octet[1], pData[i].BSSID.octet[2],
                                    pData[i].BSSID.octet[3], pData[i].BSSID.octet[4], pData[i].BSSID.octet[5] );
                                printf( "%s: BWL_GetScanResults() %s ChanSpec %s; RSSI %d; Chan %u; PrimChan %u; PhyNoise %d; AuthType %u; 801Modes %u; blocked %d; WPS %d; Rate %d; Bandwidth %s \n",
                                    __FUNCTION__, mac_addr, pData[i].cChanSpec, pData[i].lRSSI, pData[i].ulChan, pData[i].ulPrimChan,
                                    pData[i].lPhyNoise, pData[i].ulAuthType, pData[i].ul802_11Modes, pData[i].bLocked, pData[i].bWPS, pData[i].lRate,
                                    Bsysperf_WifiBandwidthStr( pData[i].tBandwidth ));
                            }
                            printf( "%s:\n\n\n", __FUNCTION__ );
                        }

                        {
                            char *temp = (char*) pData;
                            Bsysperf_Free( temp );
                            pData = NULL;
                        }
                    }
                }
            }
        }
    }

    return( rate_buf );
}                                                          /* wl_rate_print */

#if 0
/**
 *  Function: This function will call the BWL function that will return the driver version string. The caller of
 *  this function is responsible for freeing the version string buffer.
 **/
char *Bsysperf_WifiGetVersion(
    const char *ifname
    )
{
    char *verBuffer = NULL;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            break;
        }
        else
        {
            if (( verBuffer = BWL_GetVersion( hBwl )) != NULL)
            {
                /* ioctl was successful */
            }
            else
            {
                printf( "%s() BWL_GetVersion() failed \n", __FUNCTION__ );
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( verBuffer );
}                                                          /* Bsysperf_WifiGetVersion */

#endif /* if 0 */

/**
 *  Function: This function will read the results that are output from the 'wl scanresults' command.
 **/
int Bsysperf_GetScanResultsWl(
    void
    )
{
    char  line[200];
    FILE *cmd           = NULL;
    bool  bHeaderOutput = false;

    sprintf( line, "wl scanresults" );
    cmd = popen( line, "r" );

    printf( "\n\n" );

    do
    {
        memset( line, 0, sizeof( line ));
        fgets( line, sizeof( line ), cmd );

        if (strlen( line ) > 0)
        {
            int len = strlen( line );
            if (line[len-1] == '\n') {line[len-1] = 0; }
        }

        if (strlen( line ) == 0)
        {
            if (bHeaderOutput == true)
            {
                bHeaderOutput = false;
                printf( "</textarea></td></tr>\n" );
            }
        }
        else
        {
            if (bHeaderOutput == false)
            {
                bHeaderOutput = true;
                printf( "<tr><td><textarea cols=120 rows=20 >" );
            }
            printf( "%s\n", line );
        }
    } while (!feof( cmd ));

    pclose( cmd );

    return( 0 );
}                                                          /* Bsysperf_GetScanResultsWl */

static int Bsysperf_WifiScanResults80211Modes(
    e802_11Modes_t modes
    )
{
    if (modes & e802_11_a) {printf( "A " ); }
    if (modes & e802_11_b) {printf( "B " ); }
    if (modes & e802_11_g) {printf( "G " ); }
    if (modes & e802_11_n) {printf( "N " ); }
    if (modes & e802_11_ac) {printf( "AC " ); }
    return( 0 );
}

int Bsysperf_WifiScanResultsFormat(
    ScanInfo_t *pScanInfo
    )
{
    char mac_addr[32];

    if (pScanInfo == NULL)
    {
        return( -1 );
    }

    memset( mac_addr, 0, sizeof( mac_addr ));
    snprintf( mac_addr, sizeof( mac_addr ), "%02X:%02X:%02X:%02X:%02X:%02X",
        pScanInfo->BSSID.octet[0], pScanInfo->BSSID.octet[1], pScanInfo->BSSID.octet[2],
        pScanInfo->BSSID.octet[3], pScanInfo->BSSID.octet[4], pScanInfo->BSSID.octet[5] );

    printf( "~WIFISCANRESULTS~<textarea cols=120 rows=15 >" );
    printf( "SSID: \"%s\"\n", pScanInfo->tCredentials.acSSID );
    printf( "Mode:\tRSSI: %d dBm\tSNR: %d dB\tnoise: %d dBm\tFlags: \n", pScanInfo->lRSSI, 0, pScanInfo->lPhyNoise );
    printf( "Channel: %s \n", pScanInfo->cChanSpec );
    printf( "BSSID: %s \tCapability: %s\n", mac_addr, "UNKNOWN" );
    printf( "ulChan: %d \n", pScanInfo->ulChan );
    printf( "ulPrimChan: %d \n", pScanInfo->ulPrimChan );
    printf( "ulAuthType: %d \n", pScanInfo->ulAuthType );
    printf( "802.11 Modes: " );
    Bsysperf_WifiScanResults80211Modes( pScanInfo->ul802_11Modes );
    printf( "\n" );
    printf( "bLocked: %d \n", pScanInfo->bLocked );
    printf( "bWPS: %d \n", pScanInfo->bWPS );
    printf( "lRate: %d (%x) \n", pScanInfo->lRate, pScanInfo->lRate );
    printf( "Bandwidth: %s \n", Bsysperf_WifiBandwidthStr( pScanInfo->tBandwidth ));
    printf( "</textarea><br>~" );

    return( 0 );
}                                                          /* Bsysperf_WifiScanResultsFormat */

static int Bsysperf_WifiAmpdu_WriteToFile(
    const char *buffer,
    const char *filename
    )
{
    FILE *fp = NULL;

    fp = fopen( filename, "a" );

    if (fp == NULL)
    {
        printf( "%s:%u: could not open file (%s) for writing \n", __FUNCTION__, __LINE__, filename );
        return( -1 );
    }

    fwrite( buffer, 1, strlen( buffer ), fp );
    fclose( fp );

    return( 0 );
}                                                          /* Bsysperf_WifiAmpdu_WriteToFile */

/**
 *  Function: This function will search the provided buffer for the line that contains the three items:
 *  tot_mpdus, tot_ampdus, and mpduperampdu. Once the line is found, the values associated with these
 *  three tags will be scanned and written into the provided filename. The values in the provided file will
 *  eventually be read by the bsysperf.cgi when the user has checked the AMPDU Graph checkbox.
 **/
static char *Bsysperf_WifiAmpdu_ParseTot(
    const char *buffer,
    const char *filename
    )
{
    char *pos = (char*) buffer;
    char *eol = NULL;
    char  output_line[256];

    noprintf("%s:%u: buffer (%p); filename (%s) \n", __FUNCTION__, __LINE__, buffer, filename );
    if (( buffer== NULL ) || ( filename == NULL ))
    {
        return( pos );
    }
    noprintf("%s:%u: buffer len (%u); filename (%s) \n", __FUNCTION__, __LINE__, strlen(buffer), filename );

    /* look for a line like this ... tot_mpdus 10000 tot_ampdus 20000 mpduperampdu 30000 */
    pos = strstr(buffer, "tot_mpdus ");
    if ( pos ) {
        unsigned int tot_mpdus = 0;
        unsigned int tot_ampdus = 0;
        unsigned int mpduperampdu = 0;
        eol = strstr(pos, "\n");
        if ( eol ) {
            *eol = '\0';
        }
        /*printf("%s:%u: found line (%s)(%p) ... len %u \n", __FUNCTION__, __LINE__, pos, (void*) pos, strlen(pos) );*/
        sscanf( pos, "tot_mpdus %u tot_ampdus %u mpduperampdu %u", &tot_mpdus, &tot_ampdus, &mpduperampdu );

        memset( output_line, 0, sizeof(output_line) );
        snprintf(output_line, sizeof(output_line), "tot_mpdus\t%u\ttot_ampdus\t%u\tmpduperampdu\t%u\n", tot_mpdus, tot_ampdus, mpduperampdu );
        Bsysperf_WifiAmpdu_WriteToFile( output_line, filename );

        pos += strlen(pos);

        /* if we null-terminated the line above, restore the new-line character */
        if ( eol ) {
            *eol = '\n';
        }
    }
    else
    {
        /*printf("%s:%u: tot_mpdus tag not found in buffer \n", __FUNCTION__, __LINE__ );*/
        pos = (char*) buffer;
    }

    return ( pos );
}

/*
 * Parse a line that looks similar to this and extract the 12 values and 12 percentages. There could be 2, or 3, or 4
 * subsequent lines after the first one ... depending on the wifi chip and the wifi router.
 *
 *                        1    1    2    2    3    3    4    4    5    5    6    6    7    7    8    8
 *              0    5    0    5    0    5    0    5    0    5    0    5    0    5    0    5    0    5
 *              TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
 *                      :  3(0%)  0(0%)  0(0%)  0(0%)  2746(8%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
 *                      :  1408(4%)  1631(5%)  21440(70%)  3284(10%)  11(0%)  10(0%)  9(0%)  11(0%)  6(0%)  0(0%)  0(0%)  0(0%)
 *
 */
static char *Bsysperf_WifiAmpdu_ParseVht(
    const char *buffer,
    const char *keyword,
    char       *values,
    char       *percentages,
    const char *filename
    )
{
    int          idx            = 0;
    unsigned int value          = 0;
    unsigned int percentage     = 0;
    int          antenna_count  = 0;
    char        *pos            = NULL;
    char        *line           = NULL;
    char        *beg_of_section = NULL;                    /* beginning of section */
    char         keywordStr[16];
    char         tempStr32[32];
    char         tempLine[512];
    int          debug = 0;

    if (( buffer== NULL ) || ( keyword == NULL ))
    {
        return( 0 );
    }

    beg_of_section = strstr( buffer, keyword );
    if ( buffer ) {
        if(debug) printf( "%s:%u: BEFORE PROCESSING (%s) ... original buffer is len (%d) beg_of_section %p \n---------------------------------\n",
                __FUNCTION__, __LINE__, keyword, (int) strlen(buffer), beg_of_section );
    } else {
        if(debug) printf( "%s:%u: BEFORE PROCESSING (%s) ... original buffer is len (0) \n---------------------------------\n",
                __FUNCTION__, __LINE__, keyword );
    }
    if (beg_of_section)
    {
        bool bValidLineFound = false;

        memset( keywordStr, 0, sizeof( keywordStr ));
        memset( tempLine, 0, sizeof( tempLine ));

        strncpy( tempLine, beg_of_section, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( beg_of_section, '\n' ) - beg_of_section ) );
        strncpy( keywordStr, beg_of_section, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( beg_of_section, ':' ) - beg_of_section ) );

        line = beg_of_section;

        do
        {
            bValidLineFound = false;

            /* TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)                       */
            pos = strchr( line, ':' );
            if(debug) printf( "%s:%u: tempLine (%s) pos (%p) \n", __FUNCTION__, __LINE__, tempLine, pos );
            if (pos)
            {
                bool  bReplacedNewline = false;
                char *eol              = NULL;
                int   value_count      = 0;

                antenna_count++;                           /* used to decide 1x1, 2x2, 3x3, or 4x4 */

                bValidLineFound = true;

                if (( eol = strchr( pos, '\n' )))
                {
                    bReplacedNewline = true;
                    *eol             = 0;
                }

                memset( tempStr32, 0, sizeof( tempStr32 ));

                sprintf( tempStr32, "%s %dx%d\t", keywordStr, antenna_count, antenna_count );
                strcat( values, tempStr32 );

                sprintf( tempStr32, "%s %dx%d %%\t", keywordStr, antenna_count, antenna_count );
                strcat( percentages, tempStr32 );

                if(debug) printf( "%s:%u: found line (%s) ... ant %d \n", __FUNCTION__, __LINE__, tempLine, antenna_count );
                pos += 3;                                  /*skip over the colon and the 1st two spaces */
                for (idx = 0; pos; idx++)
                {
                    memset( tempStr32, 0, sizeof( tempStr32 ));
                    value      = 0;
                    percentage = 0;
                    strncpy( tempStr32, pos, 11 );
                    sscanf( pos, "%d(%d)", &value, &percentage );
                    /*if (value || percentage )*/
                    {
                        char lvalue[16], lpercentage[8];
                        sprintf( lvalue, "%d", value );
                        sprintf( lpercentage, "%d", percentage );

                        value_count++;

                        if (value_count > 1)
                        {
                            strcat( values, "\t" );
                            strcat( percentages, "\t" );
                        }
                        strcat( values, lvalue );
                        strcat( percentages, lpercentage );

                        noprintf( "%s[%d]: offset (%ld) from (%s) value %d; percentage %d \n", keywordStr, idx,
                           (long int) ((unsigned int*)pos-(unsigned int*)line), tempStr32, value, percentage );
                    }

                    pos = strstr( pos, "  " );
                    if (pos)
                    {
                        pos += 2;                          /*skip over the two spaces */
                    }
                }

                if (bReplacedNewline && eol)
                {
                    *eol = '\n';
                }

                line = strchr( line, '\n' );
                if (line)
                {
                    line++;

                    memset( tempLine, 0, sizeof( tempLine ));
                    strncpy( tempLine, line, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( line, '\n' ) - line ) );
                    if(debug) printf( "%s:%u: next line (%s) ... ant %d \n", __FUNCTION__, __LINE__, tempLine, antenna_count );
                }

                strcat( values, "\t\t\t" );                /* separate this line with a couple of tabs */
                strcat( percentages, "\n" );               /* put the percentages on the same line as the collected values just over to the right a few columns */

                Bsysperf_WifiAmpdu_WriteToFile( values, filename );
                Bsysperf_WifiAmpdu_WriteToFile( percentages, filename );
                if(debug) printf( "%s:%u: values (%s) \n", __FUNCTION__, __LINE__, values );
                if(debug) printf( "%s:%u: percen (%s) \n", __FUNCTION__, __LINE__, percentages );

                values[0]      = '\0';
                percentages[0] = '\0';
            }
            else
            {
                if(debug) printf( "%s:%u: could not find colon after beg_of_section (%s) \n", __FUNCTION__, __LINE__, beg_of_section );
            }
            if(debug) printf( "%s:%u: endwhile: pos %p;  line %p ... line[0] (%c) \n", __FUNCTION__, __LINE__, pos, line, line[0] );

            /* each TX/RX section may have 2, 3, or 4 more lines that begin with a space.
               If we encounter a blank line or a line that does NOT start with a space, we have found the end of the TX/RX section */
        } while ( bValidLineFound && line && line[0] == ' ');

        if (line)
        {
            memset( tempLine, 0, sizeof( tempLine ));
            strncpy( tempLine, line, MIN( sizeof(tempLine) - strlen(tempLine) - 1, strchr( line, '\n' ) - line ) );
        }
        if(debug) printf( "%s:%u: found end of section at line (%s) \n", __FUNCTION__, __LINE__, tempLine );
    }
    else
    {
        if(debug) printf( "%s:%u: could not find keyword (%s) \n", __FUNCTION__, __LINE__, keyword );
    }

    if(debug) printf( "%s:%u: end ... line (%p) \n", __FUNCTION__, __LINE__, (void*) line );
    return( line );
}                                                          /* Bsysperf_WifiAmpdu_ParseVht */

int Bsysperf_WifiAmpduClear(
    const char *ifname
    )
{
    int   tRetCode = 0;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            BWL_API_CHECK( BWL_ClearAmpdu( hBwl ), tRetCode );
        }
        Bsysperf_WifiUninit();
    } while (0);

err_out:
    return( tRetCode );
}                                                          /* Bsysperf_WifiAmpduClear */

/**
 *  Function: This function will call a BWL function to dump the AMPDU data similiar to the 'wl dump ampdu'
 *  command. The AMPDU data gets stored in the 'ampdu' buffer and then processed.
 **/
int Bsysperf_WifiAmpdu_GetReport(
    const char *ifname
    )
{
    int   tRetCode = 0;
    char *ampdu    = NULL;

    noprintf( "~DEBUG~%s:%u: hBwl #%p ~", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (( ampdu = (char *) BWL_GetAmpdu( hBwl )) == NULL)
            {
                printf( "~FATAL~BWL_GetAmpdu() failed with error %d~", tRetCode );
            }
            else
            {
                char *pos = ampdu;
                char  values[256];
                char  percentages[256];
                char  tempFilename[TEMP_FILE_FULL_PATH_LEN];

                PrependTempDirectory( tempFilename, sizeof( tempFilename ), FILENAME_WIFI_RECORD );

                remove( tempFilename );

                memset( values, 0, sizeof( values ));
                sprintf( values, "%lu\t%s\n", getSecondsSinceEpoch(), DateYyyyMmDdHhMmSs());
                Bsysperf_WifiAmpdu_WriteToFile( values, tempFilename );

                /*
                TX VHT  :  33(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  3(0%)  0(0%)  0(0%)  0(0%)  2746(8%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  1408(4%)  1631(5%)  21440(70%)  3284(10%)  11(0%)  10(0%)  9(0%)  11(0%)  6(0%)  0(0%)  0(0%)  0(0%)
                TX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  8(0%)  0(0%)  14211(99%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                RX VHT  :  3(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                        :  0(0%)  0(0%)  0(0%)  33(0%)  36(0%)  1672(5%)  1930(5%)  127(0%)  15656(47%)  0(0%)  0(0%)  0(0%)
                        :  0(0%)  0(0%)  0(0%)  162(0%)  187(0%)  950(2%)  2132(6%)  2294(6%)  7351(22%)  427(1%)  0(0%)  0(0%)
                RX VHT SGI:  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  15073(99%)  0(0%)  0(0%)  0(0%)
                          :  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  0(0%)  21(0%)  0(0%)  0(0%)
                */
                noprintf("\n\n%s:%u: ampdu buffer len (%d) \n", __FUNCTION__, __LINE__, strlen(ampdu) );
                noprintf( "%s", ampdu );

                Bsysperf_WifiAmpdu_ParseTot( pos, tempFilename );
                noprintf("%s:%u: after ParseTot, buffer is (%s) (%p) \n", __FUNCTION__, __LINE__, pos, (void*) pos );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = Bsysperf_WifiAmpdu_ParseVht( pos, "TX VHT  :", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = Bsysperf_WifiAmpdu_ParseVht( pos, "TX VHT SGI:", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = Bsysperf_WifiAmpdu_ParseVht( pos, "RX VHT  :", values, percentages, tempFilename );

                memset( values, 0, sizeof( values ));
                memset( percentages, 0, sizeof( percentages ));
                pos = Bsysperf_WifiAmpdu_ParseVht( pos, "RX VHT SGI:", values, percentages, tempFilename );

                {
                    char *temp = (char*) ampdu;
                    Bsysperf_Free( temp );
                    ampdu = NULL;
                }
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}                                                          /* Bsysperf_WifiAmpdu_GetReport */

int Bsysperf_WifiReadAmpduData(
    AMPDU_DATA_T *pAmpduData
    )
{
    char  tempFilename[TEMP_FILE_FULL_PATH_LEN];
    char *contents = NULL;

    PrependTempDirectory( tempFilename, sizeof( tempFilename ), FILENAME_WIFI_RECORD /* CAD "wifi_record2.txt" */ );

    contents = GetFileContents( tempFilename );

    /*printf( "~DEBUG~%s:%u: filename (%s); len (%d) ~\n", __FUNCTION__, __LINE__, tempFilename, (int) strlen(contents) );*/

    /*
1465563108	20160610-125148
TX VHT   1x1	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0			TX VHT   1x1 %	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0
TX VHT SGI 1x1	0	0	0			TX VHT SGI 1x1 %	0	0	0
RX VHT   1x1	0			RX VHT   1x1 %	0
RX VHT SGI 1x1	0			RX VHT SGI 1x1 %	0
    */
    /* if the contents of the file were successfully read */
    if (contents)
    {
        bool        bGoodLine = false;
        char       *pos       = contents;
        char       *eol       = NULL;
        char       *sgi       = NULL;
        char       *tab2      = NULL;
        char       *tot_mpdus = NULL;
        int         lines     = 0;
        ANTENNA1_T *ant       = NULL;
        do
        {
            char             *token = NULL;
            int               count = 0;
            unsigned long int level = 0;

            bGoodLine = false;

            eol = strchr( pos, '\n' );
            if (eol)
            {
                *eol = '\0';                               /* null-terminate the line */
            }

            /* look for a line like this ... tot_mpdus 10000 tot_ampdus 20000 mpduperampdu 30000 */
            tot_mpdus = strstr(pos, "tot_mpdus");
            if ( tot_mpdus )
            {
                /*printf("~DEBUG~%s:%u: found line (%s) ~", __FUNCTION__, __LINE__, tot_mpdus );*/
                sscanf( tot_mpdus, "tot_mpdus %u tot_ampdus %u mpduperampdu %u", &pAmpduData->tot_mpdus, &pAmpduData->tot_ampdus, &pAmpduData->mpduperampdu );

                /*printf("tot_mpdus %u tot_ampdus %u mpduperampdu %u \n", pAmpduData->tot_mpdus, pAmpduData->tot_ampdus, pAmpduData->mpduperampdu );*/
            }

            tab2 = strstr( pos, "\t\t" );
            if (tab2)
            {
                *tab2 = '\0';                              /* ignore the part of the line after the triple tab delimiter */
            }

            noprintf( "~DEBUG~found line (%s); ~\n", pos );

            sgi = strstr( pos, "SGI" );

            if (( pos[0] == 'T' ) && ( pos[1] == 'X' ))
            {
                ant       = &pAmpduData->antennas.tx[0];
                bGoodLine = true;
                noprintf( "~DEBUG~TX detected ant is (%p)~\n", (void *) ant );
            }
            else if (( pos[0] == 'R' ) && ( pos[1] == 'X' ))
            {
                ant       = &pAmpduData->antennas.rx[0];
                bGoodLine = true;
                noprintf( "~DEBUG~RX detected ant is (%p)~\n", (void *) ant );
            }
            else
            {
                /*printf( "~DEBUG~unknown line (%s)~\n", pos );*/
            }

            if (bGoodLine)
            {
                if (sgi)
                {
                    /* skip over the first four non-SGI antennas */
                    ant++;
                    ant++;
                    ant++;
                    ant++;
                    noprintf( "~DEBUG~SGI detected ant is (%p)~\n", (void *) ant );
                }
                else
                {
                    noprintf( "~DEBUG~SGI NOT detected ant is (%p)~\n", (void *) ant );
                }

                if (strstr( pos, "1x1" ))
                {
                    bGoodLine = true;
                }
                else if (strstr( pos, "2x2" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    bGoodLine = true;
                }
                else if (strstr( pos, "3x3" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    ant++;                                 /* skip over the 2x2 antenna */
                    bGoodLine = true;
                }
                else if (strstr( pos, "4x4" ))
                {
                    ant++;                                 /* skip over the 1x1 antenna */
                    ant++;                                 /* skip over the 2x2 antenna */
                    ant++;                                 /* skip over the 3x3 antenna */
                    bGoodLine = true;
                }

                if (bGoodLine)
                {
                    lines++;
                    noprintf( "~DEBUG~ant %p; antennas %p; sizeof 1 ant %u~", (void *) ant, (void *) &pAmpduData->antennas, sizeof( *ant ));
                    noprintf( "\n\n" );

                    token = strtok( pos, "\t" );
                    count = 0;
                    while (token)
                    {
                        /*printf("token is (%s) \n", token ); */
                        if (count == 0)
                        {
                            /*printf( "name token is (%s) ... copied to (%p) \n", token, &ant->name[0] );*/
                            strncpy( ant->name, token, sizeof( ant->name ) -1 );
                        }
                        else if (count <= ANTENNA_MAX_LEVELS)
                        {
                            sscanf( token, "%ld", &level );
                            ant->level[count-1] = level;
                            /*printf( "level token is (%s) ... ant->level[%d]= %d (%p) \n", token, count-1, ant->level[count-1], (void *) &ant->level[count-1] );*/
                        }
                        token = strtok( NULL, "\t" );
                        count++;
                    }
                }
            }

            if (eol)
            {
                eol++;                                     /* skip over the end of line character ... should be pointing to beginning of next line ... or end of file */
            }

            pos = eol;
        } while (pos);

        {
            char *temp = (char*) contents;
            Bsysperf_Free( temp );
            contents = NULL;
        }
    }

    /*Bsysperf_WifiPrintAntennas( pAmpduData->antennas );*/

    return( 0 );
} /* Bsysperf_WifiReadAmpduData */

int Bsysperf_WifiOutputAntennasHtml(
    ANTENNAS_T *antennas
    )
{
    int          i, j;
    int          xcount = 0;
    bool         bValidDataFound = false;
    unsigned int colors[16] = {0x33fffc, 0x33daff, 0x33a2ff, 0x337aff, 0xbeff33, 0x80ff33, 0x33ff6e, 0x33ffb5, /* blues and greens */
                               0xff4633, 0xff7733, 0xff9633, 0xffc133, 0xd733ff, 0xf633ff, 0xff33ca, 0xff3399}; /* reds and purples */
    unsigned int colors_idx = 0;
    char        *visibilityStr[2]  = { "hidden", "display" };
    int          visibilitySetting = 0;

    printf( "<table border=0 cellpadding=0 cellspacing=0 style=\"border-collapse:collapse;\" />" );
    printf( "<tr><th>&nbsp;</th>" );
    for (j = 0; j<ANTENNA_MAX_LEVELS; j++)
    {
        printf( "<th>%d</th>", j );
    }
    printf( "<th>MCS</th><th>&nbsp;</th></tr>" );
    for (i = 0; i<sizeof( ANTENNAS_T )/sizeof( ANTENNA1_T )/2; i++) /* output TX values */
    {
        if ( antennas->tx[i].name[0] )
        {
            visibilitySetting = 1;
            bValidDataFound = true;
        }
        else
        {
            visibilitySetting = 0;
        }
        printf( "<tr style=\"visibility:%s;\" id='r_x%d' ><th id='s_x%d' name='s_x%d' align=right >%s</th>", visibilityStr[visibilitySetting], xcount, xcount, xcount, antennas->tx[i].name );
        for (j = 0; j<ANTENNA_MAX_LEVELS; j++)
        {
            printf( "<td class=black_allborders ><input class=svg name='v%d_%d' id='v%d_%d' value='%d'></td>", xcount, j, xcount, j, MAX( antennas->tx[i].level[j], 1 ));
        }
        printf( "<td><input type=hidden class=svg name='c_x%d' id='c_x%d' value='#%x'></td></tr>", xcount, xcount, colors[colors_idx] );
        xcount++;
        colors_idx++;
    }

    colors_idx = 8; /* start the colors halfway through where the RX colors start */
    for (i = 0; i<sizeof( ANTENNAS_T )/sizeof( ANTENNA1_T )/2; i++) /* output RX values */
    {
        if ( antennas->rx[i].name[0] )
        {
            visibilitySetting = 1;
            bValidDataFound = true;
        }
        else
        {
            visibilitySetting = 0;
        }
        bValidDataFound = true;
        printf( "<tr style=\"visibility:%s;\" id='R_x%d' ><th id='S_x%d' name='S_x%d' align=right >%s</th>", visibilityStr[visibilitySetting], i, i /*xcount*/, i /*xcount*/, antennas->rx[i].name );
        for (j = 0; j<ANTENNA_MAX_LEVELS; j++)
        {
            printf( "<td class=black_allborders ><input class=svg name='V%d_%d' id='V%d_%d' value='%d'></td>", i /*xcount*/, j, i /*xcount*/, j, MAX( antennas->rx[i].level[j], 1 ));
        }
        printf( "<td><input type=hidden class=svg name='C_x%d' id='C_x%d' value='#%x'></td></tr>", i /*xcount*/, i /*xcount*/, colors[colors_idx] );
        xcount++;
        colors_idx++;
    }

    /* if the server did not find any AMPDU data, output some dummy data (possibly need to run 'wl join <BSSID>') */
    if ( bValidDataFound == false )
    {
        int max_idx = sizeof( ANTENNAS_T )/sizeof( ANTENNA1_T )/4; /* only do 4 test rows */
        colors_idx = 0; /* start the colors where the TX colors start */
        for (i = 0; i<max_idx; i++)
        {
            bValidDataFound = true;
            printf( "<tr><th id='s_x%d' align=right >Test_%d</th>", xcount, xcount+1 );
            for (j = 0; j<ANTENNA_MAX_LEVELS; j++)
            {
                printf( "<td class=black_allborders ><input class=svg name='v%d_%d' value='%d'></td>", xcount, j, MAX ( 100*(max_idx - i - 1) + ( j * 10 ), 1) );
            }
            printf( "<th><input type=hidden class=svg name='c_x%d' value='#%x'></th></tr>", xcount, colors[colors_idx] );
            xcount++;
            colors_idx++;
        }

    }

    #if 0
    printf( "<tr><th>&nbsp;</th>" );
    for (j = 0; j<ANTENNA_MAX_LEVELS; j++)
    {
        printf( "<th align=center >%d</th>", j );
    }
    printf( "<th><input type='button' onClick='ClearGrid()' style='width:44' value='Clear' /></th><th><input type='button' onClick='DrawScene()' style='width:44' value='Draw' /></th>" );
    printf( "</tr>" );
    #endif
    printf( "</table>" );

    return( 0 );
} /* Bsysperf_WifiOutputAntennasHtml */

int Bsysperf_WifiPrintAntennas(
    ANTENNAS_T *antennas
    )
{
    int ant;
    int level;

    for (ant = 0; ant<8; ant++)
    {
        if (antennas->tx[ant].name[0] == 0)
        {
            printf( "%p ... TX %dx%d not found\n", (void *) &antennas->tx[ant], ( ant%4 )+1, ( ant%4 )+1 );
        }
        else
        {
            printf( "%p ... %d:%s: ", (void *) &antennas->tx[ant], ant, antennas->tx[ant].name );
            for (level = 0; level<ANTENNA_MAX_LEVELS; level++)
            {
                printf( "L%d:%d ", level, antennas->tx[ant].level[level] );
            }
            printf( "\n" );
        }
    }
    for (ant = 0; ant<8; ant++)
    {
        if (antennas->tx[ant].name[0] == 0)
        {
            printf( "%p ... RX %dx%d not found\n", (void *) &antennas->rx[ant], ( ant%4 )+1, ( ant%4 )+1 );
        }
        else
        {
            printf( "%p ... %d:%s: ", (void *) &antennas->rx[ant], ant, antennas->rx[ant].name );
            for (level = 0; level<ANTENNA_MAX_LEVELS; level++)
            {
                printf( "L%d:%d ", level, antennas->rx[ant].level[level] );
            }
            printf( "\n" );
        }
    }
    return( 0 );
} /* Bsysperf_WifiPrintAntennas */

int Bsysperf_WifiGetRssiAnt(
    const char              *ifname,
    BSYSPERF_PHY_RSSI_ANT_T *pRssiAnt
    )
{
    int                      tRetCode    = 0;
    BSYSPERF_PHY_RSSI_ANT_T *rssi_ant_p  = NULL;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }
        else
        {
            if (( rssi_ant_p = (BSYSPERF_PHY_RSSI_ANT_T*) BWL_GetPhyRssiAnt( hBwl )) == NULL)
            {
                printf( "~FATAL~BWL_GetPhyRssiAnt() failed with error %d~", tRetCode );
            }
            else
            {
                #if 0
                int idx;

                printf( "%s:%u: rssi_ant->count #%d \n", __FILE__, __LINE__, rssi_ant_p->count );
                for (idx=0; idx<rssi_ant_p->count; idx++)
                {
                    int temp = rssi_ant_p->rssi_ant[idx];
                    printf( "rssi[%d] %d (0x%x)  ", idx, temp, temp );
                }
                printf("\n");
                #endif

                memcpy(pRssiAnt, rssi_ant_p, sizeof(*pRssiAnt) );

                {
                    char *temp = (char*) rssi_ant_p;
                    Bsysperf_Free( temp );
                    rssi_ant_p = NULL;
                }
            }
        }
        Bsysperf_WifiUninit();
    } while (0);

    return( tRetCode );
}

/**
 *  Function: This function will call a BWL function to get the current driver version string.
 *            The string will be something similar to:
 *                wl0: Nov 16 2016 10:11:39 version 15.10.15 (r670222)
 *            This function will look for the "version" string and then extract the numbers
 *            that are found after the "version" string.
 **/
int Bsysperf_WifiGetDriverVersion(
    const char *ifname,
    char *strDriverVersion,
    int   iDriverVersionLen
    )
{
    int   tRetCode = 0;
    char *dump_buf = NULL;
    char *pVersionS = NULL;
    char *pVersionE = NULL;

    noprintf( "%s:%u: hBwl #%p \n", __FILE__, __LINE__, hBwl );
    strncpy( strDriverVersion, "unknown", iDriverVersionLen - 1 );
    do
    {
        Bsysperf_WifiInit( ifname );

        if (hBwl == NULL)
        {
            printf( "%s() Failed to Init BWL! \n", __FUNCTION__ );
            tRetCode = -1;
            break;
        }

        if (( dump_buf = BWL_GetDriverVersion ( hBwl )) == NULL)
        {
            printf( "~FATAL~BWL_GetDriverVersion() failed with error %d~", tRetCode );
            tRetCode = -1;
            break;
        }

        Bsysperf_WifiUninit();

        pVersionS = strstr( dump_buf, "version " );
        if ( pVersionS )
        {
            pVersionS += strlen( "version " );

            pVersionE = strstr( pVersionS, " " );
            if ( pVersionE )
            {
                *pVersionE = '\0'; /* null-terminate the version string part */
                strncpy( strDriverVersion, pVersionS, iDriverVersionLen - 1 );
            }
        }
    } while (0);

    return( tRetCode );
}
