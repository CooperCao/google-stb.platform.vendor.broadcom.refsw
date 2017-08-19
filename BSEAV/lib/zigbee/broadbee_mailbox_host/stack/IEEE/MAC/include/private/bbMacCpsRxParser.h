/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC CPS Frame Parser interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_CPS_RX_PARSER_H
#define _BB_MAC_CPS_RX_PARSER_H

/************************* INCLUDES ***********************************************************************************/
#include "bbMacBasics.h"

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Parses MAC frame Combined Header.
 * \return  Status of operation: TRUE if frame may be processed further; FALSE if it shall be rejected.
 * \details This function has three implicit parameters:
 *  - PRJ_TestDataFramePayloadIN    - payload containing PPDU of the received frame.
 *  - MCPS_RXFC__MPDU_surr          - MPDU Surrogate object to be assigned after the parsed frame.
 *  - MCPS_RXFC__MPDU_descr         - parsed frame MPDU Descriptor structure. It contains three payload descriptors to
 *      be assigned with payloads allocated for parsed frame Combined Header, Payload Fields, and MIC.
 *
 * \details If function successfully parsed the received frame Combined Header and allocated necessary dynamic memory
 *  for Combined Header, Payload Fields, and MIC, then TRUE is returned and MCPS_RXFC__MPDU_descr is assigned with
 *  allocated payload descriptors, MCPS_RXFC__MPDU_surr is assigned with parsed parameters. Otherwise FALSE is returned,
 *  all the preallocated memory is dismissed, and MCPS_RXFC__MPDU_descr is assigned with EMPTY_PAYLOAD identifiers.
 */
extern bool MCPS_RXPR__ParseHeader(void);

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Parses MAC Command frame Payload Fields.
 * \return  Status of operation: TRUE if frame may be processed further; FALSE if it shall be rejected.
 * \details This function has two implicit parameters:
 *  - MCPS_RXFC__MPDU_surr                  - MPDU Surrogate object to be assigned after the parsed Command frame.
 *  - MCPS_RXFC__MPDU_descr.payloadFields   - parsed frame MPDU Descriptor structure, Payload Fields. It contains
 *      payload descriptor assigned with Command Payload Fields to be parsed.
 *
 * \details If function successfully parsed the received Command frame Payload Fields, then TRUE is returned and
 *  MCPS_RXFC__MPDU_surr is assigned with parsed parameters.
 */
extern bool MCPS_RXPR__ParseCmdPayload(void);
#endif

#endif /* _BB_MAC_CPS_RX_PARSER_H */

/* eof bbMacCpsRxParser.h */