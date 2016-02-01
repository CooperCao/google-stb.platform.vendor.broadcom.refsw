/******************************************************************************
 * (c) 2010-2015 Broadcom Corporation
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
 *
 *****************************************************************************/


#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_file_pes_priv.h"

BDBG_MODULE(BMUXLIB_FILE_PES);
BDBG_FILE_MODULE(BMUX_PES_FINISH);
BDBG_FILE_MODULE(BMUX_PES_MEMORY);

#ifdef BMUXLIB_PES_P_TEST_MODE
#include <stdio.h>
#endif


/**************************
    Static Definitions
**************************/
static const BMUXlib_File_PES_CreateSettings stDefaultCreateSettings =
{
   BMUXLIB_FILE_PES_P_SIGNATURE_CREATESETTINGS,
};

/* Start Settings are set explicitly in GetDefaultStartSettings() */

static const BMUXlib_File_PES_FinishSettings stDefaultFinishSettings =
{
   BMUXLIB_FILE_PES_P_SIGNATURE_FINISHSETTINGS,

   BMUXlib_FinishMode_ePrepareForStop             /* eFinishMode */
};

/*************************
*   Create/Destroy API   *
*************************/

/*
   Function:
      BMUXlib_File_PES_GetDefaultCreateSettings

   Description:
      Return the default settings for the Create() API in the
      location pointed to by pCreateSettings.

   Returns:
      None
*/
void BMUXlib_File_PES_GetDefaultCreateSettings(BMUXlib_File_PES_CreateSettings *pCreateSettings)
{
   BDBG_ENTER(BMUXlib_File_PES_GetDefaultCreateSettings);
   BDBG_ASSERT(pCreateSettings != NULL);

   *pCreateSettings = stDefaultCreateSettings;

   BDBG_LEAVE(BMUXlib_File_PES_GetDefaultCreateSettings);
}

/*
   Function:
      BMUXlib_File_PES_Create

   Description:
      Create a new "instance" of the PES file-based software mux, using the
      settings indicated by the structure pointed to by pCreateSettings.

      Handle to the created instance returned in the location pointed to by phPESMux.
      This location will be set to NULL upon failure.

      Mux will be placed in the initial state of "stopped"

   Returns:
      BERR_SUCCESS               - Mux instance successfully created
      BERR_OUT_OF_SYSTEM_MEMORY  - no memory to allocate internal storage
      BERR_INVALID_PARAMETER     - bad Create Setting (minimum space requirements not met)
*/
BERR_Code BMUXlib_File_PES_Create(BMUXlib_File_PES_Handle *phPESMux, const BMUXlib_File_PES_CreateSettings *pCreateSettings)
{
   BERR_Code rc = BERR_UNKNOWN;
   BMUXlib_File_PES_Handle hMux;
   BDBG_ENTER(BMUXlib_File_PES_Create);

   BDBG_ASSERT(pCreateSettings != NULL);
   BDBG_ASSERT(phPESMux != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CREATESETTINGS == pCreateSettings->uiSignature);

   BDBG_MSG(("====Creating PES Mux===="));

   *phPESMux = NULL;                            /* incase create fails */

   /* Allocate PES context from system memory */
   hMux = (BMUXlib_File_PES_Handle)BKNI_Malloc(sizeof(struct BMUXlib_File_PES_P_Context));
   BDBG_MODULE_MSG(BMUX_PES_MEMORY, ("Context: Allocating %d bytes", sizeof(struct BMUXlib_File_PES_P_Context)));
   if (NULL != hMux)
   {
      BKNI_Memset( hMux, 0, sizeof(struct BMUXlib_File_PES_P_Context) );

      /* fill in the signature in the context */
      hMux->uiSignature = BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT;
      *phPESMux = hMux;
      rc = BERR_SUCCESS;
   } /* hMux != NULL */
   else
   {
      /* unable to allocate the context */
      BDBG_ERR(("Unable to allocate memory for mux context"));
      rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }


   BDBG_LEAVE(BMUXlib_File_PES_Create);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_Destroy

   Description:
      Deallocates heap memory utilized by this mux instance, then frees the
      instance.  The caller should set hPESMux to NULL after this API returns.

   Returns:
      None
*/
void BMUXlib_File_PES_Destroy(BMUXlib_File_PES_Handle hPESMux)
{
   BDBG_ENTER(BMUXlib_File_PES_Destroy);

   BDBG_ASSERT(hPESMux != NULL);

   BDBG_MSG(("====Destroying PES Mux===="));

   /* the following signifies an attempt to free up something that was either
      a) not created by Create()
      b) has already been destroyed
   */
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT == hPESMux->uiSignature);

   /* free the context ... */
   BKNI_Free(hPESMux);

   BDBG_LEAVE(BMUXlib_File_PES_Destroy);
}

/****************************
*   Start/Finish/Stop API   *
****************************/

/*
   Function:
      BMUXlib_File_PES_GetDefaultStartSettings

   Description:
      Returns the default settings for the Start() API in the location
      indicated by pStartSettings.

   Returns:
      None
*/
void BMUXlib_File_PES_GetDefaultStartSettings(BMUXlib_File_PES_StartSettings *pStartSettings)
{
   BDBG_ENTER(BMUXlib_File_PES_GetDefaultStartSettings);

   BDBG_ASSERT(pStartSettings != NULL);

   /* clear the entire start settings ... */
   /* NOTE: This will ensure the following (to allow for error checking):
            * all function pointers are NULL
            * all context pointers are NULL */
   BKNI_Memset(pStartSettings, 0, sizeof(BMUXlib_File_PES_StartSettings));

   /* initialise specific values as needed ... */
   pStartSettings->uiSignature = BMUXLIB_FILE_PES_P_SIGNATURE_STARTSETTINGS;

   /* ensure invalid protocol to pick up if user did not set these ... */
   pStartSettings->stInterface.stBufferInfo.eProtocol = BAVC_VideoCompressionStd_eMax;

   BDBG_LEAVE(BMUXlib_File_PES_GetDefaultStartSettings);
}

/*
   Function:
      BMUXlib_File_PES_Start

   Description:
      Start the mux ready for processing of incoming data from the encoders
      using the configuration parameters specified in pStartSettings.  This
      call will transition the mux to the "started" state.

      Start settings define the configuration of the mux prior to starting the
      muxing process, and do not change thereafter.
      They also provide information about the encoder that is connected to the
      specific input channel, the output destination, etc.

   Returns:
      BERR_SUCCESS               - mux successfully started
      BERR_INVALID_PARAMETER     - bad start setting or missing create data needed by a start setting
      BERR_NOT_SUPPORTED         - Start() invoked from invalid state (must be invoked
                                   from "stopped" state) or invalid video/audio protocol
*/
BERR_Code BMUXlib_File_PES_Start(BMUXlib_File_PES_Handle hPESMux, const BMUXlib_File_PES_StartSettings *pStartSettings)
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER(BMUXlib_File_PES_Start);

   BDBG_ASSERT(hPESMux != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT == hPESMux->uiSignature);
   BDBG_ASSERT(pStartSettings != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_STARTSETTINGS == pStartSettings->uiSignature);

   /* verify the storage system interface has been supplied ... */
   if (
      /* verify the output interface has been supplied ... */
      (NULL == pStartSettings->stOutput.pfAddDescriptors) ||
      (NULL == pStartSettings->stOutput.pfGetCompleteDescriptors) ||
      /* verify at input interface supplied */
      (NULL == pStartSettings->stInterface.fGetBufferDescriptors ) ||
      (NULL == pStartSettings->stInterface.fConsumeBufferDescriptors ) ||
      (NULL == pStartSettings->stInterface.fGetBufferStatus )
      )
   {
      BDBG_LEAVE(BMUXlib_File_PES_Start);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BDBG_MSG(("====Starting PES Mux===="));

   /* Start() can only be performed from the Stopped state - anything else is an error */
   if (BMUXlib_State_eStopped ==  BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux))
   {
      hPESMux->stStartSettings = *pStartSettings;

      rc = BERR_SUCCESS;

      /* create input */
      /* there is an active video input */
      if ( BERR_SUCCESS == rc )
      {
         rc = BMUXlib_File_PES_P_Start(hPESMux);
      }

      if ( BERR_SUCCESS == rc )
      {
         /* mux is started ... */
         BMUXLIB_FILE_PES_P_SET_MUX_STATE(hPESMux, BMUXlib_State_eStarted);
      }
   }
   else
   {
      /* Start() invoked from an invalid state - error: do nothing */
      BDBG_ERR(("PES Mux Start:: Invoked from invalid state: %d",  BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux)));
      rc = BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   BDBG_LEAVE(BMUXlib_File_PES_Start);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_GetDefaultFinishSettings

   Description:
      Returns the default settings for the Finish() API in the location
      indicated by pFinishSettings.

   Returns:
      None
*/
void BMUXlib_File_PES_GetDefaultFinishSettings(BMUXlib_File_PES_FinishSettings *pFinishSettings)
{
   BDBG_ENTER(BMUXlib_File_PES_GetDefaultFinishSettings);
   BDBG_ASSERT(pFinishSettings != NULL);

   *pFinishSettings = stDefaultFinishSettings;

   BDBG_LEAVE(BMUXlib_File_PES_GetDefaultFinishSettings);
}

/*
   Function:
      BMUXlib_File_PES_Finish

   Description:
      Request the Mux to finish the muxing process.
      If the finish mode is "prepare for stop", then this call will transition the mux to the
      "finishing_input" state and DoMux will continue to process remaining data and will
      subsequently transition to the "finishing_output" state when it finalizes the file
      (i.e. writes the moov) and finally will transition to the "finished" state when done.
      This is the normal mode of behaviour.

      NOTE: The contents of the final output are NOT valid until the mux reaches the
      "finished" state.

   Returns:
      BERR_SUCCESS            - mux "finishing"
      BERR_INVALID_PARAMETER  - bad finish setting
      BERR_NOT_SUPPORTED      - finish called in invalid state
*/
BERR_Code BMUXlib_File_PES_Finish(BMUXlib_File_PES_Handle hPESMux, const BMUXlib_File_PES_FinishSettings *pFinishSettings)
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER(BMUXlib_File_PES_Finish);

   BDBG_ASSERT(hPESMux != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT == hPESMux->uiSignature);
   BDBG_ASSERT(pFinishSettings != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_FINISHSETTINGS == pFinishSettings->uiSignature);

   BDBG_MSG(("====Finishing PES Mux===="));

   /* Finish() can only be performed from the Started state.
      Finish() from any of the finishing/finished states is a "do nothing"
      Anything else is invalid */
   switch (BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux))
   {
      case BMUXlib_State_eStarted:
         switch (pFinishSettings->eFinishMode)
         {
            case BMUXlib_FinishMode_ePrepareForStop:
               /* mux is now "finishing input" ... */
               BDBG_MODULE_MSG(BMUX_PES_FINISH, ("eStarted --> eFinishingInput"));
               BMUXLIB_FILE_PES_P_SET_MUX_STATE(hPESMux, BMUXlib_State_eFinishingInput);
               rc = BERR_SUCCESS;
               break;

            default:
               /* unrecognized finish mode - do nothing */
               BDBG_ERR(("Invalid Finish mode supplied: %d", pFinishSettings->eFinishMode));
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);
               break;
         }
         break;
      case BMUXlib_State_eFinishingInput:
      case BMUXlib_State_eFinishingOutput:
      case BMUXlib_State_eFinished:
         /* do nothing if invoked from these states - already finishing! */
         rc = BERR_SUCCESS;
         break;
      default:
         BDBG_ERR(("PES Mux Finish:: Invoked from invalid state: %d",  BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux)));
         rc = BERR_TRACE(BERR_NOT_SUPPORTED);
         break;
   }
   BDBG_LEAVE(BMUXlib_File_PES_Finish);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_Stop

   Description:
      Stop the mux, and return the internal state to default values
      (effectively reset everything).  This can be called when the mux is in
      any state.

      For a clean stop, this should only be called after the
      "finished" event occurs after calling BMUXlib_File_PES_Finish()

      This function may need to be called without BMUXlib_File_PES_Finish()
      in cases where an abrupt stop is needed.
      If this is done, then DoMux will immediately halt muxing, and
      any remaining data will be discarded (mux will move directly to "stopped" state.
      Under these conditions, the PES will not be completed, and will be invalid.
      Immediate Stop implies hardware not available (external storage ejected, for example)
      or some other condition that needs the stop to be performed without delay.

   Returns:
      BERR_SUCCESS   - always successful
*/
BERR_Code BMUXlib_File_PES_Stop(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BMUXlib_File_PES_Stop);

   BDBG_ASSERT(hPESMux != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT == hPESMux->uiSignature);

   BDBG_MSG(("====Stopping PES Mux===="));

   /* release all resources created by Start() and ... */
   BMUXlib_File_PES_P_Stop(hPESMux);

   BDBG_LEAVE(BMUXlib_File_PES_Stop);
   return rc;
}

/*************************
*    Mux Execute API     *
*************************/
/*
   Events:          |  Mux states:
                    |     Stopped        Started           Finishing_Input   Finishing_Output     Finished
======================================================================================================
   Start            |     Started        Invalid           Invalid           Invalid              Invalid
   Finish           |     Invalid        Finishing_Input   NOP               NOP                  NOP
   Input_Done       |     Invalid        Invalid           Finishing_Output  Invalid              Invalid
   Output_Done      |     Invalid        Invalid           Invalid           Finished             Invalid
   Stop             |     (1)            Stopped           Stopped           Stopped              Stopped

   Invalid Event = Error + No change in state
   NOP = do nothing (no state change)
   (1) A stop in the stopped state simply resets everything
*/
/*
   Function:
      BMUXlib_File_PES_DoMux

   Description:
      Main processing routine for performing the muxing operation.

      DoMux must not block - once an I/O transaction is scheduled, if there is nothing that
      can be done until the I/O completes then this must return and the domux will not be
      scheduled again until at least one I/O operation is complete (the application can monitor
      the status of the storage system passed into the mux via the hStorage handle to determine
      if I/O is waiting or not for any of the I/O streams opened for this mux)

      Returns the current state of the muxing process in the location pointed to by pStatus

      The muxing is complete when in the "finished" or "stopped" state.  The output is not
      valid unless the "finished" state is reached.
      Note that the encoder can be "unhitched" from the mux once the "finishing_output" state
      is reached (no more input data to process) - this signifies an EOS on all input streams
      has been seen.

      It is expected that mux will be rescheduled once output is not busy (if mux quit due to
      output) or after the indicated execution time (if mux quit due to lack of input data).

   Returns:
      BERR_SUCCESS            - mux run successfully
      BERR_NOT_SUPPORTED      - bad mux state detected
*/
BERR_Code BMUXlib_File_PES_DoMux(BMUXlib_File_PES_Handle hPESMux, BMUXlib_DoMux_Status *pStatus)
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER(BMUXlib_File_PES_DoMux);

   BDBG_ASSERT(hPESMux != NULL);
   BDBG_ASSERT(pStatus != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_PES_P_SIGNATURE_CONTEXT == hPESMux->uiSignature);

   switch (BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux))
   {
      case BMUXlib_State_eStopped:
         /* either not started, or done muxing ... do nothing */
         /* requires a Start() to move state */
         rc = BERR_SUCCESS;
         break;

      case BMUXlib_State_eStarted:
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted(hPESMux)))
            break;
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessInputDescriptors(hPESMux)))
            break;
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(hPESMux)))
            break;
         /* If the EOS has been seen, automatically go to the finishing input state */
         if ( true == BMUXlib_File_PES_P_EOSSeen(hPESMux) )
         {
            BDBG_MODULE_MSG(BMUX_PES_FINISH, ("eStarted --> eFinishingInput (Auto)"));
            BMUXLIB_FILE_PES_P_SET_MUX_STATE(hPESMux, BMUXlib_State_eFinishingInput);
         }
         break;

      case BMUXlib_State_eFinishingInput:
         /* finish processing data from encoders after a Finish() call */
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted(hPESMux)))
            break;
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessInputDescriptors(hPESMux)))
            break;
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(hPESMux)))
            break;
         /* ensure that all input data is processed, and when done, move to "finishing output" */
         if (BMUXlib_File_PES_P_IsInputProcessingDone(hPESMux))
         {
            BDBG_MODULE_MSG(BMUX_PES_FINISH, ("eFinishingInput --> eFinishingOutput"));
            BMUXLIB_FILE_PES_P_SET_MUX_STATE(hPESMux, BMUXlib_State_eFinishingOutput);
         }
         break;

      case BMUXlib_State_eFinishingOutput:
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted(hPESMux)))
            break;
         if (BERR_SUCCESS != (rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(hPESMux)))
            break;

         if (BMUXlib_File_PES_P_IsOutputProcessingDone(hPESMux))
         {
            BDBG_MODULE_MSG(BMUX_PES_FINISH, ("eFinishingOutput --> eFinished"));
            BMUXLIB_FILE_PES_P_SET_MUX_STATE(hPESMux, BMUXlib_State_eFinished);
         }
         break;

      case BMUXlib_State_eFinished:
         /* final output completed and is valid */
         /* nothing more to be done - need a Stop() call to move state */
         rc = BERR_SUCCESS;
         break;

      default:
         /* error: invalid state (ideally, user should force a Stop() )*/
         BDBG_ERR(("DoMux:: Invalid State detected: %d",  BMUXLIB_FILE_PES_P_GET_MUX_STATE(hPESMux)));
         rc = BERR_TRACE(BERR_NOT_SUPPORTED);
         break;
   }

   *pStatus = hPESMux->stStatus;

   if ( pStatus->bBlockedOutput )
   {
      BDBG_WRN(("Blocked"));
   }

   BDBG_LEAVE(BMUXlib_File_PES_DoMux);
   return rc;
}

/*****************************************************************************
* EOF
******************************************************************************/
