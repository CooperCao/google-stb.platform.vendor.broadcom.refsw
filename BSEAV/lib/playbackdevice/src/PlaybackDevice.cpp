/*
 * (c) 1997-2009 Broadcom Corp.  All content herein is protected by
 * U.S. copyright and other applicable intellectual property laws and
 * may not be copied without the express permission of Broadcom Corp,
 * which reserves all rights.  Reuse of any of this content for any
 * purpose without the permission of Broadcom Corp. is strictly
 * prohibited.
 */
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "PlaybackDevice.h"
#include "EventQueue.h"
#include "../security/common_drm/include/drm_types.h"
#include "../security/common_drm/include/drm_widevine.h"
#define PRINT_VIDEO_DECODER_STATUS

#ifdef CARD_PLAYBACK_MEMCHECK
uint32_t mallocCnt = 0;
uint32_t freeCnt   = 0;

uint32_t   lockCnt = 0;
uint32_t unlockCnt = 0;

#define CARD_PLAYBACK_ACQUIRE_MUTEX   {lockCnt++; BKNI_AcquireMutex(m_mutex);}
#define CARD_PLAYBACK_RELEASE_MUTEX {unlockCnt++; BKNI_ReleaseMutex(m_mutex);}
#define CARD_PLAYBACK_FREE_MEM(x)     {freeCnt++; m_bufferDoneCallback(x);}

#else

#define CARD_PLAYBACK_ACQUIRE_MUTEX BKNI_AcquireMutex(m_mutex);
#define CARD_PLAYBACK_RELEASE_MUTEX BKNI_ReleaseMutex(m_mutex);
#define CARD_PLAYBACK_FREE_MEM(x)   m_bufferDoneCallback(x); 

#endif

static bool g_fileEOF = false;
static uint32_t g_pcrPid = 0x12; 
static bool gpReaderOpen = false;
static bool gpCloseRequested = false;
static uint32_t lastPcr = 0;

/**
 * @def Local Debug macros 
 */
#if 1
#undef  BDBG_ERR
#define BDBG_ERR(x) if (1) {printf x; printf("\n");}
#undef  BDBG_WRN
#define BDBG_WRN(x) if (0) {printf x; printf("\n");}
#undef  BDBG_MSG
#define BDBG_MSG(x) if (0) { printf x; printf("\n");}
#undef  BDBG_INFO
#define BDBG_INFO(x) if (0) {printf x; printf("\n");}
#endif

//#define WRITE_MEDIA_STREAM 
#ifdef WRITE_MEDIA_STREAM 

static FILE *mediaFile    = NULL;
static int   mediaFileCnt =  0; 
#endif

// Misc Settings
uint32_t CARD_PlaybackDevice::EVENT_QUEUE_SIZE = 64;

// Device Thread Priorities
int CARD_PlaybackDevice::VIDEO_DSP_PRIO = 80;

#define URL_LENGTH 256 

static void openDebugFile()
{
#ifdef WRITE_MEDIA_STREAM 
    if( (mediaFile == NULL) && (mediaFileCnt >= 0) )
    {
        char filename[128];
        sprintf(filename,"/mnt/nfs/mediaFile%d.wmv",mediaFileCnt);
        mediaFile = fopen(filename, "wb");
        if( mediaFile == NULL )
        {
            BDBG_ERR(("%s:%d fopen>%s< failed!!!", __FUNCTION__, __LINE__, filename));
            mediaFileCnt = -9999;
        }
    }
#endif
}

static void writeDebugFiles(uint8_t *pBuf, uint32_t size )
{
#ifdef WRITE_MEDIA_STREAM 
        if( mediaFile != NULL )
        {
            fwrite(pBuf, 1, size, mediaFile);
            fflush(mediaFile);
        }
#endif
}

static void closeDebugFile()
{
#ifdef WRITE_MEDIA_STREAM 
    if( mediaFile != NULL )
    {
        fclose(mediaFile);
        mediaFileCnt++;
        mediaFile = NULL;
    }
#endif
}

CARD_PlaybackDevice::CARD_PlaybackDevice(void)
{
   BDBG_MSG(("%s(): \n", __FUNCTION__));

   m_runningDeviceTasks = false;
   m_endOfStream = false;
   m_videoFrameStepping = false;
   m_dataExhaustedEventPosted = false;
   m_endOfStreamEventPosted = false;
   m_lastPostedPresentationTimestamp = INVALID_TIMESTAMP;
   m_initialized = false;

   BDBG_Init();
   m_resources = NULL;
   m_mutex = NULL;

   BDBG_MSG(("%s()\n", __FUNCTION__));

}

CARD_PlaybackDevice::CARD_PlaybackDevice(const void *pObject)
{
   CARD_PlaybackDevice *device = (CARD_PlaybackDevice *) pObject;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   m_runningDeviceTasks = device->m_runningDeviceTasks;
   m_endOfStream = device->m_endOfStream;
   m_videoFrameStepping = device->m_videoFrameStepping;
   m_dataExhaustedEventPosted = device->m_dataExhaustedEventPosted;
   m_endOfStreamEventPosted = device->m_endOfStreamEventPosted;
   m_lastPostedPresentationTimestamp = device->m_lastPostedPresentationTimestamp;
   m_mutex = NULL;

   // has to copy properly the data
   m_resources = device->m_resources;

   BDBG_MSG(("%s()\n", __FUNCTION__));

}

CARD_PlaybackDevice::~CARD_PlaybackDevice()
{
   BDBG_MSG(("%s(): \n", __FUNCTION__));
   BDBG_MSG(("%s() \n", __FUNCTION__));
}

// no need for header to be parsed
CARD_PlaybackStatus CARD_PlaybackDevice::open(CARD_StreamType   streamType,
                                     CARD_EncryptionType                    encryptType,
                                     std::vector<CARD_PlaybackVideoParams> &videoParams,
                                     std::vector<CARD_PlaybackAudioParams> &audioParams,
                                     uint32_t                               bufferDepth,
                                     CARD_PlaybackSecurity                 &security,
                                     void                                  (*bufferCallback)(void*))
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_Error nexus_rc = 0;

   BDBG_WRN(("%s: START =%p",__FUNCTION__, this));
   BDBG_MSG(("%s(): m_inintialized = %s\n", __FUNCTION__, m_initialized ? "TRUE" : "FALSE"));

   if( m_mutex != NULL )
   {
      BDBG_ERR(("%s:%d playback device already opened!!", __FUNCTION__, __LINE__));
      return(CARD_PlaybackInvalidState);
   }

   if( encryptType != CARD_EncryptType_NONE )
   {
     /* BDBG_ERR(("%s:%d Unsupported Encryption Type=%d", __FUNCTION__, __LINE__, encryptType));
      return(CARD_PlaybackInvalidArg);*/

	   m_encrypt=true;
	   m_encryptType =encryptType ;
   }

   if((streamType != CARD_StreamType_MPEG2_TS) && (streamType != CARD_StreamType_MPEG2_PS))
   {
      BDBG_ERR(("%s:%d Unsupported stream Type=%d", __FUNCTION__, __LINE__, streamType));
      return(CARD_PlaybackInvalidArg);
   }

   if( (videoParams.size() == 0) && (audioParams.size() == 0) )
   {
      BDBG_ERR(("%s:%d Video and Audio Params are Empty", __FUNCTION__, __LINE__));
      return(CARD_PlaybackInvalidArg);
   }

   CARD_PlaybackVideoParams VideoParams = {0};
   CARD_PlaybackAudioParams AudioParams = {0};

   BDBG_MSG(("%s(): videoParams[0].pid = 0x%x\n", __FUNCTION__, videoParams[0].pid));
   BDBG_MSG(("%s(): videoParams[0].pes_id = 0x%x\n", __FUNCTION__, videoParams[0].pes_id));
    printf("%s(): videoParams[0].pid = 0x%x\n", __FUNCTION__, videoParams[0].pid);
   printf("%s(): videoParams[0].pes_id = 0x%x\n", __FUNCTION__, videoParams[0].pes_id);
    printf("%s(): videoParams[0].pid = 0x%x\n", __FUNCTION__, audioParams[0].pid);
   printf("%s(): videoParams[0].pes_id = 0x%x\n", __FUNCTION__, audioParams[0].pes_id);

   if( videoParams.size() > 0 )
   {
      VideoParams = videoParams[0];

      if( videoParams.size() > 1 )
      {
         BDBG_WRN(("%s: Mulitple Video Streams - only first will be used.", __FUNCTION__));
      }
   }

   if( audioParams.size() > 0 )
   {
      AudioParams = audioParams[0];

      if( audioParams.size() > 1 )
      {
         BDBG_WRN(("%s: Mulitple Audio Streams - only first will be used.", __FUNCTION__));
      }
   }

   if(BKNI_CreateMutex(&m_mutex) != BERR_SUCCESS) 
   {
      BDBG_ERR(("%s: PlaybackDevice BKNI_CreateMutex failed",__FUNCTION__));
      m_mutex = NULL;
      return(CARD_PlaybackFailure);
   }

   CARD_PLAYBACK_ACQUIRE_MUTEX;

   // Init State Variables
   m_endOfStream = false;
   m_videoFrameStepping = false;
   m_dataExhaustedEventPosted = false;
   m_endOfStreamEventPosted = false;
   m_lastPostedPresentationTimestamp = INVALID_TIMESTAMP;

   //Device
   m_initialized = false;
   m_started = false;	
   m_preroll = 0;
   m_lastPicture = false;
   m_dataExhausted = false;
   m_lastPictureCount = false;
   m_playTime = 0;
   m_prevStreamIndex = MAX_INDEX;
   m_firstPts = false;

   //Init Device
   //

   BDBG_MSG(("%s(): initPlayback()", __FUNCTION__));

   // convert input params into nexus 
   convertParams(streamType, &VideoParams, &AudioParams);
#ifdef BRCM_COPY_PROTECTION_ENABLED	
   if(security.securityMode == CARD_OutputSecurity_NETFLIX){
	printf("\n**********Enable HDCP*********\n");
	   m_resConfig.enableHDCP=true;

   }
#endif
   

 

#ifdef WRITE_MEDIA_STREAM 
      openDebugFile();
#endif
   // initialize playback
   initPlayback();

   // initialize playback speed
   setSpeed(PAUSED_SPEED);

   BDBG_MSG(("%s(): setSpeed(PAUSED_SPEED) done ...", __FUNCTION__));
   
   // Create an event queue
   m_eventQueue.reset(new EventQueue(EVENT_QUEUE_SIZE));

   // Create Device Threads
   m_runningDeviceTasks = true;

#ifdef PLAYBACKDEVICE_STAND_ALONE_APP

   BDBG_MSG(("%s(): B_Os_Init()", __FUNCTION__));

   // Initialize our OS lib
   B_Os_Init();

   BDBG_MSG(("%s(): B_Os_Init() done ...", __FUNCTION__));

#endif

   // initialize buffer callback function
   m_bufferCallback = bufferCallback;

   // create default prioritites threads. If there is a need to change the thread parameters, please, use not NULL thread
   // settings parameter
   BDBG_MSG(("%s(): B_Thread_Create()\n", __FUNCTION__));

   m_videoDisplayThread = B_Thread_Create("video display task", CARD_PlaybackDevice::videoDisplayTask, this, NULL);

   BDBG_MSG(("%s(): B_Thread_Create() done ...\n", __FUNCTION__));

#if 0
   nexus_rc = initPlayback();

   if(nexus_rc)
   {
      rc = CARD_PlaybackFailure;
   }
#endif

   CARD_PLAYBACK_RELEASE_MUTEX;
   BDBG_WRN(("%s(): END", __FUNCTION__));

   return rc;
}

// requires header to be parsed
CARD_PlaybackStatus CARD_PlaybackDevice::open(CARD_EncryptionType  encryptType,
                                     uint32_t                     bufferDepth,
                                     CARD_PlaybackSecurity       &security,
                                     void                        (*bufferCallback)(void*))
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_Error nexus_rc = 0;

   BDBG_MSG(("%s(): \n", __FUNCTION__));
   BDBG_ERR(("%s(): Parsing is not implemented, the audio/video type and pids are not known\n", __FUNCTION__));

   // Init State Variables
   m_endOfStream = false;
   m_videoFrameStepping = false;
   m_dataExhaustedEventPosted = false;
   m_endOfStreamEventPosted = false;
   m_lastPostedPresentationTimestamp = INVALID_TIMESTAMP;

   //Device
   m_initialized = false;
   m_started = false;	
   m_preroll = 0;
   m_lastPicture = false;
   m_dataExhausted = false;
   m_lastPictureCount = false;
   m_playTime = 0;
   m_prevStreamIndex = MAX_INDEX;
   m_firstPts = false;

   //Init Device
   //

   BDBG_MSG(("%s(): initPlayback()", __FUNCTION__));
   initPlayback();
   setSpeed(PAUSED_SPEED);

   // Create an event queue
   m_eventQueue.reset(new EventQueue(EVENT_QUEUE_SIZE));

   // Create Device Threads
   m_runningDeviceTasks = true;

   // create default prioritites threads. If there is a need to change the thread parameters, please, use not NULL thread
   // settings parameter
   m_videoDisplayThread = B_Thread_Create("video display task", CARD_PlaybackDevice::videoDisplayTask, this, NULL);

#if 0
   nexus_rc = initPlayback();

   if(nexus_rc)
   {
      rc = CARD_PlaybackFailure;
   }
#endif

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return rc;
}

/***************************************************************
** old version maintained for backwards compatibility
****************************************************************/
void CARD_PlaybackDevice::close(void)
{
  

#ifdef WRITE_MEDIA_STREAM 
	closeDebugFile();
#endif
	close(false);
}

void CARD_PlaybackDevice::close(bool hold_picture)
{
   BDBG_MSG(("%s(): enter", __FUNCTION__));
   BDBG_MSG(("%s(): m_initialized = %s\n", __FUNCTION__, m_initialized ? "TRUE" : "FALSE"));

   if(m_initialized)
   {
      // make sure the device tasks (threads) will exit
      m_runningDeviceTasks = false;
      B_Thread_Destroy(m_videoDisplayThread);

      stopPlayback();

      BcmNexus_StreamPlayer_Resources_Release(m_resources);

      // Mark S. we dont even need this event queue
      m_eventQueue.reset();

      m_stcChannel = NULL;
      m_audioDecoder = NULL;
      m_audioDac = NULL;
      m_videoDecoder = NULL;
      m_videoPidChannel = NULL;
      m_audioPidChannel = NULL;
      m_bufferCallback = NULL;

      BDBG_MSG(("%s(): destroy events", __FUNCTION__));
      BDBG_MSG(("%s(): m_videoEvent = 0x%x\n", __FUNCTION__, m_videoEvent));

      // Mark S. need to figure out why this event is destroyed before 

//      BKNI_DestroyEvent(m_videoEvent);
      m_videoEvent = NULL;

      m_initialized = false;
      m_firstPts = false;
   }
   #ifdef WRITE_MEDIA_STREAM 
	closeDebugFile();
#endif

   BDBG_MSG(("%s(): leaving done ...", __FUNCTION__));

   return;
}
#if 0
CARD_PlaybackStatus CARD_PlaybackDevice::DecryptData(void *dataBuffer, uint32_t  bufferSize, CARD_EncryptKeyType keyslot_type,void *playpumpBuffer  )
{
	NEXUS_KeySlotHandle keySlotHandl;

	
	if(keyslot_type==CARD_EncryptKeyType_EVEN)
		DRM_Widevine_getKeySlot(NEXUS_SecurityKeyType_eEven, keySlotHndl);
	else if (keyslot_type==CARD_EncryptKeyType_ODD)
		DRM_Widevine_getKeySlot(NEXUS_SecurityKeyType_eEven, keySlotHndl);
	else
	{
		 BDBG_ERR(("%s:%d Invalid KeySlot Type!!", __FUNCTION__, __LINE__));
		 return CARD_PlaybackFailure;
	}

	//now call common crypto api to setup M2M and decrypt

}
#endif


CARD_PlaybackStatus CARD_PlaybackDevice::feedData(void *dataBuffer, uint32_t  bufferSize, CARD_EncryptKeyType keyslot_type )
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_Error nexus_rc = NEXUS_SUCCESS;
   BERR_Code result = BERR_SUCCESS;
   void *buffer = NULL;
   size_t local_bufferSize;
   DrmRC drm_rc=Drm_Success;
   uint8_t *dmaBuf_Src = NULL;
   uint8_t iv0[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
   //NEXUS_KeySlotHandle keySlotHandl;
   BDBG_MSG(("\nEntered %s Encrypt",__FUNCTION__));
#ifdef PRINT_VIDEO_DECODER_STATUS
   static int check_status = 1;
#endif

   CARD_PLAYBACK_ACQUIRE_MUTEX;

#ifdef PRINT_VIDEO_DECODER_STATUS
    if((check_status % 500) == 0)
    {
        NEXUS_VideoDecoderStatus Status;
        uint32_t stc;

       NEXUS_VideoDecoder_GetStatus(m_videoDecoder, &Status);
       NEXUS_StcChannel_GetStc(m_videoProgram.stcChannel, &stc);

       printf("decode %dx%d, pts %#x, stc %#x (diff %d)\n", Status.source.width, Status.source.height, Status.pts, stc, Status.pts - stc);

       printf("Status.firstPtsPassed = %s\n", Status.firstPtsPassed ? "TRUE" : "FALSE");
       printf("Status.numDecodeErrors = %d\n", Status.numDecodeErrors);
       printf("Status.muted = %s\n", Status.muted ? "TRUE" : "FALSE");
       printf("Status.format = %d\n", Status.format);
       printf("Status.frameRate = %d\n", Status.frameRate);
       printf("Status.started = %s\n", Status.started ? "TRUE" : "FALSE");
       printf("Status.numPicturesReceived = %d\n", Status.numPicturesReceived);
       printf("Status.numDecoded = %d\n", Status.numDecoded);
       printf("Status.numDecodeErrors = %d\n", Status.numDecodeErrors);
       printf("Status.numDisplayed = %d\n", Status.numDisplayed);

	   printf("Status.fifoDepth = 0x%x\n",Status.fifoDepth);
	   
    }

    check_status++;
#endif

    if( (bufferSize == 0) || (dataBuffer == NULL) )
    {
        BDBG_ERR(("%s:%d Invalid Args BufferSize=%u dataBuffer=%p", __FUNCTION__, __LINE__, bufferSize, dataBuffer));
        CARD_PLAYBACK_RELEASE_MUTEX;
        return(CARD_PlaybackInvalidArg);
    }
	
	
		

    if( m_mutex == NULL )
    {
        BDBG_ERR(("%s:%d constructor failed!!", __FUNCTION__, __LINE__));
        CARD_PLAYBACK_RELEASE_MUTEX;
        return(CARD_PlaybackUninitialized);
    }
	
	
	
	/*if(keyslot_type==CARD_EncryptKeyType_EVEN)
		DRM_Widevine_getKeySlot(NEXUS_SecurityKeyType_eEven, keySlotHndl);
	else if (keyslot_type==CARD_EncryptKeyType_ODD)
		DRM_Widevine_getKeySlot(NEXUS_SecurityKeyType_eEven, keySlotHndl);
	else
	{
		 BDBG_ERR(("%s:%d Invalid KeySlot Type!!", __FUNCTION__, __LINE__));
		 return CARD_PlaybackFailure;
	}*/

	 /*allocate a continuous physical address buffer for DMA */
	BDBG_MSG(("\n %s:allocate a continuous physical address buffer for DMA ",__FUNCTION__ ));
	
	int i=0;uint8_t j=0;
	if(bufferSize%128){
		
		BDBG_MSG(("\nbufferSize is 0x%x, alloc sz is 0x%x",bufferSize,bufferSize+(128-(bufferSize%128))));
		nexus_rc = NEXUS_Memory_Allocate( bufferSize+(128-(bufferSize%128)), NULL,(void **) &dmaBuf_Src);
		//BKNI_Memset(dmaBuf_Src,0xDD,temp);
	}
	else
		nexus_rc =NEXUS_Memory_Allocate( bufferSize, NULL,(void **) &dmaBuf_Src);
	if(nexus_rc != NEXUS_SUCCESS){
			BDBG_ERR(("\n Nexus memory alloc failed"));
			CARD_PLAYBACK_RELEASE_MUTEX;
			return CARD_PlaybackFailure;
	}
	
	
	/*copy content to DMA buffer*/
	
	BDBG_MSG(("\n %s:copy content to DMA buffer ",__FUNCTION__ ));
    BKNI_Memcpy(dmaBuf_Src, dataBuffer, bufferSize);

	if ((nexus_rc = NEXUS_Playpump_GetBuffer(m_resources->playpump[0], &buffer, &local_bufferSize)) != 0)
    {
        BDBG_MSG(("%s(): playpump getbuf failed, nexus_rc = %d", __FUNCTION__, nexus_rc));
		CARD_PLAYBACK_RELEASE_MUTEX;
        return CARD_PlaybackFailure;
    }
	
    if( (local_bufferSize == 0) || (local_bufferSize < bufferSize))
    {
       if(local_bufferSize != 0)

       {
		  
          nexus_rc = NEXUS_Playpump_ReadComplete(m_resources->playpump[0], local_bufferSize, 0);
       }
	   
	   NEXUS_Memory_Free((void*)dmaBuf_Src);
       CARD_PLAYBACK_RELEASE_MUTEX;
       return CARD_PlaybackFull;
    }
    else
    {
       	BDBG_MSG(("\n%s: calling DRM_Widevine_Decrypt...sz is 0x%x",__FUNCTION__, bufferSize-(bufferSize%16)));
		drm_rc=DRM_Widevine_Decrypt(dmaBuf_Src,
                     bufferSize-(bufferSize%16), 
                    (uint8_t*)buffer, 
                    iv0, 
                    DrmDestinationType_eExternal, 
                    DrmSecurityKeyType_eEven);
		
	if(drm_rc != Drm_Success)
	{
		BDBG_ERR(("\n widevine decrypt failed..."));
		CARD_PLAYBACK_RELEASE_MUTEX;
		return CARD_PlaybackFailure;
	}

	//if any bytes left in clear, copy them also
	if(bufferSize%16)
	{

		BDBG_MSG(("\ncopy bytes left in clear sz:0x%x from 0x%x",(bufferSize%16),(bufferSize-(bufferSize%16))));
		BKNI_Memcpy(((uint8_t*)(buffer)+(bufferSize-(bufferSize%16))), (uint8_t*)(dmaBuf_Src+(bufferSize-(bufferSize%16))),(bufferSize%16));
	}
		
       nexus_rc = NEXUS_Playpump_WriteComplete(m_resources->playpump[0], 0, bufferSize);
	   if(nexus_rc!=0){
			BDBG_ERR(("\nNEXUS_Playpump_ReadComplete return code is %d",nexus_rc));
			CARD_PLAYBACK_RELEASE_MUTEX;
			return CARD_PlaybackFailure;
	   }
	  
		  

      
    m_bufferCallback(dataBuffer);
	NEXUS_Memory_Free((void*)dmaBuf_Src);
	dmaBuf_Src=NULL;
	 
    }

    CARD_PLAYBACK_RELEASE_MUTEX;
	
	return rc;
}

CARD_PlaybackStatus CARD_PlaybackDevice::feedData(void *dataBuffer, uint32_t  bufferSize)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_Error nexus_rc = NEXUS_SUCCESS;
   BERR_Code result = BERR_SUCCESS;
   void *buffer = NULL;
   size_t local_bufferSize;

#ifdef PRINT_VIDEO_DECODER_STATUS
   static int check_status = 1;
#endif
   //printf("\nEntered clear %s\n",__FUNCTION__);
   CARD_PLAYBACK_ACQUIRE_MUTEX;

#ifdef PRINT_VIDEO_DECODER_STATUS
    if((check_status % 500) == 0)
    {
        NEXUS_VideoDecoderStatus Status;
        uint32_t stc;

       NEXUS_VideoDecoder_GetStatus(m_videoDecoder, &Status);
       NEXUS_StcChannel_GetStc(m_videoProgram.stcChannel, &stc);

       printf("decode %dx%d, pts %#x, stc %#x (diff %d)\n", Status.source.width, Status.source.height, Status.pts, stc, Status.pts - stc);

       printf("Status.firstPtsPassed = %s\n", Status.firstPtsPassed ? "TRUE" : "FALSE");
       printf("Status.numDecodeErrors = %d\n", Status.numDecodeErrors);
       printf("Status.muted = %s\n", Status.muted ? "TRUE" : "FALSE");
       printf("Status.format = %d\n", Status.format);
       printf("Status.frameRate = %d\n", Status.frameRate);
       printf("Status.started = %s\n", Status.started ? "TRUE" : "FALSE");
       printf("Status.numPicturesReceived = %d\n", Status.numPicturesReceived);
       printf("Status.numDecoded = %d\n", Status.numDecoded);
       printf("Status.numDecodeErrors = %d\n", Status.numDecodeErrors);
       printf("Status.numDisplayed = %d\n", Status.numDisplayed);
	  // getchar();
    }

    check_status++;
#endif

    if( (bufferSize == 0) || (dataBuffer == NULL) )
    {
        BDBG_ERR(("%s:%d Invalid Args BufferSize=%u dataBuffer=%p", __FUNCTION__, __LINE__, bufferSize, dataBuffer));
        CARD_PLAYBACK_RELEASE_MUTEX;
        return(CARD_PlaybackInvalidArg);
    }

    if( m_mutex == NULL )
    {
        BDBG_ERR(("%s:%d constructor failed!!", __FUNCTION__, __LINE__));
        CARD_PLAYBACK_RELEASE_MUTEX;
        return(CARD_PlaybackUninitialized);
    }

    if ((nexus_rc = NEXUS_Playpump_GetBuffer(m_resources->playpump[0], &buffer, &local_bufferSize)) != 0)
    {
        BDBG_MSG(("%s(): playpump getbuf failed, nexus_rc = %d", __FUNCTION__, nexus_rc));
        CARD_PLAYBACK_RELEASE_MUTEX;
        return CARD_PlaybackFailure;
    }

    if( (local_bufferSize == 0) || (local_bufferSize < bufferSize))
    {
       if(local_bufferSize != 0)
       {

          nexus_rc = NEXUS_Playpump_ReadComplete(m_resources->playpump[0], local_bufferSize, 0);
       }

       CARD_PLAYBACK_RELEASE_MUTEX;

       return CARD_PlaybackFull;
    }
    else
    {
       BKNI_Memcpy((void *)buffer, dataBuffer, bufferSize);
       nexus_rc = NEXUS_Playpump_ReadComplete(m_resources->playpump[0], 0, bufferSize);
	    writeDebugFiles((uint8_t *)dataBuffer, bufferSize );
       // call the buffer released callback here
		//BDBG_MSG(("\n calling m_bufferCallback from %s",__FUNCTION__ ));
       m_bufferCallback(dataBuffer);
    }

    CARD_PLAYBACK_RELEASE_MUTEX;

    return rc;
}


CARD_PlaybackStatus CARD_PlaybackDevice::setSpeed(int32_t speed)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;

   CARD_DecodeSettings videoDecodeSettings;

   BDBG_MSG(("%s(): enter", __FUNCTION__));
   BDBG_MSG(("%s(): speed = %d\n", __FUNCTION__, speed));

   if(speed < 0)
   {
      // reverse playback not supported

      BDBG_MSG(("%s(): CARD_PlaybackInvalidArg\n", __FUNCTION__));
      BDBG_MSG(("%s()\n", __FUNCTION__));

      return CARD_PlaybackInvalidArg;
   }

   BDBG_MSG(("%s(): m_started = %s\n", __FUNCTION__, m_started ? "TRUE" : "FALSE"));

   if(!m_started)
   {

      BDBG_MSG(("%s(): getDefaultSettings()\n", __FUNCTION__));

      getDefaultSettings(&videoDecodeSettings);

      BDBG_MSG(("%s(): getDefaultSettings() done ...\n", __FUNCTION__));
      BDBG_MSG(("%s(): m_videoStreamNumber = 0x%x\n", __FUNCTION__, m_videoStreamNumber));
      BDBG_MSG(("%s(): m_audioStreamNumber = 0x%x\n", __FUNCTION__, m_audioStreamNumber));
      BDBG_MSG(("%s(): m_resConfig.videoCodec = 0x%x\n", __FUNCTION__, m_resConfig.videoCodec));
      BDBG_MSG(("%s(): videoDecodeSettings.transportType = %d\n", __FUNCTION__, videoDecodeSettings.transportType));

      videoDecodeSettings.videoPid = m_videoStreamNumber;
      videoDecodeSettings.videoCodec = m_resConfig.videoCodec;
      videoDecodeSettings.audioPid = m_audioStreamNumber;
      videoDecodeSettings.audioCodec = m_resConfig.audioCodec;
      videoDecodeSettings.videoPlaypump = m_resources->playpump[0];
      videoDecodeSettings.audioPlaypump = m_resources->playpump[0];
      startPlayback(&videoDecodeSettings);
   }

   if(speed == (int32_t) PAUSED_SPEED)
   {
      rc = pausePlayback();
   }
   else if(speed == (int32_t) NORMAL_SPEED)
   {

      BDBG_MSG(("%s(): m_paused = %s\n", __FUNCTION__, m_paused ? "TRUE" : "FALSE"));

      if(!m_paused)
      {
         rc = flush();
      }

      BDBG_MSG(("%s(): resume", __FUNCTION__));

      if(!m_firstPts)
      {
         NEXUS_AudioDecoderTrickState audioState;
         NEXUS_AudioDecoder_GetTrickState(m_audioDecoder, &audioState);
         audioState.rate = NEXUS_NORMAL_PLAY_SPEED;
         audioState.muted = true;
         NEXUS_AudioDecoder_SetTrickState(m_audioDecoder, &audioState);
      }
      else if(m_paused)
      {
         NEXUS_AudioDecoderTrickState audioState;
         NEXUS_AudioDecoder_GetTrickState(m_audioDecoder, &audioState);
         audioState.rate = NEXUS_NORMAL_PLAY_SPEED;
         audioState.muted = false;
         NEXUS_AudioDecoder_SetTrickState(m_audioDecoder, &audioState);
      }

      NEXUS_StcChannel_SetRate(m_stcChannel, 1, 0 );
      m_paused = false;

      // reset the data exhausted event variables  
      m_dataExhaustedEventPosted = false;
      m_dataExhausted = false;
   }

   BDBG_MSG(("%s(): leave", __FUNCTION__));

   return rc;
}

CARD_PlaybackStatus CARD_PlaybackDevice::endOfStream(void)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   if(!m_endOfStream)
   {
      BDBG_MSG(("%s(): called", __FUNCTION__));
      m_endOfStream = true;
   }

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return rc;
}

CARD_PlaybackStatus CARD_PlaybackDevice::getEvent(CARD_PlaybackEventID &eventID, uint64_t &eventParam1)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;

//printf("%s(): \n", __FUNCTION__);

   rc = (m_eventQueue->getEvent(eventID, eventParam1) ? CARD_PlaybackSuccess : CARD_PlaybackFailure);

//printf("%s()\n", __FUNCTION__);

   return rc;
}

CARD_PlaybackStatus CARD_PlaybackDevice::getPts(TIME45k& pts)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_VideoDecoderStatus decoderStatus;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   if(!m_started)
   {
      return CARD_PlaybackInvalidState;
   }

   NEXUS_Error nexus_rc = NEXUS_VideoDecoder_GetStatus(m_videoDecoder, &decoderStatus);

   if(nexus_rc != NEXUS_SUCCESS) 
   {
      nexus_rc = BERR_TRACE(nexus_rc); 
      decoderStatus.pts = 0;
   }

   if(decoderStatus.pts > 0x7FFFFFFF ) 
   {
      return CARD_PlaybackInvalidState;
   }

   pts = decoderStatus.pts;

//   BDBG_MSG(("%s(): %u (%u) ", __FUNCTION__, (unsigned)decoderStatus.pts, pts));

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return rc;
}

CARD_PlaybackStatus CARD_PlaybackDevice::flush(void)
{
    return flush(false);
}

CARD_PlaybackStatus CARD_PlaybackDevice::flush(bool hold_picture)
{
   CARD_PlaybackStatus rc = CARD_PlaybackSuccess;
   NEXUS_Error nexus_rc = NEXUS_SUCCESS;

   BDBG_MSG(("%s(): hold_picture = %s", __FUNCTION__, hold_picture ? "TRUE" : "FALSE"));

   if(!m_paused)
   {
      BDBG_MSG(("%s(): Pause during Flush", __FUNCTION__));	
      Pause();
   }

   nexus_rc = NEXUS_Playpump_Flush(m_resources->playpump[0]);

   if(rc != NEXUS_SUCCESS)
   {
      BDBG_MSG(("%s(): playpump[0] flush failed  %d", __FUNCTION__, nexus_rc));
      rc = CARD_PlaybackInvalidState;
   }

   NEXUS_VideoDecoder_Flush(m_videoDecoder);

   nexus_rc = NEXUS_AudioDecoder_Flush(m_audioDecoder);

   if(nexus_rc != NEXUS_SUCCESS)
   {
      BDBG_MSG(("%s(): audio decoder flush failed  %d", __FUNCTION__, nexus_rc));
      rc = CARD_PlaybackInvalidState;
   }

   if(m_audioPassthrough)
   {
      nexus_rc = NEXUS_AudioDecoder_Flush(m_audioPassthrough);

      if(nexus_rc != NEXUS_SUCCESS)
      {
         BDBG_MSG(("%s(): m_audioPassthrough flush failed  %d", __FUNCTION__, nexus_rc));
         rc = CARD_PlaybackInvalidState;
      }
   }

   NEXUS_AudioDecoder_Stop(m_audioDecoder);

   nexus_rc = NEXUS_AudioDecoder_Start(m_audioDecoder, &m_audioProgram);

   if(nexus_rc != NEXUS_SUCCESS)
   {
      BDBG_MSG(("%s(): AudioDecoder start failed  %d", __FUNCTION__, nexus_rc));
   }

   NEXUS_VideoDecoder_Stop(m_videoDecoder);

   nexus_rc = NEXUS_VideoDecoder_Start(m_videoDecoder, &m_videoProgram);

   if(nexus_rc != NEXUS_SUCCESS)
   {
      BDBG_MSG(("%s(): VideoDecoder start failed  %d", __FUNCTION__, nexus_rc));
   }

   m_endOfStream = false;
   m_videoFrameStepping = false;
   m_dataExhaustedEventPosted = false;
   m_endOfStreamEventPosted = false;
   m_lastPostedPresentationTimestamp = INVALID_TIMESTAMP;
   m_lastPictureCount = 0;
   m_playTime = 0;	
   m_firstPts = false;

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return rc;
}

const CARD_VideoCodec *CARD_PlaybackDevice::getVideoCodecs(uint32_t &count)
{
    static const CARD_VideoCodec supportedVideoCodecs[] =
    {
        CARD_VideoCodec_MPEG1  ,
        CARD_VideoCodec_MPEG2  ,
        CARD_VideoCodec_MPEG4P2,
        CARD_VideoCodec_DIVX311,
        CARD_VideoCodec_H264   ,
        CARD_VideoCodec_VC1    ,
        CARD_VideoCodec_VC1_SM
    };

    BDBG_MSG(("%s()\n", __FUNCTION__));

    count = sizeof(supportedVideoCodecs) / sizeof(CARD_VideoCodec); 

    BDBG_MSG(("%s()\n", __FUNCTION__));

    return(supportedVideoCodecs);
}

const CARD_AudioCodec *CARD_PlaybackDevice::getAudioCodecs(uint32_t &count)
{
    /* these are all supported in bdvd_rap_xxxx.c */
    static const CARD_AudioCodec supportedAudioCodecs[] =
    {
       CARD_AudioCodec_MPEG2_MC, 
       CARD_AudioCodec_MPEG,
       CARD_AudioCodec_MP3,
       CARD_AudioCodec_WMA,
       CARD_AudioCodec_PCM,     
       CARD_AudioCodec_AAC,
       CARD_AudioCodec_AAC_ADTS,
       CARD_AudioCodec_AAC_LOAS,
       CARD_AudioCodec_AC3,    
       CARD_AudioCodec_AC3_PLUS,
       CARD_AudioCodec_AC3_LOSSLESS,
       CARD_AudioCodec_AC3_PLUS_7_1,
       CARD_AudioCodec_LPCM_BLURAY,
       CARD_AudioCodec_LPCM_HDDVD,
       CARD_AudioCodec_LPCM_DVD, 
       CARD_AudioCodec_MLP,     
       CARD_AudioCodec_DTS,        
       CARD_AudioCodec_DTS_CD,     
       CARD_AudioCodec_DTS_HD,    
       CARD_AudioCodec_DTS_HD_XLL,
       CARD_AudioCodec_DTS_CA,    
       CARD_AudioCodec_DTS_LBR   
    };

    BDBG_MSG(("%s(): \n", __FUNCTION__));

    count = sizeof(supportedAudioCodecs) / sizeof(CARD_AudioCodec); 

    BDBG_MSG(("%s()\n", __FUNCTION__));

    return(supportedAudioCodecs);
}

const CARD_EncryptionType *CARD_PlaybackDevice::getEncryptionTypes(uint32_t &count)
{
    static const CARD_EncryptionType supportedEncryptTypes[] =
    {
        CARD_EncryptType_NONE,
        CARD_EncryptType_WMDRM,
        CARD_EncryptType_DIVX,
		CARD_EncryptType_WV_VOD
    };

    BDBG_MSG(("%s(): \n", __FUNCTION__));

    count = sizeof(supportedEncryptTypes) / sizeof(CARD_EncryptionType); 

    BDBG_MSG(("%s()\n", __FUNCTION__));

    return(supportedEncryptTypes);
}
//#define BRCM_COPY_PROTECTION_ENABLED 0
CARD_PlaybackStatus CARD_PlaybackDevice::setOutputSecurity(CARD_PlaybackSecurity &security)
{
    BDBG_MSG(("%s(): This is DTV implementation:\n", __FUNCTION__));
#if BRCM_COPY_PROTECTION_ENABLED

	NEXUS_DisplayVbiSettings nDisplayVbiSettings;
	NEXUS_Error nexus_rc;
	bool retErr=false;
	NEXUS_DisplayMacrovisionType macrovisionType;
	//printf("%s(): This is macrovision Level is : %d\n", __FUNCTION__,security.macrovision);
	switch(security.macrovision)
	{
	
	case	0:
		macrovisionType=NEXUS_DisplayMacrovisionType_eNone;
		break;
	case 1:
		macrovisionType=NEXUS_DisplayMacrovisionType_eAgcOnly;
		break;
		
	case 2:
		macrovisionType= NEXUS_DisplayMacrovisionType_eAgc2Lines;
		break;
		
	case 3:
		macrovisionType=  NEXUS_DisplayMacrovisionType_eAgc4Lines;
		break;
	case 4:
		macrovisionType=  NEXUS_DisplayMacrovisionType_eAgc4Lines;
		break;
	default:
		printf("%s(): Invalid macrovision Level , setting to NONE :\n", __FUNCTION__);
		macrovisionType=NEXUS_DisplayMacrovisionType_eNone;
		break;
	
    }

    if(security.macrovision) //set up macrovision
	{
		printf("%s(): SetUp Macroviison :\n", __FUNCTION__);
		nexus_rc = NEXUS_Display_SetMacrovision(m_display,macrovisionType,NULL);
		if ( nexus_rc==NEXUS_NOT_SUPPORTED  ) {
			printf("\nMacrovision not supported");
			retErr=true;
					//return CARD_PlaybackNotSupported;
		}else if(nexus_rc)
		{
			printf("\nMacrovision setup failed");
			retErr=true;
			//return CARD_PlaybackFailure;
		}
		
		if(!retErr)
		{
			printf("%s(): Set MacroVision enabled in VBI settings :\n", __FUNCTION__);
			NEXUS_Display_GetVbiSettings(m_display,&nDisplayVbiSettings);
            nDisplayVbiSettings.macrovisionEnabled =  (macrovisionType==NEXUS_DisplayMacrovisionType_eNone)?0:1;
            nexus_rc = NEXUS_Display_SetVbiSettings(m_display,&nDisplayVbiSettings);
            if ( nexus_rc )			{
				
				printf("\n Failed to enable Macrovision in VBI settings");
				retErr=true;
				//return CARD_PlaybackFailure;
			}
		}
	}

	if(security.copyManagement) //setup cgms
	{
		printf("%s(): Set CGMS :\n", __FUNCTION__);
		NEXUS_Display_GetVbiSettings(m_display,&nDisplayVbiSettings);
		nDisplayVbiSettings.cgmsRouting=false;
		nDisplayVbiSettings.cgmsEnabled=true;
		nexus_rc=NEXUS_Display_SetVbiSettings(m_display, &nDisplayVbiSettings);
		if ( nexus_rc ){
			printf("\n Fail to setup cgms in VBI settings");
			retErr=true;
			//return CARD_PlaybackFailure;
		}
		nexus_rc= NEXUS_Display_SetCgms(m_display, security.copyManagement);
		if ( nexus_rc ){
			printf("\n Fail to setup cgms ");
			retErr=true;
			//return CARD_PlaybackFailure;
		}

	}
	if(security.securityMode) //setup hdcp
	{
	  //HDCP is set in nexusMgr
	}
	if(security.ict) //setup MPAA
	{
	
	}
//#endif //BRCM_COPY_PROTECTION_ENABLED
if ( retErr ) 
	return CARD_PlaybackFailure;
else
#endif //BRCM_COPY_PROTECTION_ENABLED
	return CARD_PlaybackSuccess;
}


void CARD_PlaybackDevice::getDefaultSettings(CARD_DecodeSettings* pSettings)
{
   BDBG_MSG(("%s(): \n", __FUNCTION__));

   BKNI_Memset(pSettings, 0, sizeof(*pSettings));
   pSettings->transportType = m_resConfig.transportType;

   BDBG_MSG(("%s()\n", __FUNCTION__));
}

void CARD_PlaybackDevice::getVideoPidChannelStatus(CARD_PidChannelStatus *pidStatus)
{
	BDBG_MSG(("%s(): \n", __FUNCTION__));
	BKNI_Memset(pidStatus, 0, sizeof(*pidStatus));
	NEXUS_PidChannel_GetStatus (m_videoProgram.pidChannel, &pidStatus->pidStatus);

	BDBG_MSG(("%s()\n", __FUNCTION__));
}

void CARD_PlaybackDevice::getAudioPidChannelStatus(CARD_PidChannelStatus *pidStatus)
{
	BDBG_MSG(("%s(): \n", __FUNCTION__));
	BKNI_Memset(pidStatus, 0, sizeof(*pidStatus));
	NEXUS_PidChannel_GetStatus (m_audioProgram.pidChannel, &pidStatus->pidStatus);

	BDBG_MSG(("%s()\n", __FUNCTION__));
}

// *****************************************************************************
// *****************************   Private members  *******************************
// *****************************************************************************
NEXUS_Error CARD_PlaybackDevice::startPlayback(CARD_DecodeSettings *pSettings)
{
   NEXUS_Error rc = NEXUS_SUCCESS;
   NEXUS_VideoInputSettings videoInputSettings;
   NEXUS_VideoDecoderSettings videoDecoderSettings;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

#if 1
   // Init automatically if this is the first time we're called
   if ( false == m_initialized)
   {
      rc = initPlayback();

      if(rc)
      {
         return BERR_TRACE(rc);
      }
   }
#endif

   BDBG_MSG(("%s(): NEXUS_Playpump_Start(m_resources->playpump[0] = 0x%08lx)\n", __FUNCTION__, m_resources->playpump[0]));

   NEXUS_Playpump_Start(m_resources->playpump[0]);

   BDBG_MSG(("%s(): NEXUS_Playpump_Start(m_resources->playpump[0] = 0x%08lx) done ...\n", __FUNCTION__, m_resources->playpump[0]));
   BDBG_MSG(("%s(): NEXUS_VideoInput_GetSettings()\n", __FUNCTION__));

#if NEXUS_DTV_PLATFORM
#ifndef PLAYBACKDEVICE_STAND_ALONE_APP
   rc = NEXUS_VideoInput_SetResumeMode(NEXUS_VideoDecoder_GetConnector(m_videoDecoder), NEXUS_VideoInputResumeMode_eManual);
#endif
#endif // NEXUS_DTV_PLATFORM
   NEXUS_VideoDecoder_GetSettings(m_videoDecoder, &videoDecoderSettings);

   BDBG_MSG(("%s(): videoDecoderSettings.freeze = %d\n", __FUNCTION__, videoDecoderSettings.freeze));

   // we need to keep the last picture to avoide black screen flash at rew/ff time
   videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
   videoDecoderSettings.firstPtsPassed.callback = CARD_PlaybackDevice::firstPTS;
   videoDecoderSettings.firstPtsPassed.context = this;
   NEXUS_VideoDecoder_SetSettings(m_videoDecoder, &videoDecoderSettings);

   NEXUS_VideoDecoder_GetDefaultStartSettings(&m_videoProgram);
   m_videoProgram.stcChannel = m_stcChannel;
   m_videoProgram.codec = pSettings->videoCodec;
   m_videoProgram.pidChannel = m_resources->videoPidChannel;	

   NEXUS_AudioDecoder_GetDefaultStartSettings(&m_audioProgram);
   m_audioProgram.codec = pSettings->audioCodec;
   m_audioProgram.stcChannel = m_stcChannel;
   m_audioProgram.pidChannel = m_resources->audioPidChannel;

   BDBG_MSG(("%s(): pSettings->videoPid = 0x%x\n", __FUNCTION__, pSettings->videoPid ));

   // Start Decoders
   if ( pSettings->videoPid )
   {
      BDBG_MSG(("%s(): video decoder start 2", __FUNCTION__));

      rc = NEXUS_VideoDecoder_Start(m_videoDecoder, &m_videoProgram);

      BDBG_MSG(("%s(): video decoder start 3", __FUNCTION__));

      if(rc)
      {
         return BERR_TRACE(rc);
      }
   }

   BDBG_MSG(("%s(): pSettings->audioPid = 0x%x\n", __FUNCTION__, pSettings->audioPid ));

   if( pSettings->audioPid )
   {
      rc = NEXUS_AudioDecoder_Start(m_audioDecoder, &m_audioProgram);

      if(rc)
      {
         return BERR_TRACE(rc);
      }

      if ( m_audioPassthrough )
      {
         // Only pass through AC3
         rc = NEXUS_AudioDecoder_Start(m_audioPassthrough, &m_audioProgram);

         if(rc)
         {
            return BERR_TRACE(rc);
         }
      }
   }

   m_started = true;

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return NEXUS_SUCCESS;
}

void CARD_PlaybackDevice::stopPlayback(void)
{
   NEXUS_VideoDecoderSettings videoDecoderSettings;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   if ( false == m_started)
   {
       return;
   }

   m_started = false;

   NEXUS_Playpump_Stop(m_resources->playpump[0]);

   // make sure the last picture is muted when we stop playback
   NEXUS_VideoDecoder_GetSettings(m_videoDecoder, &videoDecoderSettings);
   videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eMute;
   NEXUS_VideoDecoder_SetSettings(m_videoDecoder, &videoDecoderSettings);

   NEXUS_VideoDecoder_Stop(m_videoDecoder);

   // Enable dynamic RTS again for other channel changes
#if NEXUS_DTV_PLATFORM
   NEXUS_VideoInput_SetResumeMode(NEXUS_VideoDecoder_GetConnector(m_videoDecoder), NEXUS_VideoInputResumeMode_eAuto);
#endif
   NEXUS_AudioDecoder_Stop(m_audioDecoder);

   if(m_audioPassthrough)
   {
      NEXUS_AudioDecoder_Stop(m_audioPassthrough);
   }

   BDBG_MSG(("%s()\n", __FUNCTION__));

   return;
}

void CARD_PlaybackDevice::firstPTS(void *context, int param)
{
	CARD_PlaybackDevice *device = (CARD_PlaybackDevice *) context;
	NEXUS_AudioDecoderTrickState audioState;
	NEXUS_Error rc = NEXUS_SUCCESS;

       BDBG_MSG(("%s(): \n", __FUNCTION__));

	BSTD_UNUSED(param);

	CARD_PlaybackDevice *thisd = (CARD_PlaybackDevice *) context;

	NEXUS_AudioDecoder_GetTrickState(thisd->m_audioDecoder, &audioState);

	audioState.rate = NEXUS_NORMAL_PLAY_SPEED;
	audioState.muted = false;

	rc = NEXUS_AudioDecoder_SetTrickState(thisd->m_audioDecoder, &audioState);

	device->m_firstPts = true;

	BDBG_MSG(("%s(): callback rc = %d ", __FUNCTION__, rc));
	BDBG_MSG(("%s()\n", __FUNCTION__));

	return;
}

// no need for this callback. It is already registered in CARD source
#if 0  
void CARD_PlaybackDevice::sourcePendingCallback(void *pContext, int param) 
{
   BSTD_UNUSED(param);
   NEXUS_VideoInput input = (NEXUS_VideoInput) pContext;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   BDBG_MSG(("set input mode to NEXUS_VideoInputResumeMode_eFreezeRts"));

   // Freeze RTS using the current format settings
   NEXUS_VideoInput_SetResumeMode(input, NEXUS_VideoInputResumeMode_eFreezeRts);

   BDBG_MSG(("%s()\n", __FUNCTION__));
}
#endif

void CARD_PlaybackDevice::dataExhausted(void *context, int param)
{
   BSTD_UNUSED(param);
   CARD_PlaybackDevice *thisd = (CARD_PlaybackDevice *) context;

   if( thisd->m_playTime > 1000)
   {
      thisd->m_dataExhausted = true;
   }

   BDBG_MSG(("%s(): %d ", __FUNCTION__, param));

   return;
}

NEXUS_Error CARD_PlaybackDevice::initPlayback(void)
{
   NEXUS_Error rc = NEXUS_SUCCESS;

   BDBG_MSG(("%s(): \n", __FUNCTION__));

   if(m_initialized)
   {
      return -1;
   }

   // Mark S. we probably need only one event
   BKNI_CreateEvent(&m_videoEvent);

   BDBG_MSG(("%s(): m_videoEvent = 0x%x\n", __FUNCTION__, m_videoEvent));

   NEXUS_CALLBACKDESC_INIT(&m_resConfig.videoDataCallback);
   m_resConfig.videoDataCallback.context = m_videoEvent;
   m_resConfig.videoDataCallback.callback = playpumpDataCallback;

   BDBG_MSG(("%s(): m_videoEvent = 0x%x\n", __FUNCTION__, m_videoEvent));

   NEXUS_CALLBACKDESC_INIT(&m_resConfig.fifoEmptyCallback);
   m_resConfig.fifoEmptyCallback.callback = dataExhausted;
   m_resConfig.fifoEmptyCallback.context = this;

   BDBG_MSG(("%s(): m_resConfig.videoDataCallback.context = 0x%x\n", __FUNCTION__, m_resConfig.videoDataCallback.context));

   m_resources = BcmNexus_StreamPlayer_Resources_Acquire(&m_resConfig);

   if(!m_resources)
   {
      BDBG_MSG(("initPlayback() : failed"));
      return -1;
   }
	
   m_videoDecoder = m_resources->videoDecoder;
   m_audioDecoder = m_resources->audioDecoder[0];
   m_audioPassthrough = m_resources->audioDecoder[1];
   m_stcChannel = m_resources->stcChannel;
   m_videoWindow = m_resources->videoWindow;	
   m_videoPidChannel = m_resources->videoPidChannel;
   m_audioPidChannel = m_resources->audioPidChannel;
#if BRCM_COPY_PROTECTION_ENABLED
   m_display = m_resources->display;
#endif
   BDBG_MSG(("%s(): m_resources->videoWindow = 0x%08x", __FUNCTION__, (unsigned int) m_resources->videoWindow));

   // Enable video window
   NEXUS_VideoWindowSettings windowSettings;

   BDBG_MSG(("%s(): NEXUS_VideoWindow_GetSettings(m_videoWindow = 0x%08x)", __FUNCTION__, m_videoWindow));

   NEXUS_VideoWindow_GetSettings(m_videoWindow, &windowSettings);

   BDBG_MSG(("%s(): NEXUS_VideoWindow_GetSettings() done ...", __FUNCTION__));

   windowSettings.visible = true;
   rc = NEXUS_VideoWindow_SetSettings(m_videoWindow, &windowSettings);

   BDBG_MSG(("%s(): NEXUS_VideoWindow_SetSettings() done ...", __FUNCTION__));
   
   BDBG_MSG(("%s(): callback rc = %d ", __FUNCTION__, rc));

   m_initialized = true;

   return rc;
}

void CARD_PlaybackDevice::playpumpDataCallback(void *context, int param)
{
	BSTD_UNUSED(param);

//printf("%s(): BKNI_SetEvent(): context = 0x%x\n", __FUNCTION__, context);

	BKNI_SetEvent((BKNI_EventHandle)context);	

//printf("%s(): BKNI_SetEvent() done ...\n", __FUNCTION__);

}

CARD_PlaybackStatus CARD_PlaybackDevice::pausePlayback(void)
{
   NEXUS_Error rc = NEXUS_SUCCESS;
   NEXUS_AudioDecoderTrickState audioState;

   BDBG_MSG(("%s(): ", __FUNCTION__));

   rc = NEXUS_StcChannel_SetRate(m_stcChannel, 0, 0 );

   if(rc!=NEXUS_SUCCESS) 
   {
      BDBG_MSG(("%s(): Pause failed", __FUNCTION__));

      return CARD_PlaybackFailure;
   }

   m_paused = true;

   NEXUS_AudioDecoder_GetTrickState(m_audioDecoder, &audioState);
   audioState.rate = 0;
   audioState.muted = true;
   rc = NEXUS_AudioDecoder_SetTrickState(m_audioDecoder, &audioState);

   BDBG_MSG(("%s()", __FUNCTION__));

   return CARD_PlaybackSuccess;
}

void CARD_PlaybackDevice::videoDisplayTask(void *context)
{
   CARD_PlaybackDevice *device = (CARD_PlaybackDevice *) context;

   BDBG_MSG(("%s(): \n", __FUNCTION__));
   BDBG_MSG(("%s(): device->m_runningDeviceTasks = %s\n", __FUNCTION__, device->m_runningDeviceTasks ? "TRUE" : "FALSE"));
   BDBG_MSG(("%s(): device->m_paused = %s\n", __FUNCTION__, device->m_paused ? "TRUE" : "FALSE"));
   BDBG_MSG(("%s(): device->m_firstPts = %s\n", __FUNCTION__, device->m_firstPts ? "TRUE" : "FALSE"));

   while(device->m_runningDeviceTasks)
   {
      if(!device->m_paused)
      {
         device->postPlaybackEvents();
      }

      B_Thread_Sleep(100);
      device->m_playing = (device->m_firstPts && !device->m_paused);
   }

   BDBG_MSG(("%s() \n", __FUNCTION__));
}

void CARD_PlaybackDevice::postPlaybackEvents(void)
{
   uint64_t pts = getCurrentPTSin_ms();

//printf("%s(): \n", __FUNCTION__);

   if(pts == INVALID_TIMESTAMP)
   {
      pts = 0;

      return;
   }

   if(m_endOfStream)
   {
      if(pts == m_playTime && pts != 0 )//&& not pause//sri
      {
         if(m_lastPictureCount++ > 3)
         {
            m_lastPicture = true;
         }
      }
   }

   if(pts != m_playTime)
   {
      BDBG_MSG(("%s(): pts: %llu  ", __FUNCTION__, pts));

      if(m_eventQueue->postEvent(CARD_PlaybackEvent_PTS_UPDATE, pts))
      {
         m_playTime = pts;
      }
   }

   if(m_lastPicture && !m_endOfStreamEventPosted)
   {
      if(m_eventQueue->postEvent(CARD_PlaybackEvent_END_OF_STREAM))
      {
         m_lastPicture = false;
         m_endOfStreamEventPosted = true;
      }
   }

   if( m_dataExhausted && !m_dataExhaustedEventPosted && pts != 0)
   {
      //add the case of - do not post in pause case. //sri
      if(m_eventQueue->postEvent(CARD_PlaybackEvent_DATA_EXHAUSTED))
      {
         m_dataExhausted = false;
         m_dataExhaustedEventPosted = true;		
      }
   }

//printf("%s()\n", __FUNCTION__);

}

uint64_t CARD_PlaybackDevice::getCurrentPTSin_ms(void)
{
   NEXUS_VideoDecoderStatus decoderStatus;
   uint64_t pts = 0;

   if(!m_started)
   {
      return INVALID_TIMESTAMP;
   }

   NEXUS_Error rc = NEXUS_VideoDecoder_GetStatus(m_videoDecoder, &decoderStatus);

   if(rc != NEXUS_SUCCESS) 
   {
      rc = BERR_TRACE(rc); 
      decoderStatus.pts = 0;
   }

   if(decoderStatus.pts > 0x7FFFFFFF ) 
   {
      return INVALID_TIMESTAMP;
   }

   if(decoderStatus.ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS)
   {
      return m_playTime;
   }

   // convert from 45KHz units to msec
   pts = (uint64_t)(m_preroll + (uint32_t)(decoderStatus.pts/(uint32_t)45));

   BDBG_MSG(("%s(): %llu (%u) ", __FUNCTION__, (unsigned)decoderStatus.pts, pts));

   return  pts;
}

CARD_PlaybackStatus CARD_PlaybackDevice::Pause()
{
   NEXUS_Error nexus_rc = NEXUS_SUCCESS;
   NEXUS_AudioDecoderTrickState audioState;

   BDBG_MSG(("%s(): ", __FUNCTION__));

   nexus_rc = NEXUS_StcChannel_SetRate(m_stcChannel, 0, 0 );

   if(nexus_rc != NEXUS_SUCCESS) 
   {
      BDBG_MSG(("%s(): Pause failed", __FUNCTION__));

      return CARD_PlaybackFailure;
   }

   m_paused = true;

   NEXUS_AudioDecoder_GetTrickState(m_audioDecoder, &audioState);

   audioState.rate = 0;
   audioState.muted = true;

   nexus_rc = NEXUS_AudioDecoder_SetTrickState(m_audioDecoder, &audioState);

   return CARD_PlaybackSuccess;
}

void CARD_PlaybackDevice::convertParams(CARD_StreamType   streamType, 
                                                                 CARD_PlaybackVideoParams *VideoParams,
                                                                 CARD_PlaybackAudioParams *AudioParams)

{
   //NEXUS manager usage by product application
   //memset(&m_resConfig, 0, sizeof(m_resConfig));
BKNI_Memset(&m_resConfig, 0, sizeof(m_resConfig));
   // convert stream type 
   switch(streamType)
   {
      case CARD_StreamType_MPEG2_TS:
      {
         m_resConfig.transportType = NEXUS_TransportType_eTs;
         break;
      }
      case CARD_StreamType_MPEG2_PS:
      {
         m_resConfig.transportType = NEXUS_TransportType_eMpeg2Pes;
         break;
      }
      default:
         printf("Unsupported format, how did it get here?\n");
	  break;
   }


   // convert audio codec type 
   switch(AudioParams->codec)
   {
      case CARD_AudioCodec_MPEG:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eMpeg;
         break;
      }
      case CARD_AudioCodec_MP3:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eMp3;
         break;
      }
      case CARD_AudioCodec_WMA:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eWmaStd;
         break;
      }
      case CARD_AudioCodec_AAC:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAac;
         break;
      }
      case CARD_AudioCodec_AAC_LOAS:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAacLoas;
         break;
      }
      case CARD_AudioCodec_AAC_ADTS:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAacAdts;
         break;
      }
      case CARD_AudioCodec_AC3:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAc3;
         break;
      }
      case CARD_AudioCodec_AC3_PLUS:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAc3Plus;
         break;
      }
      case CARD_AudioCodec_AC3_LOSSLESS:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_AC3_LOSSLESS\n");
         break;
      }
      case CARD_AudioCodec_DTS:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eDts;
         break;
      }
      case CARD_AudioCodec_LPCM_HDDVD:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eLpcmHdDvd;
         break;
      }
      case CARD_AudioCodec_LPCM_BLURAY:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eLpcmBluRay;
         break;
      }
      case CARD_AudioCodec_DTS_HD:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eDtsHd;
         break;
      }
      case CARD_AudioCodec_LPCM_DVD:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eLpcmDvd;
         break;
      }
      case CARD_AudioCodec_MLP:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_MLP\n");
         break;
      }
      case CARD_AudioCodec_DTS_HD_XLL:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_DTS_HD_XLL\n");
         break;
      }
      case CARD_AudioCodec_DTS_LBR:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_DTS_LBR\n");
         break;
      }
      case CARD_AudioCodec_AC3_PLUS_7_1:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_AC3_PLUS_7_1\n");
         break;
      }
      case CARD_AudioCodec_DTS_CD:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_DTS_CD\n");
         break;
      }
      case CARD_AudioCodec_DTS_CA:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eAc3;
         printf("Unsupported audio codec: CARD_AudioCodec_DTS_CA\n");
         break;
      }
      case CARD_AudioCodec_MPEG2_MC:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         printf("Unsupported audio codec: CARD_AudioCodec_MPEG2_MC\n");
         break;
      }
      case CARD_AudioCodec_PCM:
      {
         m_resConfig.audioCodec = NEXUS_AudioCodec_ePcm;
         break;
      }
      default:
      {
         printf("Unsupported audio codec\n");
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         break;
      }
   }

   // set the audio pid number
   if(streamType == CARD_StreamType_MPEG2_PS)
   {
      m_audioStreamNumber = AudioParams->pes_id;
   }
   else
   {
      m_audioStreamNumber = AudioParams->pid;
   }

   m_resConfig.audioPid = m_audioStreamNumber;

   // convert video codec type 
   switch(VideoParams->codec)
   {
      case CARD_VideoCodec_MPEG1:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eMpeg1;
         break;
      }
      case CARD_VideoCodec_MPEG2:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eMpeg2;
         break;
      }
      case CARD_VideoCodec_MPEG4P2:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eMpeg4Part2;
         break;
      }
      case CARD_VideoCodec_DIVX311:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eDivx311;
         break;
      }
      case CARD_VideoCodec_H264:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eH264;
         break;
      }
      case CARD_VideoCodec_VC1:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eVc1;
         break;
      }
      case CARD_VideoCodec_VC1_SM:
      {
         m_resConfig.videoCodec = NEXUS_VideoCodec_eVc1SimpleMain;
         break;
      }
      default:
      {
         printf("Unsupported video codec\n");
         m_resConfig.audioCodec = NEXUS_AudioCodec_eUnknown;
         break;
      }
   }

   // set the video pid number
   if(streamType == CARD_StreamType_MPEG2_PS)
   {
      m_videoStreamNumber = VideoParams->pes_id;
   }
   else
   {
      m_videoStreamNumber = VideoParams->pid;
   }

   m_resConfig.videoPid = m_videoStreamNumber;

   // the input stream is muxed therefore use the same 
   // playpump for both audio and video
   m_resConfig.numPlaypumps = 1;

   BDBG_MSG(("%s(): m_videoStreamNumber = %d\n", __FUNCTION__, m_videoStreamNumber));
   BDBG_MSG(("%s(): m_audioStreamNumber = %d\n", __FUNCTION__, m_audioStreamNumber));

   return;
}




