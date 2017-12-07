/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//  $Id$
//
//  Filename:       TimerMuxACT.cpp
//  Author:         Sean Dunlap 
//  Creation Date:  Feb 16, 2000
//
//****************************************************************************
//  Description:
//      Derived class BcmDsxTimerACT specializes the BcmCompletionHandlerACT 
//      base class for use with the TimerMux.  Refer to POSA2 book, page 268. 
//
//****************************************************************************
//

#include "TimerMuxACT.h"
#include "typedefs.h"
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iomanip>
#else
#include <iomanip.h>
#endif


// used when you want to specify the expiration time in the constructor.
// timerDuration is the time the timer should expire.  may wrap around.
BcmTimerMuxACT::BcmTimerMuxACT(unsigned long timerDuration) :
    BcmCompletionHandlerACT(),
    fExpirationTime( 0 ),
    fStartTime( 0 ),
    fTimerDuration( timerDuration )
{
}

BcmTimerMuxACT::BcmTimerMuxACT( const BcmTimerMuxACT& rhs )
  :
    BcmCompletionHandlerACT( rhs ),
    fExpirationTime( rhs.fExpirationTime ),
    fStartTime( rhs.fStartTime ),
    fTimerDuration( rhs.fTimerDuration )
{
}

BcmTimerMuxACT::~BcmTimerMuxACT()
{
}

BcmTimerMuxACT& BcmTimerMuxACT::operator=( const BcmTimerMuxACT& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    // call base class version.
    BcmCompletionHandlerACT::operator=( rhs );

    // copy derived class members.
    fExpirationTime = rhs.fExpirationTime;
    fStartTime = rhs.fStartTime;
    fTimerDuration = rhs.fTimerDuration;

    return *this;
}

bool BcmTimerMuxACT::operator==( const BcmTimerMuxACT& rhs ) const
{   
    bool ret_val = false;

    // call base class version.
    // note: base class has no data members and does not implement op==
//    ret_val = BcmCompletionHandlerACT::operator==( rhs );

    if( ret_val )
    {
        // base class objects are ==
        // consider derived class data members.
        if( (fExpirationTime == rhs.fExpirationTime)
            && (fStartTime == rhs.fStartTime)
			&& (fTimerDuration == rhs.fTimerDuration) )
        {
            ret_val = true;
        }
    }

    return ret_val;
}

ostream& BcmTimerMuxACT::Print( ostream& ostrm ) const
{
    #if (!BCM_REDUCED_IMAGE_SIZE)
    {
        ostrm 
            << "\n  BcmTimerMuxACT:"
            << "\n    fExpirationTime = " << dec << ExpirationTime();

        DerivedPrint(ostrm);
    }
    #endif
    return ostrm;
}

// derived class should override this to get a chance to add anything 
// application specific to the printout.
ostream& BcmTimerMuxACT::DerivedPrint( ostream& ostrm ) const
{
    #if (!BCM_REDUCED_IMAGE_SIZE)
    {
        ostrm << endl;
    }
    #endif
    
    return ostrm;
}

