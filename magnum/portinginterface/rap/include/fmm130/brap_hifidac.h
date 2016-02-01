/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
*   Module name: HIFIDAC
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the top level FMM abstraction, which are exposed to the 
*   application developer.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/


#ifndef _BRAP_HIFIDAC_H_
#define _BRAP_HIFIDAC_H_

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
	Set the new volume level for the selected HiFiDAC.

Description:
	This function sets audio volume for HiFi DAC
	HiFi DAC volume control is linear in hardware ranging from 0
	min to 1FFFF max volume.  This PI has done a mapping
	from this linear range to 1/100 of DB.
	This function gets values in 1/100 of DB from 0 max to 10200 1/100 DB min,
	and for all values above 10200 the volume will be set to 0 linear
	or -102 DB.  Note: For fractions in DB a linear interpolation is used.
	PI maintains this volume upon channel change.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_GetDacVolume()
	BRAP_MIXER_SetOutputVolume()

Note: 
    The function BRAP_MIXER_SetOutputVolume() can also be used to set the DAC 
    volume. It is a generic function which can be used for any output port. It 
    works by scaling the data inside the Raptor Audio core at the mixer level.
    BRAP_OP_SetDacVolume() is an extra level of volume control specifically for 
    the DACs. It does volume control by specifically programming the DAC 
    registers.
    
****************************************************************************/
BERR_Code
BRAP_OP_SetDacVolume (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    uint32_t uiVolume				/* [in] volume attenuation in 1/100 dB*/
);

/***************************************************************************
Summary:
	Set the new volume level for the selected HiFiDAC.

Description:
	Following is the range of values which can be passed to the API.
	0x00000 = Mute
	0x1FFFF = Full scale
	The default value or Reset value of this SCALE register is 0x1cb80.

Returns:
	BERR_SUCCESS 

See Also:

Note: 
    The programming of SCALE register is done only through this API.
****************************************************************************/
BERR_Code
BRAP_OP_SetDacScaleVolume (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    uint32_t ui32ScaleVolume			/* [in] Scale volume */
);

/***************************************************************************
Summary:
	Retrieves the current volume level at the HifiDAC

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_SetDacVolume
	BRAP_MIXER_GetOutputVolume()	
****************************************************************************/
BERR_Code
BRAP_OP_GetDacVolume (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    uint32_t *puiVolume			/* [out] volume attenuation in 1/100 dB*/
);


#ifdef __cplusplus
}
#endif


#endif /* !_BRAP_HIFIDAC_H_ */

