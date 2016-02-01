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
//  $Id: V2Main.h 1.2 2002/12/04 16:06:57Z dpullen Release $
//
//  Filename:       V2Main.h
//  Author:         David Pullen
//  Creation Date:  Nov 27, 2002
//
//****************************************************************************
//  Description:
//      This file contains V2 startup code common across all V2 applications.
//
//      A bunch of this code was previously in the vxWorks/pSOS/Linux startup
//      code, and has been moved here since it isn't OS-specific, and we don't
//      like lots of duplicate code lying around.
//
//****************************************************************************

#ifndef V2Main_H
#define V2Main_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

enum
{
    kNonVolRead_NoProblem = 0,
    kNonVolRead_ErrorReadingPermanent = 0x01,
    kNonVolRead_ErrorReadingDynamic = 0x02,
    kNonVolRead_ErrorReadingBoth = 0x03
};

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmNonVolSettings;
class BcmCommandTable;

//********************** Function Declarations *******************************

// This function handles reading and verifying nonvol settings.
//
// Parameters:
//      pSettings - pointer to the settings object (which is likely the root
//                  object of a composite) that is to be read.
//
// Returns:
//      One of the bitmask values above.
//
unsigned int V2MainReadNonVol(BcmNonVolSettings *pSettings);

// This function installs the common V2 command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//      commandIdBase - the base id that I should use for the commands that I
//                      add to the command table.
//
// Returns:  Nothing.
//
void V2CmdTableInit(BcmCommandTable *pParentCmdTable,
                    unsigned int commandIdBase);

//********************** Inline Method Implementations ***********************

#endif


