//****************************************************************************
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
//  Filename:       EstbTargetMain.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This file contains the target startup code (i.e. code related to the
//      embedded system target, as opposed to a host-based simulator target).
//
//****************************************************************************

#ifndef EstbTargetMain_H
#define EstbTargetMain_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmCommandTable;

//********************** Function Declarations *******************************

// This function does the target-specific initialization for CM hardware
// (loading low-level drivers for flash, nonvol, etc), and also creates
// singletons like the target-specific BcmNonVolDevice, etc.
//
// Parameters:  None.
//
// Returns: Nothing.
//
void EstbTargetLowLevelInit(void);

// This function does the target-specific startup.  This is called after nonvol
// settings have been read and validated.  This does stuff like configure DMA
// buffer allocations, start the watchdog, and other board/target initialization
// that requires nonvol settings.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void EstbTargetPostNonvolInit(void);

// This function installs the target-specific command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//      commandIdBase - the base id that I should use for the commands that I
//                      add to the command table.
//
// Returns:  Nothing.
//
void EstbTargetCmdTableInit(BcmCommandTable *pParentCmdTable,
                          unsigned int commandIdBase);

//********************** Inline Method Implementations ***********************

#endif


