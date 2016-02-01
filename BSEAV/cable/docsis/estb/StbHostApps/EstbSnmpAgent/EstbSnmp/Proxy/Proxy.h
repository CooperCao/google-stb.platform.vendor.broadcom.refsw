//***************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       Proxy.h
//  Author:         Maurice Turcotte / Thomas Clack
//  Creation Date:  4 AUG 06
//
//****************************************************************************
//  Description:  
//
//****************************************************************************

#ifndef BCMPROXY_H
#define BCMPROXY_H

//********************** Include Files ***************************************
#include "EthernetPacket.h"
#include "ProxySnmpAgent.h"


class BcmOctetBuffer;
class BcmMutexSemaphore;
class BcmSocket;
class BcmSocketSet;
// class BcmHalIf;

#define MORE_PACKETS 0x01
#define MAX_PKT_SIZE 1400

#define PROXY_SRC_PORT 520
#define PROXY_DEST_PORT 0x4250

#define kProxyCMPrivateStackInterface 2

/*//  IPC_COMMAND_TYPES
    enum
    {
        //If any additional download commands are added, make sure that they are put before
        // the kIPCDloadAbort numeration since we do a <= check on that as the last download
        // command.
        kIPC_SnmpSetCommand = 0,
        kIPC_SnmpGetCommand,
        kIPC_SnmpNextCommand,
        kIPC_SnmpTestCommand
    };   
*/    
//  IPC_RESPONSE_TYPES
/*    enum 
    {
        kIPC_NoError = 0,
        kIPC_SnmpSetError,
        kIPC_SnmpGetError,
        kIPC_GenericReadError,
        kIPC_CommandInvalidError
    };
*/    
//  IPC_STATUS_TYPES
/*    enum
    {
        kSocketNotInitialized = 0,
        kSocketError = 1,
        kSocketOk = 2,
        kDloadNotRunning,
        kDloadStarted
    };   
*/
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmProxy
{

public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmProxy();

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmProxy();

    static uint32 DloadStatus(void);
    static uint32 ProxyStatus(void);

    static bool Initialize(BcmSnmpAgent *masterAgent);

    static bool CommandWrite(BcmOctetBuffer &value, BcmOctetBuffer *returnBuffer = NULL);
    static bool SendPacket( BcmString &SendMsg, BcmOctetBuffer *returnBuffer);

protected:

    static bool CommandRead(BcmOctetBuffer &rtvvalue, uint32 timeoutVal);
   

    // Determines how much message logging I will do.
    static BcmMessageLogSettings fMessageLogSettings; 

    static uint32 fIPCDloadStatus;
    static uint32 fProxyStatus;

    static BcmSocket *pProxySocket;
    static BcmSocketSet *pSocketSet;
//     static BcmHalIf *pHalIf;
    static BcmIpAddress fSourceIp;
    static unsigned char fStackNum;
    // Protects access to the write function.  This
    // is necessary because multiple threads can be
    // trying to send packets at the same time and this process
    // must be kept to a one-one process
    static BcmMutexSemaphore *pMutex;
    static BcmProxySnmpAgent *fpSubAgent;

};


//********************** Inline Method Implementations ***********************



#endif


