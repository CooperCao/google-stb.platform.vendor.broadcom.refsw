/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
 *
 **************************************************************************/
#ifndef BVC5_CLIENT_H__
#define BVC5_CLIENT_H__

#include "berr.h"
#include "blst_slist.h"
#include "bvc5_jobq_priv.h"
#include "bvc5_activeq_priv.h"

/* BVC5_P_Client

   All the information relevant to a particular client (usually a process)

   uiClientId is allocated at Nexus level
   hRunnableXXXQ contains the jobs that have no outstanding dependencies for
   specific units: soft (jobs which use no hardware), binner, renderer and TFU
   hCompletedQ contains jobs that have finished

 */
typedef struct BVC5_P_Client
{
   BVC5_Handle       hVC5;
   uint32_t          uiClientId;
   uint64_t          uiPlatformToken;

   void             *pContext;      /* Pointer to calling context (e.g. a Nexusv3d handle) */

   /* Lifetime of a job:
    *
    * Jobs enter the system and are placed in the wait q.  When their dependencies have been
    * satisified, they move to a runnableq depending on the type of job.
    * Once the resource is available, they move from the runnable queue into the appropriate
    * hardware or software unit (for some job types there is no unit as they run directly in the scheduler)
    * When a job has completed, it will move to the completed queue.
    *
    * Jobs in the completed queue move to the finalizable queue once all their dependencies
    * have finalised.  The platform layer will be signalled that there is data in the finalizable queue
    * and it will pull that data hence moving the jobs into the finalizing queue.
    *
    * When jobs have finalized they are removed from the system.
    *
    * The active jobs queue contains all the jobs that have been submitted, but which have not yet
    * completed.  Jobs which are not in the active jobs queue or any of the completed/finalizable/finalizing queues can
    * be assumed to have left the system and to have released all their resources.
    *
    */
   BVC5_ActiveQHandle   hActiveJobs;            /* Accelerated lookup for active jobs  */

   BVC5_JobQHandle      hWaitQ;                 /* Waiting for dependencies            */
   BVC5_JobQHandle      hRunnableSoftQ;         /* Software jobs ready to run          */
   BVC5_JobQHandle      hRunnableUsermodeQ;     /* Usermode callback jobs ready to run */
   BVC5_JobQHandle      hRunnableBinnerQ;       /* Bin jobs ready to run               */
   BVC5_JobQHandle      hRunnableRenderQ;       /* Render jobs ready to run            */
   BVC5_JobQHandle      hRunnableTFUQ;          /* TFU jobs ready to run               */
   BVC5_JobQHandle      hRunnableBarrierQ;      /* Barriers ready to run               */

   BVC5_JobQHandle      hCompletedQ;            /* Waiting for dependency finalizers   */
   BVC5_JobQHandle      hFinalizableQ;          /* Ready to launch finalizers          */
   BVC5_JobQHandle      hFinalizingQ;           /* Finalizers are running              */

   BVC5_P_InternalJob  *psOldestNotFinalized;   /* NULL if all jobs finalised          */

   uint64_t             uiMaxJobId;             /* max job ID submitted                */

   uint32_t             uiWorkWanted;           /* Scheduler uses these to keep track  */
   uint32_t             uiWorkGiven;            /* of who has beeen given what work    */

   BLST_S_ENTRY(BVC5_P_Client) sChain;
} BVC5_P_Client;

enum
{
   BVC5_CLIENT_BIN      = 1,
   BVC5_CLIENT_RENDER   = 2,
   BVC5_CLIENT_TFU      = 4,
   BVC5_CLIENT_BARRIER  = 8
};

typedef struct BVC5_P_Client     *BVC5_ClientHandle;
typedef struct BVC5_P_ClientMap  *BVC5_ClientMapHandle;

/***************************************************************************/
/* CLIENT                                                                  */
/***************************************************************************/

/* BVC5_P_ClientCreate

 * Create a new client structure and return the handle

 */
BERR_Code BVC5_P_ClientCreate(
   BVC5_Handle        hVC5,
   BVC5_ClientHandle *phClient,
   uint32_t           uiClientId,
   uint64_t           uiPlatformToken
);

/***************************************************************************/

/* BVC5_P_ClientDestroy

 * Destroy a client

 */
BERR_Code BVC5_P_ClientDestroy(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient
);


/***************************************************************************/
/* CLIENT MAP                                                              */
/***************************************************************************/

/* BVC5_P_ClientMapCreate

 * Create an empty client map

 */
BERR_Code BVC5_P_ClientMapCreate(
   BVC5_Handle           hVC5,
   BVC5_ClientMapHandle *phClientMap
);

/***************************************************************************/

/* BVC5_P_ClientMapDestroy

 * Destroys a client map

 */
BERR_Code BVC5_P_ClientMapDestroy(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap
);

/***************************************************************************/

/* BVC5_P_ClientMapGet

   Look up handle for client id.
   Returns NULL if the client does not exist.

 */
BVC5_ClientHandle BVC5_P_ClientMapGet(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap,
   uint32_t             uiClientId
);

/***************************************************************************/

/* BVC5_P_ClientMapCreateAndInsert

   Creates a new client entry and inserts it into the map

 */
BERR_Code BVC5_P_ClientMapCreateAndInsert(
   BVC5_Handle             hVC5,
   BVC5_ClientMapHandle    hClientMap,
   void                   *pContext,
   uint32_t                uiClientId,
   uint64_t                uiPlatformToken
);

/***************************************************************************/

/* BVC5_P_ClientMapRemoveAndDestroy

   Removes client from map and destroys the client entry

 */
BERR_Code BVC5_P_ClientMapRemoveAndDestroy(
   BVC5_Handle          hVC5,
   BVC5_ClientMapHandle hClientMap,
   uint32_t             uiClientId
);

/***************************************************************************/

/* BVC5_P_ClientMapSize
 *
 * Return the size of the map
 */
uint32_t BVC5_P_ClientMapSize(
   BVC5_ClientMapHandle hClientMap
);

/***************************************************************************/

/* BVC5_P_ClientMapFirst, BVC5_P_ClientMapNext

   Iteration support for the map
   First returns the first entry, and an abstract iterator in ppNext.
   Next return the next entry and moves the iterator on.

 */
BVC5_ClientHandle BVC5_P_ClientMapFirst(
   BVC5_ClientMapHandle   hClientMap,
   void                 **ppNext
);

BVC5_ClientHandle BVC5_P_ClientMapNext(
   BVC5_ClientMapHandle   hClientMap,
   void                 **ppNext
);

/***************************************************************************/

/* JOB STATE TRANSITIONS */

/* Move a job to waiting state */
void BVC5_P_ClientJobToWaiting(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
);

/* Move a job to the runnable state */
void BVC5_P_ClientJobWaitingToRunnable(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
);

/* Move a job to completed state */
void BVC5_P_ClientJobRunningToCompleted(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
);

/* Move a job to finalizable state */
void BVC5_P_ClientJobCompletedToFinalizable(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
);

/* Move a job to finalized state - should only be used if the job has no callback fn */
void BVC5_P_ClientJobCompletedToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
   );

/* Move a job to finalizing state */
void BVC5_P_ClientJobFinalizableToFinalizing(
   BVC5_ClientHandle    hClient,
   BVC5_P_InternalJob  *psJob
);

/* End a finalized job */
void BVC5_P_ClientJobFinalizingToFinalized(
   BVC5_Handle          hVC5,
   BVC5_ClientHandle    hClient,
   uint64_t             uiJobId
);

/***************************************************************************/

/* JOB QUERIES */

/* Has a job done its comutational work? */
bool BVC5_P_ClientIsJobComplete(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
);

/* Is a job in the process of finalization? */
bool BVC5_P_ClientIsJobCompleting(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
);

/* Has a job completely left the system? */
bool BVC5_P_ClientIsJobFinalized(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
);

/* Is job completed but not yet finalized? */
bool BVC5_P_ClientIsJobFinishing(
   BVC5_ClientHandle hClient,
   uint64_t          uiJobId
);

uint64_t BVC5_P_ClientGetOldestNotFinalizedId(
   BVC5_ClientHandle hClient
);

BERR_Code BVC5_P_ClientMakeFenceForJobs(
   BVC5_Handle                   hVC5,
   BVC5_ClientHandle             hClient,
   const BVC5_SchedDependencies *pCompletedDeps,
   const BVC5_SchedDependencies *pFinalizedDeps,
   bool                          bForceCreate,
   int                          *piFence
);

BERR_Code BVC5_P_ClientMakeFenceForAnyNonFinalizedJob(
   BVC5_Handle       hVC5,
   BVC5_ClientHandle hClient,
   int              *piFence
);

/***************************************************************************/

bool BVC5_P_ClientSetMaxJobId(
   BVC5_ClientHandle hClient,
   uint64_t uiMaxJobId
);

void BVC5_P_ClientMarkJobsFlushedV3D(
   BVC5_ClientHandle hClient,
   uint32_t uiCoreIndex
);

/****************************************************************************/

bool BVC5_P_ClientSatisifed(
   BVC5_ClientHandle hClient
);

void BVC5_P_ClientSetWanted(
   BVC5_ClientHandle hClient
);

void BVC5_P_ClientSetGiven(
   BVC5_ClientHandle hClient,
   uint32_t          uiGiven
);

bool BVC5_P_ClientHasHardJobs(
   BVC5_ClientHandle hClient
);

#endif /* BVC5_CLIENT_H__ */
