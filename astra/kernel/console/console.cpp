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
#include "vuartfops.h"
#include "uartfops.h"

IFile *Console::stdin;
IFile *Console::stdout;
IFile *Console::stderr;
IFile *Console::stdlog;

static uint8_t stdoutMem[sizeof(Console::Stdout)];
static uint8_t stdinMem[sizeof(Console::Stdin)];

Console::Stdout* Console::Stdout::out;
Console::Stdin* Console::Stdin::in;

IFile* Console::Stdout::fops = 0;
IFile* Console::Stdin::fops = 0;

void Console::init(bool isVuartChosen) {
    VuartFops::init(isVuartChosen);
    UartFops::init();
    Console::Stdin::init();
    Console::Stdout::init();

    if (isVuartChosen) {
        Console::Stdout::setfops((IFile *)VuartFops::VuartFile::getInstance());
        Console::Stdin::setfops((IFile *)VuartFops::VuartFile::getInstance());
    }
}

void* Console::Stdout::operator new(size_t sz, void *where) {
    UNUSED(sz);
    Console::Stdout *rv = (Console::Stdout *)where;
    return rv;
}

void* Console::Stdin::operator new(size_t sz, void *where) {
    UNUSED(sz);
    Console::Stdin *rv = (Console::Stdin *)where;
    return rv;
}

void Console::Stdout::init() {
    out = new(stdoutMem) Console::Stdout();
    stdout = out;
    stderr = out;
    stdlog = out;
    setfops((IFile *)UartFops::UartFile::getInstance());
}

size_t Console::Stdout::read(const void *data, const size_t numBytes, const uint64_t offset) {
    UNUSED(data);
    UNUSED(numBytes);
    UNUSED(offset);

    return -ENOSYS;
}

ssize_t Console::Stdout::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

    UNUSED(iov);
    UNUSED(iovcnt);
    UNUSED(offset);

    return -ENOSYS;
}

size_t Console::Stdout::write(const void *data, const size_t numBytes, const uint64_t offset) {
    return fops->write(data, numBytes, offset);
}

ssize_t Console::Stdout::writev(const iovec *iov, int iovcnt, const uint64_t offset) {
    return fops->writev(iov, iovcnt, offset);
}

void Console::Stdout::addWatcher(short pollEvent, short *pollResult, EventQueue *eq) {
    return fops->addWatcher(pollEvent, pollResult, eq);
}

void Console::Stdin::init() {
    in = new(stdinMem) Console::Stdin();
    stdin = in;
    setfops((IFile *)UartFops::UartFile::getInstance());
}

size_t Console::Stdin::read(const void *data, const size_t numBytes, const uint64_t offset) {
    return fops->read(data, numBytes, offset);
}

ssize_t Console::Stdin::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

    return fops->readv(iov, iovcnt, offset);
}

size_t Console::Stdin::write(const void *data, const size_t numBytes, const uint64_t offset) {
    UNUSED(data);
    UNUSED(numBytes);
    UNUSED(offset);

    return -ENOSYS;
}

ssize_t Console::Stdin::writev(const iovec *iov, int iovcnt, const uint64_t offset) {
    UNUSED(iov);
    UNUSED(iovcnt);
    UNUSED(offset);

    return -ENOSYS;
}

void Console::Stdin::addWatcher(short pollEvent, short *pollResult, EventQueue *eq) {
    // Do not support for now.
    UNUSED(pollEvent);
    UNUSED(eq);

    *pollResult = 0;
}

void kernel_write(const void *data, const size_t numBytes)
{
    if(Console::stdout)
        Console::stdout->write(data,numBytes,0);
    else
        if(IUart::uart) IUart::uart->puts((char *)data);
}
