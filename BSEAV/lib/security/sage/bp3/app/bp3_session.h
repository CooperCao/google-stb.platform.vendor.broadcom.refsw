/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BSEAV_LIB_SECURITY_SAGE_BP3_APP_BP3_SESSION_H_
#define BSEAV_LIB_SECURITY_SAGE_BP3_APP_BP3_SESSION_H_

#include "nexus_base_types.h"
#include "nexus_otpmsp.h"
#include "nexus_read_otp_id.h"

int bp3_session_start(uint8_t **token, uint32_t *size);
int bp3_session_end(uint8_t *ccfBuf, uint32_t ccfSize, uint8_t **logBuf, uint32_t *logSize, uint32_t **status, uint32_t *statusSize);

NEXUS_OtpKeyType find_otp_select(void);
NEXUS_Error read_otp_id(NEXUS_OtpIdType keyType, uint32_t *otpIdHi, uint32_t *otpIdLo);

#define MAP_MEM_START \
  int memfd = open("/dev/mem", O_RDONLY | O_SYNC); \
  if (memfd > 0) { \
    fcntl(memfd, F_SETFD, FD_CLOEXEC); \
  } else { \
    perror("open /dev/mem"); \
    return 1; \
  } \
  uint32_t addr, aliged;\
  size_t size; \
  void *page;


#define MAP_MEM_END close(memfd);

#define MAP_START(reg, num) \
  addr = BCHP_PHYSICAL_OFFSET + reg; \
  aliged = addr & ~(sysconf(_SC_PAGE_SIZE) - 1); \
  addr -= aliged; \
  size = addr + num * sizeof(uint32_t); \
  page = mmap(NULL, size, PROT_READ, MAP_PRIVATE, memfd, aliged); \
  if (page == MAP_FAILED) { \
    perror("mmap " #reg); \
    return 1; \
  }

#define MAP_END munmap(page, size);

#endif /* BSEAV_LIB_SECURITY_SAGE_BP3_APP_BP3_SESSION_H_ */
