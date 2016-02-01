/***************************************************************************
 *    Copyright (c) 2004-2008, Broadcom Corporation
 *    All Rights Reserved
 *    Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *	 This header contains private data and functions for user data.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 * ***************************************************************************/
#ifndef BXVD_USERDATA_PRIV_H__
#define BXVD_USERDATA_PRIV_H__

#include "bstd.h"				 /* standard types */
#include "bavc.h"				 /* for userdata */
#include "bdbg.h"				 /* Dbglib */
#include "bxvd_vdec_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Circular queue */
#define BXVD_P_USERDATA_QUEUE_START 0
#define BXVD_P_USERDATA_QUEUE_MAX   32

#define BXVD_P_USERDATA_ITEM_SIZE   2048

/*
 * uiDecodePictureId added per Jira SW7425-1780
 */
typedef struct _QUEUE_MGR_
{
      int ulReadPtr;
      int ulWritePtr;
      int ulNextPtr;
      int ulQueueDepth;
      struct data
      {
            int           protocol;
            unsigned long ulFlags;
            unsigned long ulPulldown;
            unsigned long ulPTS;
            unsigned char *uUserData;
            uint32_t uiDecodePictureId;
      } *queue_data;
} QUEUE_MGR;

/* The internal structure for the User Data handle */
typedef struct BXVD_P_UserDataContext
{
      BXVD_ChannelHandle     hXvdCh;
      BINT_CallbackHandle    hCbUserdata;
      BXVD_Userdata_Settings sUserdataSettings;
      void                   *pBfr; /* user data buffer */
      bool                   bCallbackEnabled;
      /* Userdata callback registered */
      BINT_CallbackFunc	     fUserdataCallback_isr;
      void		     *pParm1;
      int 		     parm2;
      QUEUE_MGR              queue;
      BXVD_P_HandleType      eHandleType;
      BERR_Code              errForwardError;
} BXVD_P_UserDataContext;

/*
 * Called by DM when valid user data is available in the PPB.
 */

BERR_Code BXVD_P_Userdata_EnqueueDataPointer_isr(BXVD_ChannelHandle hXvdCh,
                                                 int           protocol,
                                                 unsigned long p_UserData,
                                                 unsigned long ulFlags,
                                                 unsigned long ulPulldown,
                                                 unsigned long ulPTS,
                                                 uint32_t uiDecodePictureId);

#ifdef __cplusplus
}
#endif

#endif /* BXVD_USERDATA_PRIV_H__ */
/* End of file. */



