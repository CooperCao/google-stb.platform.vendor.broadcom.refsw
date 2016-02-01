/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_fnrt.h"
#include "bmuxlib_fnrt_priv.h"

#if BMUXLIB_FNRT_P_DUMP_DESC
#include <stdio.h>
#endif

BDBG_MODULE(BMUXLIB_FNRT);

const char * const BMUXlib_FNRT_P_InputType_FriendlyNameLUT[BMUXlib_FNRT_P_InputType_eMax] =
{
   "video",
   "audio"
};

/******************/
/* Create/Destroy */
/******************/
static const BMUXlib_FNRT_CreateSettings s_stDefaultCreateSettings =
{
   BMUXLIB_FNRT_P_SIGNATURE_CREATESETTINGS, /* Signature */
   2, /* Max Num Groups - 2 groups (1 video and 1 audio) */
};

void
BMUXlib_FNRT_GetDefaultCreateSettings(
         BMUXlib_FNRT_CreateSettings *pCreateSettings
         )
{
   BDBG_ENTER( BMUXlib_FNRT_GetDefaultCreateSettings );

   if ( NULL != pCreateSettings )
   {
      *pCreateSettings =  s_stDefaultCreateSettings;
   }

   BDBG_LEAVE( BMUXlib_FNRT_GetDefaultCreateSettings );
}

/* BMUXlib_FNRT_Create - Allocates all system/device memory required for mux operation */
BERR_Code
BMUXlib_FNRT_Create(
         BMUXlib_FNRT_Handle *phMuxFNRT,  /* [out] TSMuxer handle returned */
         const BMUXlib_FNRT_CreateSettings *pstCreateSettings
         )
{
   BMUXlib_FNRT_Handle hMuxFNRT = NULL;

   BDBG_ENTER( BMUXlib_FNRT_Create );

   BDBG_ASSERT( phMuxFNRT );
   if ( NULL == phMuxFNRT )
   {
      BDBG_ERR(("phMuxFNRT is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_Create );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   *phMuxFNRT = NULL;

   hMuxFNRT = (BMUXlib_FNRT_Handle) BKNI_Malloc( sizeof( BMUXlib_FNRT_P_Context ) );

   if ( NULL == hMuxFNRT )
   {
      BDBG_ERR(("Not enough system memory"));
      BDBG_LEAVE( BMUXlib_FNRT_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   BKNI_Memset( hMuxFNRT, 0, sizeof( BMUXlib_FNRT_P_Context ) );

   hMuxFNRT->uiSignature = BMUXLIB_FNRT_P_SIGNATURE_HANDLE;

   if ( NULL == pstCreateSettings )
   {
      BDBG_WRN(("pstCreateSettings is NULL.  Using defaults."));

      BMUXlib_FNRT_GetDefaultCreateSettings( &hMuxFNRT->stCreateSettings );
   }
   else
   {
      if ( BMUXLIB_FNRT_P_SIGNATURE_CREATESETTINGS != pstCreateSettings->uiSignature )
      {
         BDBG_ERR(("Invalid signature in create settings.  You must call BMUXlib_FNRT_GetDefaultCreateSettings()."));
         BMUXlib_FNRT_Destroy( hMuxFNRT );
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }

      hMuxFNRT->stCreateSettings = *pstCreateSettings;
   }

   /* For now, only allow 1 video and 1 audio group */
   BDBG_ASSERT( 2 == hMuxFNRT->stCreateSettings.uiMaxNumGroups );

   /* Initialize all input groups to have an unselected active input */
   {
      BMUXlib_FNRT_P_InputType eInputType;

      for ( eInputType = 0; eInputType < BMUXlib_FNRT_P_InputType_eMax; eInputType++ )
      {
         hMuxFNRT->astEncoderGroup[eInputType].eType = eInputType;
         hMuxFNRT->astEncoderGroup[eInputType].iActiveInputIndex = -1;
         hMuxFNRT->astEncoderGroup[eInputType].hMain = hMuxFNRT;

#if BMUXLIB_FNRT_P_DUMP_DESC
      {
         char fname[256];
         sprintf(fname, "BMUXlib_FNRT_DESC_");

         switch ( eInputType )
         {
            case BMUXlib_FNRT_P_InputType_eVideo:
               sprintf(fname, "%sVIDEO", fname);
               break;

            case BMUXlib_FNRT_P_InputType_eAudio:
               sprintf(fname, "%sAUDIO", fname);
               break;

            default:
               sprintf(fname, "%sUNKNOWN", fname);
         }
         sprintf(fname, "%s_%02d.csv", fname, 0 /* group ID */);
         BKNI_Printf("Creating FNRT Descriptor Dump File (%s)\n", fname);

         if (!BMuxlib_Debug_CSVOpen(&hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile, fname) )
         {
            BDBG_ERR(("Error Creating FNRTDescriptor Dump File (%s)", fname));
         }
         else
         {
            BMuxlib_Debug_CSVWrite( hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile, "count,chunk,input,flags,offset,length,opts_45khz,pts,escr,tpb,shr" );
            switch ( eInputType )
            {
               case BMUXlib_FNRT_P_InputType_eVideo:
                  BMuxlib_Debug_CSVWrite( hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile, ",vflags,dts,dut,rai" );
                  break;
               case BMUXlib_FNRT_P_InputType_eAudio:
                  BMuxlib_Debug_CSVWrite( hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile, ",raw offset,raw length,dut" );
                  break;
               default:
                  break;
            }
            BMuxlib_Debug_CSVWrite( hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile, ",action\n" );
         }
         hMuxFNRT->astEncoderGroup[eInputType].puiDescCount = &hMuxFNRT->uiDescCount; /* increment local copy unless this input added to a group */
      }
#endif
      } /* end: for input type */
   }

   /* Set the audio group to be dependent on the video group */
   hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eAudio].hMasterEncoderGroup = &hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eVideo];

#if BMUXLIB_FNRT_P_TEST_MODE
   /* create the config file needed for testing */
   BKNI_Printf("Creating FNRT Configuration File (BMUXlib_FNRT_CONFIG.csv)\n");
   if (!BMuxlib_Debug_CSVOpen(&hMuxFNRT->hConfigFile, "BMUXlib_FNRT_CONFIG.csv"))
   {
      BDBG_ERR(("Error Creating Configuration File (BMUXlib_FNRT_CONFIG.csv)\n"));
   }
   else
   {
      BMUXlib_FNRT_P_InputType eInputType;
      BMuxlib_Debug_CSVWrite(hMuxFNRT->hConfigFile, "desc_count,group_id,action,input_type,input_index,frame_base,meta_base\n");
      /* write an entry to indicate the master group ... */
      for ( eInputType = 0; eInputType < BMUXlib_FNRT_P_InputType_eMax; eInputType++ )
         if (NULL == hMuxFNRT->astEncoderGroup[eInputType].hMasterEncoderGroup)
            BMuxlib_Debug_CSVWrite(hMuxFNRT->hConfigFile, "%d,%d,set_master,%s,0,0,0\n",
               *hMuxFNRT->astEncoderGroup[eInputType].puiDescCount,
               0 /* group ID */,
               BMUXlib_FNRT_P_InputType_FriendlyNameLUT[eInputType]);
   }
#endif

   *phMuxFNRT = hMuxFNRT;

   BDBG_LEAVE( BMUXlib_FNRT_Create );
   return BERR_TRACE( BERR_SUCCESS );
}

/* BMUXlib_FNRT_Destroy - Frees all system/device memory allocated */
void
BMUXlib_FNRT_Destroy(
         BMUXlib_FNRT_Handle hMuxFNRT
         )
{
   BDBG_ENTER( BMUXlib_FNRT_Destroy );

#if BMUXLIB_FNRT_P_DUMP_DESC
   {
      BMUXlib_FNRT_P_InputType eInputType;

      for ( eInputType = 0; eInputType < BMUXlib_FNRT_P_InputType_eMax; eInputType++ )
      {
         BMuxlib_Debug_CSVClose( hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile );
         hMuxFNRT->astEncoderGroup[eInputType].hDescDumpFile = NULL;
      }
   }
#endif
#if BMUXLIB_FNRT_P_TEST_MODE
   BMuxlib_Debug_CSVClose(hMuxFNRT->hConfigFile);
   hMuxFNRT->hConfigFile = NULL;
#endif

   if ( NULL != hMuxFNRT )
   {
      BKNI_Free( hMuxFNRT );
   }
   else
   {
      BDBG_ERR(("FNRT handle is NULL!"));
   }

   BDBG_LEAVE( BMUXlib_FNRT_Destroy );
}

/*******************/
/* Input Interface */
/*******************/
static const BMUXlib_FNRT_InputSettings s_stDefaultInputSettings =
{
   BMUXLIB_FNRT_P_SIGNATURE_INPUTSETTINGS, /* Signature */
   0, /* Group ID */
};

void
BMUXlib_FNRT_GetDefaultInputSettings(
         BMUXlib_FNRT_InputSettings *pInputSettings
         )
{
   BDBG_ENTER( BMUXlib_FNRT_GetDefaultInputSettings );

   if ( NULL != pInputSettings )
   {
      *pInputSettings =  s_stDefaultInputSettings;
   }

   BDBG_LEAVE( BMUXlib_FNRT_GetDefaultInputSettings );
}

BERR_Code
BMUXlib_FNRT_P_AddInputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   const void *pstInputInterface,
   BMUXlib_FNRT_P_InputType eInputType,
   const BMUXlib_FNRT_InputSettings *pInputSettings
   )
{
   unsigned uiInputIndex;

   BDBG_ENTER( BMUXlib_FNRT_P_AddInputInterface );

   BSTD_UNUSED( pInputSettings );

   BDBG_ASSERT( hMuxFNRT );
   if ( NULL == hMuxFNRT )
   {
      BDBG_ERR(("FNRT handle is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_P_AddInputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BDBG_ASSERT( pstInputInterface );
   if ( NULL == pstInputInterface )
   {
      BDBG_ERR(("%s input interface is NULL!", BMUXlib_FNRT_P_InputType_FriendlyNameLUT[eInputType]));
      BDBG_LEAVE( BMUXlib_FNRT_P_AddInputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Find a slot to insert this input */
   for ( uiInputIndex = 0; uiInputIndex < BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_INPUTS; uiInputIndex++ )
   {
      if ( false == hMuxFNRT->astEncoderGroup[eInputType].input[uiInputIndex].bEnable )
      {
         hMuxFNRT->astEncoderGroup[eInputType].input[uiInputIndex].bEnable = true;
         hMuxFNRT->astEncoderGroup[eInputType].uiNumActiveInputs++;

         switch ( eInputType )
         {
            case BMUXlib_FNRT_P_InputType_eVideo:
            {
               BMUXlib_VideoEncoderInterface *pstVideoInputInterface = (BMUXlib_VideoEncoderInterface *) pstInputInterface;

               hMuxFNRT->astEncoderGroup[eInputType].input[uiInputIndex].interface.stVideoInterface = *pstVideoInputInterface;
               /* If this is the first input, use its buffer info as the output's buffer info */
               if ( 1 == hMuxFNRT->astEncoderGroup[eInputType].uiNumActiveInputs )
               {
                  hMuxFNRT->astEncoderGroup[eInputType].output.bufferInfo.stVideoBufferInfo = pstVideoInputInterface->stBufferInfo;
               }
            }
            break;

            case BMUXlib_FNRT_P_InputType_eAudio:
            {
               BMUXlib_AudioEncoderInterface *pstAudioInputInterface = (BMUXlib_AudioEncoderInterface *) pstInputInterface;

               hMuxFNRT->astEncoderGroup[eInputType].input[uiInputIndex].interface.stAudioInterface = *pstAudioInputInterface;
               /* If this is the first input, use its buffer info as the output's buffer info */
               if ( 1 == hMuxFNRT->astEncoderGroup[eInputType].uiNumActiveInputs )
               {
                  hMuxFNRT->astEncoderGroup[eInputType].output.bufferInfo.stAudioBufferInfo = pstAudioInputInterface->stBufferInfo;
               }
            }
            break;

            default:
               BDBG_ERR(("Unsupported input type %d!", eInputType));
               BDBG_LEAVE( BMUXlib_FNRT_P_AddInputInterface );
               return BERR_TRACE( BERR_INVALID_PARAMETER );
         }

         break;
      }
   }

   if ( BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_INPUTS == uiInputIndex )
   {
      BDBG_ERR(("No room to add a %s input", BMUXlib_FNRT_P_InputType_FriendlyNameLUT[eInputType]));
      BDBG_LEAVE( BMUXlib_FNRT_P_AddInputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
#if BMUXLIB_FNRT_P_TEST_MODE
   /* write the video interface addition to the config file */
   if (NULL != hMuxFNRT->hConfigFile)
   {
      BMuxlib_Debug_CSVWrite(hMuxFNRT->hConfigFile, "%d,%d,add_input,%s,%d,0,0\n",
         *hMuxFNRT->astEncoderGroup[eInputType].puiDescCount,
         0, /* Group ID */
         BMUXlib_FNRT_P_InputType_FriendlyNameLUT[eInputType],
         uiInputIndex);
   }
#endif

   BDBG_LEAVE( BMUXlib_FNRT_P_AddInputInterface );

   return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code
BMUXlib_FNRT_AddVideoInputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   const BMUXlib_VideoEncoderInterface *pstInputInterface,
   const BMUXlib_FNRT_InputSettings *pInputSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_FNRT_AddVideoInputInterface );

   rc = BMUXlib_FNRT_P_AddInputInterface(
      hMuxFNRT,
      pstInputInterface,
      BMUXlib_FNRT_P_InputType_eVideo,
      pInputSettings
      );

   BDBG_LEAVE( BMUXlib_FNRT_AddVideoInputInterface );
   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_FNRT_AddAudioInputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   const BMUXlib_AudioEncoderInterface *pstInputInterface,
   const BMUXlib_FNRT_InputSettings *pInputSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_FNRT_AddAudioInputInterface );

   rc = BMUXlib_FNRT_P_AddInputInterface(
      hMuxFNRT,
      pstInputInterface,
      BMUXlib_FNRT_P_InputType_eAudio,
      pInputSettings
   );

   BDBG_LEAVE( BMUXlib_FNRT_AddAudioInputInterface );
   return BERR_TRACE( rc );
}

#if 0
BERR_Code
BMUXlib_FNRT_RemoveVideoInputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   const BMUXlib_VideoEncoderInterface *pstInputInterface
   );

BERR_Code
BMUXlib_FNRT_RemoveAudioInputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   const BMUXlib_AudioEncoderInterface *pstInputInterface
   );
#endif

/********************/
/* Output Interface */
/********************/
static const BMUXlib_FNRT_OutputSettings s_stDefaultOutputSettings =
{
   BMUXLIB_FNRT_P_SIGNATURE_OUTPUTSETTINGS, /* Signature */
   0, /* Group ID */
};

void
BMUXlib_FNRT_GetDefaultOutputSettings(
         BMUXlib_FNRT_OutputSettings *pOutputSettings
         )
{
   BDBG_ENTER( BMUXlib_FNRT_GetDefaultOutputSettings );

   if ( NULL != pOutputSettings )
   {
      *pOutputSettings =  s_stDefaultOutputSettings;
   }

   BDBG_LEAVE( BMUXlib_FNRT_GetDefaultOutputSettings );
}

BERR_Code
BMUXlib_FNRT_GetVideoOutputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   BMUXlib_VideoEncoderInterface *pstOutputInterface,
   const BMUXlib_FNRT_OutputSettings *pOutputSettings
   )
{
   BDBG_ENTER( BMUXlib_FNRT_GetVideoOutputInterface );

   BSTD_UNUSED( pOutputSettings );

   BDBG_ASSERT( hMuxFNRT );
   if ( NULL == hMuxFNRT )
   {
      BDBG_ERR(("FNRT handle is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_GetVideoOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BDBG_ASSERT( pstOutputInterface );
   if ( NULL == pstOutputInterface )
   {
      BDBG_ERR(("Video output interface is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_GetVideoOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BKNI_Memset( pstOutputInterface, 0, sizeof( BMUXlib_VideoEncoderInterface ) );

   if ( 0 == hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eVideo].uiNumActiveInputs )
   {
      BDBG_ERR(("No video inputs. You need to add at least one video input before getting the video output interface"));
      BDBG_LEAVE( BMUXlib_FNRT_GetVideoOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   pstOutputInterface->pContext = &hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eVideo];
   pstOutputInterface->fGetBufferDescriptors = (BMUXlib_GetVideoBufferDescriptors) BMUXlib_FNRT_P_EncoderGroup_GetVideoBufferDescriptors;
   pstOutputInterface->fConsumeBufferDescriptors = (BMUXlib_ConsumeBufferDescriptors) BMUXlib_FNRT_P_EncoderGroup_ConsumeBufferDescriptors;
   pstOutputInterface->fGetBufferStatus = (BMUXlib_GetBufferStatus) BMUXlib_FNRT_P_EncoderGroup_GetBufferStatus;
   pstOutputInterface->stBufferInfo = hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eVideo].output.bufferInfo.stVideoBufferInfo;

   BDBG_LEAVE( BMUXlib_FNRT_GetVideoOutputInterface );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_FNRT_GetAudioOutputInterface(
   BMUXlib_FNRT_Handle hMuxFNRT,
   BMUXlib_AudioEncoderInterface *pstOutputInterface,
   const BMUXlib_FNRT_OutputSettings *pOutputSettings
   )
{
   BDBG_ENTER( BMUXlib_FNRT_GetAudioOutputInterface );

   BSTD_UNUSED( pOutputSettings );

   BDBG_ASSERT( hMuxFNRT );
   if ( NULL == hMuxFNRT )
   {
      BDBG_ERR(("FNRT handle is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_GetAudioOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BDBG_ASSERT( pstOutputInterface );
   if ( NULL == pstOutputInterface )
   {
      BDBG_ERR(("Audio output interface is NULL!"));
      BDBG_LEAVE( BMUXlib_FNRT_GetAudioOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   BKNI_Memset( pstOutputInterface, 0, sizeof( BMUXlib_AudioEncoderInterface ) );

   if ( 0 == hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eAudio].uiNumActiveInputs )
   {
      BDBG_ERR(("No audio inputs. You need to add at least one audio input before getting the audio output interface"));
      BDBG_LEAVE( BMUXlib_FNRT_GetAudioOutputInterface );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   pstOutputInterface->pContext = &hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eAudio];
   pstOutputInterface->fGetBufferDescriptors = (BMUXlib_GetAudioBufferDescriptors) BMUXlib_FNRT_P_EncoderGroup_GetAudioBufferDescriptors;
   pstOutputInterface->fConsumeBufferDescriptors = (BMUXlib_ConsumeBufferDescriptors) BMUXlib_FNRT_P_EncoderGroup_ConsumeBufferDescriptors;
   pstOutputInterface->fGetBufferStatus = (BMUXlib_GetBufferStatus) BMUXlib_FNRT_P_EncoderGroup_GetBufferStatus;
   pstOutputInterface->stBufferInfo = hMuxFNRT->astEncoderGroup[BMUXlib_FNRT_P_InputType_eAudio].output.bufferInfo.stAudioBufferInfo;

   BDBG_LEAVE( BMUXlib_FNRT_GetAudioOutputInterface );
   return BERR_TRACE( BERR_SUCCESS );
}
