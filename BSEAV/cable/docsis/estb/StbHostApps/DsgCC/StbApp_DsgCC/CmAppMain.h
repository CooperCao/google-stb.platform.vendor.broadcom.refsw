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
//  Filename:       CmAppMain.h
//  Author:         David Pullen
//  Creation Date:  Nov 27, 2002
//
//****************************************************************************
//  Description:
//      This file contains CM application startup code.
//
//      A bunch of this code was previously in the vxWorks/pSOS/Linux startup
//      code, and has been moved here since it isn't OS-specific, and we don't
//      like lots of duplicate code lying around.
//
//****************************************************************************

#ifndef CmAppMain_H
#define CmAppMain_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************
typedef enum
{
    // this means to the mini USFS snoop on none of the interfaces
    kInstallNone,
    // this means to the mini USFS snoop on all the interfaces except the default Halif
    kInstallAll,
    // this means to the mini USFS snoop on all the stacks.
    kInstallStack
} MiniUsfsInstall;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Function Declarations *******************************

void AppPreNonvolInit(void);

void AppPostNonvolInit(void);

void AppRun(void);

//********************** Inline Method Implementations ***********************

#endif


