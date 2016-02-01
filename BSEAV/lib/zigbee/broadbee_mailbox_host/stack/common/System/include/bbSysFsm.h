/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysFsm.h $
*
* DESCRIPTION:
*   Finite State Machines (FSM) engine interface.
*
* $Revision: 2364 $
* $Date: 2014-05-13 08:39:04Z $
*
****************************************************************************************/

#ifndef _BB_SYS_FSM_H
#define _BB_SYS_FSM_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Performs a transition corresponding to the specified event.
 */
#if !defined(_FSM_DEBUG_)
#   define SYS_FSM_HandleEvent(descr, eventId, data)   SYS_FSM_HandleEventInt(descr, eventId, data)
#else
#   define SYS_FSM_HandleEvent(descr, eventId, data) \
    { \
        SYS_DbgLogStr("Fsm:%s ", # descr); \
        SYS_FSM_HandleEventInt(descr, eventId, data); \
    }
#endif

/**//**
 * \brief Macro should be used to declare a state within transition table.
 *        The maximum amount of transitions which may be declared within one state
 *        declaration is 50.
 */
#define SYS_FSM_STATE(stateId, ...)    \
    SYS_FSM_HIERARCHICAL_STATE(stateId, SYS_FSM_SUPERSTATE_ID, __VA_ARGS__)

/**//**
 * \brief Macro should be used to declare a hierarchical state within transition table.
 * \note  Hierarchical states have a parent state, which event handlers are inherited by
 *        the hierarchical state. It means that if no transition for event is found
 *        within the hierarchical state the FSM engine will search for event handler
 *        among the transitions of its parent state.
 * \note  There is no restriction for inheritance level but parent state SHALL
 *        always be placed after its child state.
 * \note  The maximum amount of transitions which may be declared within one state
 *        declaration is 50.
 */
#if !defined(_FSM_DEBUG_)
#   define SYS_FSM_HIERARCHICAL_STATE(stateId, parentStateId, ...) \
    {                                     \
        (stateId) | SYS_FSM_STATE_FLAG,   \
        VA_NARGS(__VA_ARGS__)/VA_NARGS(SYS_FSM_TRN(e, g, a ,s)), \
        SYS_FSM_ILLEGAL_VALUE,            \
        parentStateId                     \
    },                                    \
    __VA_ARGS__
#else
#   define SYS_FSM_HIERARCHICAL_STATE(stateId, parentStateId, ...) \
    {                                     \
        (stateId) | SYS_FSM_STATE_FLAG,   \
        VA_NARGS(__VA_ARGS__)/VA_NARGS(SYS_FSM_TRN(e, g, a ,s)), \
        SYS_FSM_ILLEGAL_VALUE,            \
        parentStateId,                    \
        #stateId                          \
    },                                    \
    __VA_ARGS__
#endif

/**//**
 * \brief Macro declares a superstate which also is used as the End Of Table marker.
 *        Shall be placed at the very end of transition table.
 *        Shall be used once within the transition table declaration.
 *        If there is no necessity for superstate event handlers than the
 *        SYS_FSM_EOTT should be used for transition table termination.
 * \note  Superstate contains event handlers common for all other states of the FSM.
 *        Event handlers declared within the superstate declaration will be checked
 *        if no appropriate event handler is found within the current state handlers.
 */
#define SYS_FSM_SUPERSTATE(...) \
    SYS_FSM_STATE(SYS_FSM_SUPERSTATE_ID, __VA_ARGS__)

/**//**
 * \brief End Of Table declaration. Macro should be used for transition table termination.
 */
#if !defined(_FSM_DEBUG_)
#define SYS_FSM_EOTT           \
    {                          \
        SYS_FSM_SUPERSTATE_ID | SYS_FSM_STATE_FLAG, \
        0,                     \
        SYS_FSM_ILLEGAL_VALUE, \
        SYS_FSM_SUPERSTATE_ID  \
    }
#else
#define SYS_FSM_EOTT           \
    {                          \
        SYS_FSM_SUPERSTATE_ID | SYS_FSM_STATE_FLAG, \
        0,                     \
        SYS_FSM_ILLEGAL_VALUE, \
        SYS_FSM_SUPERSTATE_ID, \
        "EOTT" \
    }
#endif

/**//**
 * \brief Macro should be used to declare transition (event handler) within transition table.
 */
#if !defined(_FSM_DEBUG_)
#   define SYS_FSM_TRN(eventId, guardId, actionId, destStateId) \
    {               \
        (eventId),    \
        (guardId),    \
        (actionId),   \
        (destStateId) \
    }
#else
#   define SYS_FSM_TRN(eventId, guardId, actionId, destStateId) \
    {                \
        (eventId),     \
        (guardId),     \
        (actionId),    \
        (destStateId), \
        #eventId ":" #guardId "/" #actionId "->" #destStateId \
    }
#endif

/**//**
 * \brief Special value for all fields. Should not be used.
 */
#define SYS_FSM_ILLEGAL_VALUE       0xFF

/*********************** State Identifier related definitions **************************/

/**//**
 * \brief Checks if the state identifier is valid.
 */
#define SYS_FSM_IS_STATE_ID_VALID(stateId)  ((stateId) < SYS_FSM_SUPERSTATE_ID)

/**//**
 * \brief Special value for Destination State Id field. If it is used in transition
 *        declaration the current state won't be changed.
 * \note  Can be used by user when defining transition table, for example:
 *        SYS_FSM_TRN(SOME_EVENT, SOME_GUARD, SOME_ACTION, SYS_FSM_SAME_STATE)
 */
#define SYS_FSM_SAME_STATE          SYS_FSM_ILLEGAL_VALUE

/**//**
 * \brief Flag is used to mark the state declaration within transition table.
 *        Service value, not intended to be used by user.
 * \note  If the first byte of transition record contains value with the most
 *        significant bit set (in other words (firstByte & SYS_FSM_STATE_FLAG) == true)
 *        than this transition contains one of the service records:
 *            1. SYS_FSM_EOTT if all bytes has value of SYS_FSM_ILLEGAL_VALUE.
 *            2. Usual state declaration if state identifier (value of the first byte
 *               without MSB) is less than SYS_FSM_SUPERSTATE_ID.
 *            3. Superstate declaration if state identifier is equal to SYS_FSM_SUPERSTATE_ID.
 */
#define SYS_FSM_STATE_FLAG    0x80

/**//**
 * \brief Special value of State Id. Is used by the macro SYS_FSM_SUPERSTATE().
 *        Service value, not intended to be used by user.
 */
#define SYS_FSM_SUPERSTATE_ID       0x7F

/*********************** Event Identifier related definitions **************************/

/**//**
 * \brief Checks if the event identifier is valid.
 */
#define SYS_FSM_IS_EVENT_ID_VALID(eventId)  ((eventId) < (SYS_FSM_ILLEGAL_VALUE & ~SYS_FSM_STATE_FLAG))

/*********************** Guard Identifier related definitions **************************/

/**//**
 * \brief Checks if the guard identifier is valid.
 */
#define SYS_FSM_IS_GUARD_ID_VALID(guardId)  ((guardId) < SYS_FSM_ILLEGAL_VALUE)

/**//**
 * \brief Special value for Guard Id field. If it is used in transition declaration
 *        guard check will be omitted.
 */
#define SYS_FSM_OTHERWISE           SYS_FSM_ILLEGAL_VALUE

/**//**
 * \brief Special value for Guard Id field. If it is used in transition declaration
 *        guard check will be omitted.
 */
#define SYS_FSM_BY_DEFAULT          SYS_FSM_ILLEGAL_VALUE


/*********************** Action Identifier related definitions *************************/

/**//**
 * \brief Checks if the action identifier is valid.
 */
#define SYS_FSM_IS_ACTION_ID_VALID(actionId)  ((actionId) < SYS_FSM_ILLEGAL_VALUE)

/**//**
 * \brief Special value for Action Id field. If it used in transition declaration
  *       action will be omitted.
 */
#define SYS_FSM_DO_NOTHING          SYS_FSM_ILLEGAL_VALUE


/*********************** Macros for recursion protection. ******************************/
/**//**
 * \brief   Special value for the current FSM state code showing that FSM transition is
 *  in progress.
 * \details Used by FSM engine as mutex to protect FSM from reentry that may accidentally
 *  happen during a guard evaluation or action performing.
 */
#define IN_PROGRESS_STATE_FLAG          SYS_FSM_STATE_FLAG      /* 0x80 */

/**//**
 * \brief   Returns the FSM-in-progress flag state.
 * \param[in]   stateId     Current state identifier with FSM-in-progress mutex in the
 *  most significant bit.
 * \return  TRUE if FSM is currently in progress; FALSE otherwise.
 */
#define IS_IN_PROGRESS(stateId)         (0x0 != ((stateId) & IN_PROGRESS_STATE_FLAG))

/**//**
 * \brief   Sets the FSM-in-progress flag state to TRUE which means that from this moment
 *  FSM is considered busy.
 * \note    Flag is reset to FALSE automatically when current state is assigned with a
 *  destination state code.
 */
#define SET_IN_PROGRESS_BIT(stateId)    ((stateId) |= IN_PROGRESS_STATE_FLAG)

/************************* TYPES *******************************************************/

/**//**
 * \brief The type for storing State Id. The MSB is used for internal needs.
 *        Possible range: 0x00 - 0x7E.
 */
typedef uint8_t SYS_FSM_StateId_t;

/**//**
 * \brief The type for storing Event Id. The MSB is used for internal needs.
 *        Possible range: 0x00 - 0x7E.
 */
typedef uint8_t SYS_FSM_EventId_t;

/**//**
 * \brief The type for storing Guard Id. Possible range: 0x00 - 0xFE.
 */
typedef uint8_t SYS_FSM_GuardId_t;

/**//**
 * \brief The type for storing Guard Id. Possible range: 0x00 - 0xFE.
 */
typedef uint8_t SYS_FSM_ActionId_t;

/**//**
 * \brief The type for Guard check method.
 */
typedef bool (*SYS_FSM_GuardCheck_t)(SYS_FSM_GuardId_t guardId, void *data);

/**//**
 * \brief The type for Action method.
 */
typedef void (*SYS_FSM_Action_t)(SYS_FSM_ActionId_t actionId, void *data);

/**//**
 * \brief The type describes structure of transition.
 */
typedef struct _SYS_FSM_Transition_t
{
    SYS_FSM_EventId_t       eventId;        /*!< The Event Id which initiates transition. */
    SYS_FSM_GuardId_t       guardId;        /*!< The Guard Id which must be checked before transition. */
    SYS_FSM_ActionId_t      actionId;       /*!< The Id of action method. */
    SYS_FSM_StateId_t       destStateId;    /*!< The destination state Id which must be set
                                            after action execution. */
#if defined(_FSM_DEBUG_)
    const char *debugString;
#endif
} SYS_FSM_Transition_t;

/**//**
 * \brief The type describes structure of FSM Descriptor.
 */
typedef struct _SYS_FSM_Descriptor_t
{
    const SYS_FSM_Transition_t   *transitionTable;  /*!< The pointer to the transition table.   */
    SYS_FSM_GuardCheck_t          checkGuard;       /*!< The pointer to the Guard Check method. */
    SYS_FSM_Action_t              performAction;    /*!< The pointer to the Action method.      */
    SYS_FSM_StateId_t             currentState;     /*!< The current state of FSM.              */
} SYS_FSM_Descriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Performs a transition corresponding to the specified event.
  \param[in] descr - pointer to the FSM descriptor.
  \param[in] eventId - identifier of the event.
  \param[in] data - pointer to the additional data which may be used during transition.
****************************************************************************************/
void SYS_FSM_HandleEventInt(SYS_FSM_Descriptor_t *descr, SYS_FSM_EventId_t eventId, void *data);

/************************************************************************************//**
  \brief Resets fsm descriptor.
 * \param[in] fsmDescr              Pointer to the FSM descriptor.
 * \param[in] initialState          Code of the initial state of FSM.
 * \param[in] transitionsTable      Pointer to the first row of transitions table.
 * \param[in] guardsProvider        Entry point of guards provider function.
 * \param[in] actionsProvider       Entry point of actions provider function.
****************************************************************************************/
INLINE void SYS_FSM_Reset(SYS_FSM_Descriptor_t *const        fsmDescr,
                          const SYS_FSM_StateId_t            initialState,
                          const SYS_FSM_Transition_t *const  transitionsTable,
                          const SYS_FSM_GuardCheck_t         guardsProvider,
                          const SYS_FSM_Action_t             actionsProvider)
{
    SYS_DbgAssert(NULL != fsmDescr, HALT_SYS_FSM_Reset_NullFsmDescr);
    //SYS_DbgAssertLog(FALSE == IS_IN_PROGRESS(fsmDescr->currentState), WARN_SYS_FSM_Reset_FsmIsBusy);                  // TODO: Temporarily disabled due to contradiction with Unit-Tests 'System'.
    SYS_DbgAssert(SYS_FSM_IS_STATE_ID_VALID(initialState), HALT_SYS_FSM_Reset_InvalidInitialState);
    SYS_DbgAssert(NULL != transitionsTable, HALT_SYS_FSM_Reset_NullTransitionsTable);
    SYS_DbgAssert(NULL != guardsProvider, HALT_SYS_FSM_Reset_NullGuardsProvider);
    SYS_DbgAssert(NULL != actionsProvider, HALT_SYS_FSM_Reset_NullActionsProvider);

    fsmDescr->currentState    = initialState;
    fsmDescr->transitionTable = transitionsTable;
    fsmDescr->checkGuard      = guardsProvider;
    fsmDescr->performAction   = actionsProvider;
}

/************************************************************************************//**
 * \brief   Sets up FSM to the specified state without state transition processing.
 * \param[in]   fsm     Pointer to the FSM descriptor.
 * \param[in]   state   Numeric identifier of the state to set up the FSM.
****************************************************************************************/
INLINE void SYS_FSM_ChangeState(SYS_FSM_Descriptor_t *const fsm, const SYS_FSM_StateId_t state)
{
    SYS_DbgAssert(NULL != fsm, HALT_SYS_FSM_ChangeState_NullFsmDescr);
    SYS_DbgAssert(FALSE == IS_IN_PROGRESS(fsm->currentState), HALT_SYS_FSM_ChangeState_FsmIsBusy);
    SYS_DbgAssert(SYS_FSM_IS_STATE_ID_VALID(state), HALT_SYS_FSM_ChangeState_InvalidStoredState);

    fsm->currentState = state;
}

/************************************************************************************//**
 * \brief   Gets the current FSM state.
 * \param[in]   fsm     Pointer to the FSM descriptor.
 * \return  Code of the FSM current state.
****************************************************************************************/
INLINE SYS_FSM_StateId_t SYS_FSM_GetState(const SYS_FSM_Descriptor_t *const fsm)
{
    SYS_DbgAssert(NULL != fsm, HALT_SYS_FSM_GetState_NullFsmDescr);
    SYS_DbgAssert(FALSE == IS_IN_PROGRESS(fsm->currentState), HALT_SYS_FSM_GetState_FsmIsBusy);

    return fsm->currentState;
}

#endif /* _BB_SYS_FSM_H */