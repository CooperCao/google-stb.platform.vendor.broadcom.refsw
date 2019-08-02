/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 116460 $
 ***********************************************************************/

/*
 * IEEE1905 Data-Model
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#ifdef TARGETENV_android
#include <linux/time.h>
#else
#include <sys/time.h>
#endif /* TARGETENV_android */
#include "ieee1905_json.h"
#include "ieee1905_message.h"
#include "ieee1905_trace.h"
#include "ieee1905_tlv.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_plc.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_interface.h"
#include "ieee1905_ethstat.h"
#include "ieee1905_wlmetric.h"
#include "ieee1905_utils.h"
#include "ieee1905_glue.h"
#include "ieee1905_timer.h"
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
#include "ieee1905_cmsmdm.h"
#endif
#include "ieee1905.h"

#define I5_TRACE_MODULE i5TraceDm

#define I5_DM_MAX_LINK_METRICS_LIST_SIZE 16

#define I5_DM_NEIGHBOR_BRIDGE_DISCOVERY_TIMEOUT (2*I5_MESSAGE_TOPOLOGY_DISCOVERY_PERIOD_MSEC)

#ifdef MULTIAP
#define I5_DM_LINK_METRICS_DEFAULT_AUTO_FETCH_INTERVAL_MSEC 0 /* disabled */
#else
/* One timer for fetching link metrics from all devices, NOT "one timer per device" */
#if defined(SUPPORT_IEEE1905_GOLDENNODE)
#define I5_DM_LINK_METRICS_DEFAULT_AUTO_FETCH_INTERVAL_MSEC 0
#else
#define I5_DM_LINK_METRICS_DEFAULT_AUTO_FETCH_INTERVAL_MSEC 10000
#endif
#endif /* MULTIAP */
#define I5_DM_LINK_METRICS_MINIMUM_AUTO_FETCH_INTERVAL_MSEC 5000
#define I5_DM_LINK_METRICS_MAXIMUM_AUTO_FETCH_INTERVAL_MSEC 600000

#define I5_DM_DEVICE_TIMER_ROBUSTNESS            2
#define I5_DM_DEVICE_TIMER_LATENCY_MSEC          120000
#define I5_DM_TOP_DISC_DEVICE_GONE_TIMER         I5_DM_DEVICE_TIMER_ROBUSTNESS*I5_MESSAGE_STANDARD_TOPOLOGY_DISCOVERY_PERIOD_MSEC + I5_DM_DEVICE_TIMER_LATENCY_MSEC

#ifdef MULTIAP
/* Channel to regulatory class mapping */
static i5_dm_chan_rc_map_type chan_rc_map[] =
{
  /***********************************************************
  chan cnt 80Mhz 160Mhz        regulatory classes
  ************************************************************/
  {1,   2,   0,   0,  {81,  83,   0,   0,    0}},
  {2,   2,   0,   0,  {81,  83,   0,   0,    0}},
  {3,   2,   0,   0,  {81,  83,   0,   0,    0}},
  {4,   2,   0,   0,  {81,  83,   0,   0,    0}},
  {5,   3,   0,   0,  {81,  83,  84,   0,    0}},
  {6,   3,   0,   0,  {81,  83,  84,   0,    0}},
  {7,   3,   0,   0,  {81,  83,  84,   0,    0}},
  {8,   3,   0,   0,  {81,  83,  84,   0,    0}},
  {9,   3,   0,   0,  {81,  83,  84,   0,    0}},
  {10,  2,   0,   0,  {81,  84,   0,   0,    0}},
  {11,  2,   0,   0,  {81,  84,   0,   0,    0}},
  {12,  2,   0,   0,  {81,  84,   0,   0,    0}},
  {13,  2,   0,   0,  {81,  84,   0,   0,    0}},
  {14,  1,   0,   0,  {82,   0,   0,   0,    0}},
  {36,  5,  42,  50,  {115, 116, 128, 129, 130}},
  {40,  5,  42,  50,  {115, 117, 128, 129, 130}},
  {44,  5,  42,  50,  {115, 116, 128, 129, 130}},
  {48,  5,  42,  50,  {115, 117, 128, 129, 130}},
  {52,  5,  58,  50,  {118, 119, 128, 129, 130}},
  {56,  5,  58,  50,  {118, 120, 128, 129, 130}},
  {60,  5,  58,  50,  {118, 119, 128, 129, 130}},
  {64,  5,  58,  50,  {118, 120, 128, 129, 130}},
  {100, 5, 106, 114,  {121, 122, 128, 129, 130}},
  {104, 5, 106, 114,  {121, 123, 128, 129, 130}},
  {108, 5, 106, 114,  {121, 122, 128, 129, 130}},
  {112, 5, 106, 114,  {121, 123, 128, 129, 130}},
  {116, 5, 122, 114,  {121, 122, 128, 129, 130}},
  {120, 5, 122, 114,  {121, 123, 128, 129, 130}},
  {124, 5, 122, 114,  {121, 122, 128, 129, 130}},
  {128, 5, 122, 114,  {121, 123, 128, 129, 130}},
  {132, 4, 138,   0,  {121, 122, 128, 130,   0}},
  {136, 4, 138,   0,  {121, 123, 128, 130,   0}},
  {140, 4, 138,   0,  {121, 122, 128, 130,   0}},
  {144, 4, 138,   0,  {121, 123, 128, 130,   0}},
  {149, 5, 155,   0,  {124, 125, 126, 128, 130}},
  {153, 5, 155,   0,  {124, 125, 127, 128, 130}},
  {157, 5, 155,   0,  {124, 125, 126, 128, 130}},
  {161, 5, 155,   0,  {124, 125, 127, 128, 130}},
  {165, 1,   0,   0,  {125,   0,   0,   0,   0}},
  {169, 1,   0,   0,  {125,   0,   0,   0,   0}},
};

/* Regulatory class to channel map */
static i5_dm_rc_chan_map_type rc_chan_map[] =
{
  /*************************************************************************
  rclass cnt				channels
  **************************************************************************/
  { 81, 13, {  1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13}},
  { 82,  1, { 14,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  { 83,  9, {  1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   0,   0,   0}},
  { 84,  9, {  5,   6,   7,   8,   9,  10,  11,  12,  13,   0,   0,   0,   0}},
  {115,  4, { 36,  40,  44,  48,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {116,  2, { 36,  44,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {117,  2, { 40,  48,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {118,  4, { 52,  56,  60,  64,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {119,  2, { 52,  60,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {120,  2, { 56,  64,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {121, 12, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144,   0}},
  {122,  6, {100, 108, 116, 124, 132, 140,   0,   0,   0,   0,   0,   0,   0}},
  {123,  6, {104, 112, 120, 128, 136, 144,   0,   0,   0,   0,   0,   0,   0}},
  {124,  4, {149, 153, 157, 161,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {125,  6, {149, 153, 157, 161, 165, 169,   0,   0,   0,   0,   0,   0,   0}},
  {126,  2, {149, 157,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {127,  2, {153, 161,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {128,  6, { 42,  58, 106, 122, 138, 155,   0,   0,   0,   0,   0,   0,   0}},
  {129,  2, { 50, 114,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}},
  {130,  6, { 42,  58, 106, 122, 138, 155,   0,   0,   0,   0,   0,   0,   0}},
};

/* For 80Mhz */
static uint8 control_chan_array_5g[] ={36, 40, 44, 48,52, 56, 60, 64, 100, 104, 108,
				    112, 116, 120, 124, 128, 132, 136, 140, 144, 149,
				    153, 157, 161};

uint8 *i5DmGet5gCntrlChanArray(uint8 *size)
{
  if (!size) {
    return NULL;
  }
  *size = sizeof(control_chan_array_5g);
  return control_chan_array_5g;
}

/* Get the regulatory class to channel map data */
const i5_dm_rc_chan_map_type *i5DmGetRegClassChannelMap(void)
{
	return rc_chan_map;
}
#endif /* MULTIAP */

int i5DmInterfaceFindMatchingInterfaceId(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *other_al_mac_address, unsigned char *other_interface_id);
void i5DmDeviceFree(i5_dm_device_type *device);

i5_dm_network_topology_type i5_dm_network_topology = {};

unsigned char const* i5DmGetNameForMediaType(unsigned short mediaType)
{
   return i5UtilsGetNameForMediaType(mediaType);
}

int i5DmIsInterfaceWireless(unsigned short mediaType)
{
   return ((mediaType >= I5_MEDIA_TYPE_WIFI_B) && (mediaType <= I5_MEDIA_TYPE_WIFI_AF));
}

int i5DmIsInterfacePlc(unsigned short mediaType, unsigned char const *netTechOui)
{
   return ((mediaType >= I5_MEDIA_TYPE_1901_WAVELET) && (mediaType <= I5_MEDIA_TYPE_1901_FFT)) ||
           ((mediaType == I5_MEDIA_TYPE_UNKNOWN) && (netTechOui) &&
            (I5_GEN_PHY_HPAV2_NETTECHOUI_01 == netTechOui[0]) &&
            (I5_GEN_PHY_HPAV2_NETTECHOUI_02 == netTechOui[1]) &&
            (I5_GEN_PHY_HPAV2_NETTECHOUI_03 == netTechOui[2]) );
}

int i5DmIsInterfaceEthernet(unsigned short mediaType)
{
   return ((mediaType >= I5_MEDIA_TYPE_FAST_ETH) && (mediaType <= I5_MEDIA_TYPE_GIGA_ETH));
}

int i5DmIsMacNull(unsigned char *mac)
{
  int index = 0;
  for ( ; index < MAC_ADDR_LEN; index ++) {
    if (mac[index] != 0) {
      return 0;
    }
  }
  return 1;
}

int i5DmIsMacWildCard(unsigned char *mac)
{
  int index = 0;

  for ( ; index < MAC_ADDR_LEN; index ++) {
    if (mac[index] != 0xff) {
      return 0;
    }
  }
  return 1;
}

int i5DmAnyWirelessInterfaceUp(i5_dm_device_type *deviceToCheck)
{
  if (NULL == deviceToCheck) {
    i5TraceError("NULL device passed in\n");
    return 0;
  }
  i5_dm_interface_type*interfaceToCheck = deviceToCheck->interface_list.ll.next;
  while (interfaceToCheck) {
    if ((i5DmIsInterfaceWireless(interfaceToCheck->MediaType)) && (interfaceToCheck->Status != IF_OPER_DOWN)) {
      return 1;
    }
    interfaceToCheck = interfaceToCheck->ll.next;
  }
  return 0;
}

/*
 *  Searches through the available interfaces and returns
 *  whether or not the given frequency band is supported.
 */
int i5DmIsWifiBandSupported(char *ifname, unsigned int freqBand)
{
#if defined(WIRELESS)
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;

  if ((pdmdev = i5DmDeviceFind(i5_config.i5_mac_address)) != NULL) {
    pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
    while (pdmif != NULL) {
      unsigned int freqBandSupported = i5TlvGetFreqBandFromMediaType (pdmif->MediaType);
      if ((i5MessageFreqBand_Reserved != freqBandSupported) && (freqBandSupported == freqBand)) {
        strcpy(ifname, pdmif->wlParentName);
        return 1;
      }
      pdmif = pdmif->ll.next;
    }
  }
#endif
  return 0;
}

/* Get the local ifname from the neibhors ALID and MAC address based on the media specific info */
int i5DmGetIfnameFromMediaSpecific(char *ifname, unsigned char *alid, unsigned char *mac)
{
#if defined(WIRELESS)
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  unsigned char chan = 0;

  if ((pdmdev = i5DmDeviceFind(alid)) != NULL) {
    pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
    while (pdmif != NULL) {
	  if (pdmif->MediaSpecificInfoSize <= 8)
		goto next;
          if (0 == memcmp(mac, pdmif->InterfaceId, MAC_ADDR_LEN)) {
		chan = (unsigned char)pdmif->MediaSpecificInfo[8];
		strcpy(ifname, pdmif->wlParentName);
		break;
	  }
next:
	pdmif = pdmif->ll.next;
    }
	}
  if (chan == 0)
	  return -1;

  if ((pdmdev = i5DmDeviceFind(i5_config.i5_mac_address)) != NULL) {
    pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
    while (pdmif != NULL) {
	    if (pdmif->MediaSpecificInfoSize <= 8)
		    goto next1;
	 if (pdmif->MediaSpecificInfo[8] <= 0)
		goto next1;

          if (chan >= 100 && pdmif->MediaSpecificInfo[8] >= 100) {
		  strcpy(ifname, pdmif->wlParentName);
		  return 0;
	  } else if (chan <= 14 && pdmif->MediaSpecificInfo[8] <= 14) {
		  strcpy(ifname, pdmif->wlParentName);
		  return 0;
	  } else if (chan >= 36 && chan < 100 && pdmif->MediaSpecificInfo[8] >= 36 && pdmif->MediaSpecificInfo[8] < 100){
		  strcpy(ifname, pdmif->wlParentName);
		return 0;
	  }
next1:
	  pdmif = pdmif->ll.next;
    }
  }

#endif
  return -1;
}

void i5DmFillNeighborInterfaceId( void )
{
  i5_dm_device_type        *pCurrDev;
  i5_dm_device_type        *pNeighDev;
  i5_dm_interface_type     *pCurrDevIf;
  i5_dm_1905_neighbor_type *pCurrDevNeigh;
  i5_dm_1905_neighbor_type *pNeighDevNeigh;
  i5_dm_interface_type     *pNeighDevIf;
#ifdef MULTIAP
  i5_dm_bss_type            *pCurrDevBSS;
  i5_dm_bss_type            *pNeighDevBSS;
#endif /* MULTIAP */

  i5Trace("\n");

  pCurrDev = i5_dm_network_topology.device_list.ll.next;
  while ( pCurrDev != NULL ) {
    pCurrDevNeigh = pCurrDev->neighbor1905_list.ll.next;
    while(pCurrDevNeigh != NULL) {
      pCurrDevIf = i5DmInterfaceFind(pCurrDev, pCurrDevNeigh->LocalInterfaceId);
#ifdef MULTIAP
      /* The neighbor interface can be virtual. In that case it will be in the BSS list */
      if (pCurrDevIf == NULL) {
        pCurrDevBSS = i5DmFindBSSFromDevice(pCurrDev, pCurrDevNeigh->LocalInterfaceId);
        if (pCurrDevBSS) {
          pCurrDevIf = (i5_dm_interface_type*)I5LL_PARENT(pCurrDevBSS);
        }
      }
#endif /* MULTIAP */
      pNeighDev = i5DmDeviceFind(pCurrDevNeigh->Ieee1905Id);
      if (pNeighDev != NULL) {
        pNeighDevNeigh = pNeighDev->neighbor1905_list.ll.next;
        while ( pNeighDevNeigh != NULL ) {
          pNeighDevIf = i5DmInterfaceFind(pNeighDev, pNeighDevNeigh->LocalInterfaceId);
#ifdef MULTIAP
          if (pNeighDevIf == NULL) {
            pNeighDevBSS = i5DmFindBSSFromDevice(pNeighDev, pNeighDevNeigh->LocalInterfaceId);
            if (pNeighDevBSS) {
              pNeighDevIf = (i5_dm_interface_type*)I5LL_PARENT(pNeighDevBSS);
            }
          }
#endif /* MULTIAP */
          if (!pCurrDevIf) {
            i5TraceError("i5DmFillNeighborInterfaceId NULL==pCurrDevIf (" I5_MAC_DELIM_FMT ")\n",
              I5_MAC_PRM(pCurrDevNeigh->LocalInterfaceId));
          }
          else
          if (!pNeighDevIf) {
            i5TraceError("i5DmFillNeighborInterfaceId NULL==pNeighDevIf (" I5_MAC_DELIM_FMT ")\n",
              I5_MAC_PRM(pNeighDevNeigh->LocalInterfaceId));
          }
          else
          if ( 0 == memcmp(pCurrDev->DeviceId, pNeighDevNeigh->Ieee1905Id, MAC_ADDR_LEN) ) {
            /* note that if we have multiple connections between two neighbors and two or more
               of the connections use interfces of the same mediatype then we will not be able
               to pick the correct neighbor interface */
            if ( pCurrDevIf->MediaType == pNeighDevIf->MediaType ) {
              /* we can update the neighbor id of the current devices neighbour entry using the
                 local device id of the neighbour devives neighbor entry
                 for the local device this is technically a device change but the neighbor
                 interface id is not in the topology response so no need to flag the change */
              memcpy(&pCurrDevNeigh->NeighborInterfaceId[0], &pNeighDevNeigh->LocalInterfaceId[0], MAC_ADDR_LEN);
              i5Json1905NeighborPrint(I5_JSON_ALL_CLIENTS, pCurrDevNeigh, i5Json_Add);
            }
          }
          pNeighDevNeigh = pNeighDevNeigh->ll.next;
        }
      }
      pCurrDevNeigh = pCurrDevNeigh->ll.next;
    }
    pCurrDev = pCurrDev->ll.next;
  }
}

i5_dm_legacy_neighbor_type *i5DmLegacyNeighborNew(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor_interface_id)
{
  i5_dm_legacy_neighbor_type *new = (i5_dm_legacy_neighbor_type *)malloc(sizeof(i5_dm_legacy_neighbor_type));

  i5Trace( I5_MAC_FMT "\n", I5_MAC_PRM(neighbor_interface_id) );

  if (NULL != new) {
    memset(new, 0, sizeof(i5_dm_legacy_neighbor_type));
    i5LlItemAdd(parent, &parent->legacy_list, new);
    ++parent->LegacyNeighborNumberOfEntries;
    memcpy(new->LocalInterfaceId, local_interface_id, MAC_ADDR_LEN);
    memcpy(new->NeighborInterfaceId, neighbor_interface_id, MAC_ADDR_LEN);
  }

  return new;
}

i5_dm_legacy_neighbor_type *i5DmLegacyNeighborFind(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor_interface_id)
{
  i5_dm_legacy_neighbor_type *item = (i5_dm_legacy_neighbor_type *)parent->legacy_list.ll.next;

  while ((item != NULL) && (memcmp(item->LocalInterfaceId, local_interface_id, MAC_ADDR_LEN) || memcmp(item->NeighborInterfaceId, neighbor_interface_id, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }
  return item;
}

int i5DmLegacyNeighborFree(i5_dm_device_type *parent, i5_dm_legacy_neighbor_type *legacy_neighbor)
{
  i5Json1905LegacyNeighborPrint(I5_JSON_ALL_CLIENTS, legacy_neighbor, i5Json_Delete);
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmRemoveNetworkTopologyDevLegacyNeighbor(parent, legacy_neighbor);
#endif
  if (i5LlItemFree(&parent->legacy_list, legacy_neighbor) == 0) {
    --parent->LegacyNeighborNumberOfEntries;
    if ( i5DmDeviceIsSelf(parent->DeviceId) ) {
       parent->hasChanged++;
    }
    return 0;
  }
  return -1;
}

int i5DmLegacyNeighborPending(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_legacy_neighbor_type *item;

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_legacy_neighbor_type *)device->legacy_list.ll.next;

  while (item != NULL) {
    item->state = i5DmStatePending;
    item = item->ll.next;
  }
  return 0;
}

int i5DmLegacyNeighborDone(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_legacy_neighbor_type *item;
  i5_dm_legacy_neighbor_type *next;

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_legacy_neighbor_type *)device->legacy_list.ll.next;
  while (item != NULL) {
    next = item->ll.next;
    if (item->state == i5DmStatePending) {
      i5DmLegacyNeighborFree(device, item);
    }
    item = next;
  }
  return 0;
}

int i5DmLegacyNeighborUpdate(unsigned char *device_id, unsigned char *local_interface_id, unsigned char *neighbor_interface_id)
{
  i5_dm_device_type *device;
  i5_dm_legacy_neighbor_type *neighbor;

  i5TraceInfo(I5_MAC_FMT "\n", I5_MAC_PRM(neighbor_interface_id));
  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  if ((neighbor = i5DmLegacyNeighborFind(device, local_interface_id, neighbor_interface_id)) == NULL) {
    if ((neighbor = i5DmLegacyNeighborNew(device, local_interface_id, neighbor_interface_id)) == NULL) {
      return -1;
    }
    else {
      i5Json1905LegacyNeighborPrint(I5_JSON_ALL_CLIENTS, neighbor, i5Json_Add);
      device->hasChanged++;
    }
  }

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(device, neighbor);
#endif

  neighbor->state = i5DmStateDone;

  return 0;
}

static int i5DmLegacyNeighborRemove(unsigned char *id)
{
  i5_dm_device_type          *pDmDevice;
  i5_dm_legacy_neighbor_type *pLegNeighbor;

  i5TraceInfo("\n");

  pDmDevice = i5DmDeviceFind(&i5_config.i5_mac_address[0]);
  if ( pDmDevice != NULL ) {
    pLegNeighbor = (i5_dm_legacy_neighbor_type *)pDmDevice->legacy_list.ll.next;
    while ( pLegNeighbor != NULL ) {
       if ( 0 == memcmp(id, pLegNeighbor->NeighborInterfaceId, MAC_ADDR_LEN) ) {
         break;
       }
       pLegNeighbor = pLegNeighbor->ll.next;
    }
    if ( pLegNeighbor != NULL ) {
      i5TraceInfo("Removing legacy neighbor matching 1905 neighbor: " I5_MAC_FMT "\n", I5_MAC_PRM(pLegNeighbor->NeighborInterfaceId));
      i5DmLegacyNeighborFree(pDmDevice, pLegNeighbor);
      i5MessageTopologyNotificationSend(NULL, NULL, 0);
    }
  }

  return 0;
}

void i5DmSetLocalInterfaceInfoForNeighbor(i5_dm_device_type *parent, i5_dm_1905_neighbor_type *neighbor, char const *localifName, int localifindex)
{
#if defined(SUPPORT_IEEE1905_FM)
  i5_dm_interface_type *pLocalInterface;
#endif

  if (!i5DmDeviceIsSelf(parent->DeviceId)) {
    return;
  }

  if ((NULL == localifName) || (0 == strlen(localifName))) {
    i5TraceAssert(1==0);
    return;
  }

  i5Trace("Neighbor " I5_MAC_DELIM_FMT " LocalIf: %s (%d)\n",
          I5_MAC_PRM(neighbor->Ieee1905Id), localifName, localifindex);

  snprintf(neighbor->localIfname, sizeof(neighbor->localIfname), localifName);
  neighbor->localIfname[sizeof(neighbor->localIfname)-1] = '\0';
  neighbor->localIfindex = localifindex;

#if defined(SUPPORT_IEEE1905_FM)
  pLocalInterface = i5DmInterfaceFind(parent, neighbor->LocalInterfaceId);
  if (i5DmIsInterfaceWireless(pLocalInterface->MediaType) || i5DmIsInterfacePlc(pLocalInterface->MediaType, pLocalInterface->netTechOui)) {
    i5FlowManagerAddConnectionIndex(neighbor->Ieee1905Id, neighbor->localIfindex);
  }
#endif /* defined(SUPPORT_IEEE1905_FM) */
}

i5_dm_1905_neighbor_type *i5Dm1905NeighborNew(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor1905_al_mac_address)
{
  i5_dm_1905_neighbor_type *newNeighbor = (i5_dm_1905_neighbor_type *)malloc(sizeof(i5_dm_1905_neighbor_type));

  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(neighbor1905_al_mac_address));
  if (NULL != newNeighbor) {
    memset(newNeighbor, 0, sizeof(i5_dm_1905_neighbor_type));
    i5LlItemAdd(parent, &parent->neighbor1905_list, newNeighbor);
    ++parent->Ieee1905NeighborNumberOfEntries;
    memcpy(newNeighbor->LocalInterfaceId, local_interface_id, MAC_ADDR_LEN);
    memcpy(newNeighbor->Ieee1905Id, neighbor1905_al_mac_address, MAC_ADDR_LEN);
#if defined(SUPPORT_IEEE1905_FM)
    i5FlowManagerProcessNewNeighbor (newNeighbor);
#endif /* defined(SUPPORT_IEEE1905_FM) */
    i5Dm1905NeighborUpdateIntermediateBridgeFlag(parent, newNeighbor, 0);
#ifdef MULTIAP
    if (i5_config.cbs.neighbor_init) {
      i5_config.cbs.neighbor_init(newNeighbor);
    }
#endif /* MULTIAP */
  }
  return newNeighbor;
}

/* returns (0 and up) the number of interfaces found
 * returns -1 if the local mac isn't in the device list
 * (It will never return more actual interfaces than "maxInterfaces",
 *    but the return *value* of the function will be higher if more interfaces are available)
 */
int i5DmGetInterfacesWithNeighbor(unsigned char const *neighbor_al_mac,
                                  unsigned char * local_interface_mac_addrs, unsigned char * neighbor_interface_mac_addrs, int maxInterfaces)
{
  i5_dm_device_type * currentDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  int totalInterfacesFound = 0;

  i5_dm_1905_neighbor_type * currentNeighbor = &currentDevice->neighbor1905_list;
  while (currentNeighbor != NULL) {
    if (memcmp (currentNeighbor->Ieee1905Id, neighbor_al_mac, MAC_ADDR_LEN) == 0) {
      if (totalInterfacesFound < maxInterfaces) {
        memcpy(local_interface_mac_addrs, currentNeighbor->LocalInterfaceId, MAC_ADDR_LEN);
        local_interface_mac_addrs += MAC_ADDR_LEN;
        memcpy(neighbor_interface_mac_addrs, currentNeighbor->NeighborInterfaceId, MAC_ADDR_LEN);
        neighbor_interface_mac_addrs += MAC_ADDR_LEN;
      }
      totalInterfacesFound ++;
    }
    // check next neighbor
    currentNeighbor = (i5_dm_1905_neighbor_type *)currentNeighbor->ll.next;
  }

   return totalInterfacesFound;
}

void i5DmProcessLocalInterfaceChange (i5_dm_device_type *parent, unsigned char * localInterfaceId)
{
   i5_dm_interface_type *dmInterface = NULL;
   unsigned char macAddressList[I5_DM_MAX_LINK_METRICS_LIST_SIZE][MAC_ADDR_LEN];
   unsigned char numMacsInList = 0;

   i5Trace("called for interfaceId " I5_MAC_FMT "\n", I5_MAC_PRM(localInterfaceId) );

   /* Go through all of our neighbors */
   i5_dm_1905_neighbor_type * currNeighbor = (i5_dm_1905_neighbor_type *)&parent->neighbor1905_list.ll.next;

   while (currNeighbor != NULL) {
      if (memcmp(currNeighbor->LocalInterfaceId, localInterfaceId, MAC_ADDR_LEN) == 0){
         /* add to MAC address list */
         if (numMacsInList >= I5_DM_MAX_LINK_METRICS_LIST_SIZE) {
            i5Trace("Too many neighbors for MAC Address List (more than %d)\n", I5_DM_MAX_LINK_METRICS_LIST_SIZE);
         } else {
            memcpy(macAddressList[numMacsInList], currNeighbor->NeighborInterfaceId, MAC_ADDR_LEN);
            numMacsInList ++;
         }
      }
      currNeighbor = currNeighbor->ll.next;
   }

   i5Trace("MAC Address List ready: %d addresses in list\n", numMacsInList);
   dmInterface = i5DmInterfaceFind(parent, localInterfaceId);
   if ((dmInterface != NULL) && (dmInterface->i5MacAddrDeliver != NULL)) {
      dmInterface->i5MacAddrDeliver( &macAddressList[0][0], numMacsInList);
   }
   else {
      i5Trace("Nowhere to deliver list\n");
   }
}

void i5DmRetryPlcRegistry(void)
{
  unsigned char *localId = i5DmInterfaceGetLocalPlcInterface()->InterfaceId;
  i5_dm_device_type *selfDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  if (NULL == selfDevice) {
    return;
  }
  i5Trace("adding all PLC registries\n");
  i5DmProcessLocalInterfaceChange(selfDevice, localId);
}


i5_dm_1905_neighbor_type *i5Dm1905FindNeighborByRemoteInterface(i5_dm_device_type const *parent, unsigned char const *remote_interface_id)
{
  i5_dm_1905_neighbor_type *item = (i5_dm_1905_neighbor_type *)parent->neighbor1905_list.ll.next;

  while ((item != NULL) && memcmp(item->NeighborInterfaceId, remote_interface_id, MAC_ADDR_LEN)) {
    item = item->ll.next;
  }
  return item;
}

i5_dm_1905_neighbor_type *i5Dm1905NeighborFind(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *neighbor1905_al_mac_address)
{
  i5_dm_1905_neighbor_type *item = (i5_dm_1905_neighbor_type *)parent->neighbor1905_list.ll.next;

  while ((item != NULL) && (memcmp(item->LocalInterfaceId, local_interface_id, MAC_ADDR_LEN) || memcmp(item->Ieee1905Id, neighbor1905_al_mac_address, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }
  return item;
}

int i5Dm1905NeighborFree(i5_dm_device_type *parent, i5_dm_1905_neighbor_type *neighbor_1905)
{
  unsigned char local_interface_id[MAC_ADDR_LEN];

  if ( i5DmDeviceIsSelf(parent->DeviceId) ) {
    if ( neighbor_1905->bridgeDiscoveryTimer ) {
      i5TimerFree(neighbor_1905->bridgeDiscoveryTimer);
    }
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    i5CmsMdmLocalNeighborRemove(neighbor_1905);
#endif
  }

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmRemoveNetworkTopologyDevNeighbor(parent, neighbor_1905);
#endif

  i5Trace(I5_MAC_FMT " " I5_MAC_FMT "\n", I5_MAC_PRM(parent->DeviceId), I5_MAC_PRM(neighbor_1905->Ieee1905Id));
  i5Json1905NeighborPrint(I5_JSON_ALL_CLIENTS, neighbor_1905, i5Json_Delete);
#if defined(SUPPORT_IEEE1905_FM)
  i5FlowManagerProcessNeighborRemoved(neighbor_1905);
#endif /* defined(SUPPORT_IEEE1905_FM) */

#ifdef MULTIAP
  if (i5_config.cbs.neighbor_deinit) {
    i5_config.cbs.neighbor_deinit(neighbor_1905);
  }
#endif /* MULTIAP */

  memcpy (local_interface_id, neighbor_1905->LocalInterfaceId, MAC_ADDR_LEN);
  if (i5LlItemFree(&parent->neighbor1905_list, neighbor_1905) == 0) {
    --parent->Ieee1905NeighborNumberOfEntries;
    if (i5DmDeviceIsSelf(parent->DeviceId)) {
      /* update the MAC address list based on media type*/
      i5DmProcessLocalInterfaceChange (parent, local_interface_id);
      parent->hasChanged++;
    }
    return 0;
  }
  return -1;
}

void i5Dm1905NeighborFreeAllLinksRemoteDevice(unsigned char *remoteDeviceAlMac)
{
  i5Trace(I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(remoteDeviceAlMac));
  i5_dm_device_type *localDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  i5_dm_1905_neighbor_type *neigh = localDevice->neighbor1905_list.ll.next;
  while (neigh) {
    i5_dm_1905_neighbor_type *nextNeigh = neigh->ll.next;
    if (memcmp(neigh->Ieee1905Id, remoteDeviceAlMac, MAC_ADDR_LEN) == 0) {
      i5Dm1905NeighborFree(localDevice, neigh);
    }
    neigh = nextNeigh;
  }

}

int i5Dm1905NeighborPending(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_1905_neighbor_type *item;

  i5Trace("\n");
  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;

  while (item != NULL) {
    item->state = i5DmStatePending;
    item = item->ll.next;
  }
  return 0;
}

int i5Dm1905NeighborDone(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_1905_neighbor_type *item;
  i5_dm_1905_neighbor_type *next;

  i5Trace("\n");
  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;
  while (item != NULL) {
    next = item->ll.next;
    if (item->state == i5DmStatePending) {
      i5Dm1905NeighborFree(device, item);
    }
    item = next;
  }
  return 0;
}

#ifndef MULTIAP
void i5DmUpdateNeighborLinkMetrics(void *arg)
{
  i5_dm_device_type *pDevice;

  i5TraceInfo("\n");

  i5TimerFree(i5_dm_network_topology.pLinkMetricTimer);

  pDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  if ( NULL != pDevice ) {
    i5_dm_interface_type *pInterface = pDevice->interface_list.ll.next;
    while ( pInterface != NULL ) {
      /* update the metrics for each Ethernet interface and then update
         the corresponding neighbor entries */
      if ( i5DmIsInterfaceEthernet(pInterface->MediaType) ) {
        i5_socket_type *pif = i5SocketFindDevSocketByAddr(pInterface->InterfaceId, NULL);
        while ( pif != NULL ) {
          i5TraceModuleInfo(i5TraceEthStat, "Update stats for %s (%d)\n", i5SocketGetIfName(pif), i5SocketGetIfIndex(pif));
          if ( pif->u.sll.pInterfaceCtx != NULL ) {
            int rt = 0;
            long capacity;
            long dataRate;
            rt = i5EthStatGetDataRateAndCapacity(pif->u.sll.pInterfaceCtx, &dataRate, &capacity);
            if (rt >= 0) {
              i5_dm_1905_neighbor_type *pNeighbor = pDevice->neighbor1905_list.ll.next;
              const long MBit = 1000000L;
              i5TraceModuleInfo(i5TraceEthStat, "capacity=%ld, dataRate=%ld\n", capacity, dataRate);
              while (pNeighbor != NULL) {
                if ( pNeighbor->localIfindex == i5SocketGetIfIndex(pif)) {
                  i5Dm1905NeighborBandwidthUpdate(pNeighbor, (capacity*8)/MBit, ((capacity - dataRate)*8)/MBit, 0, pDevice->DeviceId);
                }
                pNeighbor = pNeighbor->ll.next;
              }
            }
          }
          pif = i5SocketFindDevSocketByAddr(pInterface->InterfaceId, pif);
        }
      }

#if defined(WIRELESS)
      /* collect WL neigbour devices and query WL for metrics*/
      if ( i5DmIsInterfaceWireless(pInterface->MediaType) ) {
         unsigned char macAddressList[I5_DM_MAX_LINK_METRICS_LIST_SIZE][MAC_ADDR_LEN];
         int count = 0;

         i5_dm_1905_neighbor_type *pNeighbor = pDevice->neighbor1905_list.ll.next;
         while (pNeighbor != NULL) {
           if ( 0 == memcmp(pNeighbor->LocalInterfaceId, pInterface->InterfaceId, MAC_ADDR_LEN) ) {
             memcpy(&macAddressList[count][0], pNeighbor->NeighborInterfaceId, MAC_ADDR_LEN);
             count++;
           }
           pNeighbor = pNeighbor->ll.next;
         }
         if ( count > 0 ) {
           i5wlmMetricUpdateLinkMetrics(pInterface->wlParentName, count, &macAddressList[0][0]);
         }
      }
#endif
      pInterface = pInterface->ll.next;
    }
  }

  i5_dm_network_topology.pLinkMetricTimer = i5TimerNew(I5_DM_LINK_METRICS_GET_INTERVAL_MSEC,
    i5DmUpdateNeighborLinkMetrics, NULL);
}
#endif /* MULTIAP */

int i5Dm1905NeighborUpdate(     unsigned char *device_id,
                                unsigned char *local_interface_id,
                                unsigned char *neighbor1905_al_mac_address,
                                unsigned char *neighbor1905_interface_id,
                                unsigned char *intermediate_legacy_bridge,
                                char          *localifName,
                                int            localifindex,
                                unsigned char  createNeighbor)
{
  i5_dm_device_type *device, *neighbor_dev;
  i5_dm_1905_neighbor_type *neighbor;

  if (neighbor1905_interface_id) {
    i5Trace("Dev_id " I5_MAC_FMT " Loc IF " I5_MAC_FMT
            " Nei_id " I5_MAC_FMT " Nei IF " I5_MAC_FMT "\n",
      I5_MAC_PRM(device_id), I5_MAC_PRM(local_interface_id),
      I5_MAC_PRM(neighbor1905_al_mac_address), I5_MAC_PRM(neighbor1905_interface_id)
      );
  } else {
    i5Trace("Dev_id " I5_MAC_FMT " Loc IF " I5_MAC_FMT
            " Nei_id " I5_MAC_FMT " Nei IF  { NULL } \n",
      I5_MAC_PRM(device_id), I5_MAC_PRM(local_interface_id),
      I5_MAC_PRM(neighbor1905_al_mac_address)
      );
  }

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  /* Update the controller agent flag for the device wich is running on the same device.
   * Do it only for controller.
   */
  if (I5_IS_MULTIAP_CONTROLLER(i5_dm_network_topology.selfDevice->flags) &&
    device == i5_dm_network_topology.selfDevice) {

    /* If the neighbor device present and if the local devices interface MAC is NULL(loopback), then
     * its an other 1905 stack running on the same device
     */
    if (i5DmIsMacNull(local_interface_id)) {
      if ((neighbor_dev = i5DmDeviceFind(neighbor1905_al_mac_address)) != NULL) {
        neighbor_dev->flags |= I5_CONFIG_FLAG_CTRLAGENT;
      }
    }
  }

  neighbor = i5Dm1905NeighborFind(device, local_interface_id, neighbor1905_al_mac_address);
  if ( neighbor == NULL ) {
    if (0 == createNeighbor) {
      return 0;
    }

    i5Trace("Creating New Neighbor\n");
    if ((neighbor = i5Dm1905NeighborNew(device, local_interface_id, neighbor1905_al_mac_address)) == NULL) {
      return -1;
    }
    else {
      i5Trace("Device has new neighbor\n");
      device->hasChanged++;
    }
  }

  if ( neighbor1905_interface_id && memcmp(neighbor1905_interface_id, neighbor->NeighborInterfaceId, MAC_ADDR_LEN)) {
    memcpy(neighbor->NeighborInterfaceId, neighbor1905_interface_id, MAC_ADDR_LEN);
    i5DmLegacyNeighborRemove(neighbor1905_interface_id);
    /* for the local device this is technically a device change but the neighbor
       interface id is not in the topology response so no need to flag the change */
    if (i5DmDeviceIsSelf(device_id)) {
      i5DmSetLocalInterfaceInfoForNeighbor (neighbor->ll.parent, neighbor, localifName, localifindex);
      /* update the MAC address list based on media type*/
      i5DmProcessLocalInterfaceChange (device, local_interface_id);
    }
    if (!i5DmIsMacNull(neighbor->NeighborInterfaceId)) {
      i5Json1905NeighborPrint(I5_JSON_ALL_CLIENTS, neighbor, i5Json_Add);
    }
  }

  if (intermediate_legacy_bridge) {
   i5Dm1905NeighborUpdateIntermediateBridgeFlag(device, neighbor, *intermediate_legacy_bridge);
  }

  i5DmFillNeighborInterfaceId();
  neighbor->state = i5DmStateDone;

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  if (i5DmDeviceIsSelf(device_id)) {
    i5CmsMdmLocalNeighborUpdate(neighbor);
  }
  i5CmsMdmUpdateNetworkTopologyDevNeighbor(device, neighbor);
#endif

  return 0;
}

/* For a given remote interface MAC, this function searches through the neighbors of the local device
   and returns the neighbor structure that corresponds to the given remote MAC */
i5_dm_1905_neighbor_type *i5Dm1905GetLocalNeighbor (unsigned char const *neighbor1905_interface_id)
{
   i5_dm_device_type const *selfDevice = i5DmDeviceFind(i5_config.i5_mac_address);
   if (NULL == selfDevice) {
     return NULL;
   }
   i5_dm_1905_neighbor_type *neighbor = i5Dm1905FindNeighborByRemoteInterface(selfDevice, neighbor1905_interface_id);
   if (NULL == neighbor) {
     return NULL;
   }

   i5TraceInfo("%02x:%02x:%02x:%02x:%02x:%02x\n",
      neighbor1905_interface_id[0], neighbor1905_interface_id[1], neighbor1905_interface_id[2],
      neighbor1905_interface_id[3], neighbor1905_interface_id[4], neighbor1905_interface_id[5]);

   return neighbor;
}

int i5Dm1905NeighborBandwidthUpdate ( i5_dm_1905_neighbor_type *neighbor,
                                      unsigned short MacThroughputCapacity,
                                      unsigned short availableThroughputCapacity,
                                      unsigned int rxBytesCumulative,
                                      unsigned char* sourceAlMac)
{
    /* Note: capacities are in Mbit/s here */

   if (!neighbor) {
      return -1;
   }
   neighbor->MacThroughputCapacity = MacThroughputCapacity;
   neighbor->availableThroughputCapacity = availableThroughputCapacity;
   neighbor->prevRxBytes = neighbor->latestRxBytes;
   neighbor->latestRxBytes = rxBytesCumulative;

   if (neighbor->ignoreLinkMetricsCountdown) {
      neighbor->ignoreLinkMetricsCountdown --;
      i5Trace("Countdown now %d\n",neighbor->ignoreLinkMetricsCountdown);
      if (!neighbor->ignoreLinkMetricsCountdown) {
         i5_dm_network_topology.updateStpNeeded = 1;
      }
   }

   if (i5DmDeviceIsSelf(sourceAlMac)) {
     /* Data from our local drivers is given preference */
     i5Json1905NeighborUpdatePrint(I5_JSON_ALL_CLIENTS, neighbor, NULL);
   }
   else if (i5DmDeviceIsSelf(neighbor->Ieee1905Id)) {
     /* Don't listen to another device telling us about ourselves */
   }
   else {
     /* This is a link between two devices which aren't us, so take an average of both ends */
     i5_dm_device_type *symmDevice = i5DmDeviceFind(neighbor->Ieee1905Id);
     i5_dm_1905_neighbor_type *symmNeighbor = NULL;
     int success = 0;

     if (symmDevice != NULL) {
       symmNeighbor = i5Dm1905NeighborFind(symmDevice, neighbor->NeighborInterfaceId, sourceAlMac);
       if (symmNeighbor != NULL) {
         i5Json1905NeighborUpdatePrint(I5_JSON_ALL_CLIENTS, neighbor, symmNeighbor);
         success = 1;
       }
     }

     if (0 == success) {
       /* couldn't do the average, so take what was given */
       i5Json1905NeighborUpdatePrint(I5_JSON_ALL_CLIENTS, neighbor, NULL);
     }
   }

#if defined(SUPPORT_IEEE1905_FM) && defined(SUPPORT_FBCTL)
   i5FlowManagerCheckNeighborForOverload(neighbor);
#endif /* defined(SUPPORT_IEEE1905_FM) */

   return 0;
}

#ifdef MULTIAP
/* Update the transmitter and reciever link metric. updateFlag is 0 for TX, 1 for Rx and 2 for Both */
int i5Dm1905NeighborLinkMetricUpdate(i5_dm_1905_neighbor_type *neighbor,
  ieee1905_backhaul_link_metric *metric, unsigned int rxBytesCumulative, unsigned char updateFlag)
{
  i5_dm_device_type *pDeviceSource, *pDeviceNeighbor;

  /* Note: capacities are in Mbit/s here */

  if (!neighbor) {
    return -1;
  }
  pDeviceSource = neighbor->ll.parent;

  if ((pDeviceNeighbor = i5DmDeviceFind(neighbor->Ieee1905Id)) == NULL) {
    return -1;
  }

  clock_gettime(CLOCK_REALTIME, &neighbor->metric.queried);
  i5TraceModuleInfo(i5TraceEthStat, "capacity=%d linkAvailability=%d latestRxBytes=%d "
    "rxBytesCumulative=%d txPacketErrors=%d transmittedPackets=%d metric->phyRate=%d "
    "receivedPackets=%d rxPacketErrors=%d rcpi=%d\n",
    metric->macThroughPutCapacity, metric->linkAvailability, metric->latestRxBytes,
    rxBytesCumulative, metric->txPacketErrors, metric->transmittedPackets,
    metric->phyRate, metric->receivedPackets, metric->rxPacketErrors, metric->rcpi);

  /* Update Tx */
  if (updateFlag & I5_DM_LINK_METRIC_UPDATE_TX) {
    neighbor->metric.macThroughPutCapacity = metric->macThroughPutCapacity;
    neighbor->metric.linkAvailability = metric->linkAvailability;

    /* Store link rate between the sending device and the parent device. Dont store the rate if
     * the sending agent is running in the same device
     */
    if (!I5_IS_CTRLAGENT(pDeviceSource->flags) && pDeviceSource->parentDevice == pDeviceNeighbor) {
      pDeviceSource->macThroughPutCapacity = metric->macThroughPutCapacity;
    }

    if (updateFlag & I5_DM_LINK_METRIC_UPDATE_RAW) { /* Dont take the delta values */
      neighbor->metric.txPacketErrors = metric->txPacketErrors;
      neighbor->metric.transmittedPackets = metric->transmittedPackets;
    } else {
      neighbor->metric.txPacketErrors = metric->txPacketErrors - neighbor->old_metric.txPacketErrors;
      neighbor->metric.transmittedPackets =
        (metric->transmittedPackets - neighbor->old_metric.transmittedPackets);
      neighbor->old_metric.txPacketErrors = metric->txPacketErrors;
      neighbor->old_metric.transmittedPackets = metric->transmittedPackets;
    }
    neighbor->metric.phyRate = metric->phyRate;
  }

  /* Update Rx */
  if (updateFlag & I5_DM_LINK_METRIC_UPDATE_RX) {
    if (updateFlag & I5_DM_LINK_METRIC_UPDATE_RAW) { /* Dont take the delta values */
      neighbor->metric.receivedPackets = metric->receivedPackets;
      neighbor->metric.rxPacketErrors = metric->rxPacketErrors;
    } else {
      neighbor->metric.receivedPackets = metric->receivedPackets - neighbor->old_metric.receivedPackets;
      neighbor->metric.rxPacketErrors = metric->rxPacketErrors - neighbor->old_metric.rxPacketErrors;
      neighbor->old_metric.receivedPackets = metric->receivedPackets;
      neighbor->old_metric.rxPacketErrors = metric->rxPacketErrors;
    }
    neighbor->metric.rcpi = metric->rcpi;
  }

  if (updateFlag & I5_DM_LINK_METRIC_UPDATE_LOCAL) {
    neighbor->metric.prevRxBytes = metric->latestRxBytes;
    neighbor->metric.latestRxBytes = rxBytesCumulative;
  }

  if (neighbor->ignoreLinkMetricsCountdown) {
    neighbor->ignoreLinkMetricsCountdown --;
    i5Trace("Countdown now %d\n",neighbor->ignoreLinkMetricsCountdown);
    if (!neighbor->ignoreLinkMetricsCountdown) {
      i5_dm_network_topology.updateStpNeeded = 1;
    }
  }

  return 0;
}
#endif /* MULTIAP */

/* Given a neighbor device, return the interface structure of the interface that connects to it */
i5_dm_interface_type *i5Dm1905GetLocalInterface(i5_dm_1905_neighbor_type const *neighbor)
{
   i5_dm_device_type* devSelf = i5DmDeviceFind(i5_config.i5_mac_address);
   if (NULL == devSelf) {
      return NULL;
   }
   return i5DmInterfaceFind(devSelf, neighbor->LocalInterfaceId);
}

i5_dm_bridging_tuple_info_type *i5DmBridgingTupleNew(i5_dm_device_type *parent, char *ifname, unsigned char tuple_num_macaddrs, unsigned char *forwarding_interface_list)
{
  i5_dm_bridging_tuple_info_type *new = (i5_dm_bridging_tuple_info_type *)malloc(sizeof(i5_dm_bridging_tuple_info_type));

  i5Trace("\n");
  if (NULL != new) {
    memset(new, 0, sizeof(i5_dm_bridging_tuple_info_type));
    i5LlItemAdd(parent, &parent->bridging_tuple_list, new);
    ++parent->BridgingTuplesNumberOfEntries;
    if ( ifname ) {
      memcpy(new->ifname, ifname, I5_MAX_IFNAME);
    }
    new->forwardingInterfaceListNumEntries = tuple_num_macaddrs;
    memcpy(new->ForwardingInterfaceList, forwarding_interface_list, tuple_num_macaddrs*MAC_ADDR_LEN);
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    i5CmsMdmUpdateBridgingTuple(parent, new);
#endif

    return new;
  }
  return NULL;
}

int i5DmBridgingTupleFree(i5_dm_device_type *parent, i5_dm_bridging_tuple_info_type *bridging_tuple)
{
  if ( NULL == bridging_tuple) {
    return -1;
  }

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmDeleteBridgingTuple(parent, bridging_tuple);
#endif

  if (i5LlItemFree(&parent->bridging_tuple_list, bridging_tuple) == 0) {
    --parent->BridgingTuplesNumberOfEntries;
    return 0;
  }
  return -1;
}

int i5DmBridgingTupleUpdate(unsigned char *device_id, int version, char *ifname, unsigned char tuple_num_macaddrs, unsigned char *bridging_tuple_list)
{
  i5_dm_device_type *device;
  i5_dm_bridging_tuple_info_type *pdmbrtuple;

  i5Trace("ifname %s, numentries %d\n", ifname ? ifname : "NULL", tuple_num_macaddrs );
  device = i5DmDeviceFind(device_id);
  if (device == NULL) {
    return -1;
  }

  if ( 0 == tuple_num_macaddrs ) {
    pdmbrtuple = (i5_dm_bridging_tuple_info_type *)device->bridging_tuple_list.ll.next;
    while(pdmbrtuple != NULL) {
      i5_dm_bridging_tuple_info_type *nexttuple = pdmbrtuple->ll.next;
      /* NULL ifname means delete all, otherwise delete matching entry */
      if ( (NULL == ifname) || (0 == strcmp(ifname, pdmbrtuple->ifname)) ) {
        if ( i5DmDeviceIsSelf(device_id) ) {
          device->hasChanged++;
        }
        i5DmBridgingTupleFree(device, pdmbrtuple);
      }
      pdmbrtuple = nexttuple;
    }
  }
  else {
    if ( tuple_num_macaddrs > I5_DM_BRIDGE_TUPLE_MAX_INTERFACES ) {
      return -1;
    }

    /* find a matching tuple */
    if ( NULL == ifname ) {
      i5_dm_bridging_tuple_info_type *savedTuple = NULL;
      /* NULL ifname means this is not the local device.
         A matching device is one that is in the pending state.
         To maintain the order, we want the entry that is closest to the
         last entry of the list since this was the first one added */
      pdmbrtuple = (i5_dm_bridging_tuple_info_type *)device->bridging_tuple_list.ll.next;
      while (pdmbrtuple != NULL) {
        if ( pdmbrtuple->state == i5DmStatePending ) {
          savedTuple = pdmbrtuple;
        }
        pdmbrtuple = pdmbrtuple->ll.next;
      }
      pdmbrtuple = savedTuple;
    }
    else {
      pdmbrtuple = (i5_dm_bridging_tuple_info_type *)device->bridging_tuple_list.ll.next;
      while (pdmbrtuple != NULL) {
        if (0 == strcmp(ifname, pdmbrtuple->ifname)) {
          break;
        }
        pdmbrtuple = pdmbrtuple->ll.next;
      }
    }

    /* found an entry to update */
    if (pdmbrtuple) {
      int i, j;
      int found;
      int changed = 0;

      for (i = 0; i < tuple_num_macaddrs; i++ ) {
        found = 0;
        for ( j = 0; j < pdmbrtuple->forwardingInterfaceListNumEntries; j++) {
          if (0 == memcmp(&bridging_tuple_list[i*MAC_ADDR_LEN], &pdmbrtuple->ForwardingInterfaceList[j*MAC_ADDR_LEN], MAC_ADDR_LEN)) {
            found = 1;
            break;
          }
        }
        if ( found == 0 ) {
          /* interface added */
          changed = 1;
        }
      }

      for (i = 0; i < pdmbrtuple->forwardingInterfaceListNumEntries; i++ ) {
        found = 0;
        for ( j = 0; j < tuple_num_macaddrs; j++) {
          if (0 == memcmp(&pdmbrtuple->ForwardingInterfaceList[i*MAC_ADDR_LEN], &bridging_tuple_list[j*MAC_ADDR_LEN], MAC_ADDR_LEN)) {
            found = 1;
            break;
          }
        }
        if ( found == 0 ) {
          /* interface removed */
          changed = 1;
        }
      }

      pdmbrtuple->state = i5DmStateDone;
      if ( changed ) {
        /* update entry */
        for (i = 0; i < tuple_num_macaddrs; i++ ) {
          memcpy(&pdmbrtuple->ForwardingInterfaceList[i*MAC_ADDR_LEN], &bridging_tuple_list[i*MAC_ADDR_LEN], MAC_ADDR_LEN);
        }
        pdmbrtuple->forwardingInterfaceListNumEntries = tuple_num_macaddrs;
        if ( i5DmDeviceIsSelf(device_id) ) {
          device->hasChanged++;
        }
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
        i5CmsMdmUpdateBridgingTuple(device, pdmbrtuple);
#endif
      }
    }
    else {
      i5DmBridgingTupleNew(device, ifname, tuple_num_macaddrs, bridging_tuple_list);
      if ( i5DmDeviceIsSelf(device_id) ) {
        device->hasChanged++;
      }
    }
  }
  return 0;
}

int i5DmBridgingTuplePending(unsigned char *device_id)
{
  i5_dm_device_type *pdevice;
  i5_dm_bridging_tuple_info_type *item;

  if ((pdevice = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_bridging_tuple_info_type *)pdevice->bridging_tuple_list.ll.next;
  while (item != NULL) {
    item->state = i5DmStatePending;
    item = item->ll.next;
  }
  return 0;
}

int i5DmBridgingTupleDone(unsigned char *device_id)
{
  i5_dm_device_type *pdevice;
  i5_dm_bridging_tuple_info_type *item;
  i5_dm_bridging_tuple_info_type *next;

  if ((pdevice = i5DmDeviceFind(device_id)) == NULL) {
    return -1;
  }

  item = (i5_dm_bridging_tuple_info_type *)pdevice->bridging_tuple_list.ll.next;
  while (item != NULL) {
    next = item->ll.next;
    if (item->state == i5DmStatePending) {
      i5DmBridgingTupleFree(pdevice, item);
    }
    item = next;
  }
  return 0;
}

void i5Dm1905NeighborBridgeDiscoveryTimeout(void *arg)
{
  i5_dm_device_type* selfDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  if (NULL == selfDevice) {
    return;
  }
  i5_dm_1905_neighbor_type *neighbor = (i5_dm_1905_neighbor_type *)arg;

  i5Dm1905NeighborUpdateIntermediateBridgeFlag(selfDevice, neighbor, 1);
}

void i5Dm1905NeighborUpdateIntermediateBridgeFlag(i5_dm_device_type *device, i5_dm_1905_neighbor_type *neighbor, unsigned char bridgeFlag)
{
  if ( i5DmDeviceIsSelf(device->DeviceId) ) {
    if ( neighbor->bridgeDiscoveryTimer ) {
      i5TimerFree(neighbor->bridgeDiscoveryTimer);
    }
    neighbor->bridgeDiscoveryTimer = i5TimerNew(I5_DM_NEIGHBOR_BRIDGE_DISCOVERY_TIMEOUT, i5Dm1905NeighborBridgeDiscoveryTimeout, neighbor);
  }

  if ( neighbor->IntermediateLegacyBridge != bridgeFlag) {
    neighbor->IntermediateLegacyBridge = bridgeFlag;
    device->hasChanged++;
  }
}

i5_dm_interface_type *i5DmInterfaceGetLocalPlcInterface(void)
{
  i5_dm_device_type* selfDevice = i5DmDeviceFind(i5_config.i5_mac_address);
  if (NULL == selfDevice) {
    return NULL;
  }
  i5_dm_interface_type *currInterface = (i5_dm_interface_type *)selfDevice->interface_list.ll.next;

  while (currInterface != NULL) {
    if (i5DmIsInterfacePlc(currInterface->MediaType, currInterface->netTechOui)) {
      return currInterface;
    }
    currInterface = currInterface->ll.next;
  }
  return NULL;
}

/* parentDev, destinterface and interface_id MUST NOT be null
 * media_type MUST be set (i.e. GEN PHY calls MUST set it to 0xFFFF even if that's already known)
 */
void i5DmInterfaceCopyInfo(i5_dm_device_type* parentDev, i5_dm_interface_type *destInterface,
                           unsigned char *interface_id, unsigned short media_type,
                           unsigned char const *media_specific_info, unsigned int media_specific_info_size,
                           unsigned char const *pNetTechOui, unsigned char const *pNetTechVariant, unsigned char const *pNetTechName, unsigned char const *url,
                           i5MacAddressDeliveryFunc deliverFunc)
{
  i5Trace(I5_MAC_DELIM_FMT " %s %d %s\n", I5_MAC_PRM(interface_id),
          i5DmIsInterfaceWireless(media_type) ? "WL" : "NOT-WL",
          destInterface->Status,
          (destInterface->Status == IF_OPER_DOWN) ? "DOWN" : "UP" );
  if (destInterface) {
    unsigned char mediaTypeChanged = 0;

    memcpy(destInterface->InterfaceId, interface_id, MAC_ADDR_LEN);
    if (destInterface->MediaType != media_type) {
      destInterface->MediaType = media_type;
      mediaTypeChanged = 1;
    }
    if (deliverFunc == NULL) {
      if (destInterface->i5MacAddrDeliver == NULL) {
        i5Trace("  DeliveryFunc not set.\n");
      }
    }
    else {
      destInterface->i5MacAddrDeliver = deliverFunc;
    }

    if ((NULL != media_specific_info) && (media_specific_info_size > 0)) {
      if (media_specific_info_size > I5_MEDIA_SPECIFIC_INFO_MAX_SIZE) {
        printf("Error media_specific_info_size > I5_MEDIA_SPECIFIC_INFO_MAX_SIZE\n");
      } else {
        destInterface->MediaSpecificInfoSize = media_specific_info_size;
        memcpy(destInterface->MediaSpecificInfo, media_specific_info, media_specific_info_size);
      }
    }
    if (pNetTechOui) {
      memcpy(destInterface->netTechOui, pNetTechOui, I5_PHY_INTERFACE_NETTECHOUI_SIZE );
    }
    if (pNetTechVariant) {
      destInterface->netTechVariant = *pNetTechVariant;
    }
    if (pNetTechName) {
      memcpy(destInterface->netTechName, pNetTechName, I5_PHY_INTERFACE_NETTECHNAME_SIZE );
    }
    if (url) {
      strncpy((char *)destInterface->url, (char *)url, sizeof(destInterface->url));
    }
    if (destInterface->state != i5DmStateNew) {
      if (mediaTypeChanged == 1) {
        /* This will cause only the label (the ifname) to be updated */
        i5JsonUpdateInterfacePrint(I5_JSON_ALL_CLIENTS, destInterface);
        if ( i5DmDeviceIsSelf(parentDev->DeviceId) ) {
          parentDev->hasChanged++;
        }
      }
    }
    else {
      i5JsonInterfacePrint(I5_JSON_ALL_CLIENTS, parentDev, destInterface, i5Json_Add);
    }
#if defined(SUPPORT_IEEE1905_FM) && defined(SUPPORT_IEEE1905_AUTO_WDS)
    if (i5DmIsInterfaceWireless(media_type) && (destInterface->Status != IF_OPER_DOWN ) ) {
      i5FlowManagerProcessWirelessUp();
    }
#endif
    if (i5DmDeviceIsSelf(parentDev->DeviceId) ) {
      i5DmLegacyNeighborRemove(interface_id);
    }
  }
}

i5_dm_interface_type *i5DmInterfaceNew(i5_dm_device_type *parent)
{
  i5_dm_interface_type *newIf = (i5_dm_interface_type *)malloc(sizeof(i5_dm_interface_type));

  if (NULL != newIf) {
    memset(newIf, 0, sizeof(i5_dm_interface_type));
    i5LlItemAdd(parent, &parent->interface_list, newIf);
    ++parent->InterfaceNumberOfEntries;
    return newIf;
  }

  return NULL;
}

i5_dm_interface_type *i5DmInterfaceFind(i5_dm_device_type const *parent, unsigned char const *interface_id)
{
  i5_dm_interface_type *item = (i5_dm_interface_type *)parent->interface_list.ll.next;

  while ((item != NULL) && (memcmp(item->InterfaceId, interface_id, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }
  return item;
}

int i5DmInterfaceFree(i5_dm_device_type *parent, i5_dm_interface_type *interface)
{
  i5JsonInterfacePrint(I5_JSON_ALL_CLIENTS, parent, interface, i5Json_Delete);
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmRemoveNetworkTopologyDevIfc(parent, interface);
#endif

#ifdef MULTIAP
  /* First Free all the BSS */
  while (interface->bss_list.ll.next != NULL) {
    i5DmBSSFree(interface, interface->bss_list.ll.next);
  }
#endif /* MULTIAP */

  i5DmFreeRadioCaps(&interface->ApCaps.RadioCaps);

#ifdef MULTIAP
  if (i5_config.cbs.interface_deinit) {
    i5_config.cbs.interface_deinit(interface);
  }
#endif /* MULTIAP */

  if (i5LlItemFree(&parent->interface_list, interface) == 0) {
    --parent->InterfaceNumberOfEntries;
    if ( i5DmDeviceIsSelf(parent->DeviceId) ) {
      parent->hasChanged++;
    }
    return 0;
  }
  return -1;
}

void i5DmInterfacePending(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_interface_type *interface;

  if ((device = i5DmDeviceFind(device_id)) != NULL) {
    interface = (i5_dm_interface_type *)device->interface_list.ll.next;
    while (interface != NULL) {
      interface->state = i5DmStatePending;
      interface = interface->ll.next;
    }
  }
}

void i5DmInterfaceDone(unsigned char *device_id)
{
  i5_dm_device_type *device;
  i5_dm_interface_type *interface, *next;

  if ((device = i5DmDeviceFind(device_id)) != NULL) {
    interface = (i5_dm_interface_type *)device->interface_list.ll.next;
    while (interface != NULL) {
      next = interface->ll.next;
      if (interface->state == i5DmStatePending) {
        i5DmInterfaceFree(device, interface);
      }
      interface = next;
    }
  }
}

int i5DmInterfaceUpdate(unsigned char *device_id, unsigned char *interface_id, int version, unsigned short media_type,
                        unsigned char const *media_specific_info, unsigned int media_specific_info_size,
                        i5MacAddressDeliveryFunc deliverFunc, char const *ifname, unsigned char status)
{
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pinterface;
  chanspec_t chanspec = 0;

  i5Trace(I5_MAC_FMT ", ifname (%s)\n", I5_MAC_PRM(interface_id), ifname ? ifname : "NULL");

  pdevice = i5DmDeviceFind(device_id);
  if ( pdevice == NULL ) {
    return -1;
  }

  if ((pinterface = i5DmInterfaceFind(pdevice, interface_id)) == NULL) {
    if ((pinterface = i5DmInterfaceNew(pdevice)) == NULL) {
      return -1;
    }
    pdevice->hasChanged++;
    pinterface->state = i5DmStateNew;
    pinterface->Status = status;
    if (ifname != NULL) {
      memcpy(pinterface->ifname, ifname, sizeof(pinterface->ifname));
    }
#if defined(WIRELESS)
    if (i5DmIsInterfaceWireless(media_type)) {
      if ( (ifname != NULL) && i5DmDeviceIsSelf(device_id)) {
        char wlname[I5_MAX_IFNAME];
        char *wlparent;
        wlparent = i5WlcfgGetWlParentInterface(ifname, &wlname[0]);
        if (wlparent) {
          strncpy(pinterface->wlParentName, wlparent, I5_MAX_IFNAME-1);
          pinterface->wlParentName[I5_MAX_IFNAME-1] = '\0';
        }
      }

#ifdef MULTIAP
      /* For neighbor device get the chanspec from media information */
      if ((pinterface->chanspec == 0) && !i5DmDeviceIsSelf(device_id) && media_specific_info) {
        i5WlCfgGetChanspecFromMediaInfo((unsigned char*)media_specific_info, &pinterface->chanspec);
      }
#endif /* MULTIAP */
    }
#endif

#ifdef MULTIAP
    if (i5_config.cbs.interface_init) {
      i5_config.cbs.interface_init(pinterface);
    }
#endif /* MULTIAP */
  } else {
#ifdef MULTIAP
    /* For neighbor device get the chanspec from media information */
    if (!I5_IS_MULTIAP_CONTROLLER(pdevice->flags) && !i5DmDeviceIsSelf(device_id) && media_specific_info) {
      chanspec = pinterface->chanspec;
      i5WlCfgGetChanspecFromMediaInfo((unsigned char*)media_specific_info, &pinterface->chanspec);
      /* Check if channel changed and valid */
      if (i5_config.cbs.interface_chan_change && pinterface->chanspec && (pinterface->chanspec != chanspec)) {
        i5_config.cbs.interface_chan_change(pinterface);
      }
    }
#endif /* MULTIAP */

  }

  /* Check if this device is WiFi onboarded or not. If there is any STA interfaces with
   * BSSID(Associated) in the media specific information and that STA interface is in the
   * neighbor list of this device with neighbor AL MAC address of the parent device
   */
  if (pdevice->parentDevice && media_specific_info &&
    media_specific_info[MAC_ADDR_LEN] & I5_MEDIA_INFO_ROLE_STA) {
    /* Check if the backhaul STA is associated or not by checking the BSSID field for NULL */
    if (!i5DmIsMacNull((unsigned char*)media_specific_info)) {
      /* Now find if the local interface is connected to the parent device using this STA
       * interface or not by checking in its neighbor list
       */
      if (i5Dm1905NeighborFind(pdevice, interface_id, pdevice->parentDevice->DeviceId)) {
        /* There can be a situation where it can also be connected via ethernet */
        pdevice->flags |= I5_CONFIG_FLAG_DWDS;
      }
    }
  }

  i5DmInterfaceCopyInfo(pdevice, pinterface, interface_id, media_type,
                        media_specific_info, media_specific_info_size,
                        NULL, NULL, NULL, NULL,
                        deliverFunc);

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  if (i5DmDeviceIsSelf(device_id)) {
    i5CmsMdmLocalInterfaceUpdate(pinterface);
  }
  i5CmsMdmUpdateNetworkTopologyDevIfc(pdevice, pinterface);
#endif

  pinterface->state = i5DmStateDone;

  return 0;
}

int i5DmInterfacePhyUpdate(unsigned char *device_id, unsigned char *interface_id,
                           unsigned char const *pNetTechOui, unsigned char const *pNetTechVariant, unsigned char const *pNetTechName, unsigned char const *url)
{
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pinterface;

  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(interface_id) );

  pdevice = i5DmDeviceFind(device_id);
  if ( pdevice == NULL ) {
    i5Trace("Generic Phy Info received for a non-existent device\n");
    return -1;
  }

  if ((pinterface = i5DmInterfaceFind(pdevice, interface_id)) == NULL) {
    i5Trace("Generic Phy Info received for a non-existent interface ID\n");
    return -1;
  }

  i5DmInterfaceCopyInfo(pdevice, pinterface, interface_id, I5_MEDIA_TYPE_UNKNOWN,
                        NULL, 0,
                        pNetTechOui, pNetTechVariant, pNetTechName, url,
                        NULL);

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  if (i5DmDeviceIsSelf(device_id)) {
    i5CmsMdmLocalInterfaceUpdate(pinterface);
  }
  i5CmsMdmUpdateNetworkTopologyDevIfc(pdevice, pinterface);
#endif

  pinterface->state = i5DmStateDone;

  return 0;
}


/* Given a device, local interface ID and remote device, find:
   the remote device's interface ID */
int i5DmInterfaceFindMatchingInterfaceId(i5_dm_device_type *parent, unsigned char *local_interface_id, unsigned char *other_al_mac_address, unsigned char *other_interface_id)
{
  i5_dm_interface_type *interface;
  i5_dm_device_type *other_device;
  i5_dm_interface_type *other_interface;

  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(other_al_mac_address));
  if ((interface = i5DmInterfaceFind(parent, local_interface_id)) != NULL) {
    if ((other_device = i5DmDeviceFind(other_al_mac_address)) != NULL) {
      other_interface = (i5_dm_interface_type *)other_device->interface_list.ll.next;
      while (other_interface != NULL) {
        if (interface->MediaType == other_interface->MediaType) {
          memcpy(other_interface_id, other_interface->InterfaceId, MAC_ADDR_LEN);
          return 0;
        }
        other_interface = other_interface->ll.next;
      }
    }
  }
  return -1;
}

static void i5DmLinkMetricsFreeTimer(void)
{
  if (NULL != i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer) {
    i5TimerFree(i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer);
    i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer = NULL;
  }
}

static void i5DmLinkMetricsFreeMetricsActivatedTimer(void)
{
  if (NULL != i5_dm_network_topology.linkMetricAuto.dmLinkMetricActivatedTimer) {
    i5TimerFree(i5_dm_network_topology.linkMetricAuto.dmLinkMetricActivatedTimer);
    i5_dm_network_topology.linkMetricAuto.dmLinkMetricActivatedTimer = NULL;
  }
}

void i5DmLinkMetricsActivateTimedOut(void *arg)
{
  i5Trace("\n");
  i5DmLinkMetricsFreeMetricsActivatedTimer();
  i5DmLinkMetricsFreeTimer();
}

static void i5DmValidateLinkMetricInterval (void)
{
  if (!i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalValid) {
    i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec = I5_DM_LINK_METRICS_DEFAULT_AUTO_FETCH_INTERVAL_MSEC;
    i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalValid = 1;
    i5Trace("Validated: %dms\n", i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec);
  }
}

static void i5DmLinkMetricsQueryTimeout(void *arg)
{
  i5Trace("Next Interval: %dms\n", i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec);
  i5DmLinkMetricsFreeTimer();

  if (i5MessageSendLinkQueries() > 0) {
    /* If there were actual Devices to which we can send queries, start a new timer */
    i5DmValidateLinkMetricInterval();
    if (0 != i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec) {
      i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer = i5TimerNew(i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec,
                                                                           i5DmLinkMetricsQueryTimeout, NULL);
    }
  } else {
    i5TraceInfo("No more devices to Query.  Stopping Timer.\n");
  }
}

void i5DmLinkMetricsActivate(void)
{
  i5Trace("\n");
  i5DmLinkMetricsFreeMetricsActivatedTimer();
  i5_dm_network_topology.linkMetricAuto.dmLinkMetricActivatedTimer = i5TimerNew(I5_DM_LINK_METRIC_ACTIVATED_TIME_MSEC, i5DmLinkMetricsActivateTimedOut, NULL);

  /* Only trigger a link metric query if we're not doing queries yet */
  if (NULL == i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer) {
    i5DmLinkMetricsQueryTimeout(NULL);
  }
}


/* Returns 1 if the device is still present
 *         0 if the device can be timed out
 */
int i5DmIsDeviceConnected (i5_dm_device_type *device)
{
  i5_dm_device_type *self = i5DmDeviceFind(i5_config.i5_mac_address);
  i5_dm_1905_neighbor_type *neighbor = self->neighbor1905_list.ll.next;
  i5Trace(I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(device->DeviceId));
  // check for direct neighbours
  while (neighbor) {
    // Is this a neighbour to the device in question?
    if (memcmp (neighbor->Ieee1905Id, device->DeviceId, MAC_ADDR_LEN) == 0) {
      unsigned char  netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
      unsigned short mediaType = i5GlueInterfaceGetMediaInfoFromName(neighbor->localIfname, NULL, NULL, netTechOui, NULL, NULL, NULL, 0);
      if ( i5DmIsInterfacePlc(mediaType, netTechOui)) {
        // homeplugd would have told us that the device was gone
        i5TraceInfo("Device Connected via PLC\n");
        return 1;
      }
      else if ( i5DmIsInterfaceWireless(mediaType) ) {
        if (neighbor->latestRxBytes != neighbor->prevRxBytes) {
          i5TraceInfo("Device Connected via WDS with rxbytes accumulating\n");
          return 1;
        }
      }
      // For ETH, MoCA, etc, continue
    }
    neighbor = (i5_dm_1905_neighbor_type *)(neighbor->ll.next);
  }
  return 0;
}

void i5DmDeviceWatchDogTimeout(void *arg)
{
  i5_dm_device_type *device = (i5_dm_device_type *) arg;

  if (!device) {
    return;
  }

  i5TraceError("Device Timed Out: " I5_MAC_DELIM_FMT " \"%s\" \n", I5_MAC_PRM(device->DeviceId),
               device->friendlyName);
  // Before freeing, check for traffic in link metrics
  if (i5DmIsDeviceConnected (device)) {
    i5TraceError("Ignoring Time Out and resetting timer\n");
    i5DmRefreshDeviceTimer(device->DeviceId, 0);
    return;
  }

  if (device->watchdogTimer) {
    i5TimerFree(device->watchdogTimer);
    device->watchdogTimer = NULL;
  }
  i5DmDeviceFree(device);
}

void i5DmRefreshDeviceTimer(unsigned char *alMacAddress, char createFlag)
{
  i5_dm_device_type *device = i5DmDeviceIsSelf(alMacAddress) ? NULL : i5DmDeviceFind(alMacAddress);

  if (device != NULL) {
    if (device->watchdogTimer != NULL) {
      i5TimerFree(device->watchdogTimer);
      createFlag = 1;
    }
    if (1 == createFlag) {
      device->watchdogTimer = i5TimerNew(i5_config.deviceWatchdogTimeoutMsec, i5DmDeviceWatchDogTimeout, (void *)device);
    }
  }
}

void i5DmRefreshAllDeviceTimer(void)
{
  i5_dm_device_type *device = i5_dm_network_topology.device_list.ll.next;

  i5Trace("All device timers Reset to %d ms\n", i5_config.deviceWatchdogTimeoutMsec);

  while (device != NULL) {
    i5DmRefreshDeviceTimer(device->DeviceId, 0);
    device = device->ll.next;
  }
}

void i5DmConfigureDeviceWatchdogTimer(void)
{
  unsigned int linkMetricInt = i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec;

  /* Check if Link Metric Queries are currently in use */
  if (0 != linkMetricInt) {
    i5_config.deviceWatchdogTimeoutMsec = linkMetricInt * I5_DM_DEVICE_TIMER_ROBUSTNESS + I5_DM_DEVICE_TIMER_LATENCY_MSEC;
    if (i5_config.deviceWatchdogTimeoutMsec > I5_DM_TOP_DISC_DEVICE_GONE_TIMER) {
      /* Link Queries are so far apart that it's faster to use Top Disc Timer Value */
      i5_config.deviceWatchdogTimeoutMsec = I5_DM_TOP_DISC_DEVICE_GONE_TIMER;
    }
  }
  else {
    /* There are no more link queries going out, so use the Topology Discovery Timer Value */
    i5_config.deviceWatchdogTimeoutMsec = I5_DM_TOP_DISC_DEVICE_GONE_TIMER;
  }

  i5DmRefreshAllDeviceTimer();
}

void i5DmSetLinkMetricInterval (unsigned int newIntervalMsec)
{
  int oldIntervalMsec = i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec;
  i5Trace("Called with %dms\n", newIntervalMsec);

  if (newIntervalMsec == i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec) {
    i5TraceInfo("No change.  Exiting.\n");
    return;
  }

  if (0 == newIntervalMsec) {
    /* this means turn it off */
  }
  else if (newIntervalMsec < I5_DM_LINK_METRICS_MINIMUM_AUTO_FETCH_INTERVAL_MSEC) {
    newIntervalMsec = I5_DM_LINK_METRICS_MINIMUM_AUTO_FETCH_INTERVAL_MSEC;
  }
  else if (newIntervalMsec > I5_DM_LINK_METRICS_MAXIMUM_AUTO_FETCH_INTERVAL_MSEC) {
    newIntervalMsec = I5_DM_LINK_METRICS_MAXIMUM_AUTO_FETCH_INTERVAL_MSEC;
  }

  i5Trace("Using %dms\n", newIntervalMsec);
  i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec = newIntervalMsec;
  i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalValid = 1;
  if ((oldIntervalMsec == 0) || (oldIntervalMsec > i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec)){
    if (i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer) {
      i5TimerFree(i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer);
    }
    i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer = i5TimerNew(i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec,
                                                                         i5DmLinkMetricsQueryTimeout, NULL);
  }

  i5DmConfigureDeviceWatchdogTimer();
}

void i5DmDeviceVersionTimeout(void *arg)
{
  i5_dm_device_type *destDev = arg;

  i5TraceError("Device Version Timed Out: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(destDev->DeviceId) );

  if ( (NULL != destDev) && (I5_DM_NODE_VERSION_UNKNOWN == destDev->nodeVersion) ){
    i5Trace("  Treating as Legacy Device\n");
    // Since it didn't send us a Generic PHY in time, we regard it as a Legacy Node
    destDev->nodeVersion = I5_DM_NODE_VERSION_1905;
    i5MessageTopologyNotificationSend(NULL, NULL, 0);
  }
  else {
    i5Trace("  Treating as 1905.1a Device\n");
  }

  if (destDev && destDev->nodeVersionTimer) {
    i5TimerFree(destDev->nodeVersionTimer);
    destDev->nodeVersionTimer = NULL;
  }
}

void i5DmWaitForGenericPhyQuery(i5_dm_device_type *destDev)
{
  if (destDev) {
    i5Trace("Starting Timer for Generic Phy query: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(destDev->DeviceId) );
    if (destDev->nodeVersionTimer) {
      i5TimerFree(destDev->nodeVersionTimer);
    }
    destDev->nodeVersionTimer = i5TimerNew(I5_DM_VERSION_TIMER_MSEC, i5DmDeviceVersionTimeout, destDev);
  }
}

unsigned char i5DmAreThereNodesWithVersion(int nodeVersion)
{
  i5_dm_device_type *device = i5_dm_network_topology.device_list.ll.next;

  while (device != NULL) {
    if(device->nodeVersion == nodeVersion) {
      return 1;
    }
    device = device->ll.next;
  }
  return 0;
}


i5_dm_device_type *i5DmDeviceNew(unsigned char *device_id, int version, char const* pDevFriendlyName)
{
  i5_dm_device_type *new = (i5_dm_device_type *)malloc(sizeof(i5_dm_device_type));

  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(device_id));
  if (NULL != new) {
    memset(new, 0, sizeof(i5_dm_device_type));
    i5LlItemAdd(&i5_dm_network_topology, &i5_dm_network_topology.device_list, new);
    ++i5_dm_network_topology.DevicesNumberOfEntries;
    memcpy(new->DeviceId, device_id, MAC_ADDR_LEN);
    new->Version = version;
    /* Now initialize it with default ethernet rate and fill it when we get the rate from nieghbor
     * link metrics report
     */
    new->macThroughPutCapacity = I5_DM_DEFAULT_ETH_TX_RATE;
    if ( pDevFriendlyName ) {
      strncpy(new->friendlyName, (char *)pDevFriendlyName, I5_DEVICE_FRIENDLY_NAME_LEN-1);
      new->friendlyName[I5_DEVICE_FRIENDLY_NAME_LEN-1] = '\0';
    }
    if (i5DmDeviceIsSelf(device_id)) {
      /* Never need to query self */
      new->queryState = i5DmStateDone;
      /* Store the self device pointer for easy access */
      i5_dm_network_topology.selfDevice = new;
    }
    else {
      new->queryState = i5DmStateNew;
      new->validated = 0;
      new->numTopQueryFailures = 0;
      /* Start doing Link Metrics Queries if this is the first new device */
      if (NULL == i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer) {
        i5DmValidateLinkMetricInterval();
        if (0 != i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec) {
          i5TraceInfo("Start Link Metric Query Timer\n");
          i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer = i5TimerNew(i5_dm_network_topology.linkMetricAuto.dmLinkMetricIntervalMsec,
                                                                               i5DmLinkMetricsQueryTimeout, NULL);
        }
      }

      if ( NULL == i5_dm_network_topology.pLinkMetricTimer ) {
#ifdef MULTIAP
        i5_dm_network_topology.pLinkMetricTimer = i5TimerNew(I5_DM_LINK_METRICS_GET_INTERVAL_MSEC,
          i5WlmUpdateMAPMetrics, NULL);
#else
        i5_dm_network_topology.pLinkMetricTimer = i5TimerNew(I5_DM_LINK_METRICS_GET_INTERVAL_MSEC,
          i5DmUpdateNeighborLinkMetrics, NULL);
#endif /* MULTIAP */
      }

      i5DmLegacyNeighborRemove(device_id);
    }
    i5JsonDevicePrint(I5_JSON_ALL_CLIENTS, new, i5Json_Add);
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
    i5CmsMdmUpdateNetworkTopologyDev(new);
#endif

#ifdef MULTIAP
    if (i5_config.cbs.device_init) {
      i5_config.cbs.device_init(new);
    }
#endif /* MULTIAP */

    return new;
  }

  return NULL;
}

i5_dm_device_type *i5DmDeviceFind(unsigned char const *device_id)
{
  i5_dm_device_type *item = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;

  while ((item != NULL) && (memcmp(item->DeviceId, device_id, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }
  return item;
}

void i5DmDeviceQueryStateSet(unsigned char *device_id, unsigned char queryState)
{
  i5_dm_device_type *device;

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return;
  }
  if (i5DmStateDone == queryState) {
    device->validated = 1;
    device->numTopQueryFailures = 0;
  }
  device->queryState = queryState;
}

unsigned char i5DmDeviceQueryStateGet(unsigned char const *device_id)
{
  i5_dm_device_type *device;

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return i5DmStateNotFound;
  }

  return (device->queryState);
}

void i5DmDeviceFree(i5_dm_device_type *device)
{
  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(device->DeviceId));
  while (device->interface_list.ll.next != NULL) {
    i5DmInterfaceFree(device, device->interface_list.ll.next);
  }

  while (device->legacy_list.ll.next != NULL) {
    i5DmLegacyNeighborFree(device, device->legacy_list.ll.next);
  }

  while (device->neighbor1905_list.ll.next != NULL) {
    i5Dm1905NeighborFree(device, device->neighbor1905_list.ll.next);
  }

#ifdef MULTIAP
  if (i5_config.cbs.device_deinit) {
    i5_config.cbs.device_deinit(device);
  }
#endif /* MULTIAP */

  i5Dm1905NeighborFreeAllLinksRemoteDevice(device->DeviceId);

  while (device->bridging_tuple_list.ll.next != NULL) {
    i5DmBridgingTupleFree(device, device->bridging_tuple_list.ll.next);
  }

  if (device->watchdogTimer) {
    i5TimerFree(device->watchdogTimer);
  }

  if (device->nodeVersionTimer) {
    i5TimerFree(device->nodeVersionTimer);
  }

  i5JsonDevicePrint(I5_JSON_ALL_CLIENTS, device, i5Json_Delete);
#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
  i5CmsMdmRemoveNetworkTopologyDev(device->DeviceId);
#endif

  if (device == i5_dm_network_topology.selfDevice) {
    i5_dm_network_topology.selfDevice = NULL;
  }

  i5LlItemFree(&i5_dm_network_topology.device_list, device);
  --i5_dm_network_topology.DevicesNumberOfEntries;
}

void i5DmDevicePending(void)
{
  i5_dm_device_type *device;

  i5Trace("\n");
  device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (device != NULL) {
    device->state = i5DmStatePending;
    device = device->ll.next;
  }
}

void i5DmDeviceDone(void)
{
  i5_dm_device_type *device, *next;

  i5Trace("\n");
  device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (device != NULL) {
    next = device->ll.next;
    if (device->state == i5DmStatePending) {
      i5DmDeviceFree(device);
    }
    device = next;
  }
}

void i5DmDeviceDelete(unsigned char *device_id)
{
  i5_dm_device_type *device;

  i5Trace("\n");
  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return;
  }

  i5DmDeviceFree(device);
}

void i5DmDeviceNewIfNew(unsigned char *neighbor_al_mac_address)
{
  if (i5DmDeviceFind(neighbor_al_mac_address) == NULL) {
    i5DmDeviceNew(neighbor_al_mac_address, 0, NULL);
  }
}

void i5DmDeviceTopologyQuerySendToAllNew(i5_socket_type *psock)
{
  i5_dm_device_type *device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;

  while (device != NULL) {
    if (device->queryState == i5DmStateNew) {
      i5MessageTopologyQuerySend(psock, device->DeviceId);
    }
    device = device->ll.next;
  }
}

int i5DmDeviceIsSelf(unsigned char *device_id)
{
   return (memcmp(device_id, i5_config.i5_mac_address, MAC_ADDR_LEN) == 0);
}

/* Get the self device pointer */
i5_dm_device_type *i5DmGetSelfDevice()
{
  return (i5_dm_network_topology.selfDevice ? i5_dm_network_topology.selfDevice :
    i5DmDeviceFind(i5_config.i5_mac_address));
}

int i5DmDeviceTopologyChangeProcess(unsigned char *device_id)
{
  i5_dm_device_type *device;

  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return 0;
  }

  if (i5DmDeviceIsSelf(device_id)) {
    if (device->hasChanged) {
      device->hasChanged = 0;
      i5TraceInfo("Local device has changed\n");
      return 1;
    }
  }
  return 0;
}

void i5DmTopologyFreeUnreachableDevices(void)
{
  i5_dm_device_type *device;
  i5_dm_device_type *neighbor_device;
  i5_dm_1905_neighbor_type *neighbor_1905;
  i5_ll_search_item_type search_list = {{0}};

  i5Trace("\n");

  /* Start with self and traverse all devices based on reachable neighbors */
  i5DmDevicePending();
  if ((device = i5DmDeviceFind(i5_config.i5_mac_address)) != NULL) {
    i5LlSearchItemPush(&search_list, device);
    device->state = i5DmStateDone;
  }
  else {
    i5Trace("Self not found!\n");
  }

  while ((device = (i5_dm_device_type *)i5LlSearchItemPop(&search_list)) != NULL) {
    neighbor_1905 = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;
    while (neighbor_1905 != NULL) {
      neighbor_device = i5DmDeviceFind(neighbor_1905->Ieee1905Id);
      if (neighbor_device != NULL && neighbor_device->state == i5DmStatePending) {
        i5LlSearchItemPush(&search_list, neighbor_device);
        neighbor_device->state = i5DmStateDone;
      }
      neighbor_1905 = (i5_dm_1905_neighbor_type *)neighbor_1905->ll.next;
    }
  }
  i5DmDeviceDone();
}

void i5DmDeviceFreeUnreachableNeighbors(unsigned char *device_id, int ifindex, unsigned char *neighbor_interface_list, unsigned int length)
{
  i5_dm_device_type *device;
  i5_dm_1905_neighbor_type *item;
  i5_dm_1905_neighbor_type *next;
  int i;

  i5Trace(I5_MAC_FMT " Ifindex=%d \n", I5_MAC_PRM(device_id), ifindex);
  if ((device = i5DmDeviceFind(device_id)) == NULL) {
    return;
  }

  item = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;
  while (item != NULL) {
    next = item->ll.next;
    if (item->localIfindex == ifindex) {
      if (NULL == neighbor_interface_list) {
        i5Dm1905NeighborFree(device, item);
      }
      else {
        for (i=0;i<length-5;i+=6) {
          if (memcmp(item->NeighborInterfaceId, &neighbor_interface_list[i], MAC_ADDR_LEN) == 0) {
            i5Dm1905NeighborFree(device, item);
          }
        }
      }
    }
    item = next;
  }
  i5DmTopologyFreeUnreachableDevices();
}

unsigned int i5DmInterfaceStatusGet(unsigned char *device_id, unsigned char *interface_id)
{
  i5_dm_device_type *device;
  i5_dm_interface_type *interface;

  i5Trace(I5_MAC_FMT "\n", I5_MAC_PRM(interface_id));
  if ((device = i5DmDeviceFind(device_id)) != NULL) {
    if ((interface = i5DmInterfaceFind(device, interface_id)) != NULL) {
      return interface->Status;
    }
  }

  return IF_OPER_UNKNOWN;
}

void i5DmInterfaceStatusSet(unsigned char *device_id, unsigned char * interfaceId, int ifindex, unsigned int status)
{
  i5_dm_device_type *device;
  i5_dm_interface_type *interface;

  i5Trace(I5_MAC_FMT " %d %s \n", I5_MAC_PRM(device_id), ifindex,
                                                  (status==IF_OPER_DOWN) ? "DOWN" : ((status==IF_OPER_UP) ? "UP" : "OTHER") );
  if ((device = i5DmDeviceFind(device_id)) != NULL) {
    if ((interface = i5DmInterfaceFind(device, interfaceId)) != NULL) {
      interface->Status = status;
    }
    if (IF_OPER_DOWN == status) {
      i5DmDeviceFreeUnreachableNeighbors(device_id, ifindex, NULL, 0);
    }
  }
}

int i5DmCtlSizeGet( void )
{
  i5_dm_device_type *device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
#ifdef MULTIAP
  i5_dm_interface_type *pinterface;
  i5_dm_bss_type *bss;
#endif /* MULTIAP */
  int msgSize;

  msgSize = sizeof(i5_dm_network_topology_type);
  while (device != NULL) {
    msgSize += sizeof(i5_dm_device_type);
    msgSize += sizeof(i5_dm_interface_type) * device->InterfaceNumberOfEntries;
#ifdef MULTIAP
    pinterface = device->interface_list.ll.next;
    while (pinterface != NULL) {
      msgSize += sizeof(i5_dm_bss_type) * pinterface->BSSNumberOfEntries;
      bss = pinterface->bss_list.ll.next;
      while (bss != NULL) {
        msgSize += sizeof(i5_dm_clients_type) * bss->ClientsNumberOfEntries;
        bss = bss->ll.next;
      }
      pinterface = pinterface->ll.next;
    }
#endif /* MULTIAP */
    msgSize += sizeof(i5_dm_legacy_neighbor_type) * device->LegacyNeighborNumberOfEntries;
    msgSize += sizeof(i5_dm_1905_neighbor_type) * device->Ieee1905NeighborNumberOfEntries;
    msgSize += sizeof(i5_dm_bridging_tuple_info_type) * device->BridgingTuplesNumberOfEntries;
    device = device->ll.next;
  }

  return msgSize;
}

void i5DmCtlAlMacRetrieve(char *pMsgBuf)
{
  memcpy(pMsgBuf, i5_config.i5_mac_address, MAC_ADDR_LEN);
}

void i5DmCtlRetrieve(char *pMsgBuf)
{
  i5_dm_device_type *device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;

  memcpy(pMsgBuf, &i5_dm_network_topology, sizeof(i5_dm_network_topology_type));
  pMsgBuf += sizeof(i5_dm_network_topology_type);

  device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  while (device != NULL) {
    i5_dm_interface_type *interface;
    i5_dm_1905_neighbor_type *neighbor;
    i5_dm_legacy_neighbor_type *legacy;
    i5_dm_bridging_tuple_info_type *bridging;
#ifdef MULTIAP
    i5_dm_bss_type *bss;
    i5_dm_clients_type *clients;
#endif /* MULTIAP */

    memcpy(pMsgBuf, device, sizeof(i5_dm_device_type));
    pMsgBuf += sizeof(i5_dm_device_type);

    interface = (i5_dm_interface_type *)device->interface_list.ll.next;
    while (interface != NULL) {
      memcpy(pMsgBuf, interface, sizeof(i5_dm_interface_type));
      pMsgBuf += sizeof(i5_dm_interface_type);

#ifdef MULTIAP
      /* Add all the BSS */
      bss = (i5_dm_bss_type*)interface->bss_list.ll.next;
      while (bss != NULL) {
        memcpy(pMsgBuf, bss, sizeof(i5_dm_bss_type));
        pMsgBuf += sizeof(i5_dm_bss_type);

        /* Add all the clients */
        clients = (i5_dm_clients_type*)bss->client_list.ll.next;
        while (clients != NULL) {
          memcpy(pMsgBuf, clients, sizeof(i5_dm_clients_type));
          pMsgBuf += sizeof(i5_dm_clients_type);
          clients = clients->ll.next;
        }
        bss = bss->ll.next;
      }
#endif /* MULTIAP */
      interface = interface->ll.next;
    }

    legacy = (i5_dm_legacy_neighbor_type *)device->legacy_list.ll.next;
    while (legacy != NULL) {
      memcpy(pMsgBuf, legacy, sizeof(i5_dm_legacy_neighbor_type));
      pMsgBuf += sizeof(i5_dm_legacy_neighbor_type);
      legacy = legacy->ll.next;
    }

    neighbor = (i5_dm_1905_neighbor_type *)device->neighbor1905_list.ll.next;
    while (neighbor != NULL) {
      memcpy(pMsgBuf, neighbor, sizeof(i5_dm_1905_neighbor_type));
      pMsgBuf += sizeof(i5_dm_1905_neighbor_type);
      neighbor = neighbor->ll.next;
    }

    bridging = (i5_dm_bridging_tuple_info_type *)device->bridging_tuple_list.ll.next;
    while (bridging != NULL) {
      memcpy(pMsgBuf, bridging, sizeof(i5_dm_bridging_tuple_info_type));
      pMsgBuf += sizeof(i5_dm_bridging_tuple_info_type);
      bridging = bridging->ll.next;
    }

    device = device->ll.next;
  }
}

void i5DmSetFriendlyName(const char * name) {

    i5_dm_device_type *device;
    device = i5DmDeviceFind(i5_config.i5_mac_address);
    if (device == NULL) {
        // can't find myself...
        i5TraceError("Self not found!\n");
        return;
    }
    snprintf(device->friendlyName, I5_DEVICE_FRIENDLY_NAME_LEN, "%s", name);
}

void i5DmDeinit(void)
{
  if ( i5_dm_network_topology.pLinkMetricTimer ) {
    i5TimerFree(i5_dm_network_topology.pLinkMetricTimer);
  }
  if (i5_dm_network_topology.pPerAPLinkMetricTimer) {
    i5TimerFree(i5_dm_network_topology.pPerAPLinkMetricTimer);
  }
  if ( i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer ) {
    i5TimerFree(i5_dm_network_topology.linkMetricAuto.dmLinkMetricTimer);
  }
  while ( i5_dm_network_topology.device_list.ll.next ) {
    i5DmDeviceFree(i5_dm_network_topology.device_list.ll.next);
  }
}

i5_wsc_m2_type *i5DmM2New(unsigned char *m2, unsigned int m2_len)
{
  i5_wsc_m2_type *newM2 = (i5_wsc_m2_type *)malloc(sizeof(i5_wsc_m2_type));

  if (NULL != newM2) {
    memset(newM2, 0, sizeof(*newM2));
    i5LlItemAppend(&i5_config, &i5_config.m2_list, newM2);
    newM2->m2 = m2;
    newM2->m2_len = m2_len;
    i5_config.m2_count++;
  }
  return newM2;
}

static void i5DmM2Free(i5_wsc_m2_type *m2_item)
{
  free(m2_item->m2);
  i5LlItemFree(&i5_config.m2_list, m2_item);
}

void i5DmM2ListFree()
{
  while (i5_config.m2_list.ll.next != NULL) {
    i5DmM2Free(i5_config.m2_list.ll.next);
  }
  i5_config.m2_count = 0;
}

int i5DmInit(unsigned int supServiceFlag, int isRegistrar)
{
  /* create local device */
  i5_dm_device_type *pdevice = i5DmDeviceNew(i5_config.i5_mac_address, I5_MESSAGE_VERSION, i5_config.friendlyName);

  if (NULL == pdevice) {
    return -1;
  }

#ifdef MULTIAP
  /* Supported Service Flag */
  if (I5_IS_MULTIAP_CONTROLLER(supServiceFlag)) {
    pdevice->flags |= I5_CONFIG_FLAG_CONTROLLER;
  }
  if (I5_IS_MULTIAP_AGENT(supServiceFlag)) {
    pdevice->flags |= I5_CONFIG_FLAG_AGENT;
  }
#endif /* MULTIAP */
  if (isRegistrar) {
    pdevice->flags |= I5_CONFIG_FLAG_REGISTRAR;
  }
  i5DmValidateLinkMetricInterval();
  i5DmConfigureDeviceWatchdogTimer();

  return 0;
}

#ifdef MULTIAP
/* Count the BSS which has clients associated */
unsigned char i5DmCountBSSWithAssociatedClients(i5_dm_device_type *pdmdev)
{
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss;
  unsigned char bss_count = 0;

  /* For all the interfaces in this devices */
  pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
  while (pdmif != NULL) {
    /* For all the operational BSS */
    pdmbss = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
    while (pdmbss != NULL) {
      if (pdmbss->ClientsNumberOfEntries > 0) {
        bss_count++;
      }
      pdmbss = pdmbss->ll.next;
    }
    pdmif = pdmif->ll.next;
  }

  return bss_count;
}

/* Find the BSS in a Interface */
i5_dm_bss_type *i5DmFindBSSFromInterface(i5_dm_interface_type *pdmif, unsigned char *bssid)
{
  i5_dm_bss_type *item = (i5_dm_bss_type *)pdmif->bss_list.ll.next;

  while ((item != NULL) && (memcmp(item->BSSID, bssid, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }

  return item;
}

/* Find the BSS from all the device in the network */
i5_dm_bss_type *i5DmFindBSSFromNetwork(unsigned char *bssid)
{
  i5_dm_device_type *device = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
  i5_dm_bss_type *pdmbss = NULL;

  while (device != NULL) {
    if ((pdmbss = i5DmFindBSSFromDevice(device, bssid)) != NULL) {
      break;
    }
    device = device->ll.next;
  }

  return pdmbss;
}

/* Find the BSS in a device */
i5_dm_bss_type *i5DmFindBSSFromDevice(i5_dm_device_type *pdmdev, unsigned char *bssid)
{
  i5_dm_interface_type *pdmif;
  i5_dm_bss_type *pdmbss = NULL;

  /* For all the interfaces in this devices */
  pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
  while (pdmif != NULL) {
    if ((pdmbss = i5DmFindBSSFromInterface(pdmif, bssid)) != NULL) {
      break;
    }
    pdmif = pdmif->ll.next;
  }

  return pdmbss;
}

/* Find the Client in a BSS */
i5_dm_clients_type *i5DmFindClientInBSS(i5_dm_bss_type *pdmbss, unsigned char *mac)
{
  i5_dm_clients_type *item = (i5_dm_clients_type *)pdmbss->client_list.ll.next;

  while ((item != NULL) && (memcmp(item->mac, mac, MAC_ADDR_LEN))) {
    item = item->ll.next;
  }

  return item;
}

/* Find the Client in a Device */
i5_dm_clients_type *i5DmFindClientInDevice(i5_dm_device_type *pdmdev, unsigned char *mac)
{
  i5_dm_interface_type *pdmif;
  i5_dm_clients_type *pdmclient = NULL;

  /* For all the interfaces in this devices */
  pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
  while (pdmif != NULL) {
    if ((pdmclient = i5DmFindClientInInterface(pdmif, mac)) != NULL) {
      break;
    }
    pdmif = pdmif->ll.next;
  }

  return pdmclient;
}

/* Find the Client in a Interface */
i5_dm_clients_type *i5DmFindClientInInterface(i5_dm_interface_type *pdmif, unsigned char *mac)
{
  i5_dm_bss_type *item = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
  i5_dm_clients_type *pdmclient = NULL;

  while (item != NULL) {
    if ((pdmclient = i5DmFindClientInBSS(item, mac)) != NULL) {
      break;
    }
    item = item->ll.next;
  }

  return pdmclient;
}

/* Create New BSS */
i5_dm_bss_type *i5DmBSSNew(i5_dm_interface_type *parent, unsigned char *bssid, unsigned char *ssid,
  unsigned char ssid_len)
{
  i5_dm_bss_type *newBSS = (i5_dm_bss_type *)malloc(sizeof(i5_dm_bss_type));

  i5Trace("New BSS " I5_MAC_FMT "\n", I5_MAC_PRM(bssid));
  if (NULL != newBSS) {
    memset(newBSS, 0, sizeof(*newBSS));
    i5LlItemAdd(parent, &parent->bss_list, newBSS);
    ++parent->BSSNumberOfEntries;
    memcpy(newBSS->BSSID, bssid, MAC_ADDR_LEN);
    I5STRNCPY((char*)newBSS->ssid.SSID, (char*)ssid, sizeof(newBSS->ssid.SSID));
    newBSS->ssid.SSID_len = ssid_len;

#ifdef MULTIAP
    if (i5_config.cbs.bss_init) {
      i5_config.cbs.bss_init(newBSS);
    }
#endif /* MULTIAP */
  }
  return newBSS;
}

/* Remove BBS from the interface */
int i5DmBSSFree(i5_dm_interface_type *parent, i5_dm_bss_type *bss)
{
  /* First Free all the Clients */
  while (bss->client_list.ll.next != NULL) {
    i5DmClientFree(bss, bss->client_list.ll.next);
  }

#ifdef MULTIAP
  if (i5_config.cbs.bss_deinit) {
    i5_config.cbs.bss_deinit(bss);
  }
#endif /* MULTIAP */

  if (i5LlItemFree(&parent->bss_list, bss) == 0) {
    i5_dm_device_type *pdevice;

    --parent->BSSNumberOfEntries;

    /* Modify the hasChanged in self device */
    pdevice = I5LL_PARENT(parent);
    if ( i5DmDeviceIsSelf(pdevice->DeviceId) ) {
      pdevice->hasChanged++;
    }
    return 0;
  }

  return -1;
}

/* Create New Client */
i5_dm_clients_type *i5DmClientNew(i5_dm_bss_type *parent, unsigned char *mac, struct timeval assoc_tm)
{
  i5_dm_clients_type *newClient = (i5_dm_clients_type *)malloc(sizeof(i5_dm_clients_type));

  i5Trace("New STA " I5_MAC_FMT "\n", I5_MAC_PRM(mac));
  if (NULL != newClient) {
    memset(newClient, 0, sizeof(*newClient));
    i5LlItemAdd(parent, &parent->client_list, newClient);
    ++parent->ClientsNumberOfEntries;
    memcpy(newClient->mac, mac, MAC_ADDR_LEN);
    newClient->assoc_tm = assoc_tm;

#ifdef MULTIAP
    if (i5_config.cbs.client_init) {
      i5_config.cbs.client_init(newClient);
    }
#endif /* MULTIAP */
  }
  return newClient;
}

/* Remove STA from the BSS */
int i5DmClientFree(i5_dm_bss_type *parent, i5_dm_clients_type *client)
{
  if (client->assoc_frame) {
    free(client->assoc_frame);
    client->assoc_frame = NULL;
  }

#ifdef MULTIAP
  if (i5_config.cbs.client_deinit) {
    i5_config.cbs.client_deinit(client);
  }
#endif /* MULTIAP */

  if (i5LlItemFree(&parent->client_list, client) == 0) {
    i5_dm_interface_type *pinterface;
    i5_dm_device_type *pdevice;

    --parent->ClientsNumberOfEntries;

    /* Modify the hasChanged in self device */
    pinterface = I5LL_PARENT(parent);
    pdevice = I5LL_PARENT(pinterface);
    if ( i5DmDeviceIsSelf(pdevice->DeviceId) ) {
      pdevice->hasChanged++;
    }
    return 0;
  }

  return -1;
}

/* Add the BSS and update the SSID */
int i5DmBSSUpdate(unsigned char *device_id, unsigned char *interface_id, unsigned char *bssid,
  ieee1905_ssid_type *ssid)
{
  i5_dm_device_type *pdevice;
  i5_dm_interface_type *pinterface;
  i5_dm_bss_type *pdmbss;

  i5Trace("Device : " I5_MAC_DELIM_FMT " Interface : " I5_MAC_DELIM_FMT " BSSID " I5_MAC_DELIM_FMT "\n",
    I5_MAC_PRM(device_id), I5_MAC_PRM(interface_id), I5_MAC_PRM(bssid));

  pdevice = i5DmDeviceFind(device_id);
  if ( pdevice == NULL ) {
    i5TraceError("Device " I5_MAC_DELIM_FMT " not found\n", I5_MAC_PRM(device_id));
    return -1;
  }

  /* First check if the interface exist. If not, it could be because the RUID
   * and interface mac are different.
   * According to spec, device information TLV should have all the operational
   * BSS as interfaces. So find an interface with bssid and create another interface
   * by copying the information from it and using RUID as interface mac
   */
  if ((pinterface = i5DmInterfaceFind(pdevice, interface_id)) == NULL) {
    if ((pinterface = i5DmInterfaceFind(pdevice, bssid)) == NULL) {
      i5TraceError("In Device " I5_MAC_DELIM_FMT " BSS "I5_MAC_DELIM_FMT" not found in interface list\n",
        I5_MAC_PRM(device_id), I5_MAC_PRM(bssid));
      return -1;
    } else {
      i5DmInterfaceUpdate(device_id, interface_id, I5_MESSAGE_VERSION, pinterface->MediaType,
        pinterface->MediaSpecificInfo, pinterface->MediaSpecificInfoSize, NULL, NULL, 0);
      if ((pinterface = i5DmInterfaceFind(pdevice, interface_id)) == NULL) {
        i5TraceError("In Device " I5_MAC_DELIM_FMT " Interface "I5_MAC_DELIM_FMT" not found\n",
          I5_MAC_PRM(device_id), I5_MAC_PRM(interface_id));
        return -1;
      }
    }
  }

  /* If BSS already exists, dont create */
  if ((pdmbss = i5DmFindBSSFromInterface(pinterface, bssid)) != NULL) {
    pdmbss->flags |= I5_BSS_FLAG_QUERY_STATE; /* Set the state to done */
    memcpy(&pdmbss->ssid, ssid, sizeof(pdmbss->ssid));
    return 0;
  }

  /* Create New BSS */
  if ((pdmbss = i5DmBSSNew(pinterface, bssid, ssid->SSID, ssid->SSID_len)) == NULL) {
    i5TraceError("In Device " I5_MAC_DELIM_FMT " Interface "I5_MAC_DELIM_FMT" Failed to add BSS "
      I5_MAC_DELIM_FMT "\n",
      I5_MAC_PRM(device_id), I5_MAC_PRM(interface_id), I5_MAC_PRM(bssid));
    return -1;
  }
  pdmbss->flags |= I5_BSS_FLAG_QUERY_STATE; /* Set the state to done */
  pdevice->hasChanged++;

  return 0;
}

/* Add the Client and update the time */
int i5DmClientUpdate(i5_message_type *pmsg, i5_dm_device_type *pdevice, unsigned char *bssid,
  unsigned char *mac, unsigned short time_elapsed)
{
  int rc = 0;

  i5Trace(" BSSID " I5_MAC_FMT " MAC " I5_MAC_FMT "\n", I5_MAC_PRM(bssid), I5_MAC_PRM(mac));

  rc = i5DmAssociateClient(pmsg, pdevice, bssid, mac, time_elapsed, 0, NULL, 0);
  if (rc == 0) {
    pdevice->hasChanged++;
  }

  if (rc == IEEE1905_STA_ALREADY_EXISTS) {
    rc = 0;
  }

  return rc;
}

/* Update the client capability */
void i5DmUpdateClientCapability(i5_dm_clients_type *pdmclient, unsigned char *assoc_frame,
  unsigned int assoc_frame_len)
{
  if (assoc_frame && (assoc_frame_len > 0)) {
    if (pdmclient->assoc_frame != NULL) {
      free(pdmclient->assoc_frame);
      pdmclient->assoc_frame = NULL;
      pdmclient->assoc_frame_len= 0;
    }

    if ((pdmclient->assoc_frame = (unsigned char *)malloc(assoc_frame_len)) == NULL) {
      printf("malloc error\n");
      return;
    }
    memcpy(pdmclient->assoc_frame, assoc_frame, assoc_frame_len);
    pdmclient->assoc_frame_len = assoc_frame_len;
  }
}

/* Associate a client to device */
int i5DmAssociateClient(i5_message_type *pmsg, i5_dm_device_type *pdmdev, unsigned char *bssid,
  unsigned char *mac, unsigned short time_elapsed, unsigned char notify, unsigned char *assoc_frame,
  unsigned int assoc_frame_len)
{
  i5_dm_bss_type *pdmbss;
  i5_dm_clients_type *pdmclient;
  struct timeval assoc_tm;

  /* Find the BSS in the device */
  pdmbss = i5DmFindBSSFromDevice(pdmdev, bssid);
  if (pdmbss == NULL) {
    i5TraceError("BSS " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(bssid));
    return IEEE1905_BSS_NOT_FOUND;
  }

  /* If the Client is there in the BSS dont add */
  if ((pdmclient = i5DmFindClientInBSS(pdmbss, mac)) != NULL) {
    pdmclient->flags |= I5_CLIENT_FLAG_QUERY_STATE; /* Set the state to done */
    i5DmUpdateClientCapability(pdmclient, assoc_frame, assoc_frame_len);
    return IEEE1905_STA_ALREADY_EXISTS;
  }

  /* To get the assoc time get the current time and deduct the time elapsed */
  gettimeofday(&assoc_tm, NULL);
  assoc_tm.tv_sec -= time_elapsed;

  if ((pdmclient = i5DmClientNew(pdmbss, mac, assoc_tm)) == NULL) {
    i5TraceError("Failed to Add STA " I5_MAC_FMT "In BSS " I5_MAC_FMT "\n", I5_MAC_PRM(mac), I5_MAC_PRM(bssid));
    return IEEE1905_FAIL;
  }

  pdmclient->flags |= I5_CLIENT_FLAG_QUERY_STATE; /* Set the state to done */

  /* For the current device */
  i5DmUpdateClientCapability(pdmclient, assoc_frame, assoc_frame_len);

  i5TraceInfo("BSSID "I5_MAC_DELIM_FMT" MAC "I5_MAC_DELIM_FMT" Successfully added",
        I5_MAC_PRM(bssid), I5_MAC_PRM(mac));
  /* Send topology notification if the client newly joined */
  if (i5DmDeviceIsSelf(pdmdev->DeviceId)) {
    if (notify && I5_IS_MULTIAP_AGENT(i5_config.flags)) {
      i5MessageTopologyNotificationSend(bssid, mac, 1);
    }
  } else {
    /* pmsg can be NULL for the current devices associated clients */
    if (pmsg) {
      i5MessageClientCapabilityQuerySend(pmsg->psock, pdmdev->DeviceId, mac, bssid);
    }
  }

  return 0;
}

/* Disassociate a client from device */
int i5DmDisAssociateClient(i5_dm_device_type *pdmdev, unsigned char *bssid, unsigned char *mac, unsigned char notify)
{
  i5_dm_bss_type *pdmbss;
  i5_dm_clients_type *pdmclient;

  /* Find the BSS in the device */
  pdmbss = i5DmFindBSSFromDevice(pdmdev, bssid);
  if (pdmbss == NULL) {
    i5TraceError("BSS " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(bssid));
    return IEEE1905_BSS_NOT_FOUND;
  }

  /* If the client is not there return error */
  if ((pdmclient = i5DmFindClientInBSS(pdmbss, mac)) == NULL) {
    i5TraceError("STA " I5_MAC_FMT "In BSS " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(mac), I5_MAC_PRM(bssid));
    return IEEE1905_STA_NOT_FOUND;
  }

  /* Delete the STA */
  if (i5DmClientFree(pdmbss, pdmclient) != 0) {
    return IEEE1905_FAIL;
  }

  i5TraceInfo("BSSID "I5_MAC_DELIM_FMT" MAC "I5_MAC_DELIM_FMT" Successfully Removed",
        I5_MAC_PRM(bssid), I5_MAC_PRM(mac));

  /* Send topology notification if the client is left BSS */
  if (notify && i5DmDeviceIsSelf(pdmdev->DeviceId) &&
    I5_IS_MULTIAP_AGENT(i5_config.flags)) {
    i5MessageTopologyNotificationSend(bssid, mac, 0);
  }

  return 0;
}

/* Delete all the node from generic list */
void i5DmGlistCleanup(ieee1905_glist_t *list)
{
  dll_t *item_p, *next_p;

  /* Travese List */
  for (item_p = dll_head_p(&list->head);
    !dll_end(&list->head, item_p);
    item_p = next_p) {

    /* need to keep next item incase we remove node in between */
    next_p = dll_next_p(item_p);

    /* Remove item itself from list */
    ieee1905_glist_delete(list, item_p);
    free(item_p);
  }
}

/* Free the memory allocated for STAs and BSSs in steer request structure */
void i5DmSteerRequestInfoFree(ieee1905_steer_req *steer_req)
{
  i5DmGlistCleanup(&steer_req->sta_list);
  i5DmGlistCleanup(&steer_req->bss_list);
}

/* Closes the steering opportunity timer */
void i5DmDeviceSteerOpportunityTimeout(void *arg)
{
  i5_dm_device_type *destDev = arg;

  if (!destDev || !destDev->steerOpportunityTimer) {
    return;
  }

  i5TraceError("Steering Opportunity Timed Out: " I5_MAC_DELIM_FMT "\n",
    I5_MAC_PRM(destDev->DeviceId));

  i5TimerFree(destDev->steerOpportunityTimer);
  destDev->steerOpportunityTimer = NULL;
}

/* Creates Steering Opportunity Timer for the particular device */
void i5DmSteerOpportunityTimer(i5_dm_device_type *destDev, unsigned short opportunity_window)
{
  if (!destDev) {
    return;
  }

  i5Trace("Starting Timer for Steer Opportunity: " I5_MAC_DELIM_FMT "\n",
    I5_MAC_PRM(destDev->DeviceId) );
  if (destDev->steerOpportunityTimer) {
    i5TimerFree(destDev->steerOpportunityTimer);
    destDev->steerOpportunityTimer = NULL;
  }

  if (opportunity_window > 0) {
    destDev->steerOpportunityTimer = i5TimerNew(I5_SEC_MSEC(opportunity_window),
      i5DmDeviceSteerOpportunityTimeout, destDev);
  }
}

/* Check if is there any steer opportunity for the device */
int i5DmIsSteerOpportunity(i5_dm_device_type *destDev)
{
  /* If the steerOpportunityTimer is NULL, steer opportunity is there */
  if ((destDev) && (destDev->steerOpportunityTimer == NULL)) {
    return 1;
  }

  return 0;
}

/* Free the memory allocated for block unblock request structure */
void i5DmBlockUnblockInfoFree(ieee1905_block_unblock_sta *block_unblock_sta)
{
  i5DmGlistCleanup(&block_unblock_sta->sta_list);
}

/* Find controller in the network */
i5_dm_device_type *i5DmFindController()
{
  i5_dm_device_type *item = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;

  while ((item != NULL) && (!I5_IS_MULTIAP_CONTROLLER(item->flags))) {
    item = item->ll.next;
  }
  return item;
}

/* Free the memory allocated for config list */
void i5DmConfigListFree(ieee1905_policy_config *config_list)
{
  i5DmGlistCleanup(&config_list->no_steer_sta_list);
  i5DmGlistCleanup(&config_list->no_btm_steer_sta_list);
  i5DmGlistCleanup(&config_list->steercfg_bss_list);
  i5DmGlistCleanup(&config_list->metricrpt_config.ifr_list);
}

/* Find if the MAC is in ieee1905_sta_list list */
int i5DmIsMACInList(ieee1905_glist_t *list, unsigned char *mac)
{
  dll_t *item_p, *next_p;
  ieee1905_sta_list *sta;

  /* For each item in the list */
  for (item_p = dll_head_p(&list->head);
    !dll_end(&list->head, item_p);
    item_p = next_p) {
    next_p = dll_next_p(item_p);
    sta = (ieee1905_sta_list*)item_p;
    if (memcmp(sta->mac, mac, sizeof(sta->mac)) == 0) {
      return 1;
    }
  }

  return 0;
}

/* Get the Channel to Regulatory class mapping array */
i5_dm_chan_rc_map_type *i5DmGetChannelRCMap(unsigned int *rc_map_count)
{
  *rc_map_count = sizeof(chan_rc_map) / sizeof(chan_rc_map[0]);

  return chan_rc_map;
}

/* Get the Regulatory class to Channel mapping array */
i5_dm_rc_chan_map_type *i5DmGetRCChannelMap(unsigned int *reg_class_count)
{
  *reg_class_count = sizeof(rc_chan_map) / sizeof(rc_chan_map[0]);

  return rc_chan_map;
}

/* Check whether the channel is valid in the regulatory class */
int i5DmIsChannelValidInRC(unsigned char channel, unsigned char rc)
{
  int idx, j;
  unsigned int rc_map_count = sizeof(chan_rc_map) / sizeof(chan_rc_map[0]);

  /* Go through chan_rc_map to get the channel entry */
  for (idx = 0; idx < rc_map_count; idx++) {
    if (chan_rc_map[idx].channel == channel) {
      /* Now compare the regulatory class */
      for (j = 0; j < chan_rc_map[idx].count; j++) {
        if (chan_rc_map[idx].regclass[j] == rc) {
          return 1;
        }
      }
    }
  }

  return 0;
}

/* Copy AP Capability */
int i5DmCopyAPCaps(ieee1905_ap_caps_type *ToApCaps, ieee1905_ap_caps_type *FromApCaps)
{
  /* Update Radio Capabilities */
  if (FromApCaps != NULL && ToApCaps != NULL) {
    ToApCaps->HTCaps = FromApCaps->HTCaps;
    memcpy(&ToApCaps->VHTCaps, &FromApCaps->VHTCaps, sizeof(ToApCaps->VHTCaps));

    if (ToApCaps->RadioCaps.List) {
      free(ToApCaps->RadioCaps.List);
      ToApCaps->RadioCaps.ListSize = 0;
    }
    ToApCaps->RadioCaps.maxBSSSupported = FromApCaps->RadioCaps.maxBSSSupported;
    ToApCaps->RadioCaps.List = (unsigned char *)malloc(I5_RADIO_CAP_SIZE);
    if (NULL == ToApCaps->RadioCaps.List) {
      i5TraceError("Malloc failed for Radio Caps List\n");
      return -1;
    } else {
      ToApCaps->RadioCaps.ListSize = I5_RADIO_CAP_SIZE;
      ToApCaps->RadioCaps.Valid = FromApCaps->RadioCaps.Valid;
      ToApCaps->RadioCaps.Len = (FromApCaps->RadioCaps.Len > I5_RADIO_CAP_SIZE) ?
        I5_RADIO_CAP_SIZE : FromApCaps->RadioCaps.Len;
      memset(ToApCaps->RadioCaps.List, 0, I5_RADIO_CAP_SIZE);
      if (FromApCaps->RadioCaps.Valid) {
        memcpy(ToApCaps->RadioCaps.List, FromApCaps->RadioCaps.List,
          ToApCaps->RadioCaps.Len);
      }
    }
  }

  return 0;
}

/* Check if all the wireless interfaces configured */
int i5DmIsAllInterfacesConfigured()
{
  i5_dm_device_type *pdevice = i5DmDeviceFind(i5_config.i5_mac_address);
  i5_dm_interface_type *pinterface = NULL;

  if (pdevice == NULL) {
    return 0;
  }

  pinterface = pdevice->interface_list.ll.next;

  while (pinterface) {
    if (i5DmIsInterfaceWireless(pinterface->MediaType) && !pinterface->isConfigured) {
      return 0;
    }
    pinterface = pinterface->ll.next;
  }

  return 1;
}

/* Check if M1 is sent for all the interfaces */
int i5DmIsM1SentToAllWirelessInterfaces()
{
  i5_dm_device_type *pdevice = i5DmDeviceFind(i5_config.i5_mac_address);
  i5_dm_interface_type *pinterface = NULL;

  if (pdevice == NULL) {
    return 0;
  }

  pinterface = pdevice->interface_list.ll.next;

  while (pinterface) {
    if (i5DmIsInterfaceWireless(pinterface->MediaType) && !pinterface->isM1Sent) {
      return 0;
    }
    pinterface = pinterface->ll.next;
  }

  return 1;
}
/* Pre configure all virtual radios */
void i5DmPreConfigureVirtualInterfaces()
{
  i5_dm_device_type *pdevice = i5DmDeviceFind(i5_config.i5_mac_address);
  i5_dm_interface_type *pdmif = NULL;

  if (pdevice == NULL) {
    return;
  }

  pdmif = pdevice->interface_list.ll.next;
  while (pdmif) {
    if (i5DmIsInterfaceWireless(pdmif->MediaType) && i5WlCfgIsVirtualInterface(pdmif->ifname)) {
      pdmif->isConfigured = 1;
    }
    pdmif = pdmif->ll.next;
  }
}

/* Process Per AP Link metrics timer callback */
static void i5DmPerAPlinkMetricsTimerProcess(void *arg)
{
  i5_dm_device_type *pDeviceController = i5DmFindController();

  i5Trace("Per AP Link Metrics Timeout\n");

  if (pDeviceController == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Controller not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  /* Send the AP Metrics response packet to the controller */
  i5_config.last_message_identifier++;
  i5MessageAPMetricsResponseSend(pDeviceController->psock, pDeviceController->DeviceId,
    i5_config.last_message_identifier, NULL, 0, NULL, 1);
}

/* Process AP Metric Reporting Policy */
void i5DmProcessAPMetricReportingPolicy()
{
  unsigned int interval_msec;
  /* If the AP Metrics Reporting Interval field is 0 then dont send AP metric anymore */
  if (i5_config.policyConfig.metricrpt_config.ap_rpt_intvl == 0) {
    if (i5_dm_network_topology.pPerAPLinkMetricTimer) {
      i5TimerFree(i5_dm_network_topology.pPerAPLinkMetricTimer);
      i5_dm_network_topology.pPerAPLinkMetricTimer = NULL;
    }
  } else {
    /* send an AP Metrics Response message once every reporting interval specified */
    if (i5_dm_network_topology.pPerAPLinkMetricTimer) {
      i5TimerFree(i5_dm_network_topology.pPerAPLinkMetricTimer);
      i5_dm_network_topology.pPerAPLinkMetricTimer = NULL;
    }
    interval_msec = I5_SEC_MSEC(i5_config.policyConfig.metricrpt_config.ap_rpt_intvl);
    i5_dm_network_topology.pPerAPLinkMetricTimer = i5TimerNew(interval_msec,
      i5DmPerAPlinkMetricsTimerProcess, NULL);
  }
}

/* Find the interface metric policy */
ieee1905_ifr_metricrpt *i5DmFindMetricReportPolicy(unsigned char *ifrMAC)
{
  dll_t *item_p, *next_p;
  ieee1905_ifr_metricrpt *metricrpt = NULL;

  /* Find the metric report policy for the interface */
  for (item_p = dll_head_p(&i5_config.policyConfig.metricrpt_config.ifr_list.head);
    !dll_end(&i5_config.policyConfig.metricrpt_config.ifr_list.head, item_p);
    item_p = next_p) {
    next_p = dll_next_p(item_p);
    metricrpt = (ieee1905_ifr_metricrpt*)item_p;

    if (memcmp(metricrpt->mac, ifrMAC, MAC_ADDR_LEN) == 0) {
      return metricrpt;
    }
  }

  return  NULL;
}

/* From the neigbor device type fill local neighbor if not exists */
int i5DmFillSelfNeighborFromTopologyResp(unsigned char *local_ifr, unsigned char *neighbor_al_mac,
  unsigned char *neighbor_ifr, char *localIfname, int localIfindex)
{
  i5_dm_device_type *device;
  i5_dm_1905_neighbor_type *neighbor;

  if ((device = i5DmDeviceFind(i5_config.i5_mac_address)) == NULL) {
    return -1;
  }

  /* Find if the neighbor entry exists for our local interface MAC and neighbor AL */
  neighbor = i5Dm1905NeighborFind(device, local_ifr, neighbor_al_mac);
  if (neighbor != NULL) {
    return 0;
  }

  i5Trace("Creating New Neighbor\n");
  if ((neighbor = i5Dm1905NeighborNew(device, local_ifr, neighbor_al_mac)) == NULL) {
    i5TraceError("Failed to create new neighbor AL["I5_MAC_DELIM_FMT"] Local["I5_MAC_DELIM_FMT"] "
      "neighbor ifr["I5_MAC_DELIM_FMT"]\n",
      I5_MAC_PRM(neighbor_al_mac), I5_MAC_PRM(local_ifr), I5_MAC_PRM(neighbor_ifr));
    return -1;
  }
  i5DmLegacyNeighborRemove(neighbor_ifr);
  I5STRNCPY(neighbor->localIfname, localIfname, sizeof(neighbor->localIfname));
  memcpy(neighbor->NeighborInterfaceId, neighbor_ifr, sizeof(neighbor->NeighborInterfaceId));
  neighbor->localIfindex = localIfindex;

  return 0;
}

/* Helper function to derive Bandwidth from Global operating class */
uint i5DmGetBandWidthFromOpClass(unsigned char opClass)
{
  int bw = 0;

  switch (opClass) {
    case 81:
    case 82:
    case 115:
    case 118:
    case 121:
    case 124:
    case 125:
      bw = WL_CHANSPEC_BW_20;
      break;
    case  83:
    case  84:
    case 116:
    case 117:
    case 119:
    case 120:
    case 122:
    case 123:
    case 126:
    case 127:
      bw = WL_CHANSPEC_BW_40;
      break;
    case 128:
      bw = WL_CHANSPEC_BW_80;
      break;
    case 129:
      bw = WL_CHANSPEC_BW_160;
      break;
    case 130:
      bw = WL_CHANSPEC_BW_8080;
      break;
  }
  return bw;
}

/* Unset the state of all BSS and clients */
void i5DmBSSClientPending(i5_dm_device_type *device)
{
  i5_dm_interface_type *pinterface;
  i5_dm_bss_type *pbss;
  i5_dm_clients_type *pclient;

  for (pinterface = device->interface_list.ll.next; pinterface; pinterface = pinterface->ll.next) {

    for (pbss = pinterface->bss_list.ll.next; pbss; pbss = pbss->ll.next) {

      /* Unset the state */
      pbss->flags &= ~I5_BSS_FLAG_QUERY_STATE;
      for (pclient = pbss->client_list.ll.next; pclient; pclient = pclient->ll.next) {

        /* Unset the state */
        pclient->flags &= ~I5_CLIENT_FLAG_QUERY_STATE;
      }
    }
  }
}

/* Remove all the BSS and clients with state not set */
void i5DmBSSClientDone(i5_dm_device_type *device)
{
  i5_dm_interface_type *pinterface;
  i5_dm_bss_type *pbss, *pnextbss;
  i5_dm_clients_type *pclient, *pnextclient;

  for (pinterface = device->interface_list.ll.next; pinterface; pinterface = pinterface->ll.next) {

    for (pbss = pinterface->bss_list.ll.next; pbss; pbss = pnextbss) {

      pnextbss = pbss->ll.next;
      /* If the state is not set, remove the BSS */
      if (!(pbss->flags & I5_BSS_FLAG_QUERY_STATE)) {
        i5TraceInfo("Remove BSS "I5_MAC_DELIM_FMT" which is not in topology response\n",
            I5_MAC_PRM(pbss->BSSID));
        i5DmBSSFree(pinterface, pbss);
        continue;
      }

      for (pclient = pbss->client_list.ll.next; pclient; pclient = pnextclient) {

        pnextclient = pclient->ll.next;
        /* If the state is not set, remove the client */
        if (!(pclient->flags & I5_CLIENT_FLAG_QUERY_STATE)) {
          i5TraceInfo("Remove Client "I5_MAC_DELIM_FMT" From BSS "I5_MAC_DELIM_FMT" which is "
            "not in topology response\n",
            I5_MAC_PRM(pclient->mac), I5_MAC_PRM(pbss->BSSID));
          i5DmClientFree(pbss, pclient);
        }
      }
    }
  }
}
#endif /* MULTIAP */

/* Is this neighbor device running in the self device */
int i5DmIsNeighborDeviceOnSameDevice(i5_dm_device_type *neighbor_device)
{
  i5_dm_device_type *localDevice;
  i5_dm_1905_neighbor_type *neigh;

  if ((localDevice = i5DmDeviceFind(i5_config.i5_mac_address)) == NULL) {
    return 0;
  }

  /* Loop local devices neighbor list */
  for (neigh = localDevice->neighbor1905_list.ll.next; neigh; neigh = neigh->ll.next) {
    /* If the nighbor is not our neighbor skip it */
    if (memcmp(neigh->Ieee1905Id, neighbor_device->DeviceId, MAC_ADDR_LEN) != 0) {
      continue;
    }

    /* If the neighbor device present and if the local devices interface MAC is NULL(loopback), then
     * its an other 1905 stack running on the same device
     */
    if (i5DmIsMacNull(neigh->LocalInterfaceId)) {
      return 1;
    }
  }

  return 0;
}

/* Update AP Caps */
void i5DmUpdateAPCaps(char *ifname, i5_dm_interface_type *pdmif)
{
  ieee1905_ifr_info info;

  memset(&info, 0, sizeof(info));

  if (i5_config.cbs.get_interface_info) {
    if (i5_config.cbs.get_interface_info(ifname, &info) != 0) {
      i5TraceError("Failed to get interface info[%s]\n",
        ifname);
      return;
    }

    i5DmCopyAPCaps(&pdmif->ApCaps, &info.ap_caps);
    i5DmFreeRadioCaps(&info.ap_caps.RadioCaps);
  }
}

/* Free Radio Caps */
void i5DmFreeRadioCaps(ieee1905_radio_caps_type *RadioCaps)
{
  if (RadioCaps && RadioCaps->List) {
    free(RadioCaps->List);
    RadioCaps->List = NULL;
    RadioCaps->ListSize = 0;
  }
}

/* Update the MAP flags from controller BSS Table */
void i5DmUpdateMAPFlagsFromControllerBSSTable(i5_dm_device_type *pdevice,
  i5_dm_interface_type *pdmif)
{
  i5_dm_device_type *pDeviceCtrl = i5DmDeviceFind(i5_config.i5_mac_address);
  unsigned char tmpALMac[MAC_ADDR_LEN];
  unsigned char isDedicated = 1;
  dll_t *item_p, *next_p;
  ieee1905_client_bssinfo_type *list;
  i5_dm_bss_type *pbss;

  /* Validate Arguments */
  if (!pdevice || !pDeviceCtrl || !pdmif) {
    return;
  }

  /* If it is not controller exit */
  if (!I5_IS_MULTIAP_CONTROLLER(pDeviceCtrl->flags)) {
    return;
  }

  /* Just to handle the case where the BSS info for agent is not specified. In that case
   * Try to provide the default set
   */
  if (!i5WlCfgIsALMACPresentInControllerTable(pdevice->DeviceId)) {
    memset(tmpALMac, 0, sizeof(tmpALMac));
  } else {
    memcpy(tmpALMac, pdevice->DeviceId, sizeof(tmpALMac));
  }

  /* For each BSS in a interface */
  for (pbss = pdmif->bss_list.ll.next; pbss; pbss = pbss->ll.next) {
    if (pbss->mapFlags != 0) {
      continue;
    }
    /* For each BSS info in controller table check the matching AL MAC address */
    for (item_p = dll_head_p(&i5_config.client_bssinfo_list.head);
      !dll_end(&i5_config.client_bssinfo_list.head, item_p);
      item_p = next_p) {
      next_p = dll_next_p(item_p);
      list = (ieee1905_client_bssinfo_type*)item_p;

      /* Compare AL MAC */
      if (memcmp(list->ALID, tmpALMac, sizeof(list->ALID)) != 0) {
        continue;
      }
      if ((list->ssid.SSID_len != pbss->ssid.SSID_len) ||
        strcmp((char*)list->ssid.SSID, (char*)pbss->ssid.SSID) != 0) {
        continue;
      }
      /* If the band is not populated, populate it */
      if (pdmif->band == 0) {
        pdmif->band = (unsigned char)ieee1905_get_band_from_radiocaps(&pdmif->ApCaps.RadioCaps);
      }

      /* Validate the band as well */
      if (pdmif->band & list->band_flag) {
        pbss->mapFlags |= (list->BackHaulBSS ? IEEE1905_MAP_FLAG_BACKHAUL : 0);
        pbss->mapFlags |= (list->FrontHaulBSS ? IEEE1905_MAP_FLAG_FRONTHAUL : 0);
        if (pbss->mapFlags & IEEE1905_MAP_FLAG_FRONTHAUL) {
          isDedicated = 0;
        }
        i5Trace("Device["I5_MAC_DELIM_FMT"] BSS["I5_MAC_DELIM_FMT"] band[0x%x] list->band[0x%x] "
          "Update MAPFLAGS[0x%x]\n", I5_MAC_PRM(pdevice->DeviceId), I5_MAC_PRM(pbss->BSSID),
          pdmif->band, list->band_flag, pbss->mapFlags);
      }
    }
  }

  if (isDedicated) {
    pdevice->flags |= I5_CONFIG_FLAG_DEDICATED_BK;
  }
}

/* Update the parent device of the neighbors in pDevice except for pDevicePrev Device */
static void i5DmFindNeighborsParentDevice(i5_dm_device_type *pDevice,
  i5_dm_device_type *pDevicePrev)
{
  i5_dm_1905_neighbor_type *neighbor;

  for (neighbor = pDevice->neighbor1905_list.ll.next; neighbor; neighbor = neighbor->ll.next) {
    i5_dm_device_type *pDeviceTmp = i5DmDeviceFind(neighbor->Ieee1905Id);

    if (pDeviceTmp == NULL) {
      continue;
    }

    /* Inbetween devices will have neighbors as next device as well as previous device, So don't
     * process previous device
     */
    if (pDevicePrev &&
      !memcmp(pDevicePrev->DeviceId, pDeviceTmp->DeviceId, sizeof(pDevicePrev->DeviceId))) {
      continue;
    }

    /* Don't process the another instance running on the same device as it will become loop */
    if (I5_IS_CTRLAGENT(pDeviceTmp->flags)) {
      pDeviceTmp->parentDevice = i5_dm_network_topology.selfDevice;
      continue;
    }

    pDeviceTmp->parentDevice = pDevice;
    i5DmFindNeighborsParentDevice(pDeviceTmp, pDevice);
  }
}

/* Update the parent devices for all the devices in the network */
void i5DmUpdateParentDevice()
{
  i5_dm_device_type *pDeviceCtrl = i5_dm_network_topology.selfDevice;

  /* Validate Arguments */
  if (!pDeviceCtrl) {
    return;
  }

  pDeviceCtrl->parentDevice = NULL;

  i5DmFindNeighborsParentDevice(pDeviceCtrl, pDeviceCtrl);
}

