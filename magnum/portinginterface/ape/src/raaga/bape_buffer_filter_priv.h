/***************************************************************************
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
*
* API Description:
*
***************************************************************************/
#ifndef BAPE_BUFFER_FILTER_PRIV_H_
#define BAPE_BUFFER_FILTER_PRIV_H_

#include "bape_priv.h"
#include "bape_buffer.h"

typedef struct BAPE_BufferGroupFilter *BAPE_BufferGroupFilterHandle;

typedef void BAPE_BufferGroupFilterFn(void *ctx, BAPE_BufferDescriptor *pDesc,
                                      uint8_t *pIn, unsigned inCount,
                                      uint8_t *pInWrap, unsigned inWrapCount,
                                      uint8_t *pOut, unsigned outCount,
                                      uint8_t *pOutWrap, unsigned outWrapCount,
                                      unsigned *inSize, unsigned *outSize);

BAPE_BufferGroupFilterHandle BAPE_BufferGroupFilter_P_Open(BAPE_Handle hDevice, BAPE_BufferGroupHandle hUpstreamBG, unsigned bufferSize, BAPE_BufferGroupFilterFn *pFunc, void *ctx);
void BAPE_BufferGroupFilter_P_Close(BAPE_BufferGroupFilterHandle hFilter);

BERR_Code BAPE_BufferGroupFilter_P_Enable_isr(BAPE_BufferGroupFilterHandle hFilter, bool enable);
BERR_Code BAPE_BufferGroupFilter_P_Read_isr(BAPE_BufferGroupFilterHandle hFilter, BAPE_BufferDescriptor *pGroupDescriptor);
BERR_Code BAPE_BufferGroupFilter_P_ReadComplete_isr(BAPE_BufferGroupFilterHandle hFilter, size_t size);
#endif /* #ifndef BAPE_BUFFER_FILTER_PRIV_H_ */
