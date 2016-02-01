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
//  $Id: Counter64.cpp 1.2 2003/11/10 21:51:01Z dpullen Release $
//
//  Filename:       Counter64.cpp
//  Author:         Kevin O'Neal/David Pullen
//  Creation Date:  October 28, 2002
//
//****************************************************************************
//  Description:
//      This is a numerical class that represents a 64-bit number.  It is most
//      often used as a counter, and was originally used for the ifXTable MIB
//      counters, which is where the name came from.  This is a very shallow
//      implementation, with the bare minimum numerical operations that we
//      need, most of which are implemented inline.
//
//      This class was originally implemented by Kevin O'Neal in CoreObjs.h/cpp
//      in the SNMP directory.
//
//      The class was moved to the common directory and 'improved' by David
//      Pullen to support other uses.  I decided against renaming it because of
//      the rather large amount of existing code that uses it, and because this
//      isn't really a generic number class.  It's pretty much just designed
//      for counting.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "Counter64.h"

#include <string.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


ostream &BcmCounter64::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)

    char Str[32];

    // UNFINISHED - this could possibly be better...do we want to allow
    // decimal and hex output based on the current state of the ostream?
    sprintf (Str, "0x%08lX %08lX", High32(), Low32());
    outStream << Str;

#endif

    return outStream;
}


