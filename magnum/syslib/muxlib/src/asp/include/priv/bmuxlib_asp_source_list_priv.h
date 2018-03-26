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

#ifndef BMUXLIB_ASP_SOURCE_LIST_PRIV_H_
#define BMUXLIB_ASP_SOURCE_LIST_PRIV_H_

#include "bmuxlib_asp_source.h"
#include "bmuxlib_asp_source_entry_priv.h"

typedef struct BMUXlib_Source_List_Context_t* BMUXlib_Source_List_Handle;

typedef struct BMUXlib_Source_List_Dispatch_t
{
   BMUXLIB_ASP_P_Source_Entry_Start fStart;
   BMUXLIB_ASP_P_Source_Entry_Reconcile fReconcile;
   BMUXLIB_ASP_P_Source_Entry_DoMux fDoMux;
} BMUXlib_Source_List_Dispatch_t;

void
BMUXlib_ASP_P_Source_List_Reset(
   BMUXlib_Source_List_Handle hSourceListHandle
   );

bool_t
BMUXlib_ASP_P_Source_List_IsEmpty(
   BMUXlib_Source_List_Handle hSourceListHandle
   );

void
BMUXlib_ASP_P_Source_List_Insert(
   BMUXlib_Source_List_Handle hSourceListHandle,
   const BMUXlib_Source_Entry_Handle hSourceEntryHandle
   );

void
BMUXlib_ASP_P_Source_List_Peek(
   BMUXlib_Source_List_Handle hSourceListHandle,
   BMUXlib_Source_Entry_Handle *phSourceEntryHandle
   );

void
BMUXlib_ASP_P_Source_List_Pop(
   BMUXlib_Source_List_Handle hSourceListHandle,
   BMUXlib_Source_Entry_Handle *phSourceEntryHandle
   );

#endif /* BMUXLIB_ASP_SOURCE_PRIV_H_ */
