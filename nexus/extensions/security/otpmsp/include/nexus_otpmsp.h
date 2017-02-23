/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef NEXUS_OTPMSP_H__
#define NEXUS_OTPMSP_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
Summary:
This enum defines types as passed by the MIPS to the 8051 that say whether the programming
command is in bit mode or array mode

Description:
This enumeration defines all the supported programming modes.
*****************************************************************************/
typedef enum NEXUS_OtpMspCmdProgMode
{
    NEXUS_OtpMspCmdProgMode_eEnum    = 0x10112,
    NEXUS_OtpMspCmdProgMode_eEnumHal = 0x34,
    NEXUS_OtpMspCmdProgMode_eBit     = 0x78
} NEXUS_OtpMspCmdProgMode;


/*****************************************************************************
Summary:
*****************************************************************************/
typedef enum NEXUS_OtpCmdMsp
{
    NEXUS_OtpCmdMsp_eReserved0                              =   0,
    NEXUS_OtpCmdMsp_eReserved1                              =   1,
    NEXUS_OtpCmdMsp_eReserved2                              =   2,
    NEXUS_OtpCmdMsp_eReserved3                              =   3,
    NEXUS_OtpCmdMsp_eReserved4                              =   4,
    NEXUS_OtpCmdMsp_eCrLockEnable                           =   5,
    NEXUS_OtpCmdMsp_eCrDisable                              =   6,
    NEXUS_OtpCmdMsp_ePcie0HostProtect                       =   7,
    NEXUS_OtpCmdMsp_ePcie0ClientProtect                     =   8,
    NEXUS_OtpCmdMsp_ePcie1HostProtect                       =   9,
    NEXUS_OtpCmdMsp_ePcie1ClientProtect                     =   10,
    NEXUS_OtpCmdMsp_eReserved11                             =   11,
    NEXUS_OtpCmdMsp_eReserved12                             =   12,
    NEXUS_OtpCmdMsp_eReserved13                             =   13,
    NEXUS_OtpCmdMsp_eReserved14                             =   14,
    NEXUS_OtpCmdMsp_eReserved15                             =   15,
    NEXUS_OtpCmdMsp_eReserved16                             =   16,
    NEXUS_OtpCmdMsp_eReserved17                             =   17,
    NEXUS_OtpCmdMsp_eForceDramScrambler                     =   18,
    NEXUS_OtpCmdMsp_eReserved19                             =   19,
    NEXUS_OtpCmdMsp_eReserved20                             =   20,
    NEXUS_OtpCmdMsp_eReserved21                             =   21,
    NEXUS_OtpCmdMsp_eReserved22                             =   22,
    NEXUS_OtpCmdMsp_eReserved23                             =   23,
    NEXUS_OtpCmdMsp_eReserved24                             =   24,
    NEXUS_OtpCmdMsp_eReserved25                             =   25,
    NEXUS_OtpCmdMsp_eReserved26                             =   26,
    NEXUS_OtpCmdMsp_eReserved27                             =   27,
    NEXUS_OtpCmdMsp_eReserved28                             =   28,
    NEXUS_OtpCmdMsp_eReserved29                             =   29,
    NEXUS_OtpCmdMsp_eReserved30                             =   30,
    NEXUS_OtpCmdMsp_eReserved31                             =   31,
    NEXUS_OtpCmdMsp_eReserved32                             =   32,
    NEXUS_OtpCmdMsp_eReserved33                             =   33,
    NEXUS_OtpCmdMsp_eReserved34                             =   34,
    NEXUS_OtpCmdMsp_eDataSectionLockByte                    =   35,
    NEXUS_OtpCmdMsp_eDataSectionReadProtectBits             =   36,
    NEXUS_OtpCmdMsp_eDestinationDisallowKeyA                =   37,
    NEXUS_OtpCmdMsp_eDestinationDisallowKeyB                =   38,
    NEXUS_OtpCmdMsp_eDestinationDisallowKeyC                =   39,
    NEXUS_OtpCmdMsp_eReserved40                             =   40,
    NEXUS_OtpCmdMsp_eReserved41                             =   41,
    NEXUS_OtpCmdMsp_eReserved42                             =   42,
    NEXUS_OtpCmdMsp_eAskmStbOwnerId                         =   43,
    NEXUS_OtpCmdMsp_eReserved44                             =   44,
    NEXUS_OtpCmdMsp_eCaControlBits                          =   45,
    NEXUS_OtpCmdMsp_eCpControlBits                          =   46,
    NEXUS_OtpCmdMsp_eCaKeyLadderDisable                     =   47,
    NEXUS_OtpCmdMsp_eCpKeyLadderDisable                     =   48,
    NEXUS_OtpCmdMsp_eCustKeytoCaKeyLadderDisable            =   49,
    NEXUS_OtpCmdMsp_eCustKeytoCpKeyLadderDisable            =   50,
    NEXUS_OtpCmdMsp_eReserved51                             =   51,
    NEXUS_OtpCmdMsp_eReserved52                             =   52,
    NEXUS_OtpCmdMsp_eReserved53                             =   53,
    NEXUS_OtpCmdMsp_eReserved54                             =   54,
    NEXUS_OtpCmdMsp_eReserved55                             =   55,
    NEXUS_OtpCmdMsp_eReserved56                             =   56,
    NEXUS_OtpCmdMsp_eReserved57                             =   57,
    NEXUS_OtpCmdMsp_eRouted3DesKeyK1K2CheckEnable           =   58,
    NEXUS_OtpCmdMsp_eCaSoftwareKeyDisable                   =   59,
    NEXUS_OtpCmdMsp_eCpAesEcbMscSoftwareKeyDisable          =   60,
    NEXUS_OtpCmdMsp_eCpM2mDesSoftwareKeyDisable             =   61,
    NEXUS_OtpCmdMsp_eCpM2m3DesSoftwareKeyDisable            =   62,
    NEXUS_OtpCmdMsp_eCpM2mAesSoftwareKeyDisable             =   63,
    NEXUS_OtpCmdMsp_eCpM2mAesCounterSoftwareKeyDisable      =   64,
    NEXUS_OtpCmdMsp_eM2mC2CssSoftwareKeyDisable             =   65,
    NEXUS_OtpCmdMsp_eM2mM6SoftwareKeyDisable                =   66,
    NEXUS_OtpCmdMsp_eReserved67                             =   67,
    NEXUS_OtpCmdMsp_eReserved68                             =   68,
    NEXUS_OtpCmdMsp_eReserved69                             =   69,
    NEXUS_OtpCmdMsp_eReserved70                             =   70,
    NEXUS_OtpCmdMsp_eReserved71                             =   71,
    NEXUS_OtpCmdMsp_eAskmCAVendorIdSelect                   =   72,
    NEXUS_OtpCmdMsp_ePublicKey0Index                        =   73,
    NEXUS_OtpCmdMsp_eSecureBootEnable                       =   74,
    NEXUS_OtpCmdMsp_eReserved75                             =   75,
    NEXUS_OtpCmdMsp_eRaagaAVerifyEnable                     =   76,
    NEXUS_OtpCmdMsp_eVideoVerifyEnable                      =   77,
    NEXUS_OtpCmdMsp_eRaveVerifyEnable                       =   78,
    NEXUS_OtpCmdMsp_eHostBootCodeHideEnable                 =   79,
    NEXUS_OtpCmdMsp_eHostBootCodeDecryptionSelect           =   80,
    NEXUS_OtpCmdMsp_eReserved81                             =   81,
    NEXUS_OtpCmdMsp_eReserved82                             =   82,
    NEXUS_OtpCmdMsp_eReserved83                             =   83,
    NEXUS_OtpCmdMsp_eReserved84                             =   84,
    NEXUS_OtpCmdMsp_eReserved85                             =   85,
    NEXUS_OtpCmdMsp_eReserved86                             =   86,
    NEXUS_OtpCmdMsp_eSystemEpoch0                           =   87,
    NEXUS_OtpCmdMsp_eReserved88                             =   88,
    NEXUS_OtpCmdMsp_eBseckEnable                            =   89,
    NEXUS_OtpCmdMsp_eReserved90                             =   90,
    NEXUS_OtpCmdMsp_eReserved91                             =   91,
    NEXUS_OtpCmdMsp_eReserved92                             =   92,
    NEXUS_OtpCmdMsp_eReserved93                             =   93,
    NEXUS_OtpCmdMsp_eMarketId0                              =   94,
    NEXUS_OtpCmdMsp_eReserved95                             =   95,
    NEXUS_OtpCmdMsp_eReserved96                             =   96,
    NEXUS_OtpCmdMsp_eReserved97                             =   97,
    NEXUS_OtpCmdMsp_eReserved98                             =   98,
    NEXUS_OtpCmdMsp_eReserved99                             =   99,
    NEXUS_OtpCmdMsp_eReserved100                            =   100,
    NEXUS_OtpCmdMsp_eReserved101                            =   101,
    NEXUS_OtpCmdMsp_eReserved102                            =   102,
    NEXUS_OtpCmdMsp_eReserved103                            =   103,
    NEXUS_OtpCmdMsp_eReserved104                            =   104,
    NEXUS_OtpCmdMsp_eReserved105                            =   105,
    NEXUS_OtpCmdMsp_eReserved106                            =   106,
    NEXUS_OtpCmdMsp_eReserved107                            =   107,
    NEXUS_OtpCmdMsp_eReserved108                            =   108,
    NEXUS_OtpCmdMsp_eReserved109                            =   109,
    NEXUS_OtpCmdMsp_eReserved110                            =   110,
    NEXUS_OtpCmdMsp_eReserved111                            =   111,
    NEXUS_OtpCmdMsp_eReserved112                            =   112,
    NEXUS_OtpCmdMsp_eReserved113                            =   113,
    NEXUS_OtpCmdMsp_eReserved114                            =   114,
    NEXUS_OtpCmdMsp_eReserved115                            =   115,
    NEXUS_OtpCmdMsp_eReserved116                            =   116,
    NEXUS_OtpCmdMsp_eReserved117                            =   117,
    NEXUS_OtpCmdMsp_eReserved118                            =   118,
    NEXUS_OtpCmdMsp_eReserved119                            =   119,
    NEXUS_OtpCmdMsp_eReserved120                            =   120,
    NEXUS_OtpCmdMsp_eReserved121                            =   121,
    NEXUS_OtpCmdMsp_eReserved122                            =   122,
    NEXUS_OtpCmdMsp_eReserved123                            =   123,
    NEXUS_OtpCmdMsp_eReserved124                            =   124,
    NEXUS_OtpCmdMsp_eReserved125                            =   125,
    NEXUS_OtpCmdMsp_eReserved126                            =   126,
    NEXUS_OtpCmdMsp_eReserved127                            =   127,
    NEXUS_OtpCmdMsp_eReserved128                            =   128,
    NEXUS_OtpCmdMsp_eReserved129                            =   129,
    NEXUS_OtpCmdMsp_eReserved130                            =   130,
    NEXUS_OtpCmdMsp_eViceVerifyEnable                       =   131,
    NEXUS_OtpCmdMsp_eReserved132                            =   132,
    NEXUS_OtpCmdMsp_ePcieGwinDisable                        =   133,
    NEXUS_OtpCmdMsp_ePcie0MwinDisable                       =   134,
    NEXUS_OtpCmdMsp_ePcie0MwinRestrictEnable                =   135,
    NEXUS_OtpCmdMsp_ePcie1MwinDisable                       =   136,
    NEXUS_OtpCmdMsp_ePcie1MwinRestrictEnable                =   137,
    NEXUS_OtpCmdMsp_eCaAesEcbMscSoftwareKeyDisable          =   138,
    NEXUS_OtpCmdMsp_eReserved139                            =   139,
    NEXUS_OtpCmdMsp_eReserved140                            =   140,
    NEXUS_OtpCmdMsp_eReserved141                            =   141,
    NEXUS_OtpCmdMsp_eSidVerifyEnable                        =   142,
    NEXUS_OtpCmdMsp_eReserved143                            =   143,
    NEXUS_OtpCmdMsp_eReserved144                            =   144,
    NEXUS_OtpCmdMsp_eReserved145                            =   145,
    NEXUS_OtpCmdMsp_eCpMulti2EcbCbcSoftwareKeyDisable       =   146,
    NEXUS_OtpCmdMsp_eReserved147                            =   147,
    NEXUS_OtpCmdMsp_eReserved148                            =   148,
    NEXUS_OtpCmdMsp_eReserved149                            =   149,
    NEXUS_OtpCmdMsp_eReserved150                            =   150,
    NEXUS_OtpCmdMsp_eReserved151                            =   151,
    NEXUS_OtpCmdMsp_eReserved152                            =   152,
    NEXUS_OtpCmdMsp_eMMCBootDisable                         =   153,
    NEXUS_OtpCmdMsp_eReserved154                            =   154,
    NEXUS_OtpCmdMsp_eReserved155                            =   155,
    NEXUS_OtpCmdMsp_eEbiCsSwapDisable                       =   156,
    NEXUS_OtpCmdMsp_eReserved157                            =   157,
    NEXUS_OtpCmdMsp_eReserved158                            =   158,
    NEXUS_OtpCmdMsp_eReserved159                            =   159,
    NEXUS_OtpCmdMsp_eReserved160                            =   160,
    NEXUS_OtpCmdMsp_eReserved161                            =   161,
    NEXUS_OtpCmdMsp_eReserved162                            =   162,
    NEXUS_OtpCmdMsp_eTwoStageCaKeyLadderDisable             =   163,
    NEXUS_OtpCmdMsp_eTwoStageCpKeyLadderDisable             =   164,
    NEXUS_OtpCmdMsp_eReserved165                            =   165,
    NEXUS_OtpCmdMsp_eReserved166                            =   166,
    NEXUS_OtpCmdMsp_eReserved167                            =   167,
    NEXUS_OtpCmdMsp_eReserved168                            =   168,
    NEXUS_OtpCmdMsp_eReserved169                            =   169,
    NEXUS_OtpCmdMsp_eReserved170                            =   170,
    NEXUS_OtpCmdMsp_eReserved171                            =   171,
    NEXUS_OtpCmdMsp_eReserved172                            =   172,
    NEXUS_OtpCmdMsp_eReserved173                            =   173,
    NEXUS_OtpCmdMsp_eReserved174                            =   174,
    NEXUS_OtpCmdMsp_eReserved175                            =   175,
    NEXUS_OtpCmdMsp_eReserved176                            =   176,
    NEXUS_OtpCmdMsp_eReserved177                            =   177,
    NEXUS_OtpCmdMsp_eReserved178                            =   178,
    NEXUS_OtpCmdMsp_eReserved179                            =   179,
    NEXUS_OtpCmdMsp_eReserved180                            =   180,
    NEXUS_OtpCmdMsp_eReserved181                            =   181,
    NEXUS_OtpCmdMsp_eReserved182                            =   182,
    NEXUS_OtpCmdMsp_eReserved183                            =   183,
    NEXUS_OtpCmdMsp_eReserved184                            =   184,
    NEXUS_OtpCmdMsp_eReserved185                            =   185,
    NEXUS_OtpCmdMsp_eReserved186                            =   186,
    NEXUS_OtpCmdMsp_eReserved187                            =   187,
    NEXUS_OtpCmdMsp_eReserved188                            =   188,
    NEXUS_OtpCmdMsp_eReserved189                            =   189,
    NEXUS_OtpCmdMsp_eDualCoreAcpuMode                       =   190,
    NEXUS_OtpCmdMsp_eSecureSandboxEnable                    =   191,
    NEXUS_OtpCmdMsp_eReserved192                            =   192,
    NEXUS_OtpCmdMsp_eRaagaBVerifyEnable                     =   193,
    NEXUS_OtpCmdMsp_eReserved194                            =   194,
    NEXUS_OtpCmdMsp_eExternalKeyTableDisable                =   195,
    NEXUS_OtpCmdMsp_eReserved196                            =   196,
    NEXUS_OtpCmdMsp_eReserved197                            =   197,
    NEXUS_OtpCmdMsp_eReserved198                            =   198,
    NEXUS_OtpCmdMsp_eReserved199                            =   199,
    NEXUS_OtpCmdMsp_eSecureRsa2Sha1TruncateDisable          =   200,
    NEXUS_OtpCmdMsp_eReserved201                            =   201,
    NEXUS_OtpCmdMsp_eReserved202                            =   202,
    NEXUS_OtpCmdMsp_eReserved203                            =   203,
    NEXUS_OtpCmdMsp_eReserved204                            =   204,
    NEXUS_OtpCmdMsp_eReserved205                            =   205,
    NEXUS_OtpCmdMsp_eReserved206                            =   206,
    NEXUS_OtpCmdMsp_eReserved207                            =   207,
    NEXUS_OtpCmdMsp_eReserved208                            =   208,
    NEXUS_OtpCmdMsp_eReserved209                            =   209,
    NEXUS_OtpCmdMsp_eReserved210                            =   210,
    NEXUS_OtpCmdMsp_eReserved211                            =   211,
    NEXUS_OtpCmdMsp_eReserved212                            =   212,
    NEXUS_OtpCmdMsp_eReserved213                            =   213,
    NEXUS_OtpCmdMsp_eReserved214                            =   214,
    NEXUS_OtpCmdMsp_eReserved215                            =   215,
    NEXUS_OtpCmdMsp_eReserved216                            =   216,
    NEXUS_OtpCmdMsp_eAesCounterGenericModeDisable           =   217,
    NEXUS_OtpCmdMsp_eCaCpAesCounterAsfModeDisable           =   218,
    NEXUS_OtpCmdMsp_eAesCounterWmdrmNdModeDisable           =   219,
    NEXUS_OtpCmdMsp_eCaCpAesCounterHdcp21ModeDisable        =   220,
    NEXUS_OtpCmdMsp_eReserved221                            =   221,
    NEXUS_OtpCmdMsp_eReserved222                            =   222,
    NEXUS_OtpCmdMsp_eReserved223                            =   223,
    NEXUS_OtpCmdMsp_eReserved224                            =   224,
    NEXUS_OtpCmdMsp_eReserved225                            =   225,
    NEXUS_OtpCmdMsp_e1DesHwKeyLadderDisable                 =   226,
    NEXUS_OtpCmdMsp_eRoutedHwKLCa3DesKeyK1K2CheckEnable     =   227,
    NEXUS_OtpCmdMsp_eRoutedHwKLCp3DesKeyK1K2CheckEnable     =   228,
    NEXUS_OtpCmdMsp_eRoutedHwKLNon3DesKeyK1K2checkEnable    =   229,
    NEXUS_OtpCmdMsp_eReserved230                            =   230,
    NEXUS_OtpCmdMsp_eReserved231                            =   231,
    NEXUS_OtpCmdMsp_eReserved232                            =   232,
    NEXUS_OtpCmdMsp_eReserved233                            =   233,
    NEXUS_OtpCmdMsp_eReserved234                            =   234,
    NEXUS_OtpCmdMsp_eCpAesGHddvdBlurayDisable               =   235,
    NEXUS_OtpCmdMsp_eReserved236                            =   236,
    NEXUS_OtpCmdMsp_eUnsignedRsaKeyKpkDecryptDisallow       =   237,
    NEXUS_OtpCmdMsp_eAsymmetricCrLockEnable                 =   238,
    NEXUS_OtpCmdMsp_eReserved239                            =   239,
    NEXUS_OtpCmdMsp_eReserved240                            =   240,
    NEXUS_OtpCmdMsp_eReserved241                            =   241,
    NEXUS_OtpCmdMsp_eReserved242                            =   242,
    NEXUS_OtpCmdMsp_eReserved243                            =   243,
    NEXUS_OtpCmdMsp_eReserved244                            =   244,
    NEXUS_OtpCmdMsp_eCaDvbCsa3SoftwareKeyDisable            =   245,
    NEXUS_OtpCmdMsp_eSizeCommonMSPs                         =   246,
    NEXUS_OtpCmdMsp_eReserved247_511                        =   247,
    NEXUS_OtpCmdMsp_eReserved512                            =   512,
    NEXUS_OtpCmdMsp_eReserved513                            =   513,
    NEXUS_OtpCmdMsp_eReserved514                            =   514,
    NEXUS_OtpCmdMsp_eReserved515                            =   515,
    NEXUS_OtpCmdMsp_eReserved516                            =   516,
    NEXUS_OtpCmdMsp_eReserved517                            =   517,
    NEXUS_OtpCmdMsp_eSystemEpoch3                           =   518,
    NEXUS_OtpCmdMsp_eSystemEpoch2                           =   519,
    NEXUS_OtpCmdMsp_eSystemEpoch1                           =   520,
    NEXUS_OtpCmdMsp_eReserved521                            =   521,
    NEXUS_OtpCmdMsp_eReserved522                            =   522,
    NEXUS_OtpCmdMsp_eReserved523                            =   523,
    NEXUS_OtpCmdMsp_eReserved524                            =   524,
    NEXUS_OtpCmdMsp_eReserved525                            =   525,
    NEXUS_OtpCmdMsp_eReserved526                            =   526,
    NEXUS_OtpCmdMsp_eReserved527                            =   527,
    NEXUS_OtpCmdMsp_eReserved528                            =   528,
    NEXUS_OtpCmdMsp_eDataSection1Lock                       =   529,
    NEXUS_OtpCmdMsp_eReserved530                            =   530,
    NEXUS_OtpCmdMsp_eReserved531                            =   531,
    NEXUS_OtpCmdMsp_eReserved532                            =   532,
    NEXUS_OtpCmdMsp_eReserved533                            =   533,
    NEXUS_OtpCmdMsp_eReserved534                            =   534,
    NEXUS_OtpCmdMsp_eReserved535                            =   535,
    NEXUS_OtpCmdMsp_eReserved536                            =   536,
    NEXUS_OtpCmdMsp_eReserved537                            =   537,
    NEXUS_OtpCmdMsp_eReserved538                            =   538,
    NEXUS_OtpCmdMsp_eReserved539                            =   539,
    NEXUS_OtpCmdMsp_eReserved540                            =   540,
    NEXUS_OtpCmdMsp_eReserved541                            =   541,
    NEXUS_OtpCmdMsp_eReserved542                            =   542,
    NEXUS_OtpCmdMsp_eReserved543                            =   543,
    NEXUS_OtpCmdMsp_eReserved544                            =   544,
    NEXUS_OtpCmdMsp_eReserved545                            =   545,
    NEXUS_OtpCmdMsp_eReserved546                            =   546,
    NEXUS_OtpCmdMsp_eReserved547                            =   547,
    NEXUS_OtpCmdMsp_eReserved548                            =   548,
    NEXUS_OtpCmdMsp_eReserved549                            =   549,
    NEXUS_OtpCmdMsp_eReserved550                            =   550,
    NEXUS_OtpCmdMsp_eReserved551                            =   551,
    NEXUS_OtpCmdMsp_eReserved552                            =   552,
    NEXUS_OtpCmdMsp_eReserved553                            =   553,
    NEXUS_OtpCmdMsp_eReserved554                            =   554,
    NEXUS_OtpCmdMsp_eReserved555                            =   555,
    NEXUS_OtpCmdMsp_eReserved556                            =   556,
    NEXUS_OtpCmdMsp_eReserved557                            =   557,
    NEXUS_OtpCmdMsp_eReserved558                            =   558,
    NEXUS_OtpCmdMsp_eReserved559                            =   559,
    NEXUS_OtpCmdMsp_eReserved560                            =   560,
    NEXUS_OtpCmdMsp_eReserved561                            =   561,
    NEXUS_OtpCmdMsp_eReserved562                            =   562,
    NEXUS_OtpCmdMsp_eReserved563                            =   563,
    NEXUS_OtpCmdMsp_eReserved564                            =   564,
    NEXUS_OtpCmdMsp_eReserved565                            =   565,
    NEXUS_OtpCmdMsp_eReserved566                            =   566,
    NEXUS_OtpCmdMsp_eReserved567                            =   567,
    NEXUS_OtpCmdMsp_eReserved568                            =   568,
    NEXUS_OtpCmdMsp_eReserved569                            =   569,
    NEXUS_OtpCmdMsp_eReserved570                            =   570,
    NEXUS_OtpCmdMsp_eReserved571                            =   571,
    NEXUS_OtpCmdMsp_eReserved572                            =   572,
    NEXUS_OtpCmdMsp_eReserved573                            =   573,
    NEXUS_OtpCmdMsp_eReserved574                            =   574,
    NEXUS_OtpCmdMsp_eReserved575                            =   575,
    NEXUS_OtpCmdMsp_eReserved576                            =   576,
    NEXUS_OtpCmdMsp_eReserved577                            =   577,
    NEXUS_OtpCmdMsp_eReserved578                            =   578,
    NEXUS_OtpCmdMsp_eReserved579                            =   579,
    NEXUS_OtpCmdMsp_eReserved580                            =   580,
    NEXUS_OtpCmdMsp_eReserved581                            =   581,
    NEXUS_OtpCmdMsp_eReserved582                            =   582,
    NEXUS_OtpCmdMsp_eReserved583                            =   583,
    NEXUS_OtpCmdMsp_eReserved584                            =   584,
    NEXUS_OtpCmdMsp_eReserved585                            =   585,
    NEXUS_OtpCmdMsp_eReserved586                            =   586,
    NEXUS_OtpCmdMsp_eReserved587                            =   587,
    NEXUS_OtpCmdMsp_eReserved588                            =   588,
    NEXUS_OtpCmdMsp_eReserved589                            =   589,
    NEXUS_OtpCmdMsp_eReserved590                            =   590,
    NEXUS_OtpCmdMsp_eReserved591                            =   591,
    NEXUS_OtpCmdMsp_eReserved592                            =   592,
    NEXUS_OtpCmdMsp_eReserved593                            =   593,
    NEXUS_OtpCmdMsp_eReserved594                            =   594,
    NEXUS_OtpCmdMsp_eReserved595                            =   595,
    NEXUS_OtpCmdMsp_eReserved596                            =   596,
    NEXUS_OtpCmdMsp_eReserved597                            =   597,
    NEXUS_OtpCmdMsp_eReserved598                            =   598,
    NEXUS_OtpCmdMsp_eReserved599                            =   599,
    NEXUS_OtpCmdMsp_eReserved600                            =   600,
    NEXUS_OtpCmdMsp_eReserved601                            =   601,
    NEXUS_OtpCmdMsp_eReserved602                            =   602,
    NEXUS_OtpCmdMsp_eReserved603                            =   603,
    NEXUS_OtpCmdMsp_eReserved604                            =   604,
    NEXUS_OtpCmdMsp_eReserved605                            =   605,
    NEXUS_OtpCmdMsp_eReserved606                            =   606,
    NEXUS_OtpCmdMsp_eReserved607                            =   607,
    NEXUS_OtpCmdMsp_eReserved608                            =   608,
    NEXUS_OtpCmdMsp_eReserved609                            =   609,
    NEXUS_OtpCmdMsp_eReserved610                            =   610,
    NEXUS_OtpCmdMsp_eReserved611                            =   611,
    NEXUS_OtpCmdMsp_eReserved612                            =   612,
    NEXUS_OtpCmdMsp_eReserved613                            =   613,
    NEXUS_OtpCmdMsp_eReserved614                            =   614,
    NEXUS_OtpCmdMsp_eReserved615                            =   615,
    NEXUS_OtpCmdMsp_eReserved616                            =   616,
    NEXUS_OtpCmdMsp_eReserved617                            =   617,
    NEXUS_OtpCmdMsp_eReserved618                            =   618,
    NEXUS_OtpCmdMsp_eReserved619                            =   619,
    NEXUS_OtpCmdMsp_eForceSCDisallow                        =   620,
    NEXUS_OtpCmdMsp_ePCIe0MwinSizeEnforceEnable             =   621,
    NEXUS_OtpCmdMsp_ePCIe1MwinSizeEnforceEnable             =   622,
    NEXUS_OtpCmdMsp_eKeyladderKeyContributionDisallow       =   623,
    NEXUS_OtpCmdMsp_eKeyladderKeyContributionFeatureDisable =   624,
    NEXUS_OtpCmdMsp_eCaCpd1DES3DESCBCSoftwareKeyDisable     =   625,
    NEXUS_OtpCmdMsp_eReserved626                            =   626,
    NEXUS_OtpCmdMsp_eReserved627                            =   627,
    NEXUS_OtpCmdMsp_eHostBootCodeDecryptExt                 =   628,
    NEXUS_OtpCmdMsp_eHvdFwEpoch                             =   629,
    NEXUS_OtpCmdMsp_eRaagaFwEpoch                           =   630,
    NEXUS_OtpCmdMsp_eViceFwEpoch                            =   631,
    NEXUS_OtpCmdMsp_eRaveFwEpoch                            =   632,
    NEXUS_OtpCmdMsp_eMarketId1                              =   633,
    NEXUS_OtpCmdMsp_eArcType                                =   634,
    NEXUS_OtpCmdMsp_eKey0PrimeSigningRightsSDL              =   635,
    NEXUS_OtpCmdMsp_eKey0SigningRightsSDL                   =   636,
    NEXUS_OtpCmdMsp_eSageGpBits_0                           =   637,
    NEXUS_OtpCmdMsp_eSageGpBits_1                           =   638,
    NEXUS_OtpCmdMsp_eMax,

    /* legacy enumerators. */
    NEXUS_OtpCmdMsp_eReserved7                              = NEXUS_OtpCmdMsp_ePcie0HostProtect,
    NEXUS_OtpCmdMsp_eReserved8                              = NEXUS_OtpCmdMsp_ePcie0ClientProtect,
    NEXUS_OtpCmdMsp_ePcieHostProtect                        = NEXUS_OtpCmdMsp_ePcie1HostProtect,
    NEXUS_OtpCmdMsp_ePcieClientProtect                      = NEXUS_OtpCmdMsp_ePcie1ClientProtect,
    NEXUS_OtpCmdMsp_eCpM2MControlBits                       = NEXUS_OtpCmdMsp_eCpControlBits,
    NEXUS_OtpCmdMsp_eReserved76                             = NEXUS_OtpCmdMsp_eRaagaAVerifyEnable,
    NEXUS_OtpCmdMsp_eReserved77                             = NEXUS_OtpCmdMsp_eVideoVerifyEnable,
    NEXUS_OtpCmdMsp_eReserved78                             = NEXUS_OtpCmdMsp_eRaveVerifyEnable,
    NEXUS_OtpCmdMsp_eSystemEpoch                            = NEXUS_OtpCmdMsp_eSystemEpoch0,
    NEXUS_OtpCmdMsp_eMarketId                               = NEXUS_OtpCmdMsp_eMarketId0,
    NEXUS_OtpCmdMsp_eReserved131                            = NEXUS_OtpCmdMsp_eViceVerifyEnable,
    NEXUS_OtpCmdMsp_eReserved134                            = NEXUS_OtpCmdMsp_ePcie0MwinDisable,
    NEXUS_OtpCmdMsp_eReserved135                            = NEXUS_OtpCmdMsp_ePcie0MwinRestrictEnable,
    NEXUS_OtpCmdMsp_ePcieMwinDisable                        = NEXUS_OtpCmdMsp_ePcie1MwinDisable,
    NEXUS_OtpCmdMsp_ePcieMwinRestrictEnable                 = NEXUS_OtpCmdMsp_ePcie1MwinRestrictEnable,
    NEXUS_OtpCmdMsp_eReserved142                            = NEXUS_OtpCmdMsp_eSidVerifyEnable,
    NEXUS_OtpCmdMsp_eReserved146                            = NEXUS_OtpCmdMsp_eCpMulti2EcbCbcSoftwareKeyDisable,
    NEXUS_OtpCmdMsp_eReserved156                            = NEXUS_OtpCmdMsp_eEbiCsSwapDisable,
    NEXUS_OtpCmdMsp_eReserved163                            = NEXUS_OtpCmdMsp_eTwoStageCaKeyLadderDisable,
    NEXUS_OtpCmdMsp_eReserved164                            = NEXUS_OtpCmdMsp_eTwoStageCpKeyLadderDisable,
    NEXUS_OtpCmdMsp_eReserved193                            = NEXUS_OtpCmdMsp_eRaagaBVerifyEnable,
    NEXUS_OtpCmdMsp_eReserved217                            = NEXUS_OtpCmdMsp_eAesCounterGenericModeDisable,
    NEXUS_OtpCmdMsp_eReserved218                            = NEXUS_OtpCmdMsp_eCaCpAesCounterAsfModeDisable,
    NEXUS_OtpCmdMsp_eReserved219                            = NEXUS_OtpCmdMsp_eAesCounterWmdrmNdModeDisable,
    NEXUS_OtpCmdMsp_eReserved220                            = NEXUS_OtpCmdMsp_eCaCpAesCounterHdcp21ModeDisable,
    NEXUS_OtpCmdMsp_eReserved518                            = NEXUS_OtpCmdMsp_eSystemEpoch3,
    NEXUS_OtpCmdMsp_eReserved519                            = NEXUS_OtpCmdMsp_eSystemEpoch2,
    NEXUS_OtpCmdMsp_eReserved520                            = NEXUS_OtpCmdMsp_eSystemEpoch1,
    NEXUS_OtpCmdMsp_eAltMarketId                            = NEXUS_OtpCmdMsp_eMarketId1,
    NEXUS_OtpCmdMsp_eReserved635                            = NEXUS_OtpCmdMsp_eKey0PrimeSigningRightsSDL,
    NEXUS_OtpCmdMsp_eReserved636                            = NEXUS_OtpCmdMsp_eKey0SigningRightsSDL

} NEXUS_OtpCmdMsp;



/*****************************************************************************
Summary:
Select bits to be read
*****************************************************************************/
typedef enum NEXUS_OtpCmdReadRegister
{
    NEXUS_OtpCmdReadRegister_eMc0S_Reserved0                         = 0,
    NEXUS_OtpCmdReadRegister_eMc0S_Reserved1                         = 1,

    NEXUS_OtpCmdReadRegister_eMc0S_Reserved2                         = 2,
    NEXUS_OtpCmdReadRegister_eMc0S_Reserved3                         = 3,

    NEXUS_OtpCmdReadRegister_eMc0S_Reserved4                         = 4,
    NEXUS_OtpCmdReadRegister_eMc0S_Reserved5                         = 5,

    NEXUS_OtpCmdReadRegister_eKeyMc0_CustomerMode                    = 6,
    NEXUS_OtpCmdReadRegister_eKeyMc0_Reserved7                         = 7,
    NEXUS_OtpCmdReadRegister_eKeyMc0_DeobfuscationEnable             = 8,
    NEXUS_OtpCmdReadRegister_eKeyMc0_BlackBoxId                      = 9,
    NEXUS_OtpCmdReadRegister_eKeyMc0_CaKeyLadderDisallow             = 10,
    NEXUS_OtpCmdReadRegister_eKeyMc0_CpKeyLadderDisallow             = 11,
    NEXUS_OtpCmdReadRegister_eKeyMc0_Gp1KeyLadderDisallow            = 12,
    NEXUS_OtpCmdReadRegister_eKeyMc0_Gp2KeyLadderDisallow            = 13,
    NEXUS_OtpCmdReadRegister_eKeyMc0_Reserved14                         = 14,
    NEXUS_OtpCmdReadRegister_eKeyID                                  = 15,
    NEXUS_OtpCmdReadRegister_eKeyHash                                = 16,
    NEXUS_OtpCmdReadRegister_eBseckHashCrc                           = 17,
    NEXUS_OtpCmdReadRegister_eMc0S_Reserved18                         = 18,
    NEXUS_OtpCmdReadRegister_eMc0S_FixedDeobfuscationVariantEnable   = 19,
    NEXUS_OtpCmdReadRegister_eKeyMc0_RootKeySwapDisallow             = 20,
    NEXUS_OtpCmdReadRegister_eKeyMc0_Reserved21                        = 21,
    NEXUS_OtpCmdReadRegister_eMax
} NEXUS_OtpCmdReadRegister;


/* select otp key type for the field to be read */
typedef enum NEXUS_OtpKeyType
{
    NEXUS_OtpKeyType_eA,
    NEXUS_OtpKeyType_eB,
    NEXUS_OtpKeyType_eC,
    NEXUS_OtpKeyType_eD,
    NEXUS_OtpKeyType_eE,
    NEXUS_OtpKeyType_eF,
    NEXUS_OtpKeyType_eG,
    NEXUS_OtpKeyType_eH,

    NEXUS_OtpKeyType_eSize

} NEXUS_OtpKeyType ;


#define NEXUS_OTP_KEY_ID_LEN            8
#define NEXUS_MSP_DATA_LEN              4
#define NEXUS_MSP_SIGNATURE_DATA_LEN    20          /* 16 for legacy, 20 for ASKM */
#define NEXUS_MSP_OUTPUT_DATA_LEN       4
#define NEXUS_OTP_DATASECTION_LEN       (32)        /* in byte now, may change to word32*/
#define NEXUS_OTP_CRC_LEN               8



/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadOtpIO {
    unsigned char       otpKeyIdBuf[NEXUS_OTP_KEY_ID_LEN];      /* Buffer to hold OTP Key ID for the current OTP Key ID read request */
    unsigned int        otpKeyIdSize;                           /* Actual size of Otp read buffer */
} NEXUS_ReadOtpIO;

/*****************************************************************************
Summary:
This function returns one OTP key identifiers or one MC0 OTP value.

Description:
This function shall be used to read either OTP key identifier or other OTP field value.
Depends on the access control matrix, only certain OTP fields can be read in a specific
customer mode.  Note that this function can only read back one OTP value at a time.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadMSP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ReadOTP(
    NEXUS_OtpCmdReadRegister    readOtpEnum,
    NEXUS_OtpKeyType            keyType,
    NEXUS_ReadOtpIO             *pReadOtpIO         /* [out] structure holding read OTP buffer and size */
    );


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to program, with what data,  using what mask and proper mode,  and holds the returned status
of a programming request

See Also:
NEXUS_OtpMsp_ProgramMSP
**************************************************************************************************/
typedef struct NEXUS_ProgramMspIO {
    NEXUS_OtpMspCmdProgMode    progMode;    /* This field should contain the value of NEXUS_OtpMspCmdProgMode
           for the bits to be programmed. This is a sanity check on the command.
           The value NEXUS_OtpMspCmdProgMode_Enum  specifies that command enum mode
           programming is used.
        */

    NEXUS_OtpCmdMsp            progMspEnum; /* specifies which MSP bits to program. The values this field can take are specified by the
          typedef enum NEXUS_OtpCmdMsp in the share C header file.  Each chip will have different enums
          and customers will only have access to the files for the chips that they use.
        */

    unsigned char               dataBitLen; /* number of bits of the MSP enum to program, from 1 to 32 */

    unsigned char               dataBitMask [NEXUS_MSP_DATA_LEN];   /* 0x0000_0001 to 0xFFFF_FFFF
          A value 1 in a bit position specifies that the data value at that bit position is to be programmed.
          BSP will not program bits that have a zero bit in the mask.
          For example 0x0F means to program the 4 LSBits of an enum.
          For example 0x8F means to program the bit 7 and 4 LSBits of an enum.
        */
    unsigned char               mspData[NEXUS_MSP_DATA_LEN];    /* the value that needs to be programmed */

} NEXUS_ProgramMspIO;



/*****************************************************************************
Summary:
This function allows the programming of each of the field programmable
OTP (MSP) bits.

Description:
This function allows the programming of each of the field programmable
OTP (MSP) bits.  Based on the Access Control Matrix (ACL), programming of the
bit is allowed or disallowed.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ProgramOTP
*****************************************************************************/
NEXUS_Error NEXUS_Security_ProgramMSP(
    const NEXUS_ProgramMspIO    *pProgMspIO
    );


#define NEXUS_MSP_KEY3_DATA_LEN        16
#define NEXUS_MSP_KEY4_DATA_LEN        16


/**************************************************************************************************
Summary:

Description:
Structure that defines which MSP field to read and its required key/data if command authentication is needed by BSP,
and holds the returned value of the MSP field

See Also:
NEXUS_OtpMsp_ReadMSP
**************************************************************************************************/
typedef struct NEXUS_ReadMspParms
{
    NEXUS_OtpCmdMsp             readMspEnum;     /* which MSP to read */

} NEXUS_ReadMspParms;


/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadMspIO {
    unsigned char       mspDataBuf[NEXUS_MSP_OUTPUT_DATA_LEN];      /* Buffer to hold MSP data for the current MSP read request   */
    unsigned char        lockMspDataBuf[NEXUS_MSP_OUTPUT_DATA_LEN];  /* Buffer to hold lock MSP data for the current read request       */
                                                                    /* This is used to tell if value '0' is programmed or unprogrammed */
    unsigned int        mspDataSize;                                /* Actual size of MSP output buffer */
} NEXUS_ReadMspIO;

/*****************************************************************************
Summary:
This function returns one MSP value.

Description:

This function shall be used to read MSP field value. Depends on the access control matrix, only
certain MSP fields can be read in a specific customer mode.  Note that this function can only
read back one MSP value at a time.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadOTP
*****************************************************************************/
NEXUS_Error      NEXUS_Security_ReadMSP(
    const NEXUS_ReadMspParms    *pReadMspParms,     /* structure holding input parameters */
    NEXUS_ReadMspIO             *pReadMspIO         /* [out] structure holding read MSP buufer and size */
    );

/*****************************************************************************
Summary:
*****************************************************************************/
typedef enum NEXUS_OtpDataSection
{
    NEXUS_OtpDataSection_e0     = 0x0,
    NEXUS_OtpDataSection_e1     = 0x1,
    NEXUS_OtpDataSection_e2     = 0x2,
    NEXUS_OtpDataSection_e3     = 0x3,
    NEXUS_OtpDataSection_e4     = 0x4,
    NEXUS_OtpDataSection_e5     = 0x5,
    NEXUS_OtpDataSection_e6     = 0x6,
    NEXUS_OtpDataSection_e7     = 0x7,
    NEXUS_OtpDataSection_eMax   = 0x8

} NEXUS_OtpDataSection;

/*****************************************************************************
Summary:
*****************************************************************************/
typedef struct NEXUS_ReadDataSectIO {
    unsigned char       dataSectBuf[NEXUS_OTP_DATASECTION_LEN]; /* Buffer to hold DataSection data for the current DataSect read request */
    unsigned int        dataSectSize;                           /* Actual size of data section read */
    bool                isShaOfData;                            /* True is the value in dataSectBuf is sha */
} NEXUS_ReadDataSectIO;


/*****************************************************************************
Summary:
This function returns one 32-byte data section value.

Description:
There are total of 8 32-byte data sections. This function shall be used to read each 32-byte data section.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ProgramDataSect
*****************************************************************************/
NEXUS_Error NEXUS_Security_ReadDataSect(
    NEXUS_OtpDataSection        readDsEnum,       /* NEXUS_OtpDataSection enum of which data section to be read */
    NEXUS_ReadDataSectIO       *pReadDataSectIO    /* [out] structure holding read datasect buffer and size */
    );



#define NEXUS_OTP_DATASECTION_CRC_LEN               4
#define NEXUS_OTP_DATASECTIONPROG_MODE              0x00010112


/**************************************************************************************************
Summary:

Description:
Structure that defines which OTP data section to program with what data using a proper mode, and holds
the returned status

See Also:
NEXUS_Security_ProgramDataSect
**************************************************************************************************/
typedef struct NEXUS_ProgramDataSectIO {
    NEXUS_OtpDataSection       progDsEnum; /* select which OTP data section to program, between NEXUS_OtpDataSection_e0 ~
            NEXUS_OtpDataSection_e7 */

    unsigned char               dataSectData[NEXUS_OTP_DATASECTION_LEN]; /* provide the actual 32-byte data to be programmed into the specified OTP data section*/

    unsigned char               crc[NEXUS_OTP_DATASECTION_CRC_LEN]; /* provide the crc of data section */

    uint32_t                    mode; /* a kind of program magic number, must be NEXUS_OTP_DATASECTIONPROG_MODE (0x00010112).
            if it is not this value the command will be rejected by  a sanity check at BSP */

    uint32_t                    padding; /* padding field, to use the entire structure with HSM PI  */
} NEXUS_ProgramDataSectIO;


/*****************************************************************************
Summary:
This function is used to program one 32-byte data section value.

Description:
There are total of 8 32-byte data sections. This function shall be used to write one 32-byte data
section.

Performance and Timing:
This is a synchronous/blocking function that would not return until it is done or failed.

See Also:
NEXUS_Security_ReadDataSect
*****************************************************************************/
NEXUS_Error NEXUS_Security_ProgramDataSect(
    const NEXUS_ProgramDataSectIO   *pProgDataSectIO
    );

#ifdef __cplusplus
}
#endif

#endif
