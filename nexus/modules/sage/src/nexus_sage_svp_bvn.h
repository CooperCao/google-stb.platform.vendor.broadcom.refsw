/***************************************************************************
*     (c)2015 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
***************************************************************************/
#ifndef NEXUS_SAGE_SVP_BVN_H__
#define NEXUS_SAGE_SVP_BVN_H__

#include "bstd.h"
#include "bavc.h"
/*#include "breg.h"*/


/***************************************************************************/
/* TODO: Rename if necessary to conform with SAGE code guideline and style.*/
/***************************************************************************/
#define BBVN_NUM_HDMI_PATH                (1)
#define BBVN_NUM_XCODE_PATH               (6)
#define BBVN_NUM_ANALOG_PATH              (3)

/***************************************************************************
Summary:
    This is the handle for BVN Monitor
Description:
Returns:
See Also:
****************************************************************************/
typedef struct BBVN_P_Monitor_Context      *BBVN_Monitor_Handle;

/***************************************************************************
Summary:
    This structure return the status from BVN Monitor.

Description:
Returns:
See Also:
***************************************************************************/
typedef struct
{
    bool                               bViolation;  /* [out] true if BVN routing violation. */

    struct {
        bool                           bSecure;     /* [out] true Hdmi output contains secure content */
        uint32_t                       ulHSize;     /* [out] BVN Width of this path */
        uint32_t                       ulVSize;     /* [out] BVN Height of this path */
    } Hdmi[BBVN_NUM_HDMI_PATH];

    struct {
        bool                           bSecure;     /* [out] true if Analog output contains secure content */
        uint32_t                       ulHSize;     /* [out] BVN Width of this path */
        uint32_t                       ulVSize;     /* [out] BVN Height of this path */
    } Analog[BBVN_NUM_ANALOG_PATH];

    struct {
        bool                           bSecure;     /* [out] true if transcode output contains secure content */
        uint32_t                       ulHSize;     /* [out] BVN Width of this path */
        uint32_t                       ulVSize;     /* [out] BVN Height of this path */
    } Xcode[BBVN_NUM_XCODE_PATH];

} BBVN_Monitor_Status;


/***************************************************************************
Summary:
    This function initializes the BVN monitor context.

Description:
Returns:
See Also:
****************************************************************************/
BERR_Code BBVN_Monitor_Init
    ( BBVN_Monitor_Handle             *phBvnMonitor,    /* [out] a reference to a BBVN_Handle. */
      BREG_Handle                      hReg             /* [in] to traverse vnet/vec routing, maybe opaque handle with hReg. */
    );

/***************************************************************************
Summary:
    This function uninitializes the BVN monitor context.
Description:
Returns:
See Also:
****************************************************************************/
BERR_Code BBVN_Monitor_Uninit
    ( BBVN_Monitor_Handle              hBvnMonitor     /* [out] BVN Monitor Handle becomes invalid. */
    );

/***************************************************************************
Summary:
   This is the beef of the BVN routing violation detection
Description:
   Monitor if there is BVN violation to be call periodically by SAGE.
Returns:
See Also:
    BBVN_Monitor_Init, BBVN_Monitor_Uninit.
****************************************************************************/
BERR_Code BBVN_Monitor_Check
    ( BBVN_Monitor_Handle              hBvnMonitor,     /* [in] A valid BVN Monitor Handle created earlier. */
      const BAVC_CoreList             *pSecureCores,    /* [in] list of secure cores */
      BBVN_Monitor_Status             *pStatus          /* [out] BVN status */
    );

#endif /* NEXUS_SAGE_SVP_BVN_H__ */
/* End of file */
