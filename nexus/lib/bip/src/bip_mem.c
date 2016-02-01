/******************************************************************************
 * (c) 2014 Broadcom Corporation
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

#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/poll.h>
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

#include "bip_priv.h"
#include "bip_mem.h"

BDBG_MODULE( bip_mem );


void BIP_Mem_Dump(const char * msg, const void * dumpBuf, size_t len)
{
    unsigned char  * buf = (unsigned char *) dumpBuf;   /* Incomming buffer to be dumped. */
    char             fmtBuf[256];

    unsigned         fmtIdx;                            /* which byte in fmtBuf */
    unsigned         lineIdx;                           /* which value on the line */
    unsigned         bufIdx;                            /* index into buf */

    const unsigned   bytesPerLine = 16;

    for (bufIdx=0 ; bufIdx<len ; bufIdx+=bytesPerLine)
    {
        fmtIdx = 0;
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%s: ", msg);
        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " 0x%04x: ", bufIdx);

        lineIdx = 0;
        while (lineIdx<16 )
        {
            if (lineIdx == 8)
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "  ");
            }

            if (bufIdx+lineIdx < len)
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " %02x", buf[bufIdx+lineIdx]);
            }
            else
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   ");
            }
            lineIdx++;
        }

        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "   |");

        lineIdx = 0;
        while (lineIdx<16 )
        {
            if (bufIdx+lineIdx < len)
            {
                unsigned char   byteToPrint = buf[bufIdx+lineIdx];
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "%c", isprint(byteToPrint)?byteToPrint:'.');
            }
            else
            {
                fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, " ");
            }
            lineIdx++;
        }

        fmtIdx += snprintf(fmtBuf+fmtIdx, sizeof fmtBuf - fmtIdx, "|");

        BDBG_LOG(("%s", fmtBuf));
    }
    return;
}
