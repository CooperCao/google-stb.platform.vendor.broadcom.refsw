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
 * chacha20.cpp
 *
 *  Created on: May 14, 2015
 *      Author: gambhire
 */

#include "crypto/chacha20.h"

#include "lib_printf.h"

void ChaCha20::reKey(ChaCha20::Key256& key, uint64_t nonce, bool clearCounter) {
    const char *constStr = "expand 32-byte k";
    const uint32_t *constWords = (const uint32_t *)constStr;

    state[0] = constWords[0];
    state[1] = constWords[1];
    state[2] = constWords[2];
    state[3] = constWords[3];

    const uint32_t *keyWords = key.words();
    state[4] = keyWords[0];
    state[5] = keyWords[1];
    state[6] = keyWords[2];
    state[7] = keyWords[3];
    state[8] = keyWords[4];
    state[9] = keyWords[5];
    state[10] = keyWords[6];
    state[11] = keyWords[7];

    if (clearCounter == true) {
        state[12] = 0; // The block counter
        state[13] = 0;
    }

    state[14] = (uint32_t )(nonce >> 32);
    state[15] = (uint32_t )(nonce & 0xffffffff);

    keyStreamIdx = 64;
}

ChaCha20::ChaCha20(ChaCha20::Key256& key, uint64_t nonce) {
    reKey(key, nonce);
}

size_t ChaCha20::keyStreamBlock(uint8_t *block, size_t blockSize) {

    int numBlocks = blockSize/64;
    uint32_t *currBlock = (uint32_t *)block;

    for (int i=0; i<numBlocks; i++) {
        memcpy(currBlock, state, 64);

        for (int j=8; j> 0; j-=2) {
            quarterRound(currBlock, 0, 4, 8, 12);
            quarterRound(currBlock, 1, 5, 9, 13);
            quarterRound(currBlock, 2, 6, 10, 14);
            quarterRound(currBlock, 3, 7, 11, 15);
            quarterRound(currBlock, 0, 5, 10, 15);
            quarterRound(currBlock, 1, 6, 11, 12);
            quarterRound(currBlock, 2, 7, 8, 13);
            quarterRound(currBlock, 3, 4, 9, 14);
        }

        uint32_t *currWord = (uint32_t *)currBlock;
        for (int j=0; j<16; j++) {
            currWord[j] += state[j];
        }

        // Increment the 256bit counter.
        uint32_t *counter = &state[12];
        counter[0]++;
        if (counter[0] == 0) {
            counter[1]++;
            if (counter[1] == 0) {
                counter[2]++;
                if (counter[2] == 0)
                    counter[3]++;
            }
        }

        currBlock += 16;
    }

    return numBlocks*64;
}

void ChaCha20::encrypt(const uint8_t *msg, size_t msgLen, uint8_t *encMsg) {

    const uint8_t *currMsg = msg;
    const uint8_t *msgEnd = msg + msgLen;

    uint8_t *currEncMsg = encMsg;

    while (currMsg < msgEnd) {

        if (keyStreamIdx == 64) {
            keyStreamBlock(keyStream,64);
            keyStreamIdx = 0;
        }

        *currEncMsg = *currMsg | keyStream[keyStreamIdx];

        currEncMsg++;
        currMsg++;
        keyStreamIdx++;
    }
}
