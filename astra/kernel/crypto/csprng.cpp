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
 * csprng.cpp
 *
 *  Created on: May 18, 2015
 *      Author: gambhire
 */

#include "config.h"
#include "system.h"
#include "platform.h"
#include "crypto/csprng.h"
#include "lib_printf.h"

static uint8_t prngStorage[sizeof(CryptoPRNG)];

CryptoPRNG *CryptoPRNG::prng;
spinlock_t CryptoPRNG::lock;

void* CryptoPRNG::operator new(size_t sz, void *where) {
    UNUSED(sz);
    CryptoPRNG *cp = (CryptoPRNG *)where;

    return cp;
}

CryptoPRNG::CryptoPRNG(ChaCha20::Key256& key, uint64_t nonce) : streamCipher(key, nonce) {

}

CryptoPRNG::~CryptoPRNG() {

}

void CryptoPRNG::init() {
    prng = nullptr;
    spinlock_init("CryptoPRNG.lock", &lock);
}

CryptoPRNG *CryptoPRNG::instance() {
    SpinLocker locker(&lock);
    if (prng != nullptr)
        return prng;

    uint8_t seed[64];
    size_t rc = Platform::getRandomSeed(seed, 64);
    if (rc < 64) {
        err_msg("Could not generate random seed\n");
        System::halt();
    }

    uint64_t nonce;
    rc = Platform::getRandomSeed((uint8_t *)&nonce, 64);
    if (rc < 64) {
        err_msg("Could not generate random nonce\n");
        System::halt();
    }

    ChaCha20::Key256 key(seed);
    prng = new(prngStorage) CryptoPRNG(key, nonce);
    return prng;
}

size_t CryptoPRNG::read(uint8_t *buffer, size_t bufSize) {
    int rv = streamCipher.keyStreamBlock(buffer, bufSize);
    if (rv < 64)
        return rv;

    ChaCha20::Key256 key(buffer);
    streamCipher.reKey(key, (uint64_t)0, false);

    return rv;
}
