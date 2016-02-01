//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//
//  Filename:       EstbCannedSnmp.h
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  September 22, 2008
//
//****************************************************************************

#ifndef ESTB_CANNEDSNMP_H
#define ESTB_CANNEDSNMP_H

//********************** Include Files ***************************************
#include "MessageLog.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;
class BcmEstbCannedSnmpCommandTable;

class BcmEstbCannedSnmp 
{

public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmEstbCannedSnmp();
    
    // Copy Constructor.  
    //
    BcmEstbCannedSnmp( const BcmEstbCannedSnmp& rhs );

    // Destructor.  
    //
    virtual ~BcmEstbCannedSnmp();

    static BcmEstbCannedSnmp *GetSingletonInstance(void);

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;
   
    //
    // end functions required to be param for STL containers...

#if ( BCM_CONSOLE_SUPPORT )
    // This static method allows an object (including myself) to get access to
    // the singleton command table for controlling objects of my type.  This
    // helps avoid order of initialization problems by ensuring that the
    // object is created and initialized on-demand.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A reference to the command table singleton for my class.
    //
    static BcmEstbCannedSnmpCommandTable &GetCommandTableSingleton(void);

    // Since the command handler code is in the command table, I need to allow
    // it full access to me.
    friend class BcmEstbCannedSnmpCommandTable;
#endif


public:

	// Set_docsDevSwFilename(const char *fileName)
	//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
	//
	// Parameters:
	//      fileName - Pinter to null terminated file name ascii string.
	//
	// Returns:
	//      true - All is well
	//		False - Something didn't work
	//
	bool Set_docsDevSwFilename(const char *fileName);
	bool Get_docsDevSwFilename(char *fileName);

	// Set_docsDevSwServer()
	//		Create, send, and verify and SNMP SET for the docsDevSwServer MIB. The
	//		IP address used will always be 192.168.100.1 since this is the standard
	//		address used by the eCM for early SNMP access.
	//
	// Parameters:
	//      none.
	//
	// Returns:
	//      true - All is well
	//		False - Something didn't work
	//
	bool Set_docsDevSwServer(unsigned long ipaddress);
	bool Get_docsDevSwServer(unsigned long &ipaddress);


	// Set_docsDevSwAdminStatus()
	//		Create, send, and verify and SNMP SET for the docsDevSwAdminStatus MIB. The
	//		MIB will always be set to one since this is the value we need to initiate a
	//		secure software download.
	//
	// Parameters:
	//      none.
	//
	// Returns:
	//      true - All is well
	//		False - Something didn't work
	//
	bool Set_docsDevSwAdminStatus(uint8 status);
	bool Get_docsDevSwAdminStatus(uint16 &status);

	// Set_docsDevSwOperStatus()
	//		Create, send, and verify and SNMP SET for the docsDevSwOperStatus MIB. The
	//		MIB will always be set to one since this is the value we need to initiate a
	//		secure software download.
	//
	// Parameters:
	//      none.
	//
	// Returns:
	//      true - All is well
	//		False - Something didn't work
	//
	bool Set_docsDevSwOperStatus(uint8 status);
	bool Get_docsDevSwOperStatus(uint16 &status);

	// Get_docsDevSwCurrentVers(uint16 &major, uint16 &minor, uint16 &subMinor)
	//		Using SNMP, retrieve the version of the image currently running in the eCM
	//
	// Parameters:
	//      major - Major version number.
	//      minor - Minor version number.
	//      subMinor - Minor sub-version number.
	//
	// Returns:
	//      None. If fails then major, minor and subMinor are returned as 0
	//
	bool Get_docsDevSwCurrentVers(uint16 &major, uint16 &minor, uint16 &subMinor);

	bool Set_docsDevResetNow(uint8 docsDevResetNow);

private:   

    // For controlling message log output.
    BcmMessageLogSettings fMessageLogSettings;
 
	// This is the singleton instance of this class.
	static BcmEstbCannedSnmp *pfSingletonInstance;  
	
	unsigned long fMyIpAddress;

	// Request id for SNMP GET/SET requests
	uint16 fRequestId;

private:

    // Assignment operator.  NOT IMPLEMENTED.
    BcmEstbCannedSnmp& operator=( const BcmEstbCannedSnmp& rhs );

    // Less than operator. NOT IMPLEMENTED.
	bool operator<( const BcmEstbCannedSnmp& rhs ) const;

    // Equality operator. NOT IMPLEMENTED.
	bool operator==( const BcmEstbCannedSnmp& rhs ) const;  

};
//********************** Inline Method Implementations ***********************
inline ostream& operator<<( ostream& ostrm, const BcmEstbCannedSnmp& rhs )
{
    return rhs.Print( ostrm );
}
#endif

