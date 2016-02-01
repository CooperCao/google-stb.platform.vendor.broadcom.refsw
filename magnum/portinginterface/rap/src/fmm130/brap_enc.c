/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: 
* $brcm_Date: 
*
* Module Description:
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer by the Raptor Audio Encoder PI. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
 
#include "brap.h"
#include "brap_priv.h"
#include "brap_enc.h"
#include "brap_enc_priv.h"
#include "bchp_xpt_rave.h"
#include "brap_img.h"

BDBG_MODULE(rap_enc);

#define BRAP_ENC_MP3_INPUT_FRAME_SIZE              (1152*4) /* for MP3 */
#define BRAP_ENC_MP3_OUTPUT_FRAME_SIZE             (500*4) 
#if ( BCHP_CHIP == 7440 )
#define BRAP_ENC_DTS_INPUT_FRAME_SIZE              (512*4) /* for DTS */
#define BRAP_ENC_DTS_OUTPUT_FRAME_SIZE             (2048*4) 
#endif
#define BRAP_RBUFENC_DEFAULT_SIZE              (BRAP_ENC_MP3_INPUT_FRAME_SIZE*4) 
#define BRAP_RBUFENC_OUTPUT_SIZE               (BRAP_ENC_MP3_OUTPUT_FRAME_SIZE*4) 

BERR_Code BRAP_DSP_P_CopyFWImageToMem (BRAP_DSP_Handle hDsp, uint32_t memAdr, uint32_t firmware_id);
BERR_Code BRAP_DSP_P_GetFWSize (BRAP_DSP_Handle hDsp, uint32_t firmware_id, uint32_t *size);

BERR_Code BRAP_ENC_OpenChannel (
	BRAP_Handle 			hRap,			      /* [in] The Raptor Audio device handle*/
	BRAP_ENC_ChannelHandle 		*phRapEncCh,		  /* [out] The RAP Encoder Channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	const BRAP_ENC_ChannelSettings	*pChnSettings /*[in] Channel settings*/ 
	)
{
	void *pAddressAllocated = NULL;
	uint32_t ui32RegVal = 0, i = 0, k = 0, ui32CFG1 = 0, ui32CFG2 = 0;
	BRAP_ENC_ChannelHandle hRapEncCh;
	uint32_t physAddress;
       BRAP_RM_P_EncResrcReq      sEncResrcReq;
       BERR_Code ret;
	BRAP_RBUF_P_Settings	sRbufSettings[MAX_ENC_INPUT_CHANNELS];

	/* Assert the function argument(s) */
	BDBG_ASSERT(hRap);
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
	{
		BDBG_ASSERT(pChnSettings);
	}
	BDBG_ASSERT(phRapEncCh);
	BDBG_ENTER (BRAP_ENC_OpenChannel);    

    if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
    {
	hRapEncCh = (BRAP_ENC_ChannelHandle)BKNI_Malloc( sizeof(BRAP_ENC_P_Channel));
	
	if(hRapEncCh == NULL)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Reset the entire structure */
	BKNI_Memset( hRapEncCh, 0, sizeof(BRAP_ENC_P_Channel) );
	*phRapEncCh = hRapEncCh;
	hRapEncCh->hRap = hRap;
	hRap->hRapEncCh[uiChannelNo] = hRapEncCh;
	hRapEncCh->eChannelType = BRAP_P_ChannelType_eEncode;
	hRapEncCh->hChip = hRapEncCh->hRap->hChip;
	hRapEncCh->hHeap = hRapEncCh->hRap->hHeap;
	hRapEncCh->hInt = hRapEncCh->hRap->hInt;
	hRapEncCh->hRegister = hRapEncCh->hRap->hRegister;
	hRapEncCh->hRm = hRapEncCh->hRap->hRm;
	hRapEncCh->eaudtype = pChnSettings->eaudtype;
	hRapEncCh->eipstream = pChnSettings->eipstream;
	hRapEncCh->eInterLeaving[0] = pChnSettings->eInterLeaving[0];
	hRapEncCh->eInterLeaving[1] = pChnSettings->eInterLeaving[1];
	hRapEncCh->eInterLeaving[2] = pChnSettings->eInterLeaving[2];
	hRapEncCh->bLFEpresent = pChnSettings->bLFEpresent;

       /* Form the resource requirement depending on the Capture Mode */
	sEncResrcReq.eChannelType = hRapEncCh->eChannelType;
/* TODO : Better this implementation */
	if((pChnSettings->eipstream == BRAP_ENC_Ipstream_eOneCentre_1_0_C)||
		(pChnSettings->eipstream == BRAP_ENC_Ipstream_eMono)||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eStereoInterleaved;
	if(((pChnSettings->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eStereoNoninterleaved;
	if(((pChnSettings->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR)
																	&&(pChnSettings->eInterLeaving[2] == BRAP_ENC_Interleaving_eCLFE)
																	&&(pChnSettings->bLFEpresent == true))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eFiveptoneInterLeaved;
	if(((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone))||
		((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eStereoSurround;
	if(((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->bLFEpresent == false)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eFiveptoneNoLFENonInterLeaved;
	if(((pChnSettings->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(pChnSettings->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->eInterLeaving[2] == BRAP_ENC_Interleaving_eNone)
																	&&(pChnSettings->bLFEpresent == true)))
		sEncResrcReq.eEncBufDataMode = BRAP_EncBufDataMode_eFiveptoneLFENonInterLeaved;

	hRapEncCh->eEncBufDataMode = sEncResrcReq.eEncBufDataMode;
	BDBG_MSG(("BRAP_ENC_OpenChannel: hRapEncCh->eEncBufDataMode > %x ",hRapEncCh->eEncBufDataMode));
       if(pChnSettings->pInputBuffer == NULL)
	       sEncResrcReq.bAllocateRBuf = true;
       else sEncResrcReq.bAllocateRBuf = false;

       /* Call resource manager to allocate required resources. */
       ret = BERR_TRACE(BRAP_RM_P_AllocateResourcesEnc (hRap->hRm, 
									     &sEncResrcReq, 
									     &(hRapEncCh->sEncRsrcGrnt)));
       if(ret != BERR_SUCCESS)
       {
        	BDBG_ERR(("BRAP_ENC_OpenChannel: Resource allocation failed for RAP ENC Channel handle 0x%x", hRapEncCh));
        }
        BDBG_MSG(("BRAP_ENC_OpenChannel: Resource allocation for AUD ENC succeeded"));

        BDBG_MSG(("BRAP_ENC_OpenChannel: hRapEncCh->sEncRsrcGrnt.uiDspId 0x%x", hRapEncCh->sEncRsrcGrnt.uiDspId));
        BDBG_MSG(("BRAP_ENC_OpenChannel: hRapEncCh->sEncRsrcGrnt.uiDspContextId 0x%x", hRapEncCh->sEncRsrcGrnt.uiDspContextId));
	 for(i=0; i< hRapEncCh->eEncBufDataMode; i++)
	        BDBG_MSG(("BRAP_ENC_OpenChannel: hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i] 0x%x", hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i]));

	    /* Store DSP handle inside the Audio Encoder Channel handle */ 
	    /* Update this code for multiple DSPs */
	hRapEncCh->hDsp[uiChannelNo] = hRap->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]; 

    	}
    else
    {
        /* If watchdog recovery */
    	hRapEncCh = *phRapEncCh;
    }

	/* Install the interrupts */
	if(BRAP_P_InterruptInstallEnc(hRapEncCh) != BERR_SUCCESS)
	{
		BDBG_ERR(("Interrupt installation failed for RAP Encode Channel handle 0x%x", hRapEncCh));
	}
		
	if(hRapEncCh->eaudtype == BRAP_ENC_AudioType_eMpeg1Layer3)
		{
			hRapEncCh->uiInputFrameSize = BRAP_ENC_MP3_INPUT_FRAME_SIZE;
			hRapEncCh->uiOutputFrameSize = BRAP_ENC_MP3_OUTPUT_FRAME_SIZE;
		}
#if ( BCHP_CHIP == 7440 )
	else if(hRapEncCh->eaudtype == BRAP_ENC_AudioType_eDTS)
		{
			hRapEncCh->uiInputFrameSize = BRAP_ENC_DTS_INPUT_FRAME_SIZE;
			hRapEncCh->uiOutputFrameSize = BRAP_ENC_DTS_OUTPUT_FRAME_SIZE;
		}
#endif
	else
		{
			hRapEncCh->uiInputFrameSize = BRAP_RBUFENC_DEFAULT_SIZE;
			hRapEncCh->uiOutputFrameSize = BRAP_RBUFENC_OUTPUT_SIZE;
		}
		
	if(!BRAP_P_GetWatchdogRecoveryFlag(hRap)) 
	 if(pChnSettings->pInputBuffer == NULL) {
		/* Ring buffer settings */
		for(k=0; k < MAX_ENC_INPUT_CHANNELS; k++)
		{
			sRbufSettings[k].sExtSettings.pBufferStart = NULL;
            		sRbufSettings[k].bProgRdWrRBufAddr = true;
			if(pChnSettings->uiInputPCMBufferSize == 0)
	            		sRbufSettings[k].sExtSettings.uiSize = hRapEncCh->uiInputFrameSize*4;
			else sRbufSettings[k].sExtSettings.uiSize = pChnSettings->uiInputPCMBufferSize;
		}
		for(i=0; i < MAX_ENC_INPUT_CHANNELS; i++)
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				/* Instantiate the ring buffer */
				ret = BERR_TRACE(BRAP_RBUF_P_Open (
						hRap->hFmm[0],
						&(hRapEncCh->hRBuf[i]),
						hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i],
						&sRbufSettings[i]));
				if(ret != BERR_SUCCESS)
				    {
				        BDBG_ERR(("BRAP_ENC_OpenChannel: Ring Buffer allocation failed for RAP ENC Channel handle 0x%x", hRapEncCh));
				    }
				BDBG_MSG(("Ring buffer %d opened", hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i]));
			}
			else
			{
				/* RBuf is not needed for current channel */
				hRapEncCh->hRBuf[i] = NULL;
			}
		}
		}

#if ( BCHP_CHIP == 7400 ) || ( BCHP_CHIP == 7401 && (BCHP_VER >= BCHP_VER_B0)) || (BCHP_CHIP == 7403)
		/* Choosing Ring Buffers for the input */
			ui32CFG1 = BRAP_Read32(hRap->hRegister,(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
			ui32CFG2 = BRAP_Read32(hRap->hRegister,(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
/* TODO : Better this implementation */
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eOneCentre_1_0_C)||
		(hRapEncCh->eipstream == BRAP_ENC_Ipstream_eMono))
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
	((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
	((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->bLFEpresent == false)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[2] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->bLFEpresent == true)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[5] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[5]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_EN, Enabled);
			}
		}

			BRAP_Write32 (hRap->hRegister, (BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32CFG1);	
			BRAP_Write32 (hRap->hRegister, (BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32CFG2);	
/*	}*/
#else
		/* Choosing Ring Buffers for the input */
			ui32CFG1 = BRAP_Read32(hRap->hRegister,(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
			ui32CFG2 = BRAP_Read32(hRap->hRegister,(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
/* TODO : Better this implementation */
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eOneCentre_1_0_C)||
		(hRapEncCh->eipstream == BRAP_ENC_Ipstream_eMono))
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
	((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))||
	((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))||
		((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_EN, Enabled);
			}
		}
	if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->bLFEpresent == false)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
		}
	if(((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->eInterLeaving[2] == BRAP_ENC_Interleaving_eNone)
																	&&(hRapEncCh->bLFEpresent == true)))
		{
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[0]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[1]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[2]);
				ui32CFG1 = ui32CFG1 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[3]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[4]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
			}
			if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[5] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_BUF_ID, hRapEncCh->sEncRsrcGrnt.uiEncRbufId[5]);
				ui32CFG2 = ui32CFG2 | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_EN, Enabled);
			}
		}

			BRAP_Write32 (hRap->hRegister, (BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32CFG1);	
			BRAP_Write32 (hRap->hRegister, (BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0+
				(BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT1 - BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32CFG2);	
/*	}*/
#endif

    if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
    {
	/* The Output buffer will be a CDB. Which CDB depends on the following Register write */

	/* Program which CDB is going to be used */
	BRAP_Write32 (hRap->hRegister, (BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0+
						(BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT1 - BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
						(hRapEncCh->sEncRsrcGrnt.uiDspContextId)<<BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT1_RAVE_CTXT_SEL_SHIFT);

	/* Allocate for CDB n */
	pAddressAllocated = BRAP_P_AllocAligned (hRap, hRapEncCh->uiOutputFrameSize*4, 8, 0
#if (BRAP_SECURE_HEAP == 1 )
						,false
#endif
						);
	BRAP_P_ConvertAddressToOffset(hRap->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->hHeap, (void *)pAddressAllocated, &physAddress);
	
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_BASE_PTR, 
                        CDB_BASE_PTR, 
                        ((uint32_t)(physAddress) >> 
                         BCHP_XPT_RAVE_CX3_AV_CDB_BASE_PTR_CDB_BASE_PTR_SHIFT)));

	BRAP_Write32 (hRap->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);    

	/* program the end address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_END_PTR, 
                        CDB_END_PTR, 
                        (uint32_t)(physAddress)+hRapEncCh->uiOutputFrameSize*4 - 1)); 
	
	BRAP_Write32 (hRap->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_END_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);

	
	/* Program the Read pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_READ_PTR, 
                        CDB_READ_PTR, 
                        (uint32_t)(physAddress)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_READ_PTR_CDB_READ_PTR_SHIFT)); 
	
	BRAP_Write32 (hRap->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);

	/* Program the Write pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_WRITE_PTR, 
                        CDB_WRITE_PTR, 
                        (uint32_t)(physAddress)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_WRITE_PTR_CDB_WRITE_PTR_SHIFT)); 
	
	BRAP_Write32 (hRap->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);

	/* Program the Write Wrap pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_WRAPAROUND_PTR, 
                        CDB_WRAPAROUND_PTR, 
                        (uint32_t)(physAddress)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_WRAPAROUND_PTR_CDB_WRAPAROUND_PTR_SHIFT)); 
	
	BRAP_Write32 (hRap->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);
    	}

	BDBG_LEAVE(BRAP_ENC_OpenChannel);
	return ( BERR_SUCCESS );

}

BERR_Code BRAP_ENC_Start (
	BRAP_ENC_ChannelHandle hRapEncCh,			/* [in] The RAP Encoder Channel handle */
	const BRAP_ENC_AudioParams	*pAudioParams,	/* [in] Audio parameters required */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
	volatile uint32_t ui32RegVal = 0, ui32BaseAddr = 0;
	uint32_t algoPtr = 0, algoTblPtr = 0, i = 0;

       BSTD_UNUSED (uiChannelNo);
	BDBG_ASSERT(hRapEncCh);
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRapEncCh->hRap))
	{
		BDBG_ASSERT(pAudioParams);
	}
	BDBG_ENTER (BRAP_ENC_Start);    

	BDBG_MSG(("hRapEncCh->eaudtype",hRapEncCh->eaudtype));
	if(hRapEncCh->eaudtype == BRAP_ENC_AudioType_eMpeg1Layer3)
	{
		if(hRapEncCh->hDsp[0]->hRap->sSettings.bFwAuthEnable==false)
		{
		/* Add encoder PreProcessing Algos */
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encAlgoBasePtr;
		algoTblPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encTblBasePtr;

		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_STG0_ID);
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_STG1_ID);
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
/*		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_STG2_ID);
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2];*/

		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoTblPtr, BRAP_IMG_MP3_ENCODE_TABLE_ID);
		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoTblPtr, BRAP_IMG_MP3_ENCODE_TABLE_ID);
		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
/*		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoTblPtr, BRAP_IMG_MP3_ENCODE_TABLE_ID);
		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2];*/
		/* Add encoder PostProcessing Algos */
		}

		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encScratchBufsBasePtr, BRAP_IMG_MP3_ENCODE_SCRATCH_ID);
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encIFrameBasePtr;
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_INTER_FRAME_ID);
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encIStageBasePtr;
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_MP3_ENCODE_INTER_STAGE_ID);
	}
#if ( BCHP_CHIP == 7440 )
	else if(hRapEncCh->eaudtype == BRAP_ENC_AudioType_eDTS)
	{
		/* Add encoder PreProcessing Algos */
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encAlgoBasePtr;
		algoTblPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encTblBasePtr;

		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
/*		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2];*/
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_DTS_ENCODE_CODE_ID);
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encAlgoSizes[BAF_ALGORITHM_DTS_ENCODER];

		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+0];
		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+1];
/*		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER+2];*/
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoTblPtr, BRAP_IMG_DTS_ENCODE_TABLE_ID);
		algoTblPtr +=hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encTblSizes[BAF_ALGORITHM_DTS_ENCODER];
		/* Add encoder PostProcessing Algos */

		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encScratchBufsBasePtr, BRAP_IMG_DTS_ENCODE_SCRATCH_ID);
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encIFrameBasePtr;
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encIFrameSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER];
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_DTS_ENCODE_INTER_FRAME_ID);
		algoPtr = hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->memInfo.encIStageBasePtr;
		algoPtr += hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId]->psEncMitArrays->encIStageSizes[BAF_ALGORITHM_MPEG1_L3_ENCODER];
		BRAP_DSP_P_CopyFWImageToMem (hRapEncCh->hDsp[hRapEncCh->sEncRsrcGrnt.uiDspId], algoPtr, BRAP_IMG_DTS_ENCODE_INTER_STAGE_ID);
	}
#endif

	/* Reset RBUFs - This is actually for Start-Stop-Start */
	for(i=0; i < MAX_ENC_INPUT_CHANNELS; i++)
	{
		if(hRapEncCh->sEncRsrcGrnt.uiEncRbufId[i] != (unsigned int)BRAP_RM_P_INVALID_INDEX)
		{
			ui32BaseAddr = BRAP_Read32 (hRapEncCh->hRBuf[i]->hRegister,  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRapEncCh->hRBuf[i]->ui32Offset);
	/* program the end address */
			ui32RegVal = (BCHP_FIELD_DATA (
			                    AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR, 
			                    RINGBUF_ENDADDR, 
			                    (ui32BaseAddr + hRapEncCh->hRBuf[i]->sSettings.sExtSettings.uiSize - 1))); 
			BRAP_Write32 (hRapEncCh->hRBuf[i]->hRegister,  
			             (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRapEncCh->hRBuf[i]->ui32Offset), 
			              ui32RegVal);
	/* program the Read & Write address */
		    	ui32RegVal = (BCHP_FIELD_DATA (
			                  AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
			                  RINGBUF_RDADDR, 
			                  ui32BaseAddr));       
			BRAP_Write32 (hRapEncCh->hRBuf[i]->hRegister,  
			              (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRapEncCh->hRBuf[i]->ui32Offset), 
			              ui32RegVal);
			ui32RegVal = (BCHP_FIELD_DATA (
			                  AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
			                  RINGBUF_WRADDR, 
			                  ui32BaseAddr));       
			BRAP_Write32 (hRapEncCh->hRBuf[i]->hRegister,  
			              (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRapEncCh->hRBuf[i]->ui32Offset), 
			              ui32RegVal);
		}
	}

	/* Reset CDB pointers */	
	ui32RegVal = BRAP_Read32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));    
	/* program the end address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_END_PTR, 
                        CDB_END_PTR, 
                        (uint32_t)(ui32RegVal)+hRapEncCh->uiOutputFrameSize*4 - 1)); 
	BRAP_Write32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_END_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);
	/* Program the Read pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_READ_PTR, 
                        CDB_READ_PTR, 
                        (uint32_t)(ui32RegVal)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_READ_PTR_CDB_READ_PTR_SHIFT)); 
	BRAP_Write32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);
	/* Program the Write pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_WRITE_PTR, 
                        CDB_WRITE_PTR, 
                        (uint32_t)(ui32RegVal)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_WRITE_PTR_CDB_WRITE_PTR_SHIFT)); 
	BRAP_Write32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);
	/* Program the Write Wrap pointer to Base Address */
	ui32RegVal = (BCHP_FIELD_DATA (
                        XPT_RAVE_CX3_AV_CDB_WRAPAROUND_PTR, 
                        CDB_WRAPAROUND_PTR, 
                        (uint32_t)(ui32RegVal)>>
                        BCHP_XPT_RAVE_CX3_AV_CDB_WRAPAROUND_PTR_CDB_WRAPAROUND_PTR_SHIFT)); 
	BRAP_Write32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), 
                  ui32RegVal);
	/* Reset RBUFs */
	
    if (!BRAP_P_GetWatchdogRecoveryFlag(hRapEncCh->hRap)) {
	/* Program the Algo Type and Enc Algo ID */
	ui32RegVal = (BCHP_FIELD_DATA (AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID,
		(hRapEncCh->eaudtype<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_ALGO_ID_SHIFT)|
		(BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ALGO_TYPE_Encode<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ALGO_TYPE_SHIFT))); 
	ui32RegVal |=((hRapEncCh->eipstream<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_ACMOD_VALUE_SHIFT)|
				(hRapEncCh->eInterLeaving[0]<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_INTERLEAVE_L_R_SHIFT)|
				(hRapEncCh->eInterLeaving[1]<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_INTERLEAVE_SL_SR_SHIFT)|
				(hRapEncCh->eInterLeaving[2]<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_INTERLEAVE_C_LFE_SHIFT)|
				(hRapEncCh->bLFEpresent<<BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_LFE_PRESENT_SHIFT));
	/* Setting for Interrupt Frequency */
	ui32RegVal |= ((BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_PCM_INT_RATE_EVERY_FRAME << BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_PCM_INT_RATE_SHIFT)|
					(BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_CDB_INT_RATE_EVERY_FRAME << BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0_ENC_CDB_INT_RATE_SHIFT));
	BRAP_Write32 (hRapEncCh->hRap->hRegister, (BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0 + 
		(BCHP_AUD_DSP_CFG0_ALGO_ID_CXT1 - BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32RegVal);	
	
	/* Programming the BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1 register */
	ui32RegVal = (0<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_SW_UNDEFINED_SHIFT)|
		 (0<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_PRIVATE_SHIFT)|
		 (BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_ORIGINAL_Original<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_ORIGINAL_SHIFT)|
		 (BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_COPYRIGHT_Copyright_Absent<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_COPYRIGHT_SHIFT)|
		 (BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_CRC_PROTECT_CRC_Absent<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_CRC_PROTECT_SHIFT)|
		 (pAudioParams->emphasis<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_EMPHASIS_SHIFT)|
		 (pAudioParams->psychomod<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_PSYCHO_MODEL_SHIFT)|
		 (pAudioParams->sampfreq<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_FS_INDEX_SHIFT)|
		 (pAudioParams->bitrate<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_BITRATE_INDEX_SHIFT)|
		 (pAudioParams->opmode<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_OUTPUT_MODE_SHIFT)|
		 (pAudioParams->ipmode<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_INPUT_MODE_SHIFT)|
		 (pAudioParams->bpsi<<BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1_ENC_MPEG1_BPSI_SHIFT);
	BRAP_Write32 (hRapEncCh->hRap->hRegister, (BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + 
		(BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT1 - BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32RegVal);	
    	}
	hRapEncCh->bEncoderStopped = false;
	
	/* program the start-encode bit */
	ui32RegVal = BRAP_Read32(hRapEncCh->hRap->hRegister,(BCHP_AUD_DSP_ESR_SI00_INT_SET + (BCHP_AUD_DSP_ESR_SI10_INT_SET - BCHP_AUD_DSP_ESR_SI00_INT_SET)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, ENC_START)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, ENC_START,1);
	BRAP_Write32(hRapEncCh->hRap->hRegister,(BCHP_AUD_DSP_ESR_SI00_INT_SET + (BCHP_AUD_DSP_ESR_SI10_INT_SET - BCHP_AUD_DSP_ESR_SI00_INT_SET)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32RegVal);

	/* Program the enable scheduler bit */
	ui32RegVal = BRAP_Read32(hRapEncCh->hRap->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET);
	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, ENC_CH_EN)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, ENC_CH_EN,1);
	BRAP_Write32(hRapEncCh->hRap->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET, ui32RegVal);

	BDBG_LEAVE(BRAP_ENC_Start);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_Stop ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	bool					bStopNow,			/* [in] This flag can be used to tell the PI 
										whether to stop then & there or to flush all the 
										encoded data & then stop. Use this to take care of 
										getting all encoded data in an eof condition. */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
	volatile uint32_t ui32RegVal = 0, ui32WritePtr = 0, ui32ReadPtr = 0;
	
       BSTD_UNUSED (uiChannelNo);
	BDBG_ASSERT(hRapEncCh);
	BDBG_ENTER (BRAP_ENC_Stop);    
	BDBG_MSG (("STOP ACK"));
	if(bStopNow == false) {
		/* Check for Data in RBUF */
		do {
			ui32ReadPtr = BREG_Read32(hRapEncCh->hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR+hRapEncCh->hRBuf[0]->ui32Offset);
			ui32WritePtr = BREG_Read32(hRapEncCh->hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR+hRapEncCh->hRBuf[0]->ui32Offset);
		} while(ui32ReadPtr != ui32WritePtr);
	}
	/* program the stop-encode bit */
	ui32RegVal = BRAP_Read32(hRapEncCh->hRap->hRegister,(BCHP_AUD_DSP_ESR_SI00_INT_SET + (BCHP_AUD_DSP_ESR_SI10_INT_SET - BCHP_AUD_DSP_ESR_SI00_INT_SET)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, ENC_STOP)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, ENC_STOP,1);
	BRAP_Write32(hRapEncCh->hRap->hRegister,(BCHP_AUD_DSP_ESR_SI00_INT_SET + (BCHP_AUD_DSP_ESR_SI10_INT_SET - BCHP_AUD_DSP_ESR_SI00_INT_SET)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)), ui32RegVal);

	BDBG_MSG(("Waiting for stop Ack"));
	/* while(hRapEncCh->bEncoderStopped == false) ;*/
	/* TODO: Investigate why this doesn't work without a body */
	while(hRapEncCh->bEncoderStopped == false)
		BDBG_MSG(("hRapEncCh->bEncoderStopped %x", hRapEncCh->bEncoderStopped));
	BDBG_MSG(("Got stop Ack"));

	/* Program the disable scheduler bit */
	/* This should move to BRAP_ENC_Close once the decoder & encoder scheduler are merged */
	ui32RegVal = BRAP_Read32(hRapEncCh->hRap->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET);
	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, ENC_CH_DIS)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, ENC_CH_DIS,1);
	BRAP_Write32(hRapEncCh->hRap->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET, ui32RegVal);
	BDBG_LEAVE(BRAP_ENC_Stop);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_GetInputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stInputBufferInfo *pInpStatusInfo,
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
	uint32_t  ui32WritePtr = 0, ui32ReadPtr = 0;
	uint32_t  i = 0;
	uint32_t InputBufStart, InputBufEnd;
	void *pInputBufferWritePtr1 = NULL;
	void *pInputBufferWritePtr2 = NULL;
	uint32_t uiNumBytesFree1 = 0, uiNumBytesFree2 = 0;

       BSTD_UNUSED (uiChannelNo);
	BDBG_ASSERT(hRapEncCh);
	BDBG_ASSERT(pInpStatusInfo);
	BDBG_ENTER (BRAP_ENC_GetInputBufferInfo);    

	pInpStatusInfo->uiNumInputChannels = hRapEncCh->eEncBufDataMode;
	BDBG_MSG(("pInpStatusInfo->uiNumInputChannels : %x",pInpStatusInfo->uiNumInputChannels));

	for(i = 0; i < pInpStatusInfo->uiNumInputChannels; i++) {
		InputBufStart = BRAP_Read32 (hRapEncCh->hRBuf[i]->hRegister,  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRapEncCh->hRBuf[i]->ui32Offset);
		InputBufEnd = InputBufStart + hRapEncCh->uiInputFrameSize*4 - 1;

		ui32WritePtr = BREG_Read32(hRapEncCh->hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR+hRapEncCh->hRBuf[i]->ui32Offset);

		BRAP_ConvertOffsetToAddress(hRapEncCh->hRap->hHeap, (ui32WritePtr & 0x7fffffff), (void **)&(pInputBufferWritePtr1));
		BRAP_ConvertOffsetToAddress(hRapEncCh->hRap->hHeap, InputBufStart, (void **)&(pInputBufferWritePtr2));

		ui32ReadPtr = BREG_Read32(hRapEncCh->hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR+hRapEncCh->hRBuf[i]->ui32Offset);

		/* For Left Input Buffer */
		if ((ui32WritePtr & 0x7fffffff) > (ui32ReadPtr & 0x7fffffff))
		{
			/* The Write Pointer is higher than the Read pointer. The resulting Ring buffer could take one of the 2 forms
				1. The Read Pointer is at the top while the write pointer has gone down towards the end
				2. The Read pointer has stepped to a few places down reading and the write pointer is further down
			*/
			uiNumBytesFree1 = InputBufEnd - (ui32WritePtr & 0x7fffffff) + 1;
			uiNumBytesFree2 = (ui32ReadPtr & 0x7fffffff) - InputBufStart;
		}
		else if ((ui32WritePtr & 0x7fffffff) == (ui32ReadPtr & 0x7fffffff))
		{
			/* The Write Pointer is higher than the Read pointer. The resulting Ring buffer could take one of the 2 forms
				1. The Read Pointer is at the top while the write pointer has gone down towards the end
				2. The Read pointer has stepped to a few places down reading and the write pointer is further down
			*/
			if ((ui32WritePtr & 0x80000000) == (ui32ReadPtr & 0x80000000))
			{
				uiNumBytesFree1 = InputBufEnd - (ui32WritePtr & 0x7fffffff) + 1;
				uiNumBytesFree2 = (ui32ReadPtr & 0x7fffffff) - InputBufStart;
			}
			else 
			{
				uiNumBytesFree1 = 0;
				uiNumBytesFree2 = 0;
			}
		}
		else
		{
			uiNumBytesFree1 = (ui32ReadPtr & 0x7fffffff) - (ui32WritePtr & 0x7fffffff);
			uiNumBytesFree2 = 0;
		}

		/* Put in the proper structure */
		/* 1 RBUF */
		if(hRapEncCh->eipstream == BRAP_ENC_Ipstream_eMono)
		{
			pInpStatusInfo->inputtype.sMono.pInputBufferWritePtr1 = pInputBufferWritePtr1;
			pInpStatusInfo->inputtype.sMono.pInputBufferWritePtr2 = pInputBufferWritePtr2;
			pInpStatusInfo->inputtype.sMono.uiNumBytesFree1 = uiNumBytesFree1;
			pInpStatusInfo->inputtype.sMono.uiNumBytesFree2 = uiNumBytesFree2;
		}
		if(hRapEncCh->eipstream == BRAP_ENC_Ipstream_eOneCentre_1_0_C)
		{
			pInpStatusInfo->inputtype.sCentre.pInputBufferWritePtr1 = pInputBufferWritePtr1;
			pInpStatusInfo->inputtype.sCentre.pInputBufferWritePtr2 = pInputBufferWritePtr2;
			pInpStatusInfo->inputtype.sCentre.uiNumBytesFree1 = uiNumBytesFree1;
			pInpStatusInfo->inputtype.sCentre.uiNumBytesFree2 = uiNumBytesFree2;
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			pInpStatusInfo->inputtype.sInterleavedStereo.pInputBufferWritePtr1 = pInputBufferWritePtr1;
			pInpStatusInfo->inputtype.sInterleavedStereo.pInputBufferWritePtr2 = pInputBufferWritePtr2;
			pInpStatusInfo->inputtype.sInterleavedStereo.uiNumBytesFree1 = uiNumBytesFree1;
			pInpStatusInfo->inputtype.sInterleavedStereo.uiNumBytesFree2 = uiNumBytesFree2;
		}
		/* 2 RBUF */
/*		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eTwoMono_1_1_ch1_ch2)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
			}
			if(i==1)
			{
			}
		}*/
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eStereo_2_0_L_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sStereo.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sStereo.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sStereo.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sStereo.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sStereo.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sStereo.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sStereo.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sStereo.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																		&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sLR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sLR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sLR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sLR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sSLSR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sSLSR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sSLSR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRSLSR.sSLSR.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRC.sLR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sLR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sLR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sLR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRC.sC.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sC.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sC.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRC.sC.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRS.sLR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sLR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sLR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sLR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRS.sS.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sS.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sS.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRS.sS.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		/* 3 RBUF */
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_3_0_L_C_R)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sLRC.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRC.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRC.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRC.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sLRC.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRC.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRC.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRC.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sLRC.sC.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRC.sC.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRC.sC.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRC.sC.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eThree_2_1_L_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sLRS.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRS.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRS.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRS.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sLRS.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRS.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRS.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRS.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sLRS.sS.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRS.sS.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRS.sS.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRS.sS.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sLR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sLR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sLR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sLR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sC.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sC.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sC.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sC.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sS.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sS.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sS.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterLeavedLRCS.sS.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eLR)
																		&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eSLSR))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sLR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sLR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sLR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sLR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sSLSR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sSLSR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sSLSR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sSLSR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sCLFE.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sCLFE.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sCLFE.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sInterleavedFiveptone.sCLFE.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		/* 4 RBUF */
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_2_2_L_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																		&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sLRSLSR.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRSLSR.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRSLSR.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRSLSR.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sLRSLSR.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRSLSR.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRSLSR.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRSLSR.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sLRSLSR.sSL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRSLSR.sSL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRSLSR.sSL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRSLSR.sSL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==3)
			{
				pInpStatusInfo->inputtype.sLRSLSR.sSR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRSLSR.sSR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRSLSR.sSR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRSLSR.sSR.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFour_3_1_L_C_R_S)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sLRCS.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRCS.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRCS.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRCS.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sLRCS.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRCS.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRCS.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRCS.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sLRCS.sC.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRCS.sC.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRCS.sC.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRCS.sC.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==3)
			{
				pInpStatusInfo->inputtype.sLRCS.sS.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sLRCS.sS.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sLRCS.sS.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sLRCS.sS.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
		/* 5 RBUF */
		if((hRapEncCh->eipstream == BRAP_ENC_Ipstream_eFive_3_2_L_C_R_SL_SR)&&(hRapEncCh->eInterLeaving[0] == BRAP_ENC_Interleaving_eNone)
																		&&(hRapEncCh->eInterLeaving[1] == BRAP_ENC_Interleaving_eNone)
																		&&(hRapEncCh->eInterLeaving[2] == BRAP_ENC_Interleaving_eNone))
		{
			if(i==0)
			{
				pInpStatusInfo->inputtype.sFiveptone.sL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==1)
			{
				pInpStatusInfo->inputtype.sFiveptone.sR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==2)
			{
				pInpStatusInfo->inputtype.sFiveptone.sSL.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sSL.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sSL.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sSL.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==3)
			{
				pInpStatusInfo->inputtype.sFiveptone.sSR.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sSR.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sSR.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sSR.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if(i==4)
			{
				pInpStatusInfo->inputtype.sFiveptone.sC.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sC.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sC.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sC.uiNumBytesFree2 = uiNumBytesFree2;
			}
			if((i==5)&&(hRapEncCh->bLFEpresent == true))
			{
				pInpStatusInfo->inputtype.sFiveptone.sLFE.pInputBufferWritePtr1 = pInputBufferWritePtr1;
				pInpStatusInfo->inputtype.sFiveptone.sLFE.pInputBufferWritePtr2 = pInputBufferWritePtr2;
				pInpStatusInfo->inputtype.sFiveptone.sLFE.uiNumBytesFree1 = uiNumBytesFree1;
				pInpStatusInfo->inputtype.sFiveptone.sLFE.uiNumBytesFree2 = uiNumBytesFree2;
			}
		}
	}
	BDBG_LEAVE(BRAP_ENC_GetInputBufferInfo);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_UpdateInputBufferInfo ( 
	BRAP_ENC_ChannelHandle hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stInputBufferInfo *pInpStatusInfo,
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	uint32_t sizeLeft1[MAX_ENC_INPUT_CHANNELS], /* Input Data filled pre-wraparound for Channels */
	uint32_t sizeLeft2[MAX_ENC_INPUT_CHANNELS] /* Input Data filled post-wraparound for Channels */
	)
{
	uint32_t ui32RegVal = 0;
	uint32_t ui32WritePtr = 0, i = 0;
	uint32_t InputBufStart, InputBufEnd;

       BSTD_UNUSED (uiChannelNo);
	BDBG_ASSERT(hRapEncCh);
	BDBG_ASSERT(pInpStatusInfo);
	BDBG_ENTER (BRAP_ENC_UpdateInputBufferInfo);    
       
	pInpStatusInfo->uiNumInputChannels = hRapEncCh->eEncBufDataMode;

	for(i = 0; i < pInpStatusInfo->uiNumInputChannels; i++) {
		InputBufStart = BRAP_Read32 (hRapEncCh->hRap->hRegister,  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRapEncCh->hRBuf[i]->ui32Offset);
		InputBufEnd = InputBufStart + hRapEncCh->uiInputFrameSize*4 - 1;

		ui32WritePtr = BREG_Read32(hRapEncCh->hRap->hRegister, BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR+hRapEncCh->hRBuf[i]->ui32Offset);
		if (sizeLeft2[i])
		{
			if(!(ui32WritePtr & 0x80000000))
				{
					ui32WritePtr = InputBufStart + sizeLeft2[i];
					ui32WritePtr |= 0x80000000;
				}
			else 
				{
					ui32WritePtr = InputBufStart + sizeLeft2[i];
				}
		}
		else if (sizeLeft1[i])
		{

			if (((ui32WritePtr & 0x7fffffff) + sizeLeft1[i]) > InputBufEnd)
				{
					if(ui32WritePtr & 0x80000000)
						ui32WritePtr = InputBufStart;
					else ui32WritePtr = (InputBufStart | 0x80000000);
				}
			else
				{
					ui32WritePtr += sizeLeft1[i];
				}
		}

		ui32RegVal = (BCHP_FIELD_DATA (AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR, ui32WritePtr)); 
		BRAP_Write32 (hRapEncCh->hRap->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR+hRapEncCh->hRBuf[i]->ui32Offset), ui32RegVal);
	}
	BDBG_LEAVE(BRAP_ENC_UpdateInputBufferInfo);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_GetOutputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stOutputBufferInfo *pOutBuffStatusInfo,
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
	uint32_t ui32ReadPtr = 0, ui32WritePtr = 0;
	uint32_t ui32WrapPtr = 0;
	uint32_t OutputBufStart, OutputBufEnd;

       BSTD_UNUSED (uiChannelNo);
	BDBG_ASSERT(hRapEncCh);
	BDBG_ASSERT(pOutBuffStatusInfo);
	BDBG_ENTER (BRAP_ENC_GetOutputBufferInfo);    
       
	OutputBufStart = BREG_Read32(hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	OutputBufEnd = OutputBufStart+hRapEncCh->uiOutputFrameSize*4 - 1; 

	ui32ReadPtr = BREG_Read32(hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	BRAP_ConvertOffsetToAddress(hRapEncCh->hRap->hHeap, ui32ReadPtr, (void **)&(pOutBuffStatusInfo->pOutputBufferReadPtr1));	
	BRAP_ConvertOffsetToAddress(hRapEncCh->hRap->hHeap, OutputBufStart, (void **)&(pOutBuffStatusInfo->pOutputBufferReadPtr2));	

	ui32WritePtr = BREG_Read32(hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));

	if (ui32WritePtr >= ui32ReadPtr)
	{
		/* The Write Pointer is higher than the Read pointer. The resulting Ring buffer could take one of the 2 forms
			1. The Read Pointer is at the top while the write pointer has gone down towards the end
			2. The Read pointer has stepped to a few places down reading and the write pointer is further down
		*/
		pOutBuffStatusInfo->uiNumBytesFull1 = ui32WritePtr - ui32ReadPtr; 
		pOutBuffStatusInfo->uiNumBytesFull2 = 0; 
	}
	else
	{
		ui32WrapPtr = BREG_Read32(hRapEncCh->hRap->hRegister,
              (BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_WRAPAROUND_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRAPAROUND_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));

		if (ui32WrapPtr == OutputBufStart)
		{
			pOutBuffStatusInfo->uiNumBytesFull1 = OutputBufEnd - ui32ReadPtr + 1; 
			pOutBuffStatusInfo->uiNumBytesFull2 = 0;
		}
		else
		{
			pOutBuffStatusInfo->uiNumBytesFull1 = ui32WrapPtr - ui32ReadPtr; 
			pOutBuffStatusInfo->uiNumBytesFull2 = ui32WritePtr - OutputBufStart;
		}
	}
	BDBG_LEAVE(BRAP_ENC_GetOutputBufferInfo);
	return ( BERR_SUCCESS );
}


BERR_Code BRAP_ENC_UpdateOutputBufferInfo ( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,		/* [in] The RAP Encode Channel handle */
	stOutputBufferInfo *pOutBuffStatusInfo,
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	uint32_t size1,		/* Output Data filled pre-wraparound */
	uint32_t size2		/* Output Data filled post-wraparound */
	)
{
	uint32_t ui32RegVal = 0;
	uint32_t ui32ReadPtr = 0;
	uint32_t OutputBufStart, OutputBufEnd;

       BSTD_UNUSED (pOutBuffStatusInfo);    
       BSTD_UNUSED (uiChannelNo);    
	BDBG_ASSERT(hRapEncCh);
	BDBG_ENTER (BRAP_ENC_UpdateOutputBufferInfo);    
       
	OutputBufStart = BREG_Read32(hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	OutputBufEnd = OutputBufStart+hRapEncCh->uiOutputFrameSize*4 - 1; 

	ui32ReadPtr = BREG_Read32(hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));
	if(size2)
		ui32ReadPtr = OutputBufStart + size2;
	else if(size1)
	{
		if ((ui32ReadPtr + size1) > OutputBufEnd)
			ui32ReadPtr = OutputBufEnd;
		else
			ui32ReadPtr += size1;
	}
	ui32RegVal = (BCHP_FIELD_DATA (XPT_RAVE_CX3_AV_CDB_READ_PTR, CDB_READ_PTR, ui32ReadPtr)); 
	BRAP_Write32 (hRapEncCh->hRap->hRegister, 
              (BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_READ_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)),
		ui32RegVal);
	BDBG_LEAVE(BRAP_ENC_UpdateOutputBufferInfo);
	return ( BERR_SUCCESS );
}

#ifndef BCHP_7411_VER /* For chips other than 7411 */
static const  BINT_Id ui32DSPInterruptId[] =
{
	  BCHP_INT_ID_ESR_SO0
	, BCHP_INT_ID_ESR_SO1
	, BCHP_INT_ID_ESR_SO2
#if (( BCHP_CHIP == 7400 ) || ( BCHP_CHIP == 7440 ))
	, BCHP_INT_ID_ESR_SO3
	, BCHP_INT_ID_ESR_SO4
	, BCHP_INT_ID_ESR_SO5	
#endif /* ( BCHP_CHIP == 7400 ) */
};
#endif
BERR_Code BRAP_P_InterruptInstallEnc (
	BRAP_ENC_ChannelHandle hRapCh		/* [in] Raptor Channel handle */
)
{

	BERR_Code ret = BERR_SUCCESS;
#if 0
	BREG_Handle hRegister;
#endif

	BDBG_ASSERT (hRapCh);            

	BDBG_ENTER (BRAP_P_InterruptInstallEnc);    

#ifndef EMULATION /* dont handled interrupts in emulation */

#if 0
	hRegister = hRapCh->hRegister;
#endif

#if 0 /* Done during attaching callbacks */
	ui32RegVal = 0;
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PCM_DATA_CONSUMED, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CDB_DATA_DUMPED, 1))
				|(BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, ENC_STOP_ACK, 1));
	BRAP_Write32 (hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + 
		(BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR - BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR)*hRapCh->sEncRsrcGrnt.uiDspContextId, ui32RegVal );
#endif

	/* Install the interrupt callback */
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap))
	{
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eEncode)
		{
			ret = BINT_CreateCallback(
					&hRapCh->hDSPCallback,
					hRapCh->hInt,
					ui32DSPInterruptId[hRapCh->sEncRsrcGrnt.uiDspContextId],
					BRAP_P_DSP2HostEnc_isr,
					(void*)hRapCh,
					0 /* Not used */);
			ret = BINT_EnableCallback(hRapCh->hDSPCallback);
		}
	}
	else
	{/* Watchdog recovery mode */
		/* First disable and then enable the callback */
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eEncode
            && hRapCh->hDSPCallback)
		{	
   			ret = BINT_DisableCallback(hRapCh->hDSPCallback);		
		}
		
		if(hRapCh->eChannelType == BRAP_P_ChannelType_eEncode)
		{
			ret = BINT_EnableCallback(hRapCh->hDSPCallback);
		}
	}
	BDBG_MSG (("INSTALLED"));

#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptInstallEnc);
	return ret;    
}

BERR_Code BRAP_P_InterruptUnInstallEnc (
	BRAP_ENC_ChannelHandle 		hRapCh		/* [in] Raptor Channel handle */
)
{
	BERR_Code ret = BERR_SUCCESS;

#if 0	
	BREG_Handle hRegister;
#endif

	BDBG_ASSERT (hRapCh);            

	BDBG_ENTER (BRAP_P_InterruptUnInstallEnc);    

#ifndef EMULATION /* dont handle interrupts in emulation */

#if 0
	hRegister = hRapCh->hRegister;
#endif

	/* UnInstall the interrupt callback */
    if(hRapCh->hDSPCallback)
    {
    	ret = BINT_DisableCallback(hRapCh->hDSPCallback);
    	ret = BINT_DestroyCallback(hRapCh->hDSPCallback);
    }
	BDBG_MSG(("Callbacks destroyed."));
#endif /* ifndef EMULATION */

	BDBG_LEAVE(BRAP_P_InterruptUnInstallEnc);
	return ret;    
}

/* for 7401 */
void BRAP_P_DSP2HostEnc_isr (
        void * pParm1, /* [in] Raptor channel handle */
        int    iParm2  /* [in] Not used */        
)
{
	BRAP_ENC_ChannelHandle hRapEncCh;
	uint32_t ui32Offset=0;
	uint32_t ui32IntStatus=0;
	uint32_t ui32MaskStatus=0;

	BDBG_ENTER (BRAP_P_DSP2HostEnc_isr);
	BDBG_ASSERT (pParm1);

	BSTD_UNUSED(iParm2);
	hRapEncCh = (BRAP_ENC_ChannelHandle) pParm1;

	BDBG_MSG(("BRAP_P_DSP2HostEnc_isr\n"));

	/* Check all ESR registers and call corresponding ISR for all supported
	interrupts */
	
	/* Get correct register for this DSP context */
	switch (hRapEncCh->sEncRsrcGrnt.uiDspContextId)
	{
		case 0:
			ui32Offset = 0;
			break;
		case 1:
			ui32Offset = BCHP_AUD_DSP_ESR_SO10_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 2:
			ui32Offset = BCHP_AUD_DSP_ESR_SO20_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
		break;
#if ( BCHP_CHIP == 7400 )
		case 3:
			ui32Offset = BCHP_AUD_DSP_ESR_SO30_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 4:
			ui32Offset = BCHP_AUD_DSP_ESR_SO40_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
		case 5:
			ui32Offset = BCHP_AUD_DSP_ESR_SO50_INT_STATUS - BCHP_AUD_DSP_ESR_SO00_INT_STATUS;
			break;
#endif
	}

	/* For DSP context X, check AUD_DSP_ESR_SOX0 */ 
	ui32MaskStatus = BRAP_Read32_isr (hRapEncCh->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_STATUS + ui32Offset);
	BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_MASK_STATUS=0x%x", ui32MaskStatus));
	ui32IntStatus = BRAP_Read32_isr (hRapEncCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_STATUS + ui32Offset);
	BDBG_MSG (("BCHP_AUD_DSP_ESR_SOX0_INT_STATUS=0x%x", ui32IntStatus));

	/* Clear the interrupts in BCHP_AUD_DSP_ESR_SOX0.
	* This is edge triggered. so we need to clear first.*/
	ui32IntStatus &=  ~ui32MaskStatus;
	BRAP_Write32_isr (hRapEncCh->hRegister, BCHP_AUD_DSP_ESR_SO00_INT_CLEAR + ui32Offset, ui32IntStatus);

	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, PCM_DATA_CONSUMED) == 1)
	{
	BDBG_MSG (("INPUT"));
		if(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eInputDataConsumed].BRAP_ENC_P_pAppIntCallbackPtr != NULL)
			((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eInputDataConsumed].BRAP_ENC_P_pAppIntCallbackPtr
				(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eInputDataConsumed].pParm1,
					((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eInputDataConsumed].pParm2);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, CDB_DATA_DUMPED) == 1)
	{
	BDBG_MSG (("OUTPUT"));
		if(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eOutputDataGenerated].BRAP_ENC_P_pAppIntCallbackPtr != NULL)
			((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eOutputDataGenerated].BRAP_ENC_P_pAppIntCallbackPtr
				(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eOutputDataGenerated].pParm1,
					((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eOutputDataGenerated].pParm2);
	}
	if (BCHP_GET_FIELD_DATA(ui32IntStatus, AUD_DSP_ESR_SO00_INT_STATUS, ENC_STOP_ACK) == 1)
	{
	BDBG_MSG (("STOP ACK I"));
		((BRAP_ENC_ChannelHandle)pParm1)->bEncoderStopped = true;
		if(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eStopAckReceived].BRAP_ENC_P_pAppIntCallbackPtr != NULL)
			((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eStopAckReceived].BRAP_ENC_P_pAppIntCallbackPtr
				(((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eStopAckReceived].pParm1,
					((BRAP_ENC_ChannelHandle)pParm1)->stInterruptCallbackInfo[BRAP_ENC_Interrupt_eStopAckReceived].pParm2);
	}

	BDBG_LEAVE (BRAP_P_DSP2HostEnc_isr);
	return ;
}

BERR_Code BRAP_ENC_GetChannelDefaultSettings(
	BRAP_ENC_ChannelSettings *pDefSettings			/* [out] Default channel settings */
	)
{
	BDBG_ASSERT(pDefSettings);
	BDBG_ENTER (BRAP_ENC_GetChannelDefaultSettings);    
#ifdef BRAP_ENC_TEST_MODE
	pDefSettings->bExtDeviceMem = false;
	pDefSettings->ui32MemBase = NULL;
	pDefSettings->ui32MemSize = 0;
#endif
	pDefSettings->eaudtype = BRAP_ENC_AudioType_eMpeg1Layer3;
	pDefSettings->eipstream = BRAP_ENC_Ipstream_eStereo_2_0_L_R;
/*	pDefSettings->eInterLeaving[0] = BRAP_ENC_Interleaving_eNone; *//*BRAP_ENC_Interleaving_eLR*/
	pDefSettings->eInterLeaving[0] = BRAP_ENC_Interleaving_eLR; /*BRAP_ENC_Interleaving_eLR*/
	pDefSettings->eInterLeaving[1] = BRAP_ENC_Interleaving_eNone; /*BRAP_ENC_Interleaving_eSLSR*/
	pDefSettings->eInterLeaving[2] = BRAP_ENC_Interleaving_eNone; /*BRAP_ENC_Interleaving_eCLFE*/
	pDefSettings->bLFEpresent = false;
	pDefSettings->pInputBuffer = NULL;
	pDefSettings->pOutputBuffer = NULL;
	pDefSettings->uiInputPCMBufferSize = 0;
	pDefSettings->uiOutputBufferSize = 0;
	BDBG_LEAVE(BRAP_ENC_GetChannelDefaultSettings);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_GetTotalChannels( 
	BRAP_Handle 	hRap,			/* [in] The Raptor Audio device handle*/
	unsigned int	*pTotalChannels	/* [out] Total encode Audio channels 
											supported */
	)
{
       BSTD_UNUSED ( hRap );
	BDBG_ASSERT(pTotalChannels);
 	BDBG_ENTER (BRAP_ENC_GetTotalChannels);    
      
	*pTotalChannels = BRAP_RM_P_MAX_ENC_CHANNELS;
	BDBG_LEAVE(BRAP_ENC_GetTotalChannels);
	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_CloseChannel( 
	BRAP_ENC_ChannelHandle 	hRapEncCh,	/* [in] The RAP Encoder Channel handle */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
	BERR_Code		ret = BERR_SUCCESS;
	uint32_t ui32RegVal = 0, i = 0;
	void *virtAddress = 0;

       BSTD_UNUSED ( uiChannelNo );

	BDBG_ENTER(BRAP_ENC_CloseChannel);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapEncCh);

	/* CDB Addr */
	ui32RegVal = BRAP_Read32 (hRapEncCh->hRegister,  
                 (BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR + (BCHP_XPT_RAVE_CX1_AV_CDB_BASE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR)*(hRapEncCh->sEncRsrcGrnt.uiDspContextId)));    
	BRAP_ConvertOffsetToAddress(hRapEncCh->hRap->hHeap, ui32RegVal, (void **)&(virtAddress));	

    /* Mask interrupts and uninstall callbacks */
    BRAP_P_InterruptUnInstallEnc (hRapEncCh);
	/* Call Resource manager to release these resources */
    ret = BRAP_RM_P_FreeResourcesEnc(hRapEncCh->hRm, &(hRapEncCh->sEncRsrcGrnt));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_ENC_CloseChannel: call to BRAP_RM_P_FreeResourcesEnc() failed. Ignoring error!!!!!"));
    }

	BDBG_MSG(("Resources Freed."));

	/* Remove from hRap */
	hRapEncCh->hRap->hRapEncCh[uiChannelNo] = NULL;

	/* Free RBUFs */
	for(i = 0; i < MAX_ENC_INPUT_CHANNELS; i++)
		if(hRapEncCh->hRBuf[i]!= NULL)
			BRAP_RBUF_P_Close(hRapEncCh->hRBuf[i]);
	
	BDBG_MSG(("RBUFs Freed."));
	/* Free CDB */
	BRAP_P_Free(hRapEncCh->hRap, virtAddress
#if (BRAP_SECURE_HEAP == 1 )
						,false
#endif
						);
	BDBG_MSG(("CDB Freed."));

	/* Free the channel handle */
	BKNI_Free(hRapEncCh);

	hRapEncCh = NULL;
	
	BDBG_MSG(("ENC CLOSE DONE"));
	BDBG_LEAVE(BRAP_ENC_CloseChannel);

	return ( BERR_SUCCESS );
}

BERR_Code BRAP_ENC_GetDefaultAudioParams(
	BRAP_ENC_ChannelHandle  hRapEncCh,			/* [in] The RAP Encoder channel handle */
	BRAP_ENC_AudioParams *pDefParams,	/* [out] Default channel parameters */
	unsigned int			uiChannelNo		  /* [in] the desired channel ID */					
	)
{
       BSTD_UNUSED ( uiChannelNo );
       BSTD_UNUSED ( hRapEncCh );
	BDBG_ASSERT(pDefParams);
 	BDBG_ENTER (BRAP_ENC_GetDefaultAudioParams);    
      
	pDefParams->bitrate = BRAP_ENC_MP1L3_Bitrate_e128;
	pDefParams->bpsi = BRAP_ENC_MP1L3_Bpsi_e16;
	pDefParams->emphasis = BRAP_ENC_MP1L3_Emphasis_eNone;
	pDefParams->ipmode = BRAP_ENC_MP1L3_Ipmode_eStereo;
	pDefParams->opmode = BRAP_ENC_MP1L3_Opmode_eStereo;
	pDefParams->psychomod = BRAP_ENC_MP1L3_Psycho_Model_e2;
	pDefParams->sampfreq = BRAP_ENC_MP1L3_Samp_Freq_e441;
	BDBG_LEAVE(BRAP_ENC_GetDefaultAudioParams);
	return ( BERR_SUCCESS );
}


/***************************************************************************
Summary:
	Used to enable and install an application callback for an Interrupt.

Description:
	This API should be used by the application /syslib to enable a AUD_ENC 
	specific interrupt for a given audio encode channel handle enumerated by the 
	BRAP_ENC_Interrupt enum and can also optionally install a callback for 
	the same.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_ENC_InstallInterruptCallback (
	BRAP_ENC_ChannelHandle 		hRapCh, /* [in] The Raptor Audio Encoder channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	BRAP_ENC_Interrupt 			eInterrupt, /* [in] The desired interrupt/Interrupt 
									that needs to be activated */
	BRAP_ENC_CallbackFunc		audEncInterruptCallBack, /* [in] The AUD_ENC callback 
									function which should be registered for the 
									AUD_ENC Interrupt eInterrupt */ 
	void					*pParm1, /* [in] The application specified parameter that 
									needs to be passed unchaned to the callback */
	void					*pParm2 /* [in] The application specified parameter that 
									needs to be passed unchaned to the callback */
	)
{
	uint32_t ui32RegVal = 0 ;
	BDBG_ENTER(BRAP_ENC_InstallInterruptCallback);

	BDBG_ASSERT( hRapCh );
	BDBG_ASSERT( eInterrupt<BRAP_ENC_Interrupt_eMaxInterrupts );

       BSTD_UNUSED ( uiChannelNo );
       
	/* Put the interrupt callback into our array */
	if(eInterrupt<BRAP_ENC_Interrupt_eMaxInterrupts)
	{
		hRapCh->stInterruptCallbackInfo[eInterrupt].BRAP_ENC_P_pAppIntCallbackPtr
			= audEncInterruptCallBack ;
		hRapCh->stInterruptCallbackInfo[eInterrupt].pParm1 = pParm1 ;
		hRapCh->stInterruptCallbackInfo[eInterrupt].pParm2 = pParm2 ;
	}
	else
	{
		BDBG_ERR(("BRAP_ENC_InstallInterruptCallback - INVALID INTERRUPT = %d\n",eInterrupt));
		return BERR_INVALID_PARAMETER;
	}
	ui32RegVal = BRAP_Read32 (hRapCh->hRap->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + 
		(BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR - BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR)*hRapCh->sEncRsrcGrnt.uiDspContextId);
	switch(eInterrupt)
	{
		case BRAP_ENC_Interrupt_eInputDataConsumed :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PCM_DATA_CONSUMED, 1));
			break ;
		case BRAP_ENC_Interrupt_eOutputDataGenerated :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CDB_DATA_DUMPED, 1));
			break ;
		case BRAP_ENC_Interrupt_eStopAckReceived :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, ENC_STOP_ACK, 1));
			break ;
		default :
			;
	}
	BRAP_Write32 (hRapCh->hRap->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + 
		(BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR - BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR)*hRapCh->sEncRsrcGrnt.uiDspContextId, ui32RegVal );

	BDBG_LEAVE(BRAP_ENC_InstallInterruptCallback);

	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	Used to disable and un-install an application callback for an Interrupt.

Description:
	This API should be used by the application /syslib to disable an AUD_ENC 
	specific interrupt for a given channel handle enumerated by the 
	BRAP_ENC_Interrupt enum and can also optionally uninstall the callback for 
	the same.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_ENC_UnInstallInterruptCallback (
	BRAP_ENC_ChannelHandle 		hRapCh, /* [in] The Raptor Audio Encoder channel handle */
	unsigned int			uiChannelNo,		  /* [in] the desired channel ID */					
	BRAP_ENC_Interrupt 			eInterrupt /* [in] The desired interrupt/Interrupt 
									that needs to be disabled*/
	)
{
	uint32_t ui32RegVal = 0 ;
	BDBG_ENTER(BRAP_ENC_UnInstallInterruptCallback);

	BDBG_ASSERT( hRapCh );
	BDBG_ASSERT( eInterrupt<BRAP_ENC_Interrupt_eMaxInterrupts );

       BSTD_UNUSED ( uiChannelNo );

	/* Unput the interrupt callback from the array */
	if(eInterrupt<BRAP_ENC_Interrupt_eMaxInterrupts)
	{
		hRapCh->stInterruptCallbackInfo[eInterrupt].BRAP_ENC_P_pAppIntCallbackPtr = NULL ;
		hRapCh->stInterruptCallbackInfo[eInterrupt].pParm1 = NULL ;

		hRapCh->stInterruptCallbackInfo[eInterrupt].pParm2 = NULL ;
	}
	else
	{
		BDBG_ERR(("BRAP_ENC_UnInstallInterruptCallback - INVALID INTERRUPT = %d\n",eInterrupt));
		return BERR_INVALID_PARAMETER;
	}

	ui32RegVal = BRAP_Read32 (hRapCh->hRap->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + 
		(BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR - BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR)*hRapCh->sEncRsrcGrnt.uiDspContextId);
	switch(eInterrupt)
	{
		case BRAP_ENC_Interrupt_eInputDataConsumed :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, PCM_DATA_CONSUMED, 0));
			break ;
		case BRAP_ENC_Interrupt_eOutputDataGenerated :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, CDB_DATA_DUMPED, 0));
			break ;
		case BRAP_ENC_Interrupt_eStopAckReceived :
			ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_ESR_SO00_MASK_CLEAR, ENC_STOP_ACK, 0));
			break ;
		default :
			;
	}
	BRAP_Write32 (hRapCh->hRap->hRegister, BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR + 
		(BCHP_AUD_DSP_ESR_SO10_MASK_CLEAR - BCHP_AUD_DSP_ESR_SO00_MASK_CLEAR)*hRapCh->sEncRsrcGrnt.uiDspContextId, ui32RegVal );

	BDBG_LEAVE(BRAP_ENC_UnInstallInterruptCallback);

	return BERR_SUCCESS;
}

