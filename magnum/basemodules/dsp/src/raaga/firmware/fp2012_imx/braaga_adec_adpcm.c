/*******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bchp.h"
const uint32_t BDSP_IMG_adpcm_decode_array1[] = {
	0x0bbca1cf,
	0x6abd31cf,
	0xfc1c9517,
	0xe7a18117,
	0x6d739c30,
	0x71a72bf0,
	0x3c1c9927,
	0xfc1ca147,
	0x6370044f,
	0x6fb398a2,
	0x53e431d7,
	0x50fbf549,
	0xc8801107,
	0x50d1ffff,
	0xf1c00327,
	0xd403480f,
	0x50d1ffff,
	0xf17f8217,
	0x53c271d7,
	0x50fbf54d,
	0x27bffe17,
	0xc188412f,
	0x51007e07,
	0x90102b58,
	0x51107e0f,
	0x90102b7d,
	0x53c271d7,
	0x50fbf550,
	0x08110107,
	0x38109107,
	0x08138907,
	0x38119907,
	0x21907207,
	0x88017e0f,
	0x61508fda,
	0x985ff10f,
	0x53e2f1d7,
	0x50fbf545,
	0x30000107,
	0x27bffe27,
	0x60600f51,
	0x20987e1f,
	0x98807e37,
	0xc880410f,
	0x86acbc3f,
	0xc8007e07,
	0x53dab1d7,
	0x50fbf545,
	0x3020010f,
	0x57bffe37,
	0x61d1fe6a,
	0x70008b74,
	0x7010e00a,
	0x7170a040,
	0x7020ae00,
	0x70109b0e,
	0x235c454f,
	0x832e4567,
	0x86acf4cf,
	0xc8007e07,
	0x53d731d7,
	0x50fbf545,
	0x20000907,
	0x28ae7e1f,
	0x6071fff4,
	0x60600e91,
	0x86ad3147,
	0xc8007e07,
	0x53d531d7,
	0x50fbf545,
	0x20000107,
	0x28804c0f,
	0xc8847e17,
	0x86acf13f,
	0x20000107,
	0x28817e17,
	0xc880520f,
	0x86acf20f,
	0x20805207,
	0x9188440f,
	0x86acfe5f,
	0x20805807,
	0x919e440f,
	0x86acfe47,
	0x91201307,
	0xdc088087,
	0xab2061f8,
	0x54088087,
	0xb0000080,
	0xab2121f8,
	0x50f03107,
	0x3000009b,
	0x7012f900,
	0xab5e71d7,
	0xc8801147,
	0x08001107,
	0x2000b907,
	0x70f0e0c8,
	0x09015017,
	0xa81ff107,
	0x70008b7a,
	0x20000007,
	0x218c7357,
	0x2001c907,
	0x3188735f,
	0xc880ff6f,
	0x86ad3e5f,
	0x30001107,
	0x27bffe27,
	0x218c400f,
	0x88807e37,
	0x20807e17,
	0x98947e1f,
	0x98047907,
	0x86acfeb7,
	0xc8007e07,
	0x53c931d7,
	0x50fbf545,
	0x08031107,
	0x20018107,
	0x08805417,
	0x981fa9cf,
	0x201fb9cf,
	0x38827e27,
	0x081421cf,
	0x381131cf,
	0xe7b58c07,
	0x08340037,
	0x38316037,
	0x081f8837,
	0x383f8837,
	0x983fe837,
	0x86acb8cf,
	0x08001107,
	0x20018107,
	0x2080560f,
	0x98805417,
	0xc882fe27,
	0x86acb89f,
	0x08018107,
	0x20001107,
	0x2080560f,
	0x98805417,
	0x08847e27,
	0x981fa1cf,
	0x86acb867,
	0x0805911f,
	0x2002a13f,
	0x70b0e2c8,
	0x37b17e17,
	0xc7b0cc07,
	0x61500ff4,
	0x2865f907,
	0x3c008b87,
	0x30453907,
	0x3822fe60,
	0x08652907,
	0x38663907,
	0x08807e2f,
	0x987f8107,
	0x08342907,
	0x385f3107,
	0x085f4107,
	0x3856c907,
	0x86acf5b7,
	0xc8007e07,
	0x53fcb1d7,
	0x50fbf544,
	0x27b0d207,
	0xc316441f,
	0x30807e17,
	0x97bffe27,
	0xc8807e37,
	0x86acf56f,
	0xc8007e07,
	0x53fa71d7,
	0x50fbf544,
	0x0923ed07,
	0x2004111f,
	0x3090fe4f,
	0x97b0d807,
	0x2044c907,
	0x3316441f,
	0x60100ff2,
	0x3043b907,
	0x37bffe27,
	0x0863a907,
	0x38647907,
	0x86acf4f7,
	0xc8007e07,
	0x53f6b1d7,
	0x50fbf544,
	0x20000107,
	0x28804a0f,
	0x20807e17,
	0x98987e1f,
	0x6071fff4,
	0x9816a107,
	0x86acb87f,
	0xc8007e07,
	0x53f431d7,
	0x50fbf544,
	0xc8804007,
	0x53ef31d7,
	0x50fbf549,
	0x50d1ffff,
	0xf17fc817,
	0x0a1d11cf,
	0x2c1011cf,
	0xc8007e47,
	0x60282889,
	0x66006189,
	0x6498a889,
	0xc1d873cf,
	0xf83d31ff,
	0x50d02107,
	0x3000009c,
	0x7012f900,
	0x84007e17,
	0x23e131ff,
	0x58801147,
	0x50d02107,
	0x3000009c,
	0x7012f900,
	0x8400f2d7,
	0x23dfb1ff,
	0x58801147,
	0x04038c0f,
	0x1c02900f,
	0x62886409,
	0x2880021f,
	0x9b040487,
	0x28ca040f,
	0xfb04058f,
	0xc8800217,
	0x37030e37,
	0xc7020a27,
	0x20d1ffff,
	0xf3080211,
	0x20d1ffff,
	0xf8800408,
	0x2802941f,
	0x2d808207,
	0x30d1ffff,
	0xfea30827,
	0x2603081f,
	0x1f840827,
	0xe6030417,
	0xeea20417,
	0x56ff8447,
	0x60ffff7f,
	0x52000410,
	0x70ffff80,
	0x73cfffa8,
	0x527f8410,
	0x7000007f,
	0xb001042f,
	0x8c00b80f,
	0x62880d1b,
	0x0802941f,
	0x36010c1f,
	0x0f000207,
	0xc602101f,
	0xcf840007,
	0xb6078147,
	0x9600081f,
	0xc8087e00,
	0x96000818,
	0xf83d31ff,
	0x0bdcd1cf,
	0x6add61cf,
	0x2a3421cf,
	0x30808417,
	0x6c439890,
	0xc880011f,
	0x3c1c9137,
	0xfc1c8d27,
	0xc8800337,
	0x8c139c07,
	0xe1bf8407,
	0xc186010f,
	0xdd90c31f,
	0xdc014e47,
	0xf8e3f138,
	0xf55fce47,
	0xab20a1f8,
	0x24038d0f,
	0x13070337,
	0x706bf538,
	0x2182502f,
	0x818a4307,
	0x7037ea34,
	0xd0840e37,
	0x26030d4f,
	0x667f8e17,
	0x19830c19,
	0x76030547,
	0x18ca060f,
	0xf9830400,
	0x7051f71d,
	0x3c114b1f,
	0xc8ca0127,
	0x2d8ecb1f,
	0xbd8dc91f,
	0x51ee48ff,
	0x80000047,
	0x56d1391f,
	0x2000008f,
	0xe40f3f1f,
	0x56cf551f,
	0x2000008f,
	0x8c0e1cef,
	0x56ce3d1f,
	0x2000008f,
	0x54ed0cdf,
	0x1000008f,
	0x56cd591f,
	0x2000008f,
	0x8c0c910f,
	0xcd8432c7,
	0xd08432bf,
	0xf0cbb0b7,
	0x56cb411f,
	0x2000008f,
	0x8c0a950f,
	0xcd842aa7,
	0xd0842a9f,
	0xf0c9a897,
	0x56c95d1f,
	0x2000008f,
	0x8c08990f,
	0xcd84227f,
	0xd0842287,
	0xf0c81e3f,
	0x56c3c51f,
	0x2000008f,
	0x8c071d0f,
	0xcd841c67,
	0xd0841c6f,
	0xf0c69807,
	0x56c0611f,
	0x2000008f,
	0x8c05a10f,
	0xcd84164f,
	0xd0841657,
	0xf0c51247,
	0x56c4491f,
	0x2000008f,
	0x8c03250f,
	0x56e4291f,
	0x2000002f,
	0x56e3ad1f,
	0x2000002f,
	0x50c2cd1f,
	0x3000008f,
	0xcd840c1f,
	0xd0840c17,
	0xf0c1060f,
	0x56c0e51f,
	0x2000008f,
	0x56c0c91f,
	0x2000005f,
	0x56c04d1f,
	0x2000005f,
	0x50c2691f,
	0x3000008f,
	0x28ca4f3f,
	0xf100cd37,
	0x2e01ce0f,
	0x9f5fcc47,
	0x27138207,
	0xd8007f17,
	0x7043ee39,
	0x2b0e432f,
	0x8d92011f,
	0x7043e74c,
	0xab0541f8,
	0x34030107,
	0x17b2c207,
	0xc100c517,
	0xd0860c17,
	0xabd9b1d7,
	0x32028307,
	0x17b24207,
	0xc6078a17,
	0xabd8b1d7,
	0x34020107,
	0x17b2c207,
	0xcf02081f,
	0xc6078617,
	0xabd771d7,
	0x34010507,
	0x47b24207,
	0xc6078417,
	0xabd671d7,
	0xdf514c47,
	0xabfb71f8,
	0x0e00ce27,
	0x921d11cf,
	0x0eb38817,
	0xdc1005cf,
	0xce04041f,
	0x50b4691f,
	0x3000008f,
	0x0f01060f,
	0xdc120dcf,
	0xce020207,
	0xeea0513f,
	0x50b3f11f,
	0x3000008f,
	0x0a3421cf,
	0x2c1109cf,
	0x9c1311cf,
	0xc1ac73cf,
	0xf83d31ff,
	0x24000d0f,
	0x11874307,
	0x7043ee3a,
	0x706bf538,
	0x2182504f,
	0x830e4527,
	0x2084000f,
	0xa3044d37,
	0x26030347,
	0x667f8087,
	0x218302b8,
	0x78804407,
	0x20ca2eb7,
	0xf880480f,
	0x51ee2caf,
	0x80000047,
	0xe40a2b1f,
	0xdd89ab1f,
	0x56ca391f,
	0x2000008f,
	0x8c091c9f,
	0x56c93d1f,
	0x2000008f,
	0x8c08910f,
	0xcb7fa27f,
	0xf0c7a077,
	0x56c7411f,
	0x2000008f,
	0x0406950f,
	0x1806110f,
	0xcb7f9a5f,
	0xf0c61657,
	0x56c5451f,
	0x2000008f,
	0x0404190f,
	0x1803950f,
	0x56e52d1f,
	0x2000002f,
	0x50c4cd1f,
	0x3000008f,
	0xcb7f9037,
	0xf0c38c2f,
	0x56c2c91f,
	0x2000008f,
	0x56e2a91f,
	0x2000002f,
	0x8c02190f,
	0xcf02081f,
	0xc6078617,
	0xabc4f1d7,
	0x3401190f,
	0x17b24407,
	0xc6078417,
	0xabc3f1d7,
	0xabe671ff,
	0x0b9cf1cf,
	0x6abd01cf,
	0x227901cf,
	0x3802fe0f,
	0x3c1c9117,
	0xfc1c8d07,
	0x63700002,
	0x50f0e807,
	0x2000008f,
	0x6d7398b4,
	0xc8017e07,
	0x3c1c9d47,
	0xfc1ca157,
	0x6fb398b4,
	0xfc1cad87,
	0x50e0a917,
	0x6000002f,
	0x0880440f,
	0x96000507,
	0x7001fd70,
	0xf4a00195,
	0xb21f880f,
	0x9003890f,
	0x50b16907,
	0x3000008f,
	0x50b17107,
	0x3000008f,
	0x50b17907,
	0x3000008f,
	0x9017383f,
	0x50d10107,
	0x3000008f,
	0x50d10907,
	0x3000008f,
	0x50d11107,
	0x3000008f,
	0x50d11907,
	0x3000008f,
	0x50d12107,
	0x3000008f,
	0x50d12907,
	0x3000008f,
	0x50d13107,
	0x3000008f,
	0xf55f5c87,
	0xab0121f8,
	0x56805cc7,
	0x60000001,
	0xab1d81f8,
	0x2880ff7f,
	0x9380ff1f,
	0x9000203f,
	0xdc008087,
	0xab1f61f8,
	0x27bffe27,
	0xc8800e07,
	0x209e7e1f,
	0x98807e37,
	0x7043ee01,
	0x20807e17,
	0x91887327,
	0x86acb207,
	0x20007e07,
	0x91824187,
	0x53e0b1d7,
	0x50fbf543,
	0x2007810f,
	0x2880480f,
	0x9207307f,
	0x0a06c07f,
	0x2001287f,
	0x2006507f,
	0x218c1e07,
	0x9a0721cf,
	0x9a06b1cf,
	0x980641cf,
	0x86acb4ef,
	0x0925910f,
	0x2005010f,
	0x73204149,
	0x23847e4f,
	0xa88d7e3f,
	0x70b04249,
	0x0c009787,
	0xb000a857,
	0x52001230,
	0x7000000c,
	0x37628dcf,
	0xb83f8e20,
	0x30228d19,
	0xdea26017,
	0x62b009c3,
	0x37bfc627,
	0xc7bffe37,
	0x980431cf,
	0x86ac7197,
	0xc8007e07,
	0x53d871d7,
	0x50fbf543,
	0x0c01b907,
	0x1120910f,
	0x2d840617,
	0x9c008387,
	0xd0840607,
	0x3b5e81f8,
	0x50c00417,
	0x0801b90f,
	0x2002410f,
	0xc880462f,
	0x0801a107,
	0x36021d07,
	0x21bf840f,
	0xc8804007,
	0x50818907,
	0x30000090,
	0xef008a0f,
	0x56820507,
	0x20000090,
	0xabb071d7,
	0x50c0cd07,
	0x2000008f,
	0x0801910f,
	0x2001590f,
	0x54800507,
	0x10000090,
	0xef110367,
	0x2780d967,
	0x91808417,
	0x9816290f,
	0x0f5f8147,
	0xb811801f,
	0x2001590f,
	0x38805977,
	0xab2021f8,
	0x369f005f,
	0xde9f5817,
	0x31bf9657,
	0xc1bf844f,
	0x35841457,
	0xbebf5827,
	0x2e041017,
	0x9da20917,
	0x2704042f,
	0xde01123f,
	0x35a30b67,
	0xbf03881f,
	0x7037e800,
	0x2080402f,
	0x9302061f,
	0x2da30c07,
	0xbf5fd94f,
	0xab01e1f9,
	0xc8800807,
	0xa5e17c0f,
	0xce080417,
	0xb2e17807,
	0xf5418187,
	0xabff31f8,
	0x702bf444,
	0x25430887,
	0xe1870a2f,
	0x702bf433,
	0xab18e1f8,
	0x50c0cc2f,
	0x2000008f,
	0xabfcb1ff,
	0xb6c05dc7,
	0xab4c61f8,
	0x2900dd1f,
	0xa380fe07,
	0xc880460f,
	0x53fa70ff,
	0x50fbf563,
	0x0880117f,
	0x9000203f,
	0x28d1ffff,
	0xfc008087,
	0xabe141f8,
	0x0801610f,
	0x2000910f,
	0x393fe30f,
	0x355f5d87,
	0x7260efc9,
	0x09808407,
	0x881f800f,
	0x0b5541f8,
	0x5800610f,
	0x54e0450f,
	0x1000000b,
	0xf55f8047,
	0xab5141f8,
	0x2120610f,
	0x2e015d37,
	0xdf5f8147,
	0xab4361f8,
	0x3f11fe17,
	0xd55fdc87,
	0x2601040f,
	0x9e015d37,
	0x7061f71d,
	0x20987f5f,
	0x9402038f,
	0x30807f2f,
	0x9eb8db8f,
	0x3d025b87,
	0x88807f90,
	0x24004b07,
	0xce00cb67,
	0xdf5f8147,
	0xab38e1f8,
	0x2eb2d81f,
	0xddb54b2f,
	0x2e018617,
	0x9db3d507,
	0x2712840f,
	0xd8805747,
	0x26010207,
	0x98807f27,
	0x71b0b209,
	0x7043dd7f,
	0x330571ff,
	0x5eb4c34f,
	0x36b2541f,
	0xd7b25207,
	0x35a10707,
	0xb7bfcc27,
	0x50a16817,
	0x2000008f,
	0x30807e1f,
	0x97bffe37,
	0x24025147,
	0x81824f3f,
	0x86acb7d7,
	0xc8007e07,
	0x53f631d7,
	0x50fbf542,
	0x2400cb07,
	0xc100c807,
	0xc180c927,
	0xdf400247,
	0xab3101f8,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xabfa81f8,
	0x2000010f,
	0x28805017,
	0x27bffe27,
	0xc8805a0f,
	0x23827e1f,
	0xa8807e37,
	0x86ac7717,
	0xc8007e07,
	0x53f171d7,
	0x50fbf542,
	0x3001013f,
	0x27b86207,
	0x2080461f,
	0x98805e27,
	0xab47b1d7,
	0xabf6b1ff,
	0x54e0410f,
	0x1000000b,
	0xc800fe0f,
	0x56e0c50f,
	0x2000000b,
	0xf55f8047,
	0xab3b41f8,
	0x2120610f,
	0x2e015937,
	0xdf5f8147,
	0xab1b21f8,
	0x28005997,
	0x9f5fd947,
	0x3e015937,
	0x98807e38,
	0x0d5fd887,
	0xe403dbcf,
	0x7060f711,
	0x3804d1cf,
	0x38807e00,
	0x0880436f,
	0x940059cf,
	0x20987f7f,
	0x98807f2f,
	0x2500d95f,
	0x88c07f4f,
	0x24004b07,
	0xce00cb8f,
	0xdf5f8147,
	0xab0f21f8,
	0x2eb2e25f,
	0xddb44b2f,
	0x2e019657,
	0x9db3d107,
	0x2712944f,
	0xd8805f27,
	0x26011207,
	0x98807f1f,
	0x71b0b20a,
	0x7043dd7f,
	0x3308b1ff,
	0x5eb54357,
	0xeeb1d00f,
	0xdda00307,
	0x50a0e807,
	0x2000008f,
	0x2000010f,
	0x28804817,
	0x2eb5820f,
	0xd3827e1f,
	0x6071fff4,
	0x24024927,
	0x81824f3f,
	0x86ac762f,
	0xc8007e07,
	0x53e1b1d7,
	0x50fbf542,
	0x36b1d01f,
	0xd7b1d407,
	0x35a10707,
	0xb7bfcc27,
	0x50a16817,
	0x2000008f,
	0x30807e1f,
	0x97bffe37,
	0x86acb4e7,
	0xc8007e07,
	0x53deb1d7,
	0x50fbf542,
	0x2400cb07,
	0xc100c607,
	0xc180c71f,
	0xdf400247,
	0xab0401f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8147,
	0x90000138,
	0x2800d1c8,
	0x2380fe18,
	0xc8800010,
	0xab2f61d0,
	0xb600c747,
	0xabf5d1f8,
	0x9120636f,
	0xf55f8187,
	0xab0f21f8,
	0x0bf5f1ff,
	0x5000813f,
	0x880059cf,
	0xf88401c7,
	0xab0121f8,
	0xe7b15807,
	0xf4a00195,
	0xb21f880f,
	0x26b2e217,
	0xd887fe0f,
	0x2601841f,
	0x91965b6f,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0xc40a5f7f,
	0x86acb387,
	0xc8007e07,
	0x53d3b1d7,
	0x50fbf542,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabe941f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8047,
	0xab00e1f8,
	0x56ffc10f,
	0x2000000b,
	0x2002510f,
	0x28817e1f,
	0x731400c8,
	0x56808507,
	0x10000090,
	0x70b0e049,
	0x08804c47,
	0x921d11cf,
	0x6890cc89,
	0xeeb70977,
	0x0817510f,
	0x3801b10f,
	0x0801390f,
	0x3800c10f,
	0x0800490f,
	0x327901cf,
	0x60d40189,
	0x602c4889,
	0x66048189,
	0x9c1729cf,
	0xc1e473cf,
	0xf83d31ff,
	0x06b1d017,
	0xd8005bcf,
	0xdda08507,
	0x50a1680f,
	0x2000008f,
	0x50c2196f,
	0x20000003,
	0x38d1ffff,
	0xf88401c7,
	0xab03a1f8,
	0x2e9f641f,
	0xdf5fe547,
	0xe1bf860f,
	0xc1808207,
	0x20008000,
	0x7880600f,
	0xf4a001bd,
	0xa8030817,
	0xe7220c37,
	0xd00f8c37,
	0xf8148d27,
	0xf8a87e1f,
	0xb201880f,
	0x26b1d017,
	0xd880600f,
	0xdda00507,
	0x50b86807,
	0x3000008f,
	0xabe031ff,
	0xf88465c7,
	0xab0121f8,
	0xe7b15c07,
	0xf4a00195,
	0xb21f880f,
	0x26b2d817,
	0xd887fe0f,
	0x2601841f,
	0x940a575f,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0x86acb0e7,
	0xc8007e07,
	0x53feb1d7,
	0x50fbf541,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabc001f8,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xc800fe00,
	0x56e04108,
	0x2000000b,
	0x56ffc508,
	0x2000000b,
	0xabeab1ff,
	0x255fdd87,
	0xe8807f1f,
	0x2380fe00,
	0xa880ff7f,
	0x20805c08,
	0x98805d18,
	0x53eda0f8,
	0x50fbf562,
	0x2394f1ff,
	0x58801178,
	0x0c062107,
	0x10059107,
	0x24051d07,
	0x130d462f,
	0x2d84183f,
	0x91081637,
	0x2084184f,
	0xae08160f,
	0x2d841427,
	0x90841447,
	0x38c48e1f,
	0xe0c08c07,
	0x39bf861f,
	0xc0c40827,
	0x08001107,
	0x36021d07,
	0x0e010947,
	0x6801a107,
	0xab9ee1f8,
	0x61042fe0,
	0x0b9e71ff,
	0x56001d07,
	0x2800090f,
	0x2380fe17,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86ac73ff,
	0xc8007e07,
	0x53f0b1d7,
	0x50fbf541,
	0xabc2b1ff,
	0x2380fe17,
	0xa8800e07,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86ac73a7,
	0xc8007e07,
	0x53edf1d7,
	0x50fbf541,
	0xabacb1ff,
	0x9000210f,
	0x7391fd7e,
	0xf55f8187,
	0x0baab1ff,
	0x50172908,
	0xf55f8687,
	0xab02e1f8,
	0x26bf062f,
	0xd8807e27,
	0xdda08a0f,
	0xf4a007bd,
	0x13038807,
	0xc8e3780f,
	0xc1808827,
	0xf8030f27,
	0xf8703e2f,
	0xb2028817,
	0xc8007e47,
	0xf83d31ff,
	0xf55f8687,
	0xab02e1f8,
	0x26bf062f,
	0xde010827,
	0xdda00a07,
	0xf4a007bd,
	0x1003000f,
	0x28e3f807,
	0xeea2020f,
	0xf8030f27,
	0xf8703e2f,
	0xb2028817,
	0xc8007e47,
	0xf83d31ff,
	0x01bf8417,
	0xcc01840f,
	0xc683843f,
	0xe4028e07,
	0x38d1ffff,
	0xf8ca0637,
	0x67f03de7,
	0x3682861f,
	0xd4820807,
	0xb62c0747,
	0x0b0761f8,
	0x5601840f,
	0xc82c7e07,
	0x9600040f,
	0x2f818837,
	0x9b020587,
	0x9000080f,
	0xeea20c30,
	0xdb010587,
	0xcf808818,
	0xeea18c30,
	0xdb008587,
	0xcf810820,
	0xeea20c30,
	0x2e02000f,
	0x1b040587,
	0xeea20c2f,
	0xef030828,
	0x56ff8a47,
	0x60ffff7f,
	0x52000a10,
	0x70ffff80,
	0x73cfffa8,
	0x527f8418,
	0x7000007f,
	0xb0018407,
	0x0800080f,
	0x3601800f,
	0xf83d31ff,
	0xdf41fe47,
	0x0bf9b1ff,
	0x561f8408,
	0x0bdcb1cf,
	0x6add41cf,
	0x2a3541cf,
	0x30808427,
	0x6d439132,
	0x3080013f,
	0x91bf881f,
	0x3c1c9547,
	0xfc1c8507,
	0xc1860617,
	0x3d91853f,
	0xbc1c8d27,
	0xc183471f,
	0x00d1ffff,
	0xfc001c07,
	0x706bfa7a,
	0x28d1ffff,
	0xfc01004f,
	0x3f5f8147,
	0xb8e3f149,
	0xab08e1f8,
	0x30804e07,
	0x97bffe17,
	0xc82c7e3f,
	0x706bfa7a,
	0x3405811f,
	0x1ea15467,
	0x20d1ffff,
	0xf1811837,
	0xcd84164f,
	0xd0841657,
	0xf0c51247,
	0x56e43007,
	0x2000008f,
	0x8802871f,
	0xc182471f,
	0x56e2b407,
	0x2000008f,
	0xe9045417,
	0x54e23407,
	0x1000008f,
	0x50e33807,
	0x3000008f,
	0xb62c0947,
	0xdf427e4f,
	0xab0f61f8,
	0x56e3b407,
	0x2000008f,
	0x24021d3f,
	0x1100861f,
	0x2302020f,
	0x81840007,
	0x702bf922,
	0xdf418847,
	0xabf8e1f8,
	0x21010347,
	0xa8007f37,
	0x275fd147,
	0xb8007f27,
	0x7043fa7a,
	0xab00d1f8,
	0xab07f1ff,
	0xf114cd37,
	0x20ca4c07,
	0xf182472f,
	0x51fd810f,
	0x80000011,
	0x25d0c33f,
	0xb8804707,
	0x32030307,
	0x17b0c407,
	0xc6078c17,
	0xabe3b1d7,
	0x34028107,
	0x17b0c407,
	0xd0860a17,
	0xabe2b1d7,
	0x34020107,
	0x17b0c407,
	0xc6078817,
	0xabe1b1d7,
	0x34018507,
	0x47b0c407,
	0xcf020617,
	0xc6078417,
	0xabe071d7,
	0xf552c187,
	0xabfba1f8,
	0x2100c927,
	0x8182471f,
	0xdf525147,
	0xabf8d1f8,
	0x715bf972,
	0x50d5613f,
	0x3000008f,
	0x6051a409,
	0xe7015217,
	0x60082d09,
	0x9c1415cf,
	0xeeb50557,
	0x50d5693f,
	0x3000008f,
	0x0a3541cf,
	0x2c1311cf,
	0xc1b473cf,
	0xf83d31ff,
	0x56ffb401,
	0x2000008f,
	0xabf131ff,
	0x0b9cf1cf,
	0x6abd01cf,
	0x227901cf,
	0x3802fe0f,
	0x3c1c9117,
	0xfc1c8d07,
	0x63700002,
	0x50d0d807,
	0x20000090,
	0x6d7398b4,
	0xc8017e07,
	0x3c1c9d47,
	0xfc1ca157,
	0x6fb398b4,
	0xfc1cad87,
	0x50e0d117,
	0x6000002f,
	0x0880440f,
	0x96000507,
	0x71f1fe70,
	0xf4a00195,
	0xb21f880f,
	0x9003890f,
	0x50d16107,
	0x3000008f,
	0x50d16907,
	0x3000008f,
	0x50d17107,
	0x3000008f,
	0x9017383f,
	0x50d17907,
	0x3000008f,
	0x50f10107,
	0x3000008f,
	0x50f10907,
	0x3000008f,
	0x50f11107,
	0x3000008f,
	0x50f11907,
	0x3000008f,
	0x50f12107,
	0x3000008f,
	0x50f12907,
	0x3000008f,
	0xf55f5c87,
	0xab0121f8,
	0x56805cc7,
	0x60000001,
	0xab1dc1f8,
	0x2880ff7f,
	0x9380ff1f,
	0x9000203f,
	0xdc008087,
	0xab1fa1f8,
	0x27bffe27,
	0xc8800e07,
	0x20e27e1f,
	0x98807e37,
	0x7043fa01,
	0x209e7e17,
	0x91887327,
	0x86ac3bd7,
	0x20007e07,
	0x91824187,
	0x53ef31d7,
	0x50fbf540,
	0x2007810f,
	0x2880480f,
	0x9207307f,
	0x0a06c07f,
	0x2001287f,
	0x2006507f,
	0x218c1e07,
	0x9a0721cf,
	0x9a06b1cf,
	0x980641cf,
	0x86ac3ebf,
	0x0925910f,
	0x2005010f,
	0x73204149,
	0x23847e4f,
	0xa88d7e3f,
	0x70b04249,
	0x0c009787,
	0xb000a857,
	0x52001230,
	0x7000000c,
	0x37628dcf,
	0xb83f8e20,
	0x30228d19,
	0xdea26017,
	0x62b009c3,
	0x37bfc627,
	0xc7bffe37,
	0x980431cf,
	0x86abfb67,
	0xc8007e07,
	0x53e6f1d7,
	0x50fbf540,
	0x0c01b907,
	0x1120910f,
	0x2d840617,
	0x9c008387,
	0xd0840607,
	0x3b5e81f8,
	0x50c00417,
	0x0801b90f,
	0x2002410f,
	0xc880462f,
	0x0801a107,
	0x36021d07,
	0x21bf840f,
	0xc8804007,
	0x50c1f907,
	0x30000090,
	0xef008a0f,
	0x56c27507,
	0x20000090,
	0xabc771d7,
	0x50e0b907,
	0x2000008f,
	0x0801910f,
	0x2001590f,
	0x54c07507,
	0x10000090,
	0xef110367,
	0x2780d967,
	0x91808417,
	0x9816290f,
	0x0f5f8147,
	0xb811801f,
	0x2001590f,
	0x38805977,
	0xab2061f8,
	0x369f0067,
	0xde9f585f,
	0x31bf983f,
	0xcebf5827,
	0x35850e3f,
	0xb1bf9647,
	0x2603944f,
	0x9e01102f,
	0x2f039217,
	0xdda20917,
	0x35c0043f,
	0xbf02881f,
	0x7037f202,
	0x3080402f,
	0x9ea05837,
	0x2b02061f,
	0x8da30c17,
	0xdf5fd94f,
	0xab01e1f9,
	0xc8800807,
	0xa5e17c0f,
	0xce080417,
	0xb2e17807,
	0xf5418187,
	0xabff31f8,
	0x702bf944,
	0x25430887,
	0xe1840a2f,
	0x702bf933,
	0xab18e1f8,
	0x50e0b82f,
	0x2000008f,
	0xabfcb1ff,
	0xb6c05dc7,
	0xab4c21f8,
	0x2900dd1f,
	0xa380fe07,
	0xc880460f,
	0x53c8b0ff,
	0x50fbf561,
	0x0880117f,
	0x9000203f,
	0x28d1ffff,
	0xfc008087,
	0xabe101f8,
	0x0801610f,
	0x2000910f,
	0x393fe30f,
	0x355f5d87,
	0x7260efc9,
	0x09808407,
	0x881f800f,
	0x0b5501f8,
	0x5800610f,
	0x54e0450f,
	0x1000000b,
	0xf55f8047,
	0xab5101f8,
	0x2120610f,
	0x2e015d37,
	0xdf5f8147,
	0xab4321f8,
	0x3f11fe17,
	0xd55fdc87,
	0x2601040f,
	0x9e015d37,
	0x7061f71d,
	0x20987f5f,
	0x9402038f,
	0x30807f2f,
	0x9eb8db8f,
	0x3d025b87,
	0x88807f90,
	0x24004b07,
	0xce00cb67,
	0xdf5f8147,
	0xab38a1f8,
	0x2eb2d827,
	0xddb54b2f,
	0x2601881f,
	0x9e01540f,
	0x27128617,
	0xd8805747,
	0x26010407,
	0x98807f27,
	0x70a3ec1f,
	0x71b0b209,
	0x36b4c34f,
	0xdeb3c13f,
	0xab04f1ff,
	0x36b2541f,
	0xd7b25207,
	0x7028fb32,
	0x23010507,
	0xc8807e1f,
	0x37bfcc27,
	0xc7bffe37,
	0x24025147,
	0x81824f3f,
	0x86ac719f,
	0xc8007e07,
	0x53c471d7,
	0x50fbf540,
	0x2400cb07,
	0xc100c807,
	0xc180c927,
	0xdf400247,
	0xab30c1f8,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xabfac1f8,
	0x2000010f,
	0x28805017,
	0x27bffe27,
	0xc8805a0f,
	0x23827e1f,
	0xa8807e37,
	0x86ac30df,
	0xc8007e07,
	0x53ffb1d7,
	0x50fbf53f,
	0x3001013f,
	0x27b86207,
	0x2080461f,
	0x98805e27,
	0xab95f1d7,
	0xabf6f1ff,
	0x54e0410f,
	0x1000000b,
	0xc800fe0f,
	0x56e0c50f,
	0x2000000b,
	0xf55f8047,
	0xab3b01f8,
	0x2120610f,
	0x2e015937,
	0xdf5f8147,
	0xab1b21f8,
	0x28005997,
	0x9f5fd947,
	0x3e015937,
	0x98807e38,
	0x0d5fd887,
	0xe403dbcf,
	0x7060f711,
	0x3804d1cf,
	0x38807e00,
	0x0880436f,
	0x940059cf,
	0x20987f7f,
	0x98807f2f,
	0x2500d95f,
	0x88c07f4f,
	0x24004b07,
	0xce00cb8f,
	0xdf5f8147,
	0xab0f21f8,
	0x2eb2e267,
	0xddb44b2f,
	0x2601985f,
	0x9e01504f,
	0x27129657,
	0xd8805f27,
	0x26011407,
	0x98807f1f,
	0x51d8133f,
	0x8000008f,
	0x71b0b20a,
	0x36b54357,
	0xdeb3c13f,
	0xab07f1ff,
	0xeeb1d00f,
	0x7028fb10,
	0xe3008107,
	0x2000010f,
	0x28804817,
	0x2eb5820f,
	0xd3827e1f,
	0x6071fff4,
	0x24024927,
	0x81824f3f,
	0x86abffef,
	0xc8007e07,
	0x53efb1d7,
	0x50fbf53f,
	0x36b1d01f,
	0xd7b1d407,
	0x7028fb32,
	0x23010507,
	0xc8807e1f,
	0x37bfcc27,
	0xc7bffe37,
	0x86ac3eaf,
	0xc8007e07,
	0x53ecf1d7,
	0x50fbf53f,
	0x2400cb07,
	0xc100c607,
	0xc180c71f,
	0xdf400247,
	0xab0401f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8147,
	0x90000138,
	0x2800d1c8,
	0x2380fe18,
	0xc8800010,
	0x87ffafb0,
	0xb600c747,
	0xabf651f8,
	0x9120636f,
	0xf55f8187,
	0xab0f61f8,
	0x0bf631ff,
	0x5000813f,
	0x880059cf,
	0xf88401c7,
	0xab0121f8,
	0xe7b15807,
	0xf4a00195,
	0xb21f880f,
	0x26b2e217,
	0xd887fe0f,
	0x2601841f,
	0x91965b6f,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0xc40a5f7f,
	0x86ac3d4f,
	0xc8007e07,
	0x53e1f1d7,
	0x50fbf53f,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabe941f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8047,
	0xab00e1f8,
	0x56ffc10f,
	0x2000000b,
	0x2002510f,
	0x2888fe1f,
	0x50c17907,
	0x20000090,
	0x56c0f507,
	0x10000090,
	0x70b0e049,
	0x08804c47,
	0x921d11cf,
	0x6890cc89,
	0xeeb70977,
	0x0817510f,
	0x3801b10f,
	0x0801390f,
	0x3800c10f,
	0x0800490f,
	0x327901cf,
	0x60d40189,
	0x602c4889,
	0x66048189,
	0x9c1729cf,
	0xc1e473cf,
	0xf83d31ff,
	0x06b1d017,
	0xd8005bcf,
	0x7028fb21,
	0xe3010307,
	0x50c2196f,
	0x20000003,
	0x38d1ffff,
	0xf88401c7,
	0xab03a1f8,
	0x2e9f641f,
	0xdf5fe547,
	0xe1bf860f,
	0xc1808207,
	0x20008000,
	0x7880600f,
	0xf4a001bd,
	0xa8030817,
	0xe7220c37,
	0xd00f8c37,
	0xf8148d27,
	0xf8a87e1f,
	0xb201880f,
	0x26b1d017,
	0xd880600f,
	0x7028fb20,
	0x33e0f1ff,
	0x5a380107,
	0xf88465c7,
	0xab0121f8,
	0xe7b15c07,
	0xf4a00195,
	0xb21f880f,
	0x26b2d817,
	0xd887fe0f,
	0x2601841f,
	0x940a575f,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0x86ac3ab7,
	0xc8007e07,
	0x53cd31d7,
	0x50fbf53f,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabc041f8,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xc800fe00,
	0x56e04108,
	0x2000000b,
	0x56ffc508,
	0x2000000b,
	0xabeaf1ff,
	0x255fdd87,
	0xe8807f1f,
	0x2380fe00,
	0xa880ff7f,
	0x20805c08,
	0x98805d18,
	0x53fc20f8,
	0x50fbf55f,
	0x2394f1ff,
	0x58801178,
	0x0c062107,
	0x10059107,
	0x24051d07,
	0x130d462f,
	0x2d84183f,
	0x91081637,
	0x2084184f,
	0xae08160f,
	0x2d841427,
	0x90841447,
	0x38c48e1f,
	0xe0c08c07,
	0x39bf861f,
	0xc0c40827,
	0x08001107,
	0x36021d07,
	0x0e010947,
	0x6801a107,
	0xab9ee1f8,
	0x61042fe0,
	0x0b9e71ff,
	0x56001d07,
	0x2800090f,
	0x2380fe17,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86abfdcf,
	0xc8007e07,
	0x53ff31d7,
	0x50fbf53e,
	0xabc2f1ff,
	0x2380fe17,
	0xa8800e07,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86abfd77,
	0xc8007e07,
	0x53fc71d7,
	0x50fbf53e,
	0xabacf1ff,
	0x9000210f,
	0x517cff77,
	0x90000007,
	0xf55f8187,
	0x0baab1ff,
	0x50172908,
	0x2401840f,
	0x1683843f,
	0x30d1ffff,
	0xf4028e07,
	0xf8ca0637,
	0x31840c27,
	0x8e82861f,
	0x1c820807,
	0xc62c0747,
	0x0b07a1f8,
	0x5601840f,
	0xc82c7e07,
	0x9600040f,
	0x21bf843f,
	0xcf818837,
	0x0b020f87,
	0xb000080f,
	0xeea20c30,
	0xdb010f87,
	0xcf808818,
	0xeea18c30,
	0xdb008f87,
	0xcf810820,
	0xeea20c30,
	0x2e02000f,
	0x1b040f87,
	0xeea20c2f,
	0xef030828,
	0x56ff8a47,
	0x60ffff7f,
	0x52000a10,
	0x70ffff80,
	0x73cfffa8,
	0x527f8418,
	0x7000007f,
	0xb0018407,
	0x0800080f,
	0x3601800f,
	0xf83d31ff,
	0xdf41fe47,
	0x0bf971ff,
	0x561f8408,
	0x5178fe17,
	0x90f0f0f0,
	0xe6610217,
	0x0bdc91cf,
	0x6add21cf,
	0x9a3661cf,
	0x3c1c8917,
	0xfc1c9547,
	0xc8807f47,
	0x3c1c9957,
	0xfc1c9137,
	0x2080013f,
	0x982c7f57,
	0x6c439ca0,
	0x2190054f,
	0xa18f4f1f,
	0x0103534f,
	0xac001c07,
	0x706bfa7b,
	0x7037f371,
	0x50f5b93f,
	0x3000008f,
	0x50e0c93f,
	0x3000008f,
	0x255fd287,
	0xe8000167,
	0x7043fa7a,
	0xab0c21f8,
	0x275f8147,
	0xb1914727,
	0x20804f2f,
	0x98807f37,
	0xab0a21f8,
	0x24e23d27,
	0x1310491f,
	0x51fdcd0f,
	0x80000011,
	0xddd0c33f,
	0xc63f881f,
	0x20ca0607,
	0xfb3f880f,
	0xb62c0047,
	0x56e0352f,
	0x2000008f,
	0x56e0b12f,
	0x2000008f,
	0x56f53528,
	0x2000008f,
	0x3410051f,
	0x47b0c407,
	0xcf04401f,
	0xc6078617,
	0xabe8b1d7,
	0x2fb0c407,
	0xc0864017,
	0xabe7f1d7,
	0x27b0c407,
	0xc607c017,
	0xabe731d7,
	0x37024017,
	0x97b0c407,
	0xc6078417,
	0xabe631d7,
	0xf5524787,
	0xabfc31f8,
	0x24001d3f,
	0x1100cc0f,
	0x21914727,
	0x8180cd37,
	0xc1844b2f,
	0xdf408047,
	0xabf6a1f8,
	0xc180d147,
	0xdf745347,
	0xabf491f8,
	0x0c015847,
	0xb21d11cf,
	0x08e3f010,
	0xf23661cf,
	0x0e008437,
	0x9c1005cf,
	0xeea10c2f,
	0x0e038a27,
	0x9c1109cf,
	0xef01081f,
	0x0dc08617,
	0xbc120dcf,
	0x2c1415cf,
	0x3da0035f,
	0x50d5e13f,
	0x3000008f,
	0x50c0693f,
	0x3000008f,
	0x602ca109,
	0xc1bc73cf,
	0xf83d31ff,
	0x0b9cf1cf,
	0x6abd01cf,
	0x227901cf,
	0x3802fe0f,
	0x3c1c9117,
	0xfc1c8d07,
	0x63700002,
	0x50d0d807,
	0x20000090,
	0x6d7398b4,
	0xc8017e07,
	0x3c1c9d47,
	0xfc1ca157,
	0x6fb398b4,
	0xfc1cad87,
	0x50e0d117,
	0x6000002f,
	0x0880440f,
	0x96000507,
	0x7003fcf0,
	0xf4a00195,
	0xb21f880f,
	0x9003890f,
	0x50d16107,
	0x3000008f,
	0x50d16907,
	0x3000008f,
	0x50d17107,
	0x3000008f,
	0x9015383f,
	0x50d17907,
	0x3000008f,
	0x50f10107,
	0x3000008f,
	0x50f10907,
	0x3000008f,
	0x50f11107,
	0x3000008f,
	0x50f11907,
	0x3000008f,
	0x50f12107,
	0x3000008f,
	0x50f12907,
	0x3000008f,
	0xf55f5487,
	0xab0121f8,
	0x568054c7,
	0x60000001,
	0xab1c81f8,
	0x2880ff87,
	0x9380ff1f,
	0x9000203f,
	0xdc008087,
	0xab1e61f8,
	0x27bffe27,
	0xc8800e07,
	0x20e27e1f,
	0x98807e37,
	0x7043fa01,
	0x209e7e17,
	0x91887327,
	0x86abf6e7,
	0x20007e07,
	0x91824187,
	0x53c7b1d7,
	0x50fbf53e,
	0x2006010f,
	0x2880480f,
	0x9205b067,
	0x0a054067,
	0x20012867,
	0x2004d067,
	0x218c1807,
	0x9a05a1cf,
	0x9a0531cf,
	0x9804c1cf,
	0x86abf9cf,
	0x0923910f,
	0x2000810f,
	0x70d04149,
	0x23887e37,
	0xa88d7e2f,
	0x70b04249,
	0x0c008f87,
	0xb000a80f,
	0x52000c18,
	0x70000014,
	0x376207cf,
	0xb83f8a10,
	0x20220719,
	0xd8804807,
	0x26a16017,
	0xd8807e1f,
	0x37bfc627,
	0xc7bffe37,
	0x980431cf,
	0x86abb66f,
	0xc8007e07,
	0x53ff31d7,
	0x50fbf53d,
	0x9120110f,
	0xdc008187,
	0xab5d81f8,
	0x0800390f,
	0x2001410f,
	0xc880460f,
	0x08002107,
	0x36011d07,
	0x50c07907,
	0x30000090,
	0x56c17507,
	0x20000090,
	0xc8804007,
	0xabcbf1d7,
	0x50e0b907,
	0x2000008f,
	0x0801910f,
	0x2001590f,
	0x54c07507,
	0x10000090,
	0xef11036f,
	0x2780db6f,
	0x91808417,
	0x9816a90f,
	0x0f5f8147,
	0xb811801f,
	0x2001590f,
	0x38805b57,
	0xab2521f8,
	0x369f0067,
	0xde9f5a5f,
	0x31bf983f,
	0xcebf5a27,
	0x35850e3f,
	0xb1bf9647,
	0x2603944f,
	0x9e01102f,
	0x2f039217,
	0xdda20917,
	0x35c0043f,
	0xbf02881f,
	0x7037f202,
	0x3080402f,
	0x9ea05a37,
	0x2b02061f,
	0x8da30c17,
	0xdf5fdb4f,
	0xab01e1f9,
	0xc8800807,
	0xa5e17c0f,
	0xce080417,
	0xb2e17807,
	0xf5418187,
	0xabff31f8,
	0x702bf944,
	0x25430887,
	0xe1840a2f,
	0x702bf933,
	0xab1da1f8,
	0x50e0b82f,
	0x2000008f,
	0xabfcb1ff,
	0xb6c055c7,
	0xab4be1f8,
	0x2900d51f,
	0xa380fe07,
	0xc880460f,
	0x53e270ff,
	0x50fbf55e,
	0x08801187,
	0x9000203f,
	0x28d1ffff,
	0xfc008087,
	0xabe241f8,
	0x0801610f,
	0x2000910f,
	0x393fe30f,
	0x355f5587,
	0x7260efc9,
	0x09808407,
	0x881f800f,
	0x0b54c1f8,
	0x5800610f,
	0x54e0450f,
	0x1000000b,
	0xf55f8047,
	0xab50c1f8,
	0x2120610f,
	0x2e015537,
	0xdf5f8147,
	0xab1361f8,
	0x3f11fe17,
	0xd55fd487,
	0x2601040f,
	0x9e015537,
	0x7061f71f,
	0x20987f6f,
	0x94020397,
	0x30807f2f,
	0x9eb95f97,
	0x3d025f8f,
	0x88807f60,
	0x24004b07,
	0xce00cb77,
	0xdf5f8147,
	0xab0a21f8,
	0x2eb2dc27,
	0xddb5cb2f,
	0x2601881f,
	0x9e01560f,
	0x27128617,
	0xd8805b47,
	0x26010407,
	0x98807f27,
	0x70a3ec1f,
	0x71b0b209,
	0x36b4c34f,
	0xdeb3c13f,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xab3961f8,
	0xf88459c7,
	0xab0121f8,
	0xe7b15407,
	0xf4a00195,
	0xb21f880f,
	0x36b2561f,
	0xd7b25207,
	0x7028fb32,
	0x23010507,
	0xc8807e1f,
	0x37bfcc27,
	0xc7bffe37,
	0x24025147,
	0x81824f3f,
	0x86abfc97,
	0xc8007e07,
	0x53dc31d7,
	0x50fbf53d,
	0x2400cb07,
	0xc100c807,
	0xc180c927,
	0xdf400247,
	0xabf9b1f8,
	0x26b2dc17,
	0xd887fe0f,
	0x2601841f,
	0x940a5b6f,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0x86abfbe7,
	0xc8007e07,
	0x53d6b1d7,
	0x50fbf53d,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabf001f8,
	0x54e0450f,
	0x1000000b,
	0xf55f8147,
	0xc800fe00,
	0x56e04108,
	0x2000000b,
	0x56ffc508,
	0x2000000b,
	0xab1f71ff,
	0x54e0410f,
	0x1000000b,
	0xc800fe0f,
	0x56e0c50f,
	0x2000000b,
	0xf55f8047,
	0xab3601f8,
	0x2120610f,
	0x2e015b37,
	0xdf5f8147,
	0xab1b21f8,
	0x28005b97,
	0x9f5fdb47,
	0x3e015b37,
	0x98807e38,
	0x0d5fda87,
	0xe403dbcf,
	0x7060f711,
	0x3804d1cf,
	0x38807e00,
	0x08804377,
	0x940059cf,
	0x20987f7f,
	0x98807f2f,
	0x2500db67,
	0x88c07f4f,
	0x24004b07,
	0xce00cb8f,
	0xdf5f8147,
	0xab0f21f8,
	0x2eb2e267,
	0xddb44b2f,
	0x2601985f,
	0x9e01504f,
	0x27129657,
	0xd8805f27,
	0x26011407,
	0x98807f1f,
	0x51d8133f,
	0x8000008f,
	0x71b0b20b,
	0x36b5c35f,
	0xdeb3c13f,
	0xab07f1ff,
	0xeeb1d00f,
	0x7028fb10,
	0xe3008107,
	0x2000010f,
	0x28804817,
	0x2eb6020f,
	0xd3827e1f,
	0x6071fff4,
	0x24024927,
	0x81824f3f,
	0x86abba8f,
	0xc8007e07,
	0x53c4b1d7,
	0x50fbf53d,
	0x36b1d01f,
	0xd7b1d607,
	0x7028fb32,
	0x23010507,
	0xc8807e1f,
	0x37bfcc27,
	0xc7bffe37,
	0x86abf94f,
	0xc8007e07,
	0x53c1f1d7,
	0x50fbf53d,
	0x2400cb07,
	0xc100c607,
	0xc180c71f,
	0xdf400247,
	0xab0401f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8147,
	0x90000138,
	0x2800d1c8,
	0x2380fe18,
	0xc8800010,
	0x87ff6a50,
	0xb600c747,
	0xabf651f8,
	0x91206377,
	0xf55f8187,
	0xab0f61f8,
	0x0bf631ff,
	0x5000813f,
	0x880059cf,
	0xf88401c7,
	0xab0121f8,
	0xe7b15a07,
	0xf4a00195,
	0xb21f880f,
	0x26b2e217,
	0xd887fe0f,
	0x2601841f,
	0x91965d77,
	0x27128627,
	0xd8804417,
	0x26010837,
	0x98807e1f,
	0x7020b265,
	0x26a2c207,
	0xd8804c27,
	0x30007e2f,
	0x97bffe37,
	0xc40a5f7f,
	0x86abf7ef,
	0xc8007e07,
	0x53f6f1d7,
	0x50fbf53c,
	0x2120610f,
	0x2100ca0f,
	0xc180cb2f,
	0xdf408147,
	0xabe941f8,
	0x54e0410f,
	0x1000000b,
	0xf55f8047,
	0xab00e1f8,
	0x56ffc10f,
	0x2000000b,
	0x2002510f,
	0x2888fe1f,
	0x50c17907,
	0x20000090,
	0x56c0f507,
	0x10000090,
	0x70b0e049,
	0x08804c47,
	0x921d11cf,
	0x6890ed09,
	0xeeb50957,
	0x0815510f,
	0x3801b10f,
	0x0801390f,
	0x3800c10f,
	0x0800490f,
	0x327901cf,
	0x60d40189,
	0x602c4889,
	0x66048189,
	0x9c1625cf,
	0xc1e473cf,
	0xf83d31ff,
	0x06b1d017,
	0xd8005bcf,
	0x7028fb21,
	0xe3010307,
	0x50c21977,
	0x20000003,
	0x38d1ffff,
	0xf88401c7,
	0xab03a1f8,
	0x2e9f641f,
	0xdf5fe547,
	0xe1bf860f,
	0xc1808207,
	0x20008000,
	0x7880600f,
	0xf4a001bd,
	0xa8030817,
	0xe7220c37,
	0xd00f8c37,
	0xf8148d27,
	0xf8a87e1f,
	0xb201880f,
	0x26b1d017,
	0xd880600f,
	0x7028fb20,
	0x33e0f1ff,
	0x5a380107,
	0x2000010f,
	0x28805017,
	0x27bffe27,
	0xc8805e0f,
	0x23827e1f,
	0xa8807e37,
	0x86abb58f,
	0xc8007e07,
	0x53e531d7,
	0x50fbf53c,
	0x3001013f,
	0x27b8e407,
	0x2080461f,
	0x98806027,
	0x87ff776f,
	0xabc371ff,
	0x255fd587,
	0xe8807f1f,
	0x2380fe00,
	0xa880ff87,
	0x20805408,
	0x98805518,
	0x53d620f8,
	0x50fbf55d,
	0x239671ff,
	0x58801180,
	0x0c05a107,
	0x10051107,
	0x24049d07,
	0x130d460f,
	0x2d84163f,
	0x91081447,
	0x20841637,
	0xae081427,
	0x2d84122f,
	0x90841217,
	0x38c30e07,
	0xe0c2101f,
	0x39bf8007,
	0xc0c10a17,
	0x08019107,
	0x36011d07,
	0x0e010547,
	0x68002107,
	0xab9fa1f8,
	0x61042fd3,
	0x0b9f31ff,
	0x56019d07,
	0x2800090f,
	0x2380fe17,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86abb90f,
	0xc8007e07,
	0x53d931d7,
	0x50fbf53c,
	0xabc7f1ff,
	0x2380fe17,
	0xa8800e07,
	0x7010f711,
	0x3401841f,
	0x87bffe27,
	0xc8807e37,
	0x86abb8b7,
	0xc8007e07,
	0x53d671d7,
	0x50fbf53c,
	0xabad31ff,
	0x9000210f,
	0x517cff57,
	0x90000007,
	0xf55f8187,
	0x0baaf1ff,
	0x50152908
};
const uint32_t BDSP_IMG_adpcm_decode_header [2] = {sizeof(BDSP_IMG_adpcm_decode_array1), 1};
const void * const BDSP_IMG_adpcm_decode [2] = {BDSP_IMG_adpcm_decode_header, BDSP_IMG_adpcm_decode_array1};
const uint32_t BDSP_IMG_adpcm_decode_tables_array1[] = {
	0x02000100,
	0x00c00000,
	0x01cc00f0,
	0x00000188,
	0x0000ff00,
	0x00000040,
	0xff18ff30,
	0x00e600e6,
	0x00e600e6,
	0x01990133,
	0x02660200,
	0x02660300,
	0x01990200,
	0x00e60133,
	0x00e600e6,
	0xffffffff,
	0xffffffff,
	0x00040002,
	0x00080006,
	0x00080007,
	0x000a0009,
	0x000c000b,
	0x000e000d,
	0x00110010,
	0x00150013,
	0x00190017,
	0x001f001c,
	0x00250022,
	0x002d0029,
	0x00370032,
	0x0042003c,
	0x00500049,
	0x00610058,
	0x0076006b,
	0x008f0082,
	0x00ad009d,
	0x00d100be,
	0x00fd00e6,
	0x01330117,
	0x01730151,
	0x01c10198,
	0x022001ee,
	0x02920256,
	0x031c02d4,
	0x03c3036c,
	0x048e0424,
	0x05830502,
	0x06ab0610,
	0x08120756,
	0x09c308e0,
	0x0bd00abd,
	0x0e4c0cff,
	0x114c0fba,
	0x14ee1307,
	0x19541706,
	0x1ea51bdc,
	0x251521b6,
	0x2cdf28ca,
	0x364b315b,
	0x41b23bb9,
	0x4f7e4844,
	0x602f5771,
	0x746269ce,
	0x00007fff,
	0x7fff7e01,
	0x7ffb6e1a,
	0x7ff34ea4,
	0x7fe7204d,
	0x7fd6e41c,
	0x7fc29b6d,
	0x7faa47f5,
	0x7f8debbd,
	0x7f6d8928,
	0x7f4922ec,
	0x7f20bc18,
	0x7ef4580c,
	0x7ec3fa83,
	0x7e8fa787,
	0x7e57637b,
	0x7e1b3310,
	0x7ddb1b50,
	0x7d972194,
	0x7d4f4b87,
	0x7d039f25,
	0x7cb422bc,
	0x7c60dce7,
	0x7c09d493,
	0x7baf10f7,
	0x7b50999b,
	0x7aee7651,
	0x7a88af36,
	0x7a1f4cb1,
	0x79b25775,
	0x7941d879,
	0x78cdd8ff,
	0x7856628c,
	0x77db7eeb,
	0x775d382a,
	0x76db989c,
	0x7656aad1,
	0x75ce799c,
	0x7543100d,
	0x74b47973,
	0x7422c157,
	0x738df37e,
	0x72f61be5,
	0x725b46c2,
	0x71bd807f,
	0x711cd5be,
	0x70795353,
	0x6fd30641,
	0x6f29fbc0,
	0x6e7e4133,
	0x6dcfe42c,
	0x6d1ef268,
	0x6c6b79cd,
	0x6bb5886b,
	0x6afd2c77,
	0x6a42744c,
	0x69856e68,
	0x68c6296a,
	0x6804b412,
	0x67411d3c,
	0x667b73e4,
	0x65b3c71e,
	0x64ea2617,
	0x641ea016,
	0x63514473,
	0x6282229f,
	0x61b14a18,
	0x60deca70,
	0x600ab345,
	0x5f351444,
	0x5e5dfd24,
	0x5d857da4,
	0x5caba58b,
	0x5bd084a8,
	0x5af42acb,
	0x5a16a7c7,
	0x59380b70,
	0x58586599,
	0x5777c611,
	0x56963ca2,
	0x55b3d912,
	0x54d0ab1b,
	0x53ecc272,
	0x53082ebb,
	0x5222ff91,
	0x513d447f,
	0x50570d00,
	0x4f70687a,
	0x4e896644,
	0x4da2159c,
	0x4cba85ab,
	0x4bd2c581,
	0x4aeae414,
	0x4a02f03f,
	0x491af8c1,
	0x48330c38,
	0x474b3925,
	0x46638de6,
	0x457c18b8,
	0x4494e7b2,
	0x43ae08c6,
	0x42c789c2,
	0x41e17849,
	0x40fbe1d7,
	0x4016d3bb,
	0x3f325b1b,
	0x3e4e84f0,
	0x3d6b5e03,
	0x3c88f2f0,
	0x3ba75023,
	0x3ac681d4,
	0x39e6940d,
	0x390792a2,
	0x38298933,
	0x374c832d,
	0x36708bc5,
	0x3595adfa,
	0x34bbf493,
	0x33e36a21,
	0x330c18f9,
	0x32360b37,
	0x31614abe,
	0x308de134,
	0x2fbbd806,
	0x2eeb3861,
	0x2e1c0b38,
	0x2d4e5941,
	0x2c822af2,
	0x2bb78887,
	0x2aee79f8,
	0x2a270704,
	0x29613727,
	0x289d119f,
	0x27da9d6a,
	0x2719e148,
	0x265ae3b7,
	0x259daaf7,
	0x24e23d06,
	0x24289fa3,
	0x2370d84e,
	0x22baec45,
	0x2206e088,
	0x2154b9d5,
	0x20a47cae,
	0x1ff62d52,
	0x1f49cfc2,
	0x1e9f67c1,
	0x1df6f8d2,
	0x1d50863c,
	0x1cac1305,
	0x1c09a1f8,
	0x1b6935a2,
	0x1acad054,
	0x1a2e7422,
	0x199422e6,
	0x18fbde3d,
	0x1865a78c,
	0x17d17ffd,
	0x173f6881,
	0x16af61d1,
	0x16216c6f,
	0x159588a6,
	0x150bb68a,
	0x1483f5fc,
	0x13fe46a5,
	0x137aa800,
	0x12f91950,
	0x127999aa,
	0x11fc27f1,
	0x1180c2d8,
	0x110768e5,
	0x1090186f,
	0x101acfa0,
	0x0fa78c78,
	0x0f364ccc,
	0x0ec70e47,
	0x0e59ce6c,
	0x0dee8a97,
	0x0d854001,
	0x0d1debb9,
	0x0cb88aae,
	0x0c5519ac,
	0x0bf3955f,
	0x0b93fa51,
	0x0b3644ef,
	0x0ada7189,
	0x0a807c52,
	0x0a286161,
	0x09d21cb8,
	0x097daa3c,
	0x092b05be,
	0x08da2af9,
	0x088b1593,
	0x083dc120,
	0x07f22922,
	0x07a8490c,
	0x07601c40,
	0x07199e14,
	0x06d4c9d1,
	0x06919ab7,
	0x06500bfa,
	0x061018c7,
	0x05d1bc45,
	0x0594f193,
	0x0559b3cf,
	0x051ffe10,
	0x04e7cb70,
	0x04b11706,
	0x047bdbea,
	0x04481537,
	0x0415be0a,
	0x03e4d188,
	0x03b54ad9,
	0x0387252d,
	0x035a5bbd,
	0x032ee9cb,
	0x0304caa5,
	0x02dbf9a4,
	0x02b4722d,
	0x028e2fb7,
	0x02692dc5,
	0x024567eb,
	0x0222d9d0,
	0x02017f2d,
	0x01e153ce,
	0x01c25394,
	0x01a47a77,
	0x0187c485,
	0x016c2de1,
	0x0151b2cb,
	0x01384f99,
	0x012000bc,
	0x0108c2c2,
	0x00f29252,
	0x00dd6c32,
	0x00c94d44,
	0x00b6328a,
	0x00a41922,
	0x0092fe4d,
	0x0082df69,
	0x0073b9f8,
	0x00658b9b,
	0x00585215,
	0x004c0b4d,
	0x0040b54c,
	0x00364e3f,
	0x002cd475,
	0x00244664,
	0x001ca2a5,
	0x0015e7f6,
	0x00101539,
	0x000b2979,
	0x000723e3,
	0x000403cb,
	0x0001c8ac,
	0x00007226,
	0x00000000
};
const uint32_t BDSP_IMG_adpcm_decode_tables_header [2] = {sizeof(BDSP_IMG_adpcm_decode_tables_array1), 1};
const void * const BDSP_IMG_adpcm_decode_tables [2] = {BDSP_IMG_adpcm_decode_tables_header, BDSP_IMG_adpcm_decode_tables_array1};
const uint32_t BDSP_IMG_adpcm_decode_inter_frame_array1[] = {
	0x00000000,
	0x00000f0c
};
const uint32_t BDSP_IMG_adpcm_decode_inter_frame_header [2] = {sizeof(BDSP_IMG_adpcm_decode_inter_frame_array1), 1};
const void * const BDSP_IMG_adpcm_decode_inter_frame [2] = {BDSP_IMG_adpcm_decode_inter_frame_header, BDSP_IMG_adpcm_decode_inter_frame_array1};
/* End of File */
