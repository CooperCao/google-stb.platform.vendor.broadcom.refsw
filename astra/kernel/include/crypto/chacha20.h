/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/*
 * chacha20.h
 *
 *  Created on: May 14, 2015
 *      Author: gambhire
 */

#ifndef INCLUDE_CRYPTO_CHACHA20_H_
#define INCLUDE_CRYPTO_CHACHA20_H_

#include <cstdint>
#include <cstddef>

#include "lib_string.h"

class ChaCha20 {
public:

    class Key256 {
    public:
        Key256(uint8_t *keyArray) { memcpy(key, keyArray, 32); }
        ~Key256() {}

        uint8_t *bytes() { return key; }
        uint32_t *words() { return (uint32_t *)&key[0]; }

    private:
        uint8_t key[32];
    };

public:
    ChaCha20(Key256& key, uint64_t nonce);
    ~ChaCha20() {}

    size_t keyStreamBlock(uint8_t *block, size_t blockSize);

    void encrypt(const uint8_t *msg, size_t msgLen, uint8_t *encMsg);
    void decrypt(const uint8_t *encMsg, size_t msgLen, uint8_t *msg) { encrypt(encMsg, msgLen, msg); }

    void reKey(Key256& key, uint64_t nonce, bool clearCounter = true);

private:
    inline uint32_t rotl32(uint32_t val, int n) { return (val << n) | (val >> (32 - n)); }

    inline void quarterRound(uint32_t *block, int a, int b, int c, int d) {
        block[a] += block[b]; block[d] = rotl32(block[d] ^ block[a], 16);
        block[c] += block[d]; block[b] = rotl32(block[b] ^ block[c], 12);
        block[a] += block[b]; block[d] = rotl32(block[d] ^ block[a], 8);
        block[c] += block[d]; block[b] = rotl32(block[b] ^ block[c], 7);
    }

private:
    uint32_t state[16];
    uint8_t keyStream[64];
    int keyStreamIdx;
};



#endif /* INCLUDE_CRYPTO_CHACHA20_H_ */
