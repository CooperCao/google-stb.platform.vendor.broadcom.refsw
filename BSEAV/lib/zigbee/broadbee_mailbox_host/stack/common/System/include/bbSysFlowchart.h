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
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   Flowchart toolset interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_SYS_FLOWCHART_H
#define _BB_SYS_FLOWCHART_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Set of macros for defining flowcharts.
 * \details The following macros are defined:
 *  - FC_DECL(fcName) - Declares a flowchart function \p fcName.
 *  - fc_run(fcName, pEvents) - Activates the flowchart function \p fcName for the set of events \p pEvents.
 *  - FC_FUNC(fcName) - Opens definition of the flowchart function \p fcName.
 *  - FC_STEPS(stepsList) - Defines enumeration of steps identifiers listed in the \p stepsList.
 *  - FC_START - Opens definition of the flowchart START step.
 *  - FC_STEP(stepName) - Opens definition of a step \p stepName within the flowchart.
 *  - FC_SUB(stepName, subFcName) - Opens definition of a step \p stepName and calls the subflowchart \p subFcName.
 *  - fc_goto(dstStepName) - Jumps to the step \p dstStepName within the flowchart and continues in the same execution
 *      flow.
 *  - fc_post(dstStepName, sensEventsSet) - Jumps to the step \p dstStepName within the flowchart and postpones its
 *      execution returning the sensitivity events set \p sensEventsSet.
 *  - fc_end - Finishes execution of the flowchart.
 *  - FC_END - Closes definition of a flowchart function, finishes its execution if reached.
 */
/**@{*/
/**//**
 * \brief   Declares a flowchart function.
 * \param[in]   fcName      The flowchart name. A globally unique identifier or a numeric constant.
 * \details A flowchart is defined in the form of FUNCTION. One function defines a single flowchart. For more details
 *  see definition of the flowchart function.
 * \details Generally it's not necessary to DECLARE the flowchart function - indeed, its enough just to define it. The
 *  flowchart function must be declared in advance if the corresponding flowchart will be used as a subflowchart and
 *  such a subflowchart (the callee flowchart) definition may not be given in the same translation unit (source file)
 *  above the definition of the caller flowchart - in the same way as with conventional functions.
 * \note    Normally a flowchart function is declared with external linkage, but it's allowed to declare it with
 *  internal linkage (make it static), or even make it inline. To modify the linkage of the flowchart function, put the
 *  corresponding attribute infront of the DECL clause.
 */
#define FC_DECL(fcName)\
        uint32_t fcName(uint32_t *const pEvents)

/**//**
 * \brief   Activates the specified flowchart function.
 * \param[in]       fcName      The flowchart name. A globally unique identifier or a numeric constant.
 * \param[in/out]   pEvents     The pointer to the set of triggered (pending) events on which the flowchart is
 *  subscribed. The flowchart function may reset particular (or all) events in this set prior to return on its own
 *  discretion. Zero value or NULL pointer are allowed if there is no need to specify the set of pending events.
 * \return  The nonzero 32-bit mask of events to which the paused flowchart wishes to stay/become sensitive; or zero if
 *  the flowchart has finished its work.
 * \details A flowchart is defined in the form of FUNCTION. One function defines a single flowchart. For more details
 *  see definition of the flowchart function.
 * \details To activate (start or resume execution of) a particular flowchart, its function must be RUN with the help of
 *  this macro. Normally the flowchart function is called when a new event, to which the flowchart is currently
 *  sensitive, triggers - the calling application is responsible for holding all the pending events and filtering them
 *  with applying particular sensitivity mask when activating a flowchart.
 * \details When the flowchart function is called, the caller forwards (by pointer) the set of triggered/pending events
 *  to which the flowchart is currently sensitive (according to the events sensitivity mask returned by its function
 *  last time).
 * \note    Initially (and also on the first activation after restart) the flowchart is not sensitive to any events -
 *  i.e., its sensitivity mask equals 0x0 (zero). To make the flowchart sensitive to events, in the general case, one
 *  shall run it once and wait for the flowchart until it pauses its execution (until its function returns with the
 *  nonzero sensitivity mask). However, this rule may be redefined in particular cases according to the application's
 *  needs. For example, one may introduce a kind of nonmaskable event that is delivered to the flowchart function
 *  unconditionally each time it is called, including the very first initial run or the first run after restart; or the
 *  initial events sensitivity mask may be preassigned with a different value but not 0x0. Nevertheless, even in this
 *  cases, if a flowchart has finished its execution, its function shall return 0x0 (zero) as the status of the finished
 *  flowchart.
 * \details The flowchart function may reset particular (or all) events in the set prior to return - normally it
 *  clears all the processed or dropped (ignored or intercepted) events. Anyway, the application has to (1) report all
 *  the events, left active by the callee, again when the flowchart function is called next time (or the next flowchart
 *  in the queue is called), and (2) clear all the reset events and do not report them again when the flowchart function
 *  is called next time (or the next flowchart in the queue is activated), except the case if they triggered again
 *  during the period between the last and the next activations of the flowchart (the flowcharts queue).
 * \note    If a flowchart waits for particular combination of events (for example, joined with AND) it may repause its
 *  execution multiple times without clearing the signaled events until the required combination of events is achieved,
 *  and only at this time to process (and clear) all of them.
 * \note    The application and its flowcharts may be designed in two different ways with respect to the way how the
 *  set of triggered/pending events is filtered for the flowchart being called and how the flowchart clears the
 *  processed events:
 *  A) Prior to call a flowchart the application itself splits the global set of pending events into two disjoint parts
 *  according to the flowchart current sensitivity mask: events that will be delivered to the called flowchart, and
 *  events that will not be delivered to it - and passes (by pointer) only the first subset to the flowchart. The
 *  flowchart may clear particular events in this subset and return. Finally application combines the first (partially
 *  cleared) subset of events that were delivered to the flowchart with the second (unchanged) subset; their junction
 *  produces the set of remaining events. The application may also apply the original sensitivity mask on the first
 *  subset of events right after the flowchart returned, just in order to prohibit potentially possible settings of
 *  phantom events by the called flowchart.
 *  B) The application passes (by pointer) the whole set of events to the flowchart. In this case the flowchart is fully
 *  responsible for careful handling of the global set of events.
 *
 *  The variant A is more safe, because the called flowchart will not have a chance to react on or clear events outside
 *  of its sensitivity mask, or to set phantom events; while the variant B is significantly faster. Anyway, in both
 *  cases the flowchart should be called only if there are events to which it's currently sensitive (just in order to
 *  avoid unnecessary function calls).
 * \details If there is a queue of flowcharts is arranged, the set of currently active events is delivered to the first
 *  flowchart in the queue. After its function returned, the rest of events (that were not cleared by the first
 *  flowchart) is delivered to the next flowchart in the queue (if it is subscribed at least on one of these events).
 *  Between calls of flowcharts in the queue no new events are added to the set; all the newly triggered events are
 *  added to the set only prior to run the flowcharts queue again from its head.
 * \note    If there is a queue of flowcharts is arranged, the first flowchart in the queue (it has the topmost priority
 *  over others) may screen particular events from being delivered to the next flowcharts (having lower priority than
 *  the first one) - i.e. to intercept and drop the event, even if the first flowchart does not process such event. By
 *  these means the first flowchart is able to regulate activity of the subsequent flowcharts.
 * \note    It's strongly not recommended to set phantom events in the set of events, for example, as a kind of cookie
 *  or flag for selfnotifying in the next activation period or for interflowchart communication. For these purposes,
 *  different instruments must be used.
 * \details A flowchart function is running until the flowchart reaches its END terminator or POSTPONES its execution.
 *  In the first case the flowchart function returns zero, which generally means that the ended flowchart does not wait
 *  for events anymore; and in the second case it returns the nonzero 32-bit mask of events to which the postponed
 *  flowchart wishes to stay/become sensitive.
 * \note    If the subscription feature is not used by the application (and all events are delivered to the flowchart
 *  unconditionally), or if there is only one event in the system, or if there are no events in the system at all,
 *  anyway, the paused flowchart function must return a nonzero value as the 32-bit events sensitivity mask. In the
 *  mentioned case it may be recommended to return at least the value 0x1. And it has to return 0x0 (zero) in the case
 *  when the flowchart has reached the END terminator.
 */
#define fc_run(fcName, pEvents)\
        fcName(pEvents)
/* IDEA: Support the CONTEXT and/or VOID DATA transferring into and outside the flowchart.
 *      Currently a flowchart may communicate with the application and its parent and child flowcharts only with the
 *  help of shared static memory. It's better to allow transferring the CONTEXT by pointer between these levels.
 */
/* IDEA: Support the RESET signal.
 *      On the RESET signal the flowchart must: (1) if it's currently in the 'transparent' state propagating all calls
 *  to its subflowchart, it has to forward the RESET signal to the called subflowchart first, and then, when the
 *  subflowchart returned, process the RESET signal itself, (2) process the RESET signal immediately, terminating all
 *  activities, and finish its execution.
 */

/**//**
 * \brief   Opens definition of the flowchart function.
 * \param[in]   fcName      The flowchart name. A globally unique identifier or a numeric constant.
 * \details A flowchart is defined in the form of FUNCTION. One function defines a single flowchart. For more details on
 *  how to call the flowchart function, on its parameters and the returned value see the RUN operator definition.
 * \details After the flowchart function definition opening macro, the START step must follow, then a number of common
 *  steps may be defined, and finally the flowchart function definition must be closed with the END terminator.
 * \details A flowchart function defines local static variable holding the execution cursor of the flowchart. The
 *  execution cursor is used (automatically) for resuming the flowchart execution from the step where its execution was
 *  previously paused. Initially and after the flowchart was ended, the execution cursor points to the START step (it
 *  has the numeric identifier zero).
 * \note    The execution cursor may not be set on an arbitrary place somewhere in the middle of a flowchart step, but
 *  only on the entry point of a step. If one needs to postpone the flowchart in the middle of a step, the corresponding
 *  step must be divided in two (or more) separate (sequential) steps each formally having its own entry point (i.e.,
 *  unique name or numeric identifier).
 * \note    A flowchart may have only one execution cursor - i.e., it may have only one instance. Multiple execution
 *  instances of a single flowchart are not supported.
 * \details One may put the opening brace right after this macro to emphasize the limits of the flowchart definition.
 *  Still, it's not necessary. If the opening brace is used, then the closing brace must be put after the END terminator
 *  clause.
 * \details If necessary, one may put declarations (and definitions) of additional variables local to the flowchart.
 *  They will be shared by all the steps within such a flowchart. Variables may be declared/defined either automatic or
 *  static. Such declarations must follow the FUNC clause (or the opening brace after it) immediately.
 * \details If necessary, one may put additional operators right after the flowchart function opening clause FUNC (or
 *  the opening brace after it). Such operators will be executed once each time the flowchart function is activated.
 *  Formally they are not considered as a part of the flowchart, just because they are not included in one of the
 *  flowchart steps, so it's not recommended to perform the mainstream activities with them. Still, they may perform
 *  auxiliary (supplementary) operations, for example, validate the passed events set, check extended memory, evaluate
 *  expressions shared by different steps within the flowchart.
 * \note    It's strongly not recommended to put heavy calculations into the section of auxiliary operators, because
 *  they are executed each time the flowchart is activated even if it's in the 'transparent' condition forwarding the
 *  execution context to one of its subflowcharts paused its execution last time.
 * \details It's recommended to put a local enumeration of all custom flowchart steps identifiers after the FUNC clause.
 *  It's strongly recommended to use the STEPS macro for this instead of defining the enumeration directly, because the
 *  STEPS macro automatically reserves 0 (zero) identifier for the START step and enumerates all the custom steps
 *  identifiers starting with 1 (one).
 */
#define FC_FUNC(fcName)\
        FC_DECL(fcName) {\
            static int fc_cursor = 0;
/* IDEA: Implement protection from recursion within the same flowcharts tree.
 *      Currently flowchart is not able to detect the situation when it reentered itself. Surely it is prohibited and
 *  must be put under control in the debug build. The flowchart must have a kind of internal mutex that is switched off
 *  when the flowchart is activated and switched on when the flowchart postpones or finishes its execution.
 */
/* IDEA: Implement protection of being used by different flowcharts trees simultaneously.
 *      Currently flowchart is not able to detect the situation when it is started within a second tree prior it has
 *  finished its execution within the first tree. The flowchart must save a kind of reference to its parent flowchart
 *  (for example, the entry-point of its parent flowchart function) that started it, and further allow activation only
 *  to that parent flowchart until it finishes; when a flowchart finished execution it becomes free and may be called by
 *  a different parent flowchart. For this approach to work, the root flowchart (that is called by the application but
 *  not by a different flowchart) must not be used by two concurrent processed in the application.
 */
/* IDEA: Use 8-bit data type for fc_cursor.
 *      Indeed the total number of steps will not exceed 255. On the other hand, for statically allocated variable it's
 *  better to specify as small data type as possible - compiler will move it in appropriate location to avoid padding.
 */
/* IDEA: Keep events subscription mask in the local variable, and publish to the caller.
 *      Keeping the events subscription mask should facilitate REPOST operator and simplify the surrounding code in the
 *  application - i.e., the application will not be in need to split the whole set of pending events into two parts: the
 *  subset of events in the scope of interest of particular flowchart and the remaining subset. On the other hand, its
 *  better still to publish the subscription to the application in order to instruct it whether to activate particular
 *  flowchart on the newly triggered event or not.
 */

/**//**
 * \brief   Defines enumeration of steps identifiers within the flowchart.
 * \param[in]   ...     List of steps identifiers separated with commas.
 * \details When a flowchart uses named identifiers for its steps, it may be recommended to put enumeration of all
 *  custom flowchart steps identifiers after the FUNC clause. If so, it's strongly recommended also to use the STEPS
 *  macro for this instead of defining the enumeration directly, because the STEPS macro automatically reserves 0 (zero)
 *  identifier for the START step and enumerates all the custom steps identifiers starting with 1 (one).
 */
#define FC_STEPS(...)\
            enum {\
                START = 0,\
                __VA_ARGS__\
            };

/**//**
 * \brief   Opens definition of the flowchart START step.
 * \details The START step defines the step of the flowchart that is executed first when the flowchart is called for the
 *  very first time after start-up or next time after the flowchart has reached its END terminator.
 * \details A flowchart must have exactly one START step. Its definition must follow immediately after opening the
 *  definition of its function (or after definition of the flowchart local variables and auxiliary operators, and
 *  enumeration of its steps identifiers, if they are introduced).
 * \details There may be conventional operators in the START step body, for example, initializers or branching operators
 *  to the next step. However, it's not necessary to put anything into the START body. If the START body has no
 *  branching operator at its end, the next step defined right below the START body is entered and executed in the same
 *  execution flow.
 * \note    If a flowchart accepts events and uses the POST operator, it is recommended to use the START step body for
 *  defining the events sensitivity set of the flowchart and then postpone the flowchart execution.
 * \note    The START step is accessible for branching operators with the identifier 'START' or '0' (zero) as well. The
 *  'START' identifier and the '0' numeric identifier are reserved for the START step and must not be used as
 *  identifiers of other steps in the flowchart.
 * \details One may surround the START body with paired braces to emphasize its limits; the opening brace must follow
 *  the START clause.
 * \details For more details see definition of a flowchart step.
 */
#define FC_START\
            switch(fc_cursor) {\
                default: {\
                    while(1);\
                    break;\
                }\
                case_START:\
                case_0:\
                case 0: {

/**//**
 * \brief   Opens definition of a step within the flowchart, closes definition of the previous step.
 * \param[in]   stepName    The step name. An identifier or a numeric constant unique within the flowchart.
 * \details Steps are the main objects in the flowchart definition. They define subsequences of operators performed by
 *  the flowchart. A step may be considered as a set of operators, or as a block of code in terms of conventional
 *  functions.
 * \details Normally each step body contains operators that perform some specific activity, including conditional
 *  execution with IF operator, calling functions, etc. Still, a step body may be empty. Step body may contain local
 *  (private to this step) automatic or static variables.
 * \details Normally a flowchart has a number of steps in its definition. Still, it may have no steps defined explicitly
 *  at all. Indeed, the mandatory START step implicitly defines the entry step - i.e., each flowchart provides at least
 *  one step, the START step.
 * \details Steps are executed sequentially in the direct order of their definition within a flowchart. If a step does
 *  not have the END or branching (GOTO or POST) operator at the end of its body, the next step defined sequentially
 *  after this one will be entered after the current step is fully passed. For the very last step in the flowchart
 *  definition, if it has no END or branching operator at the end, the END terminator is entered (i.e., the flowchart
 *  finishes its execution).
 * \details When it's necessary to break the direct order of steps execution, a branching operator (GOTO or POST) must
 *  be used. The step names (or numeric identifiers) serve as the branching destination labels.
 * \details Steps within a flowchart must have unique names or numeric identifiers. If names are used for steps, they
 *  must be defined previously as numeric constants (for example, as macro constants or enumeration). The 'START' name
 *  and the numeric identifier '0' (zero) are reserved for the START step.
 * \note    It's allowed to mix names with numeric identifiers within a single flowchart. Even if, for some reason, a
 *  named identifier of a step will get the same numeric value as one of numerically identified steps in the same
 *  flowchart, this will produce a compile-time error, because these two conflicting identifiers are treated as two
 *  cases with the same label of a single switch block.
 * \note    Branching between steps is allowed only to entry points of steps, but not their internals. If one needs to
 *  perform jump into the middle of a destination step, the corresponding step must be divided in two (or more) separate
 *  (sequential) steps each formally having its own entry point (i.e., unique name or numeric identifier).
 * \details If a flowchart is not the endless, it should contain at least one explicit END operator. However, it may not
 *  have explicit END operator; in this case the END terminator clause will play the role of the implicit END operator
 *  at the end of the last step defined within the flowchart function.
 * \details If a flowchart intends to wait for an asynchronous event (or a combination of events) it must use the POST
 *  operator to pause (postpone) its execution. The POST operator works almost in the same manner as the GOTO operator -
 *  i.e., it performs branching to a different step within the flowchart - but the destination step will be entered in a
 *  different execution flow, when the dedicated event triggered.
 * \details One may surround the STEP body with paired braces to emphasize its limits; the opening brace must follow
 *  the STEP clause.
 */
#define FC_STEP(stepName)\
                }\
                case_##stepName:\
                case stepName: {
/* IDEA: Introduce special kind of step - FC_WAIT.
 *      Currently both GOTO and POST operators may jump on arbitrary kind of step: START, STEP, SUB - and in the case of
 *  the GOTO operator (but not the POST) it's not necessary indeed to have the 'case stepName:' in description of the
 *  branch destination step. The switch's case is necessary in the definition of a step only if such a step is used
 *  somewhere as the destination of the POST operator, and it's redundant for steps that are not used as destination for
 *  the POST operator, just because such switch's cases never trigger. So, introducing them in all steps descriptions
 *  just increases the code. --> Use 'case stepName:' only in steps that WAIT for events, and the START step also.
 *      On the other hand, if a step may be entered through the POST operator, probably (need to investigate), it must
 *  not allow entering by other means - i.e., branching through the GOTO or implicit entering from the previous step. It
 *  may be true, because the first thing the POST-destination step does is (1) it verifies if the WAIT condition holds,
 *  and (2) it clears the subset of triggered events /or repostpones - so, it must be entered only due to the subscribed
 *  event triggering, but not due to GOTO or implicit entering. --> Prohibit entering the WAIT state by means other then
 *  due to an event triggering: exclude the 'case_##stepName:' and put something like 'break;' after the first '}'.
 *      The third argument is to combine the events subscription list specified to the POST operator with the same list
 *  of cleared events in the POST-destination step block. Indeed, it would be better to specify this list only once just
 *  in order to avoid 'copy-paste' errors. Such a list may be specified in description of the WAIT state but not the
 *  POST operator. It also facilitate (1) possibility of introducing multiple POST operators with the same WAIT-
 *  destination step, and (2) the use of the REPOST operator (that postpones the same step with the same events
 *  sensitivity set).
 *      And finally, the wait-for expression may be integrated into the WAIT step declaration. It will allow to hide the
 *  automation of repostponing the wait-step without clearing triggered events if the desired combination of events has
 *  not occurred yet, and to clear the triggered events if the expression holds.
 *      Think how to combine the single FC_WAIT with different cases like START, STEP, SUB. Probably START and SUB must
 *  not be entered through the POST operator.
 */
/* IDEA: Refuse of 'case_##' prefix.
 *      It will facilitate Eclipse-navigation between flowchart steps.
 */

/**//**
 * \brief   Opens definition of a step within the flowchart, and calls the specified subflowchart.
 * \param[in]   stepName        The step name. An identifier or a numeric constant unique within the caller flowchart.
 * \param[in]   subFcName       The subflowchart name. A globally unique identifier or a numeric constant.
 * \details This macro defines a special kind of step that is used for calling a subflowchart. This SUB step is treated
 *  and behaves mostly as a general case step - i.e., it must be defined in the flowchart function with the SUB clause
 *  (as a common step is defined with the STEP clause), it may play the role of the destination step for branching (GOTO
 *  or POST) operator, and it may contain operators in its body. The only difference between steps defined with SUB and
 *  STEP clauses is: (1) the first contains special hidden code, as its first operator, for activating the specified
 *  subflowchart (calling its function). This code is executed unconditionally each time the SUB step is entered, and
 *  (2) it provides proper treatment of the status returned by the subflowchart - if it has finished or paused its
 *  execution.
 * \note    Do not use the RUN operator to call the subflowchart function from the parent flowchart. This operator does
 *  not provide automatic treatment of the status returned by the called subflowchart. When necessary to call a
 *  subflowchart, introduce a dedicated SUB step for it.
 * \details The SUB step hidden code, that calls the subflowchart function, performs the following: (1) saves the caller
 *  flowchart cursor (it's necessary for further reactivation of the paused subflowchart), (2) calls the function of the
 *  subflowchart and passes the set of currently active events to it, (3) when the subflowchart function returned,
 *  analyzes the status returned by it: if the subflowchart has paused or finished its execution, (4.a) pauses the
 *  caller flowchart if the subflowchart has paused itself, and propagates the sensitivity events set specified
 *  (returned) by the paused subflowchart upwards to the root flowchart, or (4.b) continues execution of the caller
 *  flowchart if the subflowchart has finished, (5) resumes execution of the subflowchart when the caller flowchart
 *  execution is resumed.
 * \details If the SUB step contains explicit operators in its body, they will be executed only when the called
 *  subflowchart finished its execution (i.e., returned the empty sensitivity events set). If the SUB step does not have
 *  explicit operators in its body, then in the described case the next step defined after the SUB step is entered as
 *  soon as the called subflowchart finished its execution. Inversely, while the called subflowchart pauses its
 *  execution (but not finished yet) the caller process is automatically paused at the current SUB step, and neither
 *  explicit operators in the SUB step body, nor the following steps in the caller flowchart are executed. When this
 *  (caller) process is activated next time, this SUB step is entered again and the subflowchart is activated. These
 *  path repeats until the subflowchart finished its execution.
 * \note    Take into account that from the moment when the subflowchart paused its execution and until it finished its
 *  execution (as a result of one of further reactivations) its parent flowchart and all the nested flowcharts up to the
 *  root flowchart becomes transparent. It means that they (1) just pass the execution flow to the paused subflowchart
 *  when the root flowchart is reactivated, do not propagate their own states and do not execute their own operators,
 *  and do not change or filter the currently active events set specified by the application, (2) pass the sensitivity
 *  events set specified by the paused subflowchart upwards back to the application, and do not change this set. In fact
 *  it looks like if the subflowchart would be called directly from the application but not through the chain of their
 *  parents starting from the root flowchart.
 * \details The same subflowchart may be called from number of different SUB steps of the same flowchart, or even from
 *  different flowcharts integrated in the same tree (with the same root flowchart). In this case from the moment when
 *  the mentioned subflowchart paused itself in a particular place of the nested flowcharts tree, all the tree becomes
 *  paused. Next time the tree is activated this subflowchart will receive the execution context and this will repeat
 *  further each time the tree is reactivated again until this subflowchart finishes its execution. When it finished,
 *  its caller flowchart continue execution from the point where the subflowchart was originally called.
 * \note    If a subflowchart is used in two (or more) flowcharts integrated into different flowchart trees, the
 *  application must ensure that the second tree will not ever call the shared subflowchart while it is in the paused
 *  state after been called from the first tree. It may require using a mutex and special design of caller flowcharts in
 *  both trees - i.e., to wait (to postpone its execution) for mutex to become unlocked prior to call the shared
 *  subflowchart. The application is responsible in this case for establishing a mutex and triggering a software event
 *  when the mutex becomes unlocked; both the caller flowcharts are responsible for postponing their execution if the
 *  mutex is currently locked with subscription on the mutex unlock event.
 */
#define FC_SUB(stepName, subFcName)\
                }\
                case_##stepName:\
                case stepName: {\
                    {\
                        const uint32_t sensEventsSet = subFcName(pEvents);\
                        if (sensEventsSet != 0x0) {\
                            fc_cursor = stepName;\
                            return sensEventsSet;\
                        }\
                    }

/**//**
 * \brief   Jumps to the specified step within the flowchart and continues in the same execution flow.
 * \param[in]   dstStepName     Name of the destination step. An identifier or a numeric constant.
 * \details The GOTO operator serves for breaking the direct execution order of steps within a flowchart. The
 *  destination step is entered and executed immediately in the same execution flow.
 * \details The step names (or numeric identifiers) serve as the branching destination labels for the GOTO operator.
 * \details The GOTO operator may be used as the final operator in the step body, or called conditionally in the middle
 *  of the body.
 * \note    Branching between steps is allowed only to entry points of steps, but not their internals. If one needs to
 *  perform jump into the middle of a destination step, such a step must be divided in two (or more) separate steps each
 *  formally having its own entry point (i.e., unique name or numeric identifier).
 * \note    It's allowed to jump back on the START step; the 'START' label or '0' (zero) numeric identifier must be used
 *  for this as the destination step identifier. It's not allowed to jump on the END terminator; the dedicated END
 *  operator must be used for finishing execution.
 */
#define fc_goto(dstStepName)\
                    goto case_##dstStepName

/**//**
 * \brief   Jumps to the specified step within the flowchart and postpones its execution.
 * \param[in]   dstStepName     Name of the destination step. An identifier or a numeric constant.
 * \param[in]   sensEventsSet   Set of events to which the flowchart wishes to become sensitive. Must not be zero.
 * \details The POST operator serves for pausing execution of the flowchart. It branches to the destination step, the
 *  destination step is entered and assigned to the flowchart cursor, but its execution is postponed until the next
 *  activation of the flowchart, and the flowchart function returns. The flowchart function returns the 32-bit mask
 *  describing the specified sensitivity events set for this flowchart for the next period of activation.
 * \note    The POST operator may be used for breaking the direct order of steps execution within a flowchart (like the
 *  GOTO operator), hence it's not the main purpose of it. Generally the definition of the destination step of the POST
 *  operator is placed right after the source step definition within a flowchart. However, the ability of the POST
 *  operator to perform branching may be used when necessary to reenter the postponed step again. It may be necessary
 *  when the postponed step waits for specific combination of events, but not a simple event - the paused flowchart in
 *  this case is activated just on events triggering, and it has to check the desired expression of activation itself
 *  whether if holds or not, and if it is not held the flowchart has to repostpone its execution. To repostpone the
 *  flowchart execution, its step that was labeled as the destination step for the POST operator called previous time,
 *  has to perform the POST operator on itself again.
 * \details If the paused flowchart was called as a subflowchart from another (parent) flowchart, the parent flowchart
 *  and its parents in turn until the root flowchart are also paused. The root (topmost level) flowchart finally
 *  returns to the application the sensitivity events set that was originally assigned by the paused child (lowermost
 *  level) flowchart. When the root flowchart is activated next time, its execution will be resumed from the point where
 *  it has called the paused subflowchart previous time, and the lowermost subflowchart in turn will resume its
 *  execution from the point where it was paused - the entry point of the destination step of the POST operator.
 * \details The step names (or numeric identifiers) serve as the branching destination labels for the POST operator.
 * \details The POST operator may be used as the final operator in the step body, or called conditionally in the middle
 *  of the body.
 * \note    Pausing a flowchart execution is allowed only at entry points of steps, but not within their internals. If
 *  one needs to pause execution in the middle of a current step, this step must be divided in two (or more) separate
 *  (sequential) steps each formally having its own entry point (i.e., unique name or numeric identifier).
 * \note    It's allowed to jump back on the START step and postpone its execution; the 'START' label or '0' (zero)
 *  numeric identifier must be used for this as the destination step identifier. Hence, in this case the flowchart will
 *  be considered as paused but not finished - so, it may be recommended in such case to use the END operator to finish
 *  the flowchart. It's not allowed to jump on the END terminator; the dedicated END operator must be used for that.
 */
#define fc_post(dstStepName, sensEventsSet)\
                    do {\
                        const uint32_t sensEventsSet_ = sensEventsSet;\
                        if (sensEventsSet_ == 0x0)\
                            while(1);\
                        fc_cursor = dstStepName;\
                        return sensEventsSet_;\
                    } while(0)
/* IDEA: Implement the REPOST operator.
 *      If the wait-for expression is not held, the postponed step must be repostponed. If the wait-for expression will
 *  be validated explicitly (in an IF operator), a special kind of REPOST operator must be introduced, in order to keep
 *  the pending events set unchanged and pause the flowchart execution with the same events sensitivity set. If the
 *  wait-expression will be validated implicitly (by the internal code of the special WAIT step header), there is no
 *  need in distinct REPOST operator, but its functionality must be performed by the WAIT step header automatically.
 */

/**//**
 * \brief   Finishes execution of the flowchart.
 * \details The END operator serves for finishing execution of the flowchart. It may be used as the final operator in a
 *  step body, or called conditionally in the middle of the body. A flowchart may have one or more END operators.
 * \details If a flowchart has no explicit END operator, it nevertheless has one implicit END operator at the end of the
 *  very last step body. The END terminator provides this implicit END operator.
 * \note    If one intends to make the endless flowchart, the very last step defined in the flowchart must have a
 *  branching operator (GOTO or POST) at its end to avoid execution of the implicit END operator provided by the END
 *  terminator clause.
 * \details A flowchart that executed the END operator considered as reached the END terminator and finished its
 *  execution. The next time this flowchart is activated it will be executed from its START step.
 * \note    The finished flowchart, in the general case, becomes insensitive to all events in the system.
 * \details If the finished flowchart was called as a subflowchart from another (parent) flowchart, the execution flow
 *  returns into the parent (caller) flowchart and it continues executing operators of its current step.
 */
#define fc_end\
                    goto fc_finish;

/**//**
 * \brief   Closes definition of a flowchart function, finishes its execution if reached.
 * \details The END terminator is a formal entity that must be put at the end of the flowchart definition after all
 *  other internals. Even if a particular flowchart is endless (i.e., it never executes the END operator) it must have
 *  the END terminator at least to close definition of the flowchart function. A flowchart must have exactly one END
 *  terminator.
 * \details The END terminator does not have a body (unlike the START step or intermediate steps), so no operators are
 *  allowed after the END clause.
 * \details If the last step defined in the flowchart does not contain trailing explicit END or branching (GOTO or
 *  POST) operator, the END terminator clause plays the role of the implicit END operator - it finishes the flowchart
 *  execution at the end of the very last step in the flowchart.
 * \details One may put the opening brace right after the FUNC macro (that opens a flowchart function definition) to
 *  emphasize the limits of the flowchart definition. Still, it's not necessary. If the opening brace is used, then the
 *  closing brace must be put after the END terminator clause.
 * \note    It's not allowed to jump (perform GOTO) on the END terminator directly; the dedicated END operator must be
 *  used for finishing the flowchart. It's not allowed to pause execution of the flowchart at the END terminator; the
 *  flowchart must be finished instead of pausing in this case, or consider the ability to jump back on the START step
 *  with pausing, using the POST operator.
 */
#define FC_END\
                }\
            }\
        fc_finish:\
            fc_cursor = 0;\
            return 0x0;\
        }
/* IDEA: Allow code after FC_END.
 *      Code in the END clause may be useful for common finalization, releasing resources, etc.
 */
/**@}*/

#endif /* _BB_SYS_FLOWCHART_H */
