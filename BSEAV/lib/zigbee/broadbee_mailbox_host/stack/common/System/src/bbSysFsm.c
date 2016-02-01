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
* FILENAME: $Workfile: trunk/stack/common/System/src/bbSysFsm.c $
*
* DESCRIPTION:
*   Finite State Machines (FSM) engine implementation.
*
* $Revision: 3878 $
* $Date: 2014-10-03 15:15:36Z $
*
****************************************************************************************/


/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"               /* Finite State Machines engine interface. */

/************************* DEFINITIONS *************************************************/

/* Macro for internal needs. Is used to determine if the transaction is the state declaration. */
#define IS_STATE_DECLARATION(entryPtr)                    \
    ((0 != ((entryPtr)->eventId & SYS_FSM_STATE_FLAG))    \
     && (SYS_FSM_ILLEGAL_VALUE != (entryPtr)->guardId)    \
     && (SYS_FSM_ILLEGAL_VALUE == (entryPtr)->actionId)   \
     && (SYS_FSM_ILLEGAL_VALUE != (entryPtr)->destStateId))

/* Macro for internal needs. Is used to determine if the transaction is the End Of Table declaration. */
#define IS_SUPERSTATE_DECLARATION(entryPtr)               \
    ((0 != ((entryPtr)->eventId & SYS_FSM_STATE_FLAG))    \
     && (SYS_FSM_SUPERSTATE_ID == GET_STATE_ID(entryPtr)) \
     && (SYS_FSM_ILLEGAL_VALUE != (entryPtr)->guardId)    \
     && (SYS_FSM_ILLEGAL_VALUE == (entryPtr)->actionId)   \
     && (SYS_FSM_SUPERSTATE_ID == (entryPtr)->destStateId))

/* Macro for internal needs. Is used to extract the state identifier from state declaration. */
#define GET_STATE_ID(entryPtr) ((entryPtr)->eventId & ~SYS_FSM_STATE_FLAG)
#define HAS_SAME_STATE(entryPtr, state) ((state) == GET_STATE_ID(entryPtr))

#if defined(_FSM_DEBUG_)
#   define FsmDbgStr SYS_DbgLogStr
#else
#   define FsmDbgStr(...)
#endif

#define ITERATE_ALL_EVENTS(state, iteratorName) \
    for (const SYS_FSM_Transition_t *iteratorName = state + 1; \
            iteratorName < state + (state->guardId + 1); \
            ++iteratorName)

/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
    \brief Helper function. Gets current FSM state identifier.
    \param[in] fsm - pointer to the FSM descriptor.
****************************************************************************************/
INLINE SYS_FSM_StateId_t sysFsmGetCurrentState(SYS_FSM_Descriptor_t *const fsm)
{
    SYS_FSM_StateId_t currentState = SYS_FSM_ILLEGAL_VALUE;

    /* Check the recursive protection flag */
    ATOMIC_SECTION_ENTER(SYS_FSM_START_EVENT_HANDLING)
    {
        SYS_DbgAssert(!IS_IN_PROGRESS(fsm->currentState), SYSFSM_RECURSION_DETECTED);
        currentState = fsm->currentState;
        SET_IN_PROGRESS_BIT(fsm->currentState);
    }
    ATOMIC_SECTION_LEAVE(SYS_FSM_START_EVENT_HANDLING)

    return currentState;
}

/************************************************************************************//**
    \brief Helper function. Sets current FSM state identifier.
    \param[in] fsm - pointer to the FSM descriptor.
****************************************************************************************/
INLINE void sysFsmSetCurrentState(SYS_FSM_Descriptor_t *const fsm, SYS_FSM_StateId_t currentState)
{
    /* Check the recursive protection flag */
    ATOMIC_SECTION_ENTER(SYS_FSM_FINISH_EVENT_HANDLING)
    {
        SYS_DbgAssert(IS_IN_PROGRESS(fsm->currentState), SYSFSM_IN_PROGRESS_BIT_IS_NOT_SET);
        fsm->currentState = currentState;
    }
    ATOMIC_SECTION_LEAVE(SYS_FSM_FINISH_EVENT_HANDLING)
}

/************************************************************************************//**
    \brief Helper function. Finds state declaration in transition table for the specified
           state identifier.
    \param[in] start - pointer to the start position within the transition table.
    \param[in] targetStateId - target state identifier.
    \return Pointer to the state declaration record.
****************************************************************************************/
INLINE const SYS_FSM_Transition_t *sysFsmFindStateDeclaration(const SYS_FSM_Transition_t *start,
                                                              SYS_FSM_StateId_t targetStateId)
{
    while (!HAS_SAME_STATE(start, targetStateId))
    {
        SYS_DbgAssert(IS_STATE_DECLARATION(start), SYS_FSM_INVALID_TRANSTION_TABLE_STRUCTURE00);
        SYS_DbgAssert(!IS_SUPERSTATE_DECLARATION(start), SYS_FSM_NO_APPROPRIATE_STATE00);
        start += start->guardId + 1;
    }

    SYS_DbgAssert(IS_STATE_DECLARATION(start), SYS_FSM_INVALID_TRANSTION_TABLE_STRUCTURE01);
    SYS_DbgAssert(HAS_SAME_STATE(start, targetStateId), SYS_FSM_NO_APPROPRIATE_STATE01);
    return start;
}

/************************************************************************************//**
    \brief Helper function. Checks guard.
    \param[in] fsm - pointer to the FSM descriptor.
    \param[in] entry - pointer to the FSM transition entry.
    \param[in] data - pointer to the additional data which may be used during transition.
****************************************************************************************/
INLINE bool sysFsmIsGuardPassed(SYS_FSM_Descriptor_t *const fsm, const SYS_FSM_Transition_t *const entry,
                               void *const data)
{
    if (SYS_FSM_OTHERWISE == entry->guardId)
        return true;

    SYS_DbgAssert(SYS_FSM_IS_GUARD_ID_VALID(entry->guardId), SYS_FSM_INVALID_GUARD_ID);
    return fsm->checkGuard(entry->guardId, data);
}

/************************************************************************************//**
    \brief Helper function. Finds a transition for the specified event.
    \param[in] fsm - pointer to the FSM descriptor.
    \param[in] state - pointer to the FSM state entry.
    \param[in] eventId - identifier of the event.
    \param[in] data - pointer to the additional data which may be used during transition.
    \return pointer to the found transition, NULL otherwise.
****************************************************************************************/
INLINE const SYS_FSM_Transition_t *sysFsmFindEventHandler(SYS_FSM_Descriptor_t *const fsm, const SYS_FSM_Transition_t *const state,
        const SYS_FSM_EventId_t eventId, void *const data)
{
    ITERATE_ALL_EVENTS(state, transition)
    {
        SYS_DbgAssert(!IS_STATE_DECLARATION(transition), SYS_FSM_UNEXPECTED_END_OF_TRANSITION_LIST);
        if (eventId == transition->eventId && sysFsmIsGuardPassed(fsm, transition, data))
        {
            FsmDbgStr("Transition: %s\n", transition->debugString);
            return transition;
        }
    }
    return NULL;
}

/************************************************************************************//**
    \brief Performs a transition corresponding to the specified event.
    \param[in] fsm - pointer to the FSM descriptor.
    \param[in] eventId - identifier of the event.
    \param[in] data - pointer to the additional data which may be used during transition.
****************************************************************************************/
void SYS_FSM_HandleEventInt(SYS_FSM_Descriptor_t *const fsm, const SYS_FSM_EventId_t eventId, void *const data)
{
    const SYS_FSM_StateId_t currentStateId = sysFsmGetCurrentState(fsm);
    const SYS_FSM_Transition_t *successTransition = NULL;

    SYS_DbgAssert(NULL != fsm, SYSFSM_NULL_POINTER_TO_FSM);
    SYS_DbgAssert(SYS_FSM_IS_EVENT_ID_VALID(eventId), SYSFSM_INVALID_EVENT_ID);
    SYS_DbgAssert(SYS_FSM_IS_STATE_ID_VALID(currentStateId), SYSFSM_INVALID_CURRENT_STATE_ID);

    {
        const SYS_FSM_Transition_t *stateEntry = fsm->transitionTable;
        SYS_FSM_StateId_t stateId = currentStateId;

        do
        {
            stateEntry = sysFsmFindStateDeclaration(stateEntry, stateId);
            successTransition = sysFsmFindEventHandler(fsm, stateEntry, eventId, data);
            stateId = stateEntry->destStateId;
        }
        while (NULL == successTransition && !IS_SUPERSTATE_DECLARATION(stateEntry));
    }

    SYS_DbgAssert(NULL != successTransition, SYSFSM_NO_APPROPRIATE_EVENT_HANDLER);
    SYS_DbgAssert(IMP(SYS_FSM_DO_NOTHING != successTransition->actionId,
                      SYS_FSM_IS_ACTION_ID_VALID(successTransition->actionId)),
                  SYS_FSM_INVALID_ACTION_ID);
    SYS_DbgAssert(IMP(SYS_FSM_SAME_STATE != successTransition->destStateId,
                      SYS_FSM_IS_STATE_ID_VALID(successTransition->destStateId)),
                  SYS_FSM_INVALID_DESTINATION_STATE_ID);

    if (SYS_FSM_DO_NOTHING != successTransition->actionId)
        fsm->performAction(successTransition->actionId, data);

    sysFsmSetCurrentState(fsm, (SYS_FSM_SAME_STATE == successTransition->destStateId) ?
                                    currentStateId : successTransition->destStateId);
}

/* eof bbSysFsm.c */