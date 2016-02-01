/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

#include "ENDIANESS.h"



#if defined(BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS)
/* Original code from libfp/host_src/endian.c */

uint16_t ENDIANESS_bswap16(const uint16_t arg)
{
    return ((arg<<8) | (arg>>8));
}


uint32_t ENDIANESS_bswap32(const uint32_t arg)
{
    uint8_t res[4];

    res[0] = ((uint8_t *)(&arg))[3];
    res[1] = ((uint8_t *)(&arg))[2];
    res[2] = ((uint8_t *)(&arg))[1];
    res[3] = ((uint8_t *)(&arg))[0];

    return (*(uint32_t *)res);
}


uint64_t ENDIANESS_bswap64(const uint64_t arg)
{
    uint8_t res[8];

    res[0] = ((uint8_t *)(&arg))[7];
    res[1] = ((uint8_t *)(&arg))[6];
    res[2] = ((uint8_t *)(&arg))[5];
    res[3] = ((uint8_t *)(&arg))[4];
    res[4] = ((uint8_t *)(&arg))[3];
    res[5] = ((uint8_t *)(&arg))[2];
    res[6] = ((uint8_t *)(&arg))[1];
    res[7] = ((uint8_t *)(&arg))[0];

    return (*(uint64_t *)res);
}

#elif FEATURE_IS(SW_HOST, RAAGA_MAGNUM)

/* To avoid the "ISO C forbids an empty translation unit" warning */
void ENDIANESS_unused(void)
{
}

#endif
