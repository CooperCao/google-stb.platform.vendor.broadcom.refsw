/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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

typedef enum BAPE_PllMode
{
    BAPE_PllMode_eAuto,     /* Automatically set to the required sample rate */
    BAPE_PllMode_eCustom,   /* Manually set to a fixed rate */
    BAPE_PllMode_eMax
} BAPE_PllMode;

/***************************************************************************
Summary:
PLL Settings
***************************************************************************/
typedef struct BAPE_PllSettings
{
    BAPE_PllMode mode; /* set the mode this PLL will operate in */

    unsigned vcxo;  /* Which VCXO Rate Manager will drive this PLL */
    
    /* The following settings are for "Custom" PLL Mode to drive an external
       mclock [for a non audio application] */

    unsigned nDivInt; /* manually set the integer feedback divider 
                        Feedback divider integer control: (divide ratio=code)
                        Valid values are 0-255, 0 indicates 256
                            0 = divide-by-256
                            1 = divide-by-1
                            2 = divide-by-2
                            :     :
                        254 = divide-by-254
                        255 = divide-by-255 */

    unsigned mDivCh0;  /* manually set the channel divider 
                        Clock channel post divider control (divide ratio = code value)
                        Valid values are 0-255, 0 indicates 256
                        NOTE: Ch0 output frequency is equal to Fout = (1/2)(1/Pdiv)*Fref*(nDivInt/Mdiv)
                            0 = 256
                            1 =   1
                            2 =   2
                            :     :
                        254 = 254
                        255 = 255 */
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

/***************************************************************************
Summary:
Enable an External Mclk Output
***************************************************************************/
BERR_Code BAPE_Pll_EnableExternalMclk(
    BAPE_Handle     handle,
    BAPE_Pll        pll,
    unsigned        mclkIndex,
    BAPE_MclkRate   mclkRate
    );

#endif /* #ifndef BAPE_PLL_H_ */

