/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#ifndef _BAST_4528_H__
#define _BAST_4528_H__               

#include "bast.h"
#include "breg_i2c.h"


/******************************************************************************
Summary:
   configuration parameter addresses
Description:
   These are the configuration parameters that can be accessed via 
   BAST_ReadConfig() and BAST_WriteConfig().
See Also:
   BAST_ReadConfig(), BAST_WriteConfig()
******************************************************************************/
#define BAST_4528_CONFIG_MTSIF_CTL                3 /* crystal freq in Hz */
#define BAST_4528_CONFIG_LEN_MTSIF_CTL            4  
#define BAST_4528_CONFIG_DEBUG_CTL                4 /* firmware debug output control */   
#define BAST_4528_CONFIG_LEN_DEBUG_CTL            1   
#define BAST_4528_CONFIG_FREQ_ESTIMATE_STATUS     5 /* DFT freq estimate status */
#define BAST_4528_CONFIG_LEN_FREQ_ESTIMATE_STATUS 1
#define BAST_4528_CONFIG_IF_STEP_SAVE             6 /* carrier frequency estimate */
#define BAST_4528_CONFIG_LEN_IF_STEP_SAVE         4
#define BAST_4528_CONFIG_TUNER_CTL                7 /* tuner control options */
#define BAST_4528_CONFIG_LEN_TUNER_CTL            1
#define BAST_4528_CONFIG_ACQ_TIME                 8 /* acquisition time */
#define BAST_4528_CONFIG_LEN_ACQ_TIME             4


/******************************************************************************
Summary:
   Structure containing AVS status
Description:
   This structure contains AVS status.
See Also:
   BAST_GetAvsStatus()
******************************************************************************/
typedef struct BAST_AvsStatus
{
   bool         bEnabled;
   int32_t      temperature;  /* temperature in 1/10 degrees Celsius */
   uint32_t     voltage_1p1_0; /* in mV */
} BAST_AvsStatus;


/***************************************************************************
Summary:
   This function returns the default settings for 4528 module.
Description:
   This function returns the default settings for 4528 module.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4528_GetDefaultSettings(
   BAST_Settings *pSettings   /* [out] Default settings */
);


/***************************************************************************
Summary:
   This function returns the default settings for 4528 channel device.
Description:
   This function returns the default settings for 4528 channel device.
Returns:
   BERR_Code
See Also:
   BAST_Open()
****************************************************************************/
BERR_Code BAST_4528_GetChannelDefaultSettings(
   BAST_Handle h, 
   uint32_t chnNo, 
   BAST_ChannelSettings *pChnDefSettings
);


/***************************************************************************
Summary:
   This function prints a message from the UART. 
Description:
   This function transmits the given null-terminated string to the UART at
   115200 8-N-1.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_PrintUart(BAST_Handle h, char *pBuf);


/***************************************************************************
Summary:
   This function configures BERT output to GPO/GPIO pins.
Description:
   If ctrl[3]=1, BERT output is enabled for the channel specified by 
   ctrl[2:0].
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_ConfigXbert(BAST_Handle h, uint8_t ctrl);


/***************************************************************************
Summary:
   This function enables/disables AVS.
Description:
   This function enables/disables AVS.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_EnableAvs(BAST_Handle h, bool bEnable);

/***************************************************************************
Summary:
   This function returns PVT status information.
Description:
   This function returns PVT status information.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_GetAvsStatus(BAST_Handle h, BAST_AvsStatus *pStatus);


/***************************************************************************
Summary:
   This function initiates flash sector programming.
Description:
   This function instructs the LEAP to start programming the sector given
   by the specified addr.  The sector data must be initialized in the 4 KB
   LEAP memory buffer (address 0x44000) prior to calling this function.
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_WriteFlashSector(BAST_Handle h, uint32_t addr);


#ifndef BAST_USE_HAB_PI
/***************************************************************************
Summary:
   This function writes to the BBSI register space.
Description:
   This function writes to the BBSI register space. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_WriteBbsi(BAST_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);


/***************************************************************************
Summary:
   This function reads from the BBSI register space.
Description:
   This function reads from the BBSI register space. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_ReadBbsi(BAST_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);


/***************************************************************************
Summary:
   This function writes to the RBUS register space.
Description:
   This function writes to the RBUS register space. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_WriteRbus(BAST_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);


/***************************************************************************
Summary:
   This function reads from the RBUS register space.
Description:
   This function reads from the RBUS register space. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_ReadRbus(BAST_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);


/***************************************************************************
Summary:
   This function writes to the LEAP memory. 
Description:
   This function writes to the LEAP memory. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_WriteMemory(BAST_Handle h, uint32_t addr, uint8_t *buf, uint32_t n);


/***************************************************************************
Summary:
   This function reads from the LEAP memory. 
Description:
   This function reads from the LEAP memory. 
Returns:
   BERR_Code
See Also:
   none
****************************************************************************/
BERR_Code BAST_4528_ReadMemory(BAST_Handle h, uint32_t addr, uint8_t *buf, uint32_t n);
#endif /* BAST_USE_HAB_PI */

#ifdef __cplusplus
}
#endif

#endif /* BAST_4528_H__ */

