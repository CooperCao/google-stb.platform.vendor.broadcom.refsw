/***************************************************************************
 *     (c)2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: OTF test code
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#include "stdlib.h"
#include "dosFsLib.h"
#include "dpartCbio.h"
#include "usrFdiskPartLib.h"
#include "usrLib.h"
#include "dcacheCbio.h"
#include "dosFsLib.h"
#include <private/dosFsLibP.h>
#include <private/cbioLibP.h>
/* Magnum */
#include "bstd.h"
#include "bkni.h"
#include "bxpt_priv.h"
#include "bxpt.h"
#include "bxpt_rave.h"
#include "bxpt_directv_playback.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_xcbuff.h"
#include "bmem.h"
#include "bxvd.h"
#include "bxvd_pvr.h"
#include "bpvrlib_play.h"
#include "bxpt_xcbuf.h"

#include "vxWorks.h"
#include <ftpLib.h>


/* OTF */
#include "botf.h"

void oinit(uint32_t id);
void ostart(uint32_t id);
void ostop(uint32_t id);
void orate(uint32_t id, int rate);
void omode(uint32_t id, int mode);
void offilter(uint32_t id, int filter);
void oforward(uint32_t id);
void orewind(uint32_t id);
void ofname(uint32_t id, char *filename);
void ofpos(uint32_t id, int position);
void ochunksize(uint32_t id, int size);
void ovpid(uint32_t id, int vpid);
void ofjumpsize(uint32_t id, int size);
void prepareBTP(char *buf, uint16_t pid, uint32_t *params);

#if 0
#include "lex.yy.c"
#endif
BDBG_MODULE(OTFTEST);
#if 1
#if 0
#define VIDEOPID	0x1001
#define STREAMFILE	"/ata0/videos/3MBPS.ts"
#endif
#define VIDEOPID	0x901
#define STREAMFILE	"/ata0/videos/BBC1.ts"
#else
#define VIDEOPID	0x907
#define STREAMFILE	"/ata0/videos/test.ts"
#endif
#define USE_PVRLIB 1
#define DSS 0

#define		LCMPKTSEC			((512*188)/4)
/*
 * The purpose of the tests implemented in this file are as follows:
 *  . Implement real time scenarios as close as possible by using
 *    OTF and magnum
 *  . Implement utility functions such as playback captured OTF ES file.
 * 
 */
void ConfigureTSInputs(void);
BXPT_Handle GetBXPT(void);
BXPT_Rave_Handle GetBXPT_Rave(void);
uint32_t     os_GetTick(void);
uint32_t     os_GetTicksPerSec(void);

BERR_Code InitApp(void);
BERR_Code TestVideoDisplayForFormat(uint32_t uDecoder, BFMT_VideoFmt vidfmt, bool bInstallPTSCallbacks);
BXPT_Playback_Handle GetBXPT_Playback(uint32_t uIndex);
BXVD_Handle GetBXVD(uint32_t id);
BXVD_ChannelHandle GetBXVD_Channel( uint32_t id );
BMEM_Handle GetBMEM(void);
BREG_Handle GetBREG(void);
BINT_Handle GetBINT(void);
typedef struct otf_context {
    FILE *fd;
    BXPT_RaveCx_Handle	IPRaveContext;
    BXPT_RaveCx_Handle	OPRaveContext;
    BPVRlib_Play_Handle  pvr_play;	
    unsigned int PidChannel;
    BOTF_Config  Config;
    BOTF_Handle  hOtf;
    int videopid;
    int startoffset;
    int jumpsize;
    int unitsize;
    bool changeflag;
    bool stopstreamer;
    bool streamerstopped;
    bool initdone;
}otf_context_t;
otf_context_t otf_instances[2];


BXVD_DecodeSettings        sDecodeSettings;
BAVC_XptContextMap         sContextMap;
BOTF_TaskEntry             TaskEntry;
static void init_otf_test();
extern STATUS   usrAtaConfig	(int ctrl, int drive, char *fileName);
#define		NOOFPARTS		16

static CBIO_DEV_ID dosFsGetCbio(char *devName)
{
    DOS_VOLUME_DESC_ID pVolDesc;

    if(devName == NULL) devName = "/ata0";

    pVolDesc = dosFsVolDescGet(devName, NULL);
    if(pVolDesc == NULL)
	{
		printf("Error getting volume descriptor\n");
		return((CBIO_DEV_ID)ERROR);
	}
    return (pVolDesc->pCbio->cbioSubDev);
}

void setupDisk()
{
    /* If the application "quits" then gets run again, then we don't need to setup the disk again. */
    static int doOnce;
    if (doOnce) return; doOnce = 1;

    /* For Vxworks automount ata0 partition. 

       If you have not formatted the hard drive Please do the following command on the Tornado shell before continuing:
           ->usrAtaConfig(0,0,"/ata0")
           ->dosFsVolFormat("/ata0",0x312, 8192)

       If you want to create the videos directory manually under /ata0 do the command below (make sure your brutus.cfg
       has /ata0/videos as the VIDEOSDIR if you do the command below)  otherwise this function automatically creates it if
       it is not present.
           ->mkdir("/ata0/videos")
       
       That should format the drive then reset the board and the code below will automount the ata0 drive if you 
       created the videos dir it will also appear under /ata0.
    */
    usrAtaConfig(0,0,"/ata0");

    /* This code tunes and resizes the cache to 1MB */
    {
        CBIO_DEV_ID cbio;
        
        /* Decrease the bypass threshold value to less then 4k */

        cbio = (CBIO_DEV_ID) dosFsGetCbio("/ata0");
        dcacheDevMemResize(cbio,1048576);
        dcacheDevTune(cbio,0,128,0,-1);
    }

    {

        struct stat Stat;
        /* Make sure that we create a videos directory, if it does not already exist. */
        if (stat("/ata0/videos",&Stat) == -1)
            mkdir("/ata0/videos");
    }
}

/* otf instance 0 uses playback channel 0 and otf instance 1 will use playback channel 1 */
void setupOTFPipe(int id)
{
    BERR_Code      rc;   
	BAVC_CdbItbConfig sConfig;
	int32_t iPicBufLength;		
	BXPT_Playback_Handle xpt_play;
	BXPT_Rave_AvSettings AVCtxCfg;	    	
    BXPT_Playback_ChannelSettings xpt_config;
    BXPT_Playback_ParserConfig	   parser_config;       
	BXPT_Rave_EsRanges MPEG_EsRanges[BXPT_NUM_AV_RANGES] = {
					{ 0xAF, 0x01, true, true }, /* MPEG slices */                    
					{ 0xBF, 0xB0, false, true }, /* start of frame */
					{ 0x00, 0x00, false, true }, /* start of picture */
					{ 0x00, 0x00, false , false }}; /* disabled */	
	
	
 	/* Configure Playback channel and PB parser */
 	BXPT_Playback_GetChannelDefaultSettings(GetBXPT(), id, &xpt_config);
    xpt_play = GetBXPT_Playback(id);    
    BXPT_Playback_SetChannelSettings(xpt_play, &xpt_config);			    
    BXPT_Playback_GetParserConfig(xpt_play, &parser_config);
    parser_config.AllPass = false;
    parser_config.ContCountIgnore = true;     
    BXPT_Playback_SetParserConfig(xpt_play, &parser_config);  
    BXPT_AllocPidChannel(GetBXPT(), true /* need buffer */, &otf_instances[id].PidChannel);
 	BXPT_ConfigurePidChannel( GetBXPT(),otf_instances[id].PidChannel, otf_instances[id].videopid, BXPT_PB_PARSER(id) );
	BXVD_GetBufferConfig(GetBXVD(id), &sConfig, &iPicBufLength);
	
	sConfig.Cdb.Length = 8*1024*1024;	
	sConfig.Itb.Length = 1024*1024;
    	
 	// Alloc a AV context
    rc = BXPT_Rave_AllocContext(GetBXPT_Rave(), BXPT_RaveCx_eAv, &sConfig, &(otf_instances[id].IPRaveContext));    
	
	if (rc == BERR_SUCCESS)
	{
	    /* Get AV config */
		BXPT_Rave_GetAvConfig( otf_instances[id].IPRaveContext, &AVCtxCfg ); 
		AVCtxCfg.OutputFormat = BAVC_StreamType_eEs;
		AVCtxCfg.InputFormat = BAVC_StreamType_eTsMpeg;
		AVCtxCfg.BandHoldEn = AVCtxCfg.DisableContinuityCheck = true;
		AVCtxCfg.ItbFormat =  BAVC_ItbEsType_eOTFVideo;

		AVCtxCfg.StreamIdHi = 0xEF;
		AVCtxCfg.StreamIdLo = 0xE0;
		BKNI_Memcpy(&AVCtxCfg.EsRanges, &MPEG_EsRanges, sizeof(MPEG_EsRanges));
		
		/* Set AV config */
		rc = BXPT_Rave_SetAvConfig( otf_instances[id].IPRaveContext, &AVCtxCfg );
	}	
	
	BXPT_Rave_AddPidChannel(otf_instances[id].IPRaveContext, otf_instances[id].PidChannel, false);
 	BXPT_EnablePidChannel( GetBXPT(), otf_instances[id].PidChannel ); 	

 	/* Configure and start pvrlib */
    BPVRlib_Play_Open(&otf_instances[id].pvr_play, GetBXPT(), xpt_play, GetBMEM(),
			      GetBINT(), id /* playback channel */,
			      NULL, 1024 /* 4 MB */, 128 /* no of descriptors */, &BKNI_Memcpy);
	BXPT_Playback_SetBitRate(xpt_play, 108000000);
	BXPT_XcBuf_SetBandDataRate( GetBXPT(), BXPT_XcBuf_Id_RAVE_PBP0+id, 100000000, 188 );       
    
	sConfig.Cdb.Length = 1<<sConfig.Cdb.Alignment;	
	sConfig.Itb.Length = 1<<sConfig.Itb.Alignment;	
    	
 	// Alloc a AV context
    BXPT_Rave_AllocContext(GetBXPT_Rave(), BXPT_RaveCx_eAv, &sConfig, &(otf_instances[id].OPRaveContext));

// 	BXPT_Rave_EnableContext(hRaveContext1);
}

static void DiskStreamer(void *arg)
{	
    uint32_t id = (uint32_t) arg;
	char *buf=NULL;
	int offset;		
	int complete = 0;   	
	uint8_t *pkt;
	BERR_Code      rc;
	int startoffset, jumpsize, unitsize;
    BOTF_MarkerParams marker;
    int i;
    int descs;
    uint32_t params[10];
    uint32_t tag;    		

	BPVRlib_Play_Start(otf_instances[id].pvr_play);	
	pkt = BMEM_Alloc(GetBMEM(),188*2);	
	marker.ChunkCntr = 0;
	while (1)
	{   		
		if (otf_instances[id].stopstreamer)
		{
			if (buf!= NULL)
			{ 
				BMEM_Free(GetBMEM(),buf);
				buf = NULL;
			}			
			BPVRlib_Play_Stop(otf_instances[id].pvr_play);
			while(otf_instances[id].stopstreamer)
			{
				otf_instances[id].streamerstopped = true;
				BKNI_Sleep(10);			
			}
			BPVRlib_Play_Start(otf_instances[id].pvr_play);
		}		
        otf_instances[id].streamerstopped = false;
		/* Update local variables with global ones */
		if (otf_instances[id].changeflag)
		{
			startoffset = otf_instances[id].startoffset;
			offset = startoffset;
			unitsize = otf_instances[id].unitsize;
			jumpsize = otf_instances[id].jumpsize;            
			otf_instances[id].changeflag = false;									
		}
		if (buf == NULL) 
			buf = BMEM_Alloc(GetBMEM(),unitsize);

        if (offset < 0) {
            otf_instances[id].stopstreamer = true;
            continue;
        }
		fseek(otf_instances[id].fd, offset, SEEK_SET);
//		BKNI_Printf("Offset = %d unitsize = %d \n", offset, unitsize);
		for (i=0;i<NOOFPARTS;i++)
		{
			if ( fread(buf+(i*(unitsize/NOOFPARTS)), 1, unitsize/NOOFPARTS, otf_instances[id].fd) != unitsize/NOOFPARTS)
			{
				otf_instances[id].stopstreamer = true;
				break;				
			}
			/* Add buffer to play lib */
			rc = BPVRlib_Play_AddBuffer(otf_instances[id].pvr_play, buf+(i*(unitsize/NOOFPARTS)), unitsize/NOOFPARTS, BPVRlib_Play_BlockMode_eNone);
			if (rc != BERR_SUCCESS)
			{
				BKNI_Printf("BPVRlib_Play_AddBuffer failed with err %d \n", rc);
				while(1);
			} 		
		}
				
        descs = NOOFPARTS;
		offset += (unitsize+jumpsize);
		do 
		{	
			if (otf_instances[id].stopstreamer) break;
			BKNI_Sleep(10);			
			BPVRlib_Play_IsBufferComplete(otf_instances[id].pvr_play, &complete, BPVRlib_Play_BlockMode_eNone);
			descs -= complete;
			// BKNI_Printf("1 Complete = %d", complete);  
		} while (descs != 0);
        if (jumpsize != 0)
		{
            marker.ChunkCntr++;	  
			
			marker.Pid = otf_instances[id].videopid;
            BOTF_BuildMarkerTS(&marker,pkt);	
            rc = BPVRlib_Play_AddBuffer(otf_instances[id].pvr_play, pkt, 2*188, BPVRlib_Play_BlockMode_eNone);
			if (rc != BERR_SUCCESS)
			{
				BKNI_Printf("BPVRlib_Play_AddBuffer failed with err %d \n", rc);
				while(1);
			}			
			do 
			{	
				if (otf_instances[id].stopstreamer) break;
				BKNI_Sleep(10);				
				BPVRlib_Play_IsBufferComplete(otf_instances[id].pvr_play, &complete, BPVRlib_Play_BlockMode_eNone);
				// BKNI_Printf("2 Complete = %d", complete);
			} while (complete == 0);
		}
        /* Add tag BTP */
        params[0] = 13;
#if 0
        /* To enable display last/first I-frame set the following param to 1 */
        params[1] = 1;
#else
        params[1] = 0;
#endif
        params[2] = 0;
        params[3] = 0;
        params[4] = 0;
        params[5] = 0;
        params[6] = 0;
        params[7] = 0;
        params[8] = 188;
        params[9] = offset;
//        BKNI_Printf("botf_test offset = %#x \n", offset);
//        BXVD_GetPictureTag(GetBXVD_Channel(id), &tag);
//        BKNI_Printf("current tag = %#x \n", tag);

        prepareBTP(pkt, otf_instances[id].videopid, params);
#if 1
        rc = BPVRlib_Play_AddBuffer(otf_instances[id].pvr_play, pkt, 188, BPVRlib_Play_BlockMode_eNone);
        if (rc != BERR_SUCCESS)
        {
            BKNI_Printf("BPVRlib_Play_AddBuffer failed with err %d \n", rc);
            while(1);
        }			
        do 
        {	
            if (otf_instances[id].stopstreamer) break;
            BKNI_Sleep(10);				
            BPVRlib_Play_IsBufferComplete(otf_instances[id].pvr_play, &complete, BPVRlib_Play_BlockMode_eNone);
            // BKNI_Printf("2 Complete = %d", complete);
        } while (complete == 0);    
#endif
	}
}

int modes[] = {-30, -12, -6, -2, 2, 6, 12, 30};
void otf_regress_test(uint32_t id)
{
    uint32_t counter = 0;
    int randnum;
    
    oinit(id);

    /* Initially set to -2x  */
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);
    ofpos(id, id);
    ochunksize(id, 1925120);
    ofjumpsize(id, 0);
    omode(id, 1);
    orate(id, 200);    
    offilter(id, 2);
    oforward(id);      
    
    ostart(id);

    while(counter <= 1000)
    {	
		
		/* Sleep for 30 seconds */
		BKNI_Sleep(30000);
		ostop(id);
		/* Get a random number */
		randnum = rand(); 
        BKNI_Printf("ch%dTrick mode = %d \n", id,modes[randnum%8]);        
		switch(modes[randnum%8])
		{
			case -30:				
                ofpos(id, 721920000);
                ochunksize(id, 1925120);
                ofjumpsize(id, -5775360);
                omode(id, 3);
                orate(id, 100);    
                offilter(id, 1);
                orewind(id);
				break;
				
			case -12:				
                ofpos(id, 721920000);
                ochunksize(id, 1925120);
                ofjumpsize(id, -3850240);
                omode(id, 3);
                orate(id, 100);    
                offilter(id, 1);
                orewind(id);
				break;			
			case -6:				
                ofpos(id, 721920000);
                ochunksize(id, 1925120);
                ofjumpsize(id, -2887680);
                omode(id, 4);
                orate(id, 200);    
                offilter(id, 2);
                orewind(id);
				break;			
			case -2:				
                ofpos(id, 721920000);
                ochunksize(id, 1925120);
                ofjumpsize(id, -2887680);
                omode(id, 4);
                orate(id, 200);    
                offilter(id, 3);
                orewind(id);		
                break;	
            case 2:
                ofpos(id, 0);
                ochunksize(id, 1925120);
                ofjumpsize(id, 0);
                omode(id, 1);
                orate(id, 200);    
                offilter(id, 3);
                oforward(id);      
                break;

			case 6:				
                ofpos(id, 0);
                ochunksize(id, 1925120);
                ofjumpsize(id, 0);
                omode(id, 1);
                orate(id, 200);    
                offilter(id, 2);
                oforward(id);      
				break;			
			case 12:				
                ofpos(id, 0);
                ochunksize(id, 1925120);
                ofjumpsize(id, 0);
                omode(id, 1);
                orate(id, 100);    
                offilter(id, 1);
                oforward(id);      
				break;
			case 30:											
                ofpos(id, 0);
                ochunksize(id, 1925120);
                ofjumpsize(id, 0);
                omode(id, 1);
                orate(id, 200);    
                offilter(id, 1);
                oforward(id);      
                break;
		}                       		
        ostart(id);
		counter++;
    }
}

static void init_otf_test(void)
{
    static bool initDone = false;
    BERR_Code	status = BERR_SUCCESS;
    
	if (!initDone)
	{	
		/* Code from mainApp() */
		status = InitApp();
		BERR_TRACE(status);

//        BDBG_SetModuleLevel("BVDC_BUF", BDBG_eMsg);    
//        BDBG_SetModuleLevel("BVDC_PRIV", BDBG_eMsg);            
		/* Create the initial display */
        /* First display */
		TestVideoDisplayForFormat(0, BFMT_VideoFmt_e1080i, false);
#if BCHP_CHIP==7400
        /* Second display */
        TestVideoDisplayForFormat(1, BFMT_VideoFmt_eNTSC, false);	 
#endif

        setupDisk();
		
		initDone = true;		
	}
}

/* Functions used by the automated tester */
void oinit(uint32_t id)
{	
    BOTF_OpsParams OpsParams;
    BOTF_OpsHandle hOpsOtf;
    BOTF_Params Params;    
    uint32_t MaxChannels;        

	if (otf_instances[id].initdone) 
	{
		printf(" 0 SUCCESS\n");
		return;
	}
	
	init_otf_test();
    /* Setup OTF pipe */
    setupOTFPipe(id);    

//    BXVD_PVR_SetSlowMotionRate(GetBXVD_Channel(0), 10);
//    BDBG_SetModuleLevel("botf_gop_player", BDBG_eMsg);    
//    BDBG_SetModuleLevel("botf_gop_manager", BDBG_eMsg);
//    BDBG_SetModuleLevel("botf_scv_parser", BDBG_eMsg);
//    BDBG_SetModuleLevel("BOTF_PLATFORM", BDBG_eMsg);

    printf("Opening OTF\n");    
     
    OpsParams.hBMem = GetBMEM(); 
    OpsParams.hBReg = GetBREG();       
    BKNI_CreateMutex(&OpsParams.hBMemLock);
    /* Init OTF module */
    BOTF_Init(&OpsParams, &hOpsOtf, &MaxChannels); 

    /* open parameters for OTF */
    Params.Channel = id;
    Params.hInputParser = (uint32_t) otf_instances[id].IPRaveContext;    
    Params.InputParserCDBSize = 8*1024*1024;
    Params.InputParserITBSize = 1024*1024;
    Params.OutputParserITBSize = 256*1024;
    Params.OutputParserITBAlign = 8;
    BKNI_CreateMutex(&Params.hBIpParserLock);
    Params.hDec =  (uint32_t) GetBXVD_Channel(id);
    BKNI_CreateMutex(&Params.hBDecLock);
    Params.hOutputParser = (uint32_t) otf_instances[id].OPRaveContext;
    Params.bUsePtsAsTag = false;
    BKNI_CreateMutex(&Params.hBOpParserLock);
    BOTF_Open(hOpsOtf, &Params, &otf_instances[id].hOtf);  

    /* Now start decoder */
    BXPT_Rave_GetContextRegisters(otf_instances[id].OPRaveContext, &sContextMap);        
    BXVD_GetDecodeSettings(GetBXVD_Channel(id), &sDecodeSettings);
    sDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eMPEG2;
    sDecodeSettings.ulMultiStreamId = 0;
    sDecodeSettings.ulVideoSubStreamId = 0;
    sDecodeSettings.bPlayback = true;
    sDecodeSettings.bCrcMode = false;
    sDecodeSettings.eTimeBase = BAVC_Timebase_e0+id;
    sDecodeSettings.pContextMap = &sContextMap;
    BXVD_SetVideoDisplayMode(GetBXVD_Channel(id), BXVD_DisplayMode_eVSYNCMode);        	        
    BXVD_StartDecode(GetBXVD_Channel(id), &sDecodeSettings);      
//    BXVD_SetDisplayFieldMode(GetBXVD_Channel(id), BXVD_DisplayFieldType_eTopFieldOnly);

    otf_instances[id].stopstreamer = true;
    
    /* Start disk streamer (play pump) task */
    taskSpawn("tDiskStreamer", 50, 0, 20000,
#if 1
                                (FUNCPTR)DiskStreamer,
#else
                                (FUNCPTR)testBTP,
#endif
                                id, 0, 0, 0, 0, 0, 0, 0, 0, 0);   

	otf_instances[id].initdone = true;
    printf(" 0 SUCCESS\n");
	return;
}

void ostart(uint32_t id)
{
//	printf("ostart");
    /* Flush and enable RAVE context before starting otf */
    BXPT_Rave_FlushContext(otf_instances[id].IPRaveContext);
    BXPT_Rave_EnableContext(otf_instances[id].IPRaveContext);
    BOTF_Start(otf_instances[id].hOtf, &TaskEntry);
//    BXVD_PVR_SetSlowMotionRate(GetBXVD_Channel(0), 60);
    taskSpawn("tOTF", 50, 0, 20000,
               (FUNCPTR)TaskEntry,
               (int) otf_instances[id].hOtf, 0, 0, 0, 0, 0, 0, 0, 0, 0); 
    otf_instances[id].changeflag = true;
    /* Start disk streamer (play pump) */
    otf_instances[id].stopstreamer = false;
//    printf(" 0 SUCCESS\n");
}

void ostop(uint32_t id)
{
//	printf("ostop");
    /*Stop XVD*/
    BXVD_DisableForFlush(GetBXVD_Channel(id));
    /*Stop OTF*/
	BOTF_Stop(otf_instances[id].hOtf);    
    /* Disable and flush input RAVE context after stopping */
    BXPT_Rave_DisableContext(otf_instances[id].IPRaveContext);
    BXPT_Rave_FlushContext(otf_instances[id].IPRaveContext);  

    /* Flush decoder */
    BXVD_FlushDecode(GetBXVD_Channel(id));
    /* Stop disk streamer (play pump) */
	otf_instances[id].stopstreamer = true;
	while(!otf_instances[id].streamerstopped) BKNI_Sleep(100);
//    printf(" 0 SUCCESS\n");
}

void orate(uint32_t id, int rate)
{
	otf_instances[id].Config.FrameRate = rate;
    
    BOTF_ConfigSet(otf_instances[id].hOtf, &otf_instances[id].Config); 
//    printf(" 0 SUCCESS\n");
}

void omode(uint32_t id, int mode)
{
//	printf("omode");
    switch(mode)
    {
    case 1:
        otf_instances[id].Config.FeederMode = BOTF_FeederMode_eSequential;
        break;
    case 2:
        otf_instances[id].Config.FeederMode = BOTF_FeederMode_eChunkForward;
        break;
    case 3:
        otf_instances[id].Config.FeederMode = BOTF_FeederMode_eChunkBackward;
        break;
    case 4:
        otf_instances[id].Config.FeederMode = BOTF_FeederMode_eInterleaved;
        break;
    default:
        printf(" 1 ERROR\n");
        return;
    }
    BOTF_ConfigSet(otf_instances[id].hOtf, &otf_instances[id].Config); 
//    printf(" 0 SUCCESS\n");
}

void offilter(uint32_t id, int filter)
{
//	printf("offilter");
    switch(filter)
    {
    case 1:
        otf_instances[id].Config.FrameFilter = BOTF_FrameFilter_eIFrames;
        break;
    case 2:
        otf_instances[id].Config.FrameFilter = BOTF_FrameFilter_eIPFrames;
        break;
    case 3:
        otf_instances[id].Config.FrameFilter = BOTF_FrameFilter_eIPBFrames;
        break;
    default:
        printf(" 1 ERROR\n");
        return;
    }
    BOTF_ConfigSet(otf_instances[id].hOtf, &otf_instances[id].Config); 
//    printf(" 0 SUCCESS\n");
}

void oforward(uint32_t id)
{
//	printf("oforward");
    otf_instances[id].Config.IsRewind = false;	
    BOTF_ConfigSet(otf_instances[id].hOtf, &otf_instances[id].Config); 
//    printf(" 0 SUCCESS\n");
}

void orewind(uint32_t id)
{
//	printf("orewind");
    otf_instances[id].Config.IsRewind = true;
    BOTF_ConfigSet(otf_instances[id].hOtf, &otf_instances[id].Config);
//    printf(" 0 SUCCESS\n");
}
void ofname(uint32_t id, char *filename)
{
//	printf("ofname");
    if (otf_instances[id].fd != NULL) fclose(otf_instances[id].fd);

    otf_instances[id].fd = fopen(filename, "r");

    if (otf_instances[id].fd == NULL) {
        printf(" 1 ERROR\n");
        return;
    }
//   printf(" 0 SUCCESS\n");
}

void ofpos(uint32_t id, int position)
{
//	printf("ofpos");
    otf_instances[id].startoffset = position;
    
//    printf(" 0 SUCCESS\n");
}

void ofjumpsize(uint32_t id, int size)
{
//    printf("ofjumpsize");
    otf_instances[id].jumpsize = size;

//    printf(" 0 SUCCESS\n");
}
void ochunksize(uint32_t id, int size)
{
//    printf("ochunksize");
    otf_instances[id].unitsize = size;

//    printf(" 0 SUCCESS\n");
}
void ovpid(uint32_t id, int vpid)
{
//    printf("ovpid");
    otf_instances[id].videopid = vpid;
    BXPT_ConfigurePidChannel( GetBXPT(), otf_instances[id].PidChannel, otf_instances[id].videopid, BXPT_PB_PARSER(id) );

//    printf(" 0 SUCCESS\n");
}
/*
 * params[0] = cmd
 * params[1-9] = Parameters
 */
void prepareBTP(char *buf, uint16_t pid, uint32_t *params)
{
    int i;

    buf[0] = 0x47;
    buf[1] = (pid >> 8) & 0x1f;
	buf[2] = pid & 0xff;  /* PID */
    buf[3] = 0x20; /* not scrambled, adaptation_field and no payload, 0 continuity counter */
    buf[4] = 0xb7;/* adaptation field length is 183 - the remainder of the packet */
    buf[5] = 0x82;
    buf[6] = 45; /* Number of relevant bytes */
	buf[7] = 0x00;  /* Align byte */
	buf[8] = 0x42;	/* B */
	buf[9] = 0x52;	/* R */
	buf[10] = 0x43;	/* C */
	buf[11] = 0x4d;	/* M */

    for(i=0; i<10; ++i)
	{
		int base = 12 + i*4;
        
		buf[base] = (unsigned char) ((params[i] & 0xff000000) >> 24);
		buf[base+1] = (unsigned char) ((params[i] & 0x00ff0000) >> 16);
		buf[base+2] = (unsigned char) ((params[i] & 0x0000ff00) >> 8);
		buf[base+3] = (unsigned char) (params[i] & 0x000000ff);
	}
    for(i=52; i<188; ++i)
	{
		buf[i] = 0x00;
	}
}

#if 0
void otf_test(void)
{
	yylex();
}

void mycopy(char *srcdir, char *srcfile, char *destn)
{
    int  ctrlSock;
    int  dataSock;
    char buf [64*1024];
    int  nBytes;
    STATUS       status;
    FILE *fd;    

    setupDisk();    

    fd = fopen(destn, "wb");

    ftpXfer ("10.22.65.69", "rajesh", "rajesh", "",
                 "RETR %s", srcdir, srcfile,
                 &ctrlSock, &dataSock);
        

    while ((nBytes = read (dataSock, buf, sizeof (buf))) > 0)
        {
            fwrite(buf, 1, nBytes, fd);
        }

    close (dataSock);

    if (nBytes < 0)             /* read error? */
        status = ERROR;

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
        status = ERROR;

    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
        status = ERROR;

    close (ctrlSock);

    fclose(fd);
}
#endif

#define BITRATE                 (10*1024*1024)   /* in bits per sec */
#if 0
#define SMOOTH_REV_CHUNK_SIZE   (3*BITRATE/8)   /* In bytes */
#define SMOOTH_REV_JUMP_SIZE    (-5*BITRATE/8)  /* In bytes */
#else
#define SMOOTH_REV_CHUNK_SIZE   4*1024*1024  /* In bytes */
#define SMOOTH_REV_JUMP_SIZE    (-7*1024*1024)  /* In bytes */
#endif
void otestforward3x(id)
{
    oinit(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);

    ofpos(id, 0);
    ochunksize(id, 1925120);
    ofjumpsize(id, 0);
    omode(id, 1);
    orate(id, 300);    
    offilter(id, 3);
    oforward(id);

    ostart(id);
}

void otestforward30x(id)
{
    oinit(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);

    ofpos(id, 0);
    ochunksize(id, 1925120);
    ofjumpsize(id, 1925120);
    omode(id, 2);
    orate(id, 1500);    
    offilter(id, 1);
    oforward(id);      

    ostart(id);
}

void otestrewind3x(id)
{
    oinit(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);

    ofpos(id, 721920000);
    ochunksize(id, 1925120);
    ofjumpsize(id, -2887680);
    omode(id, 4);
    orate(id, 300);    
    offilter(id, 3);
    orewind(id);		

    ostart(id);
}

void otestrewind30x(id)
{
    oinit(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);

    ofpos(id, 721920000);
    ochunksize(id, 1925120);
    ofjumpsize(id, -5775360);
    omode(id, 3);
    orate(id, 1500);
    offilter(id, 1);
    orewind(id);

    ostart(id);
}
void otest()
{
    otest0();
    otest1();
}
void otest0()
{
    int id = 0;

    oinit(id);
    omode(id, 4);
    orate(id, 600);
    offilter(id, 2);
    orewind(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);
#if 0    
    ofpos(id, 0);
#else
    ofpos(id, 400000000);
#endif
    ochunksize(id, SMOOTH_REV_CHUNK_SIZE);
    ofjumpsize(id, SMOOTH_REV_JUMP_SIZE);
    ostart(id);

}

void otest1()
{
    int id = 1;

    oinit(id);
    omode(id, 4);
    orate(id, 300);
    offilter(id, 2);
    orewind(id);
    ofname(id, "/ata0/videos/3mbps.ts");
    ovpid(id, 4097);
#if 0    
    ofpos(id, 0);
#else
    ofpos(id, 400000000);
#endif
    ochunksize(id, SMOOTH_REV_CHUNK_SIZE);
    ofjumpsize(id, SMOOTH_REV_JUMP_SIZE);
    ostart(id);

}

