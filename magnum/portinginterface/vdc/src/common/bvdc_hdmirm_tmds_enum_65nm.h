/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
/* Parameters generated 03/30/2016 at time 09:27:24 PM                       */


/*-----------------------------------------------------------*/
typedef enum BVDC_P_TmdsClock {
   BVDC_P_TmdsClock_e23_75        =    0,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e25           =    1,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e25_175       =    2,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e25_2         =    3,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e27           =    4,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e29_6875      =    5,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e31_25        =    6,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e31_46875     =    7,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e31_5         =    8,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e33_75_Ver_1  =    9,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e33_75_Ver_2  =   10,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e35_5         =   11,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e35_625       =   12,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e36           =   13,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e37_5         =   14,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e37_7625      =   15,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e37_8         =   16,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e38_21568     =   17,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e39_375       =   18,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e39_5         =   19,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e39_75        =   20,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e39_79008     =   21,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e40           =   22,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e40_5_Ver_1   =   23,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e40_5_Ver_2   =   24,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e42_1875      =   25,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e44_375       =   26,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e44_9         =   27,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e45           =   28,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e47_25        =   29,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e47_5         =   30,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e47_7696      =   31,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e49           =   32,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e49_375       =   33,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e49_5         =   34,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e49_6875      =   35,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e49_7376      =   36,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e50           =   37,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e50_625       =   38,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e53_25        =   39,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e54           =   40,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e56_125       =   41,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e56_25        =   42,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e57_32352     =   43,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e59_25        =   44,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e59_375       =   45,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e59_4         =   46,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e59_625       =   47,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e59_68512     =   48,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e60           =   49,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e60_4656      =   50,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e60_75        =   51,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e61_25        =   52,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e61_875       =   53,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e62_5         =   54,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e63_958       =   55,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e64_0224      =   56,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e64_93        =   57,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e65           =   58,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e65_22        =   59,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e65_286       =   60,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e67_35        =   61,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e67_497       =   62,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e67_5         =   63,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e67_5648      =   64,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e68_25        =   65,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e70_3125      =   66,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e71_25        =   67,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e72           =   68,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e73_5         =   69,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e74_25        =   70,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e75           =   71,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e75_5         =   72,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e75_582       =   73,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e78_75        =   74,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e79_5         =   75,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e79_9475      =   76,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e80_028       =   77,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e80_136       =   78,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e81_Ver_1     =   79,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e81_Ver_2     =   80,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e81_1625      =   81,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e81_25        =   82,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e81_525       =   83,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e81_6075      =   84,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e82           =   85,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e83_4624      =   86,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e83_5         =   87,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e84_37125     =   88,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e84_375       =   89,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e84_456       =   90,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e84_7152      =   91,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e84_75        =   92,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e85_3125      =   93,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e85_5         =   94,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e88_75        =   95,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e89_1         =   96,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e90           =   97,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e90_6984      =   98,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e92_8125      =   99,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e93_75        =  100,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e94_375       =  101,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e94_5         =  102,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e95_937       =  103,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e96_0336      =  104,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e97_395       =  105,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e97_5         =  106,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e97_83        =  107,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e97_929       =  108,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e98_4375      =  109,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e99_375       =  110,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e100_17       =  111,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e101          =  112,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e101_2455     =  113,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e101_25_Ver_1 =  114,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e101_25_Ver_2 =  115,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e101_3472     =  116,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e102_25       =  117,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e102_375      =  118,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e102_5        =  119,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e104_328      =  120,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e104_375      =  121,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e105_894      =  122,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e105_9375     =  123,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e106_5        =  124,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e106_875      =  125,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e108_Ver_1    =  126,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e108_Ver_2    =  127,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e108_108      =  128,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e109          =  129,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e110_9375     =  130,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e111_375      =  131,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e112_5        =  132,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e113_25       =  133,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e116_015625   =  134,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e117_5        =  135,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e118_125      =  136,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e119          =  137,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e119_25       =  138,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e120_204      =  139,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e121_5        =  140,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e121_75       =  141,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e123          =  142,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e125_1936     =  143,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e125_25       =  144,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e126_25       =  145,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e126_5625     =  146,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e127_0728     =  147,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e127_125      =  148,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e127_8125     =  149,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e128_125      =  150,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e128_25       =  151,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e133_125      =  152,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e135_Ver_1    =  153,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e135_Ver_2    =  154,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e135_135      =  155,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e136_25       =  156,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e136_75       =  157,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e139_21875    =  158,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e141_75       =  159,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e146_25       =  160,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e146_875      =  161,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e148_5        =  162,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e148_75       =  163,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e151_5        =  164,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e151_875      =  165,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e152_1875     =  166,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e153_375      =  167,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e153_75       =  168,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e154          =  169,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e156          =  170,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e157          =  171,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e157_5        =  172,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e159_75       =  173,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e161          =  174,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e162_Ver_1    =  175,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e162_Ver_2    =  176,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e162_162      =  177,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e163_5        =  178,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e167_0625     =  179,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e168_75       =  180,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e170_9375     =  181,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e175_5        =  182,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e176_25       =  183,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e178_5        =  184,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e182_625      =  185,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e182_8125     =  186,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e185          =  187,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e185_625      =  188,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e187          =  189,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e189          =  190,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e192_5        =  191,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e193_25       =  192,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e195          =  193,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e196_25       =  194,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e196_875      =  195,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e201_25       =  196,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e202_5_Ver_1  =  197,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e202_5_Ver_2  =  198,    /* Standard and MUL 1.001 variant */
   BVDC_P_TmdsClock_e204_75       =  199,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e205_125      =  200,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e214_75       =  201,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e218_25       =  202,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e219_375      =  203,    /* Standard and DIV 1.001 variant */
   BVDC_P_TmdsClock_e222_75       =  204     /* Standard and DIV 1.001 variant */
}
BVDC_P_TmdsClock;
