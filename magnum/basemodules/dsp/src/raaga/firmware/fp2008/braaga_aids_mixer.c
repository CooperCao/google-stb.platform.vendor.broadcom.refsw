/*******************************************************************************
 * Copyright (C) 2019 Broadcom.
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
const uint32_t BDSP_IMG_mixer_ids_array1[] = {
	0x97ed73cb,
	0x3de873cf,
	0x00fffffe,
	0x960475cb,
	0xba1cc0a7,
	0xba1cc4b7,
	0x46000307,
	0x30017e07,
	0xba1cc8c7,
	0xba1cccd7,
	0xba1cd0e7,
	0xba1cd4f7,
	0xba1cd907,
	0xba1cdd17,
	0xba1ce123,
	0xa75409d7,
	0x001fd929,
	0x1b5f9143,
	0xb43101fc,
	0x07a47f1f,
	0x2b00fe1f,
	0x3dbe4007,
	0x3daa420f,
	0x2b2c7e17,
	0x94320707,
	0x94000707,
	0x940e7f07,
	0x940c7f07,
	0x94107d07,
	0x94347d07,
	0x94307f07,
	0x942e7f07,
	0x94127f07,
	0x94147f07,
	0x94167f07,
	0x94187f07,
	0x941a7f07,
	0x941c7f07,
	0x941e7f07,
	0x3de0433f,
	0xa72309d7,
	0x001fd934,
	0x3db87337,
	0x07bffe17,
	0x46134e07,
	0x2b167e1f,
	0x461ffe27,
	0x07bffe37,
	0x3dda4317,
	0x00000008,
	0xa72389d7,
	0x001ff934,
	0x07bffe07,
	0x3da67357,
	0xa72a09d7,
	0x001fd927,
	0x46154407,
	0x07bffe17,
	0x2b127e1f,
	0x461ffe27,
	0x07bffe37,
	0x3da64517,
	0xa76989d7,
	0x001ff935,
	0x07bffe03,
	0xa7270dd7,
	0x001fd927,
	0x74544917,
	0x74700317,
	0x2b1e7e13,
	0x3d984803,
	0xa71b0dd7,
	0x001fd934,
	0x743801cf,
	0x94067d07,
	0x3d824923,
	0x1b5f0143,
	0xb42c01f8,
	0x94080103,
	0x3dde436f,
	0x4610c347,
	0x07bfff2f,
	0x07bfff67,
	0x2b0e7f7f,
	0x3d90735f,
	0x3d8c7373,
	0x6402016b,
	0x15008143,
	0xb42001f8,
	0x3d80cb2f,
	0x3dd4534f,
	0x15024b47,
	0x3d8e5147,
	0xb6fe09f8,
	0x07b1fe03,
	0xa77e8dd7,
	0x001fd929,
	0x74060107,
	0x74080307,
	0x7456050b,
	0xa74409d7,
	0x00000000,
	0x74540717,
	0x46154e07,
	0x940a1107,
	0x07b37e17,
	0xa74509d7,
	0x00000000,
	0x74700113,
	0x740c0203,
	0x15400347,
	0x000000bb,
	0xb42101f8,
	0x744a01cb,
	0x15008143,
	0xb60305f8,
	0x744c01cf,
	0x744803cf,
	0x2bc07e17,
	0x000000bb,
	0x944a7fcf,
	0x94387dcf,
	0x94080507,
	0x94060507,
	0x013fffff,
	0x15008287,
	0x15008087,
	0x94487fc8,
	0x944c7fc8,
	0x74080f07,
	0x74061107,
	0x74660117,
	0x2b00fe0f,
	0x013fffff,
	0x941c03cf,
	0x942403cb,
	0x941a0fcf,
	0x942211cf,
	0x15020143,
	0xb41981f8,
	0xd0018150,
	0x21010e01,
	0x941e01c9,
	0x941e0fc8,
	0x743801cb,
	0x1b400e83,
	0xb60481f8,
	0x745e0317,
	0x7400070f,
	0x745c0117,
	0x461ffe37,
	0x2b04fe17,
	0x3d9a7227,
	0x2b0c7e2f,
	0x94087fcf,
	0xa76789d7,
	0x001ff92a,
	0x2a927e03,
	0xa73789d7,
	0x001fd926,
	0x2aa97e03,
	0xa7368dd7,
	0x001fd926,
	0x74061107,
	0x74080f07,
	0x741a010b,
	0x1b5f8143,
	0xb60085f8,
	0x745a0313,
	0x15008343,
	0xb41f81f8,
	0x7400130f,
	0x46134e07,
	0x07bffe17,
	0x2b167e1f,
	0x461ffe27,
	0x07bffe37,
	0x944e7d07,
	0x94380fcf,
	0x943a11cb,
	0x94541303,
	0xa70b8dd7,
	0x001ff934,
	0x07bffe03,
	0xa70989d7,
	0x001fd927,
	0x740c0127,
	0x741a030f,
	0xd0008150,
	0x2a96fe05,
	0x94140327,
	0x2a95fe04,
	0x94327f01,
	0x94007f01,
	0xa72e89d7,
	0x001fd926,
	0x760475cf,
	0x740e0127,
	0x540b45cf,
	0x540c49cf,
	0x540d4dcf,
	0x540e51cf,
	0x540f55cf,
	0x541059cf,
	0x013fffff,
	0x1b5f8177,
	0x1b5f8087,
	0x943a7f06,
	0x54115dcf,
	0x541261cf,
	0x2b00fe00,
	0x943a0104,
	0x540a41cf,
	0x3d9873cf,
	0x00000001,
	0xa9fd0ffb,
	0xb6ff8dfb,
	0x05b7ca17,
	0x746a014f,
	0x07b5fe0b,
	0x3dba518f,
	0x00000003,
	0x013fffff,
	0x15010087,
	0x3dbc0587,
	0x00000003,
	0x08584383,
	0xb6dd01fc,
	0x07b87e07,
	0x7400058b,
	0xa7410dd7,
	0x001ff934,
	0x7400058f,
	0x46175607,
	0xa7590dd7,
	0x001fd927,
	0x740c01cb,
	0x1b5f8083,
	0xb40505f8,
	0x743a01cb,
	0xb6d98dff,
	0x94060107,
	0x21020e03,
	0xb6e68dff,
	0x941e01cf,
	0x740e0203,
	0x15400347,
	0x000000bb,
	0xb6de01f8,
	0x740a0003,
	0x15400087,
	0x000000bb,
	0xb4dd01f8,
	0xb6e08dfb,
	0xa70c09d7,
	0x00000000,
	0x74700313,
	0x8444000b,
	0xb6d28dff,
	0x94080107,
	0x7400038f,
	0x461fc617,
	0x30017e27,
	0x07bffe2f,
	0x461ffe37,
	0x07b87e07,
	0xa73d89d7,
	0x001ff934,
	0x07bffe03,
	0xa7740dd7,
	0x001fd926,
	0x1b5fd943,
	0xb60201f8,
	0x7408051b,
	0x07a17e03,
	0x94060503,
	0xa70689d7,
	0x00000000,
	0x74700313,
	0x8444000b,
	0x94080103,
	0x7400011b,
	0x151b0147,
	0x00266646,
	0xb6cf01f8,
	0x7450011b,
	0x1b5f8143,
	0xb6ce05f8,
	0x7408011f,
	0x2b00ff67,
	0x94060103,
	0xa7028dd7,
	0x00000000,
	0x74700313,
	0x8444000b,
	0xb6cc09ff,
	0x94080107,
	0x743a010b,
	0x15018083,
	0x94360100,
	0x940c0324,
	0x94320100,
	0xb6de8dff,
	0x942e7d00,
	0x15220087,
	0x000000ac,
	0xa9fd07fc,
	0x2b03fe47,
	0x112200c7,
	0x000000ac,
	0xb40501f8,
	0x15000087,
	0x00000177,
	0xa9fd07fc,
	0x2b05fe47,
	0x110000c7,
	0x00000177,
	0xb40805f8,
	0x15080087,
	0x000002b1,
	0xa9fd07fc,
	0x2b06fe47,
	0x15000087,
	0x000002ee,
	0xa9fd07fc,
	0x2b077e47,
	0x15000087,
	0x000001f4,
	0xb40781f8,
	0xa9fd0fff,
	0x2b067e47,
	0x15400087,
	0x0000003e,
	0xa9fd07fc,
	0x2b01fe47,
	0x114000c7,
	0x0000003e,
	0xb40585f8,
	0x15600087,
	0x0000005d,
	0xa9fd07fc,
	0x2b02fe47,
	0x15000087,
	0x0000007d,
	0xa9fd07fc,
	0x2b037e47,
	0x15110087,
	0x00000056,
	0xb40281f8,
	0xa9fd0fff,
	0x2b027e47,
	0x15000087,
	0x000000fa,
	0xa9fd07fc,
	0x2b04fe47,
	0x15440087,
	0x00000158,
	0xa9fd07fc,
	0x2b057e47,
	0xa9fd0fff,
	0x2b047e47,
	0x15088087,
	0x0000002b,
	0xa9fd07fc,
	0x2b00fe47,
	0x15700087,
	0x0000002e,
	0xa9fd07fc,
	0x2b017e47,
	0x15200087,
	0x0000001f,
	0xb4fc85f8,
	0xa9fd0fff,
	0x07bffe47,
	0x15008487,
	0x1501854f,
	0x150204b7,
	0x07a07e47,
	0xa9fd07f8,
	0xa9dd07fd,
	0x21010247,
	0x07a0fe43,
	0xa9fd0fff,
	0x21020246,
	0x97ebf3cb,
	0x3ddc73cf,
	0x00fffffe,
	0x960475cb,
	0x944c03cf,
	0x971465cf,
	0xba1cc8d7,
	0xba1cd0f7,
	0x07a1ff93,
	0xba1cd507,
	0xba1cd917,
	0xba1ce137,
	0xba1ccce7,
	0x3d8a733b,
	0xba1cc0b7,
	0xba1cc4c7,
	0x46138507,
	0x07a07f17,
	0xba1cdd27,
	0x3d927377,
	0x30017e03,
	0xa77289d7,
	0x001fd928,
	0x8c02110b,
	0x1b5f9143,
	0xb41e81f8,
	0x1b574343,
	0xb6fe09f8,
	0x74045107,
	0x74064d07,
	0x74084707,
	0x740a4907,
	0x740c5307,
	0x740e5507,
	0x07bffe1f,
	0x07bffe07,
	0x3d947387,
	0x3d9e737f,
	0x2b027e0b,
	0xa801f40b,
	0x013fffff,
	0x84400517,
	0x9e407f83,
	0x013fffff,
	0x15008487,
	0x9e40057f,
	0x3d808618,
	0x3d808003,
	0x15008747,
	0x94267fcf,
	0xac1f8e04,
	0x941c7fcf,
	0x904e01cf,
	0x07b17f67,
	0x07b17f5f,
	0x07bfff0f,
	0x3d88732b,
	0x8450db7b,
	0x1500db43,
	0xb41605f8,
	0x1b5fdb43,
	0xb60181f8,
	0x1b50d083,
	0xb64381f8,
	0x1b50cc83,
	0xb64281f8,
	0x1b50c683,
	0xb64705f8,
	0x3d80c30f,
	0x3d8e5967,
	0x15024347,
	0x3dd4575f,
	0xb6fc09f8,
	0x2101510f,
	0x94084707,
	0x0850ca07,
	0x94045107,
	0x74160207,
	0x94064d07,
	0x15008343,
	0xb43605f8,
	0x2b017e07,
	0x07bfff5f,
	0x940c0103,
	0x21014d7b,
	0x0857ca03,
	0x74160203,
	0x15008343,
	0xb42f01f8,
	0x2b017e07,
	0x07bfff0f,
	0x940e0103,
	0x2101460f,
	0x07bfff6f,
	0x0840ca03,
	0x74165803,
	0x1500d943,
	0xb43b81f8,
	0x1b5fd683,
	0xb61c05f8,
	0x0857cb2b,
	0x7416012b,
	0x15008143,
	0xb42a81f8,
	0x07b5ff0b,
	0x1500d947,
	0x07bfff5f,
	0xb42405fc,
	0x07bfff67,
	0x07b0fe0f,
	0x3990430f,
	0x117f8247,
	0x0000001f,
	0x30107e08,
	0x07b97e07,
	0x2b00fe17,
	0x1500d683,
	0x8d020407,
	0x94025197,
	0x94024c03,
	0x94044607,
	0x94065207,
	0x940a5407,
	0x94084807,
	0x940c5607,
	0x940e0207,
	0x94105807,
	0x94124207,
	0x94145700,
	0x6402013b,
	0xa71609d7,
	0x001fd929,
	0x1b574f43,
	0xb6fe8df8,
	0x760475cf,
	0x771465cf,
	0x540b41cf,
	0x540c45cf,
	0x540d49cf,
	0x540e4dcf,
	0x540f51cf,
	0x541055cf,
	0x541159cf,
	0x54125dcf,
	0x541361cb,
	0x3da473cf,
	0x00000001,
	0xa9fd0ffb,
	0xb6ff8dfb,
	0x740a015b,
	0x15010143,
	0xb60185f8,
	0x704e01cb,
	0xafe07fc3,
	0xb41b81f8,
	0x15024743,
	0xb43801f8,
	0xb6e90dff,
	0x07b0ff1f,
	0x2b0e7e17,
	0x3db2720f,
	0x05a14247,
	0x2101c38f,
	0x3dda5837,
	0x00000002,
	0x74000433,
	0x96540dcb,
	0x3ddc103f,
	0x00000002,
	0x0843c423,
	0x07a27e03,
	0x965009cb,
	0xa76f8dd7,
	0x001ff933,
	0x765407cf,
	0x3dbc722f,
	0x3db27207,
	0x08588a0f,
	0x7400041b,
	0xa70709d7,
	0x001fd927,
	0x3dbc720f,
	0x765407cf,
	0x8400818f,
	0x765009cf,
	0x117f8147,
	0x00000001,
	0xb6e201f8,
	0x3d8c7217,
	0x7400021f,
	0x8450e217,
	0x07a27e07,
	0x07bffe2f,
	0x30017e27,
	0x461ffe33,
	0x461fe213,
	0xa76f09d7,
	0x001ff933,
	0x07bffe03,
	0xa7258dd7,
	0x001fd926,
	0x7400018b,
	0x151b0147,
	0x00266646,
	0xb6dd85f8,
	0x7450018b,
	0x9e50db83,
	0x1b5f8083,
	0xb64381f8,
	0x15008143,
	0x03f0cd30,
	0xb6db8dff,
	0x03f1c318,
	0x1b5fc283,
	0xb40201f8,
	0x1500d947,
	0x2b00ff5f,
	0xb42205f8,
	0x30087f67,
	0x07bffe0f,
	0x2b807f0f,
	0x00ffffe0,
	0xb6e48dfb,
	0x1500d947,
	0x2b017f5f,
	0xb6fe09f8,
	0x2b547e0f,
	0x3db27387,
	0x05a0c617,
	0x07b87e0f,
	0x2b027f5f,
	0x30087f0f,
	0x07bfff63,
	0x0841457b,
	0x740a057f,
	0x3d8c041f,
	0x013fffff,
	0x0841c407,
	0xa77d0dd7,
	0x001ff933,
	0x740a057f,
	0x4612e007,
	0xa7770dd7,
	0x001fd926,
	0x740801cb,
	0x117f80c7,
	0x0000000f,
	0xb60181f8,
	0x2a90fe03,
	0xa7408dd7,
	0x001fd925,
	0x2abafe03,
	0xa74009d7,
	0x001fd925,
	0x2b547e13,
	0x05a1460b,
	0x0840c513,
	0x740a0113,
	0x15010083,
	0xb6da05f8,
	0x1b5fda83,
	0xb6d985f8,
	0x1a56c3c3,
	0xb6d909ff,
	0x03f6c308,
	0x74100103,
	0x15008083,
	0xb60085f8,
	0x74140103,
	0x15008143,
	0xb6e301f8,
	0x2b00fe03,
	0xb6e20dff,
	0x94120107,
	0x1a55c3c3,
	0xb6d509ff,
	0x03f5c308,
	0x740c0203,
	0x15008343,
	0xb43205f8,
	0x2b547e0f,
	0x3db27367,
	0x05a0cc17,
	0x07b67e0f,
	0x740e5503,
	0x0841450b,
	0x740a050f,
	0x3d8c0407,
	0x013fffff,
	0x08404407,
	0xa7520dd7,
	0x001ff933,
	0x740a050f,
	0x4612d807,
	0xa76a0dd7,
	0x001fd926,
	0xb6cc89ff,
	0x740843cf,
	0x740c0203,
	0x15008343,
	0xb41b81f8,
	0x740c5307,
	0x740a4907,
	0x2b547e27,
	0x3db27367,
	0x05a25017,
	0x2b00fe0f,
	0x07b67e0f,
	0x94100307,
	0x0850cb0b,
	0x0841455b,
	0x740a055f,
	0x3d8c041f,
	0x013fffff,
	0x0841c407,
	0xa74c09d7,
	0x001ff933,
	0x740a055f,
	0x4612d807,
	0xa76409d7,
	0x001fd926,
	0x740c010f,
	0x740857cf,
	0x15008143,
	0xb6c385f8,
	0x7404010b,
	0x74540003,
	0x1b5f0083,
	0xb6c285f8,
	0x21010003,
	0xb6c209ff,
	0x0a40575f,
	0xb6bd89ff,
	0x2b027f37,
	0xb6bd09ff,
	0x2b027f47,
	0x2b547e0f,
	0x3db27387,
	0x05a0c617,
	0x07b87e0f,
	0x0841456b,
	0x740a056f,
	0x3d8c0407,
	0x013fffff,
	0x08404407,
	0xa76289d7,
	0x001ff933,
	0x740a056f,
	0x4612e007,
	0xa75c89d7,
	0x001fd926,
	0xb6c00dff,
	0x74085bcf,
	0xb6b80dff,
	0x2b027f1f,
	0x2b547e0f,
	0x3db2735f,
	0x05a0c617,
	0x07b5fe0f,
	0x0841450b,
	0x740a050f,
	0x3d8c041f,
	0x013fffff,
	0x0841c407,
	0xa75d8dd7,
	0x001ff933,
	0x740a050f,
	0x4612d607,
	0xa7578dd7,
	0x001fd926,
	0x740a010b,
	0x15010147,
	0x740801cf,
	0xb40601f8,
	0x117f8047,
	0x0000000f,
	0x30087e04,
	0x2b01ff5f,
	0x07a07f0f,
	0x07bfff67,
	0xb6e00dfb,
	0x2b547e0b,
	0x05a0c21f,
	0x3da8720f,
	0x3d8c0613,
	0x08414403,
	0xa74589d7,
	0x001ff92a,
	0x740a055f,
	0x3da87207,
	0x07b2fe0b,
	0xa75209d7,
	0x001fd926,
	0x740801cb,
	0x117f80c7,
	0x00000017,
	0xb6ad89ff,
	0x03f1c318,
	0x1b5f8083,
	0xb60185f8,
	0x744c0bcb,
	0x7408022b,
	0x15008343,
	0xb6f881f8,
	0x117f8147,
	0x0000000f,
	0xb4f785f8,
	0x2b00ff5f,
	0x07bfff0f,
	0xb6d88dff,
	0x30087f67,
	0x74045603,
	0x7432535b,
	0x3f905203,
	0x013fffff,
	0x1511d287,
	0x742e015f,
	0x13008140,
	0x013fffff,
	0x2b017f48,
	0x110300c3,
	0xb60605f8,
	0x110200c7,
	0x2b027f27,
	0xb60201f8,
	0x15008087,
	0x2b00ff27,
	0xb60105f8,
	0x1b5f8083,
	0xb60081f8,
	0x11010143,
	0xb41381f8,
	0x2b017f23,
	0x3900d75f,
	0x7430035f,
	0x7402015b,
	0x013fffff,
	0x15008287,
	0x15010147,
	0x3d80c920,
	0xb40685f8,
	0x7402015b,
	0x15008143,
	0xb40305f8,
	0x940c5307,
	0x940a4907,
	0xb6dc09fb,
	0x15048083,
	0xb61101f8,
	0x110480c3,
	0xb60f05f8,
	0x15038083,
	0xb61001f8,
	0x15040083,
	0xb4fa05f8,
	0xb6fa09ff,
	0x2b037f27,
	0x3f82c803,
	0x11008143,
	0xb6fc01f8,
	0x2a90fe07,
	0x2b017f27,
	0xa70b89d7,
	0x001fd925,
	0x2a987e03,
	0xa70a8dd7,
	0x001fd925,
	0xb6fa09fb,
	0x3f82c803,
	0x11008143,
	0xb6f881f8,
	0x2a90fe07,
	0x2b037f27,
	0xa7088dd7,
	0x001fd925,
	0x2a98fe03,
	0xa70809d7,
	0x001fd925,
	0xb6f689fb,
	0xb6988dff,
	0x07b0ff47,
	0x74044203,
	0x7432550b,
	0x3f905403,
	0x1511d483,
	0x13008140,
	0xb40005f8,
	0x2b017f53,
	0x2b547e0f,
	0x3db2736f,
	0x05a0cc17,
	0x07b6fe0f,
	0x08414563,
	0x740a0567,
	0x3d8c0407,
	0x013fffff,
	0x08404407,
	0xa71e89d7,
	0x001ff933,
	0x740a0567,
	0x4612da07,
	0xa73689d7,
	0x001fd926,
	0x7454010f,
	0x740843cf,
	0x1b5f0083,
	0xb69805f8,
	0x21010003,
	0xb69789ff,
	0x0a40430f,
	0xb6ec0dff,
	0x2b01ff27,
	0x15110083,
	0xb60085f8,
	0x15128083,
	0xb4eb01f8,
	0xb6f089fb,
	0xb6ea89ff,
	0x2b02ff27
};
const uint32_t BDSP_IMG_mixer_ids_header [2] = {sizeof(BDSP_IMG_mixer_ids_array1), 1};
const void * const BDSP_IMG_mixer_ids [2] = {BDSP_IMG_mixer_ids_header, BDSP_IMG_mixer_ids_array1};
const uint32_t BDSP_IMG_mixer_ids_inter_frame_array1[] = {
	0xffffffff,
	0x00000002,
	0x00000004,
	0x00000003,
	0x00000002,
	0x00000003,
	0x00000000,
	0x00000003
};
const uint32_t BDSP_IMG_mixer_ids_inter_frame_header [2] = {sizeof(BDSP_IMG_mixer_ids_inter_frame_array1), 1};
const void * const BDSP_IMG_mixer_ids_inter_frame [2] = {BDSP_IMG_mixer_ids_inter_frame_header, BDSP_IMG_mixer_ids_inter_frame_array1};
/* End of File */
