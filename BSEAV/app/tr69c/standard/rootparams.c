/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "rootparams.h"

TRXGFUNC(getDeviceSummary);

extern TRxObjNode  deviceInfoDesc[];
extern TRxObjNode  mgmtServerDesc[];
extern TRxObjNode  servicesDesc[];
extern TRxObjNode  ethernetDesc[];
extern TRxObjNode  mocaDesc[];
extern TRxObjNode  X_BROADCOM_COM_spectrumAnalyzerDesc[];
#ifdef PROPRIETARY
extern TRxObjNode  ProprietaryDesc[];
#endif

/* Device. */
TRxObjNode  deviceDesc[] = {
#ifdef XML_DOC_SUPPORT
	{DeviceSummary, {{tString, 1024, 0}}, NULL, getDeviceSummary, NULL, NULL, 1, 0, 0, 0, NULL, true},
	{DeviceInfo, {{tObject, 0, 0}}, NULL, NULL, deviceInfoDesc, NULL, 1, 0, 0, 0, NULL, true},
	{ManagementServer, {{tObject, 0, 0}}, NULL, NULL, mgmtServerDesc, NULL, 1, 0, 0, 0, NULL, true},
#ifdef TR135_SUPPORT
	{Services, {{tObject, 0, 0}}, NULL, NULL, servicesDesc, NULL, 1, 0, 0, 0, NULL, true},
#endif
#ifdef TR181_SUPPORT
    {Ethernet, {{tObject, 0, 0}}, NULL, NULL, ethernetDesc, NULL, 2, 0, 0, 0, NULL, true},
    {MoCA, {{tObject, 0, 0}}, NULL, NULL, mocaDesc, NULL, 2, 0, 0, 0, NULL, true},
#endif
#ifdef TR135_SUPPORT
	{X_BROADCOM_COM_SpectrumAnalyzer, {{tObject, 0, 0}}, NULL, NULL, X_BROADCOM_COM_spectrumAnalyzerDesc, NULL, 1, 0, 0, 0, NULL, true},
#endif
#ifdef PROPRIETARY
	{X_BROADCOM_COM, {{tObject, 0, 0}}, NULL, NULL, ProprietaryDesc, NULL, 1, 0, 0, 0, NULL, false},
#endif
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{DeviceSummary, {{tString, 1024, 0}}, NULL, getDeviceSummary, NULL, NULL},
	{DeviceInfo, {{tObject, 0, 0}}, NULL, NULL, deviceInfoDesc, NULL},
	{ManagementServer, {{tObject, 0, 0}}, NULL, NULL, mgmtServerDesc, NULL},
#ifdef TR135_SUPPORT
	{Services, {{tObject, 0, 0}}, NULL, NULL, servicesDesc, NULL},
#endif
#ifdef TR181_SUPPORT
    {Ethernet, {{tObject, 0, 0}}, NULL, NULL, ethernetDesc, NULL},
    {MoCA, {{tObject, 0, 0}}, NULL, NULL, mocaDesc, NULL},
#endif
#ifdef TR135_SUPPORT
	{X_BROADCOM_COM_SpectrumAnalyzer, {{tObject, 0, 0}}, NULL, NULL, X_BROADCOM_COM_spectrumAnalyzerDesc, NULL},
#endif
#ifdef PROPRIETARY
	{X_BROADCOM_COM, {{tObject, 0, 0}}, NULL, NULL, ProprietaryDesc, NULL},
#endif
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* . */
TRxObjNode  rootDevice[] = {
#ifdef XML_DOC_SUPPORT
    {Device, {{tObject, 0, 0}}, NULL, NULL, deviceDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Device, {{tObject, 0, 0}}, NULL, NULL, deviceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

