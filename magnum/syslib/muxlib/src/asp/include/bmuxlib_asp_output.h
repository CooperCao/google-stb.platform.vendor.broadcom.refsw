/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef BMUXLIB_ASP_OUTPUT_H_
#define BMUXLIB_ASP_OUTPUT_H_

#include "bmuxlib_asp_types.h"

/* Module Overview:
 *
 * The Output Interface is an abstraction to allow the mux manager to
 * send multi-plexed input data to a module that performs TS packetization.
 *
 * The mux manager calls BMUXlib_ASP_TS_Output_AddDescriptors with an array of
 * output descriptors. Any descriptors that aren't queued, the caller is
 * required to re-try transmission of only those descriptors that were not
 * queued.
 *
 * The mux manager calls BMUXlib_ASP_TS_Output_GetCompletedDescriptors to get a
 * count of output descriptors that have completed processing (e.g. all data
 * referenced in the descriptor has been packetized)
 */

typedef struct BMUXlib_ASP_Output_Descriptor_t
{
    uint32_t uiBufferAddressLo; /* [31:00] absolute address/offset of data to be muxed. */
    uint32_t uiBufferAddressHi; /* [63:32] absolute address/offset of data to be muxed. */
    uint32_t uiBufferLength; /* length in bytes of data to be muxed */

    uint32_t uiNextPacketPacingTimestamp; /* The 32-bit timestamp for when to start transmitting
                                           * packets generated from this buffer */
    uint32_t uiPacket2PacketTimestampDelta; /* The amount to increase the pacing counter value for
                                             * each packet that is generated from this buffer */
    uint8_t uiPidChannelIndex; /* The PID channel identifier for the type of data
                                * See:
                                *   BMUXlib_ASP_TS_Source_Sys_Interface_t.uiPIDChannelIndex OR
                                *   BMUXlib_ASP_TS_Source_Interface_t.uiPIDChannelIndex
                                */

    bool_t bNextPacketPacingTimestampValid; /* TRUE if uiNextPacketPacingTimestamp is valid */
    bool_t bPacket2PacketTimestampDeltaValid; /* TRUE if uiPacket2PacketTimestampDelta is valid */
    bool_t bPidChannelIndexValid; /* TRUE if uiPidChannelIndex is valid */

    bool_t bRandomAccessIndication; /* TRUE if the first TS packet generated from this buffer
                                     * needs to have the RAI bit set */
    bool_t bPushPartialPacket; /* TRUE if the rest of this packet needs to be padded (with
                                * adaptation stuffing bytes) to make a full TS packet */
    bool_t bPushPreviousPartialPacket; /* TRUE if the *previous* packets needs to be padded
                                        * to ensure the first packet generated from this buffer
                                        * starts on a TS packet boundary */
    bool_t bHostDataInsertion; /* TRUE if this buffer contains pre-formatted TS packets
                                * (e.g. PAT, PMT, PCR, etc) */
    bool_t bInsertHostDataAsBtp; /* TRUE if this buffer contains a BTP packet that needs to be
                                  * parsed/processed by the output module */
} BMUXlib_ASP_Output_Descriptor_t;

/* BMUXlib_ASP_TS_Output_AddDescriptors - Queues the specified count of output descriptors to the TS packetizer
 *  pvContext: the output context
 *  astOutputDescriptors: array of output descriptors
 *  uiCount: the count of valid output descriptors in the array
 *  *puiQueuedCount: the count of the number of descriptors that were actually queued
 */
typedef int
(*BMUXlib_ASP_Output_AddDescriptors)(
   void *pvContext,
   const BMUXlib_ASP_Output_Descriptor_t *astOutputDescriptors, /* Array of pointers to output descriptors */
   size_t uiCount, /* Count of descriptors in array */
   size_t *puiQueuedCount /* Count of descriptors queued (*puiQueuedCount <= uiCount) */
   );

/* BMUXlib_ASP_TS_Output_GetCompletedDescriptors - Returns the number of completed output descriptors
 *  pvContext: the output context
 *  *puiCompletedCount: the number of completed descriptors
 */
typedef int
(*BMUXlib_ASP_Output_GetCompletedDescriptors)(
   void *pvContext,
   size_t *puiCompletedCount /* Count of descriptors completed */
   );

typedef struct BMUXlib_ASP_Output_Interface_t
{
    void *pvContext;
    BMUXlib_ASP_Output_AddDescriptors fAddTransportDescriptors;
    BMUXlib_ASP_Output_GetCompletedDescriptors fGetCompletedTransportDescriptors;
} BMUXlib_ASP_Output_Interface_t;

#endif /* BMUXLIB_ASP_OUTPUT_H_ */
