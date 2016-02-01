/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef _BWFE_4538_H__
#define _BWFE_4538_H__

#ifdef __cplusplus
extern "C" {
#endif



/***************************************************************************
Summary:
   This function returns the default settings for g2 module.
Description:
   This function returns the default settings for g2 module.
Returns:
   BERR_Code
See Also:
   BWFE_Open()
****************************************************************************/
BERR_Code BWFE_4538_GetDefaultSettings(
   BWFE_Settings *pDefSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for g2 channel device.
Description:
   This function returns the default settings for g2 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BWFE_4538_GetChannelDefaultSettings(
   BWFE_Handle h, 
   uint8_t chanNum, 
   BWFE_ChannelSettings *pChnDefSettings
);


#ifdef __cplusplus
}
#endif

#endif /* _BWFE_4538_H__ */

