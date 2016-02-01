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


#include "sharedparams.h"
#include "baselineparams.h"

/* Device.DeviceInfo. */
TRXGFUNC(getManufacturer);
TRXGFUNC(getManufacturerOUI);
TRXGFUNC(getModelName);
TRXGFUNC(getProductClass);
TRXGFUNC(getSerialNumber);
TRXGFUNC(getSoftwareVersion);
TRXGFUNC(getHardwareVersion);
TRXGFUNC(getSpecVersion);
TRXGFUNC(getProvisioningCode);
TRXSFUNC(setProvisioningCode);
TRXGFUNC(getUpTime);
TRXGFUNC(getDeviceLog);

TRxObjNode  deviceInfoDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Manufacturer, {{tString, 64, 0}}, NULL, getManufacturer, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ManufacturerOUI, {{tString, 6, 0}}, NULL,getManufacturerOUI, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ModelName, {{tString, 64, 0}}, NULL,getModelName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Description, {{tString, 256, 0}}, NULL, NULL, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProductClass, {{tString, 256, 0}}, NULL, getProductClass, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SerialNumber, {{tString, 256, 0}}, NULL, getSerialNumber, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SoftwareVersion, {{tString, 64, 0}}, NULL, getSoftwareVersion, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HardwareVersion, {{tString, 64, 0}}, NULL, getHardwareVersion, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SpecVersion, {{tString, 16, 0}}, NULL, getSpecVersion, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ProvisioningCode, {{tString, 64, 0}}, setProvisioningCode, getProvisioningCode, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {UpTime, {{tUnsigned, 0, 1}}, NULL, getUpTime, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DeviceLog, {{tString, 32767, 1}}, NULL, getDeviceLog, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Manufacturer, {{tString, 64, 0}}, NULL, getManufacturer, NULL, NULL},
    {ManufacturerOUI, {{tString, 6, 0}}, NULL, getManufacturerOUI, NULL, NULL},
    {ModelName, {{tString, 64, 0}}, NULL, getModelName, NULL, NULL},
    {Description, {{tString, 256, 0}}, NULL, NULL, NULL, NULL},
    {ProductClass, {{tString, 256, 0}},NULL, getProductClass, NULL, NULL},
    {SerialNumber, {{tString, 256, 0}}, NULL, getSerialNumber, NULL, NULL},
    {SoftwareVersion, {{tString, 64, 0}}, NULL, getSoftwareVersion, NULL, NULL},
    {HardwareVersion,{{tString, 64, 0}}, NULL, getHardwareVersion, NULL, NULL},
    {SpecVersion, {{tString, 16, 0}}, NULL, getSpecVersion, NULL, NULL},
    {ProvisioningCode, {{tString, 64, 0}}, setProvisioningCode, getProvisioningCode, NULL, NULL},
    {UpTime, {{tUnsigned, 0, 1}}, NULL, getUpTime, NULL, NULL},
    {DeviceLog, {{tString, 32767, 1}}, NULL, getDeviceLog, NULL, NULL},
    {NULL, {{0,0,0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.MangementServer. */
/* A copy of most of this data is stored in the acsState structure */
TRXGFUNC(getMSrvrURL);
TRXGFUNC(getMSrvrInformEnable);
TRXGFUNC(getMSrvrInformInterval);
TRXGFUNC(getMSrvrInformTime);
TRXGFUNC(getMSrvrParameterKey);
TRXGFUNC(getConnectionReqURL);
TRXGFUNC(getConnectionUsername);
TRXGFUNC(getConnectionPassword);
TRXGFUNC(getKickURL);
TRXGFUNC(getUpgradesManaged);
TRXSFUNC(setMSrvrURL);
TRXSFUNC(setMSrvrUsername);
TRXGFUNC(getMSrvrUsername);
TRXSFUNC(setMSrvrPassword);
TRXGFUNC(getMSrvrPassword);
TRXSFUNC(setMSrvrInformEnable);
TRXSFUNC(setMSrvrInformInterval);
TRXSFUNC(setMSrvrInformTime);
TRXSFUNC(setConnectionUsername);
TRXSFUNC(setConnectionPassword);
TRXSFUNC(setKickURL);
TRXSFUNC(setUpgradesManaged);
TRxObjNode  mgmtServerDesc[] = {
#ifdef XML_DOC_SUPPORT
    {URL, {{tString, 256, 1}}, setMSrvrURL, getMSrvrURL, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Username, {{tString, 256, 1}}, setMSrvrUsername, getMSrvrUsername, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Password, {{tStringSOnly, 256, 1}}, setMSrvrPassword, getMSrvrPassword, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PeriodicInformEnable, {{tBool, 0, 1}}, setMSrvrInformEnable, getMSrvrInformEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PeriodicInformInterval, {{tUnsigned, 0, 1}}, setMSrvrInformInterval, getMSrvrInformInterval, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PeriodicInformTime, {{tDateTime, 0, 1}}, setMSrvrInformTime, getMSrvrInformTime, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ParameterKey, {{tString,32,1}}, NULL, getMSrvrParameterKey, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ConnectionRequestURL, {{tString,256, 1}},NULL, getConnectionReqURL, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ConnectionRequestUsername, {{tString,256, 1}}, setConnectionUsername, getConnectionUsername, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ConnectionRequestPassword, {{tStringSOnly,256, 1}}, setConnectionPassword, getConnectionPassword, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {KickURL, {{tString,256, 1}}, setKickURL, getKickURL, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {UpgradesManaged, {{tBool, 0, 1}}, setUpgradesManaged, getUpgradesManaged, NULL, NULL, 1, 0, 0, 0, NULL, true},
    /*{DownloadProgressURL, {{tString, 256}}, NULL, NULL, NULL, NULL, 1, 0, 0, 0, NUL, trueL},*/
    {NULL, {{tUnsigned,0,1}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {URL, {{tString,256, 1}}, setMSrvrURL, getMSrvrURL, NULL, NULL},
    {Username, {{tString,256, 1}}, setMSrvrUsername, getMSrvrUsername, NULL, NULL},
    {Password, {{tStringSOnly,256, 1}}, setMSrvrPassword, getMSrvrPassword, NULL, NULL},
    {PeriodicInformEnable, {{tBool, 0, 1}}, setMSrvrInformEnable, getMSrvrInformEnable, NULL, NULL},
    {PeriodicInformInterval, {{tUnsigned, 0, 1}}, setMSrvrInformInterval, getMSrvrInformInterval, NULL, NULL},
    {PeriodicInformTime, {{tDateTime, 0, 1}}, setMSrvrInformTime, getMSrvrInformTime,NULL, NULL},
    {ParameterKey, {{tString, 32, 1}}, NULL, getMSrvrParameterKey, NULL, NULL},
    {ConnectionRequestURL, {{tString, 256, 1}},NULL,getConnectionReqURL, NULL, NULL},
    {ConnectionRequestUsername, {{tString, 256, 1}}, setConnectionUsername, getConnectionUsername, NULL, NULL},
    {ConnectionRequestPassword, {{tStringSOnly, 256, 1}}, setConnectionPassword, getConnectionPassword, NULL, NULL},
    {KickURL, {{tString, 256, 1}}, setKickURL, getKickURL, NULL, NULL},
    {UpgradesManaged, {{tBool, 0, 1}}, setUpgradesManaged, getUpgradesManaged, NULL, NULL},
    /*{DownloadProgressURL, {{tString, 256}}, NULL, NULL, NULL, NULL},*/
    {NULL, {{tUnsigned,0,1}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Services. */
extern TRxObjNode STBServiceInstanceDesc[];
#ifdef TR135_SUPPORT
TRxObjNode  servicesDesc[] = {
#ifdef XML_DOC_SUPPORT
    {STBService, {{tInstance, 0, 0}}, NULL, NULL, STBServiceInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {STBService, {{tInstance, 0, 0}}, NULL, NULL, STBServiceInstanceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};
#endif

/* Device.Ethernet. */

extern TRxObjNode  ethernetInterfaceInstanceDesc[];

TRXGFUNC(getEthernetInterfaceNumberOfEntries);
TRXGFUNC(getEthernetLinkNumberOfEntries);
TRXGFUNC(getEthernetVLANTerminationNumberOfEntries);
TRXGFUNC(getEthernetRMONStatsNumberOfEntries);

#ifdef TR181_SUPPORT
TRxObjNode  ethernetDesc[] = {
#ifdef XML_DOC_SUPPORT
    {InterfaceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true},
    /* {LinkNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetLinkNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true}, */
    /* {VLANTerminationNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetVLANTerminationNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true}, */
    /* {RMONStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetRMONStatsNumberOfEntries, NULL, NULL, 2, 0 0, 0, NULL, true}, */
    /* {RMONStats, {{tInstance, 0, 0}}, NULL, NULL, ethernetRMONStatsInstanceDesc, NULL, 2, 4, 0, 0xffffffff, NULL, true}, */
    {Interface, {{tInstance, 0, 0}}, NULL, NULL, ethernetInterfaceInstanceDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true},
    /* {Link, {{tInstance, 0, 0}}, NULL, NULL, ethernetLinkInstanceDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true}, */
    /* {VLANTermination, {{tInstance, 0, 0}}, NULL, NULL, ethernetVLANTerminationInstanceDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true}, */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {InterfaceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceNumberOfEntries, NULL, NULL},
    /* {LinkNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetLinkNumberOfEntries, NULL, NULL}, */
    /* {VLANTerminationNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetVLANTerminationNumberOfEntries, NULL, NULL}, */
    /* {RMONStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getEthernetRMONStatsNumberOfEntries, NULL, NULL}, */
    /* {RMONStats, {{tInstance, 0, 0}}, NULL, NULL, ethernetRMONStatsInstanceDesc, NUL}, */
    {Interface, {{tInstance, 0, 0}}, NULL, NULL, ethernetInterfaceInstanceDesc, NULL},
    /* {Link, {{tInstance, 0, 0}}, NULL, NULL, ethernetLinkInstanceDesc, NULL}, */
    /* {VLANTermination, {{tInstance, 0, 0}}, NULL, NULL, ethernetVLANTerminationInstanceDesc, NULL}, */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};
#endif

/* Device.Moca. */

extern TRxObjNode  mocaInterfaceDesc[];

TRXGFUNC(getMocaInterfaceNumberOfEntries);

#ifdef TR181_SUPPORT
TRxObjNode  mocaDesc[] = {
#ifdef XML_DOC_SUPPORT
    {InterfaceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Interface, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {InterfaceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNumberOfEntries, NULL, NULL},
    {Interface, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};
#endif

