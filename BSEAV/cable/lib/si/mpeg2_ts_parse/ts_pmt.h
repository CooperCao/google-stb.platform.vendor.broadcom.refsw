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
#ifndef TS_PMT_H__
#define TS_PMT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint8_t		stream_type;
	uint16_t	elementary_PID;
} TS_PMT_stream;

/**
Returns true if it's a valid PMT
The other TS_PMT functions assume you have validated the buffer beforehand.
**/
bool TS_PMT_validate(const uint8_t *buf, unsigned bfrSize);

uint16_t TS_PMT_getPcrPid( const uint8_t *buf, unsigned bfrSize);
TS_PSI_descriptor TS_PMT_getDescriptor( const uint8_t *buf, unsigned bfrSize, int descriptorNum );

int TS_PMT_getNumStreams( const uint8_t *buf, unsigned bfrSize);
BERR_Code TS_PMT_getStream( const uint8_t *buf, unsigned bfrSize, int streamNum, TS_PMT_stream *p_stream );
TS_PSI_descriptor TS_PMT_getStreamDescriptor( const uint8_t *buf, unsigned bfrSize, int streamNum, int descriptorNum );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
