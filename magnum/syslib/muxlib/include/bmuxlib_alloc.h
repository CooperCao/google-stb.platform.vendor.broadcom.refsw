/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef MAGNUM_SYSLIB_MUXLIB_INCLUDE_BMUXLIB_ALLOC_H_
#define MAGNUM_SYSLIB_MUXLIB_INCLUDE_BMUXLIB_ALLOC_H_

#define BMUXLIB_P_MAX_ALLOC_SIZE 4096

#define BMUXLIB_P_ENTRY_PER_BLOCK(_entryType, _maxAllocSize) ((_maxAllocSize)/sizeof(_entryType))
#define BMUXLIB_P_ENTRY_BLOCK_COUNT(_entryType, _entryCount, _maxAllocSize) ( ( _entryCount + ( BMUXLIB_P_ENTRY_PER_BLOCK(_entryType,_maxAllocSize) - 1 ) ) / BMUXLIB_P_ENTRY_PER_BLOCK(_entryType,_maxAllocSize) )

#define BMUXLIB_P_ENTRY_TYPE( _entryType, _pEntryList )\
      _entryType **_pEntryList##Block;\
      _entryType **_pEntryList;

#define BMUXLIB_P_ENTRY_ALLOCATE( _entryType, _pEntryList, _entryCount, _errorLabel ) \
{\
   unsigned _i,_j;\
   unsigned _uiNumDescriptors = 0;\
\
   _pEntryList = BMUXlib_Malloc( sizeof(_entryType*) * _entryCount );\
   if(!_pEntryList)\
   {\
      goto _errorLabel;\
   }\
   BKNI_Memset(_pEntryList, 0, sizeof(_entryType*) * _entryCount  );\
\
   _pEntryList##Block = BMUXlib_Malloc(sizeof(_entryType*) * BMUXLIB_P_ENTRY_BLOCK_COUNT(_entryType, _entryCount, BMUXLIB_P_MAX_ALLOC_SIZE) );\
   if(!_pEntryList##Block)\
   {\
      goto _errorLabel;\
   }\
   BKNI_Memset(_pEntryList##Block, 0, sizeof(_entryType*) * BMUXLIB_P_ENTRY_BLOCK_COUNT(_entryType, _entryCount, BMUXLIB_P_MAX_ALLOC_SIZE) );\
\
   for ( _i = 0; _i < BMUXLIB_P_ENTRY_BLOCK_COUNT(_entryType, _entryCount, BMUXLIB_P_MAX_ALLOC_SIZE); _i++ )\
   {\
      _entryType *pstEntry = (_pEntryList##Block)[_i] = ( _entryType * ) BMUXlib_Malloc( BMUXLIB_P_MAX_ALLOC_SIZE );\
\
      if (!(_pEntryList##Block)[_i] )\
      {\
         goto _errorLabel;\
      }\
\
      BKNI_Memset( (_pEntryList##Block)[_i], 0, BMUXLIB_P_MAX_ALLOC_SIZE );\
\
      for ( _j = 0; ( _j < BMUXLIB_P_ENTRY_PER_BLOCK(_entryType, BMUXLIB_P_MAX_ALLOC_SIZE) ) && ( _uiNumDescriptors < _entryCount ); _j++ )\
      {\
         (_pEntryList)[_uiNumDescriptors++] = pstEntry++;\
      }\
   }\
   BDBG_ASSERT( _uiNumDescriptors == _entryCount );\
}\

#define BMUXLIB_P_ENTRY_FREE( _entryType, _pEntryList, _entryCount ) \
if (_pEntryList)\
{\
   unsigned _i;\
\
   if(_pEntryList##Block)\
   {\
      for ( _i = 0; _i < BMUXLIB_P_ENTRY_BLOCK_COUNT(_entryType, _entryCount, BMUXLIB_P_MAX_ALLOC_SIZE); _i++ )\
      {\
         if ((_pEntryList##Block)[_i])\
         {\
            BKNI_Free((_pEntryList##Block)[_i]);\
            (_pEntryList##Block)[_i] = 0;\
         }\
      }\
      BKNI_Free(_pEntryList##Block);\
      (_pEntryList##Block) = 0;\
   }\
   BKNI_Free(_pEntryList);\
   (_pEntryList) = 0;\
}

#define BMUXLIB_P_CONTEXT_ALLOCATE( _contextType, _pContext, _errorLabel )\
{\
   (_pContext) = (_contextType*)BMUXlib_Malloc(sizeof(_contextType));\
   if (!(_pContext))\
   {\
      goto _errorLabel;\
   }\
   else\
   {\
      BKNI_Memset( (_pContext) , 0, sizeof(_contextType) );\
   }\
}

#define BMUXLIB_P_CONTEXT_FREE( _pContext )\
if(_pContext)\
{\
   BKNI_Free(_pContext);\
   (_pContext) = 0;\
}

#endif /* MAGNUM_SYSLIB_MUXLIB_INCLUDE_BMUXLIB_ALLOC_H_ */
