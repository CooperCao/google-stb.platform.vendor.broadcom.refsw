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
******************************************************************************/
/*****************************************************************************
 *
 * FILENAME: $Workfile$
 *
 * DESCRIPTION: Profiling Engine Interface.
 *
 * $Revision$
 * $Date$
 *
 ****************************************************************************************/
#ifndef _SYS_DBG_PROFILING_ENGINE_H
#define _SYS_DBG_PROFILING_ENGINE_H

/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"
#include "bbSysPayload.h"

/************************* TYPES ********************************************************/
/**//**
 * \brief Reset request parameters type.
 */
typedef struct _SYS_DbgPeResetReqParams_t
{
    uint32_t resetType;
} SYS_DbgPeResetReqParams_t;

/**//**
 * \brief Reset request descriptor type.
 */
typedef struct _SYS_DbgPeResetReqDescr_t
{
    SYS_DbgPeResetReqParams_t params;
} SYS_DbgPeResetReqDescr_t;


/**//**
 * \brief GetData request parameters type.
 */
typedef struct _SYS_DbgPeGetDataReqParams_t
{
    uint32_t            offset;
} SYS_DbgPeGetDataReqParams_t;

/**//**
 * \brief GetData confirmation type.
 */
typedef struct _SYS_DbgPeGetDataConfParams_t
{
    uint32_t            offset;
    SYS_DataPointer_t   payload;
} SYS_DbgPeGetDataConfParams_t;

/**//**
 * \brief GetData request descriptor type.
 */
typedef struct _SYS_DbgPeGetDataReqDescr_t SYS_DbgPeGetDataReqDescr_t;
typedef struct _SYS_DbgPeGetDataReqDescr_t
{
    SYS_DbgPeGetDataReqParams_t params;
    void (*callback)(SYS_DbgPeGetDataReqDescr_t *, SYS_DbgPeGetDataConfParams_t *);
} SYS_DbgPeGetDataReqDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Request to reset array with Profiling data.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void SYS_DbgPeResetReq(SYS_DbgPeResetReqDescr_t *const reqDescr);

/************************************************************************************//**
  \brief Request to get Profiling data.
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void SYS_DbgPeGetDataReq(SYS_DbgPeGetDataReqDescr_t *const reqDescr);

#endif /* _SYS_DBG_PROFILING_ENGINE_H */