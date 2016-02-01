//
//  Copyright (c) 2005  Broadcom Corporation
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
//  Filename:       DsgAppName.h
//  Author:         Maurice Turcotte
//  Creation Date:  February 2, 2005
//
//****************************************************************************
//  Description:
//      This object maps Application Names to Application IDs.
//
//
//****************************************************************************

#ifndef DSGAPPNAME_H
#define DSGAPPNAME_H

//********************** Include Files ***************************************

// For the portable types.                        
#include "typedefs.h"
#include <iostream>

// For the byte swapping functions.
// #include "Utilities.h"

#include <string.h>
#include "BcmString.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************



class BcmDSGAppName
{
public:

    // Default Constructor.  
    //
    // Parameters:
    //      pBuffer - pointer to the memory buffer containing the text application name.
    //      bufferSize - the number of bytes in the buffer.
    //      appId - the integral application ID to which this name maps.
    //
    // Returns:  N/A
    //
    BcmDSGAppName(uint8 *pBuffer, int bufferSize, int appId);

    // Alternate Constructor.  
    //
    // Parameters:
    //      pString - pointer to a BcmString with the application name.
    //      appId - the integral application ID to which this name maps.
    //
    // Returns:  N/A
    //
    BcmDSGAppName(BcmString *appString, int appId);

    // Destructor.  If the buffer was created by this class, or if it was given
    // ownership of a buffer, then the buffer will be deleted.  If a buffer was
    // given to this class but it was not told to own the buffer, then it just
    // "forgets" the pointer to the buffer without deleting it.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDSGAppName();

    bool operator == (const char *s) const; 
    bool operator == (const BcmDSGAppName &appName) const; 
    bool operator == (const BcmString &appNameString) const; 

    inline int  AppId() {return fAppId;};
    inline bool AppNameMatch(const char *s) {return (*pfAppNameString == s);};
    inline bool AppNameMatch(const BcmString &s) {return (s == *pfAppNameString);};
    inline void Dump() {printf("%s -> %d\n", pfAppNameString->c_str(), fAppId);};
    inline bool IsValid() {return isValid;};

protected:

private:

    BcmString *pfAppNameString;
    unsigned int fAppId;
    bool isValid;
};


#endif


