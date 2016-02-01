/***************************************************************************
 *     Copyright (c) 2004-2011, Broadcom Corporation
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
 *   This file contains various equalizer generation algorithms.
 *   The fixed point code is written for 3548 HW where coefs are represented in
 *   2.26 format and the equation implemented is 
 *   y(n) = b0 * x(n) + b1 * x(n-1) + b2 * x(n-2) + a1 * y(n-1) + a2 * y(n-2)
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"

#if BAPE_CHIP_SRC_TYPE_IS_IIR       /* This code in this file only applies to IIR sample rate converters */

#include "bape_eq_coeff_priv.h"

/* Local functions used in Equalizer */
void EQ_convert_analog_to_digital ( const int32_t ptr_mant[],
        const int32_t ptr_exp[],
        int32_t fc,
        int32_t fs,
        int32_t coef_mant[],
        int32_t coef_exp[]);

int32_t EQ_generate_bass (int32_t gain, int32_t fs, int32_t fc,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[]);

int32_t EQ_generate_treble (int32_t gain, int32_t fs, int32_t fc,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[]);

void EQ_int2fix(int32_t inp,
        int32_t *mant,
        int32_t *exp);

int32_t EQ_mul_32_32 (int32_t a,
        int32_t b);

void EQ_mul (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t mant,
        int32_t exp,
        int32_t src_mant,
        int32_t src_exp);

void EQ_add (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t mant,
        int32_t exp,
        int32_t src_mant,
        int32_t src_exp);

void EQ_add_32_32 (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t src_mant,
        int32_t src_exp);

void EQ_renormalise (int32_t *p_mant,
        int32_t *p_exp);

int32_t EQ_div_32_32 (int32_t num,
        int32_t den);

void EQ_div (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t mant,
        int32_t exp,
        int32_t src_mant,
        int32_t src_exp);

void EQ_sqrt (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t src_mant,
        int32_t src_exp);

int32_t EQ_sqrt_32 (int32_t in);

void EQ_cos_sin (int32_t phase,
        int32_t *cos_val,
        int32_t *sin_val);

void EQ_tan(int32_t f1,
        int32_t f2,
        int32_t *fc_a_mant,
        int32_t *fc_a_exp);

void EQ_db_2_linear (int32_t *dest_mant,
        int32_t *dest_exp,
        int32_t src_mant,
        int32_t src_exp);

int32_t EQ_db_2_linear_table(int32_t in,
        const int32_t table[]);

/* Tables used in equalizer */
static const int32_t EQ_db2linear_mant_table [] = {
    0x4000075e, 0x40000ebd, 0x40001d79, 0x40003af2,
    0x400075e5, 0x4000ebcb, 0x4001d799, 0x4003af3f,
    0x40075eb4, 0x400ebe41, 0x401d7fe7, 0x403b0d66,
    0x4076514a, 0x40ed7d4f, 0x41de6be3, 0x43cad023,

    0x47cf267e, 0x50923be4, 0x656ee3db, 0x50615fa7,
    0x64f40349, 0x4f9f164e, 0x630e4a84, 0x4ca81cdd,
    0x5bd0fe24, 0x41dc7e98,
};
static const int32_t EQ_db2linear_exp_table [] = {
    -30, -30, -30, -30,
    -30, -30, -30, -30,
    -30, -30, -30, -30,
    -30, -30, -30, -30,

    -30, -30, -30, -29,
    -28, -25, -20, -9,
    12,  55,
};

static const int32_t EQ_sin_table[] = {
    0x0000003, 0x0000006,
    0x0000000d, 0x00000019, 0x00000032, 0x00000065,
    0x000000c9, 0x00000192, 0x00000324, 0x00000649,
    0x00000c91, 0x00001922, 0x00003244, 0x00006488,

    0x0000c90f, 0x0001921f, 0x0003243f, 0x0006487e,
    0x000c90fd, 0x001921fa, 0x003243f1, 0x006487c3,
    0x00c90e8f, 0x0192155f, 0x0323ecbe, 0x0645e9af,
    0x0c7c5c1e, 0x187de2a6, 0x2d413ccc, 0x40000000,
};

static const int32_t EQ_cos_table[] = {
    0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0x40000000, 0x40000000, 0x40000000, 0x40000000,

    0x3ffffffe, 0x3ffffffb, 0x3fffffec, 0x3fffffb1,
    0x3ffffec4, 0x3ffffb10, 0x3fffec42, 0x3fffb10b,
    0x3ffec42d, 0x3ffb10c1, 0x3fec43c6, 0x3fb11b47,
    0x3ec52f9f, 0x3b20d79e, 0x2d413ccc, 0x00000000,
};
static const int32_t EQ_GEQ_A_table[5][3] = {
    { 0x00a0d97b,    0x0074b75a,    0x006b3ba7 },
    { 0x01fc48d8,    0x0170d2c1,    0x0152db3b },
    { 0x06487ed5,    0x048f2987,    0x0430548e },
    { 0x13dad878,    0x0e683b8d,    0x0d3c9050 },
    { 0x3ed4f452,    0x2d979f4a,    0x29e34d8c }
};

static const int32_t EQ_GEQ_B_table[5][3] = {
    { 0x00000ca21, 0x000006a6d, 0x000059d5 },
    { 0x00007e262, 0x0000426bd, 0x0003810f },
    { 0x0004ef4f3, 0x0002992b7, 0x00231788 },
    { 0x003146e6e, 0x0019f21e7, 0x015e69f8 },
    { 0x01ed7aefb, 0x0103d4f9a, 0x0db53153 },
};

static const int32_t EQ_pow_table_db_int_neg[] = {
    0x0ffffc50, 0x0ffff8a1, 0x0ffff143, 0x0fffe286,
    0x0fffc50e, 0x0fff8a1d, 0x0fff143d, 0x0ffe2889,
    0x0ffc5149, 0x0ff8a36a, 0x0ff14a39, 0x0fe2a1f8,
    0x0fc579d7, 0x0f8bc9c0, 0x0f1adf93, 0x0e429057,

    0x0cb59185, 0x0a1866ba, 0x065ea59f, 0x02892c18,
    0x0066e309, 0x0002959b, 0x0000006a, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

static const int32_t EQ_matr_int[5][5] = {
    { 0x04b41d9c, 0xe6b58c1b, 0x021af614, 0xffd74c36, 0x000303f1 },
    { 0xe6b5ab37, 0x097f2df7, 0xe660ede3, 0x0222d777, 0xffd7503d },
    { 0x021ad54d, 0xe6622b92, 0x09869086, 0xe64dd084, 0x021c5dee },
    { 0xffd76060, 0x0221d872, 0xe6564cd7, 0x0981594e, 0xe6c01389 },
    { 0x00030012, 0xffd78307, 0x021a43b8, 0xe6be6ea4, 0x04b08528 }
};

/************ Tables for Butterworth filter (Begin) *********************/
/* Butterworth filter (LPF) order-2 */
static const int32_t EQ_LPF_butter_1_mant [] = {
    0, 0, 0x40000000, 0x40000000, 0x5a82799a, 0x40000000
};
static const int32_t EQ_LPF_butter_1_exp [] = {
    -30, -30, -30, -30, -30, -30
};

/* Butterworth filter (LPF) order-4 */
static const int32_t EQ_LPF_butter_2_mant [] = 
{
    0, 0, 0x40000000, 0x40000000, 0x30fbc54d, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x7641af3c, 0x40000000
};
static const int32_t EQ_LPF_butter_2_exp [] = {
    -30, -30, -30, -30, -30, -30,
    -30, -30, -30, -30, -30, -30
};

/* Butterworth filter (LPF) order-6 */
static const int32_t EQ_LPF_butter_3_mant [] = {
    0, 0, 0x40000000, 0x40000000, 0x2120fb83, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x5a82799a, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x7ba3751d, 0x40000000
};
static const int32_t EQ_LPF_butter_3_exp [] = {
    -30, -30, -30, -30, -30, -30,
    -30, -30, -30, -30, -30, -30,
    -30, -30, -30, -30, -30, -30
};
/************ Tables for Butterworth filter (End) *********************/

/************ Tables for Linkwitz-Riley filter (Begin) *********************/
/* Linkwitz-Riley filter (LPF) order-2 */
static const int32_t EQ_LPF_lriley_1_mant [] = {
    0, 0, 0x40000000, 0x40000000, 0x40000000, 0x40000000
};
static const int32_t EQ_LPF_lriley_1_exp [] = {
    -30, -30, -30, -30, -29, -30
};

/* Linkwitz-Riley filter (LPF) order-4 */
static const int32_t EQ_LPF_lriley_2_mant [] = {
    0, 0, 0x40000000, 0x40000000, 0x5a82799a, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x5a82799a, 0x40000000
};
static const int32_t EQ_LPF_lriley_2_exp [] = {
    -30, -30, -30, -30, -30, -30,
    -30, -30, -30, -30, -30, -30
};

/* Linkwitz-Riley filter (LPF) order-6 */
static const int32_t EQ_LPF_lriley_3_mant [] = {
    0, 0, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
    0, 0, 0x40000000, 0x40000000, 0x40000000, 0x40000000,
};
static const int32_t EQ_LPF_lriley_3_exp [] = {
    -30, -30, -30, -30, -29, -30,
    -30, -30, -30, -30, -30, -30,
    -30, -30, -30, -30, -30, -30
};
/************ Tables for Linkwitz-Riley filter (End) *********************/

static const int32_t EQ_bass_control_mant [] = {
    0x40000000, 0x5a82799a, 0x40000000, 0x40000000, 0x5a82799a, 0x40000000
};
static const int32_t EQ_bass_control_exp [] = {
    -30, -30, -30, -30, -30, -30
};


/** API functions */
int32_t EQ_generate_peq_isrsafe ( int32_t q_int, int32_t fc_int, int32_t dbval_int, int32_t fs_int, int32_t gb_int, int32_t b_int[], int32_t a_int[])
{
    int32_t bypass = 0;
    int32_t g_mant = 0, g_exp = 0;
    int32_t gb_mant = 0, gb_exp = 0;
    int32_t fc_mant = 0, fc_exp = 0;
    int32_t fs_mant = 0, fs_exp = 0;
    int32_t q_mant = 0, q_exp = 0;
    int32_t t1_mant = 0, t1_exp = 0;
    int32_t t2_mant = 0, t2_exp = 0;
    int32_t f1_mant = 0, f1_exp = 0;
    int32_t fc_a_mant = 0, fc_a_exp = 0;
    int32_t f1_a_mant = 0, f1_a_exp = 0;
    int32_t g0_mant = 0, g0_exp = 0;
    int32_t w0_mant = 0, w0_exp = 0;
    int32_t dw_mant = 0, dw_exp = 0;
    int32_t p_mant = 0, p_exp = 0;
    int32_t temp1_mant = 0, temp1_exp = 0;
    int32_t temp2_mant = 0, temp2_exp = 0;
    int32_t t3_mant = 0, t3_exp = 0;
    int32_t t4_mant = 0, t4_exp = 0;
    int32_t g1_mant = 0, g1_exp = 0;
    int32_t X_mant = 0, X_exp = 0;
    int32_t v1_mant = 0, v1_exp = 0;
    int32_t v2_mant = 0, v2_exp = 0;
    int32_t B_mant = 0, B_exp = 0;
    int32_t A_mant = 0, A_exp = 0;
    int32_t a0_mant = 0, a0_exp = 0;
    int32_t a1_mant = 0, a1_exp = 0;
    int32_t a2_mant = 0, a2_exp = 0;
    int32_t b0_mant = 0, b0_exp = 0;
    int32_t b1_mant = 0, b1_exp = 0;
    int32_t b2_mant = 0, b2_exp = 0;

    bypass = 0;
    if ((fs_int < 32000) || (fs_int > 48000))
        bypass = 1;
    if (fc_int > 20000)
        bypass = 1;

    /* Equivalent of 15.5 kHz @ 32kHz */
    /* The algorithm is unstable closer to Nyquist rate because of extreme warping */
    if (fc_int > (fs_int/2) * 31/32)
        bypass = 1;

    if (dbval_int == 0)
        bypass = 1;

    if (bypass != 0)
    {
        b_int[0] = 0x02000000;
        b_int[1] = b_int[2] = a_int[1] = a_int[2] = 0;
        return 0;
    }

    /* Limit the Q value */
    if (q_int > 1000)
        q_int = 1000;
    if (q_int < 30)
        q_int = 30;

    EQ_int2fix (dbval_int, &temp1_mant, &temp1_exp);
    EQ_int2fix (10, &temp2_mant, &temp2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp2_mant, temp2_exp);
    EQ_db_2_linear (&g_mant, &g_exp, temp1_mant, temp1_exp);

    EQ_int2fix (dbval_int * gb_int, &temp1_mant, &temp1_exp);
    EQ_int2fix (10 * 100, &temp2_mant, &temp2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp2_mant, temp2_exp);
    EQ_db_2_linear (&gb_mant, &gb_exp, temp1_mant, temp1_exp);

    EQ_int2fix(fc_int, &fc_mant, &fc_exp);
    EQ_int2fix(fs_int, &fs_mant, &fs_exp);
    EQ_int2fix(q_int, &q_mant, &q_exp);

    EQ_int2fix(100, &t1_mant, &t1_exp);
    EQ_div (&q_mant, &q_exp, q_mant, q_exp, t1_mant, t1_exp);

    EQ_int2fix (1, &t1_mant, &t1_exp);
    EQ_div (&t1_mant, &t1_exp, t1_mant, t1_exp, q_mant, q_exp);
    EQ_mul (&t2_mant, &t2_exp, t1_mant, t1_exp, t1_mant, t1_exp);
    EQ_int2fix(4, &temp1_mant, &temp1_exp);
    EQ_add (&t2_mant, &t2_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);

    EQ_sqrt(&t2_mant, &t2_exp, t2_mant, t2_exp);
    EQ_add (&f1_mant, &f1_exp, t2_mant, t2_exp-1, -t1_mant, t1_exp-1);
    EQ_mul (&f1_mant, &f1_exp, f1_mant, f1_exp, fc_mant, fc_exp);

    EQ_tan (fc_int, fs_int, &fc_a_mant, &fc_a_exp);
    EQ_tan (f1_mant >> (-f1_exp-14), fs_int<<14, &f1_a_mant, &f1_a_exp);

    p_mant = 0x6487ED51;
    p_exp = -29;
    EQ_mul (&fc_a_mant, &fc_a_exp, fc_a_mant, fc_a_exp, fs_mant, fs_exp);
    EQ_mul (&f1_a_mant, &f1_a_exp, f1_a_mant, f1_a_exp, fs_mant, fs_exp);
    EQ_div (&fc_a_mant, &fc_a_exp, fc_a_mant, fc_a_exp, p_mant, p_exp);
    EQ_div (&f1_a_mant, &f1_a_exp, f1_a_mant, f1_a_exp, p_mant, p_exp);

    EQ_div (&w0_mant, &w0_exp, fc_mant, fc_exp, fs_mant, fs_exp);
    EQ_mul (&w0_mant, &w0_exp, w0_mant, w0_exp, p_mant, p_exp+1);
    EQ_div (&dw_mant, &dw_exp, w0_mant, w0_exp, q_mant, q_exp);

    p_mant = 0x5FD18E31;
    p_exp = -29;
    p_mant = 0x6487ED51;
    p_exp = -29;
    EQ_int2fix(1, &g0_mant, &g0_exp);

    EQ_mul (&t1_mant, &t1_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp1_mant, &temp1_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_add (&t1_mant, &t1_exp, t1_mant, t1_exp, -temp1_mant, temp1_exp);

    EQ_mul (&t2_mant, &t2_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g0_mant, g0_exp, g0_mant, g0_exp);
    EQ_add (&t2_mant, &t2_exp, t2_mant, t2_exp, -temp1_mant, temp1_exp);

    EQ_mul (&t3_mant, &t3_exp, w0_mant, w0_exp, w0_mant, w0_exp);
    EQ_mul (&temp1_mant, &temp1_exp, p_mant, p_exp, p_mant, p_exp);
    EQ_add (&t3_mant, &t3_exp, t3_mant, t3_exp, -temp1_mant, temp1_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, g0_mant, g0_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g_mant, g_exp, p_mant, p_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, dw_mant, dw_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t1_mant, t1_exp);
    EQ_add (&t3_mant, &t3_exp, t3_mant, t3_exp, temp1_mant, temp1_exp);

    EQ_mul (&t4_mant, &t4_exp, w0_mant, w0_exp, w0_mant, w0_exp);
    EQ_mul (&temp1_mant, &temp1_exp, p_mant, p_exp, p_mant, p_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -temp1_mant, temp1_exp);
    EQ_mul (&t4_mant, &t4_exp, t4_mant, t4_exp, t4_mant, t4_exp);
    EQ_mul (&temp1_mant, &temp1_exp, dw_mant, dw_exp, p_mant, p_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t2_mant, t2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, t1_mant, t1_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, temp1_mant, temp1_exp);

    EQ_div (&g1_mant, &g1_exp, t3_mant, t3_exp, t4_mant, t4_exp);
    EQ_sqrt(&g1_mant, &g1_exp, g1_mant, g1_exp);

    EQ_mul (&X_mant, &X_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, g1_mant, g1_exp);
    EQ_add (&X_mant, &X_exp, X_mant, X_exp, -temp1_mant, temp1_exp);
    EQ_mul (&temp1_mant, &temp1_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&temp2_mant, &temp2_exp, g0_mant, g0_exp, g0_mant, g0_exp);
    EQ_add (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, -temp2_mant, temp2_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, temp1_mant, temp1_exp);
    EQ_sqrt (&X_mant, &X_exp, X_mant, X_exp);

    EQ_mul (&t1_mant, &t1_exp, fc_a_mant, fc_a_exp, fc_a_mant, fc_a_exp);
    EQ_mul (&X_mant, &X_exp, X_mant, X_exp, t1_mant, t1_exp);

    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, -X_mant, X_exp);
    EQ_mul (&t2_mant, &t2_exp, t2_mant, t2_exp, t2_mant, t2_exp);

    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, t1_mant, t1_exp);
    EQ_mul (&t3_mant, &t3_exp, X_mant, X_exp, g0_mant, g0_exp);
    EQ_add (&t3_mant, &t3_exp, temp1_mant, temp1_exp, -t3_mant, t3_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);

    EQ_mul (&t4_mant, &t4_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_mul (&t4_mant, &t4_exp, t2_mant, t2_exp, t4_mant, t4_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -t3_mant, t3_exp);
    EQ_div (&v1_mant, &v1_exp, t4_mant, t4_exp, t1_mant, t1_exp);

    EQ_mul (&t1_mant, &t1_exp, f1_a_mant, f1_a_exp, f1_a_mant, f1_a_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, -X_mant, X_exp);
    EQ_mul (&t2_mant, &t2_exp, t2_mant, t2_exp, t2_mant, t2_exp);

    EQ_mul (&temp1_mant, &temp1_exp, g1_mant, g1_exp, t1_mant, t1_exp);
    EQ_mul (&t3_mant, &t3_exp, X_mant, X_exp, g0_mant, g0_exp);
    EQ_add (&t3_mant, &t3_exp, temp1_mant, temp1_exp, -t3_mant, t3_exp);
    EQ_mul (&t3_mant, &t3_exp, t3_mant, t3_exp, t3_mant, t3_exp);

    EQ_mul (&t4_mant, &t4_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&t4_mant, &t4_exp, t2_mant, t2_exp, t4_mant, t4_exp);
    EQ_add (&t4_mant, &t4_exp, t4_mant, t4_exp, -t3_mant, t3_exp);
    EQ_div (&v2_mant, &v2_exp, t4_mant, t4_exp, t1_mant, t1_exp);

    EQ_mul (&temp1_mant, &temp1_exp, gb_mant, gb_exp, gb_mant, gb_exp);
    EQ_mul (&temp2_mant, &temp2_exp, g_mant, g_exp, g_mant, g_exp);
    EQ_add (&temp1_mant, &temp1_exp, temp2_mant, temp2_exp, -temp1_mant, temp1_exp);
    EQ_add (&t1_mant, &t1_exp, v2_mant, v2_exp, -v1_mant, v1_exp);
    EQ_div (&t1_mant, &t1_exp, t1_mant, t1_exp, temp1_mant, temp1_exp);

    EQ_sqrt (&A_mant, &A_exp, t1_mant, t1_exp);

    EQ_mul (&B_mant, &B_exp, A_mant, A_exp, A_mant, A_exp);
    EQ_mul (&B_mant, &B_exp, B_mant, B_exp, g_mant, g_exp);
    EQ_mul (&B_mant, &B_exp, B_mant, B_exp, g_mant, g_exp);
    EQ_add (&B_mant, &B_exp, B_mant, B_exp, v1_mant, v1_exp);
    EQ_sqrt (&B_mant, &B_exp, B_mant, B_exp);

    p_mant = 0x6487ED51;
    p_exp = -29;
    EQ_div (&fs_mant, &fs_exp, fs_mant, fs_exp, p_mant, p_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, fs_mant, fs_exp);
    EQ_div (&X_mant, &X_exp, X_mant, X_exp, fs_mant, fs_exp);
    EQ_div (&A_mant, &A_exp, A_mant, A_exp, fs_mant, fs_exp);
    EQ_div (&B_mant, &B_exp, B_mant, B_exp, fs_mant, fs_exp);

    EQ_int2fix(1, &t1_mant, &t1_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, X_mant, X_exp);

    EQ_add (&a0_mant, &a0_exp, t2_mant, t2_exp, A_mant, A_exp);
    EQ_add (&a1_mant, &a1_exp, -t1_mant, t1_exp+1, X_mant, X_exp+1);
    EQ_add (&a2_mant, &a2_exp, t2_mant, t2_exp, -A_mant, A_exp);

    EQ_div (&a2_mant, &a2_exp, a2_mant, a2_exp, a0_mant, a0_exp);
    EQ_div (&a1_mant, &a1_exp, a1_mant, a1_exp, a0_mant, a0_exp);

    EQ_mul (&t1_mant, &t1_exp, g0_mant, g0_exp, X_mant, X_exp);
    EQ_add (&t2_mant, &t2_exp, t1_mant, t1_exp, g1_mant, g1_exp);

    EQ_add (&b0_mant, &b0_exp, t2_mant, t2_exp, B_mant, B_exp);
    EQ_add (&b1_mant, &b1_exp, t1_mant, t1_exp+1, -g1_mant, g1_exp+1);
    EQ_add (&b2_mant, &b2_exp, t2_mant, t2_exp, -B_mant, B_exp);

    EQ_div (&b0_mant, &b0_exp, b0_mant, b0_exp, a0_mant, a0_exp);
    EQ_div (&b1_mant, &b1_exp, b1_mant, b1_exp, a0_mant, a0_exp);
    EQ_div (&b2_mant, &b2_exp, b2_mant, b2_exp, a0_mant, a0_exp);

    b_int[0] = b0_mant >> (-b0_exp - 25);
    b_int[1] = b1_mant >> (-b1_exp - 25);
    b_int[2] = b2_mant >> (-b2_exp - 25);

    a_int[0] = a0_mant >> (-a0_exp - 25);
    a_int[1] = a1_mant >> (-a1_exp - 25);
    a_int[2] = a2_mant >> (-a2_exp - 25);

    a_int[1] = -a_int[1];
    a_int[2] = -a_int[2];

    return 0;
}

int32_t EQ_generate_geq_isrsafe( int32_t fs_int, int32_t req_db_int[5], int32_t b0_int[5], int32_t b1_int[5], int32_t b2_int[5], int32_t a1_int[5], int32_t a2_int[5])
{
    int32_t req_f[5];
    int32_t mag[5];
    int i,j;
    int32_t ret_val = 0;
    int32_t fac = -1, fs_index = 0;

    for (i=0;i<5;i++)
    {
        req_f[i] = mag[i] = 0;
    }

    {
        int32_t max = -100;
        int32_t min = 100;
        int32_t flag = 0, diff;
        for (i=0;i<5;i++)
        {
            if (max < req_db_int[i]) max = req_db_int[i];
            if (min > req_db_int[i]) min = req_db_int[i];
        }
        if (max < -10)
            flag = 1;
        if (min > 10)
            flag = 1;
        if (flag == 1)
        {
            if (max < 0)
            {
                int32_t temp = max;
                max = -min;
                min = -temp;
            }

            if (2 * min >= max)
            {
                diff = (max - min) * 32768;
                diff = diff / max;
                diff = 16384 - diff;

                for (i=0;i<5;i++)
                {
                    fac = req_db_int[i] * diff;
                    fac = fac / 16384;
                    fac = fac / 7;      /* 15% compensation */
                    req_db_int[i] = req_db_int[i] - fac;
                }
            }
        }
    }

    fac = 1;
    fs_index = -1;
    while(fs_int)
    {
        if (fs_int == 32000) {
            fs_index = 0;
        } else if (fs_int == 44100) {
            fs_index = 1;
        } else if (fs_int == 48000) {
            fs_index = 2;
        }
        if (fs_index != -1)
            break;
        fac = fac << 1;
        fs_int = fs_int >> 1;
    }
    fs_int = fs_int * fac;

    /* Validate the sample rate */
    if (fs_index == -1)
    {
        ret_val = -1;
    }
    /* Validate the dB levels */
    for (i=0;i<5;i++)
    {
        if (req_db_int[i] > 120)
            ret_val = -1;
        if (req_db_int[i] < -120)
            ret_val = -1;
    }

    if (ret_val == 0)
    {
        for (i=0;i<5;i++)
        {
            req_db_int[i] = req_db_int[i] * 32768;
            req_db_int[i] = req_db_int[i] / 10;
        }
        for (i=0;i<5;i++)
        {
            req_f[i] = req_db_int[i];
            for(j=0;j<5;j++)
            {
                req_f[i] += EQ_mul_32_32 (EQ_matr_int[i][j], req_db_int[j]);
            }
        }
        /* Print the power table */
        for (i=0;i<5;i++)
        {
            int32_t q_inv = 0x48ba2e8b; /* 1/0.22 in 4.28 format */
            int32_t aa1, aa2, A1, A2, B1, B2, div;
            int32_t num;
            if (req_f[i] == 0)
            {
                a1_int[i] = 0x0;
                a2_int[i] = 0x0;
                b0_int[i] = 0x2000000 << 4;
                b1_int[i] = 0x0;
                b2_int[i] = 0x0;
                continue;
            }
            if (req_f[i] < 0)
                mag[i] = EQ_db_2_linear_table(-req_f[i], EQ_pow_table_db_int_neg);
            else
                mag[i] = EQ_db_2_linear_table(req_f[i], EQ_pow_table_db_int_neg);

            /* main for loop for generating coefs */
            if (req_f[i] < 0)
            {
                aa2 = q_inv;
                aa1 = EQ_mul_32_32 (mag[i], q_inv) << 3;
            }
            else
            {
                aa1 = q_inv;
                aa2 = EQ_mul_32_32 (mag[i], q_inv) << 3;
            }

            A1 = EQ_mul_32_32 (aa1, EQ_GEQ_A_table[i][fs_index]) / fac;
            A2 = EQ_mul_32_32 (aa2, EQ_GEQ_A_table[i][fs_index]) / fac;

            B1 = EQ_GEQ_B_table[i][fs_index] / 2;
            B2 = EQ_GEQ_B_table[i][fs_index] / 2;

            B1 = B1 / fac / fac;
            B2 = B2 / fac / fac;

            div = 0x10000000 + A2 + B2;

            num = -(2*B2 - 0x20000000);
            a1_int[i] = EQ_div_32_32 (num>>1 , div);

            num = -(0x10000000 + B2 - A2);
            a2_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (0x10000000 + B1 + A1);
            b0_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (2*B1 - 0x20000000);
            b1_int[i] = EQ_div_32_32 (num>>1 , div);

            num = (0x10000000 + B1 - A1);
            b2_int[i] = EQ_div_32_32 (num>>1 , div);
        }

        for (i=0;i<5;i++)
        {
            a1_int[i] = a1_int[i] >> 4;
            a2_int[i] = a2_int[i] >> 4;
            b0_int[i] = b0_int[i] >> 4;
            b1_int[i] = b1_int[i] >> 4;
            b2_int[i] = b2_int[i] >> 4;
        }
    }
    else
    {
        /* Disable all IIR filters (bypass) */
        for (i=0;i<5;i++)
        {
            a1_int[i] = 0;
            a2_int[i] = 0;
            b0_int[i] = 0x0200000;
            b1_int[i] = 0;
            b2_int[i] = 0;
        }
    }
    return ret_val;
}

int32_t EQ_generate_tone_control_isrsafe (int32_t fs, int32_t gain_bass, int32_t gain_treble,
        int32_t fc_bass, int32_t fc_treble, BAPE_ToneControlEqType type_bass,
	   	BAPE_ToneControlEqType type_treble, int32_t bandwidth_bass,
	   	int32_t bandwidth_treble,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[])
{
	/* Initialize the coefs */
	b0[0] = b0[1] = 0x2000000;
	b1[0] = b1[1] = b2[0] = b2[1] = 0x0;
	a1[0] = a1[1] = a2[0] = a2[1] = 0x0;

	if (type_bass == BAPE_ToneControlEqType_eShelving)
	{
		EQ_generate_bass (gain_bass, fs, fc_bass,
				&b0[1], &b1[1], &b2[1], &a1[1], &a2[1]);
	}
	else
	{
		/* Find the q = fc/Bandwidth */
		int32_t q_mant, q_exp;
		int32_t b[3], a[3];
		EQ_div (&q_mant, &q_exp, fc_bass * 100, 0, bandwidth_bass, 0);
		q_mant = q_mant >> (-q_exp);
		EQ_generate_peq_isrsafe (q_mant, fc_bass, gain_bass, fs, 70, b, a);
		b0[1] = b[0];
		b1[1] = b[1];
		b2[1] = b[2];

		a1[1] = a[1];
		a2[1] = a[2];
	}
	if (type_treble == BAPE_ToneControlEqType_eShelving)
	{
		EQ_generate_treble (gain_treble, fs, fc_treble,
				&b0[0], &b1[0], &b2[0], &a1[0], &a2[0]);
	}
	else
	{
		/* Find the q = fc/Bandwidth */
		int32_t q_mant, q_exp;
		int32_t b[3], a[3];
		EQ_div (&q_mant, &q_exp, fc_treble * 100, 0, bandwidth_treble, 0);
		q_mant = q_mant >> (-q_exp);
		EQ_generate_peq_isrsafe (q_mant, fc_treble, gain_treble, fs, 70, b, a);
		b0[0] = b[0];
		b1[0] = b[1];
		b2[0] = b[2];

		a1[0] = a[1];
		a2[0] = a[2];
	}

    /* Multiply the Bass filter by 2 and divide the treble filter by 2 */
    b0[0] = b0[0] >> 1;
    b1[0] = b1[0] >> 1;
    b2[0] = b2[0] >> 1;

    b0[1] = b0[1] << 1;
    b1[1] = b1[1] << 1;
    b2[1] = b2[1] << 1;

    return 0;
}

int32_t EQ_subwoofer_LPF_isrsafe (int32_t fs, int32_t fc, int32_t type, int32_t order,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[])
{
    int32_t biquad_mant[6], biquad_exp[6];
    static const int32_t *ptr_mant;
    static const int32_t *ptr_exp;
    int32_t i;
    int32_t ret_val = 0;

    ptr_mant = &EQ_LPF_butter_2_mant[0];
    ptr_exp = &EQ_LPF_butter_2_exp[0];
    for (i=0;i<6;i++)
    {
        biquad_mant [i] = biquad_exp[i] = 0;
    }

    if ((fs < 32000) || (fs > 48000))
    {
        ret_val = -1;
    }

    if ((fc < 30) || (fc > (fs/2-30)))
    {
        ret_val = -1;
    }

    if ((order < 2) || (order > 4))
    {
        /* Do not fill anything in this case */
        ret_val = -1;
    }

    if (ret_val < 0)
    {
        for (i=0;i<3;i++)
        {
            b0[i] = 0x2000000;
            b1[i] = b2[i] = a1[i] = a2[i] = 0x0;
        }
        return 0;
    }

    if (type == EQ_FILTER_TYPE_BUTTER)
    {
        /* Butter-worth filter */
        switch (order)
        {
            case 2:
                ptr_mant = &EQ_LPF_butter_1_mant[0];
                ptr_exp = &EQ_LPF_butter_1_exp[0];
                break;
            case 4:
                ptr_mant = &EQ_LPF_butter_2_mant[0];
                ptr_exp = &EQ_LPF_butter_2_exp[0];
                break;
#if 0 /* Fix coverity issue. Either remove this or handle accordingly at above */
            case 6:
                ptr_mant = &EQ_LPF_butter_3_mant[0];
                ptr_exp = &EQ_LPF_butter_3_exp[0];
                break;
#endif
            default:
                break;
        }
    }
    else if (type == EQ_FILTER_TYPE_LRILEY)
    {
        /* Linkwitz-Riley filter */
        switch (order)
        {
            case 2:
                ptr_mant = &EQ_LPF_lriley_1_mant[0];
                ptr_exp = &EQ_LPF_lriley_1_exp[0];
                break;
            case 4:
                ptr_mant = &EQ_LPF_lriley_2_mant[0];
                ptr_exp = &EQ_LPF_lriley_2_exp[0];
                break;
#if 0 /* Fix coverity issue. Either remove this or handle accordingly at above */
            case 6:
                ptr_mant = &EQ_LPF_lriley_3_mant[0];
                ptr_exp = &EQ_LPF_lriley_3_exp[0];
                break;
#endif                
            default:
                break;                
        }
    }
    else
    {
        /* Fill with default values */
        for (i=0;i<order/2;i++)
        {
            b0[i] = 0x02000000;
            b1[i] = b2[i] = a1[i] = a2[i] = 0;
        }
        return 0;
    }

    for (i=0;i<order/2;i++)
    {
        EQ_convert_analog_to_digital (
                &ptr_mant [6*i+0],
                &ptr_exp [6*i+0],
                fc,
                fs,
                &biquad_mant[0],
                &biquad_exp[0]
                );

        b0[i] = biquad_mant[0] >> (-biquad_exp[0] - 25);
        b1[i] = biquad_mant[1] >> (-biquad_exp[1] - 25);
        b2[i] = biquad_mant[2] >> (-biquad_exp[2] - 25);
        a1[i] = -biquad_mant[4] >> (-biquad_exp[4] - 25);
        a2[i] = -biquad_mant[5] >> (-biquad_exp[5] - 25);
    }
    return 0;
}

int32_t EQ_subsonic_HPF_isrsafe (int32_t fs, int32_t fc, int32_t type, int32_t order,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[])
{
    int32_t i;
    EQ_subwoofer_LPF_isrsafe (fs, (fs/2) - fc, type, order, b0, b1, b2, a1, a2);
    for (i=0;i<order/2;i++)
    {
        b1[i] = -b1[i];
        a1[i] = -a1[i];
    }
    return 0;
}
/*********** End of API functions *************/

/* Functions internal to Equalizer module */
int32_t EQ_db_2_linear_table(int32_t in, const int32_t table[])
{
    int32_t ret, i;
    ret = 0x10000000;
    for (i=0;i<20;i++)
    {
        if (in & (1 << i))
        {
            ret = EQ_mul_32_32 (ret, table[i]) << 3;
        }
    }
    return ret;
}

void EQ_div (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    *dest_mant = EQ_div_32_32 (mant>>1, src_mant);
    *dest_exp = exp - src_exp - 30 + 1;
    return;
}

void EQ_add (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    EQ_add_32_32 (&mant, &exp, src_mant, src_exp);
    *dest_exp = exp;
    *dest_mant = mant;
    return;
}

void EQ_mul (int32_t *dest_mant, int32_t *dest_exp, int32_t mant, int32_t exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&mant, &exp);
    EQ_renormalise (&src_mant, &src_exp);
    *dest_mant = EQ_mul_32_32 (mant, src_mant);
    *dest_exp = exp + src_exp + 31;
    return;
}

void EQ_int2fix(int32_t inp, int32_t *mant, int32_t *exp)
{
    int32_t sign;
    int32_t i;

    sign = 1;
    if (inp < 0)
    {
        inp = -inp;
        sign = -1;
    }
    /* Normalize the input */
    i = 0;
    if (inp != 0)
    {
        while ((inp & 0xc0000000) == 0)
        {
            inp = inp << 1;
            i++;
        }
    }
    *mant = inp * sign;
    *exp = -i;
    return;
}

int32_t EQ_mul_32_32 (int32_t a, int32_t b)
{
    int32_t a1, a0, b1, b0;
    int32_t acc;

    a0 = a & 0x0000ffff;
    b0 = b & 0x0000ffff;

    /* preserve the sign bits */
    a1 = (a >> 16);
    b1 = (b >> 16);

    acc = 0;
    /* First add the unsigned muls */
    acc += a1 * b1;
    acc = (acc >> 17) & 0x00007fff;     /* Unsigned number */
    /* a1 * b0 + a0 * b1; Shift by 1 to avoid overflow */
    acc += (a1 * b0) >> 1;
    acc += (b1 * a0) >> 1;

    acc = acc >> 15;
    /* Now add a1 * b1 */
    acc += a1 * b1;
    if (acc == 0x40000000) {
        /* Overflow. Will happen when a = 0x80000000
           and b = 0x80000000 and numbers close to it */
        acc = 0x7fffffff;
    } else {
        acc = acc << 1;
    }

    return acc;
}

void EQ_add_32_32 (int32_t *dest_mant, int32_t *dest_exp, int32_t src_mant, int32_t src_exp)
{
    int32_t ret = 0;
    if (*dest_mant == 0)
    {
        *dest_mant = src_mant;
        *dest_exp = src_exp;
        ret = 1;
    }
    if (src_mant == 0)
    {
        /* Nothing to do */
        ret = 1;
    }
    if (ret == 0)
    {
        /* Reduce the exp by one to avoid overflow */
        *dest_mant = *dest_mant >> 1;
        *dest_exp = *dest_exp + 1;
        src_mant = src_mant >> 1;
        src_exp = src_exp + 1;
        if (src_exp != *dest_exp)
        {
            if (src_exp > *dest_exp)
            {
                if (src_exp > *dest_exp + 31)
                {
                    *dest_mant = 0;
                    *dest_exp = src_exp;
                }
                else
                {
                    *dest_mant = (*dest_mant) >> (src_exp - (*dest_exp));
                    *dest_exp = src_exp;
                }
            }
            else
            {
                if (src_exp+31 < *dest_exp)
                {
                    src_mant = 0;
                    src_exp = *dest_exp;
                }
                else
                {
                    src_mant = (src_mant) >> (*dest_exp - (src_exp));
                    src_exp = *dest_exp;
                }
            }
        }
        *dest_mant = *dest_mant + src_mant;
    }
    return;
}

void EQ_renormalise (int32_t *p_mant, int32_t *p_exp)
{
    int32_t ret = 0;
    int32_t mant, exp;
    int32_t sign = 1;
    mant = *p_mant;
    exp = *p_exp;
    if (mant == 0)
        ret = 1;
    if (ret == 0)
    {
        if (mant < 0)
        {
            mant = -mant;
            sign = -1;
        }
        while ((mant & 0xc0000000) == 0)
        {
            mant = mant << 1;
            exp = exp - 1;
        }
        *p_mant = mant * sign;
        *p_exp = exp;
    }
    return;
}

int32_t EQ_div_32_32 (int32_t num, int32_t den)
{
    int32_t i, res = 0;
    int32_t begin = 0, end = 0, mid = 0;
    int32_t sign = 1;
    if (num == 0)
        return 0;
    if (den == 0)
        return 0;
    if (num < 0)
    {
        num = -num;
        sign = -sign;
    }
    if (den < 0)
    {
        den = -den;
        sign = -sign;
    }

    num = num/2;
    if (num > den) return 0;

    begin = 0x00000000;
    end = 0x40000000;
    for (i=0;i<32;i++)
    {
        mid = (begin + end)/2;

        res = EQ_mul_32_32 (mid, den);
        if (res > num )
            end = mid;
        else
            begin = mid;
    }

    return end*sign;        /* num is already divided by 2. */
    /* The return value should be in 2.30 format. Both are compensated */
}

void EQ_sqrt (int32_t *dest_mant, int32_t *dest_exp, int32_t src_mant, int32_t src_exp)
{
    EQ_renormalise (&src_mant, &src_exp);
    if ((src_exp&1) == 0)
    {
        src_mant = src_mant >> 1;
        src_exp = src_exp + 1;
    }
    *dest_mant = EQ_sqrt_32 (src_mant);
    *dest_exp = (src_exp+1)/2 - 16;
    return;
}

int32_t EQ_sqrt_32 (int32_t in)
{
    int32_t begin = 0, end = 0, mid = 0;
    int32_t i, res = 0;
    begin = 0;
    end = 0x40000000;
    if (in < 0) return 0;
    in = in >> 2;

    for (i=0;i<30;i++)
    {
        mid = (begin+end)/2;
        res = EQ_mul_32_32 (mid, mid);
        if (res > in)
            end = mid;
        else
            begin = mid;
    }
    end = end << 1;
    if (end < 0)        /* Corner case of 0x80000000 */
        end--;
    return end;
}

void EQ_tan(int32_t f1, int32_t f2, int32_t *fc_a_mant, int32_t *fc_a_exp)
{
    int32_t phase = 0;
    int32_t cos_val = 0, sin_val = 0;
    int32_t sin_mant = 0, sin_exp = 0;
    int32_t cos_mant = 0, cos_exp = 0;
    int32_t f1_exp = 0, f2_exp = 0;

    if (f1 > f2/2)
    {
        *fc_a_mant = 0;
        *fc_a_exp = 0;
    }

    EQ_renormalise (&f1, &f1_exp);
    EQ_renormalise (&f2, &f2_exp);
    phase = EQ_div_32_32(f1>>1, f2);
    phase = phase >> (f2_exp - f1_exp - 1);
    EQ_cos_sin (phase, &cos_val, &sin_val);

    EQ_int2fix (sin_val, &sin_mant, &sin_exp);
    EQ_int2fix (cos_val, &cos_mant, &cos_exp);
    EQ_div (fc_a_mant, fc_a_exp, sin_mant, sin_exp, cos_mant, cos_exp);
    return;
}

void EQ_cos_sin (int32_t phase, int32_t *cos_val, int32_t *sin_val)
{
    int32_t sign = 0;
    int32_t sv = 0, cv = 0, i;
    *cos_val = 0;
    *sin_val = 0;
    sign = 1;
    if (phase < 0)
    {
        phase = -phase;
        sign = -1;
    }
    /* Supports only between +/- PI */
    if (phase > 0x3fffffff)
        return;

    sv = 0x00000000;
    cv = 0x40000000;
    for (i=0;i<30;i++)
    {
        if (phase & (1<<i))
        {
            int32_t stemp, ctemp;
            /* Update both sin and cos value */
            /* sin(A+B) = sinA cosB + cosA sinB */
            /* cos(A+B) = cosA cosB - sinA sinB */
            stemp = EQ_mul_32_32 (sv, EQ_cos_table[i]);
            stemp += EQ_mul_32_32 (cv, EQ_sin_table[i]);

            ctemp = EQ_mul_32_32 (cv, EQ_cos_table[i]);
            ctemp -= EQ_mul_32_32 (sv, EQ_sin_table[i]);

            sv = stemp << 1;
            cv = ctemp << 1;
        }
    }
    *sin_val = sv * sign;
    *cos_val = cv;
    return;
}

void EQ_db_2_linear (int32_t *dest_mant, int32_t *dest_exp, int32_t src_mant, int32_t src_exp)
{
    /* Renormalize the db values in 16.16 format */
    int sign = 0, i;
    int32_t val_mant = 0, val_exp = 0;
    int32_t t1_mant = 0, t1_exp = 0;
    EQ_renormalise (&src_mant, &src_exp);
    if (src_mant != 0)
    {
        src_mant >>= -src_exp - 16;
        src_exp = -16;
    }
    sign = 0;
    if (src_mant < 0)
    {
        sign = 1;
        src_mant = -src_mant;
    }
    EQ_int2fix (1, &val_mant, &val_exp);
    for (i=0;i<16+10;i++)
    {
        if ((src_mant & (1<<i)) != 0)
        {
            t1_mant = EQ_db2linear_mant_table[i];
            t1_exp = EQ_db2linear_exp_table[i];
            EQ_mul (&val_mant, &val_exp, val_mant, val_exp, t1_mant, t1_exp);
        }
    }
    if (sign)
    {
        EQ_int2fix (1, &t1_mant, &t1_exp);
        EQ_div (&val_mant, &val_exp, t1_mant, t1_exp, val_mant, val_exp);
    }
    *dest_mant = val_mant;
    *dest_exp = val_exp;
    return;
}

void EQ_convert_analog_to_digital (const int32_t ptr_mant[], const int32_t ptr_exp[], int32_t fc, int32_t fs, int32_t coef_mant[], int32_t coef_exp[])
{
    /*
       The input is an analog filter with normalized frequency.
       In order to convert to digital, the sample rate and the frequency of
       interest to be taken into account. Filters using multiple frequency
       point of interest should use different algorithm like Band-pass filter
       (f1, f2), Parametric equalzier etc (fc, f1). This algorithm can be used
       for Bass control, Treble control, low-pass filter, high-pass filter, etc
     */
    /* The coefs represent 2nd order butter-worth filter with cut-off at 1Hz */
    /* Transform this to required frequency and required sample rate */
    /* s = (1-z^-1)/(1+z^-1) * (1/(fs/2)) * fc */

    /* Find the warping frequency Find the warping frequency */

    int32_t b0_a_mant = 0, b0_a_exp = 0, b1_a_mant = 0, b1_a_exp = 0, b2_a_mant = 0, b2_a_exp = 0;
    int32_t a0_a_mant = 0, a0_a_exp = 0, a1_a_mant = 0, a1_a_exp = 0, a2_a_mant = 0, a2_a_exp = 0;
    int32_t a0_mant = 0, a0_exp = 0, a1_mant = 0, a1_exp = 0, a2_mant = 0, a2_exp = 0;
    int32_t b0_mant = 0, b0_exp = 0, b1_mant = 0, b1_exp = 0, b2_mant = 0, b2_exp = 0;
    int32_t fc_mant = 0, fc_exp = 0, fs_mant = 0, fs_exp = 0, fac_mant = 0, fac_exp = 0;
    int32_t pi_mant = 0, pi_exp = 0;

    EQ_int2fix(fs, &fs_mant, &fs_exp);
    pi_mant = 0x6487ED51;
    pi_exp = -29;

    /* Frequency warping */
    EQ_tan (fc, fs, &fc_mant, &fc_exp);
    EQ_mul (&fc_mant, &fc_exp, fc_mant, fc_exp, fs, 0);
    EQ_div (&fc_mant, &fc_exp, fc_mant, fc_exp, pi_mant, pi_exp);

    EQ_div (&fac_mant, &fac_exp, fc_mant, fc_exp, fs_mant, fs_exp);
    EQ_mul (&fac_mant, &fac_exp, fac_mant, fac_exp, pi_mant, pi_exp);

    b0_a_exp = ptr_exp[0];
    b1_a_exp = ptr_exp[1];
    b2_a_exp = ptr_exp[2];
    a0_a_exp = ptr_exp[3];
    a1_a_exp = ptr_exp[4];
    a2_a_exp = ptr_exp[5];

    b0_a_mant = ptr_mant[0];
    b1_a_mant = ptr_mant[1];
    b2_a_mant = ptr_mant[2];
    a0_a_mant = ptr_mant[3];
    a1_a_mant = ptr_mant[4];
    a2_a_mant = ptr_mant[5];

    EQ_mul (&b1_a_mant, &b1_a_exp, b1_a_mant, b1_a_exp, fac_mant, fac_exp);
    EQ_mul (&b2_a_mant, &b2_a_exp, b2_a_mant, b2_a_exp, fac_mant, fac_exp);
    EQ_mul (&b2_a_mant, &b2_a_exp, b2_a_mant, b2_a_exp, fac_mant, fac_exp);

    EQ_mul (&a1_a_mant, &a1_a_exp, a1_a_mant, a1_a_exp, fac_mant, fac_exp);
    EQ_mul (&a2_a_mant, &a2_a_exp, a2_a_mant, a2_a_exp, fac_mant, fac_exp);
    EQ_mul (&a2_a_mant, &a2_a_exp, a2_a_mant, a2_a_exp, fac_mant, fac_exp);

    EQ_add (&b0_mant, &b0_exp, b0_a_mant, b0_a_exp, b1_a_mant, b1_a_exp);
    EQ_add (&b0_mant, &b0_exp, b0_mant, b0_exp, b2_a_mant, b2_a_exp);
    EQ_add (&b1_mant, &b1_exp, -b0_a_mant, b0_a_exp+1, b2_a_mant, b2_a_exp+1);
    EQ_add (&b2_mant, &b2_exp, b0_a_mant, b0_a_exp, -b1_a_mant, b1_a_exp);
    EQ_add (&b2_mant, &b2_exp, b2_mant, b2_exp, b2_a_mant, b2_a_exp);

    EQ_add (&a0_mant, &a0_exp, a0_a_mant, a0_a_exp, a1_a_mant, a1_a_exp);
    EQ_add (&a0_mant, &a0_exp, a0_mant, a0_exp, a2_a_mant, a2_a_exp);
    EQ_add (&a1_mant, &a1_exp, -a0_a_mant, a0_a_exp+1, a2_a_mant, a2_a_exp+1);
    EQ_add (&a2_mant, &a2_exp, a0_a_mant, a0_a_exp, -a1_a_mant, a1_a_exp);
    EQ_add (&a2_mant, &a2_exp, a2_mant, a2_exp, a2_a_mant, a2_a_exp);

    EQ_div (&b0_mant, &b0_exp, b0_mant, b0_exp, a0_mant, a0_exp);
    EQ_div (&b1_mant, &b1_exp, b1_mant, b1_exp, a0_mant, a0_exp);
    EQ_div (&b2_mant, &b2_exp, b2_mant, b2_exp, a0_mant, a0_exp);
    EQ_div (&a1_mant, &a1_exp, a1_mant, a1_exp, a0_mant, a0_exp);
    EQ_div (&a2_mant, &a2_exp, a2_mant, a2_exp, a0_mant, a0_exp);

    EQ_div (&a0_mant, &a0_exp, a0_mant, a0_exp, a0_mant, a0_exp);

    coef_exp[0] = b0_exp;
    coef_exp[1] = b1_exp;
    coef_exp[2] = b2_exp;
    coef_exp[3] = a0_exp;
    coef_exp[4] = a1_exp;
    coef_exp[5] = a2_exp;

    coef_mant[0] = b0_mant;
    coef_mant[1] = b1_mant;
    coef_mant[2] = b2_mant;
    coef_mant[3] = a0_mant;
    coef_mant[4] = a1_mant;
    coef_mant[5] = a2_mant;

    return;
}

/* Type - 0 Butterworth filter
   Type - 1 Linkwitz-Riley */

int32_t EQ_generate_bass (int32_t gain, int32_t fs, int32_t fc,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[])
{
    int32_t temp1_mant = 0, temp1_exp = 0;
    int32_t temp2_mant = 0, temp2_exp = 0;
    int32_t g_mant = 0, g_exp = 0;
    int32_t g1_mant = 0, g1_exp = 0;
    int32_t coef_a_mant[6], coef_a_exp[6];
    int32_t coef_mant[6], coef_exp[6];
    int32_t sign = 0, fac_mant = 0, fac_exp = 0, i;

    for (i=0;i<6;i++)
    {
        coef_a_mant [i] = coef_a_exp [i] = 0;
        coef_mant [i] = coef_exp [i] = 0;
    }

    if ( (fs < 32000) || (fs > 48000))
    {
        return 0;
    }
    if ((fc < 30) || (fc > (fs/2)))
    {
        return 0;
    }

    sign = 1;
    if (gain < 0)
    {
        gain = -gain;
        sign = -1;
    }

    /* Find the gain value and apply the gain */
    EQ_int2fix (gain, &temp1_mant, &temp1_exp);
    EQ_int2fix (10, &temp2_mant, &temp2_exp);
    EQ_div (&temp1_mant, &temp1_exp, temp1_mant, temp1_exp, temp2_mant, temp2_exp);
    EQ_db_2_linear (&g_mant, &g_exp, temp1_mant, temp1_exp);
    EQ_sqrt (&g1_mant, &g1_exp, g_mant, g_exp);

    /* Apply the factor to b1 and b2 */
    coef_a_mant[0] = EQ_bass_control_mant[0];
    coef_a_mant[1] = EQ_bass_control_mant[1];
    coef_a_mant[2] = EQ_bass_control_mant[2];
    coef_a_mant[3] = EQ_bass_control_mant[3];
    coef_a_mant[4] = EQ_bass_control_mant[4];
    coef_a_mant[5] = EQ_bass_control_mant[5];

    coef_a_exp[0] = EQ_bass_control_exp[0];
    coef_a_exp[1] = EQ_bass_control_exp[1];
    coef_a_exp[2] = EQ_bass_control_exp[2];
    coef_a_exp[3] = EQ_bass_control_exp[3];
    coef_a_exp[4] = EQ_bass_control_exp[4];
    coef_a_exp[5] = EQ_bass_control_exp[5];

    EQ_mul (&coef_a_mant[1], &coef_a_exp[1], coef_a_mant[1], coef_a_exp[1], g1_mant, g1_exp);
    EQ_mul (&coef_a_mant[2], &coef_a_exp[2], coef_a_mant[2], coef_a_exp[2], g_mant, g_exp);

    EQ_convert_analog_to_digital (
            &coef_a_mant[0],
            &coef_a_exp[0],
            fc,
            fs,
            &coef_mant[0],
            &coef_exp[0]
            );

    if (sign == -1)
    {
        /* Inverse fitler to be implemented */
        fac_mant = coef_mant[0];
        fac_exp = coef_exp[0];
        for (i=0;i<6;i++)
            EQ_div (&coef_mant[i], &coef_exp[i], coef_mant[i], coef_exp[i], fac_mant, fac_exp);
        for (i=0;i<3;i++)
        {
            /* Interchange Numerator and denominator */
            fac_mant = coef_mant[i];
            coef_mant[i] = coef_mant[i+3];
            coef_mant[i+3] = fac_mant;

            fac_exp = coef_exp[i];
            coef_exp[i] = coef_exp[i+3];
            coef_exp[i+3] = fac_exp;
        }
    }
    b0[0] = coef_mant[0] >> (-coef_exp[0] - 25);
    b1[0] = coef_mant[1] >> (-coef_exp[1] - 25);
    b2[0] = coef_mant[2] >> (-coef_exp[2] - 25);
    a1[0] = -coef_mant[4] >> (-coef_exp[4] - 25);
    a2[0] = -coef_mant[5] >> (-coef_exp[5] - 25);
    return 0;
}
int32_t EQ_generate_treble (int32_t gain, int32_t fs, int32_t fc,
        int32_t b0[], int32_t b1[], int32_t b2[], int32_t a1[], int32_t a2[])
{
    EQ_generate_bass (gain, fs, (fs/2) - fc, b0, b1, b2, a1, a2);
    b1[0] = -b1[0];
    a1[0] = -a1[0];
    return 0;
}
#endif /* BAPE_CHIP_SRC_TYPE_IS_IIR */  

