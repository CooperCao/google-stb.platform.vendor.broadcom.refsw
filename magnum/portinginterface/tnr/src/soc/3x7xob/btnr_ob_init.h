/*************************************************************************
*     (c)2005-2011 Broadcom Corporation
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

#ifndef _BTNR_OB_INIT_H__
#define _BTNR_OB_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif


#define BTNR_OB_SAW_TABLE_SIZE  1
#define BTNR_OB_SAW_COEFF_SIZE 33
static const uint32_t BTNR_Ob_SAW_Table[BTNR_OB_SAW_TABLE_SIZE][BTNR_OB_SAW_COEFF_SIZE] =
{
	
	/* OOB SAW filter */
	{ 0x0000006B, 0x000001D6, 0x00000234, 0x00000008, 0x0007FD1E, 0x0007FE21, 0x0000031D, 0x000004C8,
	  0x0007FE73, 0x0007F824, 0x0007FD70, 0x0000097D, 0x00000921, 0x0007F84B, 0x0007EF2E, 0x000000D5,
	  0x000016FD, 0x00000BC0, 0x0007E801, 0x0007E31E, 0x00000FE1, 0x00002F39, 0x000004DB, 0x0007C2D4,
	  0x0007D721, 0x00003EBD, 0x00005F58, 0x0007D908, 0x00074BA9, 0x0007CF91, 0x0001882C, 0x00032323,
	  0x00000000
	}

};




BERR_Code BTNR_Ob_P_TnrInit(BTNR_Ob_3x7x_Handle hTnr, uint32_t RfFreq); 
void BTNR_Ob_P_Program_TNR(BTNR_Ob_3x7x_Handle hTnr, uint32_t RfFreq);


#ifdef __cplusplus
}
#endif

#endif