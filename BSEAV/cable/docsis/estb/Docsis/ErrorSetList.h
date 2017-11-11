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
// 
//  $Id: ErrorSetList.h 1.3 2003/12/04 16:07:09Z msieweke Release $ 
// 
//  Filename:       ErrorSetList.h 
//  Author:         [author] 
//  Creation Date:  [date] 
// 
//**************************************************************************** 
//  Description: 
//       
//       
//       
//       
// 
//**************************************************************************** 
//
// ErrorSetList.h
//
#ifndef ERRORSETLIST_H
#define ERRORSETLIST_H

#include "ErrorSet.h"
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <list>
#else
#include <list.h>
#endif

typedef list< BcmErrorSet > ErrorSetList;
typedef ErrorSetList::iterator ErrorSetListIterator;
typedef ErrorSetList::const_iterator ErrorSetListConstIterator;


// ParseErrorSet - Load a new BcmErrorSet object from the specified 
//      buffer at the specified offset and length.  Add the resulting 
//      BcmErrorSet object to the specified ErrorSetList object.
//
// Parameters:
//
//      obuf - BcmOctetBuffer& from which the new object is to be loaded.
//
//      offset - unsigned int& offset into obuf where data for the object 
//          begins.
//
//      length - number of bytes to read from obuf.
//
//      error_sets_list - ErrorSetsList to which the new object is
//          to be added.
//
//      log_sets - BcmMessageLogSettings used to control debug output.
//
// Returns:
//      bool - true if successful, otherwise false.  obuf and offset
//          reference params are updated by this function.
//
bool ParseErrorSet( const BcmOctetBuffer &obuf, unsigned int &offset,
    uint8 length, ErrorSetList& error_set_list, 
    BcmMessageLogSettings* log_sets = NULL );


#endif

