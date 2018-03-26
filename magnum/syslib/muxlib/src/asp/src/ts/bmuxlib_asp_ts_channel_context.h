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

#ifndef BMUXLIB_ASP_TS_CHANNEL_CONTEXT_H_
#define BMUXLIB_ASP_TS_CHANNEL_CONTEXT_H_

#include "bmuxlib_asp_ts_settings.h"
#include "bmuxlib_asp_output_context.h"
#include "bmuxlib_asp_output_context.h"
#include "bmuxlib_asp_source_list_context.h"
#include "bmuxlib_asp_ts_memory_context.h"
#include "bmuxlib_asp_ts_source_av_context.h"
#include "bmuxlib_asp_ts_source_sys_context.h"
#include "bmuxlib_asp_ts_source_user_context.h"

/* accessor macros for use in testing to manipulate mux state */
#define BMUXLIB_ASP_TS_P_CHANNEL_GET_STATE(handle)         ((handle)->eState)
#define BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE(handle, state)  ((handle)->eState = (state))

typedef struct BMUXlib_ASP_TS_Channel_Context_t
{
   BMUXlib_ASP_Channel_State_e eState;
   BMUXlib_ASP_TS_ChannelStartSettings_t stStartSettings;
   BMUXlib_ASP_TS_ChannelStopSettings_t stStopSettings;

   struct
   {
      BMUXlib_ASP_TS_Memory_Context_t stMemory;
      BMUXlib_ASP_Output_Context_t stOutput;
      BMUXlib_ASP_TS_Source_AV_Context_t stSourceAV[BMUXLIB_ASP_TS_MAX_AV_SOURCE];
      BMUXlib_ASP_Source_List_Context_t stSourceList;
      BMUXlib_ASP_TS_Source_Sys_Context_t stSystem;
      BMUXlib_ASP_TS_Source_User_Common_Context_t stUserDataCommon;
      BMUXlib_ASP_TS_Source_User_Context_t stUserData[BMUXLIB_ASP_TS_MAX_USERDATA_SOURCE];
   } stContext;

   union
   {
      struct
      {
         BMUXlib_ASP_Output_Descriptor_Metadata_t stOutputDescriptorMetadata;
         BMUXlib_ASP_Output_Descriptor_t astTransportDescriptors[1];
      } stOutput;
   } stTemp;
} BMUXlib_ASP_TS_Channel_Context_t;

#endif /* BMUXLIB_ASP_TS_CHANNEL_CONTEXT_H_ */
