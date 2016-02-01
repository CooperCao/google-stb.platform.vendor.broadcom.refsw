//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//  Filename:       DsgCommandTable.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  December 13, 2004
//
//****************************************************************************
//  Description:
//      This derived class handles controlling the DSG-CC objects.
//
//****************************************************************************

#ifndef DSGCOMMANDTABLE_H
#define DSGCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "CommandTable.h"
#include "DsgClientCtlThread.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

typedef enum
{
	kTestDsgSetDSGMode = 40,
	kTestDsgEnableDocsis,
    kTestDsgSetTimer,
    kTestDsgGetTimer,
	kTestDsgSetChannelList,
	kTestDsgResetChannelList,
	kTestDsgScan,
	kTestDsgStatus,
	kTestDsgDocsisStatus
}DsgApiTestMessageType;

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmDsgCommandTable : public BcmCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDsgCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgCommandTable();

protected:

private:

#ifndef DSG_RPC_INCLUDED //#if !defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) && !defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	static unsigned char singleProcessModelPortNumber;
#endif

    // This is the entrypoint that is called whenever one of my commands is
    // entered.
    //
    // Parameters:
    //      pInstanceValue - the instance value to which the command should be
    //                       applied.  Instances must be registered with the
    //                       command table in order for them to receive the
    //                       command.
    //      command - the command (with parameters) that was entered by the user.
    //
    // Returns:  Nothing.
    //
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgCommandTable(const BcmDsgCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgCommandTable & operator = (const BcmDsgCommandTable &otherInstance);

	static void SetMode( uint32 mode );
	static void EnableDocsis (bool enable);

	static void OpenTunnel( uint8 tunnelListIndex);
	static void CloseTunnel( int tunnel_num );

	static void SetTimer( uint8 timer_id, uint16 timeout );
	static void GetTimer( uint8 timer_id );

	static void SetChannelList (uint32 num_chans, uint32 * channel_list);
	static void ResetChannelList ( void ) ;
	static void Scan ( bool enable );

	static void DsgStatus ( void );
	static void DocsisStatus ( void );

	static bool AddTestClientAppType(BcmString &name);
	static bool AddTestClientMacAddress(const BcmMacAddress &macAddress);
	static bool AddTestClientUint16(const uint32 type, const uint16 id, dsg_classifier *dsgClassifier);

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	static bool Ping(const BcmIpAddress &ipAddress, uint32 numberOfPings);
#endif

};


//********************** Inline Method Implementations ***********************


#endif


