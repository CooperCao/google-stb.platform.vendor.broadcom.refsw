//****************************************************************************
//
// Copyright (c) 2011 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//    Description:                                                          
//                                                                          
//****************************************************************************
//    Revision History:                                                     
//                                                                          
//****************************************************************************
//    Filename: ocHnBridge.h
//    Creation Date: Friday, June 10, 2011 at 11:07:23
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file oc-home-network.mib
//
//****************************************************************************

#ifndef OCHNBRIDGE_H
#define OCHNBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "ocHnMib.h"


/*\
 *  ocHnDevUpnpServiceEntry 
\*/

#define MINVAL_OCHNDEVUPNPSERVICEINDEX  1
#define MAXVAL_OCHNDEVUPNPSERVICEINDEX  4294967295
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_CDS  1
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_SRS  2
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_QOSMANAGER  3
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_QOSDEVICE  4
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_QOSPOLICYHOLDER  5
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_AVT  6
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_CM  7
#define CONSTVAL_OCHNDEVUPNPSERVICETYPE_RCS  8
#define CONSTVAL_OCHNDEVUPNPSERVICEAVAILABLE_TRUE  1
#define CONSTVAL_OCHNDEVUPNPSERVICEAVAILABLE_FALSE  2

class ocHnDevUpnpServiceEntryBridge : public TabularMibBridge
{
  friend class ocHnDevUpnpServiceEntry;

  public:
    ocHnDevUpnpServiceEntryBridge (unsigned long ocHnDevUpnpServiceIndex, BcmSnmpAgent *pAgent);
   ~ocHnDevUpnpServiceEntryBridge ();

    /* GET methods */

    unsigned int Get_ocHnDevUpnpServiceIndex ();
    int Get_ocHnDevUpnpServiceType ();
    bool Get_ocHnDevUpnpServiceAvailable ();

    /* TEST methods - not required */


    /* SET methods */

};



/*\
 *  ocHnDevSupportedChannelEntry 
\*/

#define MINVAL_OCHNDEVSUPPORTEDCHANNELINDEX  1
#define MAXVAL_OCHNDEVSUPPORTEDCHANNELINDEX  4294967295
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELLASTOPERATINGFREQ_TRUE  1
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELLASTOPERATINGFREQ_FALSE  2
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELINUSE_TRUE  1
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELINUSE_FALSE  2
#define MINVAL_OCHNDEVSUPPORTEDCHANNELFREQUENCY  0
#define MAXVAL_OCHNDEVSUPPORTEDCHANNELFREQUENCY  2000000000
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELELIGIBLE_TRUE  1
#define CONSTVAL_OCHNDEVSUPPORTEDCHANNELELIGIBLE_FALSE  2

class ocHnDevSupportedChannelEntryBridge : public TabularMibBridge
{
  friend class ocHnDevSupportedChannelEntry;

  public:
    ocHnDevSupportedChannelEntryBridge (int ifIndex, unsigned long ocHnDevSupportedChannelIndex, BcmSnmpAgent *pAgent);
   ~ocHnDevSupportedChannelEntryBridge ();

    /* GET methods */

    unsigned int Get_ocHnDevSupportedChannelIndex ();
    bool Get_ocHnDevSupportedChannelLastOperatingFreq ();
    bool Get_ocHnDevSupportedChannelInUse ();
    unsigned int Get_ocHnDevSupportedChannelFrequency ();
    bool Get_ocHnDevSupportedChannelEligible ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ocHnDevSupportedChannelInUse (bool ocHnDevSupportedChannelInUse);
    SNMP_STATUS Set_ocHnDevSupportedChannelEligible (bool ocHnDevSupportedChannelEligible);
};



/*\
 *  ocHnRsdManagerStatusGroup 
\*/

#define MINVAL_OCHNRSDMANAGERIMPORTANCENUMBER  0
#define MAXVAL_OCHNRSDMANAGERIMPORTANCENUMBER  255
#define CONSTVAL_OCHNRSDMANAGERPREFERREDSTATUS_TRUE  1
#define CONSTVAL_OCHNRSDMANAGERPREFERREDSTATUS_FALSE  2

class ocHnRsdManagerStatusGroupBridge : public ScalarMibBridge
{
  friend class ocHnRsdManagerStatusGroup;

  public:
    ocHnRsdManagerStatusGroupBridge (BcmSnmpAgent *pAgent);
   ~ocHnRsdManagerStatusGroupBridge ();

    /* GET methods */

    unsigned int Get_ocHnRsdManagerImportanceNumber ();
    bool Get_ocHnRsdManagerPreferredStatus ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ocHnRsdManagerImportanceNumber (unsigned int ocHnRsdManagerImportanceNumber);
};



/*\
 *  ocHnDevPropertiesGroup 
\*/

#define MINLEN_OCHNDEVPROPERTIESFRIENDLYNAME  0
#define MAXLEN_OCHNDEVPROPERTIESFRIENDLYNAME  255

class ocHnDevPropertiesGroupBridge : public ScalarMibBridge
{
  friend class ocHnDevPropertiesGroup;

  public:
    ocHnDevPropertiesGroupBridge (BcmSnmpAgent *pAgent);
   ~ocHnDevPropertiesGroupBridge ();

    /* GET methods */

    int Get_ocHnDevPropertiesFriendlyName (BcmString &ocHnDevPropertiesFriendlyName);

    /* TEST methods - not required */


    /* SET methods */

};



/*\
 *  ocHnDevInterfaceConfigEntry 
\*/

#define CONSTVAL_OCHNDEVINTERFACECONFIGPOWERUNITS_OTHER  1
#define CONSTVAL_OCHNDEVINTERFACECONFIGPOWERUNITS_DBMV  2
#define CONSTVAL_OCHNDEVINTERFACECONFIGPOWERUNITS_DBM  3
#define CONSTVAL_OCHNDEVINTERFACECONFIGPOWERUNITS_MW  4
#define CONSTVAL_OCHNDEVINTERFACECONFIGENABLEENCRYPTION_ENABLED  1
#define CONSTVAL_OCHNDEVINTERFACECONFIGENABLEENCRYPTION_DISABLED  2
#define CONSTLEN_OCHNDEVINTERFACECONFIGENCRYPTIONPASSPHRASE  24 | 32

class ocHnDevInterfaceConfigEntryBridge : public TabularMibBridge
{
  friend class ocHnDevInterfaceConfigEntry;

  public:
    ocHnDevInterfaceConfigEntryBridge (int ifIndex, BcmSnmpAgent *pAgent);
   ~ocHnDevInterfaceConfigEntryBridge ();

    /* GET methods */

    int Get_ocHnDevInterfaceConfigMaxPowerLevel ();
    int Get_ocHnDevInterfaceConfigPowerUnits ();
    unsigned int Get_ocHnDevInterfaceConfigMaxParameterizedBandwidth ();
    int Get_ocHnDevInterfaceConfigEnableEncryption ();
    int Get_ocHnDevInterfaceConfigEncryptionPassphrase (BcmString &ocHnDevInterfaceConfigEncryptionPassphrase);

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ocHnDevInterfaceConfigMaxPowerLevel (int ocHnDevInterfaceConfigMaxPowerLevel);
    SNMP_STATUS Set_ocHnDevInterfaceConfigMaxParameterizedBandwidth (unsigned int ocHnDevInterfaceConfigMaxParameterizedBandwidth);
    SNMP_STATUS Set_ocHnDevInterfaceConfigEnableEncryption (int ocHnDevInterfaceConfigEnableEncryption);
    SNMP_STATUS Set_ocHnDevInterfaceConfigEncryptionPassphrase (const BcmString &ocHnDevInterfaceConfigEncryptionPassphrase);
};



/*\
 *  ocHnDevInterfaceStatusEntry 
\*/

#define CONSTVAL_OCHNDEVINTERFACESTATUSPOWERUNITS_OTHER  1
#define CONSTVAL_OCHNDEVINTERFACESTATUSPOWERUNITS_DBMV  2
#define CONSTVAL_OCHNDEVINTERFACESTATUSPOWERUNITS_DBM  3
#define CONSTVAL_OCHNDEVINTERFACESTATUSPOWERUNITS_MW  4
#define CONSTVAL_OCHNDEVINTERFACESTATUSLAYER2SCHEDULER_TRUE  1
#define CONSTVAL_OCHNDEVINTERFACESTATUSLAYER2SCHEDULER_FALSE  2

class ocHnDevInterfaceStatusEntryBridge : public TabularMibBridge
{
  friend class ocHnDevInterfaceStatusEntry;

  public:
    ocHnDevInterfaceStatusEntryBridge (int ifIndex, BcmSnmpAgent *pAgent);
   ~ocHnDevInterfaceStatusEntryBridge ();

    /* GET methods */

    unsigned int Get_ocHnDevInterfaceStatusTxBroadcastRate ();
    int Get_ocHnDevInterfaceStatusTxBroadcastLevel ();
    int Get_ocHnDevInterfaceStatusMaxTxPowerLevel ();
    int Get_ocHnDevInterfaceStatusPowerUnits ();
    unsigned int Get_ocHnDevInterfaceStatusMaxParameterizedBandwidth ();
    bool Get_ocHnDevInterfaceStatusLayer2Scheduler ();

    /* TEST methods - not required */


    /* SET methods */

};



/*\
 *  ocHnDevConnectionEntry 
\*/

#define MINVAL_OCHNDEVCONNECTIONINDEX  1
#define MAXVAL_OCHNDEVCONNECTIONINDEX  4294967295
#define MINLEN_OCHNDEVCONNECTIONRXMODULATIONTYPE  0
#define MAXLEN_OCHNDEVCONNECTIONRXMODULATIONTYPE  255
#define CONSTVAL_OCHNDEVCONNECTIONPOWERUNITS_OTHER  1
#define CONSTVAL_OCHNDEVCONNECTIONPOWERUNITS_DBMV  2
#define CONSTVAL_OCHNDEVCONNECTIONPOWERUNITS_DBM  3
#define CONSTVAL_OCHNDEVCONNECTIONPOWERUNITS_MW  4
#define MINLEN_OCHNDEVCONNECTIONTXMODULATIONTYPE  0
#define MAXLEN_OCHNDEVCONNECTIONTXMODULATIONTYPE  255

class ocHnDevConnectionEntryBridge : public TabularMibBridge
{
  friend class ocHnDevConnectionEntry;

  public:
    ocHnDevConnectionEntryBridge (int ifIndex, unsigned long ocHnDevConnectionIndex, BcmSnmpAgent *pAgent);
   ~ocHnDevConnectionEntryBridge ();

    /* GET methods */

    unsigned int Get_ocHnDevConnectionIndex ();
    void Get_ocHnDevConnectionDestMac (BcmMacAddress &ocHnDevConnectionDestMac);
    unsigned int Get_ocHnDevConnectionRxLinkRate ();
    int Get_ocHnDevConnectionRxModulationType (BcmString &ocHnDevConnectionRxModulationType);
    unsigned int Get_ocHnDevConnectionRxBroadcastRate ();
    int Get_ocHnDevConnectionRxSignalStrength ();
    int Get_ocHnDevConnectionTxUnicastLevel ();
    int Get_ocHnDevConnectionPowerUnits ();
    unsigned int Get_ocHnDevConnectionTxLinkRate ();
    int Get_ocHnDevConnectionTxModulationType (BcmString &ocHnDevConnectionTxModulationType);

    /* TEST methods - not required */


    /* SET methods */

};



#endif


