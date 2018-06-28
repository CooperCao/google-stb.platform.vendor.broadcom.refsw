/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#include "console.h"
#include "eventqueue.h"
#include "poll.h"
#include "uartfops.h"
#include "uart_boot.h"

using namespace UartFops;

UartFile* UartFile::instance = 0;
static uint8_t puartFileMem[sizeof(UartFile)];

void UartFops::init(void) {
    /* parse device tree for uart*/
}

void* UartFile::operator new(size_t sz, void* where) {
    UNUSED(sz);
    UartFile *rv = (UartFile*)where;
    return rv;
}

void UartFile::init() {
    instance = new(puartFileMem) UartFile();
}

size_t UartFile::write(const void *data, const size_t numBytes, const uint64_t offset) {
    UNUSED(offset);
    const uint8_t *cdata = (const uint8_t *)data;
    for (size_t i=0; i<numBytes; i++) {
        if (cdata[i] == '\n'){
            if(IUart::uart)
                IUart::uart->putc('\r');
            else
                early_uart_putc('\r');
        }

        if(IUart::uart)
            IUart::uart->putc(cdata[i]);
        else
            early_uart_putc(cdata[i]);
    }

    return numBytes;
}

ssize_t UartFile::writev(const iovec *iov, int iovcnt, const uint64_t offset) {

    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        rv += write(iov[i].iov_base, iov[i].iov_len, offset);
    }

    return rv;
}

void UartFile::addWatcher(short pollEvent, short *pollResult, EventQueue *eq) {
    *pollResult = 0;
    if (pollEvent & POLLOUT) {
        *pollResult |= POLLOUT;
        eq->signal();
    }
}

size_t UartFile::read(const void *data, const size_t numBytes, const uint64_t offset) {
    UNUSED(offset);

    int readCount = 0;
    uint8_t *cdata = (uint8_t *)data;
    while (readCount < numBytes) {
        char c;
        if(IUart::uart)
            IUart::uart->getc(&c);
        else
            return readCount;

        if (c == '\r')
            break;

        cdata[readCount] = c;
        readCount++;
    }

    return readCount;
}

ssize_t UartFile::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        rv += read(iov[i].iov_base, iov[i].iov_len, offset);
    }

    return rv;
}
