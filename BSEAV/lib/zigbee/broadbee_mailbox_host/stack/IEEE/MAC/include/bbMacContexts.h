/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacContexts.h $
*
* DESCRIPTION:
*   MAC Contexts interface.
*
* $Revision: 3402 $
* $Date: 2014-08-26 14:23:56Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_CONTEXTS_H
#define _BB_MAC_CONTEXTS_H


/************************* INCLUDES *****************************************************/
#include "bbMacOptions.h"           /* MAC options setup. */


/************************* DEFINITIONS **************************************************/
/*
 * Macros for unifying declarations and definitions of functions having MAC context
 * identifier argument or using active context identifier in the case of the dual context
 * MAC or the single context MAC.
 * For the dual context target the ContextId parameter is included into the function
 * arguments list and used as a parameter for function calls.
 * For the single context target the ContextId parameter is eliminated.
 */
#if defined(_MAC_DUAL_CONTEXT_)
#
/**//**
 * \brief MAC contexts identifiers enumeration.
 * \note This enumeration is defined only for the dual context MAC.
 */
typedef enum _MacContextId_t
{
    MAC_ZBPRO_CONTEXT_ID = 0,       /*!< ZigBee PRO stack context identifier. */
    MAC_RF4CE_CONTEXT_ID = 1,       /*!< RF4CE stack context identifier.      */
} MacContextId_t;
#
# define MAC_CONTEXTS_NUMBER        2       /*!< The total number of different contexts
                                                implemented in the MAC. */
#
# /* Macros for accessing to the special context-identifier variables:              */
# /* - MAC_GIVEN_CONTEXT_ID  - identifier of the context specified within           */
# /*                           the current function parameters list,                */
# /* - MAC_ACTIVE_CONTEXT_ID - identifier of the context of the currently active    */
# /*                           request processor.                                   */
# define MAC_GIVEN_CONTEXT_ID               __givenContextId
# define MAC_ACTIVE_CONTEXT_ID              __activeContextId
#
#
# /* Macros for describing the parameters list of a context-dependent function:     */
# /* - MAC_WITHIN_GIVEN_CONTEXT    - for the case when such a function has no other */
# /*                                 parameters except the context identifier,      */
# /* - MAC_WITH_GIVEN_CONTEXT(...) - for the case when a function has additional    */
# /*                                 parameters except the context identifier.      */
# define MAC_WITHIN_GIVEN_CONTEXT           MacContextId_t __givenContextId
# define MAC_WITH_GIVEN_CONTEXT(...)        MacContextId_t __givenContextId, __VA_ARGS__
#
#
# /* Macro for accessing to the context of the currently active request.            */
/* TODO: Eliminate precondition; verify all cases where it is used in the code. */
# define MAC_WITHIN_ACTIVE_CONTEXT          MacContextId_t __activeContextId = (    \
        (NULL != MAC_MEMORY_FE_ACTIVE_REQ()) ? (MAC_MEMORY_FE_ACTIVE_REQ_CONTEXT()) : 0)
# /* This macro must be the first line of the function body code block.             */
# /* Use semicolon at the end of this instruction.                                  */
#
#
# /* Macros for calling other context-dependent functions having a number of        */
# /* additional arguments for the specified context:                                */
# /* - MAC_GIVEN_CONTEXT  - for the context specified within the parameters list    */
# /*                        of the current function (the caller function),          */
# /* - MAC_ACTIVE_CONTEXT - for the context of the currently active request,        */
# /* - MAC_ZBPRO_CONTEXT  - for the ZigBee PRO stack context,                       */
# /* - MAC_RF4CE_CONTEXT  - for the RF4CE stack context,                            */
# /* - contextId          - for the context specified with the mentioned variable   */
# /*                        of the MacContextId_t data type.                        */
# define MAC_FOR_GIVEN_CONTEXT(...)         __givenContextId,     __VA_ARGS__
# define MAC_FOR_ACTIVE_CONTEXT(...)        __activeContextId,    __VA_ARGS__
# define MAC_FOR_ZBPRO_CONTEXT(...)         MAC_ZBPRO_CONTEXT_ID, __VA_ARGS__
# define MAC_FOR_RF4CE_CONTEXT(...)         MAC_RF4CE_CONTEXT_ID, __VA_ARGS__
# define MAC_FOR_CONTEXT(context, ...)      context,              __VA_ARGS__
# /* If the calling context-dependent function has no other parameters except       */
# /* the context identifier, then use context identifier macros directly:           */
# /* - MAC_GIVEN_CONTEXT_ID                                                         */
# /* - MAC_ACTIVE_CONTEXT_ID                                                        */
# /* - MAC_ZBPRO_CONTEXT_ID                                                         */
# /* - MAC_RF4CE_CONTEXT_ID                                                         */
# /* - MAC_CONTEXT_ID(contextId)                                                    */
#
#
# /* Macros for declaring and using custom context-identifier variables.            */
# /* - MAC_DECLARE_CONTEXT_ID - declares a context-identifier variable,             */
# /* - MAC_SET_CONTEXT_ID     - assigns the specified value to the given variable,  */
# /* - MAC_CMP_CONTEXT_ID     - compares two context identifier values,             */
# /* - MAC_IS_ZBPRO_CONTEXT   - returns TRUE if the given variable identifies       */
# /*                            the ZigBee PRO context.                             */
# /* - MAC_IS_RF4CE_CONTEXT   - returns TRUE if the given variable identifies       */
# /*                            the RF4CE context.                                  */
# define MAC_DECLARE_CONTEXT_ID(varName)        MacContextId_t varName
# define MAC_SET_CONTEXT_ID(varName, context)   (varName) = (context)
# define MAC_CONTEXT_ID(context)                (context)
# define MAC_CMP_CONTEXT_ID(varName, context)   ((varName) == (context))
# define MAC_IS_ZBPRO_CONTEXT(varName)          ((varName) == MAC_ZBPRO_CONTEXT_ID)
# define MAC_IS_RF4CE_CONTEXT(varName)          ((varName) == MAC_RF4CE_CONTEXT_ID)
#
#
# /* Macros for declaring and using custom contexts-set variables.                  */
# /* - MAC_DECLARE_CONTEXT_SET - declares a contexts-set variable,                  */
# define MAC_DECLARE_CONTEXTS_SET(varName)      BitField8_t varName
# define MAC_RESET_CONTEXTS_SET(varName)        (varName)    =  0x0
# define MAC_INCLUDE_CONTEXT(varName, context)  (varName)   |=  (1 << (context))
# define MAC_INCLUDE_ZBPRO_CONTEXT(varName)     (varName)   |=  (1 << MAC_ZBPRO_CONTEXT_ID)
# define MAC_INCLUDE_RF4CE_CONTEXT(varName)     (varName)   |=  (1 << MAC_RF4CE_CONTEXT_ID)
# define MAC_EXCLUDE_CONTEXT(varName, context)  (varName)   &= ~(1 << (context))
# define MAC_EXCLUDE_ACTIVE_CONTEXT(varName)    (varName)   &= ~(1 << (__activeContextId))
# define MAC_EXCLUDE_ZBPRO_CONTEXT(varName)     (varName)   &= ~(1 << MAC_ZBPRO_CONTEXT_ID)
# define MAC_EXCLUDE_RF4CE_CONTEXT(varName)     (varName)   &= ~(1 << MAC_RF4CE_CONTEXT_ID)
# define MAC_IS_FOR_CONTEXT(varName, context)   ((varName)  &   (1 << (context)))
# define MAC_IS_FOR_ACTIVE_CONTEXT(varName)     ((varName)  &   (1 << (__activeContextId)))
# define MAC_IS_FOR_ZBPRO_CONTEXT(varName)      ((varName)  &   (1 << MAC_ZBPRO_CONTEXT_ID))
# define MAC_IS_FOR_RF4CE_CONTEXT(varName)      ((varName)  &   (1 << MAC_RF4CE_CONTEXT_ID))
/* TODO: Simplify to (varName == 0x3); verify all cases where it is used in the code. */
# define MAC_IS_FOR_BOTH_CONTEXTS(varName)      (((varName) &  ((1 << MAC_ZBPRO_CONTEXT_ID) |   \
                                                                (1 << MAC_RF4CE_CONTEXT_ID)))   \
                                                            == ((1 << MAC_ZBPRO_CONTEXT_ID) |   \
                                                                (1 << MAC_RF4CE_CONTEXT_ID)))
/* TODO: Simplify to (varName == 0x0); verify all cases where it is used in the code. */
# define MAC_IS_FOR_NO_CONTEXTS(varName)        (((varName) &  ((1 << MAC_ZBPRO_CONTEXT_ID) |   \
                                                                (1 << MAC_RF4CE_CONTEXT_ID)))   \
                                                            ==   0x0                         )
# define MAC_WITHIN_GIVEN_CONTEXTS_SET          BitField8_t __givenContextsSet
# define MAC_WITH_GIVEN_CONTEXTS_SET(...)       BitField8_t __givenContextsSet, __VA_ARGS__
# define MAC_CONTEXTS_SET(contexts)             (contexts)
# define MAC_FOR_CONTEXTS_SET(contexts, ...)    context, __VA_ARGS__
# define MAC_GIVEN_CONTEXTS_SET                 __givenContextsSet
#
#
# /* Macros for accessing to the indexed context-specific variables.                */
# /* - MAC_ZBPRO_CONTEXT_IDX - index of the ZBPRO context array element,            */
# /* - MAC_RF4CE_CONTEXT_IDX - index of the RF4CE context array element.            */
# define MAC_GIVEN_CONTEXT_IDX                  MAC_GIVEN_CONTEXT_ID
# define MAC_ACTIVE_CONTEXT_IDX                 MAC_ACTIVE_CONTEXT_ID
# define MAC_CONTEXT_IDX(context)               MAC_CONTEXT_ID(context)
# define MAC_ZBPRO_CONTEXT_IDX                  MAC_ZBPRO_CONTEXT_ID
# define MAC_RF4CE_CONTEXT_IDX                  MAC_RF4CE_CONTEXT_ID
#
#
# /* Macros for arbitrary code inclusion in the case of dual context.               */
# define MAC_CODE_FOR_ZBPRO_CONTEXT(code)       do { code; } while(0)
# define MAC_CODE_FOR_RF4CE_CONTEXT(code)       do { code; } while(0)
# define MAC_CODE_FOR_DUAL_CONTEXT(code)        do { code; } while(0)
# define MAC_EXPR_FOR_ZBPRO_CONTEXT(expr)       (expr)
# define MAC_EXPR_FOR_RF4CE_CONTEXT(expr)       (expr)
# define MAC_EXPR_FOR_DUAL_CONTEXT(expr)        (expr)
# define MAC_PTR_FOR_ZBPRO_CONTEXT(ptr)         (ptr)
# define MAC_PTR_FOR_RF4CE_CONTEXT(ptr)         (ptr)
# define MAC_PTR_FOR_DUAL_CONTEXT(ptr)          (ptr)
#
#
#else /* _MAC_SINGLE_CONTEXT_ */
#
# define MAC_ZBPRO_CONTEXT_ID
# define MAC_RF4CE_CONTEXT_ID
# define MAC_CONTEXTS_NUMBER                    1
# define MAC_GIVEN_CONTEXT_ID
# define MAC_ACTIVE_CONTEXT_ID
# define MAC_WITHIN_GIVEN_CONTEXT               void
# define MAC_WITH_GIVEN_CONTEXT(...)            __VA_ARGS__
# define MAC_WITHIN_ACTIVE_CONTEXT
# define MAC_FOR_CONTEXT(context, ...)          __VA_ARGS__
# define MAC_FOR_GIVEN_CONTEXT(...)             __VA_ARGS__
# define MAC_FOR_ACTIVE_CONTEXT(...)            __VA_ARGS__
# define MAC_FOR_ZBPRO_CONTEXT(...)             __VA_ARGS__
# define MAC_FOR_RF4CE_CONTEXT(...)             __VA_ARGS__
# define MAC_DECLARE_CONTEXT_ID(varName)
# define MAC_SET_CONTEXT_ID(varName, context)   while(0)
# define MAC_CONTEXT_ID(varName)
# define MAC_CMP_CONTEXT_ID(varName, context)   true
# if defined(_MAC_CONTEXT_ZBPRO_)
#  define MAC_IS_ZBPRO_CONTEXT(varName)         true
#  define MAC_IS_RF4CE_CONTEXT(varName)         false
# elif defined(_MAC_CONTEXT_RF4CE_)
#  define MAC_IS_ZBPRO_CONTEXT(varName)         false
#  define MAC_IS_RF4CE_CONTEXT(varName)         true
# else
#  define MAC_IS_ZBPRO_CONTEXT(varName)         false
#  define MAC_IS_RF4CE_CONTEXT(varName)         false
# endif
# define MAC_GIVEN_CONTEXT_IDX                  0
# define MAC_ACTIVE_CONTEXT_IDX                 0
# define MAC_CONTEXT_IDX(context)               0
# define MAC_ZBPRO_CONTEXT_IDX                  0
# define MAC_RF4CE_CONTEXT_IDX                  0
# define MAC_DECLARE_CONTEXTS_SET(varName)
# define MAC_RESET_CONTEXTS_SET(varName)        while(0)
# define MAC_INCLUDE_CONTEXT(varName, context)  while(0)
# define MAC_INCLUDE_ZBPRO_CONTEXT(varName)     while(0)
# define MAC_INCLUDE_RF4CE_CONTEXT(varName)     while(0)
# define MAC_EXCLUDE_CONTEXT(varName, context)  while(0)
# define MAC_EXCLUDE_ACTIVE_CONTEXT(varName)    while(0)
# define MAC_EXCLUDE_ZBPRO_CONTEXT(varName)     while(0)
# define MAC_EXCLUDE_RF4CE_CONTEXT(varName)     while(0)
# define MAC_IS_FOR_CONTEXT(varName, context)   true
# define MAC_IS_FOR_ACTIVE_CONTEXT(varName)     true
# if defined(_MAC_CONTEXT_ZBPRO_)
#  define MAC_IS_FOR_ZBPRO_CONTEXT(varName)     true
#  define MAC_IS_FOR_RF4CE_CONTEXT(varName)     false
# elif defined(_MAC_CONTEXT_RF4CE_)
#  define MAC_IS_FOR_ZBPRO_CONTEXT(varName)     false
#  define MAC_IS_FOR_RF4CE_CONTEXT(varName)     true
# else
#  define MAC_IS_FOR_ZBPRO_CONTEXT(varName)     false
#  define MAC_IS_FOR_RF4CE_CONTEXT(varName)     false
# endif
# define MAC_IS_FOR_BOTH_CONTEXTS(varName)      false
# define MAC_IS_FOR_NO_CONTEXTS(varName)        true
# define MAC_WITHIN_GIVEN_CONTEXTS_SET          void
# define MAC_WITH_GIVEN_CONTEXTS_SET(...)       __VA_ARGS__
# define MAC_CONTEXTS_SET(contexts)
# define MAC_FOR_CONTEXTS_SET(contexts, ...)    __VA_ARGS__
# define MAC_GIVEN_CONTEXTS_SET
# if defined(_MAC_CONTEXT_ZBPRO_)
#  define MAC_CODE_FOR_ZBPRO_CONTEXT(code)      do { code; } while(0)
#  define MAC_CODE_FOR_RF4CE_CONTEXT(code)      while(0)
# elif defined(_MAC_CONTEXT_RF4CE_)
#  define MAC_CODE_FOR_ZBPRO_CONTEXT(code)      while(0)
#  define MAC_CODE_FOR_RF4CE_CONTEXT(code)      do { code; } while(0)
# else
#  define MAC_CODE_FOR_ZBPRO_CONTEXT(code)      while(0)
#  define MAC_CODE_FOR_RF4CE_CONTEXT(code)      while(0)
# endif
# define MAC_CODE_FOR_DUAL_CONTEXT(code)        while(0)
# if defined(_MAC_CONTEXT_ZBPRO_)
#  define MAC_EXPR_FOR_ZBPRO_CONTEXT(expr)      (expr)
#  define MAC_EXPR_FOR_RF4CE_CONTEXT(expr)      0
# elif defined(_MAC_CONTEXT_RF4CE_)
#  define MAC_EXPR_FOR_ZBPRO_CONTEXT(expr)      0
#  define MAC_EXPR_FOR_RF4CE_CONTEXT(expr)      (expr)
# else
#  define MAC_EXPR_FOR_ZBPRO_CONTEXT(expr)      0
#  define MAC_EXPR_FOR_RF4CE_CONTEXT(expr)      0
# endif
# define MAC_EXPR_FOR_DUAL_CONTEXT(expr)        0
# if defined(_MAC_CONTEXT_ZBPRO_)
#  define MAC_PTR_FOR_ZBPRO_CONTEXT(ptr)        (ptr)
#  define MAC_PTR_FOR_RF4CE_CONTEXT(ptr)        NULL
# elif defined(_MAC_CONTEXT_RF4CE_)
#  define MAC_PTR_FOR_ZBPRO_CONTEXT(ptr)        NULL
#  define MAC_PTR_FOR_RF4CE_CONTEXT(ptr)        (ptr)
# else
#  define MAC_PTR_FOR_ZBPRO_CONTEXT(ptr)        NULL
#  define MAC_PTR_FOR_RF4CE_CONTEXT(ptr)        NULL
# endif
# define MAC_PTR_FOR_DUAL_CONTEXT(ptr)          NULL
#
#endif /* _MAC_SINGLE_CONTEXT_ */


/************************* EXAMPLES OF USAGE ********************************************/
/*
 * All the examples below are able to be compiled for the dual context and for the single
 * context environment without any kind of conditional preprocessing, i.e. there is no
 * need to use '#if defined(_MAC_DUAL_CONTEXT_)' in the code for them.
 *
 * For the case of dual contexts build the compiler automatically adds the ContextId
 * parameter to all the context-specific functions parameters lists.
 *
 * For the case of single context build the compiler eliminates all context-specific
 * instructions, i.e. no useless code is generated.
 *
 */


/*
 * Example of a function receiving the context-identifier and using its value to call
 * another context-specific function that receives additional parameters:
 *
 *  void contextProcessCurrentTime(MAC_WITHIN_GIVEN_CONTEXT)
 *  {
 *      Date_t date = getCurrentDate();
 *      Time_t time = getCurrentTime();
 *      contextSetDateTime(MAC_FOR_GIVEN_CONTEXT(date, time));
 *  }
 *
 */


/*
 * Example of a function receiving the context-identifier and some additional parameters
 * and using their values to call another context-specific functions having none or a
 * number of additional parameters:
 *
 *  void contextSetDateTime(MAC_WITH_GIVEN_CONTEXT(Date_t date, Time_t time))
 *  {
 *      int timestamp = makeTimestamp(date, time);
 *      contextSetTimestamp(MAC_FOR_GIVEN_CONTEXT(timestamp));
 *      contextActivateAlarms(MAC_GIVEN_CONTEXT_ID);
 *  }
 *
 */


/*
 * Example of a function using the context of the currently active request to obtain some
 * properties specific to the active context:
 *
 *  bool setActiveContextPanId(PanId_t panId)
 *  {
 *      MAC_WITHIN_ACTIVE_CONTEXT;
 *      if (isAssociated(MAC_ACTIVE_CONTEXT_ID))
 *          return false;
 *      else
 *      {
 *          Variant_t attrValue = { .macPANId = panId };
 *          setMacAttribute(MAC_FOR_ACTIVE_CONTEXT(MAC_ATTR_PAN_ID, &attrValue));
 *      }
 *      return true;
 *  }
 *
 */


/*
 * Example of function calls for contexts specified directly:
 *
 *  getMacAttribute(MAC_FOR_ZBPRO_CONTEXT(MAC_ATTR_SHORT_ADDRESS));
 *  getExtendedAddress(MAC_RF4CE_CONTEXT_ID);
 *
 */


/*
 * Example of declaring and using custom context-identifier variables:
 *
 *  MAC_DECLARE_CONTEXT_ID(myContextId);                    //  MacContextId_t myContextId;
 *  MAC_SET_CONTEXT_ID(myContextId, otherContextId);        //  myContextId = otherContextId;
 *
 *  if (MAC_CMP_CONTEXT_ID(myContextId, otherContextId))    //  if (myContextId == otherContextId)
 *
 *  if (MAC_IS_ZBPRO_CONTEXT(myContextId))                  //  if (myContextId == MAC_ZBPRO_CONTEXT_ID)
 *
 *  getExtendedAddress(MAC_CONTEXT_ID(myContextId));        //  getExtendedAddress(myContextId);
 *
 *  setTime(MAC_FOR_CONTEXT(myContextId, time));            //  setTime(myContextId, time);
 *
 */


/*
 * Example of declaring and using context-specific variables:
 *
 *  Addr16bit_t macShortAddress[MAC_CONTEXTS_NUMBER] =      //  Addr16bit_t macShortAddress[2] =
 *  {                                                       //  {
 *  #if defined(_ZBPRO_)                                    //  #
 *      [MAC_ZBPRO_CONTEXT_IDX] = 0xFFFF,                   //      [0] = 0xFFFF,
 *  #endif                                                  //  #
 *  #if defined(_RF4CE_)                                    //  #
 *      [MAC_RF4CE_CONTEXT_IDX] = 0xFFFF,                   //      [1] = 0xFFFF,
 *  #endif                                                  //  #
 *  };                                                      //  };
 *
 *  return macShortAddress[MAC_GIVEN_CONTEXT_IDX];          //  return macShortAddress[__givenContextId];
 *
 *  CbDataConf[MAC_ACTIVE_CONTEXT_IDX](reqDescr, confPrm);  //  CbDataConf[__activeContextId](reqDescr, confPrm);
 *
 *  CbDataInd[MAC_CONTEXT_IDX(myContextId)](indPrm);        //  CbDataInd[myContextId](indPrm);
 *
 */


#endif /* _BB_MAC_CONTEXTS_H */