/***************************************************************************
 *     Copyright (c) 2004-2012, Broadcom Corporation
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
 * Module Description:
 *   
 *  Certain elements of the PPB are being exposed by the PI. The PPB elements
 *  declared in this public PPB mirror the elements in the private PPB. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXVD_PPB_H__
#define BXVD_PPB_H__

#if BXVD_P_AVD_ARC600
#define BXVD_PPB_EXTENDED 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t      crc_luma;            /* frame or top field of LUMA CRC */
    uint32_t      crc_cb;              /* frame or top field of CHROMA CB CRC */
    uint32_t      crc_cr;              /* frame or top field of CHROMA CR CRC */
    uint32_t      crc_luma_bot;        /* bot field of LUMA CRC */
    uint32_t      crc_cb_bot;          /* bot field of CHROMA CB CRC */
    uint32_t      crc_cr_bot;          /* bot field of CHROMA CR CRC */    
} BXVD_PPB_CRC;

typedef struct
{
  uint32_t       hidden[73];
  BXVD_PPB_CRC   crc;
  uint32_t       hidden_1[49];    
} BXVD_PPB;

#ifdef __cplusplus
}
#endif

#endif /* BXVD_PPB_H__ */
/* End of file. */
