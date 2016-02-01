//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  $Id$
//
//  Filename:       EstbStatusEventCodes.h
//  Author:         Maurice Turcotte
//  Creation Date:  22 MAY 06
//
//****************************************************************************
//  Description:
//      Event codes for use with the EstbStatusEventACT client
//
//****************************************************************************
//
//
#ifndef ESTBSTATUSEVENTCODES_H
#define ESTBSTATUSEVENTCODES_H


class BcmEstbStatusEventCodes 
{
public:
    enum
    {
        // event codes used in Asynchronous Completion Token client
        // interface.
        
        //
        // Time of Day has been reset. SNMPv3 cares about this.
        kEstbTodInitOk = 0,

        //
        // A new config file has arrived at the Estb
        // Note that customer vendor specific items are not parsed.
        kEstbConfigFileNew,

        //
        // Estb is operational  
        // event published by .
        kEstbIsOperational,

        // Estb is about to reboot itself.
        // event published by .
        kEstbIsShuttingDown

    };

    // IsDocsisIpInitStatusEvent() - Returns true if specified event 
    //      is associated with DOCSIS IP initialization.
    //
    // static bool IsDocsisIpInitStatusEvent( unsigned int event_code );
};

// returns enum name as char*.  useful for debug output.
const char* CmDocsisStatusEventCodeName( unsigned int event_code );

#endif
