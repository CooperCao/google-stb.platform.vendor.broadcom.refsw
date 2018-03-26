/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BMUXLIB_TRANSPORT_H_
#define BMUXLIB_TRANSPORT_H_

#include "bmuxlib.h"
#include "bavc_xpt.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* Legacy mappings */
#define BMUXlib_TS_TransportDescriptor  BMUXlib_TransportDescriptor
#define BMUXlib_TS_AddTransportDescriptors BMUXlib_Transport_AddDescriptors
#define BMUXlib_TS_GetCompletedTransportDescriptors BMUXlib_Transport_GetCompletedDescriptors
#define BMUXlib_TS_TransportChannelStatus BMUXlib_TransportChannelStatus
#define BMUXlib_TS_GetTransportChannelStatus BMUXlib_Transport_GetChannelStatus
#define BMUXlib_TS_TransportSettings BMUXlib_TransportSettings
#define BMUXlib_TS_GetTransportSettings BMUXlib_Transport_GetSettings
#define BMUXlib_TS_SetTransportSettings BMUXlib_Transport_SetSettings
#define BMUXlib_TS_TransportStatus BMUXlib_TransportStatus
#define BMUXlib_TS_GetTransportStatus BMUXlib_Transport_GetStatus
#define BMUXlib_TS_TransportDeviceInterface BMUXlib_Transport_DeviceInterface
#define BMUXlib_TS_TransportChannelInterface BMUXlib_Transport_ChannelInterface

/***********************/
/* Transport Interface */
/***********************/
typedef struct
{
   uint64_t uiBufferOffset; /* physical offset of data to be muxed */
   size_t uiBufferLength; /* length in bytes of data to be muxed */

   BAVC_TsMux_DescConfig stTsMuxDescriptorConfig;
} BMUXlib_TransportDescriptor;

/* BMUXlib_AddTransportDescriptors -
 * Adds transport descriptors for muxing.
 */
typedef BERR_Code
(*BMUXlib_Transport_AddDescriptors)(
         void *pvContext,
         const BMUXlib_TransportDescriptor *astDescriptors, /* Array of pointers to transport descriptors */
         size_t uiCount, /* Count of descriptors in array */
         size_t *puiQueuedCount /* Count of descriptors queued (*puiQueuedCount <= uiCount) */
         );

/* BMUXlib_Transport_GetCompletedDescriptors -
 * Returns the count of transport descriptors completed
 * since the *previous* call to BMUXlib_Transport_GetCompletedDescriptors
 */
typedef BERR_Code
(*BMUXlib_Transport_GetCompletedDescriptors)(
         void *pvContext,
         size_t *puiCompletedCount /* Count of descriptors completed */
         );

typedef struct
{
      unsigned uiIndex; /* PB Channel Index */
} BMUXlib_TransportChannelStatus;

typedef BERR_Code
(*BMUXlib_Transport_GetChannelStatus)(
         void *pvContext,
         BMUXlib_TransportChannelStatus *pstStatus
         );

typedef struct
{
      /* TODO: Add settings */
      uint32_t uiMuxDelay; /* in milliseconds */

      /* Each transport channel's pacing counter is seeded with:
       *   If bNonRealTimeMode=false: STC-uiMuxDelay
       *   If bNonRealTimeMode=true : uiPacingCounter
       */

      bool bNonRealTimeMode; /* Non Real Time Mode (NRT/AFAP) */

      struct
      {
            unsigned uiPacingCounter; /* in 27Mhz clock ticks */
      } stNonRealTimeSettings;
} BMUXlib_TransportSettings;

typedef BERR_Code
(*BMUXlib_Transport_GetSettings)(
         void *pvContext,
         BMUXlib_TransportSettings *pstSettings
         );

typedef BERR_Code
(*BMUXlib_Transport_SetSettings)(
         void *pvContext,
         const BMUXlib_TransportSettings *pstSettings
         );

typedef struct
{
      uint64_t uiSTC; /* 42-bit value in 27 Mhz */
      uint32_t uiESCR; /* 32-bit value in 27Mhz; */
      /* TODO: What other status? */
} BMUXlib_TransportStatus;

typedef BERR_Code
(*BMUXlib_Transport_GetStatus)(
         void *pvContext,
         BMUXlib_TransportStatus *pstStatus
         );

typedef struct
{
      void *pContext;
      BMUXlib_Transport_GetSettings fGetTransportSettings;
      BMUXlib_Transport_SetSettings fSetTransportSettings;
      BMUXlib_Transport_GetStatus fGetTransportStatus;
} BMUXlib_Transport_DeviceInterface;

typedef struct
{
      void *pContext;
      BMUXlib_Transport_AddDescriptors fAddTransportDescriptors;
      BMUXlib_Transport_GetCompletedDescriptors fGetCompletedTransportDescriptors;
      BMUXlib_Transport_GetChannelStatus fGetTransportChannelStatus;
} BMUXlib_Transport_ChannelInterface;

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_TRANSPORT_H_ */
