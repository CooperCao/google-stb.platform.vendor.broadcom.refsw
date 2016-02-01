/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "test.h"
#include "breg_i2c.h"
#include "breg_spi.h"
#ifdef DIAGS_BICAP_TEST
	#include "bbcp.h"
#endif
#ifdef DIAGS_ICAP_TEST
	#include "bicp.h"
#endif
#ifdef DIAGS_IRB_TEST
	#include "birb.h"
#endif
#include "bkir.h"
#ifdef DIAGS_KPD_TEST
	#include "bkpd.h"
#endif
#include "bled.h"
#include "bpwm.h"
#include "nexus_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
BERR_Code bcmOpenUpgDevices (void);
#if 0 /*TBD*/
BERR_Code bcmOpenIcpDevice ();
BERR_Code bcmOpenIrbDevice ();
#endif
BERR_Code bcmOpenKirDevice (void);
BERR_Code bcmOpenKpdDevice (bool use3250Kpd);
BERR_Code bcmOpenLedDevice (bool use3250Led);
BERR_Code bcmOpenPwmDevice (void);

NEXUS_I2cHandle bcmGetI2cHandle (uint32_t chan);
BREG_SPI_Handle bcmGetSpiRegHandle (uint32_t chan);
BKIR_Handle bcmGetKirHandle (void);
BKIR_ChannelHandle bcmGetKirChannelHandle (int chan);
#ifdef DIAGS_KPD_TEST
	BKPD_Handle bcmGetKpdHandle (void);
#endif
BLED_Handle bcmGetLedHandle (void);
BPWM_Handle bcmGetPwmHandle (void);
BPWM_ChannelHandle bcmGetPwmChannelHandle (uint32_t chan);
#ifdef DIAGS_IRB_TEST
	BIRB_Handle bcmGetIrbHandle (void);
#endif
#ifdef DIAGS_BICAP_TEST
	BBCP_Handle bcmGetBcpHandle (void);
	BBCP_ChannelHandle bcmGetBcpChannelHandle (uint32_t chan);
#endif
#ifdef DIAGS_ICAP_TEST
	BICP_Handle bcmGetIcpHandle (void);
	BICP_ChannelHandle bcmGetIcpChannelHandle (uint32_t chan);
#endif

extern NEXUS_I2cHandle diags_hI2c[];
extern BLED_Handle diags_hLed;
extern BKIR_ChannelHandle diags_hKirChan[];

#ifdef __cplusplus
}
#endif
