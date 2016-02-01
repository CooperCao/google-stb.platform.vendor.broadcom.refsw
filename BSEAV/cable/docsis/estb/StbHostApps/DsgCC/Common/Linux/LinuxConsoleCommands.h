//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  Filename:       LinuxConsoleCommands.h
//  Author:         David Pullen
//  Creation Date:  December 5, 2002
//
//****************************************************************************
//  Description:
//      This file contains V2 console commands specific to Linux.  All V2
//      applications built for Linux will likely want to include these
//      commands somewhere (usually the main command table).
//
//****************************************************************************

#ifndef LinuxConsoleCommands_H
#define LinuxConsoleCommands_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmCommandTable;

//********************** Function Declarations *******************************

// This function installs the Linux-specific command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//      commandIdBase - the base id that I should use for the commands that I
//                      add to the command table.
//
// Returns:  Nothing.
//
void LinuxCmdTableInit(BcmCommandTable *pParentCmdTable,
                       unsigned int commandIdBase);

//********************** Inline Method Implementations ***********************

#endif


