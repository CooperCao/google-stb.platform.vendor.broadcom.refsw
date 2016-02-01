/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

BVCE_STATUS_FIELD(uiTotalErrors)
BVCE_STATUS_FIELD(uiTotalPicturesReceived)
BVCE_STATUS_FIELD(uiTotalPicturesDroppedDueToFrameRateConversion)
BVCE_STATUS_FIELD(uiTotalPicturesDroppedDueToErrors)
BVCE_STATUS_FIELD(uiTotalPicturesDroppedDueToHRDUnderflow)
BVCE_STATUS_FIELD(uiTotalPicturesEncoded)
BVCE_STATUS_FIELD(uiLastPictureIdEncoded)
BVCE_STATUS_FIELD(uiSTCSnapshot)
BVCE_STATUS_FIELD(uiEtsDtsOffset)
BVCE_STATUS_FIELD(uiAverageFramesPerSecond)
#undef BVCE_STATUS_FIELD
