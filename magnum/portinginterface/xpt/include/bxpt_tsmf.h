/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 *  Users of this technology  are responsible for complying to all laws of 
 *  Japan Ministry of Internal Affairs and Communication Requirements and 
 *  securing any applicable license requirements of the JCTEA.  BRCM conveys 
 *  no license to the TSMF technology or right to use.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BXPT_TSMF_H
#define BXPT_TSMF_H

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

/*=************************ Module Overview ********************************
<verbatim>

The API module provides configuration of transport TSMF(transport stream 
multiplex frame) related modules.  It is targetted for 7038 core based chip.

TSMF, a frame structure of 53 slots of 188 byte MPEG packets is parsed and
certain streams are accepted based on slot assignments configured by host
or extracted by hardware.  In the automatic mode, the host programs a 4 bit 
relative_ts_number, the hardware will generate the slot assignment table 
automatically once per frame.  In semi automatic mode, host will need to program
the slot assignment registers.  TSMF hardware also verifies certain fields(TEI,
PUSI,CC,ADAP,SC,...) and host can disable the verification for each field.
 
The API operates on a context handle called BXPT_Handle. 

Here is an example of configuraring TSMF:

int tsmfnum=0;
BXPT_TSMFFldVerifyConfig TsmfConfig;
BXPT_TSMF_InputSel	InputSel= BXPT_TSMF_InputSel_eIB0;
uint32_t SlotMapLo = 0xABCD1111;
uint32_t SlotMapHi = 0x22222222;

// Disable cc check and Tei check   
BXPT_TSMF_GetFldVerifyConfig(hXpt, tsmfnum,&TsmfConfig); 
TsmfConfig.CCCkDis = 1;
TsmfConfig.TeiChkDis =1;
BXPT_TSMF_SetFldVerifyConfig(hXpt, tsmfnum,&TsmfConfig);
 
//Set Semi auto mode
BXPT_TSMF_EnableSimiAutoMode( bXpt, tsmfnum,InputSel,SlotMapHi, SlotMapLo,1);
  
//Set Parser band 2 selecting TSMF0 enable
BXPT_TSMF_SetParserConfig(  hXpt, 2, 0, 1 );
//Set PCR1 selectingTSMF0 enable
BXPT_TSMF_SetPcrConfig(  hXpt, 1, 0, 1 );
</verbatim>
***************************************************************************/

/***************************************************************************
Summary: 
	Modes supported for accepting packets.
Description:
	This enum represents the modes that TSMF supports when there is a change for
	the version number field inside a frame header.
***************************************************************************/
typedef enum
{
	BXPT_TSMFVersionChgMode_eAllFrame,  /*Accept all Frame Header packets*/
	BXPT_TSMFVersionChgMode_eFrameChgVer /*Accept only Frame Header packets that has a change in version number field*/
}BXPT_TSMFVersionChgMode;

/***************************************************************************
Summary:
	Structure for field verification configuration of a TSMF module
Description:
	This structure represents configurations for enabling/disabling of certain
	field verification of a TSMF module. 
***************************************************************************/
typedef struct
{ 
	bool CrcChkDis;	        /* TSMF Frame Header CRC Check Disable=1 */
	bool RelTsStatusChkDis;	/* TSMF Frame Header Rel_TS_Status Check Disable=1*/
	bool FrameTypeChkDis;  /* TSMF Frame Header Frame_Type Check Disable=1*/
	bool RelTsModeChkDis;  /*TSMF Frame Header REL_TS_MODE Check Disable */
	bool SyncChkFDis;      /*TSMF Frame Header SYNC Check Disable */
	bool CCCkDis;          /*TSMF Frame Header CC Check Disable */
	bool AdapChkDis;       /*TSMF Frame Header ADAP Check Disable */
	bool ScChkDis;         /*TSMF Frame Header SC Check Disable */
	bool TsPriorChkDis;    /*TSMF Frame Header TS_PRIOR Check Disable */
	bool PusiChkDis;       /*TSMF Frame Header PUSI Check Disable */
	bool TeiChkDis;        /*TSMF Frame Header TEI Check Disable */
	BXPT_TSMFVersionChgMode VersionChgMode; /*TSMF Version Change Mode */
} BXPT_TSMFFldVerifyConfig;

/***************************************************************************
Summary: 
	All the Inputs selections.
Description:
	This enum represents all the input selections that a TSMF module could take.
***************************************************************************/
typedef enum
{
	BXPT_TSMF_InputSel_eIB0 = 0,
	BXPT_TSMF_InputSel_eIB1,
	BXPT_TSMF_InputSel_eIB2,
	BXPT_TSMF_InputSel_eIB3,
	BXPT_TSMF_InputSel_eIB4,
	BXPT_TSMF_InputSel_eIB5,
	BXPT_TSMF_InputSel_eIB6,
	BXPT_TSMF_InputSel_eIB7,
	BXPT_TSMF_InputSel_eIB8,
	BXPT_TSMF_InputSel_eIB9,
    BXPT_TSMF_InputSel_ePB0,
	BXPT_TSMF_InputSel_eRMX = 0x10
}BXPT_TSMF_InputSel;

/***************************************************************************
Summary:
Gets the configuration for a given TSMF module.
Description:
	This function gets all the settings of a TSMF module.  It is recommended to get
	the configuration before calling BXPT_SetTSMFConfig so that some original 
	settings can be preserved. 
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code BXPT_TSMF_GetFldVerifyConfig( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	int TSMFNum,			/* [in] Which TSMF. */
	BXPT_TSMFFldVerifyConfig *TsmfConfig	/* [in] Which TSMF configuraion. */ 
 	);

/***************************************************************************
Summary:
	Sets a verification field configuration of TSMF module.
Description:
	This function sets verifivation field configurations for a given TSMF module.  
	It is recommended to call BXPT_getTSMFConfig to reserve some original settings. 
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter   
***************************************************************************/
BERR_Code BXPT_TSMF_SetFldVerifyConfig( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	int TSMFNum,			/* [in] Which TSMF. */
	const BXPT_TSMFFldVerifyConfig *TsmfConfig /* [in] Which TSMF configuration structure. */ 
	);

/***************************************************************************
Description:
	Enable/Disable aumatic mode TSMF.
Summary:
	This function enables automatic mode TSMF module. 
Returns: 
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code BXPT_TSMF_EnableAutoMode( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	BXPT_TSMF_InputSel	eInputSel,  /*TSMF Parser Input Select*/
	int TSMFNum,			/* [in] Which TSMF. */
	uint8_t RelativeTsNo     /*TSMF Relative TS Number */
 	);
/***************************************************************************
Description:
	Enable/Disable aumatic mode TSMF.
Summary:
	This function enables semi automatic mode TSMF module.
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter  
***************************************************************************/
BERR_Code BXPT_TSMF_EnableSemiAutoMode( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	BXPT_TSMF_InputSel	eInputSel,  /*TSMF Parser Input Select*/
	int TSMFNum,			/* [in] Which TSMF. */
	uint32_t SlotMapLo,	    /* [in]Lower 32 bits (LSBs) of the 52-bit TSMF Slot Map vector*/
	uint32_t SlotMapHi,     /* [in]Upper 21 bits of the 53-bit Slot Map vector for TSMF Demultiple*/ 
	uint8_t RelativeTsNo    /* [in]TSMF Relative TS Number */ 
	);

/***************************************************************************
Description:
	Disables TSMF module.
Summary:
	This function disables TSMF module. 
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code BXPT_TSMF_DisableTsmf( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	int TSMFNum			/* [in] Which TSMF. */
 	);

/***************************************************************************
Description:
	Enable/Disable TSMF control for a given parse band.
Summary:
	This function sets TSMF related configurations for a given parser band.  It
	selects a TSMF module and disable/enables TSMF parsing for the parser band
	module.
Returns:
    BERR_SUCCESS                - if successful 
    BERR_INVALID_PARAMETER      - Bad input parameter 
***************************************************************************/
BERR_Code BXPT_TSMF_SetParserConfig( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for the transport */
	int uiParserBandNum,	/* [in] Which Parser band. */
	int TSMFNum,			/* [in] Which TSMF. */
	bool   TsmftoParserEn	    /* [in] enable/disable the selected TSMF to the parser*/
	);

#ifdef __cplusplus
}
#endif

#endif

