/*************************************************************************** *     
 * Copyright (c) 2005-2009, Broadcom Corporation *     All Rights Reserved
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
 *
 ***************************************************************************/
#ifndef _BVSB_3548_IFDCOEF_H_
#define _BVSB_3548_IFDCOEF_H_

extern uint16_t ifd_coef1[];
extern uint16_t ifd_coef1_LPF_L_Pos_Mod[];
extern uint16_t ifd_coef1_AllPass[];
extern uint16_t ifd_coef2[];
extern uint16_t ifd_coef3[];
extern uint16_t ifd_coef3_GDF_Byp_NTSC[];
extern uint16_t ifd_coef3_GDF_Byp_PAL_D[];
extern uint16_t ifd_coef3_GDF_PAL_B_CurveA[];
extern uint16_t ifd_coef3_GDF_PAL_B_CurveB[];
extern uint16_t ifd_coef4[];
extern uint16_t ifd_coef4_ATF_PAL_D[];
extern uint16_t ifd_coef6[];
extern uint16_t ifd_coef7[];
extern uint16_t ifd_coef7_BTSC_LPFB_Wide[];
extern uint16_t ifd_coef7_BTSC_LPFB_Medium[];
extern uint16_t ifd_coef8[];
extern uint16_t ifd_coef6_alt[];
extern uint16_t ifd_coef7_alt[];
extern uint16_t ifd_coef7_A2_LPFB_Wide[];
extern uint16_t ifd_coef8_alt[];
extern uint16_t ifd_coef9[];
extern uint16_t ifd_coef10[];
extern uint16_t ifd_coef10_NICAM_Nyq_Type0[];
extern uint16_t ifd_coef10_NICAM_Nyq_Type1[];
extern uint16_t ifd_coef11[];

#define COEF1_LEN 39
#define COEF1_START 0x8800

#define COEF2_LEN 26
#define COEF2_START 0x9000

#define COEF3_LEN 40
#define COEF3_START 0x9800

#define COEF4_LEN 64
#define COEF4_START 0xA000

#define COEF6_LEN 47
#define COEF6_START 0xB000

#define COEF7_LEN 47
#define COEF7_START 0xB800

#define COEF8_LEN 47
#define COEF8_START 0xC000

#define COEF9_LEN 47
#define COEF9_START 0xC800

#define COEF10_LEN 47
#define COEF10_START 0xD000

#define COEF11_LEN 47
#define COEF11_START 0xD800

#endif /* _BVSB_3548_IFDCOEF_H_ */
