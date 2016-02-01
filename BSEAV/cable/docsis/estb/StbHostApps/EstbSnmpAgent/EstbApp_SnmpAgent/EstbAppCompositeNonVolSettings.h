//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbAppCompositeNonVolSettings.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This class composites all of the individual settings groups together for
//      the eSTB SNMP Agent application.
//
//****************************************************************************

#ifndef ESTBAPPCOMPOSITENONVOLSETTINGS_H
#define ESTBAPPCOMPOSITENONVOLSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "BfcAppCompositeNonVolSettings.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmEstbAppCompositeNonVolSettings : public BcmBfcAppCompositeNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEstbAppCompositeNonVolSettings(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEstbAppCompositeNonVolSettings();

    // Returns the pointer to the singleton instance for this class.  Most
    // objects in the system will use this method rather than being passed a
    // pointer to it.  The singleton pointer will be set up in the base-class
    // constructor.
    //
    // NOTES:  This can return NULL if a singleton has not been set up for the
    //         system, so you must check for this condition.
    //
    //         You must not delete this object!
    //
    //         You should not store a pointer to the object that is returned,
    //         since it may be deleted and replaced with a new one.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the instance that should be used by the system.
    //
    static BcmEstbAppCompositeNonVolSettings *GetSingletonInstance(void);

protected:

private:

    // This is the singleton instance of this class.
    static BcmEstbAppCompositeNonVolSettings *pfSingletonInstance;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbAppCompositeNonVolSettings(const BcmEstbAppCompositeNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbAppCompositeNonVolSettings & operator = (const BcmEstbAppCompositeNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************



#endif


