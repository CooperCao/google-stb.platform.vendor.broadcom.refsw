//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       BfcTargetFactory.cpp
//  Author:         David Pullen
//  Creation Date:  May 23, 2006
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "BfcTargetFactory.h"

#include "MessageLog.h"

#if (defined(WIN32) ||  defined(TARGETOS_Linux))

    #include "BfcStdHostTarget.h"

#elif defined(TARGETOS_vxWorks)

    #include "BfcStdEmbeddedTarget.h"

#elif defined(TARGETOS_eCos)

    #include "BfcStdEmbeddedTarget.h"

#endif

// For NULL...
#include <stdlib.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************



//
//
// UNFINISHED - this file should probably be in a target-specific directory,
//              rather than in the portable app directory.  The object that
//              needs to be created depends on the OS and target; we can either
//              do this via conditional compile stuff (which is what we have
//              right now), or by having a different file for each target.
//
//






// This method needs to create and return a pointer to the target object.
// This will be a class derived from BcmBfcTarget, which implements and/or
// overrides various methods to do target/platform-specific stuff.
//
// The caller becomes the owner of this object and must delete it when
// it is no longer needed.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the BFC target object.
//
BcmBfcTarget *BcmBfcTargetFactory::NewBfcTarget(void)
{
    BcmBfcTarget *pTarget = NULL;

    CallTrace("BcmBfcTargetFactory", "NewBfcTarget");

    #if (defined(WIN32) ||  defined(TARGETOS_Linux))
    
    // For Win32, use the standard host target object.
    pTarget = new BcmBfcStdHostTarget();

    #elif defined(TARGETOS_vxWorks)
    
    // For VxWorks, use the standard embedded target object.
    pTarget = new BcmBfcStdEmbeddedTarget();

    #elif defined(TARGETOS_eCos)
    
    // For VxWorks, use the standard embedded target object.
    pTarget = new BcmBfcStdEmbeddedTarget();

    #endif
    
    return pTarget;
}


