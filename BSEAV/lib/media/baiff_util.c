/***************************************************************************
 *
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
 * BMedia library, AIFF stream 
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "baiff_util.h"

BDBG_MODULE(baiff_util);


bool 
baiff_read_comm(batom_cursor *cursor, baiff_comm *comm)
{
    uint8_t extended[10]; /* IEEE 80 bits  data type */
    uint64_t mantissa;
    int exponent;
    int i;

    comm->numChannels = batom_cursor_uint16_be(cursor);
    comm->numSampleFrames = batom_cursor_uint32_be(cursor);
    comm->sampleSize  = batom_cursor_uint16_be(cursor);
    batom_cursor_copy(cursor, extended, sizeof(extended));
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }

    for(mantissa=0,i=0;i<8;i++) {
        mantissa = (mantissa<<8) | extended[i+2];
    }
    exponent = (((int)extended[0]&0x7F)<<8 ) |  extended[1];
    if(exponent == 0x7FFF && mantissa) {
        return false;
    }
    exponent -= 16383 + 63;      
    if (extended[0]&0x80) {
        mantissa = -mantissa;
    }
    if(exponent<0) {
        mantissa = mantissa>>(-exponent);
    } else {
        mantissa = mantissa<<exponent;
    }
    comm->sampleRate = mantissa;
    BDBG_MSG(("baiff_read_comm: numChannels:%u numSampleFrames:%u sampleSize:%u", (unsigned)comm->numChannels, (unsigned)comm->numSampleFrames, (unsigned)comm->sampleSize));
    return true;
}



