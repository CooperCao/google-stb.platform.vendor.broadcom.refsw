/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
*
***************************************************************************/

#ifndef NEXUS_FRONTEND_MTSIF_PRIV_H__
#define NEXUS_FRONTEND_MTSIF_PRIV_H__

#include "nexus_types.h"
#include "priv/nexus_core.h"

#define NEXUS_FRONTEND_USER_PARAM1_GET_INPUT_BAND(PARAM)      ((PARAM >> 0) & 0xFF)
#define NEXUS_FRONTEND_USER_PARAM1_SET_INPUT_BAND(PARAM, VAL) (PARAM = (PARAM & 0xFFFFFF00) | ((VAL & 0xFF) << 0))

#define NEXUS_FRONTEND_USER_PARAM1_GET_MTSIF_TX(PARAM)        ((PARAM >> 8) & 0xF)
#define NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(PARAM, VAL)   (PARAM = (PARAM & 0xFFFFF0FF) | ((VAL & 0xF) << 8))

#define NEXUS_FRONTEND_USER_PARAM1_GET_DAISYCHAIN_MTSIF_TX(PARAM)        ((PARAM >> 12) & 0xF)
#define NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_MTSIF_TX(PARAM, VAL)   (PARAM = (PARAM & 0xFFFF0FFF) | ((VAL & 0xF) << 12))

#define NEXUS_FRONTEND_USER_PARAM1_GET_DAISYCHAIN_OVERRIDE(PARAM)        ((PARAM >> 16) & 0xF)
#define NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_OVERRIDE(PARAM, VAL)   (PARAM = (PARAM & 0xFFF0FFFF) | ((VAL & 0xF) << 16))

#endif
