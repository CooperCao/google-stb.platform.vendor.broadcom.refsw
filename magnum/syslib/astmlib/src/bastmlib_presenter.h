/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bastmlib.h"

#ifndef BASTMLIB_PRESENTER_H__
#define BASTMLIB_PRESENTER_H__

void BASTMlib_Presenter_Flush_isr(
    BASTMlib_Presenter_Handle hPresenter
);

void BASTMlib_Presenter_Reset_isr(
	BASTMlib_Presenter_Handle hPresenter
);

void BASTMlib_Presenter_ResizeEventQueue(
	BASTMlib_Presenter_Handle hPresenter
);

#endif /* BASTMLIB_PRESENTER_H__ */

