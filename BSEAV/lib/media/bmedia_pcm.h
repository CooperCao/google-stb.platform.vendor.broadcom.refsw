/***************************************************************************
 *     Copyright (c) 2011-2013, Broadcom Corporation
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
 *******************************************************************************/
#ifndef _BMEDIA_PCM_H__
#define _BMEDIA_PCM_H__

#include "bfile_io.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bpcm_file_config {
    uint8_t channel_count;  /* number of channels  */
    uint8_t sample_size; /* number of bits in the each sample */
    unsigned sample_rate; /* audio sampling rate in Hz */
    bool littleEndian;
} bpcm_file_config;

typedef struct bpcm_file *bpcm_file_t;
void bpcm_file_config_init(bpcm_file_config *config);
bpcm_file_t bpcm_file_create(bfile_io_read_t fd, const bpcm_file_config *config);
void bpcm_file_destroy(bpcm_file_t pcm);
bfile_io_read_t bpcm_file_get_file_interface(bpcm_file_t pcm);

#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PCM_H__ */

