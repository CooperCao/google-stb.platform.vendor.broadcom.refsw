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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BHAB_PRIV_H
#define BHAB_PRIV_H


#ifdef __cplusplus
extern "C" {
#endif

#include "bkni.h"
#include "bhab.h"


typedef struct BHAB_P_Handle
{
    BHAB_Settings settings;    /* user settings */
    void *pImpl;                /* pointer to chip-specific structure */
    BHAB_ChannelCapability *channelCapabilities;    /* pointer to tuner capabilities */
    uint8_t totalTunerChannels;
} BHAB_P_Handle;


#ifdef __cplusplus
}
#endif

#endif
