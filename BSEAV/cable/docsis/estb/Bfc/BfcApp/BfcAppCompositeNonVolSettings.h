//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  $Id: BfcAppCompositeNonVolSettings.h 1.3 2004/04/14 21:49:56Z dpullen Release $
//
//  Filename:       BfcAppCompositeNonVolSettings.h
//  Author:         David Pullen
//  Creation Date:  June 11, 2003
//
//****************************************************************************

#ifndef BfcAppCompositeNonVolSettings_H
#define BfcAppCompositeNonVolSettings_H

//********************** Include Files ***************************************

// My base class.
#include "CompositeNonVolSettings.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


/** \ingroup BfcCore BfcNonVol
*
*   This class composites all of the individual settings groups together for
*   the BFC-based application.
*/
class BcmBfcAppCompositeNonVolSettings : public BcmCompositeNonVolSettings
{
public:

    /// Default Constructor.  Initializes the state of the object, setting all
    /// fields to good default values.
    ///
    BcmBfcAppCompositeNonVolSettings(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBfcAppCompositeNonVolSettings();

    /// Returns the pointer to the singleton instance for this class.  Most
    /// objects in the system will use this method rather than being passed a
    /// pointer to it.  The singleton pointer will be set up in the base-class
    /// constructor.
    ///
    /// \note   This can return NULL if a singleton has not been set up for the
    ///         system, so you must check for this condition.
    ///
    /// \note   You must not delete this object!
    ///
    /// \note   You should not store a pointer to the object that is returned,
    ///         since it may be deleted and replaced with a new one.
    ///
    /// \return
    ///      A pointer to the instance that should be used by the system.
    ///
    static BcmBfcAppCompositeNonVolSettings *GetSingletonInstance(void);

private:

    /// This is the singleton instance of this class.
    static BcmBfcAppCompositeNonVolSettings *pgSingletonInstance;

private:

    // Copy Constructor.  Not supported.
    BcmBfcAppCompositeNonVolSettings(const BcmBfcAppCompositeNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    BcmBfcAppCompositeNonVolSettings & operator = (const BcmBfcAppCompositeNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************



#endif


