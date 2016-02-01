/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_PLL_H_
#define BAPE_PLL_H_

/***************************************************************************
Summary:
PLL Settings
***************************************************************************/
typedef struct BAPE_PllSettings
{
    bool freeRun;   /* If true, use an internal 27 MHz clock source instead of a VCXO rate manager */
    unsigned vcxo;  /* Which VCXO Rate Manager will drive this PLL */
} BAPE_PllSettings;

/***************************************************************************
Summary:
Get PLL Settings
***************************************************************************/
void BAPE_Pll_GetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    BAPE_PllSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set PLL Settings
***************************************************************************/
BERR_Code BAPE_Pll_SetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    const BAPE_PllSettings *pSettings
    );

#endif /* #ifndef BAPE_PLL_H_ */

