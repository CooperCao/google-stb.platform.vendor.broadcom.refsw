/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
#include "bstd.h"
#include "ts_priv.h"
#include "ts_ca.h"

TS_PSI_descriptor TS_CA_getDescriptor( const uint8_t *buf, int descriptorNum )
{
	return TS_P_getDescriptor( &buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1], TS_PSI_MAX_BYTE_OFFSET(buf)-TS_PSI_LAST_SECTION_NUMBER_OFFSET, descriptorNum );
}
