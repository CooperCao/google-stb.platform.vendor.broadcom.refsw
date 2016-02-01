/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef __CPRFRAMEWORK_H__
#define __CPRFRAMEWORK_H__

#include "../inc/tr69cdefs.h"

#define MAX_DEPTH   20      /* maximum nodes in parameter tree path */
extern const char instanceIDMASK[];
extern TRxObjNode  rootDevice[];


const char *getValTypeStr(eTRxType etype);
int isWriteable(TRxObjNode *n);
TRxObjNode  *findGWParameter(const char *pstr);
TRxObjNode  *getCurrentNode(void);
int         getCurrentInstanceID(void);
int         getInstanceCount( TRxObjNode *n);
int         getInstanceCountNoPathCheck( TRxObjNode *n);
InstanceDesc *getCurrentInstanceDesc(void);
InstanceDesc  *getHigherInstanceDesc(int level);
InstanceDesc *findInstanceDesc( TRxObjNode *n, int id);
InstanceDesc *findInstanceDescNoPathCheck( TRxObjNode *n, int id);
InstanceDesc *getNewInstanceDesc( TRxObjNode *n, InstanceDesc *parent, int id);
InstanceDope *findDopeInstance( TRxObjNode *n );
int deleteCurrentInstanceDesc(void);
int deleteInstanceDescNoPathCheck( TRxObjNode *n, int id);
int deleteInstanceDesc( TRxObjNode *n, int id);
int checkInstancePath(InstanceDesc *idp);
int checkInstanceStackPath(InstanceDesc *idp);

void pushNode(TRxObjNode *n);
void popNode(void);
void pushInstance(InstanceDesc *idp);
void popInstance(void);
void replaceInstanceTop(InstanceDesc *idp);
void clearInstanceStack(void);
void clearNodeStack(void);
void clearStacks(void);

char *buildInterfaceNameParamPath(char *ifname);
char *buildExternalConnParamName(int extIP);

TRX_STATUS getGWParameterValue( const char *pstr, char **value);

#endif /* __CPRFRAMEWORK_H__ */
