/***************************************************************************
 *     Copyright (c) 2004-2005, Broadcom Corporation
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
 ****************************************************************/
#ifndef TSPSI_VALIDATE_H__
#define TSPSI_VALIDATE_H__

/*=**************************************
This module is to verify that a correct message has been returned by
the transport message filtering hardware. Due to a hardware limitation,
the first message returned after starting message capture may be corrupt.

PR's for this issue include 11331 and 8622.

This check does not provide an absolute guarantee that the message is correct,
but it obtains a high degree of probability. The tests are as follows:

1. The application should only call these functions if it is the first
message captured after start.
2. Run TS_Filter_Compare to verify that the filter matches the data.
3. Run TS_Validate_Size to make verify that we have only whole messages.
*****************************************/

#ifndef USE_LEGACY_KERNAL_INTERFACE
#include <bstd.h>
#else
#include <brcm_t.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
CPU-based filter comparison.
**/
bool TS_Filter_Compare( const uint8_t *msg, const uint8_t *inclMask, const uint8_t *exclMask,
	const uint8_t *coef, size_t filterSize );

/**
Return the current message size.
Description:
This assumes msg is pointing to a valid message.
The size includes any end padding.
**/
int TS_Get_Message_Size(const uint8_t *msg);

/**
Validate that a message size is exactly equal to the sum of its section lengths.
Description:
This test is only valid if the message starts at the beginning of the buffer.
The transport hardware will only write whole messages into the buffer, but if
you're not at the beginning, you may have wrap around and this test will fail.
**/
bool TS_Validate_Size( const uint8_t *msg, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* TSPSI_VALIDATE_H__ */
