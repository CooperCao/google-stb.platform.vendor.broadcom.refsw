/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g2_priv.h"

BDBG_MODULE(bast_g2_priv_sds);

/* #define BAST_DEBUG_DFT_FREQ_ESTIMATE */


/* local routines */
BERR_Code BAST_g2_P_InitChannels1_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_InitChannels2_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_BlindAcquire1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate2_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate3_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate4_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate5_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate6_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate7_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate8_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimate9_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DoDftFreqEstimateExit_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DftStartAndWaitForDone_isr(BAST_ChannelHandle h, BAST_g2_FUNCT funct);
BERR_Code BAST_g2_P_DftWaitForDone_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_DftGetPower_isr(BAST_ChannelHandle h);

#define BAST_SNRE_TABLE_N (128)
#define BAST_SNR_MAX (9344)  /* 36.5 (in units of 2^-8 dB) */
#define BAST_snr_quant (73)  /* BAST_SNR_MAX / BAST_SNRE_TABLE_N */

#define BAST_snr_legacy_qpsk_scale (247878528) /* 3782.326172 * 2^16 */
#define BAST_snr_legacy_qpsk_min   (0x0061C97E)
#define BAST_snr_legacy_qpsk_max   (0x0F281D00)

#define BAST_snr_ldpc_8psk_scale (126049536)   /* 1923.363281 * 2^16 */
#define BAST_snr_ldpc_8psk_min   (0x0001D56E)
#define BAST_snr_ldpc_8psk_max   (0x07853270)
#define BAST_snr_ldpc_qpsk_scale (90691248)    /* 1383.838623 * 2^16 */
#define BAST_snr_ldpc_qpsk_min   (0x0000EBF0)
#define BAST_snr_ldpc_qpsk_max   (0x0568C2A0)

#define BAST_snr_turbo_8psk_scale (63024768)   /* 961.6816405 * 2^16 */
#define BAST_snr_turbo_8psk_min   (0x0000EAB7)
#define BAST_snr_turbo_8psk_max   (0x03C29938)

static const uint16_t BAST_snr_legacy_qpsk_table[128] = {
   0x0000, /* 0: snre=0x0061C97D, snr=36.500000 dB */
   0x0038, /* 1: snre=0x00650CC7, snr=36.214844 dB */
   0x0072, /* 2: snre=0x00686BEC, snr=35.929688 dB */
   0x00AF, /* 3: snre=0x006BE7DA, snr=35.644531 dB */
   0x00ED, /* 4: snre=0x006F8187, snr=35.359375 dB */
   0x012E, /* 5: snre=0x007339EF, snr=35.074219 dB */
   0x0170, /* 6: snre=0x00771218, snr=34.789063 dB */
   0x01B5, /* 7: snre=0x007B0B12, snr=34.503906 dB */
   0x01FC, /* 8: snre=0x007F25F3, snr=34.218750 dB */
   0x0246, /* 9: snre=0x008363DC, snr=33.933594 dB */
   0x0292, /* 10: snre=0x0087C5F8, snr=33.648438 dB */
   0x02E0, /* 11: snre=0x008C4D7B, snr=33.363281 dB */
   0x0331, /* 12: snre=0x0090FBA2, snr=33.078125 dB */
   0x0385, /* 13: snre=0x0095D1B7, snr=32.792969 dB */
   0x03DC, /* 14: snre=0x009AD10D, snr=32.507813 dB */
   0x0435, /* 15: snre=0x009FFB03, snr=32.222656 dB */
   0x0492, /* 16: snre=0x00A55103, snr=31.937500 dB */
   0x04F1, /* 17: snre=0x00AAD483, snr=31.652344 dB */
   0x0554, /* 18: snre=0x00B08706, snr=31.367188 dB */
   0x05BA, /* 19: snre=0x00B66A1B, snr=31.082031 dB */
   0x0623, /* 20: snre=0x00BC7F5D, snr=30.796875 dB */
   0x0690, /* 21: snre=0x00C2C876, snr=30.511719 dB */
   0x0701, /* 22: snre=0x00C9471F, snr=30.226563 dB */
   0x0775, /* 23: snre=0x00CFFD1B, snr=29.941406 dB */
   0x07ED, /* 24: snre=0x00D6EC41, snr=29.656250 dB */
   0x0869, /* 25: snre=0x00DE1674, snr=29.371094 dB */
   0x08EA, /* 26: snre=0x00E57DA6, snr=29.085938 dB */
   0x096E, /* 27: snre=0x00ED23DC, snr=28.800781 dB */
   0x09F7, /* 28: snre=0x00F50B2A, snr=28.515625 dB */
   0x0A84, /* 29: snre=0x00FD35B4, snr=28.230469 dB */
   0x0B17, /* 30: snre=0x0105A5B2, snr=27.945313 dB */
   0x0BAE, /* 31: snre=0x010E5D6D, snr=27.660156 dB */
   0x0C4A, /* 32: snre=0x01175F3F, snr=27.375000 dB */
   0x0CEB, /* 33: snre=0x0120AD99, snr=27.089844 dB */
   0x0D92, /* 34: snre=0x012A4AFC, snr=26.804688 dB */
   0x0E3E, /* 35: snre=0x01343A00, snr=26.519531 dB */
   0x0EF0, /* 36: snre=0x013E7D4F, snr=26.234375 dB */
   0x0FA7, /* 37: snre=0x014917AB, snr=25.949219 dB */
   0x1065, /* 38: snre=0x01540BEC, snr=25.664063 dB */
   0x1129, /* 39: snre=0x015F5CFD, snr=25.378906 dB */
   0x11F4, /* 40: snre=0x016B0DE5, snr=25.093750 dB */
   0x12C5, /* 41: snre=0x017721BF, snr=24.808594 dB */
   0x139D, /* 42: snre=0x01839BC1, snr=24.523438 dB */
   0x147D, /* 43: snre=0x01907F36, snr=24.238281 dB */
   0x1563, /* 44: snre=0x019DCF88, snr=23.953125 dB */
   0x1652, /* 45: snre=0x01AB9035, snr=23.667969 dB */
   0x1748, /* 46: snre=0x01B9C4DB, snr=23.382813 dB */
   0x1846, /* 47: snre=0x01C8712E, snr=23.097656 dB */
   0x194C, /* 48: snre=0x01D79902, snr=22.812500 dB */
   0x1A5C, /* 49: snre=0x01E74045, snr=22.527344 dB */
   0x1B74, /* 50: snre=0x01F76B00, snr=22.242188 dB */
   0x1C95, /* 51: snre=0x02081D5D, snr=21.957031 dB */
   0x1DC0, /* 52: snre=0x02195BA1, snr=21.671875 dB */
   0x1EF4, /* 53: snre=0x022B2A2E, snr=21.386719 dB */
   0x2033, /* 54: snre=0x023D8D86, snr=21.101563 dB */
   0x217C, /* 55: snre=0x02508A48, snr=20.816406 dB */
   0x22D0, /* 56: snre=0x02642534, snr=20.531250 dB */
   0x242E, /* 57: snre=0x02786326, snr=20.246094 dB */
   0x2599, /* 58: snre=0x028D4919, snr=19.960938 dB */
   0x270E, /* 59: snre=0x02A2DC2A, snr=19.675781 dB */
   0x2890, /* 60: snre=0x02B92192, snr=19.390625 dB */
   0x2A1F, /* 61: snre=0x02D01EA8, snr=19.105469 dB */
   0x2BBA, /* 62: snre=0x02E7D8E3, snr=18.820313 dB */
   0x2D62, /* 63: snre=0x030055D6, snr=18.535156 dB */
   0x2F18, /* 64: snre=0x03199B32, snr=18.250000 dB */
   0x30DC, /* 65: snre=0x0333AEC3, snr=17.964844 dB */
   0x32AE, /* 66: snre=0x034E9671, snr=17.679688 dB */
   0x348F, /* 67: snre=0x036A583D, snr=17.394531 dB */
   0x367F, /* 68: snre=0x0386FA41, snr=17.109375 dB */
   0x387F, /* 69: snre=0x03A482AD, snr=16.824219 dB */
   0x3A8E, /* 70: snre=0x03C2F7C5, snr=16.539063 dB */
   0x3CAF, /* 71: snre=0x03E25FDF, snr=16.253906 dB */
   0x3EE0, /* 72: snre=0x0402C163, snr=15.968750 dB */
   0x4122, /* 73: snre=0x042422C1, snr=15.683594 dB */
   0x4376, /* 74: snre=0x04468A76, snr=15.398438 dB */
   0x45DC, /* 75: snre=0x0469FF03, snr=15.113281 dB */
   0x4855, /* 76: snre=0x048E86E7, snr=14.828125 dB */
   0x4AE1, /* 77: snre=0x04B428A0, snr=14.542969 dB */
   0x4D81, /* 78: snre=0x04DAEAA2, snr=14.257813 dB */
   0x5035, /* 79: snre=0x0502D352, snr=13.972656 dB */
   0x52FC, /* 80: snre=0x052BE8FD, snr=13.687500 dB */
   0x55D9, /* 81: snre=0x055631D5, snr=13.402344 dB */
   0x58CB, /* 82: snre=0x0581B3E7, snr=13.117188 dB */
   0x5BD2, /* 83: snre=0x05AE7511, snr=12.832031 dB */
   0x5EF0, /* 84: snre=0x05DC7AFD, snr=12.546875 dB */
   0x6224, /* 85: snre=0x060BCB11, snr=12.261719 dB */
   0x656E, /* 86: snre=0x063C6A6C, snr=11.976563 dB */
   0x68D0, /* 87: snre=0x066E5DD2, snr=11.691406 dB */
   0x6C48, /* 88: snre=0x06A1A9A5, snr=11.406250 dB */
   0x6FD9, /* 89: snre=0x06D651D7, snr=11.121094 dB */
   0x7381, /* 90: snre=0x070C59DB, snr=10.835938 dB */
   0x7741, /* 91: snre=0x0743C497, snr=10.550781 dB */
   0x7B1A, /* 92: snre=0x077C9454, snr=10.265625 dB */
   0x7EA0, /* 93: snre=0x07B0AE18, snr=9.980469 dB */
   0x82D8, /* 94: snre=0x07EEFE62, snr=9.695313 dB */
   0x872F, /* 95: snre=0x082F1C31, snr=9.410156 dB */
   0x8BA3, /* 96: snre=0x0870E749, snr=9.125000 dB */
   0x9031, /* 97: snre=0x08B43347, snr=8.839844 dB */
   0x94D5, /* 98: snre=0x08F8C767, snr=8.554688 dB */
   0x998B, /* 99: snre=0x093E5F27, snr=8.269531 dB */
   0x9E4D, /* 100: snre=0x0984AC02, snr=7.984375 dB */
   0xA316, /* 101: snre=0x09CB581F, snr=7.699219 dB */
   0xA7DF, /* 102: snre=0x0A1209D9, snr=7.414063 dB */
   0xACA2, /* 103: snre=0x0A5867AB, snr=7.128906 dB */
   0xB15A, /* 104: snre=0x0A9E1C10, snr=6.843750 dB */
   0xB601, /* 105: snre=0x0AE2D8EA, snr=6.558594 dB */
   0xBA92, /* 106: snre=0x0B265A0B, snr=6.273438 dB */
   0xBF0B, /* 107: snre=0x0B6866C0, snr=5.988281 dB */
   0xC367, /* 108: snre=0x0BA8D25C, snr=5.703125 dB */
   0xC7A5, /* 109: snre=0x0BE77BEB, snr=5.417969 dB */
   0xCBC2, /* 110: snre=0x0C244D47, snr=5.132813 dB */
   0xCFBF, /* 111: snre=0x0C5F39C5, snr=4.847656 dB */
   0xD39B, /* 112: snre=0x0C983CB7, snr=4.562500 dB */
   0xD756, /* 113: snre=0x0CCF57F2, snr=4.277344 dB */
   0xDAF0, /* 114: snre=0x0D04925D, snr=3.992188 dB */
   0xDE6B, /* 115: snre=0x0D37F6B9, snr=3.707031 dB */
   0xE1C6, /* 116: snre=0x0D69928F, snr=3.421875 dB */
   0xE504, /* 117: snre=0x0D99754D, snr=3.136719 dB */
   0xE825, /* 118: snre=0x0DC7AF95, snr=2.851563 dB */
   0xEB2A, /* 119: snre=0x0DF452AE, snr=2.566406 dB */
   0xEE15, /* 120: snre=0x0E1F701A, snr=2.281250 dB */
   0xF0E7, /* 121: snre=0x0E491943, snr=1.996094 dB */
   0xF3A1, /* 122: snre=0x0E715F40, snr=1.710938 dB */
   0xF644, /* 123: snre=0x0E9852AE, snr=1.425781 dB */
   0xF8D1, /* 124: snre=0x0EBE0390, snr=1.140625 dB */
   0xFB49, /* 125: snre=0x0EE28142, snr=0.855469 dB */
   0xFDAE, /* 126: snre=0x0F05DA6A, snr=0.570313 dB */
   0xFFFF  /* 127: snre=0x0F281CF9, snr=0.285156 dB */
};


static const uint16_t BAST_snr_ldpc_qpsk_table[128] = {
   0x0000, /* 0: snre=0x0000EBF0, snr=36.500000 dB */
   0x0002, /* 1: snre=0x0000FAAD, snr=36.214844 dB */
   0x0005, /* 2: snre=0x00010BAF, snr=35.929688 dB */
   0x0009, /* 3: snre=0x00011DDA, snr=35.644531 dB */
   0x000C, /* 4: snre=0x00013140, snr=35.359375 dB */
   0x0010, /* 5: snre=0x000145F7, snr=35.074219 dB */
   0x0014, /* 6: snre=0x00015C16, snr=34.789063 dB */
   0x0019, /* 7: snre=0x000173B5, snr=34.503906 dB */
   0x001D, /* 8: snre=0x00018CEE, snr=34.218750 dB */
   0x0022, /* 9: snre=0x0001A7DE, snr=33.933594 dB */
   0x0028, /* 10: snre=0x0001C4A1, snr=33.648438 dB */
   0x002D, /* 11: snre=0x0001E358, snr=33.363281 dB */
   0x0033, /* 12: snre=0x00020425, snr=33.078125 dB */
   0x003A, /* 13: snre=0x0002272B, snr=32.792969 dB */
   0x0041, /* 14: snre=0x00024C92, snr=32.507813 dB */
   0x0048, /* 15: snre=0x00027483, snr=32.222656 dB */
   0x0050, /* 16: snre=0x00029F29, snr=31.937500 dB */
   0x0058, /* 17: snre=0x0002CCB4, snr=31.652344 dB */
   0x0061, /* 18: snre=0x0002FD56, snr=31.367188 dB */
   0x006B, /* 19: snre=0x00033144, snr=31.082031 dB */
   0x0075, /* 20: snre=0x000368B9, snr=30.796875 dB */
   0x0080, /* 21: snre=0x0003A3F1, snr=30.511719 dB */
   0x008C, /* 22: snre=0x0003E32D, snr=30.226563 dB */
   0x0098, /* 23: snre=0x000426B4, snr=29.941406 dB */
   0x00A6, /* 24: snre=0x00046ECF, snr=29.656250 dB */
   0x00B4, /* 25: snre=0x0004BBCE, snr=29.371094 dB */
   0x00C3, /* 26: snre=0x00050E06, snr=29.085938 dB */
   0x00D3, /* 27: snre=0x000565D1, snr=28.800781 dB */
   0x00E5, /* 28: snre=0x0005C391, snr=28.515625 dB */
   0x00F7, /* 29: snre=0x000627AD, snr=28.230469 dB */
   0x010B, /* 30: snre=0x00069293, snr=27.945313 dB */
   0x0120, /* 31: snre=0x000704B8, snr=27.660156 dB */
   0x0137, /* 32: snre=0x00077E9A, snr=27.375000 dB */
   0x014F, /* 33: snre=0x000800C0, snr=27.089844 dB */
   0x0169, /* 34: snre=0x00088BB8, snr=26.804688 dB */
   0x0184, /* 35: snre=0x0009201B, snr=26.519531 dB */
   0x01A1, /* 36: snre=0x0009BE8D, snr=26.234375 dB */
   0x01C1, /* 37: snre=0x000A67BC, snr=25.949219 dB */
   0x01E2, /* 38: snre=0x000B1C61, snr=25.664063 dB */
   0x0206, /* 39: snre=0x000BDD42, snr=25.378906 dB */
   0x022C, /* 40: snre=0x000CAB34, snr=25.093750 dB */
   0x0255, /* 41: snre=0x000D8718, snr=24.808594 dB */
   0x0280, /* 42: snre=0x000E71DF, snr=24.523438 dB */
   0x02AE, /* 43: snre=0x000F6C8A, snr=24.238281 dB */
   0x02E0, /* 44: snre=0x0010782B, snr=23.953125 dB */
   0x0315, /* 45: snre=0x001195E7, snr=23.667969 dB */
   0x034D, /* 46: snre=0x0012C6F4, snr=23.382813 dB */
   0x0389, /* 47: snre=0x00140CA1, snr=23.097656 dB */
   0x03CA, /* 48: snre=0x00156850, snr=22.812500 dB */
   0x040E, /* 49: snre=0x0016DB79, snr=22.527344 dB */
   0x0458, /* 50: snre=0x001867B2, snr=22.242188 dB */
   0x04A6, /* 51: snre=0x001A0EA6, snr=21.957031 dB */
   0x04F9, /* 52: snre=0x001BD21F, snr=21.671875 dB */
   0x0553, /* 53: snre=0x001DB405, snr=21.386719 dB */
   0x05B2, /* 54: snre=0x001FB65E, snr=21.101563 dB */
   0x0617, /* 55: snre=0x0021DB53, snr=20.816406 dB */
   0x0684, /* 56: snre=0x00242530, snr=20.531250 dB */
   0x06F7, /* 57: snre=0x00269666, snr=20.246094 dB */
   0x0773, /* 58: snre=0x0029318E, snr=19.960938 dB */
   0x07F6, /* 59: snre=0x002BF96C, snr=19.675781 dB */
   0x0883, /* 60: snre=0x002EF0EF, snr=19.390625 dB */
   0x0919, /* 61: snre=0x00321B34, snr=19.105469 dB */
   0x09B9, /* 62: snre=0x00357B8A, snr=18.820313 dB */
   0x0A63, /* 63: snre=0x00391575, snr=18.535156 dB */
   0x0B19, /* 64: snre=0x003CECAB, snr=18.250000 dB */
   0x0BDB, /* 65: snre=0x0041051D, snr=17.964844 dB */
   0x0CAA, /* 66: snre=0x004562F5, snr=17.679688 dB */
   0x0D86, /* 67: snre=0x004A0A9D, snr=17.394531 dB */
   0x0E71, /* 68: snre=0x004F00BB, snr=17.109375 dB */
   0x0F6C, /* 69: snre=0x00544A38, snr=16.824219 dB */
   0x1076, /* 70: snre=0x0059EC3E, snr=16.539063 dB */
   0x1193, /* 71: snre=0x005FEC3F, snr=16.253906 dB */
   0x12C1, /* 72: snre=0x00664FF0, snr=15.968750 dB */
   0x1403, /* 73: snre=0x006D1D4D, snr=15.683594 dB */
   0x155A, /* 74: snre=0x00745A99, snr=15.398438 dB */
   0x16C7, /* 75: snre=0x007C0E5D, snr=15.113281 dB */
   0x184B, /* 76: snre=0x00843F66, snr=14.828125 dB */
   0x19E7, /* 77: snre=0x008CF4C3, snr=14.542969 dB */
   0x1B9E, /* 78: snre=0x009635C3, snr=14.257813 dB */
   0x1D6F, /* 79: snre=0x00A009EB, snr=13.972656 dB */
   0x1F5D, /* 80: snre=0x00AA78F6, snr=13.687500 dB */
   0x2169, /* 81: snre=0x00B58AC8, snr=13.402344 dB */
   0x2395, /* 82: snre=0x00C14764, snr=13.117188 dB */
   0x25E2, /* 83: snre=0x00CDB6E0, snr=12.832031 dB */
   0x2852, /* 84: snre=0x00DAE155, snr=12.546875 dB */
   0x2AE5, /* 85: snre=0x00E8CECD, snr=12.261719 dB */
   0x2D9E, /* 86: snre=0x00F7872D, snr=11.976563 dB */
   0x307E, /* 87: snre=0x0107121C, snr=11.691406 dB */
   0x3387, /* 88: snre=0x011776E7, snr=11.406250 dB */
   0x36B9, /* 89: snre=0x0128BC5D, snr=11.121094 dB */
   0x3A15, /* 90: snre=0x013AE8B4, snr=10.835938 dB */
   0x3D9E, /* 91: snre=0x014E0157, snr=10.550781 dB */
   0x4153, /* 92: snre=0x01620AC3, snr=10.265625 dB */
   0x4535, /* 93: snre=0x01770857, snr=9.980469 dB */
   0x4944, /* 94: snre=0x018CFC22, snr=9.695313 dB */
   0x4D82, /* 95: snre=0x01A3E6B8, snr=9.410156 dB */
   0x51EC, /* 96: snre=0x01BBC6FD, snr=9.125000 dB */
   0x5684, /* 97: snre=0x01D499FC, snr=8.839844 dB */
   0x5B48, /* 98: snre=0x01EE5AB9, snr=8.554688 dB */
   0x6036, /* 99: snre=0x02090215, snr=8.269531 dB */
   0x654D, /* 100: snre=0x022486B4, snr=7.984375 dB */
   0x6A8B, /* 101: snre=0x0240DCF1, snr=7.699219 dB */
   0x6FED, /* 102: snre=0x025DF6ED, snr=7.414063 dB */
   0x7571, /* 103: snre=0x027BC4A4, snr=7.128906 dB */
   0x7B12, /* 104: snre=0x029A341E, snr=6.843750 dB */
   0x80CE, /* 105: snre=0x02B931BE, snr=6.558594 dB */
   0x86A0, /* 106: snre=0x02D8A89C, snr=6.273438 dB */
   0x8C84, /* 107: snre=0x02F882F5, snr=5.988281 dB */
   0x9277, /* 108: snre=0x0318AAB1, snr=5.703125 dB */
   0x9874, /* 109: snre=0x033909E5, snr=5.417969 dB */
   0x9E78, /* 110: snre=0x03598B5D, snr=5.132813 dB */
   0xA47E, /* 111: snre=0x037A1B18, snr=4.847656 dB */
   0xAA83, /* 112: snre=0x039AA6B1, snr=4.562500 dB */
   0xB084, /* 113: snre=0x03BB1DBB, snr=4.277344 dB */
   0xB2D8, /* 114: snre=0x03DB71F4, snr=3.992188 dB */ /* low SNR is adjusted at this point and below */
   0xB81C, /* 115: snre=0x03FB9769, snr=3.707031 dB */
   0xBD4C, /* 116: snre=0x041B8478, snr=3.421875 dB */
   0xC267, /* 117: snre=0x043B31BE, snr=3.136719 dB */
   0xC76D, /* 118: snre=0x045A99F8, snr=2.851563 dB */
   0xCC5D, /* 119: snre=0x0479B9CB, snr=2.566406 dB */
   0xD138, /* 120: snre=0x04988F94, snr=2.281250 dB */
   0xD5FB, /* 121: snre=0x04B71B25, snr=1.996094 dB */
   0xDAAA, /* 122: snre=0x04D55D92, snr=1.710938 dB */
   0xDF44, /* 123: snre=0x04F358EF, snr=1.425781 dB */
   0xE3C8, /* 124: snre=0x05111027, snr=1.140625 dB */
   0xE83A, /* 125: snre=0x052E86C4, snr=0.855469 dB */
   0xEC98, /* 126: snre=0x054BC0CC, snr=0.570313 dB */
   0xFFFF  /* 127: snre=0x0568C29E, snr=0.285156 dB */
};


static const uint16_t BAST_snr_ldpc_8psk_table[128] = {
   0x0000, /* 0: snre=0x0001D56E, snr=36.500000 dB */
   0x0004, /* 1: snre=0x0001F548, snr=36.214844 dB */
   0x0008, /* 2: snre=0x0002174A, snr=35.929688 dB */
   0x000D, /* 3: snre=0x00023B9C, snr=35.644531 dB */
   0x0012, /* 4: snre=0x00026264, snr=35.359375 dB */
   0x0018, /* 5: snre=0x00028BCE, snr=35.074219 dB */
   0x001E, /* 6: snre=0x0002B806, snr=34.789063 dB */
   0x0024, /* 7: snre=0x0002E73F, snr=34.503906 dB */
   0x002B, /* 8: snre=0x000319AB, snr=34.218750 dB */
   0x0032, /* 9: snre=0x00034F82, snr=33.933594 dB */
   0x0039, /* 10: snre=0x00038900, snr=33.648438 dB */
   0x0042, /* 11: snre=0x0003C664, snr=33.363281 dB */
   0x004A, /* 12: snre=0x000407F2, snr=33.078125 dB */
   0x0054, /* 13: snre=0x00044DF1, snr=32.792969 dB */
   0x005E, /* 14: snre=0x000498AF, snr=32.507813 dB */
   0x0068, /* 15: snre=0x0004E87D, snr=32.222656 dB */
   0x0074, /* 16: snre=0x00053DB4, snr=31.937500 dB */
   0x0080, /* 17: snre=0x000598B2, snr=31.652344 dB */
   0x008D, /* 18: snre=0x0005F9D9, snr=31.367188 dB */
   0x009A, /* 19: snre=0x00066196, snr=31.082031 dB */
   0x00A9, /* 20: snre=0x0006D05A, snr=30.796875 dB */
   0x00B9, /* 21: snre=0x0007469D, snr=30.511719 dB */
   0x00CA, /* 22: snre=0x0007C4E3, snr=30.226563 dB */
   0x00DC, /* 23: snre=0x00084BB5, snr=29.941406 dB */
   0x00EF, /* 24: snre=0x0008DBA8, snr=29.656250 dB */
   0x0103, /* 25: snre=0x00097558, snr=29.371094 dB */
   0x0119, /* 26: snre=0x000A196E, snr=29.085938 dB */
   0x0130, /* 27: snre=0x000AC89D, snr=28.800781 dB */
   0x0149, /* 28: snre=0x000B83A5, snr=28.515625 dB */
   0x0164, /* 29: snre=0x000C4B51, snr=28.230469 dB */
   0x0180, /* 30: snre=0x000D207B, snr=27.945313 dB */
   0x019F, /* 31: snre=0x000E040C, snr=27.660156 dB */
   0x01BF, /* 32: snre=0x000EF6FA, snr=27.375000 dB */
   0x01E1, /* 33: snre=0x000FFA4E, snr=27.089844 dB */
   0x0206, /* 34: snre=0x00110F21, snr=26.804688 dB */
   0x022E, /* 35: snre=0x0012369E, snr=26.519531 dB */
   0x0258, /* 36: snre=0x00137205, snr=26.234375 dB */
   0x0284, /* 37: snre=0x0014C2AB, snr=25.949219 dB */
   0x02B4, /* 38: snre=0x001629F9, snr=25.664063 dB */
   0x02E7, /* 39: snre=0x0017A973, snr=25.378906 dB */
   0x031E, /* 40: snre=0x001942B3, snr=25.093750 dB */
   0x0358, /* 41: snre=0x001AF76F, snr=24.808594 dB */
   0x0396, /* 42: snre=0x001CC979, snr=24.523438 dB */
   0x03D8, /* 43: snre=0x001EBAC1, snr=24.238281 dB */
   0x041F, /* 44: snre=0x0020CD55, snr=23.953125 dB */
   0x046A, /* 45: snre=0x00230367, snr=23.667969 dB */
   0x04BA, /* 46: snre=0x00255F4A, snr=23.382813 dB */
   0x0510, /* 47: snre=0x0027E378, snr=23.097656 dB */
   0x056C, /* 48: snre=0x002A9290, snr=22.812500 dB */
   0x05CD, /* 49: snre=0x002D6F5C, snr=22.527344 dB */
   0x0635, /* 50: snre=0x00307CD2, snr=22.242188 dB */
   0x06A4, /* 51: snre=0x0033BE13, snr=21.957031 dB */
   0x071A, /* 52: snre=0x00373673, snr=21.671875 dB */
   0x0798, /* 53: snre=0x003AE976, snr=21.386719 dB */
   0x081F, /* 54: snre=0x003EDAD5, snr=21.101563 dB */
   0x08AE, /* 55: snre=0x00430E80, snr=20.816406 dB */
   0x0946, /* 56: snre=0x004788A0, snr=20.531250 dB */
   0x09E9, /* 57: snre=0x004C4D98, snr=20.246094 dB */
   0x0A96, /* 58: snre=0x00516209, snr=19.960938 dB */
   0x0B4E, /* 59: snre=0x0056CAD2, snr=19.675781 dB */
   0x0C13, /* 60: snre=0x005C8D14, snr=19.390625 dB */
   0x0CE3, /* 61: snre=0x0062AE30, snr=19.105469 dB */
   0x0DC2, /* 62: snre=0x006933CB, snr=18.820313 dB */
   0x0EAE, /* 63: snre=0x007023CF, snr=18.535156 dB */
   0x0FA9, /* 64: snre=0x0077846B, snr=18.250000 dB */
   0x10B5, /* 65: snre=0x007F5C0F, snr=17.964844 dB */
   0x11D1, /* 66: snre=0x0087B170, snr=17.679688 dB */
   0x12FE, /* 67: snre=0x00908B87, snr=17.394531 dB */
   0x143E, /* 68: snre=0x0099F188, snr=17.109375 dB */
   0x1592, /* 69: snre=0x00A3EAE7, snr=16.824219 dB */
   0x16FB, /* 70: snre=0x00AE7F4D, snr=16.539063 dB */
   0x1879, /* 71: snre=0x00B9B695, snr=16.253906 dB */
   0x1A0E, /* 72: snre=0x00C598C4, snr=15.968750 dB */
   0x1BBB, /* 73: snre=0x00D22E02, snr=15.683594 dB */
   0x1D80, /* 74: snre=0x00DF7E8C, snr=15.398438 dB */
   0x1F60, /* 75: snre=0x00ED92AA, snr=15.113281 dB */
   0x215B, /* 76: snre=0x00FC72A0, snr=14.828125 dB */
   0x2372, /* 77: snre=0x010C269B, snr=14.542969 dB */
   0x25A6, /* 78: snre=0x011CB6A2, snr=14.257813 dB */
   0x27F9, /* 79: snre=0x012E2A7C, snr=13.972656 dB */
   0x2A6B, /* 80: snre=0x0140899B, snr=13.687500 dB */
   0x2CFD, /* 81: snre=0x0153DB01, snr=13.402344 dB */
   0x2FB0, /* 82: snre=0x01682520, snr=13.117188 dB */
   0x3286, /* 83: snre=0x017D6DBF, snr=12.832031 dB */
   0x357D, /* 84: snre=0x0193B9D9, snr=12.546875 dB */
   0x3898, /* 85: snre=0x01AB0D76, snr=12.261719 dB */
   0x3BD7, /* 86: snre=0x01C36B8E, snr=11.976563 dB */
   0x3F39, /* 87: snre=0x01DCD5E0, snr=11.691406 dB */
   0x42BE, /* 88: snre=0x01F74CD4, snr=11.406250 dB */
   0x4668, /* 89: snre=0x0212CF59, snr=11.121094 dB */
   0x4A34, /* 90: snre=0x022F5AC5, snr=10.835938 dB */
   0x4E24, /* 91: snre=0x024CEAC3, snr=10.550781 dB */
   0x5235, /* 92: snre=0x026B793B, snr=10.265625 dB */
   0x5667, /* 93: snre=0x028AFE4C, snr=9.980469 dB */
   0x5AB8, /* 94: snre=0x02AB704C, snr=9.695313 dB */
   0x5F28, /* 95: snre=0x02CCC3D1, snr=9.410156 dB */
   0x63B4, /* 96: snre=0x02EEEBCE, snr=9.125000 dB */
   0x685A, /* 97: snre=0x0311D9B3, snr=8.839844 dB */
   0x6D18, /* 98: snre=0x03357DA1, snr=8.554688 dB */
   0x71ED, /* 99: snre=0x0359C6A7, snr=8.269531 dB */
   0x76D5, /* 100: snre=0x037EA30D, snr=7.984375 dB */
   0x7BCE, /* 101: snre=0x03A400A1, snr=7.699219 dB */
   0x80D6, /* 102: snre=0x03C9CD0A, snr=7.414063 dB */
   0x85EA, /* 103: snre=0x03EFF622, snr=7.128906 dB */
   0x8B08, /* 104: snre=0x04166A42, snr=6.843750 dB */
   0x902E, /* 105: snre=0x043D1892, snr=6.558594 dB */
   0x955A, /* 106: snre=0x0463F147, snr=6.273438 dB */
   0x9A89, /* 107: snre=0x048AE5D9, snr=5.988281 dB */
   0x9FBB, /* 108: snre=0x04B1E929, snr=5.703125 dB */
   0xA4EC, /* 109: snre=0x04D8EFA0, snr=5.417969 dB */
   0xAA1D, /* 110: snre=0x04FFEF34, snr=5.132813 dB */
   0xAF4C, /* 111: snre=0x0526DF6E, snr=4.847656 dB */
   0xB478, /* 112: snre=0x054DB95C, snr=4.562500 dB */
   0xB9A0, /* 113: snre=0x05747780, snr=4.277344 dB */
   0xBEC4, /* 114: snre=0x059B15B9, snr=3.992188 dB */
   0xC3E3, /* 115: snre=0x05C19120, snr=3.707031 dB */
   0xC8FD, /* 116: snre=0x05E7E7F0, snr=3.421875 dB */
   0xCE13, /* 117: snre=0x060E195F, snr=3.136719 dB */
   0xD323, /* 118: snre=0x06342583, snr=2.851563 dB */
   0xD82F, /* 119: snre=0x065A0D2F, snr=2.566406 dB */
   0xDD35, /* 120: snre=0x067FD1D8, snr=2.281250 dB */
   0xE238, /* 121: snre=0x06A5757B, snr=1.996094 dB */
   0xE736, /* 122: snre=0x06CAFA85, snr=1.710938 dB */
   0xEC31, /* 123: snre=0x06F063BD, snr=1.425781 dB */
   0xF129, /* 124: snre=0x0715B430, snr=1.140625 dB */
   0xF61D, /* 125: snre=0x073AEF25, snr=0.855469 dB */
   0xFB0F, /* 126: snre=0x0760180A, snr=0.570313 dB */
   0xFFFF, /* 127: snre=0x0785326D, snr=0.285156 dB */
};


static const uint32_t script_sds_init[] =
{
   BAST_SCRIPT_WRITE(BCHP_SDS_CGMISC, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGCTRL, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGPDWN, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV0, 0x01),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV1, 0x02),  /* uP_clk = 60.75 MHz */
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV3, 0x3D),  /*  diseqc clk = 1 MHz */
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV4, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV6, 0x10),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV8, 0xC4), /* 0xE1=300KHz, 0xC4=340KHz */
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV9, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGDIV10, 0x44),
   BAST_SCRIPT_WRITE(BCHP_SDS_CGMISC, 0x20),

   BAST_SCRIPT_AND(BCHP_SDS_GLBCTL, 0xFFFFFF7F),
   BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS1, 0xFFFFFFFF),
   BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS2, 0xFFFFFFFF),
   BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS3, 0xFFFFFFFF),
   BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS4, 0xFFFFFFFF),
   BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS5, 0xFFFFFFFF),
   BAST_SCRIPT_WRITE(BCHP_SDS_TPCTL3, 0x00000003),
   BAST_SCRIPT_WRITE(BCHP_SDS_ADCPCTL, 0x01),
   BAST_SCRIPT_WRITE(BCHP_SDS_EXTIFDIR, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_AGICTL, 0x2B),
   BAST_SCRIPT_WRITE(BCHP_SDS_AGTCTL, 0x2B),
   BAST_SCRIPT_WRITE(BCHP_SDS_AGCMISC, 0x00),
   BAST_SCRIPT_WRITE(BCHP_SDS_ATHR, 0x1B000000),
   BAST_SCRIPT_WRITE(BCHP_SDS_ABW, 0x0A0A0000),
   BAST_SCRIPT_WRITE(BCHP_SDS_AII, 0xFF000000),
   BAST_SCRIPT_WRITE(BCHP_SDS_AIT, 0x00000000),
   BAST_SCRIPT_OR(BCHP_SDS_DCOCTL1, 0x31),  /* bypass SDS post-DCO */
   BAST_SCRIPT_WRITE(BCHP_SDS_AGT, 0xFFFF0000),
   BAST_SCRIPT_WRITE(BCHP_SDS_AGI, 0x00010000),
   BAST_SCRIPT_WRITE(BCHP_SDS_DSGIN, 0x80008000),
   BAST_SCRIPT_WRITE(BCHP_SDS_AFECTL1, 0x30),
   BAST_SCRIPT_WRITE(BCHP_SDS_AFECTL, 0x28A21000),
   BAST_SCRIPT_WRITE(BCHP_SDS_AFECTL2, 0x0219AAA1),
   BAST_SCRIPT_WRITE(BCHP_SDS_BFOS, 0x06133333),
   BAST_SCRIPT_WRITE(BCHP_SDS_TPDIR, 0xFF4B008F),
   BAST_SCRIPT_WRITE(BCHP_SDS_TPCTL2, 0x8F000000),
   BAST_SCRIPT_AND(BCHP_SDS_ICB_CTL, 0xFFFFFFFE), /* disable bus errors */
   BAST_SCRIPT_WRITE(BCHP_SDS_GENCTRL1, 0x00000002), /* route ftm_pkt_tx_enable to dsec_selvtop */
   BAST_SCRIPT_EXIT
};


static const uint32_t script_sds_ext_tuner_init[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_AGICTL, 0x0B),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGTCTL, 0x0B),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGI, 0x30000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGT, 0x60000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AII, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AIT, 0x80000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DCOCTL1, 0),
      BAST_SCRIPT_WRITE(BCHP_SDS_DCOCTL2, 0x0B), /* initialize DCO BW to 2 */
      BAST_SCRIPT_WRITE(BCHP_SDS_AGICTL, 0x08),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGTCTL, 0x08),
      BAST_SCRIPT_WRITE(BCHP_SDS_AFECTL1, 0x30),
      BAST_SCRIPT_WRITE(BCHP_SDS_AFECTL2, 0x021DAAA1),
      BAST_SCRIPT_EXIT
   };

/******************************************************************************
 BAST_g2_P_InitHandleDefault()
******************************************************************************/
void BAST_g2_P_InitHandleDefault(BAST_Handle h)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);

   hDev->bcm3445Address = BAST_BCM3445_DEFAULT_ADDRESS;
   hDev->searchRange = 5000000;
   hDev->bBcm3445 = false;
   hDev->bcm3445Mi2cChannel = 0;
   hDev->lnaExtConfig = 0xFF;
   hDev->lnaIntConfig = 0xFF;
   hDev->reacqDelay = 1000000;
   hDev->vcoSeparation = 2000000;
   hDev->bFtmLocalReset = false; /* must initialize this flag independent of ftm init */
   hDev->bFtmPoweredDown = false;
   hDev->ftmChannelSelect = BAST_FskChannelConfig_eCh0Tx_Ch0Rx;
   hDev->ftmTxPower = 0x39;
   hDev->dftRangeStart = 0x0000;
   hDev->dftRangeEnd = 0x00FF;
   hDev->dftSize = BAST_G2_DFT_SIZE_512;
   hDev->ndiv = 40;
   hDev->m1div = 8;
   hDev->m2div = 5;
   hDev->m3div = 5;
}


/******************************************************************************
 BAST_g2_P_InitChannelHandle()
******************************************************************************/
void BAST_g2_P_InitChannelHandle(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   int i;

   BAST_g2_P_InitConfig(h);
   hChn->baudTimerIsr = NULL;
   hChn->berTimerIsr = NULL;
   hChn->gen1TimerIsr = NULL;
   hChn->gen2TimerIsr = NULL;
   hChn->xtalTimerIsr = NULL;
   hChn->acqState = BAST_AcqState_eIdle;
   hChn->bMi2cInProgress = false;
   hChn->acqCount = 0;
   hChn->maxReacq = 0;
   hChn->berErrors = 0;
   hChn->mpegErrorCount = 0;
   hChn->mpegFrameCount = 0;
   hChn->ldpcBadBlocks = 0;
   hChn->ldpcCorrBlocks = 0;
   hChn->ldpcTotalBlocks = 0;
   hChn->rsCorr = 0;
   hChn->rsUncorr = 0;
   hChn->preVitErrors = 0;
   hChn->turboBadBlocks = 0;
   hChn->turboCorrBlocks = 0;
   hChn->turboTotalBlocks = 0;
   hChn->ldpcCtl = 0;
   hChn->extTunerIfOffset = 0;
   hChn->extTunerRfAgcTop = 0xFFFF;       /* external RF AGC Threshold */
   hChn->extTunerIfAgcTop = 0x0000;       /* external IF AGC Threshold */
   hChn->intTuner3StageAgcTop = 0xA000;   /* 3-stage IF AGC Threshold */
   hChn->bLastLocked = false;
   hChn->diseqcPreTxDelay = 15; /* default 15 ms delay */
   hChn->diseqcRRTOuSec = 210000;
   hChn->diseqcCtl = BAST_G2_DISEQC_CTL_ENVELOPE;
   hChn->diseqcToneThreshold = 0x6E;   /* 687.5mV * 0.16 = 110 */
   hChn->diseqcBitThreshold = 0x2D3;   /* 723us */
   hChn->diseqcVsenseThreshHi = 0x30;  /* ~22 V */
   hChn->diseqcVsenseThreshLo = 0xD1;  /* ~10 V */
   hChn->blindScanModes = BAST_G2_BLIND_SCAN_MODES_DVB | BAST_G2_BLIND_SCAN_MODES_TURBO | BAST_G2_BLIND_SCAN_MODES_LDPC;
#ifndef BAST_EXCLUDE_POWERDOWN
   hChn->coresPoweredDown = BAST_CORE_ALL;
#else
   hChn->coresPoweredDown = 0;
#endif
   hChn->dtvScanCodeRates = 0xFF;
   hChn->dvbScanCodeRates = 0xFF;
   hChn->turboScanCurrMode = 0;
   hChn->turboScanModes = 0xFFFF;
   hChn->tuneDacDivRatio = BAST_G2_TUNER_DAC_DIV_RATIO;
   hChn->splitterModeAdj = 0;
   hChn->peakScanSymRateMin = 0;
   hChn->peakScanSymRateMax = 0;
   hChn->acqTime = 0;
   hChn->acqTimeState = 0;
   hChn->statusEventIndicators = 0;
   hChn->rainFadeThreshold = 3<<24; /* 3dB*2^24 */
   hChn->freqDriftThreshold = 1500000;
   hChn->bStatusInterrupts = false;
   hChn->rainFadeWindow = 3000; /* 300 secs */
   hChn->bcm3445Status = 0;
   hChn->bcm3445Mi2cChannel = BAST_Mi2cChannel_e0;
   hChn->bcm3445TunerInput = BAST_Bcm3445OutputChannel_eNone;
   hChn->bcm3445Ctl = BAST_G2_BCM3445_CTL_AGC_TOP_MID;
   hChn->stuffBytes = 0;
   hChn->currentCnrThreshold1 = 0;
   hChn->turboCtl = 0;
   hChn->tunerBandgap = 0x10; /* set initial bandgap to 4 */
   hChn->bTunerCalFailed = false;
   hChn->tuneMixStatus = 0;
   hChn->tunerGmxIdx = 0xFE;
   hChn->freqTransferInt = 0;
   hChn->tunerOffset = 0;
   hChn->bFsNotDefault = true;
   hChn->dftFreqEstimateStatus = 0;
   hChn->bForceReacq = false;
   hChn->reacqCtl = 0;
   hChn->spinvState = 0;

   for (i = 0; i < 16; i++)
      hChn->ldpcScramblingSeq[i] = 0x00;
   hChn->ldpcScramblingSeq[2] = 0x01;

   hChn->acqParams.symbolRate = 20000000;
   hChn->acqParams.carrierFreqOffset = 0;
   hChn->acqParams.mode = BAST_Mode_eDss_scan;
   hChn->acqParams.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
   hChn->diseqcStatus.status = BAST_DiseqcSendStatus_eSuccess;

   BAST_g2_ResetInterruptCounters(h);

#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_g2_P_ResetStatusIndicators_isrsafe(h);
#endif
}


/******************************************************************************
 BAST_g2_P_InitChannels_isrsafe()
******************************************************************************/
void BAST_g2_P_InitChannels_isrsafe(BAST_ChannelHandle h, bool bInitAllChannels)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val = 0;

   BAST_g2_P_IndicateNotLocked_isrsafe(h);

   if (hChn->bHasAfec)
   {
      /* power up afec to set test config */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_SPLL_PWRDN, ~0x10, 0x00);
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_TEST_CONFIG, &val);
   }

   BAST_g2_P_SetDefaultSampleFreq_isrsafe(h);

   /* initialize clockgen and sds */
   BAST_g2_P_ProcessScript_isrsafe(h, script_sds_init);
   val = hChn->intTuner3StageAgcTop << 16;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IAGCTH, &val);

   BAST_g2_P_SetDiseqcClock_isrsafe(h);

   if (!hChn->bExternalTuner)
   {
      BAST_g2_P_TunerInit(h, bInitAllChannels ? BAST_g2_P_InitChannels1_isrsafe : BAST_g2_P_InitChannels2_isrsafe);
   }
   else
   {
      BAST_g2_P_ProcessScript_isrsafe(h, script_sds_ext_tuner_init);
      if (bInitAllChannels)
         BAST_g2_P_InitChannels1_isrsafe(h);
      else
         BAST_g2_P_InitChannels2_isrsafe(h);
   }
   return;
}


/******************************************************************************
 BAST_g2_P_InitChannels1_isrsafe()
******************************************************************************/
BERR_Code BAST_g2_P_InitChannels1_isrsafe(BAST_ChannelHandle h)
{
   if (h->channel == (h->pDevice->totalChannels - 1))
      return BAST_g2_P_InitChannels2_isrsafe(h);
   else
   {
      BAST_g2_P_InitChannels_isrsafe(h->pDevice->pChannels[h->channel + 1], true);
      return BERR_SUCCESS;
   }
}


/******************************************************************************
 BAST_g2_P_InitChannels2_isrsafe()
******************************************************************************/
BERR_Code BAST_g2_P_InitChannels2_isrsafe(BAST_ChannelHandle h)
{
   BKNI_SetEvent(((BAST_g2_P_Handle *)(h->pDevice->pImpl))->hInitDoneEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_SetDefaultSampleFreq_isrsafe()
******************************************************************************/
void BAST_g2_P_SetDefaultSampleFreq_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t p1, val;

   if (hChn->bFsNotDefault)
   {
      val = 0x01;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CGDIV0, &val);
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CGMISC, 0xFF, 0x20);

      p1 = (hDev->xtalFreq == 54000000 ? 2 : 1);
      BAST_g2_P_SetSampleFreq_isrsafe(h, p1, 1, hDev->ndiv, hDev->m1div, hDev->m2div, hDev->m3div); /* VCO=1080MHz */

      BAST_g2_P_SetDiseqcClock_isrsafe(h);
      if (hChn->sampleFreq == BAST_DEFAULT_SAMPLE_FREQ)
         hChn->bFsNotDefault = false;
      else
      {
         BDBG_ERR(("default sample freq is not %d", BAST_DEFAULT_SAMPLE_FREQ));
      }
   }
}


/******************************************************************************
 BAST_g2_P_GetLockChangeEventHandle()
******************************************************************************/
BERR_Code BAST_g2_P_GetLockChangeEventHandle(BAST_ChannelHandle h, BKNI_EventHandle *hLockChangeEvent)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ENTER(BAST_g2_P_GetLockChangeEventHandle);

   *hLockChangeEvent = ((BAST_g2_P_ChannelHandle *)h->pImpl)->hLockChangeEvent;

   BDBG_LEAVE(BAST_g2_P_GetLockChangeEventHandle);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_DisableTimer_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g2_P_DisableTimer_isr(BAST_ChannelHandle h, BAST_TimerSelect t)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t bcktmr = 0xFFFFFFFF;
   BINT_CallbackHandle hCb = NULL;
   BERR_Code retCode = BERR_SUCCESS;

   switch (t)
   {
      case BAST_TimerSelect_eBaud:
      case BAST_TimerSelect_eBaudUsec:
         bcktmr &= ~0x02;
         hCb = hChn->hBaudTimerCb;
         hChn->baudTimerIsr = NULL;
         break;
      case BAST_TimerSelect_eBer:
         bcktmr &= ~0x04;
         hCb = hChn->hBerTimerCb;
         hChn->berTimerIsr = NULL;
         break;
      case BAST_TimerSelect_eGen1:
         bcktmr &= ~0x08;
         hCb = hChn->hGen1TimerCb;
         hChn->gen1TimerIsr = NULL;
         break;
      case BAST_TimerSelect_eGen2:
         bcktmr &= ~0x10;
         hCb = hChn->hGen2TimerCb;
         hChn->gen2TimerIsr = NULL;
         break;
      case BAST_TimerSelect_eXtal:
         bcktmr &= ~0x20;
         hCb = hChn->hXtalTimerCb;
         hChn->xtalTimerIsr = NULL;
         break;
      default:
         BDBG_ASSERT(0);
   }

   BINT_DisableCallback_isr(hCb);
   BINT_ClearCallback_isr(hCb);

   if (retCode == BERR_SUCCESS)
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BCKTMR, bcktmr, 0);

   return retCode;
}


/******************************************************************************
 BAST_g2_P_EnableTimer() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_EnableTimer(BAST_ChannelHandle h, BAST_TimerSelect t, uint32_t delay, BAST_g2_FUNCT func)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BAST_g2_P_EnableTimer_isr(h, t, delay, func);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BAST_g2_P_EnableTimer_isr()
******************************************************************************/
BERR_Code BAST_g2_P_EnableTimer_isr(BAST_ChannelHandle h, BAST_TimerSelect t, uint32_t delay, BAST_g2_FUNCT func)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BINT_CallbackHandle hCb = NULL;
   uint32_t P_hi, P_lo, Q_hi, val, mask, reg = 0;
   uint32_t bcktmr_set = 0, bcktmr_clr = 0xFFFFFFFF;

   BDBG_ASSERT(delay);
   BDBG_ASSERT(h);

   BAST_g2_P_DisableTimer_isr(h, t);

   switch (t)
   {
      case BAST_TimerSelect_eBaud:
      case BAST_TimerSelect_eBaudUsec:
         if ((t == BAST_TimerSelect_eBaudUsec) && (hChn->acqParams.symbolRate < 3000000))
         {
            /* convert BAST_TimerSelect_eBaudUsec to BAST_TimerSelect_eBaud */
            t = BAST_TimerSelect_eBaud;
            BMTH_HILO_32TO64_Mul(delay, hChn->acqParams.symbolRate, &P_hi, &P_lo);
            BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1000000, &Q_hi, &delay);
         }

         /* load timer value */
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BTMR, &delay);

         /* clear the baud clock timer interrupt */
         reg = BCHP_SDS_IRQSTS1;
         mask = 0x20;

         if (func)
         {
            hChn->baudTimerIsr = func;
            hCb = hChn->hBaudTimerCb;
         }
         else
            hChn->baudTimerIsr = NULL;

         if (t == BAST_TimerSelect_eBaudUsec)
            bcktmr_set |= 0x01;
         else
            bcktmr_clr &= ~0x01;
         bcktmr_set |= 0x02; /* timer start */
         break;

      case BAST_TimerSelect_eBer:
         /* load timer value */
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BERTMR, &delay);

         /* clear the ber timer interrupt */
         /* clear the baud clock timer interrupt */
         reg = BCHP_SDS_IRQSTS1;
         mask = 0x40;

         if (func)
         {
            hChn->berTimerIsr = func;
            hCb = hChn->hBerTimerCb;
         }
         else
            hChn->berTimerIsr = NULL;

         bcktmr_set |= 0x04;
         break;

      case BAST_TimerSelect_eGen1:
         /* load timer value */
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_GENTMR1, &delay);

         /* clear the timer interrupt */
         reg = BCHP_SDS_IRQSTS5;
         mask = 0x00000200;

         if (func)
         {
            hChn->gen1TimerIsr = func;
            hCb = hChn->hGen1TimerCb;
         }
         else
            hChn->gen1TimerIsr = NULL;

         bcktmr_set |= 0x08;
         break;

      case BAST_TimerSelect_eGen2:
         /* load timer value */
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_GENTMR2, &delay);

         /* clear the timer interrupt */
         reg = BCHP_SDS_IRQSTS5;
         mask = 0x00020000;

         if (func)
         {
            hChn->gen2TimerIsr = func;
            hCb = hChn->hGen2TimerCb;
         }
         else
            hChn->gen2TimerIsr = NULL;

         bcktmr_set |= 0x10;
         break;

      case BAST_TimerSelect_eXtal:
         /* load timer value */
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_XTLTMR, &delay);

         /* clear the timer interrupt */
         reg = BCHP_SDS_IRQSTS5;
         mask = 0x00040000;

         if (func)
         {
            hChn->xtalTimerIsr = func;
            hCb = hChn->hXtalTimerCb;
         }
         else
            hChn->xtalTimerIsr = NULL;

         bcktmr_set |= 0x20;
         break;

      default:
         BDBG_ASSERT(0); /* should never get here */
   }

   if (func)
   {
      /* clear interrupt and enable */
      BINT_ClearCallback_isr(hCb);
      BINT_EnableCallback_isr(hCb);
   }
   else
   {
      /* clear timer interrupt status for polling */
      BAST_g2_P_WriteRegister_isrsafe(h, reg, &mask);
   }

   /* start the timer */
   BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_BCKTMR, bcktmr_clr, bcktmr_set);

   /* poll timer if no isr */
   if (func == NULL)
   {
      val = 0;
      while ((val & mask) == 0)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, reg, &val);
         /* if (val & mask) break; */
      }
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_BaudTimer_isr()
******************************************************************************/
void BAST_g2_P_BaudTimer_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BAST_g2_FUNCT funct;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eBaudTimer);
   funct = hChn->baudTimerIsr;
   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBaudUsec);
   if (funct)
      funct(h);
}


/******************************************************************************
 BAST_g2_P_BerTimer_isr()
******************************************************************************/
void BAST_g2_P_BerTimer_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BAST_g2_FUNCT funct;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eBerTimer);
   funct = hChn->berTimerIsr;
   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBer);
   if (funct)
      funct(h);
}


/******************************************************************************
 BAST_g2_P_Gen1Timer_isr()
******************************************************************************/
void BAST_g2_P_Gen1Timer_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BAST_g2_FUNCT funct;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eGenTimer1);
   funct = hChn->gen1TimerIsr;
   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eGen1);
   if (funct)
      funct(h);
}


/******************************************************************************
 BAST_g2_P_Gen2Timer_isr()
******************************************************************************/
void BAST_g2_P_Gen2Timer_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BAST_g2_FUNCT funct;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eGenTimer2);
   funct = hChn->gen2TimerIsr;
   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eGen2);
   if (funct)
      funct(h);
}


/******************************************************************************
 BAST_g2_P_XtalTimer_isr()
******************************************************************************/
void BAST_g2_P_XtalTimer_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BAST_g2_FUNCT funct;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eXtalTimer);
   funct = hChn->xtalTimerIsr;
   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eXtal);
   if (funct)
      funct(h);
}


/******************************************************************************
 BAST_g2_P_IfAgcLeThreshRise_isr()
******************************************************************************/
void BAST_g2_P_IfAgcLeThreshRise_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eIfAgcLeThreshRise);
   BAST_g2_P_3StageAgc_isr(p, param);
}


/******************************************************************************
 BAST_g2_P_IfAgcLeThreshFall_isr()
******************************************************************************/
void BAST_g2_P_IfAgcLeThreshFall_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eIfAgcLeThreshFall);
   BAST_g2_P_3StageAgc_isr(p, param);
}


/******************************************************************************
 BAST_g2_P_RfAgcLeThreshRise_isr()
******************************************************************************/
void BAST_g2_P_RfAgcLeThreshRise_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eRfAgcLeThreshRise);
   BAST_g2_P_3StageAgc_isr(p, param);
}


/******************************************************************************
 BAST_g2_P_RfAgcLeThreshFall_isr()
******************************************************************************/
void BAST_g2_P_RfAgcLeThreshFall_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eRfAgcLeThreshFall);
   BAST_g2_P_3StageAgc_isr(p, param);
}


/******************************************************************************
 BAST_g2_P_RfAgcGtMaxChange_isr()
******************************************************************************/
void BAST_g2_P_RfAgcGtMaxChange_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eRfAgcGtMaxChange);
   BAST_g2_P_3StageAgc_isr(p, param);
}


/******************************************************************************
 BAST_g2_P_3StageAgc_isr()
******************************************************************************/
void BAST_g2_P_3StageAgc_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t aii, ait;
   uint32_t ldaii_thresh;

   static const uint32_t script_3agc_0[] =
   {
      BAST_SCRIPT_AND(BCHP_SDS_AGCMISC, 0xFB), /* enable 2 stage AGC algorithm */
      BAST_SCRIPT_AND(BCHP_SDS_AGTCTL, 0xFD),  /* unfreeze RF PGA */
      BAST_SCRIPT_AND(BCHP_SDS_AGICTL, 0xFD),  /* unfreeze IF PGA */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_3agc_1[] =
   {
      BAST_SCRIPT_OR(BCHP_SDS_AGCMISC, 0x04), /* disable 2 stage AGC algorithm */
      BAST_SCRIPT_OR(BCHP_SDS_AGTCTL, 0x02),  /* freeze RF PGA */
      BAST_SCRIPT_AND(BCHP_SDS_AGICTL, 0xFD), /* unfreeze IF PGA */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_3agc_2[] =
   {
      BAST_SCRIPT_OR(BCHP_SDS_AGCMISC, 0x04), /* disable 2 stage AGC algorithm */
      BAST_SCRIPT_AND(BCHP_SDS_AGTCTL, 0xFD), /* unfreeze RF PGA */
      BAST_SCRIPT_OR(BCHP_SDS_AGICTL, 0x02),  /* freeze IF PGA */
      BAST_SCRIPT_EXIT
   };
   BSTD_UNUSED(param);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AII, &aii);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AIT, &ait);

   ldaii_thresh = hChn->intTuner3StageAgcTop << 16;

   if (aii > ldaii_thresh) /* IF PGA gain exceeds threshold */
   {
      if ((ait & 0xFFFFFFF0) == 0xFFFF0000) /* RF PGA gain is max out */
      {
         goto enable_2_stage;
      }
      else /* RF PGA gain is not max out */
      {
         goto disable_2_stage_2;
      }
   }
   else if ((aii & 0xFFFFFFF0) == ldaii_thresh)
   {
      /* IF PGA gain is fix at threshold */
      if ((ait & 0xFFFFFFF0) == 0xFFFF0000)
      {
         /* RF PGA gain is max out */
         enable_2_stage:
         BAST_g2_P_ProcessScript_isrsafe(h, script_3agc_0);
      }
      else if ((ait & 0xFFFFFFF0) == 0x00000000)
      {
         /* RF PGA gain is bottom out */
         goto disable_2_stage;
      }
   }
   else   /* IF PGA gain is lower than threshold */
   {
      if ((ait & 0xFFFF0000) == 0x00000000)   /* RF PGA gain is bottom out */
      {
         disable_2_stage:
         BAST_g2_P_ProcessScript_isrsafe(h, script_3agc_1);

#if 0
         /* disable PGA hysteresis when writting to AIT */
         val = 0;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TUNER_DIGCTL5, &val);
#endif

         /* fix RF PGA gain */
         ait = 0x00000000;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AIT, &ait);
      }
      else   /* RF PGA gain is not minimum */
      {
         disable_2_stage_2:
         BAST_g2_P_ProcessScript_isrsafe(h, script_3agc_2);

#if 0
         /* disable PGA hysteresis when writting to AII */
         val = 0;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TUNER_DIGCTL5, &val);
#endif

         /* fix IF PGA gain */
         aii = ldaii_thresh;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AII, &aii);
      }

#if 0
      /* re-enable PGA hysteresis */
      val = 0x01;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TUNER_DIGCTL5, &val);
#endif
   }
}


/******************************************************************************
 BAST_g2_P_ProcessScript_isrsafe()
******************************************************************************/
BERR_Code BAST_g2_P_ProcessScript_isrsafe(BAST_ChannelHandle hChn, uint32_t const *pScr)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb, val, reg, op;

   while (1)
   {
      mb = *pScr++;
      val = *pScr++;
      reg = mb & ~BAST_SCRIPT_OPCODE;
      op = mb & BAST_SCRIPT_OPCODE;
      if (op == BAST_SCRIPT_OPCODE_WRITE)
      {
         BAST_g2_P_WriteRegister_isrsafe(hChn, reg, &val);
      }
      else if (op == BAST_SCRIPT_OPCODE_AND)
      {
         BAST_g2_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb &= val;
         BAST_g2_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_OR)
      {
         BAST_g2_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb |= val;
         BAST_g2_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_AND_OR)
      {
         BAST_g2_P_ReadRegister_isrsafe(hChn, reg, &mb);
         mb &= val;
         mb |= *pScr++;
         BAST_g2_P_WriteRegister_isrsafe(hChn, reg, &mb);
      }
      else if (op == BAST_SCRIPT_OPCODE_DEBUG)
      {
         BDBG_MSG(("script debug: %d", val));
      }
      else if (op == BAST_SCRIPT_OPCODE_EXIT)
         break;
      else
      {
         BDBG_ERR(("invalid script operation"));
         retCode = BERR_INVALID_PARAMETER;
         break;
      }
   }

   return retCode;
}


/******************************************************************************
 BAST_g2_P_Enable3StageAgc_isr() - ISR context
******************************************************************************/
void BAST_g2_P_Enable3StageAgc_isr(BAST_ChannelHandle h, bool bEnable)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t mb;

   /* skip agc if external tuner */
   if (hChn->bExternalTuner)
      return;

   mb = hChn->intTuner3StageAgcTop << 16;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IAGCTH, &mb);

   if (bEnable)
   {
      BINT_EnableCallback_isr(hChn->h3StageAgc0Cb);
      BINT_EnableCallback_isr(hChn->h3StageAgc1Cb);
      BINT_EnableCallback_isr(hChn->h3StageAgc2Cb);
      BINT_EnableCallback_isr(hChn->h3StageAgc3Cb);
      BINT_EnableCallback_isr(hChn->h3StageAgc4Cb);
   }
   else
   {
      BINT_DisableCallback_isr(hChn->h3StageAgc0Cb);
      BINT_DisableCallback_isr(hChn->h3StageAgc1Cb);
      BINT_DisableCallback_isr(hChn->h3StageAgc2Cb);
      BINT_DisableCallback_isr(hChn->h3StageAgc3Cb);
      BINT_DisableCallback_isr(hChn->h3StageAgc4Cb);

      BINT_ClearCallback_isr(hChn->h3StageAgc0Cb);
      BINT_ClearCallback_isr(hChn->h3StageAgc1Cb);
      BINT_ClearCallback_isr(hChn->h3StageAgc2Cb);
      BINT_ClearCallback_isr(hChn->h3StageAgc3Cb);
      BINT_ClearCallback_isr(hChn->h3StageAgc4Cb);
   }
}


/******************************************************************************
 BAST_g2_P_GCF_isr()
******************************************************************************/
uint32_t BAST_g2_P_GCF_isr(uint32_t m, uint32_t n)
{
   uint32_t L1, L2, L3, L4;

   L1 = m;
   L2 = n;

   while (L2 > 0)
   {
      L3 = L1 / L2;
      L4 = L1 - (L2 * L3);
      L1 = L2;
      L2 = L4;
   }
   return L1;
}


/******************************************************************************
 BAST_g2_P_ReadModifyWriteRegister_isrsafe()
******************************************************************************/
void BAST_g2_P_ReadModifyWriteRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, reg, &val);
   val = (val & and_mask) | or_mask;
   BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g2_P_OrRegister_isrsafe()
******************************************************************************/
void BAST_g2_P_OrRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, reg, &val);
   val |= or_mask;
   BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g2_P_AndRegister_isrsafe()
******************************************************************************/
void BAST_g2_P_AndRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, reg, &val);
   val &= and_mask;
   BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g2_P_SetSampleFreq_isrsafe()
******************************************************************************/
void BAST_g2_P_SetSampleFreq_isrsafe(BAST_ChannelHandle h, uint32_t p1, uint32_t p2, uint32_t ndiv, uint32_t m1div, uint32_t m2div, uint32_t m3div)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, freq;

   if ((hChn->acqState == BAST_AcqState_eIdle) || BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
         val = 0x18; /* turn off both tfec and afec */
      else if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
         val = 0x08; /* turn off tfec, turn on afec */
      else /* BAST_MODE_IS_TURBO(hChn->acqParams.mode) */
         val = 0x10;  /* turn on tfec, turn off afec */
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_SPLL_PWRDN, 0xFFFFFFE7, val);

   if (hChn->bHasDedicatedPll == true)
   {
      BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_SPLL_MODE, 0x04); /* set enb_clkout */

      /* issue i_areset and i_dreset */
      val = 0x03;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_RST, &val);

      val = 0;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_FRAC, &val);

      val = ndiv & 0x1FF;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_INT, &val);

      val = p2 & 0x0F;
      val |= (p1 & 0x0F) << 4;
      val |= (m1div & 0xFF) << 8; /* Fs */
      val |= (m2div & 0xFF) << 16; /* tfec */
      val |= (m3div & 0xFF) << 24; /* afec */
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_MPDIV, &val);

      /* set up PLL VCO range */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_MODE, &val);
      freq = (((BAST_g2_P_Handle *)h->pDevice->pImpl)->xtalFreq * ndiv * (p2 & 0x0F)) / (p1 & 0x0F);
      if (freq < 1600000000UL)
         val &= ~0x00000100;
      else
         val |= 0x00000100;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_MODE, &val);

      /* release i_areset */
      val = 0x02;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_RST, &val);

      BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_SPLL_MODE, ~0x04); /* clear enb_clkout */

      /* wait for SPLL lock */
      while (1)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_MODE, &val);
         if (val & 0x01000000)
            break;
      }

      /* release i_dreset */
      val = 0x00;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_RST, &val);
   }
   else
   {
      /* clock comes from SDS0 SPLL or Ext REF PLL */
      val = p2 & 0x0F;
      val |= (p1 & 0x0F) << 4;
      val |= (m1div & 0xFF) << 8; /* Fs */
      val |= (m2div & 0xFF) << 16; /* tfec */
      val |= (m3div & 0xFF) << 24; /* afec */
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SPLL_MPDIV, &val);
   }

   BAST_g2_P_GetSampleFreq_isrsafe(h);
}


/******************************************************************************
 BAST_g2_P_GetSampleFreq_isrsafe()
******************************************************************************/
void BAST_g2_P_GetSampleFreq_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, cgdiv1, cgdiv0, P_hi, P_lo, Q_hi, Q_lo;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGDIV1, &cgdiv1);
   cgdiv1 &= 0x0F;

   if (hChn->bHasDedicatedPll == true)
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGDIV0, &cgdiv0);
      cgdiv0 &= 0x0F;
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_INT, &lval1);
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_FRAC, &lval2);
      lval1 += lval2;
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_MPDIV, &lval2);
      lval1 *= (lval2 & 0x0F);
      BMTH_HILO_32TO64_Mul(((BAST_g2_P_Handle *)h->pDevice->pImpl)->xtalFreq, lval1, &P_hi, &P_lo);
      lval1 = ((lval2 >> 8) & 0xFF) * ((lval2 >> 4) & 0x0F);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &Q_hi, &Q_lo);

      hChn->upClk = (((Q_lo << 1) / cgdiv1) + 1) >> 1;
      hChn->sampleFreq = (((Q_lo << 1) / cgdiv0) + 1) >> 1;
   }
   else
   {
      hChn->sampleFreq = BAST_DEFAULT_SAMPLE_FREQ;
      hChn->upClk = hChn->sampleFreq / cgdiv1;
   }
   BDBG_MSG(("channel %d: Fs=%u", h->channel, hChn->sampleFreq));
}


/******************************************************************************
 BAST_g2_P_GetDiseqcClock_isrsafe()
******************************************************************************/
void BAST_g2_P_GetDiseqcClock_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, dsec_div;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGDIV4, &val);
   dsec_div = (val & 0x07) << 8;
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGDIV3, &val);
   dsec_div |= (val & 0xFF);
   hChn->diseqcClk = (((hChn->upClk << 1) / dsec_div) + 1) >> 1;
}


/******************************************************************************
 BAST_g2_P_SetDiseqcClock_isrsafe()
******************************************************************************/
void BAST_g2_P_SetDiseqcClock_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, dsec_div;

   dsec_div = (hChn->upClk + 500000) / 1000000;
   val = (dsec_div >> 8) & 0xFF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CGDIV4, &val);
   val = dsec_div & 0xFF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CGDIV3, &val);
   val = 0x20;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CGMISC, &val);

   BAST_g2_P_GetDiseqcClock_isrsafe(h);

   /* adjust FCW for 22KHz DDFS tone generator */
   if (hChn->bHasDiseqc)
   {
      val = (((16777216 * 64) / hChn->diseqcClk) * 22587) + 32;
      val = val >> 6;
      BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_DTCT, 0xFF000000, val & 0x00FFFFFF);
   }
}


/******************************************************************************
 BAST_g2_P_SetNyquistAlpha_isr()
******************************************************************************/
void BAST_g2_P_SetNyquistAlpha_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t Fs_over_Fb, nvctl, pfctl;

   Fs_over_Fb = (hChn->sampleFreq * 4) / hChn->acqParams.symbolRate;
   if (Fs_over_Fb < 9)
     nvctl = 0x80;
   else if (Fs_over_Fb < 10)
     nvctl = 0x40;
   else
     nvctl = 0;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_NYQUIST_20)
   {
      /* alpha = 0.20 */
      nvctl |= 0x04; /* nvctl */
      pfctl = 0x01;  /* pfctl */
   }
   else
   {
      /* alpha = 0.35 */
      pfctl = 0x00; /* pfctl */
   }

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_NVCTL, &nvctl);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PFCTL, &pfctl);
}


/******************************************************************************
 BAST_g2_P_SetSymbolRate_isr()
******************************************************************************/
void BAST_g2_P_SetSymbolRate_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t Fs_over_Fb, val, val2, data0, P_hi, P_lo, Q_hi, Q_lo, filtctl;

   Fs_over_Fb = (hChn->sampleFreq * 8) / hChn->acqParams.symbolRate; /* scaled 2^3 */
   if (Fs_over_Fb < 33)
   {
      /* undersample mode */
      BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_CGCTRL, 0x02);
   }
   else
   {
      BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_CGCTRL, ~0x02);
   }

   val2 = Fs_over_Fb;
   data0 = 1;
   while ((val2 > 64) && (data0 < 64))
   {
      data0 = data0 << 1;
      val2 = val2 >> 1;
   }
   if (data0 > 32) /* all 6 HBs are used */
      filtctl = 0x40;
   else
      filtctl = (32 - data0) | 0xC0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FILTCTL, &filtctl);

   if (val2 >= 51)
      val = 0x00; /* quarterband */
   else if (val2 > 34)
      val = 0x40; /* third band */
   else
      val = 0xC0; /* half band */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_NDFCTL, &val);

   BMTH_HILO_32TO64_Mul(16777216, hChn->sampleFreq, &P_hi, &P_lo);
   val = hChn->acqParams.symbolRate * data0;
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, val, &Q_hi, &Q_lo);
   if (Fs_over_Fb < 33)
      Q_lo = Q_lo << 1;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BFOS, &Q_lo);

   val = Q_lo >> 2;
   filtctl &= ~0x60;
   filtctl &= 0xFF;
   if (filtctl == 0x9F)
      data0 = 0;
   else if (filtctl == 0x9E)
      data0 = 1;
   else if (filtctl == 0x9C)
      data0 = 2;
   else if (filtctl == 0x98)
      data0 = 3;
   else if (filtctl == 0x90)
      data0 = 4;
   else if (filtctl == 0x80)
      data0 = 5;
   else
      data0 = 6;
   val = val << data0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRM, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRMR, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FFNORM, &val);

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLIF, &val);

   BMTH_HILO_32TO64_Mul(134217728, hChn->acqParams.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq >> 1, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;
   Q_lo &= 0x3FFF0000;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FILTCTL, &filtctl);
   filtctl &= 0xFF;
   if (filtctl == 0x10)
      val = 0x08;
   else if (filtctl == 0x00)
      val = 0x18;
   else
      val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFCTL, &val);
}


/******************************************************************************
 BAST_g2_P_InitBert_isr()
******************************************************************************/
void BAST_g2_P_InitBert_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PN_INVERT)
      val = 0x02;
   else
      val = 0x00;
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PRBS15)
      val |= 0x01;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BERCTLA, &val);
}


/******************************************************************************
 BAST_g2_P_StartBert_isr()
******************************************************************************/
void BAST_g2_P_StartBert_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BERT_ENABLE)
   {
      /* enable BERT */
      if (hChn->xportCtl & BAST_G2_XPORT_CTL_SERIAL)
         val = 0xD0; /* serial */
      else
         val = 0xC0; /* parallel */

      if ((hChn->xportCtl & BAST_G2_XPORT_CTL_XBERT) &&
          (BAST_MODE_IS_DTV(hChn->acqParams.mode)))
      {
         val |= 0x08; /* XBERT */
      }

      for (i = 0; i < 3; i++)
      {
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BERCTLB, &val);
         val++;
      }
   }
   else
   {
      val = 0x01;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BERCTLB, &val);
   }
}


/******************************************************************************
 BAST_g2_P_ResyncBert_isr()
******************************************************************************/
void BAST_g2_P_ResyncBert_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BERT_RESYNC_DISABLE)
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BEIT, &val);
      if (val != 0x1FFF2FFF)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BEST, &val);
         if ((val & 0x0E000000) == 0x02000000)
         {
            /* bert is locked */
            val = 0x1FFF2FFF;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BEIT, &val);
         }
      }
   }
}


/******************************************************************************
 BAST_g2_P_SetBaudBw_isr() - damp is scaled 2^2
******************************************************************************/
void BAST_g2_P_SetBaudBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, lval3, P_hi, P_lo;

   BMTH_HILO_32TO64_Mul(bw, 1073741824, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval2, &lval1);
   lval2 = ((lval1 * damp) + 1) >> 1;
   lval2 &= 0xFFFFFF00;
   BMTH_HILO_32TO64_Mul(lval1, bw, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval3, &lval1);

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRLC, &lval2);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRIC, &lval1);
}


/******************************************************************************
 BAST_g2_P_SetCarrierBw_isr() - damp is scaled 2^2
******************************************************************************/
void BAST_g2_P_SetCarrierBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2, P_hi, P_lo, x, mb, stfllc;
   static const uint32_t int_scale[7] = {1, 4, 16, 64, 256, 1024, 4096};
   static const uint32_t lin_scale[5] = {1, 4, 16, 64, 256};

   for (x = 0; x < 5; x++)
   {
      lval1 = ((damp * bw) >> 1) * lin_scale[x];
      if (lval1 > hChn->sampleFreq)
         break;
   }
   if (x)
      x--;

   BMTH_HILO_32TO64_Mul(damp * bw, 8192 * 256 * lin_scale[x], &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &lval2, &lval1);
   stfllc = (lval1 << 7) + 0x8000;
   stfllc &= 0x3FFF0000;
   stfllc |= ((x & 0x07) << 8);

   /* compute 16-bit integrator coefficient */
   for (x = 0; x < 7; x++)
   {
      BMTH_HILO_32TO64_Mul(bw * bw, int_scale[x], &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &lval2, &lval1);
      if (lval1 > hChn->sampleFreq)
         break;
   }
   if (x)
      x--;

   lval2 = ((bw + 5) / 10);
   BMTH_HILO_32TO64_Mul(lval2 * int_scale[x] * 8, lval2 * 8192, &P_hi, &P_lo);
   lval1 = ((hChn->acqParams.symbolRate + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   lval1 = ((hChn->sampleFreq + 5) / 10);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, lval1, &P_hi, &P_lo);
   mb = ((P_lo << 15) + 0x8000);
   mb &= 0xFFFF0000;
   mb |= ((x & 0x0F) << 8);

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLLC, &stfllc);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLIC, &mb);
}


/******************************************************************************
 BAST_g2_P_SetOpll_isr()
******************************************************************************/
BERR_Code BAST_g2_P_SetOpll_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t opll, opll2, P_hi, P_lo, Q_hi, Q_lo, data0, val = 0;

   /* compute output bit rate */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OPLL, &opll);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OPLL2, &opll2);
   BMTH_HILO_32TO64_Mul(opll, hChn->acqParams.symbolRate * 8, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, opll+opll2, &Q_hi, &Q_lo);
   hChn->outputBitRate = (Q_lo + 1) >> 1;
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGCTRL, &val);
   if (val & 0x02)
      hChn->outputBitRate = hChn->outputBitRate >> 1;

   BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_OIFCTL3, 0x14); /* bypass OPLL, bypass FIFO */

   /* set OIFCTL */
   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
      val = 0x00;
   else if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
      val = 0x81;
   else /* LDPC */
      val = 0x80;
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PN_INVERT)
      val |= 0x08;
   if (hChn->xportCtl & BAST_G2_XPORT_CTL_XBERT)
      val |= 0x04;

   Q_lo = 0;
   write_oifctl:
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OIFCTL, &val);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OIFCTL, &data0);
   if (val != data0)
   {
      if (Q_lo++ > 10)
         return BERR_TIMEOUT;
      goto write_oifctl;
   }

   /* set EXTIFDIR */
   val = 0x00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EXTIFDIR, &val);

   /* set OIFCTL2 */
   data0 = hChn->xportCtl >> 8;
   val = 0;
   if (data0 & 0x80)
      val |= 0x80;
   if (data0 & 0x40)
      val |= 0x40;
   if (data0 & 0x20)
      val |= 0x20;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OIFCTL2, &val);

   /* set PSCTL */
   val = hChn->xportCtl & 0x00FA;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PSCTL, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_SetEqffe3_isr()
******************************************************************************/
void BAST_g2_P_SetEqffe3_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGCTRL, &val);
   if ((val & 0x02) || (BAST_MODE_IS_DTV(hChn->acqParams.mode)))
      val = 0x06;
   else
      val = 0x0C;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE3, &val);
}


/******************************************************************************
 BAST_g2_P_SetAthr_isr()
******************************************************************************/
void BAST_g2_P_SetAthr_isr(BAST_ChannelHandle h)
{
   uint32_t val;

#if 1	/* for SMATV euro mode, do not set ATHR=0x10000000 */
   if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
      val = 0x10000000;
   else
#endif
      val = 0x1B000000;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_ATHR, &val);
}


/******************************************************************************
 BAST_g2_P_Acquire_isr()
******************************************************************************/
BERR_Code BAST_g2_P_Acquire_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t afectl1;

   hChn->acqState = BAST_AcqState_eAcquiring;

   /* check for bypass acquire option */
   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_BYPASS_ACQUIRE)
   {
      hChn->acqState = BAST_AcqState_eIdle;
      return BERR_SUCCESS;
   }
   else
   {
      /* AFECTL1=0x10 if BCM3445 is in path, otherwise 0x30 */
#if 0
      if (hChn->bcm3445TunerInput == BAST_Bcm3445OutputChannel_eNone)
         afectl1 = 0x30;
      else
         afectl1 = 0x10;
#else
      afectl1 = 0x30;
#endif
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AFECTL1, &afectl1);

      if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
      {
         /* BDBG_MSG(("starting legacy qpsk acquisition, mode=%d", hChn->acqParams.mode)); */
         return BAST_g2_P_QpskAcquire_isr(h);
      }
      else if ((BAST_MODE_IS_LDPC(hChn->acqParams.mode)) && hChn->bHasAfec)
      {
         /* BDBG_MSG(("starting LDPC acquisition, mode=%d", hChn->acqParams.mode)); */
         return BAST_g2_P_LdpcAcquire_isr(h);
      }
      else if ((BAST_MODE_IS_TURBO(hChn->acqParams.mode)) && hChn->bHasTfec)
      {
         /* BDBG_MSG(("starting Turbo acquisition, mode=%d", hChn->acqParams.mode)); */
         return BAST_g2_P_TurboAcquire_isr(h);
      }
      else if (hChn->acqParams.mode == BAST_Mode_eBlindScan)
      {
         /* BDBG_MSG(("starting Blind acquisition")); */
         return BAST_g2_P_BlindAcquire_isr(h);
      }
      else
      {
         hChn->acqState = BAST_AcqState_eIdle;
         return BERR_NOT_SUPPORTED;
      }
   }
}


/******************************************************************************
 BAST_g2_P_OkToEnableLoTracking_isrsafe()
******************************************************************************/
bool BAST_g2_P_OkToEnableLoTracking_isrsafe(BAST_ChannelHandle h)
{
   BSTD_UNUSED(h);
   return false;
}


/******************************************************************************
 BAST_g2_P_GetActualMode_isrsafe()
******************************************************************************/
BAST_Mode BAST_g2_P_GetActualMode_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t fmod, vst, i;
   BAST_Mode actual_mode;

   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
   {
      /* Legacy QPSK */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FMOD, &fmod);
      fmod = (fmod >> 8) & 0x0C;
      if (fmod == 0x04)
         actual_mode = BAST_Mode_eDss_1_2; /* DTV */
      else if (fmod == 0x00)
         actual_mode = BAST_Mode_eDvb_1_2; /* DVB */
      else
         actual_mode = BAST_Mode_eDcii_1_2; /* DCII */

      /* determine code rate */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_VST, &vst);
      vst = (vst >> 24) & 0x0F;
      switch (vst)
      {
         case 0x00: /* DVB/DTV 1/2 */
         case 0x09: /* DCII 1/2 */
            i = 0;
            break;
         case 0x01: /* DVB/DTV 2/3 */
         case 0x0B: /* DCII 2/3 */
            i = 1;
            break;
         case 0x02: /* DVB 3/4 */
         case 0x0C: /* DCII 3/4 */
         case 0x04: /* DTV 6/7 */
            i = 2;
            break;
         case 0x03: /* DVB 5/6 */
         case 0x0E: /* DCII 5/6 */
            i = 3;
            break;
         case 0x05: /* DVB 7/8 */
         case 0x0F: /* DCII 7/8 */
            i = 4;
            break;
         case 0x08: /* DCII 5/11 */
            i = 5;
            break;
         case 0x0A: /* DCII 3/5 */
            i = 6;
            break;
         case 0x0D: /* DCII 4/5 */
            i = 7;
            break;
         default:
            i = 0xFF;
            break;
      }
      if (i == 0xFF)
         actual_mode = BAST_Mode_eUnknown;
      else
         actual_mode += i;
   }
   else if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      if (hChn->acqParams.mode == BAST_Mode_eLdpc_scan)
         actual_mode = hChn->actualMode;
      else
         actual_mode = hChn->acqParams.mode;
   }
   else if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
         actual_mode = hChn->actualMode;
      else
         actual_mode = hChn->acqParams.mode;
   }
   else
   {
      BDBG_MSG(("BAST_g2_P_GetActualMode_isrsafe(): cannot get actualMode"));
      actual_mode = hChn->actualMode; /* TBD... */
   }
   return actual_mode;
}


/******************************************************************************
 BAST_g2_P_GetSymbolRateError()
******************************************************************************/
int32_t BAST_g2_P_GetSymbolRateError(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t blpctl2, filtctl, afectl1, cgctrl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;
   int32_t div, bri, bfos, slval1, symbol_rate_error;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BLPCTL2, &blpctl2);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FILTCTL, &filtctl);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AFECTL1, &afectl1);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BRI, (uint32_t *)&bri);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BFOS, (uint32_t *)&bfos);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGCTRL, &cgctrl);

   blpctl2 &= 0x06;
   switch (blpctl2)
   {
      case 0:
         div = 64;
         break;
      case 0x02:
         div = 256;
         break;
      case 0x04:
         div = 1024;
         break;
      default: /* 0x06 */
         div = 4096;
         break;
   }
   if (afectl1 & 0x02)
      div = div >> 1;
   slval1 = (bri / div) + bfos;
   slval1 *= (8 - (filtctl & 0x07));
   val = (uint32_t)slval1;

   BMTH_HILO_32TO64_Mul(hChn->sampleFreq, 16777216, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, val, &Q_hi, &Q_lo);

   if (cgctrl & 0x02)
      Q_lo = Q_lo << 1;

   symbol_rate_error = (int32_t)Q_lo - (int32_t)hChn->acqParams.symbolRate;
   return symbol_rate_error;
}


/******************************************************************************
 BAST_g2_P_GetCarrierError_isrsafe()
******************************************************************************/
int32_t BAST_g2_P_GetCarrierError_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   bool bLoTrackingEnabled = false, bSpinv = false, bFrontLoopEnabled = false, bBackLoopEnabled = false;
   uint32_t val, x, P_hi, P_lo, Q_hi, Q_lo, afectl1, anactl14, anactl10, ovrdsel;
   int32_t carrier_error, slval1, slval2, frf, div, flifOffset, fli = 0, pli = 0;

   hChn->ratio1 = (int32_t)hChn->sampleFreq;
   hChn->ratio2 = (hChn->acqParams.symbolRate + 4) >> 3;

   if (BAST_g2_P_OkToEnableLoTracking_isrsafe(h))
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TUNER_DIGCTL7, &val);
      if (val & 0x08)
         bLoTrackingEnabled = true;
   }

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
   if (val & 0x02)
   {
      /* HP is enabled */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00020000)
         bSpinv = true;
   }

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OVRDSEL, &ovrdsel);
   if ((ovrdsel & 0x00010000) || (hChn->bExternalTuner))
   {
      /* front loop enable is not controlled by HP */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CLCTL, &val);
      if (val & 0x10)
         bFrontLoopEnabled = true;
   }
   else /* front loop enable is controlled by HP */
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00000200)
         bFrontLoopEnabled = true;
   }
   val = 0;
   if (bFrontLoopEnabled)	/* read FLI register if front loop is enable */
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FLI, (uint32_t *)&fli);
      if (fli >= 0)
         val = fli;
      else
         val = -fli;
   }

   if (bLoTrackingEnabled)
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TUNER_ANACTL10, &anactl10);
      anactl10 &= 0x07;

      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TUNER_ANACTL14, &anactl14);
      if ((anactl14 & 0x01) == 0) /* if lo_div == 12 */
         x = 32; /* 10.66667 * 3 */
      else
         x = 24; /* 8.0 * 3 */
      BMTH_HILO_32TO64_Mul(hChn->tunerFddfs * 3, val, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);
      BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, x, &Q_hi, &Q_lo);

      BMTH_HILO_32TO64_Mul(hChn->tunerFddfs * 3, (uint32_t)hChn->ratio1, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, x, &P_hi, (uint32_t*)&(hChn->ratio1));

      if (anactl10 == 6)
      {
         Q_lo = Q_lo >> 1;
         hChn->ratio1 = hChn->ratio1 / 2;
      }
   }
   else
   {
      BMTH_HILO_32TO64_Mul(hChn->sampleFreq, val, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);
   }

   carrier_error = Q_lo;
   if (fli < 0)
      carrier_error = -carrier_error;
   if (bSpinv)
   {
      carrier_error = -carrier_error;
      /* hChn->ratio1 = -(hChn->ratio1); */
   }

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AFECTL1, &afectl1);
   if (afectl1 & 0x02)
      div = 16;
   else
      div = 8;
   carrier_error /= div;
   hChn->ratio1 /= div;
/* BDBG_MSG(("FLI=0x%X, offset(FLI)=%d", fli, carrier_error)); */

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OVRDSEL, &ovrdsel);
   if ((ovrdsel & 0x00080000) || (hChn->bExternalTuner))	/* back loop enable is not controlled by HP */
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CLMISC2, &val);
      if (val & 0x10)
         bBackLoopEnabled = true;
   }
   else /* back loop enable is controlled by HP */
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00000200)
         bBackLoopEnabled = true;
   }
   val = 0;
   if (bBackLoopEnabled) /* read PLI register if back loop is enable */
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_PLI, (uint32_t *)&pli);
      if (pli >= 0)
         val = pli;
      else
         val = -pli;
   }
   BMTH_HILO_32TO64_Mul(val, (hChn->acqParams.symbolRate + 4) >> 3, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1073741824, &Q_hi, &Q_lo);
   hChn->carrierErrorPli = ((pli < 0) ? -Q_lo : Q_lo);
   carrier_error += hChn->carrierErrorPli;
/* BDBG_MSG(("PLI=0x%X, offset(PLI)=%d", pli, hChn->carrierErrorPli)); */

   if (!(BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode)))
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FRF, (uint32_t *)&frf);
      frf = frf / 256;
      if (frf >= 0)
         val = frf;
      else
         val = -frf;
      if (bLoTrackingEnabled)
         BMTH_HILO_32TO64_Mul(val, hChn->tunerFs, &P_hi, &P_lo);
      else
         BMTH_HILO_32TO64_Mul(val, hChn->sampleFreq, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, 16777216, &Q_hi, &Q_lo);

      if (bLoTrackingEnabled && (anactl10 == 6))
         Q_lo = Q_lo >> 1;

      slval1 = Q_lo;
      if (frf < 0)
         slval1 = -slval1;
      if (bSpinv && ((hChn->tunerCtl & BAST_G2_TUNER_CTL_ENABLE_SPLITTER_MODE) == 0))
         slval1 = -slval1;

/* BDBG_MSG(("FRF[31:8]=0x%X, offset(FRF)=%d", frf, slval1));         */
#if 0
      if (slval1 < 0)
         carrier_error -= Q_lo;
      else
         carrier_error += Q_lo;
#else
      carrier_error += slval1;
#endif

      if (hChn->bExternalTuner == false)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FFOFF, (uint32_t *)&slval1);
         slval1 /= 256;
         if (slval1 >= 0)
            val = slval1;
         else
            val = -slval1;
         BMTH_HILO_32TO64_Mul(val, bLoTrackingEnabled ? hChn->tunerFs : hChn->sampleFreq, &P_hi, &P_lo);
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, 16777216, &Q_hi, &Q_lo);

         slval2 = Q_lo;
         if (slval1 < 0)
            slval2 = -slval2;

         if (bLoTrackingEnabled)
         {
            if (anactl10 == 6)
               slval2 /= 2;
         }
         if (bSpinv)
            slval2 = -slval2;
         carrier_error += slval2;
/* BDBG_MSG(("FFOFF[31:8]=0x%X, offset(FFOFF)=%d", slval1, slval2));    */
      }
   }

   BAST_g2_P_GetFlifOffset_isrsafe(h, &flifOffset);
/* BDBG_MSG(("offset(FLIF)=%d", flifOffset));   */
/* BDBG_MSG(("offset(tuner)=%d", hChn->tunerIfStep));  */
   carrier_error += flifOffset;

   if (hChn->bExternalTuner == false)
   {
#ifndef BAST_EXCLUDE_SPLITTER_MODE
      if (hChn->tunerCtl & BAST_G2_TUNER_CTL_ENABLE_SPLITTER_MODE)
      {
         carrier_error -= hChn->splitterModeAdj;
         carrier_error -= hChn->tunerIfStep;
      }
      else
#endif
         carrier_error -= hChn->tunerIfStep;
   }
   return -carrier_error;
}


/******************************************************************************
 BAST_g2_P_GetFlifOffset_isrsafe()
******************************************************************************/
void BAST_g2_P_GetFlifOffset_isrsafe(BAST_ChannelHandle h, int32_t *pOffset)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, flif;
   int32_t offset, slval1;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FLIF, (uint32_t *)&flif);
   if (flif & 0x08000000)
   {
      val = flif & 0x0FFFFFFF;
      slval1 = (int32_t)((int32_t)val - 0x10000000);
      val = -slval1;
   }
   else
      val = flif;
   BMTH_HILO_32TO64_Mul(val, hChn->sampleFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 268435456, &P_hi, &P_lo);
   if (flif & 0x08000000)
      offset = -P_lo;
   else
      offset = P_lo;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
   if (val & 0x02)
   {
      /* HP is enabled */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00020000)
      {
         /* spectrum is inverted */
         offset = -offset;
      }
   }

   /* BDBG_MSG(("BAST_g2_P_GetFlifOffset_isrsafe(): FLIF=0x%08X, offset=%d", flif, offset)); */
   *pOffset = offset;
}


/******************************************************************************
 BAST_g2_P_SetFlifOffset_isr()
******************************************************************************/
void BAST_g2_P_SetFlifOffset_isr(BAST_ChannelHandle h, int32_t offset)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo;
   int32_t flif;

   if (offset >= 0)
      val = offset;
   else
      val = -offset;

   BMTH_HILO_32TO64_Mul(val, 268435456, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &P_hi, &P_lo);
   if (offset >= 0)
      flif = P_lo;
   else
      flif = -P_lo;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
   if (val & 0x02)
   {
      /* HP is enabled */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00020000)
      {
         /* spectrum is inverted */
         flif = -flif;
      }
   }

   val = (uint32_t)flif;
   val &= 0x0FFFFF00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLIF, (uint32_t *)&val);
   /* BDBG_MSG(("BAST_g2_P_SetFlifOffset_isr(): offset=%d, FLIF=0x%08X", offset, val)); */
}


/******************************************************************************
 BAST_g2_P_CarrierOffsetOutOfRange_isr()
******************************************************************************/
bool BAST_g2_P_CarrierOffsetOutOfRange_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   int32_t carrierError;
   uint32_t val, range;
   bool bOutOfRange = false;

   if (hChn->bExternalTuner == false)
   {
      carrierError = BAST_g2_P_GetCarrierError_isrsafe(h);

      if (carrierError < 0)
         val = -carrierError;
      else
         val = carrierError;

      range = (hDev->searchRange * 6) / 5;
      if (val > range)
      {
         bOutOfRange = true;
         BDBG_MSG(("carrier offset out of range (%d Hz)", val));
      }
   }

   return bOutOfRange;
}


/******************************************************************************
 BAST_g2_P_IndicateLocked_isr()
******************************************************************************/
void BAST_g2_P_IndicateLocked_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = true;
   if (hChn->bLastLocked == false)
   {
      BDBG_MSG(("SDS%d locked", h->channel));
      BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = true;
}


/******************************************************************************
 BAST_g2_P_IndicateNotLocked_isrsafe()
******************************************************************************/
void BAST_g2_P_IndicateNotLocked_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->bLocked = false;
   if (hChn->bLastLocked)
   {
      BDBG_MSG(("SDS%d not locked", h->channel));
      BKNI_SetEvent(hChn->hLockChangeEvent);
   }

   hChn->bLastLocked = false;
}


/******************************************************************************
 BAST_g2_P_GetFecFreq_isr()
******************************************************************************/
void BAST_g2_P_GetFecFreq_isr(BAST_ChannelHandle h)
{
#if (BCHP_CHIP==7342)
   BAST_ChannelHandle h0 = (BAST_ChannelHandle)(h->pDevice->pChannels[0]);
#endif
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t val1, val2, data0, P_hi, P_lo, Q_hi, Q_lo;

#if (BCHP_CHIP==7342)
   /* always read SDS0 SPLL_NDIV registers */
   BAST_g2_P_ReadRegister_isrsafe(h0, BCHP_SDS_SPLL_NDIV_INT, &val1);
   BAST_g2_P_ReadRegister_isrsafe(h0, BCHP_SDS_SPLL_NDIV_FRAC, &val2);

#else
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_INT, &val1);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_NDIV_FRAC, &val2);
#endif
   val1 += val2;
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_MPDIV, &val2);
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
      data0 = (val2 >> 24) & 0xFF; /* ldpc */
   else
      data0 = (val2 >> 16) & 0xFF; /* turbo */
   BMTH_HILO_32TO64_Mul(val1 * (val2 & 0x0F), hDev->xtalFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, data0 * ((val2 >> 4) & 0x0F), &P_hi, &P_lo);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGDIV6, &val1);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, (val1 >> 4) & 0x0F, &Q_hi, &Q_lo);
   hChn->fecFreq = Q_lo;
}


/******************************************************************************
 BAST_g2_P_TunerTestMode_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TunerTestMode_isr(BAST_ChannelHandle h)
{
    BSTD_UNUSED(h);
   /* any register settings for tuner test mode goes here... */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_BlindAcquire_isr()
******************************************************************************/
BERR_Code BAST_g2_P_BlindAcquire_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->blindScanStatus = BAST_BlindScanStatus_eAcquire;
   hChn->blindScanCurrMode = 0;
   return BAST_g2_P_BlindAcquire1_isr(h);
}


/******************************************************************************
 BAST_g2_P_BlindReacquire_isr()
******************************************************************************/
BERR_Code BAST_g2_P_BlindReacquire_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->tunerIfStep = 0;
   hChn->tunerOffset = 0;
   hChn->acqState = BAST_AcqState_eTuning;
   hChn->acqParams.carrierFreqOffset = 0;
   return BAST_g2_P_TunerSetFreq_isr(h, BAST_g2_P_BlindAcquire1_isr);
}


/******************************************************************************
 BAST_g2_P_BlindAcquire1_isr()
******************************************************************************/
BERR_Code BAST_g2_P_BlindAcquire1_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   /* determine next mode to try */
   next_mode:
   while (hChn->blindScanModes)
   {
      if (hChn->blindScanCurrMode == 0)
         hChn->blindScanCurrMode = 1;
      else
         hChn->blindScanCurrMode = (hChn->blindScanCurrMode << 1) & BAST_G2_BLIND_SCAN_MODES_MASK;

      if (hChn->blindScanCurrMode == 0)
         hChn->acqCount++;
      else if (hChn->blindScanCurrMode & hChn->blindScanModes)
         break;
   }

   if (hChn->blindScanCurrMode & BAST_G2_BLIND_SCAN_MODES_DVB)
   {
      BDBG_MSG(("Blind scan: trying DVB scan"));
      hChn->acqParams.mode = BAST_Mode_eDvb_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G2_BLIND_SCAN_MODES_DTV)
   {
      BDBG_MSG(("Blind scan: trying DTV scan"));
      hChn->acqParams.mode = BAST_Mode_eDss_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G2_BLIND_SCAN_MODES_DCII)
   {
      BDBG_MSG(("Blind scan: trying DCII scan"));
      hChn->acqParams.mode = BAST_Mode_eDcii_scan;
   }
   else if (hChn->blindScanCurrMode & BAST_G2_BLIND_SCAN_MODES_LDPC)
   {
      if (hChn->bHasAfec)
      {
         BDBG_MSG(("Blind scan: trying LDPC scan"));
         hChn->acqParams.mode = BAST_Mode_eLdpc_scan;
      }
      else
      {
         /* remove LDPC from blind scan because this channel does not have an AFEC */
         hChn->blindScanModes &= ~BAST_G2_BLIND_SCAN_MODES_LDPC;
         goto next_mode;
      }
   }
   else if (hChn->blindScanCurrMode & BAST_G2_BLIND_SCAN_MODES_TURBO)
   {
      if (hChn->bHasTfec)
      {
         BDBG_MSG(("Blind scan: trying Turbo scan"));
         hChn->acqParams.mode = BAST_Mode_eTurbo_scan;
      }
      else
      {
         /* remove Turbo from blind scan because this channel does not have a TFEC */
         hChn->blindScanModes &= ~BAST_G2_BLIND_SCAN_MODES_TURBO;
         goto next_mode;
      }
   }
   else
      return BERR_SUCCESS;

   return BAST_g2_P_Acquire_isr(h);
}


/******************************************************************************
 BAST_g2_P_GetSnrEstimate_isrsafe() - returns SNR in units of 1/256 dB
******************************************************************************/
BERR_Code BAST_g2_P_GetSnrEstimate_isrsafe(BAST_ChannelHandle h, uint32_t *pSnr)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t snre, val1, val2, snre_min, snre_max, snre_scale, snre_idx, x, P_hi, P_lo, Q_hi, Q_lo;
   const uint16_t *snre_table;
   uint16_t snre_adj;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SNRE, &snre);

   if ((hChn->actualMode == BAST_Mode_eBlindScan) || (hChn->actualMode == BAST_Mode_eUnknown))
   {
      *pSnr = 0;
      goto done;
   }
   else if (BAST_MODE_IS_LEGACY_QPSK(hChn->actualMode))
   {
      /* Legacy QPSK */
      snre_table = BAST_snr_legacy_qpsk_table;
      snre_min = BAST_snr_legacy_qpsk_min;
      snre_max = BAST_snr_legacy_qpsk_max;
      snre_scale = BAST_snr_legacy_qpsk_scale;
   }
   else if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      /* Turbo 8PSK */
      snre_table = BAST_snr_ldpc_8psk_table;
      snre_min = BAST_snr_turbo_8psk_min;
      snre_max = BAST_snr_turbo_8psk_max;
      snre_scale = BAST_snr_turbo_8psk_scale;
   }
   else if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
   {
      /* LDPC 8PSK */
      snre_table = BAST_snr_ldpc_8psk_table;
      snre_min = BAST_snr_ldpc_8psk_min;
      snre_max = BAST_snr_ldpc_8psk_max;
      snre_scale = BAST_snr_ldpc_8psk_scale;
   }
   else
   {
      /* LDPC/Turbo QPSK */
      snre_table = BAST_snr_ldpc_qpsk_table;
      snre_min = BAST_snr_ldpc_qpsk_min;
      snre_max = BAST_snr_ldpc_qpsk_max;
      snre_scale = BAST_snr_ldpc_qpsk_scale;
   }

   if ((snre >= snre_min) && (snre <= snre_max))
   {
      BMTH_HILO_32TO64_Mul(snre - snre_min, 65536, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, snre_scale, &Q_hi, &Q_lo);
      if (Q_lo > 0x0000FFFF)
         snre_adj = 0xFFFF;
      else
         snre_adj = (uint16_t)Q_lo;

      /* find snre_idx such that (snre_adj >= snre_table[snre_idx]) && (snre_adj < snre_table[snre_idx+1]) */
      /* use binary search */
      snre_idx = BAST_SNRE_TABLE_N >> 1;
      x = BAST_SNRE_TABLE_N >> 2;
      while (!((snre_adj >= snre_table[snre_idx]) && (snre_adj <= snre_table[snre_idx+1])))
      {
         if (snre_adj < snre_table[snre_idx])
            snre_idx -= x;
         else
            snre_idx += x;
         if (x > 1)
            x = x >> 1;
      }

      /* do linear interpolation */
      val1 = BAST_snr_quant * (BAST_SNRE_TABLE_N - snre_idx);     /* in 2^-8 dB SNR */
      val2 = BAST_snr_quant * (BAST_SNRE_TABLE_N - snre_idx - 1); /* in 2^-8 dB SNR */
      *pSnr = val1 - (((((snre_adj - snre_table[snre_idx]) * (val1 - val2) * 2) / (snre_table[snre_idx+1] - snre_table[snre_idx])) + 1) >> 1);

      if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_4)
         *pSnr -= 677; /* 2.646*256 */
      else if (hChn->actualMode == BAST_Mode_eLdpc_8psk_5_6) /* 8PSK 5/6 is adjusted */
         *pSnr -= 307; /* 1.200*256 */
   }
   else if (snre > snre_max)
      *pSnr = 0;
   else /* (snre < snre_min) */
      *pSnr = BAST_SNR_MAX;

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_FreezeLoops_isr()
******************************************************************************/
void BAST_g2_P_FreezeLoops_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AGTCTL, &hChn->agtctl);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_AGICTL, &hChn->agictl);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OVRDSEL, &hChn->ovrdselSave);

   /* freeze AGC */
   val = hChn->agtctl | 0x02;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGTCTL, &val);
   val = hChn->agictl | 0x02;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGICTL, &val);

   BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_OVRDSEL, 0xD0DA0000);

   /* freeze baud loop */
   BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_BLPCTL1, 0x10);

   /* freeze carrier loop */
   BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_CLMISC, 0x07);

   /* freeze equalizer */
   val = 0x07;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE2, &val);
}


/******************************************************************************
 BAST_g2_P_UnfreezeLoops_isr() - ISR context
******************************************************************************/
void BAST_g2_P_UnfreezeLoops_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGTCTL, &(hChn->agtctl));
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGICTL, &(hChn->agictl));
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OVRDSEL, &(hChn->ovrdselSave));

   BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_BLPCTL1, 0xEF);
   BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_CLMISC, 0xF9);
   val = 0x02;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE2, &val);

   BAST_g2_P_3StageAgc_isr(h, 0);
}


/******************************************************************************
 BAST_g2_P_ResetAcquisitionTimer_isr() - ISR context
******************************************************************************/
void BAST_g2_P_ResetAcquisitionTimer_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, bcktmr;

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eXtal);

   hChn->acqTimeState = 1;
   hChn->acqTime = 0;

   /* start XTAL TIMER */
   val = 0xFFFFFFFF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_XTLTMR, &val);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BCKTMR, &bcktmr);
   bcktmr |= 0x20;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BCKTMR, &bcktmr);
}


/******************************************************************************
 BAST_g2_P_UpdateAcquisitionTime_isr()
******************************************************************************/
void BAST_g2_P_UpdateAcquisitionTime_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t lval1;

   hChn->acqTimeState = 2;
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_XTLTMR, &lval1);

   hChn->acqTime = 0xFFFFFFFF - lval1;
   if (hDev->xtalFreq == 54000000UL)
      hChn->acqTime = hChn->acqTime >> 1;
/* BDBG_MSG(("UpdateAcquisitionTime(): lval1=0x%X, acqTime=%d", lval1, hChn->acqTime)); */
}


/******************************************************************************
 BAST_g2_P_TransferFreqOffset_isr()
******************************************************************************/
void BAST_g2_P_TransferFreqOffset_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   int32_t pli, fli, slval2, slval3, x, carrier_error;
   uint32_t P_hi, P_lo, val;

   if (hChn->reacqCtl & BAST_G2_REACQ_CTL_FREQ_DRIFT)
   {
      if (BAST_g2_P_CarrierOffsetOutOfRange_isr(h))
      {
         hChn->bForceReacq = true;
         return;
      }
   }

   /* check if carrier_error exceeds BAST_DCO_OFFSET */
   if (hChn->tunerOffset)
   {
	   carrier_error = BAST_g2_P_GetCarrierError_isrsafe(h);
	   if (carrier_error >= 0)
	      val = carrier_error;
	   else
	      val = -carrier_error;
	   if (val < BAST_DCO_OFFSET_THRESHOLD)
	      BAST_g2_P_TunerRampMb2_isr(h, (carrier_error <= 0 ? false : true), BAST_DCO_OFFSET + val);
   }

   if ((BAST_MODE_IS_LDPC(hChn->actualMode)) && BAST_g2_P_LdpcIsPilotOn_isr(h))
      return;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_PLI, (uint32_t*)&pli);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FLI, (uint32_t*)&fli);

   if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      slval3 = 32768;
      slval2 = 181;
   }
   else if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_5_6)
   {
      /* LDPC 8PSK 5/6, 8/9, and 9/10 */
      slval3 = 16384;
      slval2 = 128;
   }
   else if ((hChn->actualMode >= BAST_Mode_eLdpc_Qpsk_2_3) && (hChn->actualMode <= BAST_Mode_eLdpc_Qpsk_9_10))
   {
      /* LDPC QPSK 2/3, 3/4, 4/5, 5/6, 8/9, and 9/10 */
      slval3 = 8192;
      slval2 = 91;
   }
   else if (BAST_MODE_IS_LEGACY_QPSK(hChn->actualMode))
   {
      /* legacy QPSK */
      slval3 = 16384;
      slval2 = 128;
   }
   else
   {
      /* LDPC 8PSK 3/5, 2/3, 3/4 and QPSK 1/2, 3/5 */
      slval3 = 65536;
      slval2 = 256;
   }

   hChn->freqTransferInt += (pli / slval3);
   slval2 = (pli / slval2) + hChn->freqTransferInt;

   if (slval2 < 0)
      val = -slval2;
   else
      val = slval2;

   BMTH_HILO_32TO64_Mul(val, hChn->ratio2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->ratio1, &P_hi, &P_lo);
   if (slval2 >= 0)
      x = (int32_t)P_lo;
   else
      x = (int32_t)-P_lo;
   fli += x;

   if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, (uint32_t*)&val);
      if (val & 0x20000)
      {
         /* turbo spectral inversion */
         pli -= slval2;
      }
      else
      {
         /* turbo normal spectrum */
         pli += slval2;
      }
   }
   else
      pli -= slval2;

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLI, (uint32_t*)&pli);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLI, (uint32_t*)&fli);
}


#ifndef BAST_EXCLUDE_STATUS_EVENTS
/******************************************************************************
 BAST_g2_P_CheckStatusIndicators_isr()
******************************************************************************/
void BAST_g2_P_CheckStatusIndicators_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   bool bSetEvent = false;
   int32_t offset;
   uint32_t diff, snr, snr24, val, snrThreshold1, snrThreshold2;
   int idx;

   static const uint16_t BAST_SNR_THRESHOLD1[] = /* in units of 1/256 dB */
   {
      897,  /* 0: DVB 1/2 */
      1353, /* 1: DVB 2/3 */
      1607, /* 2: DVB 3/4 */
      1857, /* 3: DVB 5/6 */
      2055, /* 4: DVB 7/8 */
      15 * 256, /* 5: LDPC QPSK 1/2 */
      15 * 256, /* 6: LDPC QPSK 3/5 */
      15 * 256, /* 7: LDPC QPSK 2/3 */
      15 * 256, /* 8: LDPC QPSK 3/4 */
      15 * 256, /* 9: LDPC QPSK 4/5 */
      15 * 256, /* 10: LDPC QPSK 5/6 */
      15 * 256, /* 11: LDPC QPSK 8/9 */
      15 * 256, /* 12: LDPC QPSK 9/10 */
      15 * 256, /* 13: LDPC 8PSK 3/5 */
      15 * 256, /* 14: LDPC 8PSK 2/3 */
      15 * 256, /* 15: LDPC 8PSK 3/4 */
      15 * 256, /* 16: LDPC 8PSK 5/6 */
      15 * 256, /* 17: LDPC 8PSK 8/9 */
      15 * 256, /* 18: LDPC 8PSK 9/10 */
      753,  /* 19: Turbo QPSK 1/2 */
      1223, /* 20: Turbo QPSK 2/3 */
      1482, /* 21: Turbo QPSK 3/4 */
      1757, /* 22: Turbo QPSK 5/6 */
      1932, /* 23: Turbo QPSK 7/8 */
      2127, /* 24: Turbo 8PSK 2/3 */
      2325, /* 25: Turbo 8PSK 3/4 */
      2446, /* 26: Turbo 8PSK 4/5 */
      2822, /* 27: Turbo 8PSK 5/6 */
      2952  /* 28: Turbo 8PSK 8/9 */
   };

   static const uint16_t BAST_SNR_THRESHOLD2[] = /* in units of 1/256 dB */
   {
      659,  /* 0: DVB 1/2 */
      1097, /* 1: DVB 2/3 */
      1353, /* 2: DVB 3/4 */
      1607, /* 3: DVB 5/6 */
      1807, /* 4: DVB 7/8 */
      10 * 256, /* 5: LDPC QPSK 1/2 */
      10 * 256, /* 6: LDPC QPSK 3/5 */
      10 * 256, /* 7: LDPC QPSK 2/3 */
      10 * 256, /* 8: LDPC QPSK 3/4 */
      10 * 256, /* 9: LDPC QPSK 4/5 */
      10 * 256, /* 10: LDPC QPSK 5/6 */
      10 * 256, /* 11: LDPC QPSK 8/9 */
      10 * 256, /* 12: LDPC QPSK 9/10 */
      10 * 256, /* 13: LDPC 8PSK 3/5 */
      10 * 256, /* 14: LDPC 8PSK 2/3 */
      10 * 256, /* 15: LDPC 8PSK 3/4 */
      10 * 256, /* 16: LDPC 8PSK 5/6 */
      10 * 256, /* 17: LDPC 8PSK 8/9 */
      10 * 256, /* 18: LDPC 8PSK 9/10 */
      525,  /* 19: Turbo QPSK 1/2 */
      972,  /* 20: Turbo QPSK 2/3 */
      1223, /* 21: Turbo QPSK 3/4 */
      1507, /* 22: Turbo QPSK 5/6 */
      1682, /* 23: Turbo QPSK 7/8 */
      1882, /* 24: Turbo 8PSK 2/3 */
      2079, /* 25: Turbo 8PSK 3/4 */
      2200, /* 26: Turbo 8PSK 4/5 */
      2568, /* 27: Turbo 8PSK 5/6 */
      2693  /* 28: Turbo 8PSK 8/9 */
   };

   if (hChn->bStatusInterrupts == false)
      return;

   /* check freq drift threshold */
   offset = BAST_g2_P_GetCarrierError_isrsafe(h);
   if (offset > hChn->initFreqOffset)
      diff = (uint32_t)(offset - hChn->initFreqOffset);
   else
      diff = (uint32_t)(hChn->initFreqOffset - offset);
   if (diff > hChn->freqDriftThreshold)
   {
      if ((hChn->statusEventIndicators & BAST_STATUS_EVENT_FREQ_DRIFT) == 0)
      {
         /* BDBG_MSG(("ch %d freq drift detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_STATUS_EVENT_FREQ_DRIFT;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_STATUS_EVENT_FREQ_DRIFT;

   BAST_g2_P_GetSnrEstimate_isrsafe(h, &snr); /* snr is in 1/256 dB */

   /* determine threshold 1 and 2 values from mode */
   if (BAST_MODE_IS_DVB(hChn->actualMode))
   {
      idx = hChn->actualMode - BAST_Mode_eDvb_1_2;
   }
   else if (BAST_MODE_IS_LDPC(hChn->actualMode))
   {
      idx = 5 + (hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2);
   }
   else if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      idx = 19 + (hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2);
   }
   else
   {
      snrThreshold1 = snrThreshold2 = 0;
      goto check_snr_threshold;
   }

   /* BDBG_MSG(("snrThreshold_idx=%d (actualMode=%d)", idx, hChn->actualMode)); */
   snrThreshold1 = BAST_SNR_THRESHOLD1[idx];
   snrThreshold2 = BAST_SNR_THRESHOLD2[idx];

   hChn->currentCnrThreshold1 = snrThreshold1;

   /* check SNR threshold 1 */
   check_snr_threshold:
   if (snr < snrThreshold1)
   {
      if ((hChn->statusEventIndicators & BAST_STATUS_EVENT_THRESHOLD1) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold1 detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_STATUS_EVENT_THRESHOLD1;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_STATUS_EVENT_THRESHOLD1;

   /* check SNR threshold 2 */
   if (snr < snrThreshold2)
   {
      if ((hChn->statusEventIndicators & BAST_STATUS_EVENT_THRESHOLD2) == 0)
      {
         /* BDBG_MSG(("ch %d CNR threshold2 detected", h->channel)); */
         hChn->statusEventIndicators |= BAST_STATUS_EVENT_THRESHOLD2;
         bSetEvent = true;
      }
   }
   else
      hChn->statusEventIndicators &= ~BAST_STATUS_EVENT_THRESHOLD2;

   /* check for rain fade */
   snr24 = snr << 16; /* snr24=snr in 1/2^24 dB */
   if (hChn->rainFadeSnrAve == 0)
   {
      hChn->rainFadeSnrAve = snr24;
      hChn->rainFadeSnrMax = snr24;
   }
   else
   {
      /* compute ave snr in window */
      if (snr24 > hChn->rainFadeSnrAve)
         val = snr24 - hChn->rainFadeSnrAve;
      else
         val = hChn->rainFadeSnrAve - snr24;
      val = val / hChn->rainFadeWindow;
      if (snr24 > hChn->rainFadeSnrAve)
         hChn->rainFadeSnrAve += val;
      else
         hChn->rainFadeSnrAve -= val;

      if (snr24 > hChn->rainFadeSnrMax)
         hChn->rainFadeSnrMax = snr24; /* set new max */
      else
         hChn->rainFadeSnrMax -= ((hChn->rainFadeSnrMax - hChn->rainFadeSnrAve) / hChn->rainFadeWindow); /* decay max snr */

      if (snr24 < (hChn->rainFadeSnrMax - hChn->rainFadeThreshold))
      {
         if ((hChn->statusEventIndicators & BAST_STATUS_EVENT_RAIN_FADE) == 0)
         {
            hChn->statusEventIndicators |= BAST_STATUS_EVENT_RAIN_FADE;
            bSetEvent = true;
         }
      }
      else
         hChn->statusEventIndicators &= ~BAST_STATUS_EVENT_RAIN_FADE;
   }
   /* BDBG_MSG(("rain_fade: ave=0x%08X, max=0x%08X, curr=0x%08X", hChn->rainFadeSnrAve, hChn->rainFadeSnrMax, snr24)); */

   if (bSetEvent)
   {
      /* BDBG_MSG(("set status event")); */
      BKNI_SetEvent(hChn->hStatusEvent);
   }
}


/******************************************************************************
 BAST_g2_P_ResetStatusIndicators_isrsafe()
******************************************************************************/
void BAST_g2_P_ResetStatusIndicators_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->statusEventIndicators = 0;
   hChn->rainFadeSnrAve = 0;
   hChn->rainFadeSnrMax = 0;
}
#endif


/******************************************************************************
 BAST_g2_P_IsLdpcOn_isrsafe()
******************************************************************************/
bool BAST_g2_P_IsLdpcOn_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (!hChn->bHasAfec)
      return false;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_PWRDN, &val);
   return ((val & 0x10) == 0) ? true : false;
}


/******************************************************************************
 BAST_g2_P_IsTurboOn_isrsafe()
******************************************************************************/
bool BAST_g2_P_IsTurboOn_isrsafe(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (!hChn->bHasTfec)
      return false;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_SPLL_PWRDN, &val);
   return ((val & 0x08) == 0) ? true : false;
}


/******************************************************************************
 BAST_g2_P_UpdateErrorCounters()
******************************************************************************/
void BAST_g2_P_UpdateErrorCounters(BAST_ChannelHandle h)
{
   BKNI_EnterCriticalSection();
   BAST_g2_P_UpdateErrorCounters_isr(h);
   BKNI_LeaveCriticalSection();
}


/******************************************************************************
 BAST_g2_P_UpdateErrorCounters_isr()
******************************************************************************/
void BAST_g2_P_UpdateErrorCounters_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, frc, ferc, oifctl4;

   /* take snapshot of mpeg frame and error counter */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_OIFCTL4, &oifctl4);
   val = oifctl4 | 0x40;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OIFCTL4, &val);

   frc = 0;
   val = 0;
   while (frc == 0)
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FRC, &frc);
      if (frc == 0)
      {
         if ((hChn->bLocked == false) || (val > 100))
            break;
      }
      val++;
   }

   hChn->mpegFrameCount += frc;
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FERC, &ferc);
   hChn->mpegErrorCount += ferc;

   /* reset counters */
   val = oifctl4 | 0x20;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OIFCTL4, &val);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BERC, &val);
   hChn->berErrors += val;
}


/******************************************************************************
 BAST_g2_P_ResetErrorCounters()
******************************************************************************/
void BAST_g2_P_ResetErrorCounters(BAST_ChannelHandle h)
{
   BKNI_EnterCriticalSection();
   BAST_g2_P_ResetErrorCounters_isr(h);
   BKNI_LeaveCriticalSection();
}


/******************************************************************************
 BAST_g2_P_ResetErrorCounters_isr()
******************************************************************************/
void BAST_g2_P_ResetErrorCounters_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   BAST_g2_P_UpdateErrorCounters_isr(h);
   BAST_g2_P_QpskUpdateErrorCounters_isr(h);

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      if (BAST_g2_P_IsLdpcOn_isrsafe(h))
         BAST_g2_P_LdpcUpdateBlockCounters_isr(h);
      else
      {
         BDBG_MSG(("BAST_g2_P_ResetStatus(): AFEC powered off"));
      }
   }
#ifndef BAST_EXCLUDE_TURBO
   else if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      if (BAST_g2_P_IsTurboOn_isrsafe(h))
      {
         BAST_g2_P_TurboUpdateErrorCounters_isr(h);
      }
      else
      {
         BDBG_MSG(("BAST_g2_P_ResetStatus(): TFEC powered off"));
      }
   }
#endif

   hChn->berErrors = 0;
   hChn->mpegErrorCount = 0;
   hChn->mpegFrameCount = 0;
   hChn->ldpcBadBlocks = 0;
   hChn->ldpcCorrBlocks = 0;
   hChn->ldpcTotalBlocks = 0;
   hChn->rsCorr = 0;
   hChn->rsUncorr = 0;
   hChn->preVitErrors = 0;
   hChn->turboBadBlocks = 0;
   hChn->turboCorrBlocks = 0;
   hChn->turboTotalBlocks = 0;
}


/******************************************************************************
 BAST_g2_P_KillTransport()
******************************************************************************/
BERR_Code BAST_g2_P_KillTransport(BAST_ChannelHandle h)
{
   static const uint32_t script_reset_channel[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS5, 0x00003C00), /* disable 3-stage agc interrupts */
      BAST_SCRIPT_WRITE(BCHP_SDS_OIFCTL2, 0x08),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x51),         /* reset and disable HP */
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL2, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FECTL, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FECTL, 0x00),
      BAST_SCRIPT_OR(BCHP_SDS_PSCTL, 0x01), /* reset oif */
      BAST_SCRIPT_AND(BCHP_SDS_PSCTL, ~0x01), /* reset oif */
      BAST_SCRIPT_EXIT
   };

   return BAST_g2_P_ProcessScript_isrsafe(h, script_reset_channel);

}


/******************************************************************************
 BAST_g2_P_IncrementInterruptCounter_isr()
******************************************************************************/
void BAST_g2_P_IncrementInterruptCounter_isr(BAST_ChannelHandle h, BAST_Sds_IntID idx)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->irqCount[idx]++;
}


/******************************************************************************
 BAST_g2_P_SetBackCarrierBw_isr() - damp is scaled 2^2, bw is in Hz
******************************************************************************/
void BAST_g2_P_SetBackCarrierBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t i, j, b0, b1, b2, b3, val, P_hi, P_lo, Q_hi, Q_lo;
   bool ovr;

   /* compute 6-bit linear coefficient */
   for (i = 0; i < 7; i++)
   {
      b0 = (1 << (i << 1)) * (damp >> 1) * bw;
      if (b0 > hChn->acqParams.symbolRate)
         break;
   }
   i--;
   BMTH_HILO_32TO64_Mul((1 << (i << 1)) * damp, bw << 6, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   b0 = (Q_lo + 2) >> 2;
   if (b0 & 0xFFFFFFE0)
      b0 = 0x1F;
   b1 = 7 - i;

   /* compute 6-bit integrator coefficient */
   if (bw > 65535)
      ovr = true;
   else
      ovr = false;
   for (j = 0; j < 14; j++)
   {
      if (ovr)
         BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 1, bw, &P_hi, &P_lo);
      else
         BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 1, bw * bw, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);

      if (ovr)
         Q_lo *= bw;
      Q_lo = (Q_lo + 1) >> 1; /* round */
      if (Q_lo > hChn->acqParams.symbolRate)
         break;
   }
   j--;
   if (ovr)
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, bw << 7, &P_hi, &P_lo);
   else
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, (bw * bw), &P_hi, &P_lo);

   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   if (ovr)
   {
      Q_lo = (Q_lo + 1) >> 1;
      BMTH_HILO_32TO64_Mul(Q_lo, bw << 2, &P_hi, &P_lo);
   }
   else
      BMTH_HILO_32TO64_Mul(Q_lo, 1 << 8, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   b2 = (Q_lo + 128) >> 8;
   if (b2 & 0xFFFFFFE0)
      b2 = 0x1F;
   b3 = 13 - j;

   val = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC1, &val);
#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FBLC, &val);
#endif
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate_isr(BAST_ChannelHandle h, BAST_g2_FUNCT funct)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->bFsNotDefault = false;
   hChn->passFunct = funct; /* passFunct = function to call after DFT freq estimate */

   hChn->dftFreqEstimate = 0;
   hChn->dftFreqEstimateStatus = BAST_FREQ_ESTIMATE_STATUS_START;

   if ((hChn->bExternalTuner) ||
       (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_OQPSK))
      return BAST_g2_P_DoDftFreqEstimateExit_isr(h);

   if (hChn->acqParams.carrierFreqOffset)
   {
      if (hChn->acqCount == 0)
      {
         /* warm start */
         hChn->tunerIfStep = hChn->acqParams.carrierFreqOffset;
         return BAST_g2_P_DoDftFreqEstimate7_isr(h);
      }
      else
         hChn->acqParams.carrierFreqOffset = 0; /* warm start failed previously, so do cold start */
   }

   /* turn off 3-stage AGC while doing DFT */
   BAST_g2_P_3StageAgc_isr((void*)h, 0);

   return BAST_g2_P_DoDftFreqEstimate1_isr(h);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate1_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate1_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t ratio, p1, p2, m1div, cgdiv0;

   /* wait until diseqc completes before changing Fs */
   if (hChn->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
      return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 500, BAST_g2_P_DoDftFreqEstimate1_isr);
   else
   {
      ratio = (hChn->sampleFreq * 16) / hChn->acqParams.symbolRate;
      if ((ratio < 205) && (hChn->sampleFreq == BAST_DEFAULT_SAMPLE_FREQ))
         goto dft_0;

      /* change sample freq */
#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
      BDBG_MSG(("DFT: changing Fs"));
#endif
      hChn->dftFreqEstimateStatus |= BAST_FREQ_ESTIMATE_STATUS_FS;
      hChn->bFsNotDefault = true;
      cgdiv0 = 1;
      if (ratio < 205) /* 12.8 * 16 */
      {
         /* Fs */
         m1div = hDev->m1div;
      }
      else if (ratio < 410)  /* 25.6 * 16 */
      {
         /* Fs/2 */
         m1div = hDev->m1div * 2;
      }
      else if (ratio < 819) /* 51.2 * 16 */
      {
         /* Fs/4 */
         m1div = hDev->m1div * 4;
      }
      else if (ratio < 1638) /* 102.4 * 16 */
      {
         /* Fs/8 */
         m1div = hDev->m1div * 3;
         cgdiv0 = 2;
      }
      else if (ratio < 3277) /* 204.8 * 16 */
      {
         /* Fs/16 */
         m1div = hDev->m1div * 3;
         cgdiv0 = 2; /* cgdiv0 = 4; */
      }
      else
      {
         /* Fs/32 */
         m1div = hDev->m1div * 3;
         cgdiv0 = 2; /* cgdiv0 = 8; */
      }

      p1 = (hDev->xtalFreq == 54000000 ? 2 : 1);
      p2 = 1;
      BAST_g2_P_SetSampleFreq_isrsafe(h, p1, p2, hDev->ndiv, m1div, hDev->m2div, hDev->m3div);

      if (cgdiv0 != 0x01)
      {
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CGDIV0, &cgdiv0);
         BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CGMISC, 0xFF, 0x20);
         BAST_g2_P_GetSampleFreq_isrsafe(h);
      }
      BAST_g2_P_SetDiseqcClock_isrsafe(h);

      dft_0:
      return BAST_g2_P_DoDftFreqEstimate2_isr(h);
   }
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate2_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate2_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t Fb;

   /* coarse freq step is 10% of Fb */
   hChn->dftFreqEstimateStatus |= BAST_FREQ_ESTIMATE_STATUS_COARSE;
   /* limit IF step size */
   Fb = hChn->acqParams.symbolRate;
   if (BAST_MODE_IS_LEGACY_QPSK(hChn->acqParams.mode))
   {
      if (Fb > 20000000UL)
         Fb = 20000000UL;
   }
   else if (Fb > 30000000UL)
      Fb = 30000000UL;

   hChn->tunerIfStepSize = Fb / 10;
   hChn->tunerIfStep = (int32_t)(-(hDev->searchRange) - hChn->tunerIfStepSize);
   hChn->tunerIfStepMax = (int32_t)(hDev->searchRange);

   if (hChn->tunerIfStepSize > hDev->searchRange)
      return BAST_g2_P_DoDftFreqEstimate8_isr(h);

   hChn->count1 = 0; /* count1 = current max */

#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
   BDBG_MSG(("DFT(coarse): StepSize=%d, Fs=%d", hChn->tunerIfStepSize, hChn->sampleFreq));
#endif

   return BAST_g2_P_DoDftFreqEstimate3_isr(h);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate3_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate3_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->tunerIfStep > hChn->tunerIfStepMax)
   {
      if ((hChn->dftFreqEstimateStatus & BAST_FREQ_ESTIMATE_STATUS_FINE) == 0)
      {
#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
         BDBG_MSG(("DFT(coarse): select %d Hz", hChn->dftFreqEstimate));
#endif
         hChn->tunerIfStep = hChn->dftFreqEstimate - (int32_t)(hChn->tunerIfStepSize);
         if (hChn->tunerIfStep < (int32_t)-(hDev->searchRange))
            hChn->tunerIfStep = (int32_t)-(hDev->searchRange);
         hChn->tunerIfStepMax = hChn->dftFreqEstimate + (int32_t)(hChn->tunerIfStepSize);
         if (hChn->tunerIfStepMax > (int32_t)(hDev->searchRange))
            hChn->tunerIfStepMax = (int32_t)(hDev->searchRange);

         return BAST_g2_P_DoDftFreqEstimate8_isr(h);
      }

      hChn->dftFreqEstimateStatus |= BAST_FREQ_ESTIMATE_STATUS_DONE;
      hChn->tunerIfStep = hChn->dftFreqEstimate;
      return BAST_g2_P_DoDftFreqEstimate7_isr(h);
   }
   hChn->tunerIfStep += hChn->tunerIfStepSize;
   return BAST_g2_P_TunerQuickTune_isr(h, BAST_g2_P_DoDftFreqEstimate9_isr);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate8_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate8_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, Fb;

   /* reset baud loop integrator */
   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRI, &val);

   hChn->dftFreqEstimateStatus |= BAST_FREQ_ESTIMATE_STATUS_FINE;
   Fb = hChn->acqParams.symbolRate;
   if (Fb > 30000000UL)
      Fb = 30000000UL;
   hChn->tunerIfStepSize = Fb / 20;
   if (hChn->tunerIfStepSize > 1000000)
      hChn->tunerIfStepSize = 1000000;
   hChn->tunerIfStep -= hChn->tunerIfStepSize;

#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
   BDBG_MSG(("DFT(fine): StepSize=%d, Fs=%d", hChn->tunerIfStepSize, hChn->sampleFreq));
#endif

   hChn->count1 = 0; /* count1 = current max */
   return BAST_g2_P_DoDftFreqEstimate3_isr(h);
}


/******************************************************************************
 BAST_g2_P_SetDftDdfsFcw_isr()
******************************************************************************/
void BAST_g2_P_SetDftDdfsFcw_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   val = 0x97;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL1, &val);

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, 32768, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_DDFS_FCW, &Q_lo);
}


/******************************************************************************
 BAST_g2_P_DftStartAndWaitForDone_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DftStartAndWaitForDone_isr(BAST_ChannelHandle h, BAST_g2_FUNCT funct)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   hChn->failFunct = funct;

   BAST_g2_P_SetDftDdfsFcw_isr(h);

   /* start the DFT engine */
   val = 1;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);

   /* time it takes for DFT to run is 32768 Fs cycles */
   BMTH_HILO_32TO64_Mul(32768, 2000000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo = (Q_lo + 1) >> 1;

   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBer, Q_lo, BAST_g2_P_DftWaitForDone_isr);
}


/******************************************************************************
 BAST_g2_P_DftWaitForDone_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DftWaitForDone_isr(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_CTRL0, &val);
   if ((val & 0xE0000000) == 0xC0000000)
      return BAST_g2_P_DftGetPower_isr(h);

#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
   BDBG_MSG(("DFT not yet ready.  Waiting..."));
#endif
   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBer, 20, BAST_g2_P_DftWaitForDone_isr);
}


/******************************************************************************
 BAST_g2_P_DftGetPower_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DftGetPower_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t peak_pow, total_pow;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_PEAK_POW, &peak_pow);
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_DFT_TOTAL_POW, &total_pow);

   if (peak_pow > hChn->maxCount1)
      hChn->maxCount1 = peak_pow;
   hChn->count2 += total_pow;
   return hChn->failFunct(h);
}

/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate9_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate9_isr(BAST_ChannelHandle h)
{
   /* wait for tuner to settle? */
   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBer, 250, BAST_g2_P_DoDftFreqEstimate4_isr);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate4_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate4_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   hChn->count2 = 0; /* count2 = sum of total_pow */
   hChn->maxCount1 = 0; /* maxCount1 = max peak_pow */

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
   val = 0x0F;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

   return BAST_g2_P_DftStartAndWaitForDone_isr(h, BAST_g2_P_DoDftFreqEstimate5_isr);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate5_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate5_isr(BAST_ChannelHandle h)
{
   uint32_t val;

   val = 0x7F0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_START, &val);
   val = 0x7FF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DFT_RANGE_END, &val);

   return BAST_g2_P_DftStartAndWaitForDone_isr(h, BAST_g2_P_DoDftFreqEstimate6_isr);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate6_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate6_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = hChn->maxCount1 - ((hChn->count2 - hChn->maxCount1) >> 5);
#ifdef BAST_DEBUG_DFT_FREQ_ESTIMATE
   BDBG_MSG(("%9d Hz:pow=%u", hChn->tunerIfStep, val));
#endif
   if (val > hChn->count1)
   {
      hChn->count1 = val;
      hChn->dftFreqEstimate = hChn->tunerIfStep;
   }

   /* hChn->tunerIfStep += hChn->tunerIfStepSize; */
   return BAST_g2_P_DoDftFreqEstimate3_isr(h);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimate7_isr() - set the final freq offset
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimate7_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->bFsNotDefault)
   {
      /* wait until diseqc completes before changing Fs */
      if (hChn->diseqcStatus.status == BAST_DiseqcSendStatus_eBusy)
         return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBer, 500, BAST_g2_P_DoDftFreqEstimate7_isr);
      else
         BAST_g2_P_SetDefaultSampleFreq_isrsafe(h);
   }

#if 1 /*def BAST_DEBUG_DFT_FREQ_ESTIMATE */
   BDBG_MSG(("DFT: select %d Hz", hChn->tunerIfStep));
#endif

   return BAST_g2_P_TunerQuickTune_isr(h, BAST_g2_P_DoDftFreqEstimateExit_isr);
}


/******************************************************************************
 BAST_g2_P_DoDftFreqEstimateExit_isr()
******************************************************************************/
BERR_Code BAST_g2_P_DoDftFreqEstimateExit_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   hChn->dftFreqEstimateStatus |= 0x80;
   BAST_g2_P_Enable3StageAgc_isr(h, true); /* re-enable 3-stage AGC */

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRI, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLI, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLI, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLIF, &val);

   return hChn->passFunct(h);
}


