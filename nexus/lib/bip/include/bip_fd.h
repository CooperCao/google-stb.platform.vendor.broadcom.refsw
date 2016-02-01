/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BIP_FD_H
#define BIP_FD_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_fd

BIP_Fd - File Descriptor Utility Functions

**/


/**
Summary:
Read with timeout

Description:
A function to read from a file descriptor for a given number of bytes, or until the specified timeout.


Calling Context:
\code
    #include <unistd.h>
    #define ONE_SECOND_TIMEOUT   (1000)

    BIP_Status  rc;
    int         fd = STDIN_FILENO;
    char        inputBuffer[128];
    ssize_t     bytesRead;
    size_t      count = sizeof(inputBuffer);
    unsigned    timeInMs = ONE_SECOND_TIMEOUT;

    rc = BIP_Fd_ReadWithTimeout( fd, count, timeInMs, inputBuffer, &bytesRead);
\endcode

Input: fd  File descriptor to read from (returned from "open" system call).
Input: bufferSize Maximum number of bytes to read.
Input: timeout Maximum time (in milliseconds) to wait for input.
Output: inputBuffer address where received data should be placed.
Output: &bytesRead where to put number of bytes received.

Return:
    BIP_SUCCESS         :     Normal success (bytesRead==0 means EOF)
Return:
    BIP_INF_TIMEOUT     :     Timed out, no data received
Return:
    Other               :     Probably errno from select or read... Call BIP_ERR_TRACE()
**/

BIP_Status BIP_Fd_ReadWithTimeout(int fd, size_t count, unsigned timeInMs, void *inputBuffer, size_t *bytesRead );

#ifdef __cplusplus
}
#endif

#endif /* BIP_FD_H */
