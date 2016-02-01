//****************************************************************************
//
//  Copyright (c) 1999  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       DcdMsg.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 2, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the DCD MAC
//      Management Message.
//
//****************************************************************************

#ifndef DCDMSG_H
#define DCDMSG_H

//********************** Include Files ***************************************

// .
#include <iostream>
#include <list>
#include "OctetBuffer.h"

#include "DsgEstbTunnelSettings.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;
class BcmDsgEstbTunnelSettingsList;
class BcmDsgConfigSettings;

class BcmDcdMsg
{
public:
    // Default Constructor.
    //
    BcmDcdMsg();

    // Constructor.
    //
    // Parameters:
    //      pTunnelManager - destination MAC address.
    //
    //      pClassifierList - source MAC address.
    //
    //      sid - primary sid of CM.
    //
    //      pChannelList - summary result code.
    //
    //
    //
    BcmDcdMsg(BcmDsgEstbTunnelSettingsList* pTunnelManager, ClassifierSettingsList* pClassifierList, 
					 BcmDsgConfigSettings* pConfigSettings );

    // Copy Constructor.
    //
    BcmDcdMsg( const BcmDcdMsg& rhs );

    // Assignment operator.
    //
    BcmDcdMsg& operator=( const BcmDcdMsg& rhs );

    // Destructor.
    //
    virtual ~BcmDcdMsg();

    // misc accessors.
    uint8 ConfigurationChangeCount() const { return fConfigurationChangeCount; }
    void ConfigurationChangeCount( uint8 new_val ) { fConfigurationChangeCount = new_val; }
    //
    uint8 NumberOfFragments() const { return fNumberOfFragments; }
    void NumberOfFragments( uint8 new_val ) { fNumberOfFragments = new_val; }
    //
    uint8 FragmentSequenceNumber() const { return fFragmentSequenceNumber; }
    void FragmentSequenceNumber( uint8 new_val ) { fFragmentSequenceNumber = new_val; }

    // This is called by WriteTo() after the MAC Mgmt header has been
    // written to the buffer.  It gives the derived class a chance to write
    // its message-specific data to the buffer.
    //
    // Parameters:
    //      octetBuffer - reference to the octet buffer into which the message
    //                    should be written.
    //
    // Returns:
    //      true if the message was stored in the buffer correctly.
    //      false if there was a problem (buffer not big enough, incorrect or
    //          invalid configuration, etc.).
    //
    // Notes:
    //      Called when a CM is composing a REG-REQ to send.
    //
    //virtual bool DerivedWriteTo( BcmOctetBuffer &octetBuffer );

    // This is called by ParseFrom() after the MAC Mgmt header has been
    // parsed from the buffer.  It gives the derived class a chance to parse
    // its message-specific data from the buffer.
    //
    // Note that the return code from this method only implies success or
    // failure in parsing of the message, not the contents or meaning of the
    // message (i.e. whether or not the CMTS sent a SUCCESS or FAILURE
    // confirmation code).
    //
    // Parameters:
    //      octetBuffer - reference to the octet buffer from which the message
    //                    should be parsed.  If is assumed that the first byte
    //                    in the buffer corresponds to the first byte of the
    //                    payload data.
    //      offset - the offset into the buffer where the first byte of the
    //               message is located (allows other header info to be skipped).
    //
    // Returns:
    //      true if the message was parsed correctly.
    //      false if there was a problem (incomplete message, invalid data,
    //          etc.).
    //
    // Notes:
    //      
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer,
		unsigned int &offset );

	byte ParseTLVBuffer(const BcmOctetBuffer &octetBuffer,
		unsigned int &offset, BcmMessageLogSettings* log_sets, bool parse_only);


	// This method should only be called after all DCD fragments are received.
	// DCD fragments are parsed as they are received. Tunnel Settings List 
	// and Classifier Settings List are filled in with DCD contents.
	// After the DCD is completed, these two lists must be merged for final processing.
	//
    // Parameters:
	//
    // Returns:
    //      true if at least one tunnel is usable
    //      false if all tunnels are defected.
    //
    // Notes:
    //      
    //
	bool FinalizeDcdParsing(void);

	void OctetBuffer(BcmOctetBuffer *pDcdBuffer);
	const BcmOctetBuffer * OctetBuffer(void);

    // DerivedPrint - This function is implemented by classes derived
    //      from BcmMacManagementMessage to print formatted derived 
    //      object contents to the specified ostream.  It is called 
    //      by BcmMacManagementMessage::Print() (a.k.a. the base class
    //      Print() function).
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

	void PrintTLVs(void);

protected:
	byte ParseDsPacketClassificationSets(const BcmOctetBuffer &obuf,
		unsigned int &offset, uint8 length, ErrorSetList& err_set_list, 
		BcmMessageLogSettings* log_sets, bool strict_checking, bool parse_only = false );

	byte ParseDsgRuleSets( const BcmOctetBuffer &obuf, unsigned int &offset,
		BcmMessageLogSettings* log_sets, bool parse_only = false );

	byte ParseDsgConfigurationSets( const BcmOctetBuffer &octetBuffer, unsigned int &offset,
		BcmMessageLogSettings* log_sets, bool strict_checking = false );

private:
    // Configuration Change Count.
    uint8 fConfigurationChangeCount;
    // 
	uint8 fNumberOfFragments;
    //
	uint8 fFragmentSequenceNumber;

	// Pointers to several lists to use when parsing the buffer.
	// The lists live in Dsg Client Controller object.
	BcmDsgEstbTunnelSettingsList* pfTunnelManager;
	ClassifierSettingsList* pfClassifierList; 
	BcmDsgConfigSettings* pfConfigSettings;

	bool fPacketClassificationSetsSpecified;
	bool fPacketClassificationSetsRequired;
	bool fDsgRuleSetsSpecified;	
	bool fDsgConfigurationSetsSpecified;

	// Pointer to the buffer that stores the most
	// recently received NEW DCD.
	BcmOctetBuffer *pfDcdBuffer;

    // For controlling message log output.
    BcmMessageLogSettings fMessageLogSettings;

};

inline ostream& operator<<( ostream& ostrm, BcmDcdMsg& rhs )
{
    return rhs.Print( ostrm );
}





#endif


