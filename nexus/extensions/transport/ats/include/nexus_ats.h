/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

#ifndef NEXUS_ATS_H__
#define NEXUS_ATS_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
A Nexus extension to control the transport core's front-end
arrival timestmaps. Each transport packet that arrives at the
front-end input bands is tagged with an arrival timestamp. The
timestamps are used for various things, such as correcting for
jitter introduced by the Rate Smoothing and Xport Client DRAM
buffers. This extension exposes some of the programmable
features of the arrival timestamp logic.

NOTE: For transport streams brought in over MTSIF, the arrival
timestamp logic is located in the demod/tuner device. A separate
API is required for that.
**/

/*
Summary:
Reset the hardware ATS counter.
*/
void NEXUS_Ats_ResetCounter( void );

/*
Summary:
Configure timestamp counter to use externally generated clocks, i.e. the
timetamp is driven by the front-end demod/tuner chip.
*/
void NEXUS_Ats_SetExternal( void );

/*
Summary:
Configure timestamp counter to use internally generated clocks, i.e. the
timetamp is driven by the back-end chip.
*/
void NEXUS_Ats_SetInternal( void );

/*
Summary:
Return the current binary arrival timestamp.
*/
uint32_t NEXUS_Ats_GetBinaryTimestamp( void );

/*
Summary:
Set the current binary arrival timestamp.
*/
void NEXUS_Ats_SetBinaryTimestamp( uint32_t uiVal );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_ATS_H__ */
