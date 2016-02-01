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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
 
#ifndef TEST_H
#define TEST_H

#include "bchp_common.h"

#ifdef BCHP_BSCA_REG_START
    #define DIAGS_I2C_TEST
#endif

#ifdef BCHP_BICAP_REG_START
    #define DIAGS_BICAP_TEST
#endif

#ifdef BCHP_ICAP_REG_START
    #define DIAGS_ICAP_TEST
#endif

#ifdef BCHP_IRB_REG_START
    #define DIAGS_IRB_TEST
#endif

#define DIAGS_KIR_TEST
/*#define DIAGS_KPD_TEST*/
#define DIAGS_PWM_TEST

#ifdef BCHP_MSPI_REG_START
    #define DIAGS_SPI_TEST
#endif

/*#define DIAGS_TIMER_TEST*/

#if defined(BCHP_UHFR_REG_START) || defined(BCHP_UHFR_1_REG_START)
    #define DIAGS_UHFR_TEST
#endif

#endif
