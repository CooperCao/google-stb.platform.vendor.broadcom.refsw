/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
* Module Description:
*   Module name: RF Audio Decoder
*   This file contains the implementation of all APIs for the hardware RF Audio decoder
*   abstraction.
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"
#include "brap_img.h"

BDBG_MODULE(rap_rfaudio);		/* Register software module with debug interface */

/* #define 		BRAP_RFAUDIO_P_VERIFY_FW_DOWNLOAD */
#define		BRAP_RFAUDIO_P_DEFAULT_STERERO_LEVEL 0x16B52000
#define		BRAP_RFAUDIO_P_DEFAULT_BASIC_CTRL2   0x400032AF
#define		BRAP_RFAUDIO_P_EIAJ_DEFAULT_BASIC_CTRL2   0x40004322
#define		BRAP_RFAUDIO_P_EIAJ_DEFAULT_STERERO_LEVEL 0x30006000
#define 	BRAP_RFAUDIO_P_DEFAULT_CTRL_SAP_LEVEL 0x10001300

static const BRAP_P_RfAudioRateMangerSR gsRateMangerSR[3] =
{
	{ BAVC_AudioSamplingRate_e32k,   47, 256, 13, 0x00026d61},
	{ BAVC_AudioSamplingRate_e44_1k, 111, 196, 9, 0x00035856},
	{ BAVC_AudioSamplingRate_e48k,   101, 128, 8, 0x0003a411},
};

static void BRAP_RFAUDIO_P_SoftReset(BRAP_Handle hRap);

static void BRAP_RFAUDIO_P_SoftReset(BRAP_Handle hRap)
{
	uint32_t	regVal = 0;
	unsigned int uiIfOrgState = 0;

	regVal = BREG_Read32 ( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );

	uiIfOrgState = BCHP_GET_FIELD_DATA( regVal, BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE );

	/* First turn off if_input if it is turned on */
	regVal = ( regVal & (~BCHP_MASK (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE )))
			| BCHP_FIELD_ENUM (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, IFinput_disabled);
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, regVal );

	/* To reset RF Audio decoder, reset the toggle bit */
	regVal = ( regVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET )))
			| BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET, SoftReset );
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, regVal );
    /* Delay for the register write to actually occur */
    (void)BREG_Read32(hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL);

	regVal = ( regVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET )))
			| BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET, NormalOp );
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, regVal );

	/* Restore the original state of IF Input */
	regVal |= BCHP_FIELD_DATA (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, uiIfOrgState);
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, regVal );
}

void BRAP_RFAUDIO_P_SapXMute_isr (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
	)
{
    bool bSapMono1Mute0, bOutMuteEnable, bMute;
    uint32_t ui32RegVal;

    ui32RegVal = BRAP_Read32_isr( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL );
    bSapMono1Mute0 = BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_SAP_CTRL, SAP_MONO1_MUTE0 );

    ui32RegVal = BRAP_Read32_isr( phCapPort->hRegister, BCHP_BTSC_CTRL_STATUS2 );
    bOutMuteEnable = BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, OUT_MUTE_ENABLE );

    if(( false==bSapMono1Mute0 )&&( true==bOutMuteEnable ))
        bMute = true;
    else
        bMute = false;

    ui32RegVal = BRAP_Read32_isr(  phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL );
    ui32RegVal = (ui32RegVal & ~BCHP_MASK( BTSC_CTRL_VOLUME_CTRL, VC_MUTE ))
        | BCHP_FIELD_DATA( BTSC_CTRL_VOLUME_CTRL, VC_MUTE, bMute );

    BRAP_Write32_isr( phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL, ui32RegVal );
    return;    
}
    
BERR_Code BRAP_RFAUDIO_P_SetOutputSampleRate ( 
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
	)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32OrigRegVal = 0, ui32RegVal;
    bool    bMute;
    BAVC_AudioSamplingRate eOutputSamplingRate = 0;

    /* Set output sample rate of RF Audio decoder equal to input sample rate for the
     * capture channel */
    ui32RegVal = BCHP_GET_FIELD_DATA(BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_MAI_FORMAT_WORD ),
                    BTSC_CTRL_MAI_FORMAT_WORD, AUDIO_SAMPLING_RATE);
    /* Sampling Rate (only supported ones) */
    switch (ui32RegVal)
    {
        case 7:
            eOutputSamplingRate = BAVC_AudioSamplingRate_e32k;
            break;

        case 8:
            eOutputSamplingRate = BAVC_AudioSamplingRate_e44_1k;
            break;

        case 9:
            eOutputSamplingRate = BAVC_AudioSamplingRate_e48k;
            break;

        default:
            break;
    }    
    /*eOutputSamplingRate = 2;*/ /* TODO: Fix this properly, done for bringup */
     
    /* Get the original setting for vc_mute bit and input_enable bit*/
    ui32OrigRegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
    err = BRAP_RFAUDIO_P_GetMute( phCapPort, &bMute);
    if( BERR_SUCCESS!=err )
        return BERR_TRACE( err );

    /* Stop and mute the decoder */
    err = BRAP_RFAUDIO_P_Stop( phCapPort );
    if( BERR_SUCCESS!=err )
        return BERR_TRACE( err );
	err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );	
    if( BERR_SUCCESS!=err )
        return BERR_TRACE( err );

	/* program the rate manager*/
    ui32RegVal = 0;
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_RM_RATE_RATIO, gsRateMangerSR[eOutputSamplingRate].ui32Denominator);
	BDBG_MSG(("BCHP_BTSC_RM_RATE_RATIO = 0x%08x", gsRateMangerSR[eOutputSamplingRate].ui32Denominator));
    ui32RegVal |= BCHP_FIELD_DATA( BTSC_RM_SAMPLE_INC, NUMERATOR, gsRateMangerSR[eOutputSamplingRate].ui32Numerator );
    ui32RegVal |= BCHP_FIELD_DATA( BTSC_RM_SAMPLE_INC, SAMPLE_INC, gsRateMangerSR[eOutputSamplingRate].ui32SampleInc );
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_RM_SAMPLE_INC, ui32RegVal );
	BDBG_MSG(("BCHP_BTSC_RM_SAMPLE_INC = 0x%08x", ui32RegVal));
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_RM_PHASE_INC, gsRateMangerSR[eOutputSamplingRate].ui32PhaseInc);
	BDBG_MSG(("BCHP_BTSC_RM_PHASE_INC = 0x%08x", gsRateMangerSR[eOutputSamplingRate].ui32PhaseInc));

	/* restore the original ctrl reg value which has mute and input enable bit*/
	err = BRAP_RFAUDIO_P_SetMute( phCapPort, bMute );	
    if( BERR_SUCCESS!=err )
        return BERR_TRACE( err );
	BRAP_Write32( phCapPort->hRegister,	BCHP_BTSC_CTRL_BASIC_CTRL, ui32OrigRegVal );	

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SelectTimebase (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
	)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32 ( phCapPort->hRegister,  BCHP_BTSC_RM_CONTROL );

	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_RM_CONTROL, TIMEBASE )))
				| BCHP_FIELD_DATA ( BTSC_RM_CONTROL, TIMEBASE, phCapPort->sParams.eInputBitsPerSample );

	BREG_Write32( phCapPort->hRegister,  BCHP_BTSC_RM_CONTROL, ui32RegVal );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_InitDecoder (
	BRAP_Handle	hRap		/* [in] RAP Handle */
	)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t	ui32UpperInstrCtrl = 0, ui32LowerInstrCtrl = 0, ui32DataCtrl = 0;
	uint32_t	ui32RamAddr = 0, ui32FirmwareSize = 0, ui32RegVal = 0;
	uint32_t 	ui32FirmwareId = 0;
	unsigned int uiNumChunks = 0, i, j, len;
#ifdef BRAP_RFAUDIO_P_VERIFY_FW_DOWNLOAD	
	unsigned int readbackVal;
#endif
	void *image;	
	const void *data = NULL, *pui32LowerInstr = NULL, *pui32UpperInstr = NULL, *pui32Data = NULL;
	const BIMG_Interface *iface = hRap->sSettings.pImgInterface;	
	void *context = hRap->sSettings.pImgContext;
    BRAP_RfAudioStandard eRfAudioStandard = hRap->sInputSettings[BRAP_CapInputPort_eRfAudio].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioStandard;

	/* Reset the RF Audio block */
	ui32RegVal = BREG_Read32( hRap->hRegister, BCHP_AIO_MISC_RESET );
	ui32RegVal &= ~BCHP_MASK( AIO_MISC_RESET, RESET_BTSC );
    ui32RegVal |= BCHP_FIELD_DATA( AIO_MISC_RESET, RESET_BTSC, 1 );
	BREG_Write32( hRap->hRegister, BCHP_AIO_MISC_RESET, ui32RegVal );
    /* Dummy readback for delay of one clock */
    (void)BREG_Read32( hRap->hRegister, BCHP_AIO_MISC_RESET );
	/* Withdraw reset */
    ui32RegVal &= ~BCHP_MASK( AIO_MISC_RESET, RESET_BTSC );
	BREG_Write32( hRap->hRegister, BCHP_AIO_MISC_RESET, ui32RegVal );
	
	/* First turn off if_input if it is turned on */
	ui32RegVal = BREG_Read32 ( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );

	ui32RegVal = ( ui32RegVal & (~BCHP_MASK (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE )))
			| BCHP_FIELD_ENUM (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, IFinput_disabled);
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

    /* Go back to BTSC mode.  The A2 firmware images can be confused if they start in A2 mode for some reason... */
    ui32RegVal = BREG_Read32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL);
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, BTSC_OUTPUT_MODE ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, US0_J1_SELECT ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, MUX_INFIFO_A2 ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));
    BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal);

	/* Prepare the control words for downloading firmware */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		1) ; /* Select write */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		1) ; /* Write to instruction RAM in the processor */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		1) ; /* Upper 20-bit of processor instruction*/
	
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		1) ; /* Select write */
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		1) ; /* Write to instruction RAM in the processor */
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		0) ; /* Lower 20-bit of processor instruction*/

	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		1) ; /* Select write */
	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		2) ; /* Write/read to/from fixed coefficient code RAM in the processor */
	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		0) ; /* Lower 20-bit of processor word */

	BDBG_MSG(("ui32UpperInstrCtrl = 0x%08x, ui32LowerInstrCtrl = 0x%08x, ui32DataCtrl = 0x%08x",ui32UpperInstrCtrl, ui32LowerInstrCtrl, ui32DataCtrl));
#ifdef RAP_RFAUDIO_SUPPORT 	
	switch (eRfAudioStandard)
	{
		case BRAP_RfAudioStandard_eBtsc:
			ui32FirmwareId = BRAP_Img_Id_eRfAudio_Btsc;
			break;
		case BRAP_RfAudioStandard_eEiaj:
			ui32FirmwareId = BRAP_Img_Id_eRfAudio_Eiaj;
            break;
        case BRAP_RfAudioStandard_eKoreaA2:
            ui32FirmwareId = BRAP_Img_Id_eRfAudio_KoreaA2;
			break;
        case BRAP_RfAudioStandard_eNicam:
            ui32FirmwareId = BRAP_Img_Id_eRfAudio_Nicam;
			break;
        case BRAP_RfAudioStandard_ePalA2:
            ui32FirmwareId = BRAP_Img_Id_eRfAudio_PalA2;
			break;            
        case BRAP_RfAudioStandard_eSecamL:
            ui32FirmwareId = BRAP_Img_Id_eRfAudio_SecamL;
            break;
        case BRAP_RfAudioStandard_eIndia:
            ui32FirmwareId = BRAP_Img_Id_eRfAudio_India;
            break;
		default:
			ui32FirmwareId = BRAP_Img_Id_eRfAudio_Btsc;
            eRfAudioStandard = BRAP_Img_Id_eRfAudio_Btsc;
	}
#endif
	err = iface->open (context, &image, ui32FirmwareId);
	if (err!= BERR_SUCCESS) 
	{		
		BDBG_ERR(("Error in Opening the Image Interface"));;	
		return BERR_TRACE( err );

	}

	err = iface->next(image, 0, &data, 8);
	if (err!= BERR_SUCCESS) 
	{		
		BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
		iface->close(image);
		return BERR_TRACE(err);
	}

	ui32RamAddr = 0;
	uiNumChunks = ((uint32_t *) data)[0];
	ui32FirmwareSize =((uint32_t *) data)[1];
	BDBG_MSG(("uiNumChunks = %d, ui32FirmwareSize = 0x%08x", uiNumChunks, ui32FirmwareSize));
	
	for (i=1; i<=uiNumChunks; i++)
	{
		len = (i==uiNumChunks) ?  ui32FirmwareSize-((i-1)*BRAP_IMG_CHUNK_SIZE): BRAP_IMG_CHUNK_SIZE;
		err = iface->next(image, 3, &pui32LowerInstr, len);
		if (err != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close(image);
			return BERR_TRACE(err);
		}

		err = iface->next(image, 4, &pui32UpperInstr, len);
		if (err != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close(image);
			return BERR_TRACE(err);
		}

		/* Load the upper and lower instruction RAM */
		for ( j = 0; j < len/4; j++)
		{
			ui32RegVal = ui32LowerInstrCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_WRITE, ((uint32_t *) pui32LowerInstr)[j]);
#ifdef BRAP_RFAUDIO_P_VERIFY_FW_DOWNLOAD	
			BDBG_MSG(("LWCtrl = 0x%08x, j = %d, LWInstr[j] = 0x%08x",ui32RegVal,j,((uint32_t *) pui32LowerInstr)[j]));
#endif

			ui32RegVal = ui32UpperInstrCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_WRITE, ((uint32_t *) pui32UpperInstr)[j]);
#ifdef BRAP_RFAUDIO_P_VERIFY_FW_DOWNLOAD	
			BDBG_MSG(("UWCtrl = 0x%08x, j = %d, UWInstr[j] = 0x%08x",ui32RegVal,j,((uint32_t *) pui32UpperInstr)[j]));
#endif

			ui32RamAddr++;
		}
			
	}	

	err = iface->next(image, 2, &data, 8);
	if (err != BERR_SUCCESS) 
	{		
		BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
		iface->close(image);
		return BERR_TRACE(err);
	}
	
	ui32RamAddr = 0;
	uiNumChunks = ((uint32_t *) data)[0];
	ui32FirmwareSize =((uint32_t *) data)[1];
	
	for (i=1; i<=uiNumChunks; i++)
	{
		len = (i==uiNumChunks) ?  ui32FirmwareSize-((i-1)*BRAP_IMG_CHUNK_SIZE): BRAP_IMG_CHUNK_SIZE;
		err = iface->next(image, 3, &pui32Data, len);
		if (err!= BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close(image);
			return BERR_TRACE(err);
		}

		err = iface->next(image, 5, &pui32Data, len);
		if (err != BERR_SUCCESS) 
		{		
			BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;	
			iface->close(image);
			return BERR_TRACE(err);
		}

		/* Load the upper and lower instruction RAM */
		for ( j = 0; j < len/4; j++)
		{
			ui32RegVal = ui32DataCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_WRITE, ((uint32_t *) pui32Data)[j]);

			ui32RamAddr++;
		}
			
	}	

#ifdef BRAP_RFAUDIO_P_VERIFY_FW_DOWNLOAD	

	/* Prepare the control words for downloading firmware */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		0) ; /* Select Read */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		1) ; /* Read from instruction RAM in the processor */
	ui32UpperInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		1) ; /* Upper 20-bit of processor instruction*/
	
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		0) ; /* Select Read */
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		1) ; /* Read from instruction RAM in the processor */
	ui32LowerInstrCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		0) ; /* Lower 20-bit of processor instruction*/

	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_WR_RD,
		0) ; /* Select Read */
	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, PROCRAM_SEL,
		2) ; /* Write/read to/from fixed coefficient code RAM in the processor */
	ui32DataCtrl |=  BCHP_FIELD_DATA (BTSC_CTRL_RAM_ACCESS, INSTRAM_WD_SEL,
		0) ; /* Lower 20-bit of processor word */

	BDBG_MSG(("ui32UpperInstrCtrl = 0x%08x, ui32LowerInstrCtrl = 0x%08x, ui32DataCtrl = 0x%08x",ui32UpperInstrCtrl, ui32LowerInstrCtrl, ui32DataCtrl));
	/* Read out lower instr ram */
	BDBG_MSG(("<<<<<<<<UPPER INSTR VERIFICATION>>>>>>"));
	ui32RamAddr = 0;
	for (i = 0; i < 506; i++)
	{
			ui32RegVal = ui32UpperInstrCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			readbackVal = BREG_Read32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_READ);

			ui32RamAddr++;
			BDBG_MSG(("[%d]/tWrite = 0x%08x, Read = 0x%08x', %s", i,((uint32_t *) pui32UpperInstr)[i], readbackVal, (((uint32_t *) pui32UpperInstr)[i]==readbackVal)?"Passed":"Failed"));
	}

	BDBG_MSG(("<<<<<<<<LOWER INSTR VERIFICATION>>>>>>"));
	ui32RamAddr = 0;
	for (i = 0; i < 506; i++)
	{
			ui32RegVal = ui32LowerInstrCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			readbackVal = BREG_Read32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_READ);

			ui32RamAddr++;
			BDBG_MSG(("[%d]/tWrite = 0x%08x, Read = 0x%08x', %s",i, ((uint32_t *) pui32LowerInstr)[i], readbackVal, (((uint32_t *) pui32LowerInstr)[i]==readbackVal)?"Passed":"Failed"));
	}
	
	BDBG_MSG(("<<<<<<<<DATA VERIFICATION>>>>>>"));
	ui32RamAddr = 0;
	for (i = 0; i < 256; i++)
	{
			ui32RegVal = ui32DataCtrl | ui32RamAddr;
			BREG_Write32 (hRap->hRegister, BCHP_BTSC_CTRL_RAM_ACCESS, ui32RegVal);
			readbackVal = BREG_Read32 (hRap->hRegister, BCHP_BTSC_CTRL_PROCRAM_READ);

			ui32RamAddr++;
			BDBG_MSG(("[%d]/tWrite = 0x%08x, Read = 0x%08x', %s",i, ((uint32_t *) pui32Data)[i], readbackVal, (((uint32_t *) pui32Data)[i]==readbackVal)?"Passed":"Failed"));
	}
#endif

	/* Load registers with default values according to the audio standard. After hardware
	    reset, these registers are loaded with US default values. Nevertheless, we will load
	    default values for BTSC also. */
	if ( (eRfAudioStandard == BRAP_RfAudioStandard_eEiaj)||
		 (eRfAudioStandard == BRAP_RfAudioStandard_eKoreaA2)||
		 (eRfAudioStandard == BRAP_RfAudioStandard_eNicam)||
         (eRfAudioStandard == BRAP_RfAudioStandard_eSecamL) ||
		 (eRfAudioStandard == BRAP_RfAudioStandard_ePalA2)
	   )
	{
 		BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, BRAP_RFAUDIO_P_EIAJ_DEFAULT_BASIC_CTRL2 );
		BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_PILOT_LEVEL, BRAP_RFAUDIO_P_EIAJ_DEFAULT_STERERO_LEVEL );
	}
	else
	{
 		BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, BRAP_RFAUDIO_P_DEFAULT_BASIC_CTRL2 );
		BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_PILOT_LEVEL, BRAP_RFAUDIO_P_DEFAULT_STERERO_LEVEL );
	}
	
	/* set the SAP_AM on/off levels register */
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_SAP_LEVEL, BRAP_RFAUDIO_P_DEFAULT_CTRL_SAP_LEVEL) ;
	
	/* Set Audio Standard which shall not be changed after the code has been downloaded*/
	ui32RegVal = BREG_Read32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
    ui32RegVal &= (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, US0_J1_SELECT ));
    ui32RegVal &= (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, MUX_INFIFO_A2 ));

    if (BRAP_RfAudioStandard_eBtsc == eRfAudioStandard || BRAP_RfAudioStandard_eIndia == eRfAudioStandard)
    {
        ui32RegVal |= BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, US0_J1_SELECT, US );    
    }
    else
    {
        ui32RegVal |= BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, US0_J1_SELECT, Japan);        
        if ( BRAP_RfAudioStandard_eEiaj != eRfAudioStandard)
        {
            ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, MUX_INFIFO_A2, 1);
        }
    }
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

    BKNI_Sleep(1);
    /* Turn on if_input */
    ui32RegVal = BREG_Read32 ( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );

    ui32RegVal = ( ui32RegVal & (~BCHP_MASK (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE )))
            | BCHP_FIELD_ENUM (BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, IFinput_enabled);
    BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

    ui32RegVal = BREG_Read32 ( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );

    ui32RegVal = ( ui32RegVal & (~BCHP_MASK (BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET )))
            | BCHP_FIELD_DATA (BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET, 1);
    BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );
    /* Delay for the register write to actually occur */
    (void)BREG_Read32(hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL);

    ui32RegVal = ( ui32RegVal & (~BCHP_MASK (BTSC_CTRL_BASIC_CTRL, BTSC_DECODER_SOFT_RESET )));
    BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

    /* Upon reset, output PCM is muted, unmute PCM samples */
	ui32RegVal = BREG_Read32( hRap->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL );
	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_VOLUME_CTRL, VC_MUTE )))
				| BCHP_FIELD_DATA( BTSC_CTRL_VOLUME_CTRL, VC_MUTE, 0 );
	BREG_Write32( hRap->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL, ui32RegVal );

#if 0 /* Useful for debug */
    BDBG_ERR(("Firmware loaded"));
#endif

    return ( err );
}


BERR_Code BRAP_RFAUDIO_P_SetSettings (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [in] Pointer to Capture Port handle */
)   
{
	BERR_Code err = BERR_SUCCESS;
    BRAP_Handle hRap = NULL;

    hRap = phCapPort->hFmm->hRap;
    
    err = BRAP_RFAUDIO_P_SetMute(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.bRfAudioMute);
    
	if ( err!= BERR_SUCCESS )
		return err;
    
    switch(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioStandard)
    {
        case BRAP_RfAudioStandard_eEiaj:
            err = BRAP_RFAUDIO_P_SetEiajOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioEiajOpMode);
            break;
        case BRAP_RfAudioStandard_eKoreaA2:
            err = BRAP_RFAUDIO_P_SetKoreaA2OutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioKoreaA2OpMode);
            break;
        case BRAP_RfAudioStandard_eNicam:
        case BRAP_RfAudioStandard_eSecamL:
            err = BRAP_RFAUDIO_P_SetNicamOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioNicamOpMode);
            break;
        case BRAP_RfAudioStandard_ePalA2:
            err = BRAP_RFAUDIO_P_SetPalA2OutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioPalA2OpMode);
            break;            
        case BRAP_RfAudioStandard_eIndia:
            err = BRAP_RFAUDIO_P_SetBtscOutputMode(phCapPort, BRAP_RfAudioBtscOutputMode_eMono);
            break;
        case BRAP_RfAudioStandard_eBtsc:
        default:
            err = BRAP_RFAUDIO_P_SetBtscOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioBtscOpMode);
            break;
    }
	if ( err!= BERR_SUCCESS )
		return err;

    err = BRAP_RFAUDIO_P_SetStereoConfig(phCapPort, &(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.sRfAudioStereoConfig));
	if ( err!= BERR_SUCCESS )
		return err;

    err = BRAP_RFAUDIO_P_SetSapConfig(phCapPort, &(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.sRfAudioSapConfig));
	if ( err!= BERR_SUCCESS )
		return err;

	err = BRAP_RFAUDIO_P_SelectTimebase( phCapPort );
	if ( err!= BERR_SUCCESS )
		return err;
	
	err = BRAP_RFAUDIO_P_SetOutputSampleRate( phCapPort );
	if ( err!= BERR_SUCCESS )
		return err;


    return err;
}
BERR_Code BRAP_RFAUDIO_P_Start (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [in] Pointer to Capture Port handle */
)   
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
    BRAP_Handle hRap = NULL;

    hRap = phCapPort->hFmm->hRap;    
	
    /* Program configuration set before start, if any */
    err = BRAP_RFAUDIO_P_SetMute(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.bRfAudioMute);
	if ( err!= BERR_SUCCESS )
		return err;
    switch(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioStandard)
    {
        case BRAP_RfAudioStandard_eEiaj:
            err = BRAP_RFAUDIO_P_SetEiajOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioEiajOpMode);
            break;
        case BRAP_RfAudioStandard_eKoreaA2:
            err = BRAP_RFAUDIO_P_SetKoreaA2OutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioKoreaA2OpMode);
            break;
        case BRAP_RfAudioStandard_eNicam:
        case BRAP_RfAudioStandard_eSecamL:
            err = BRAP_RFAUDIO_P_SetNicamOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioNicamOpMode);
            /* In case of Nicam we need to set the output sampling rate */
            BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_NCO_FCW, 393216 );
            BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_NCO_SCL, 43690 );
            break;
        case BRAP_RfAudioStandard_ePalA2:
            err = BRAP_RFAUDIO_P_SetPalA2OutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioPalA2OpMode);
            break;            
        case BRAP_RfAudioStandard_eIndia:
            err = BRAP_RFAUDIO_P_SetBtscOutputMode(phCapPort, BRAP_RfAudioBtscOutputMode_eMono);
            break;
        case BRAP_RfAudioStandard_eBtsc:
        default:
            err = BRAP_RFAUDIO_P_SetBtscOutputMode(phCapPort, hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioBtscOpMode);
            break;
    }
	if ( err!= BERR_SUCCESS )
		return err;
    err = BRAP_RFAUDIO_P_SetStereoConfig(phCapPort, &(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.sRfAudioStereoConfig));
	if ( err!= BERR_SUCCESS )
		return err;
    err = BRAP_RFAUDIO_P_SetSapConfig(phCapPort, &(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.sRfAudioSapConfig));
	if ( err!= BERR_SUCCESS )
		return err;

	/* Program time base selection and output rate*/
	err = BRAP_RFAUDIO_P_SelectTimebase( phCapPort );
	if ( err!= BERR_SUCCESS )
		return err;
	
	err = BRAP_RFAUDIO_P_SetOutputSampleRate( phCapPort );
	if ( err!= BERR_SUCCESS )
		return err;

	/* Enable IF input */
	ui32RegVal = BREG_Read32 ( phCapPort->hRegister,  BCHP_BTSC_CTRL_BASIC_CTRL );
	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE)))
			| BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, IFinput_enabled );
	BREG_Write32( phCapPort->hRegister,  BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

    /* Soft reset decoder before start */
    BRAP_RFAUDIO_P_SoftReset( phCapPort->hFmm->hRap );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL );
	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_SAP_CTRL, SAP_IN_COEF );
	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_SAP_CTRL, SAP_IN_COEF, 0x32AF );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL, ui32RegVal );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, USSTEREO_J_IN_COEF );
    switch(hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioStandard)
    {
        case BRAP_RfAudioStandard_eEiaj:
        case BRAP_RfAudioStandard_eKoreaA2:
        case BRAP_RfAudioStandard_eSecamL:
        case BRAP_RfAudioStandard_ePalA2:
            ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_STEREO_CTRL, USSTEREO_J_IN_COEF, 0x2000 );
            break;            
        case BRAP_RfAudioStandard_eNicam:            
            ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_STEREO_CTRL, USSTEREO_J_IN_COEF, 0x6000 );            
            break;
        case BRAP_RfAudioStandard_eBtsc:
        case BRAP_RfAudioStandard_eIndia:
        default:
	        ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_STEREO_CTRL, USSTEREO_J_IN_COEF, 0x32AF );
            break;
    }
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL, ui32RegVal );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2 );
	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL2, VAR_OUTSCALING );
	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL2, VAR_OUTSCALING, 0x3C64 );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, ui32RegVal );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2 );	
	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL2, FIXED_FMDIV_INSCALING );	
	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL2, FIXED_FMDIV_INSCALING, 0x32AF );	
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, ui32RegVal );	
	
    	
    /* Soft reset decoder after start */
    BRAP_RFAUDIO_P_SoftReset( phCapPort->hFmm->hRap );

#if 0
	/* Enable RF Audio Interrupts -- These only seem to have the ability to hang the system when they runaway */
    BRAP_P_EnableRfAudioInterrupts( phCapPort );
#endif

	return err;

}

BERR_Code BRAP_RFAUDIO_P_Stop (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
)   
{
	BERR_Code err = BERR_SUCCESS;

#if 0
    uint32_t ui32RegVal = 0;
	/* Disable RF Audio Interrupts */
    BRAP_P_DisableRfAudioInterrupts( phCapPort );

    /* Disable IF input */
	ui32RegVal = BREG_Read32 ( phCapPort->hRegister,  BCHP_BTSC_CTRL_BASIC_CTRL );
	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE)))
			| BCHP_FIELD_ENUM( BTSC_CTRL_BASIC_CTRL, IF_INPUT_ENABLE, IFinput_disabled );
	BREG_Write32( phCapPort->hRegister,  BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );
#else
    BSTD_UNUSED(phCapPort);
#endif

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetStatus (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioStatus  *sStatus			/* [out] Status of the channel */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STATUS1 );
	sStatus->ui16PltSubPower = ( uint16_t ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS1, PLT_POW );
	sStatus->ui16SapAmPower = ( uint16_t ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS1, SAP_AM_POW );
	sStatus->bPltOff = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS1, PLT_AVAILABLE );
	sStatus->bSapOff = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS1, SAP_AM_AVAILABLE );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STATUS2 );
	sStatus->bMuteStatus = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, OUT_MUTE_ENABLE );
	sStatus->ui32RfAudioMode = ( uint32_t ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, BTSC_MODE );
	sStatus->bInputFifoEmpty = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, IFIFO_EMPTY );
	sStatus->bInputFifoFull = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, IFIFO_FULL );
	sStatus->bOutputFifoEmpty = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, OFIFO_EMPTY );
	sStatus->bOutputFifoFull = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, OFIFO_FULL );
	sStatus->bProcErr = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STATUS2, PROCESSOR_ERR );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetEnvelopeFreqAmpl (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t  *ui32EnvelopeFreqAmpl			/* [out] Status of the channel */
)
{
	*ui32EnvelopeFreqAmpl = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_EF_AMPL );

	return BERR_SUCCESS;
}

BERR_Code BRAP_RFAUDIO_P_SetMute (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    bool						bMute		/* [out ] Mute Enable
    											true = mute
    											false = un-mute */
)   
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL );
	
	if ( bMute )
	{
		ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_VOLUME_CTRL, VC_MUTE )))
				| BCHP_FIELD_DATA( BTSC_CTRL_VOLUME_CTRL, VC_MUTE, 1 );
		BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL, ui32RegVal );
	}
	else
	{
		ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_VOLUME_CTRL, VC_MUTE )))
				| BCHP_FIELD_DATA( BTSC_CTRL_VOLUME_CTRL, VC_MUTE, 0 );
		BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL, ui32RegVal );
	}

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetMute (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    bool *bMute							/* [out] Mute status */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_VOLUME_CTRL );
	*bMute = (bool) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_VOLUME_CTRL, VC_MUTE );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetBtscOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioBtscOutputMode		eOutputMode	 /* [in] US Output Mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	bool bMuteState;

	/* Get the mute status */
	err = BRAP_RFAUDIO_P_GetMute ( phCapPort, &bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	/* Mute the output */
	err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, BTSC_OUTPUT_MODE ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));

	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, BTSC_OUTPUT_MODE, eOutputMode );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

	/* Restore original mute state */
	err = BRAP_RFAUDIO_P_SetMute ( phCapPort, bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );
	
	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetBtscOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioBtscOutputMode		*eOutputMode	 /* [out] US Output Mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	*eOutputMode = ( BRAP_RfAudioBtscOutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_BASIC_CTRL, BTSC_OUTPUT_MODE );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetEiajOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,      /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioEiajOutputMode		eOutputMode	    /* [in] Eiaj Output Mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	bool bMuteState, bAutoSwitchMode;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	bAutoSwitchMode = BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
	if ( bAutoSwitchMode )
	{
		BDBG_MSG(("Auto Switch Mode (PLT_PSW_ON) is ON. Not changing the output mode" ));
	}
	else
	{
		/* Get the mute status */
		err = BRAP_RFAUDIO_P_GetMute ( phCapPort, &bMuteState );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );

		/* Mute the output */
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );

		ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
		ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE ));
        ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));

		ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, eOutputMode );
		BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

		/* Restore original mute state */
		err = BRAP_RFAUDIO_P_SetMute ( phCapPort, bMuteState );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );
		
		return BERR_TRACE( err );
	}

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetEiajOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,      /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioEiajOutputMode		*eOutputMode	/* [out] Eiaj Output Mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	*eOutputMode = ( BRAP_RfAudioEiajOutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetKoreaA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,      /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioKoreaA2OutputMode  eOutputMode	        /* [in] Eiaj Outputmode stereo/mono/dualmono*/
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0, ui32Reg2Val = 0;
	bool bMuteState;
	BRAP_RfAudioStatus  sStatus;

	/* Get the mute status */
	err = BRAP_RFAUDIO_P_GetMute ( phCapPort, &bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	/* Mute the output */
	err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));

	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, eOutputMode );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

	/* Restore original mute state */
	err = BRAP_RFAUDIO_P_SetMute ( phCapPort, bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	/* This is A2 requirement: Turn OFF the auto switch between Mono1 and Mono2, but
	 * turn ON auto switch between stereo and Mono */

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	ui32Reg2Val = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1 );
/*
    This was required for 3560 to do some autoconfigure. 
    Since it is not required for 3563 it is being commented out. Refer PR 38044.
	BKNI_Sleep( 500 );
*/	
	err = BRAP_RFAUDIO_P_GetStatus( phCapPort,  &sStatus); 
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	switch( eOutputMode )
	{
		case BRAP_RfAudioKoreaA2OutputMode_eMain:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		case BRAP_RfAudioKoreaA2OutputMode_eStereo:
			ui32RegVal |= BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		case BRAP_RfAudioKoreaA2OutputMode_eSub:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val |= BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		default:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
	}
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL, ui32RegVal);
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1, ui32Reg2Val);

	if (!(( BRAP_RfAudioKoreaA2OutputMode_eSub==eOutputMode ) && ( sStatus.ui32RfAudioMode != 2 )))
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, false );
	else
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, bMuteState);

	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	return BERR_TRACE( err );

}

BERR_Code BRAP_RFAUDIO_P_GetKoreaA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioKoreaA2OutputMode  *eOutputMode	/* [out] Janpan Outputmode stereo/mono/dualmono*/
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	bool bMonoAutoSwitch;

	ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	*eOutputMode = ( BRAP_RfAudioKoreaA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
		BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );
	if ( BRAP_RfAudioKoreaA2OutputMode_eSub==(*eOutputMode ))
		return err;

	ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	bMonoAutoSwitch = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON);
	if ( bMonoAutoSwitch )
	{
		ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_STATUS2 );
		*eOutputMode = ( BRAP_RfAudioKoreaA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
			BTSC_CTRL_STATUS2, BTSC_MODE );
	}
	else
	{
		ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
		*eOutputMode = ( BRAP_RfAudioKoreaA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
			BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );
	}
	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetNicamOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioNicamOutputMode  eOutputMode	  /* [in] Nicam Output mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	bool bMuteState, bAutoSwitchMode;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	bAutoSwitchMode = BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
	if ( bAutoSwitchMode )
	{
		BDBG_MSG(("Auto Switch Mode (PLT_PSW_ON) is ON. Not changing the output mode" ));
	}
	else
	{
		/* Get the mute status */
		err = BRAP_RFAUDIO_P_GetMute ( phCapPort, &bMuteState );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );

		/* Mute the output */
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );

		ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
		ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE ));
		ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));

        

        switch (eOutputMode)
        {
            case BRAP_RfAudioNicamOutputMode_eFmAmMono:
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 0); 
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 0);  
                break;
                
            case BRAP_RfAudioNicamOutputMode_eNicamMono:
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 1); 
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 2);                                
                break;
                
            case BRAP_RfAudioNicamOutputMode_eNicamStereo:
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 1);                 
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 1);                                
                break;
                
            case BRAP_RfAudioNicamOutputMode_eDualMono1:
                 ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 1);                 
                 ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 2);                                
                 break;

      
            case BRAP_RfAudioNicamOutputMode_eDualMono2: 

                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 1);                 
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 3);                                
                break;
               
            case BRAP_RfAudioNicamOutputMode_eDualMono1And2:  

               
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 1);                
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 1);                
                break;
                
            case BRAP_RfAudioNicamOutputMode_eMax:
            default:
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, NICAM_SEL, 0); 
                ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, 0);                
                break;                
        }

        
		BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

		/* Soft reset */
		BRAP_RFAUDIO_P_SoftReset( phCapPort->hFmm->hRap );
        
		/* Restore original mute state */
		err = BRAP_RFAUDIO_P_SetMute ( phCapPort, bMuteState );
		if ( BERR_SUCCESS != err )
			return BERR_TRACE ( err );
		
		return BERR_TRACE( err );
	}

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetNicamOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioNicamOutputMode  *eOutputMode	/* [out] Nicam Output mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	*eOutputMode = ( BRAP_RfAudioNicamOutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetPalA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioPalA2OutputMode  eOutputMode	  /* [in] PAL-A2 Output mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0, ui32Reg2Val = 0;
	bool bMuteState;
	BRAP_RfAudioStatus  sStatus;

	/* Get the mute status */
	err = BRAP_RFAUDIO_P_GetMute ( phCapPort, &bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	/* Mute the output */
	err = BRAP_RFAUDIO_P_SetMute( phCapPort, true );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE ));
    ui32RegVal = ui32RegVal & (~BCHP_MASK( BTSC_CTRL_BASIC_CTRL, NICAM_SEL ));

	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE, eOutputMode );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL, ui32RegVal );

	/* Soft reset */
	BRAP_RFAUDIO_P_SoftReset( phCapPort->hFmm->hRap );
	
	/* Restore original mute state */
	err = BRAP_RFAUDIO_P_SetMute ( phCapPort, bMuteState );
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	/* This is PAL A2 requirement: Turn OFF the auto switch between Mono1 and Mono2, but
	 * turn ON auto switch between stereo and Mono */

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	ui32Reg2Val = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1 );
/*
    This was required for 3560 to do some autoconfigure. 
    Since it is not required for 3563/3548 it is being commented out. Refer PR 38044.
	BKNI_Sleep( 500 );
*/	
	err = BRAP_RFAUDIO_P_GetStatus( phCapPort,  &sStatus); 
	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	switch( eOutputMode )
	{
		case BRAP_RfAudioPalA2OutputMode_eMain:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		case BRAP_RfAudioPalA2OutputMode_eStereo:
			ui32RegVal |= BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		case BRAP_RfAudioPalA2OutputMode_eSub:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val |= BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
			break;
		default:
			ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
			ui32Reg2Val &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
	}
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL, ui32RegVal);
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1, ui32Reg2Val);

	/* Soft reset */
	BRAP_RFAUDIO_P_SoftReset( phCapPort->hFmm->hRap );

	if (!(( BRAP_RfAudioPalA2OutputMode_eSub==eOutputMode ) && ( sStatus.ui32RfAudioMode != 2 )))
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, false );
	else
		err = BRAP_RFAUDIO_P_SetMute( phCapPort, bMuteState);

	if ( BERR_SUCCESS != err )
		return BERR_TRACE ( err );

	return BERR_TRACE( err );
}

BERR_Code BRAP_RFAUDIO_P_GetPalA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,      /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioPalA2OutputMode  *eOutputMode	/* [out] PAL-A2 Output mode */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	bool bMonoAutoSwitch;

	ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
	*eOutputMode = ( BRAP_RfAudioPalA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
		BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );
	if ( BRAP_RfAudioPalA2OutputMode_eSub==(*eOutputMode ))
		return err;

	ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	bMonoAutoSwitch = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON);
	if ( bMonoAutoSwitch )
	{
		ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_STATUS2 );
		*eOutputMode = ( BRAP_RfAudioPalA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
			BTSC_CTRL_STATUS2, BTSC_MODE );
	}
	else
	{
		ui32RegVal = BRAP_Read32(  phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL );
		*eOutputMode = ( BRAP_RfAudioPalA2OutputMode ) BCHP_GET_FIELD_DATA( ui32RegVal, 
			BTSC_CTRL_BASIC_CTRL, JBTSC_OUTPUT_MODE );
	}
	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetStereoConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,              /* [in] Pointer to Capture Port handle */
    const BRAP_RfAudioStereoConfig		*psStereoCfg	/* [in] Stereo Config */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	ui32RegVal = ( ui32RegVal & ( ~BCHP_MASK ( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON )))
				| BCHP_FIELD_DATA( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON, psStereoCfg->bPltPswOn );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL, ui32RegVal );

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_PILOT_LEVEL );
	ui32RegVal = ( ui32RegVal & ( ~BCHP_MASK ( BTSC_CTRL_PILOT_LEVEL, PLT_ON_LEVEL )))
				| BCHP_FIELD_DATA( BTSC_CTRL_PILOT_LEVEL, PLT_ON_LEVEL, psStereoCfg->ui16PltOnLevel );
	ui32RegVal = ( ui32RegVal & ( ~BCHP_MASK ( BTSC_CTRL_PILOT_LEVEL, PLT_OFF_LEVEL )))
				| BCHP_FIELD_DATA( BTSC_CTRL_PILOT_LEVEL, PLT_OFF_LEVEL, psStereoCfg->ui16PltOffLevel );
	BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_PILOT_LEVEL, ui32RegVal );
	
	return err;
	
}

BERR_Code BRAP_RFAUDIO_P_GetStereoConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,          /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioStereoConfig		*psStereoCfg	/* [out] Stereo Config */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
	psStereoCfg->bPltPswOn = (bool) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON );
	
	ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_PILOT_LEVEL );
	psStereoCfg->ui16PltOffLevel = (uint16_t) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_PILOT_LEVEL, PLT_OFF_LEVEL );
	psStereoCfg->ui16PltOnLevel = (uint16_t) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_PILOT_LEVEL, PLT_ON_LEVEL );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_SetSapConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,                  /* [in] Pointer to Capture Port handle */
    const BRAP_RfAudioSapConfig			*psSapConfig		/* [in] SAP config */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;
	BRAP_RfAudioStandard eRfAudioStandard;

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL );
	ui32RegVal = ( ui32RegVal & (~(BCHP_MASK( BTSC_CTRL_SAP_CTRL, SAP_PSW_ON ))))
		| BCHP_FIELD_DATA (BTSC_CTRL_SAP_CTRL, SAP_PSW_ON, psSapConfig->bSapPswOn );
	ui32RegVal = ( ui32RegVal & (~(BCHP_MASK( BTSC_CTRL_SAP_CTRL, SAP_MONO1_MUTE0 ))))
		| BCHP_FIELD_DATA (BTSC_CTRL_SAP_CTRL, SAP_MONO1_MUTE0, psSapConfig->bSapMono1Mute0);
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL, ui32RegVal );

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_LEVEL );
	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_SAP_LEVEL, SAP_AM_ON_LEVEL )))
		| BCHP_FIELD_DATA( BTSC_CTRL_SAP_LEVEL, SAP_AM_ON_LEVEL, psSapConfig->ui16SapOnLevel );
	ui32RegVal = ( ui32RegVal & (~BCHP_MASK( BTSC_CTRL_SAP_LEVEL, SAP_AM_OFF_LEVEL )))
		| BCHP_FIELD_DATA( BTSC_CTRL_SAP_LEVEL, SAP_AM_OFF_LEVEL, psSapConfig->ui16SapOffLevel );
	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_LEVEL, ui32RegVal );

	/* Set PLT_PSW_ON if the country code is Korea */
	eRfAudioStandard = phCapPort->hFmm->hRap->sInputSettings[phCapPort->eCapPort].sCapPortParams.uCapPortParams.sRfAudioParams.eRfAudioStandard;

	if (BRAP_RfAudioStandard_eKoreaA2==eRfAudioStandard)
	{
		ui32RegVal = BREG_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL );
		ui32RegVal = ( ui32RegVal & ( ~BCHP_MASK ( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON )))
					| BCHP_FIELD_DATA( BTSC_CTRL_STEREO_CTRL, PLT_PSW_ON, psSapConfig->bSapPswOn );
		BREG_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_STEREO_CTRL, ui32RegVal );
	}

	return err;
}

BERR_Code BRAP_RFAUDIO_P_GetSapConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioSapConfig			*psSapConfig		/* [out] SAP config */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_CTRL );
	psSapConfig->bSapMono1Mute0 = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_SAP_CTRL, SAP_MONO1_MUTE0 );
	psSapConfig->bSapPswOn = ( bool ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_SAP_CTRL, SAP_PSW_ON );

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_SAP_LEVEL );
	psSapConfig->ui16SapOffLevel = ( uint16_t ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_SAP_LEVEL, SAP_AM_OFF_LEVEL );
	psSapConfig->ui16SapOnLevel = ( uint16_t ) BCHP_GET_FIELD_DATA( ui32RegVal, BTSC_CTRL_SAP_LEVEL, SAP_AM_ON_LEVEL );

	return err;
}
	
BERR_Code BRAP_RFAUDIO_P_SetFirmwareReg ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t					uRegNum,	   /* [in] Reg number 1, 2, 3, 4 */
	bool						bValue		   /* true or false */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1 );

	/* Now, firmware only use JMPCOND_RESV3 */
	if(uRegNum == 3)
	{
		ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV3 );
		ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV3, bValue );
	}
	else if (uRegNum == 2)
	{
		ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2 );
		ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL1, JMPCOND_RESV2, bValue );
	}
	else
	{
		/*add if needed*/
	}

	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL1, ui32RegVal );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_ScaleOutput ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t ui32Reg							/* [in] Input gain scale */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2 );

	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL2, VAR_OUTSCALING );
	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL2, VAR_OUTSCALING, (ui32Reg & 0xffff) );

	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, ui32RegVal );

	return err;
}

BERR_Code BRAP_RFAUDIO_P_ScaleInput ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t ui32InputScaleValue				/* [in] Input scale value */
)
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ui32RegVal = 0;

	ui32RegVal = BRAP_Read32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2 );

	ui32RegVal &= ~BCHP_MASK( BTSC_CTRL_BASIC_CTRL2, FIXED_FMDIV_INSCALING );
	ui32RegVal |= BCHP_FIELD_DATA( BTSC_CTRL_BASIC_CTRL2, FIXED_FMDIV_INSCALING , (ui32InputScaleValue & 0xffff) );

	BRAP_Write32( phCapPort->hRegister, BCHP_BTSC_CTRL_BASIC_CTRL2, ui32RegVal );

	return err;

}

/* End of File */
	
