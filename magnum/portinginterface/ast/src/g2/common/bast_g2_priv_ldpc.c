/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

/* #define BAST_LDPC_DEBUG */
BDBG_MODULE(bast_g2_priv_ldpc);


/* this data was generated on 03-28-2011 11:10:50 */
/* table data range 9, 255 --> OK */

#define BAST_PLC_NUM_SYMRATES 6
#define BAST_PLC_DAMP_SCALE 8

/* acq_b0_default_piloton */
static const uint8_t BAST_bw_acq_b0_default_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	69	,
	52	,
	33	,
	37	,
	47	,
	69
};

static const uint8_t BAST_damp_acq_b0_default_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	72	,
	70	,
	50	,
	48	,
	28	,
	28
};

static const uint8_t BAST_bw_acq_b0_default_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	80	,
	46	,
	36	,
	45	,
	45	,
	70
};

static const uint8_t BAST_damp_acq_b0_default_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	62	,
	64	,
	48	,
	46	,
	28	,
	28
};

static const uint8_t BAST_bw_acq_b0_default_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	88	,
	49	,
	36	,
	51	,
	60	,
	72
};

static const uint8_t BAST_damp_acq_b0_default_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	72	,
	68	,
	56	,
	56	,
	36	,
	36
};

static const uint8_t BAST_bw_acq_b0_default_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	85	,
	55	,
	32	,
	42	,
	64	,
	80
};

static const uint8_t BAST_damp_acq_b0_default_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	50	,
	42	,
	40	,
	37	,
	28	,
	28
};

/* acq_b0_default_pilotoff */
static const uint8_t BAST_bw_acq_b0_default_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	111	,
	71	,
	69	,
	93	,
	97	,
	103
};

static const uint8_t BAST_damp_acq_b0_default_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	82	,
	72	,
	66	,
	60	,
	54	,
	46
};

static const uint8_t BAST_bw_acq_b0_default_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	137	,
	72	,
	78	,
	102	,
	107	,
	103
};

static const uint8_t BAST_damp_acq_b0_default_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	98	,
	86	,
	72	,
	60	,
	54	,
	46
};

static const uint8_t BAST_bw_acq_b0_default_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	135	,
	95	,
	75	,
	88	,
	102	,
	109
};

static const uint8_t BAST_damp_acq_b0_default_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	118	,
	84	,
	86	,
	76	,
	64	,
	56
};

static const uint8_t BAST_bw_acq_b0_default_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	105	,
	111	,
	135	,
	141	,
	136	,
	133
};

static const uint8_t BAST_damp_acq_b0_default_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	98	,
	86	,
	96	,
	78	,
	64	,
	56
};

/* acq_b0_euro_piloton */
static const uint8_t BAST_bw_acq_b0_euro_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	121	,
	158	,
	188	,
	96	,
	107	,
	109
};

static const uint8_t BAST_damp_acq_b0_euro_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	84	,
	72	,
	70	,
	68	,
	52	,
	48
};

static const uint8_t BAST_bw_acq_b0_euro_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	131	,
	166	,
	153	,
	138	,
	155	,
	150
};

static const uint8_t BAST_damp_acq_b0_euro_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	66	,
	72	,
	66	,
	64	,
	56	,
	48
};

static const uint8_t BAST_bw_acq_b0_euro_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	108	,
	117	,
	136	,
	189	,
	184	,
	156
};

static const uint8_t BAST_damp_acq_b0_euro_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	80	,
	60	,
	60	,
	60	,
	56	,
	48
};

static const uint8_t BAST_bw_acq_b0_euro_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	102	,
	80	,
	90	,
	84	,
	83	,
	109
};

static const uint8_t BAST_damp_acq_b0_euro_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	90	,
	82	,
	76	,
	64	,
	56	,
	48
};

/* acq_b0_euro_pilotoff */
static const uint8_t BAST_bw_acq_b0_euro_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	242	,
	185	,
	108	,
	177	,
	175	,
	172
};

static const uint8_t BAST_damp_acq_b0_euro_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	112	,
	112	,
	116	,
	98	,
	76	,
	64
};

static const uint8_t BAST_bw_acq_b0_euro_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	191	,
	154	,
	114	,
	162	,
	175	,
	164
};

static const uint8_t BAST_damp_acq_b0_euro_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	136	,
	120	,
	104	,
	68	,
	64	,
	56
};

static const uint8_t BAST_bw_acq_b0_euro_pilotoff_8PSK_low[] = {
	255	,
	255	,
	255	,
	254	,
	252	,
	250
};

static const uint8_t BAST_damp_acq_b0_euro_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	160	,
	184	,
	120	,
	84	,
	72	,
	64
};

static const uint8_t BAST_bw_acq_b0_euro_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	223	,
	185	,
	218	,
	162	,
	160	,
	156
};

static const uint8_t BAST_damp_acq_b0_euro_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	160	,
	128	,
	106	,
	92	,
	76	,
	64
};

/* trk_b0_default_piloton */
static const uint8_t BAST_bw_trk_b0_default_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	45	,
	49	,
	32	,
	42	,
	40	,
	49
};

static const uint8_t BAST_damp_trk_b0_default_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	144	,
	144	,
	104	,
	88	,
	52	,
	28
};

static const uint8_t BAST_bw_trk_b0_default_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	47	,
	73	,
	37	,
	48	,
	42	,
	51
};

static const uint8_t BAST_damp_trk_b0_default_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	104	,
	96	,
	88	,
	80	,
	52	,
	28
};

static const uint8_t BAST_bw_trk_b0_default_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	26	,
	9	,
	25	,
	32	,
	39	,
	54
};

static const uint8_t BAST_damp_trk_b0_default_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	59	,
	59	,
	52	,
	48	,
	36	,
	28
};

static const uint8_t BAST_bw_trk_b0_default_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	19	,
	9	,
	28	,
	42	,
	41	,
	57
};

static const uint8_t BAST_damp_trk_b0_default_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	224	,
	176	,
	90	,
	48	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_default_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	44	,
	36	,
	35	,
	42	,
	43	,
	60
};

static const uint8_t BAST_damp_trk_b0_default_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	82	,
	82	,
	82	,
	82	,
	50	,
	32
};

static const uint8_t BAST_bw_trk_b0_default_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	54	,
	47	,
	48	,
	29	,
	42	,
	63
};

static const uint8_t BAST_damp_trk_b0_default_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	80	,
	80	,
	80	,
	36	,
	32	,
	28
};

/* trk_b0_euro_piloton */
static const uint8_t BAST_bw_trk_b0_euro_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	233	,
	234	,
	208	,
	139	,
	133	,
	100
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	96	,
	84	,
	62	,
	62	,
	54	,
	40
};

static const uint8_t BAST_bw_trk_b0_euro_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	232	,
	232	,
	190	,
	148	,
	143	,
	114
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	92	,
	78	,
	66	,
	80	,
	60	,
	48
};

static const uint8_t BAST_bw_trk_b0_euro_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	158	,
	183	,
	123	,
	152	,
	143	,
	114
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	80	,
	72	,
	64	,
	64	,
	56	,
	48
};

static const uint8_t BAST_bw_trk_b0_euro_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	146	,
	146	,
	190	,
	239	,
	210	,
	186
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	88	,
	76	,
	72	,
	61	,
	56	,
	48
};

static const uint8_t BAST_bw_trk_b0_euro_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	144	,
	164	,
	190	,
	187	,
	167	,
	150
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	80	,
	80	,
	64	,
	63	,
	56	,
	48
};

static const uint8_t BAST_bw_trk_b0_euro_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	134	,
	124	,
	121	,
	145	,
	133	,
	114
};

static const uint8_t BAST_damp_trk_b0_euro_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	80	,
	92	,
	64	,
	60	,
	54	,
	48
};

/* trk_b0_awgn_piloton */
static const uint8_t BAST_bw_trk_b0_awgn_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	56	,
	47	,
	28	,
	31	,
	35	,
	49
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	72	,
	70	,
	50	,
	48	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_awgn_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	65	,
	42	,
	30	,
	37	,
	37	,
	51
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	62	,
	64	,
	48	,
	46	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_awgn_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	71	,
	45	,
	31	,
	38	,
	39	,
	54
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	60	,
	60	,
	48	,
	47	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_awgn_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	77	,
	47	,
	31	,
	39	,
	41	,
	57
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	60	,
	60	,
	48	,
	48	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_awgn_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	70	,
	45	,
	32	,
	37	,
	43	,
	60
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	50	,
	42	,
	44	,
	44	,
	28	,
	28
};

static const uint8_t BAST_bw_trk_b0_awgn_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	90	,
	44	,
	31	,
	35	,
	45	,
	63
};

static const uint8_t BAST_damp_trk_b0_awgn_piloton_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	50	,
	42	,
	40	,
	37	,
	28	,
	28
};

/* trk_a1_default_pilotoff */
static const uint8_t BAST_bw_trk_a1_default_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	67	,
	51	,
	45	,
	46	,
	53	,
	79
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	144	,
	128	,
	116	,
	96	,
	80	,
	64
};

static const uint8_t BAST_bw_trk_a1_default_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	64	,
	55	,
	56	,
	77	,
	95	,
	114
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	112	,
	80	,
	72	,
	64	,
	48	,
	40
};

static const uint8_t BAST_bw_trk_a1_default_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	154	,
	142	,
	141	,
	140	,
	138	,
	140
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	130	,
	118	,
	98	,
	84	,
	72	,
	64
};

static const uint8_t BAST_bw_trk_a1_default_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	198	,
	184	,
	177	,
	177	,
	176	,
	179
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	174	,
	136	,
	122	,
	108	,
	88	,
	72
};

static const uint8_t BAST_bw_trk_a1_default_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	151	,
	161	,
	165	,
	165	,
	162	,
	164
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	166	,
	120	,
	112	,
	100	,
	84	,
	72
};

static const uint8_t BAST_bw_trk_a1_default_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	107	,
	104	,
	100	,
	100	,
	100	,
	114
};

static const uint8_t BAST_damp_trk_a1_default_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	162	,
	142	,
	128	,
	112	,
	96	,
	80
};

/* trk_a1_euro_pilotoff */
static const uint8_t BAST_bw_trk_a1_euro_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	201	,
	182	,
	108	,
	107	,
	106	,
	106
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	144	,
	120	,
	80	,
	72	,
	68	,
	64
};

static const uint8_t BAST_bw_trk_a1_euro_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	181	,
	219	,
	132	,
	131	,
	129	,
	129
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	160	,
	96	,
	80	,
	68	,
	64	,
	56
};

static const uint8_t BAST_bw_trk_a1_euro_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	174	,
	109	,
	160	,
	159	,
	157	,
	157
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	128	,
	104	,
	116	,
	84	,
	72	,
	64
};

static const uint8_t BAST_bw_trk_a1_euro_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	230	,
	219	,
	255	,
	255	,
	252	,
	251
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	248	,
	120	,
	132	,
	116	,
	96	,
	80
};

static const uint8_t BAST_bw_trk_a1_euro_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	255	,
	250	,
	208	,
	206	,
	204	,
	204
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	224	,
	246	,
	150	,
	132	,
	104	,
	88
};

static const uint8_t BAST_bw_trk_a1_euro_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	248	,
	255	,
	225	,
	224	,
	221	,
	221
};

static const uint8_t BAST_damp_trk_a1_euro_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	176	,
	184	,
	124	,
	92	,
	76	,
	64
};

/* trk_a1_awgn_pilotoff */
static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	85	,
	71	,
	43	,
	46	,
	53	,
	79
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	80	,
	76	,
	64	,
	67	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	99	,
	72	,
	46	,
	52	,
	56	,
	77
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	82	,
	77	,
	59	,
	58	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	55	,
	55	,
	32	,
	45	,
	59	,
	81
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	80	,
	40	,
	80	,
	59	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	113	,
	74	,
	45	,
	49	,
	61	,
	86
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_8PSK_3_4[BAST_PLC_NUM_SYMRATES] = {
	80	,
	72	,
	63	,
	61	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	116	,
	78	,
	44	,
	57	,
	64	,
	90
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_8PSK_5_6[BAST_PLC_NUM_SYMRATES] = {
	72	,
	72	,
	59	,
	64	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_a1_awgn_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	131	,
	76	,
	40	,
	55	,
	67	,
	94
};

static const uint8_t BAST_damp_trk_a1_awgn_pilotoff_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	72	,
	72	,
	56	,
	51	,
	40	,
	40
};

static const uint8_t BAST_bw_trk_turbo_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	145	,
	179	,
	133	,
	100	,
	100	,
	114
};

static const uint8_t BAST_damp_trk_turbo_QPSK_low[BAST_PLC_NUM_SYMRATES] = {
	72	,
	64	,
	38	,
	36	,
	36	,
	36
};

static const uint8_t BAST_bw_trk_turbo_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	145	,
	161	,
	147	,
	150	,
	130	,
	114
};

static const uint8_t BAST_damp_trk_turbo_QPSK_high[BAST_PLC_NUM_SYMRATES] = {
	64	,
	58	,
	48	,
	40	,
	34	,
	34
};

static const uint8_t BAST_bw_trk_turbo_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	253	,
	225	,
	160	,
	157	,
	210	,
	214
};

static const uint8_t BAST_damp_trk_turbo_8PSK_low[BAST_PLC_NUM_SYMRATES] = {
	70	,
	64	,
	34	,
	28	,
	25	,
	20
};

static const uint8_t BAST_bw_trk_turbo_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	250	,
	250	,
	240	,
	236	,
	220	,
	214
};

static const uint8_t BAST_damp_trk_turbo_8PSK_high[BAST_PLC_NUM_SYMRATES] = {
	48	,
	40	,
	40	,
	32	,
	24	,
	19
};

static const unsigned long BAST_plc_symbol_rate[BAST_PLC_NUM_SYMRATES] = {
	30000000	,
	15000000	,
	7500000	,
	3750000	,
	1875000	,
	1000000
};

static const uint16_t BAST_plc_trk_bw_scale[BAST_PLC_NUM_SYMRATES] = {
	298	,
	274	,
	231	,
	155	,
	105	,
	70
};

static const uint16_t BAST_plc_acq_bw_scale[BAST_PLC_NUM_SYMRATES] = {
	314	,
	325	,
	255	,
	167	,
	103	,
	64
};
#if 0
static const uint16_t BAST_plc_acq_bw_min[BAST_PLC_NUM_SYMRATES] = {
	21775	,
	15080	,
	8100	,
	6240	,
	4600	,
	4420
};

static const uint16_t BAST_plc_trk_bw_min[BAST_PLC_NUM_SYMRATES] = {
	5800	,
	2330	,
	5750	,
	4500	,
	3700	,
	3400
};
#endif
static const uint16_t BAST_plc_turbo_trk_bw_scale[BAST_PLC_NUM_SYMRATES] = {
	76	,
	56	,
	15	,
	14	,
	10	,
	7
};

/* end of generated data */


#define BAST_LDPC_INIT_LOCK_FILTER_TIME 2000
#define BAST_LDPC_LOCK_FILTER_TIME 10000
#define BAST_LDPC_LOCK_FILTER_TIME_MAX 80000
#define BAST_LDPC_LOCK_FILTER_INCREMENT 20000
/* #define BAST_DEBUG_PLC */

/* local functions */
BERR_Code BAST_g2_P_LdpcAcquire1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcAcquire2_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcAcquire3_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcAcquire4_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcAcquire5_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcAcquire7_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcGetModcod_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcModeConfig_isr(BAST_ChannelHandle h, uint32_t reg, const uint32_t *pVal);
void BAST_g2_P_LdpcSetQpsk_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetHd8psk_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetGoldCodeSeq_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetSftdly_isr(BAST_ChannelHandle h);
bool BAST_g2_P_LdpcIs8psk_isr(BAST_ChannelHandle h);
bool BAST_g2_P_LdpcIsPilotOn_isr(BAST_ChannelHandle h);
bool BAST_g2_P_LdpcIsPilotPll_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetSistep_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetHpctrl_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetBlen_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetHpmode_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetHpParams_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcConfig_isr(BAST_ChannelHandle h, uint32_t reg, const uint32_t *pVal);
void BAST_g2_P_LdpcGenerate8pskPdTable_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcGenerateQpskPdTable_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetPilotctl_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcEnableHpStateChange_isr(BAST_ChannelHandle h, bool bEnable);
void BAST_g2_P_LdpcSetConfig2_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetMpcfg1_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetOpll_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetPsl_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcGetModcod1_isr(BAST_ChannelHandle h);
static uint8_t BAST_g2_P_LdpcRmd_isr(uint32_t enc_msg);
void BAST_g2_P_ProcessHpState0_isr(BAST_ChannelHandle h);
void BAST_g2_P_ProcessHpState15_isr(BAST_ChannelHandle h);
void BAST_g2_P_ProcessHpState16_isr(BAST_ChannelHandle h);
void BAST_g2_P_ProcessHpState17_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcFailedAcquisition_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcGetTunerFs_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcHp17Timeout_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcEnableLockInterrupts_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcSetHp17Timeout_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcMonitor_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcTransferCarrierToTuner_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcTransferCarrierToTuner1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcMonitorNotLocked_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_LdpcSetupNotLockedMonitor_isr(BAST_ChannelHandle h);
void BAST_g2_P_LdpcLookupPlcBwDamp_isr(const uint8_t *pBwTable, const uint16_t *pBwScaleTable, const uint8_t *pDampTable, int32_t i, uint32_t *pBw, uint32_t *pDamp);
BERR_Code BAST_g2_P_LdpcAcquire1a_isr(BAST_ChannelHandle h);

static const uint32_t BAST_ldpc_eqmode[2] = {0x00000010, 0x0000003A};
static const uint32_t BAST_ldpc_f0b[2] = {0x10000000, 0x20000000};
static const uint32_t BAST_ldpc_plctl[2] = {0x059F2316, 0x059F230E};
static const uint32_t BAST_ldpc_phdiv2[2] = {0x18780000, 0x2EF80000};
static const uint32_t BAST_ldpc_ffcoef0[2] = {0x11111111, 0x1A1A1A19};
static const uint32_t BAST_ldpc_ffcoef1[2] = {0x1010100F, 0x18171513};
static const uint32_t BAST_ldpc_ffcoef2[2] = {0x0F0E0D0D, 0x110F0D0A};
static const uint32_t BAST_ldpc_ffcoef3[2] = {0x0C0B0A09, 0x07040000};
static const uint32_t BAST_ldpc_ffcoef4[2] = {0x08070604, 0x00000000};
static const uint32_t BAST_ldpc_ffcoef5[2] = {0x03010000, 0x00000000};

#ifndef BAST_EXCLUDE_TURBO
static const uint32_t BAST_turbo_eqmode[2] = {0x00000000, 0x0000000A};
static const uint32_t BAST_turbo_eqffe1[2] = {0x00000031, 0x00000029};
static const uint32_t BAST_turbo_eqblnd[2] = {0x00000044, 0x00000044};
static const uint32_t BAST_turbo_clpdctl[2] = {0x0000001A, 0x00000012};
static const uint32_t BAST_turbo_cloon[2] = {0x00000000, 0x00000000};
static const uint32_t BAST_turbo_vlctl1[2] = {0x00000005, 0x00000045};
static const uint32_t BAST_turbo_vlctl3[2] = {0x00000004, 0x00000005};
static const uint32_t BAST_turbo_hpctrl4[2] = {0x00000055, 0x00000057};
static const uint32_t BAST_turbo_qpsk[2] = {0x01000000, 0x01000000};
static const uint32_t BAST_turbo_plc[2] = {0x050F0410, 0x050F0410};
static const uint32_t BAST_turbo_cormsk[2] = {0x00000000, 0x00000008};
static const uint32_t BAST_turbo_trnlen[2] = {0x0000007F, 0x0000003F};
static const uint32_t BAST_turbo_blen[2] = {0x00288F00, 0x00284F00};
static const uint32_t BAST_turbo_hpbp[2] = {0x001E8B00, 0x001E4B00};
static const uint32_t BAST_turbo_ofnseq[2] = {0x00000000, 0x80000000};
static const uint32_t BAST_turbo_tzsy[2] = {0x0007070F, 0x0002070F};
#endif

static const uint32_t BAST_ldpc_plhdrcfg[] =
{
   0x00000024,  /* LDPC QPSK 1/2 */
   0x00000025,  /* LDPC QPSK 3/5 */
   0x00000026,  /* LDPC QPSK 2/3 */
   0x00000027,  /* LDPC QPSK 3/4 */
   0x00000028,  /* LDPC QPSK 4/5 */
   0x00000029,  /* LDPC QPSK 5/6 */
   0x0000002a,  /* LDPC QPSK 8/9 */
   0x0000002b,  /* LDPC QPSK 9/10 */
   0x0000002c,  /* LDPC 8PSK 3/5 */
   0x0000002d,  /* LDPC 8PSK 2/3 */
   0x0000002e,  /* LDPC 8PSK 3/4 */
   0x0000002f,  /* LDPC 8PSK 5/6 */
   0x00000030,  /* LDPC 8PSK 8/9 */
   0x00000031   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_config_1[] =
{
   0x00050103,  /* LDPC QPSK 1/2 */
   0x00070104,  /* LDPC QPSK 3/5 */
   0x00080105,  /* LDPC QPSK 2/3 */
   0x00080106,  /* LDPC QPSK 3/4 */
   0x00080107,  /* LDPC QPSK 4/5 */
   0x00080108,  /* LDPC QPSK 5/6 */
   0x00080109,  /* LDPC QPSK 8/9 */
   0x0008010a,  /* LDPC QPSK 9/10 */
   0x00080204,  /* LDPC 8PSK 3/5 */
   0x00080205,  /* LDPC 8PSK 2/3 */
   0x00080206,  /* LDPC 8PSK 3/4 */
   0x00080208,  /* LDPC 8PSK 5/6 */
   0x00090209,  /* LDPC 8PSK 8/9 */
   0x0009020a   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_bch_deccfg0[] =
{
   0x7e907dd0,  /* LDPC QPSK 1/2 */
   0x97e09720,  /* LDPC QPSK 3/5 */
   0xa8c0a820,  /* LDPC QPSK 2/3 */
   0xbdd8bd18,  /* LDPC QPSK 3/4 */
   0xca80c9c0,  /* LDPC QPSK 4/5 */
   0xd2f0d250,  /* LDPC QPSK 5/6 */
   0xe100e080,  /* LDPC QPSK 8/9 */
   0xe3d0e350,  /* LDPC QPSK 9/10 */
   0x97e09720,  /* LDPC 8PSK 3/5 */
   0xa8c0a820,  /* LDPC 8PSK 2/3 */
   0xbdd8bd18,  /* LDPC 8PSK 3/4 */
   0xd2f0d250,  /* LDPC 8PSK 5/6 */
   0xe100e080,  /* LDPC 8PSK 8/9 */
   0xe3d0e350   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_bch_deccfg1[] =
{
   0x0C5A0001,  /* LDPC QPSK 1/2 */
   0x0C6C0001,  /* LDPC QPSK 3/5 */
   0x0A780001,  /* LDPC QPSK 2/3 */
   0x0C870001,  /* LDPC QPSK 3/4 */
   0x0C900001,  /* LDPC QPSK 4/5 */
   0x0A960001,  /* LDPC QPSK 5/6 */
   0x08A00001,  /* LDPC QPSK 8/9 */
   0x08A20001,  /* LDPC QPSK 9/10 */
   0x0C6C0001,  /* LDPC 8PSK 3/5 */
   0x0A780001,  /* LDPC 8PSK 2/3 */
   0x0C870001,  /* LDPC 8PSK 3/4 */
   0x0A960001,  /* LDPC 8PSK 5/6 */
   0x08A00001,  /* LDPC 8PSK 8/9 */
   0x08A20001   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_vlci[] =
{
   0x2c4d0000,  /* LDPC QPSK 1/2 */
   0x391d0000,  /* LDPC QPSK 3/5 */
   0x3fd10000,  /* LDPC QPSK 2/3 */
   0x3cb40000,  /* LDPC QPSK 3/4 */
   0x3ab50000,  /* LDPC QPSK 4/5 */
   0x395f0000,  /* LDPC QPSK 5/6 */
   0x371f0000,  /* LDPC QPSK 8/9 */
   0x36b60000,  /* LDPC QPSK 9/10 */
   0x27f80000,  /* LDPC 8PSK 3/5 */
   0x267c0000,  /* LDPC 8PSK 2/3 */
   0x2f800000,  /* LDPC 8PSK 3/4    */
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
   0x2d000000,  /* LDPC 8PSK 5/6 */
#else
   0x32e80000,  /* LDPC 8PSK 5/6 */
#endif
   0x26e80000,  /* LDPC 8PSK 8/9 */
   0x26b80000   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_snrht[] =
{
   0x03c03235,  /* LDPC QPSK 1/2 */
   0x02e95977,  /* LDPC QPSK 3/5 */
   0x026bf466,  /* LDPC QPSK 2/3 */
   0x01ec7290,  /* LDPC QPSK 3/4 */
   0x01a3240c,  /* LDPC QPSK 4/5 */
   0x01758f45,  /* LDPC QPSK 5/6 */
   0x0128ba9e,  /* LDPC QPSK 8/9 */
   0x011b5fbc,  /* LDPC QPSK 9/10 */
   0x0299deea,  /* LDPC 8PSK 3/5 */
   0x0210eb81,  /* LDPC 8PSK 2/3 */
   0x01881801,  /* LDPC 8PSK 3/4 */
   0x011594c5,  /* LDPC 8PSK 5/6 */
   0x00c916fa,  /* LDPC 8PSK 8/9 */
   0x00bbaafa   /* LDPC 8PSK 9/10 */
};

static const uint32_t BAST_ldpc_snrlt[] =
{
   0x2581f613,  /* LDPC QPSK 1/2 */
   0x2581f613,  /* LDPC QPSK 3/5 */
   0x18378c00,  /* LDPC QPSK 2/3 */
   0x133c79a2,  /* LDPC QPSK 3/4 */
   0x105f6879,  /* LDPC QPSK 4/5 */
   0x0e9798ae,  /* LDPC QPSK 5/6 */
   0x0b974a29,  /* LDPC QPSK 8/9 */
   0x0b11bd5a,  /* LDPC QPSK 9/10 */
   0x1a02b525,  /* LDPC 8PSK 3/5 */
   0x14a9330f,  /* LDPC 8PSK 2/3 */
   0x0f50f00e,  /* LDPC 8PSK 3/4 */
   0x0ad7cfb3,  /* LDPC 8PSK 5/6 */
   0x07dae5c5,  /* LDPC 8PSK 8/9 */
   0x0754adc5   /* LDPC 8PSK 9/10 */
};


static const uint32_t BAST_turbo_titr[] =
{
   0x007F2824,  /* turbo qpsk 1/2 */
   0x00D4C828,  /* turbo qpsk 2/3 */
   0x00BF4830,  /* turbo qpsk 3/4 */
   0x00D4C836,  /* turbo qpsk 5/6 */
   0x00DF483A,  /* turbo qpsk 7/8 */
   0x00FF8024,  /* turbo 8psk 2/3 */
   0x01118028,  /* turbo 8psk 3/4 */
   0x01190030,  /* turbo 8psk 4/5 */
   0x01328036,  /* turbo 8psk 5/6 */
   0x013F803A,  /* turbo 8psk 8/9 */
};

#define abs(x) ((x)<0?-(x):(x))

/******************************************************************************
 BAST_g2_P_LdpcAcquire_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   hChn->bForceReacq = false;

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_scan)
   {
      hChn->ldpcScanState = BAST_LDPC_SCAN_ON;
      hChn->actualMode = BAST_Mode_eUnknown;
   }
   else
   {
      hChn->ldpcScanState = BAST_LDPC_SCAN_OFF;
      hChn->actualMode = hChn->acqParams.mode;
   }

#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
   if (((hChn->ldpcCtl & BAST_G2_LDPC_CTL_DISABLE_AUTO_PILOT_PLL) == 0) &&
       ((hChn->ldpcScanState & BAST_LDPC_SCAN_ON) == 0))
   {
      if ((hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2) ||
          (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_3_5) ||
          ((hChn->actualMode >= BAST_Mode_eLdpc_8psk_3_5) && (hChn->actualMode <= BAST_Mode_eLdpc_8psk_5_6)))
      {
         /* turn off pilot pll */
         hChn->acqParams.acq_ctl &= ~BAST_ACQSETTINGS_LDPC_PILOT_PLL;
      }
      else
      {
         /* turn on pilot pll */
         hChn->acqParams.acq_ctl |= BAST_ACQSETTINGS_LDPC_PILOT_PLL;
      }
   }
#else
   /* disable pilot pll for 7325/7335 B0 */
   hChn->acqParams.acq_ctl &= ~BAST_ACQSETTINGS_LDPC_PILOT_PLL;
#endif

   if (hChn->firstTimeLock == 0)
   {
      /* BDBG_MSG(("initializing lockFilterTime")); */
      hChn->lockFilterTime = BAST_LDPC_INIT_LOCK_FILTER_TIME;
      if (hChn->acqParams.symbolRate < 10000000)
      {
         /* scale lockFilterTime for symbol rate */
         BMTH_HILO_32TO64_Mul(hChn->lockFilterTime, 10000000, &P_hi, &P_lo);
         BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
         hChn->lockFilterTime = Q_lo;
      }
   }
   hChn->funct_state = 0;
   return BAST_g2_P_LdpcAcquire1_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire1_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire1_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, val2, filtctl, data0, data1, data2;

   static const uint32_t script_ldpc_0[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_FIRQSTS5, 0x00003C00), /* disable 3-stage agc interrupts */
      BAST_SCRIPT_WRITE(BCHP_SDS_OIFCTL2, 0x08),
      BAST_SCRIPT_WRITE(BCHP_SDS_PILOTCTL, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_OVRDSEL, 0xFFFF0000),

      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x50),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL2, 0x01),
      BAST_SCRIPT_AND_OR(BCHP_SDS_SPLL_PWRDN, 0xFFFFFFE7, 0x08), /* power up afec, power down tfec */
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x01),      /* reset HP state machine */
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL5, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_ABW, 0x0E0E0000),
      BAST_SCRIPT_AND_OR(BCHP_SDS_AGICTL, 0xFFFFFF28, 0x22),
      BAST_SCRIPT_AND_OR(BCHP_SDS_AGTCTL, 0xFFFFFF28, 0x02),
      BAST_SCRIPT_WRITE(BCHP_SDS_AII, 0x00000000),      /* set IF to min */
      BAST_SCRIPT_WRITE(BCHP_SDS_AIT, 0xFFFFFFF0),      /* set RF to max */
      BAST_SCRIPT_WRITE(BCHP_SDS_NTCH_CTRL, 0x00000F30), /* disable any notch filter that was on from the previous acquisition */
      /* BAST_SCRIPT_WRITE(BCHP_SDS_CGPDWN, 0x00), */

      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL1, 0x00),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL5, 0x00),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL2, 0x00),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL6, 0x00),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL4, 0x02),
      BAST_SCRIPT_WRITE(BCHP_SDS_ABW, 0x0E0E0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGI, 0x00010000),
      BAST_SCRIPT_WRITE(BCHP_SDS_AGT, 0xFFFF0000),
      BAST_SCRIPT_AND(BCHP_SDS_AGICTL, 0x28),
      BAST_SCRIPT_AND(BCHP_SDS_AGTCTL, 0x28),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_1[] =
   {
      BAST_SCRIPT_WRITE(BCHP_AFEC_LDPC_CONFIG_0, 0x02000002),   /* reset LDPC FEC */
      BAST_SCRIPT_WRITE(BCHP_AFEC_LDPC_CONFIG_0, 0x00000002),
      BAST_SCRIPT_WRITE(BCHP_SDS_IFMAX, 0x00),      /* software workaround to reset HP */
      BAST_SCRIPT_WRITE(BCHP_SDS_PSMAX, 0x00ff0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_BLEN, 0x00010000),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x03),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_SNRCTL, 0x03),     /* more average, ckp */
      BAST_SCRIPT_WRITE(BCHP_SDS_ADC, 0x7e7e0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_DSTGCTL, 0x02),
      BAST_SCRIPT_WRITE(BCHP_SDS_ADCPCTL, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_IQPHS, 0x10),
      BAST_SCRIPT_WRITE(BCHP_SDS_IQAMP, 0x20),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL15, 0xFF), /* open up DCO bw in case of re-acq with no tune */
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL16, 0xFF), /* make sure it doesn't interact with high AGC bw */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_3[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL7, 0x24), /* always acquire with tuner not in LO tracking mode*/
      BAST_SCRIPT_WRITE(BCHP_SDS_MIXCTL, 0x03),    /* front mixer control */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLCTL, 0x0F),     /* carrier loop is controlled by HP */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLOON, 0x88),
      BAST_SCRIPT_WRITE(BCHP_SDS_CLQCFD, 0x00),    /* ??? pilot/non-pilot, QPSK/8PSK, full-full/soft decision PD */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLMISC, 0x07),    /* ??? these setting are good for pilot mode only, need to */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLMISC2, 0x60),   /* ??? add programmability for other modes */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLSTS, 0x00),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_4[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLTD, 0x20000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLI, 0x00000000),   /* reset front loop integrator */
      BAST_SCRIPT_WRITE(BCHP_SDS_FLPA, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_PLTD, 0x20000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_PLI, 0x00000000),   /* reset back loop integrator */
      BAST_SCRIPT_WRITE(BCHP_SDS_PLPA, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_BRI, 0x00000000),   /* reset timing loop integrator */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLFFCTL, 0x02),
#if 0
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC, 0x20000100),  /* ??? set up loop bandwidth for front carrier loop */
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC, 0x40000200),  /* ??? this is not needed for pilot mode */
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC1, 0x01000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC1, 0x01000000),
#endif
      BAST_SCRIPT_WRITE(BCHP_SDS_BLPCTL1, 0x07),          /* freeze baud loop */
      BAST_SCRIPT_WRITE(BCHP_SDS_BLPCTL2, 0x01),          /* was 0x03 */
      BAST_SCRIPT_WRITE(BCHP_SDS_BRLC, 0x00099C10),
      BAST_SCRIPT_WRITE(BCHP_SDS_BRIC, 0x0000033C),
      BAST_SCRIPT_WRITE(BCHP_SDS_PLTD, 0x28000000),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_5[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_ABW, 0x0B0B0000),  /* setting up AGC loop, narrowed, ckp */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQFFE2, 0x06),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQMU, 0x03),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQMISC, 0x14),   /* set PLHEADER auto save */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQBLND, 0x04),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQCFAD, 0x4C),   /* initialization of EQ tap, org. 0x46 */
      BAST_SCRIPT_WRITE(BCHP_SDS_FERR, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_CLCTL, 0x0F),   /* carrier loop is controlled by HP */

#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
      BAST_SCRIPT_WRITE(BCHP_SDS_CLFBCTL, 0x12),
#endif

      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_6[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_CLPDCTL, 0xDA),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC, 0x20000100),  /* ??? set up loop bandwidth for front carrier loop */
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC, 0x40000200),  /* ??? this is not needed for pilot mode */
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC1, 0x01000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC1, 0x01000000),
      BAST_SCRIPT_EXIT
   };

#ifndef BAST_EXCLUDE_TURBO
   static const uint32_t script_ldpc_7[] = /* turbo only */
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC, 0x01000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC, 0x01000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLLC1, 0x01000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_FLIC1, 0x01000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_CLMISC2, 0x62),
      BAST_SCRIPT_WRITE(BCHP_SDS_F0B, 0x19000000),
      BAST_SCRIPT_EXIT
   };
#endif

   if ((hChn->bExternalTuner == false) && hChn->funct_state)
      BAST_g2_P_3StageAgc_isr(h, 0);

   switch (hChn->funct_state)
   {
      case 0:
#ifdef BAST_LDPC_DEBUG
         BDBG_MSG(("starting LDPC acquisition"));
#endif
         hChn->bFrfTransferred = false;
         hChn->frfStep = 0;
         BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_0);
         BAST_g2_P_SetAthr_isr(h);

         if (hChn->bExternalTuner == false)
         {
            BAST_g2_P_3StageAgc_isr(h, 0);
            BAST_g2_P_Enable3StageAgc_isr(h, true);
         }

         if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
            BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_1);

         BAST_g2_P_SetEqffe3_isr(h);
         BAST_g2_P_SetNyquistAlpha_isr(h);

         BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_2);
         hChn->funct_state = 1;
         return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, BAST_g2_P_LdpcAcquire1_isr);

      case 1:
         BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_AGICTL, 0xFFFFFF28);
         BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_AGTCTL, 0xFFFFFF28);
         hChn->funct_state = 2;
         return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, BAST_g2_P_LdpcAcquire1_isr);

      case 2:
         val = 0x05;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGFCTL, &val); /* reset AGF */
         val = 0x0A0A0000;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGF, &val);
         hChn->funct_state = 3;
         return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, BAST_g2_P_LdpcAcquire1_isr);

      case 3:
         val = 0x00;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_AGFCTL, &val); /* enable AGF */
         BAST_g2_P_LdpcGetTunerFs_isr(h);
         BAST_g2_P_TunerSetDigctl7_isr(h, 0x04);
         BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_3);
         hChn->funct_state = 4;
         return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 1000, BAST_g2_P_LdpcAcquire1_isr);

      case 4:
         BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_4);

         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_TUNER_TEST_MODE)
            return BAST_g2_P_TunerTestMode_isr(h);
         hChn->funct_state = 5;  /* drop down to state 5 */

      case 5:
         if (BAST_g2_P_LdpcIsPilotLoopless_isr(h))
         {
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
            val = 0xC3;
#else
            val = 0xC0;
#endif
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL6, &val);
            val = 0x39;
         }
         else
         {
            val = 0xC0;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL6, &val);
            val = 0x31;
         }
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE1, &val); /* enable AGF */
         BAST_g2_P_SetEqffe3_isr(h);
         BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_5);

         if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
         {
            /* LDPC */
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_EQMODE, BAST_ldpc_eqmode);

            BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_6);

            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_F0B, BAST_ldpc_f0b);
            val = 0x15;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL1, &val);
            val = 0x09;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL2, &val);
            val = 0x04;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL3, &val);
            BAST_g2_P_LdpcSetQpsk_isr(h);
            BAST_g2_P_LdpcSetHd8psk_isr(h);
            BAST_g2_P_LdpcSetGoldCodeSeq_isr(h);
            val = 0x00000100;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_XTAP1, &val);
            val = 0x00805000;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_XTAP2, &val);
            val = 0x14;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLDCTL, &val);
            BAST_g2_P_LdpcSetSftdly_isr(h);
         }
#ifndef BAST_EXCLUDE_TURBO
         else
         {
            /* TURBO */
            BAST_g2_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_SPLL_PWRDN, 0xFFFFFFE7, 0x10);   /* power down afec, power up tfec */
            val = 0;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TTUR, &val);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_EQMODE, BAST_turbo_eqmode);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_EQFFE1, BAST_turbo_eqffe1);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_EQBLND, BAST_turbo_eqblnd);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_CLPDCTL, BAST_turbo_clpdctl);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_CLOON, BAST_turbo_cloon);

            BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_7);

            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_VLCTL1, BAST_turbo_vlctl1);
            val = 0x07;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL2, &val);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_VLCTL3, BAST_turbo_vlctl3);
            val = 0x76303000;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VCOS, &val);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_QPSK, BAST_turbo_qpsk);
            val = 0x014E01D9;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK1, &val);
            val = 0x008B00C4;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK2, &val);
            val = 0x00;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLDCTL, &val);
         }
#endif

         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_BFOS, &val);
         val = val << 1;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRM, &val);
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRMR, &val);
         val2 = val >> 3;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FFNORM, &val2);

         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FILTCTL, &filtctl);
         filtctl &= 0x9F;
         if (filtctl == 0x9F)  /* no decimation filter */
         {
            data0 = 0; /* FNRM lshift */
            data1 = 1; /* FNRMR lshift */
            data2 = 1; /* FFNORM lshift */
         }
         else if (filtctl == 0x9E)   /* 1 decimation filter */
         {
            data0 = 1; /* FNRM lshift */
            data1 = 2; /* FNRMR lshift */
            data2 = 2; /* FFNORM lshift */
         }
         else if (filtctl == 0x9C)   /* 2 decimation filters */
         {
            data0 = 2; /* FNRM lshift */
            data1 = 3; /* FNRMR lshift */
            data2 = 3; /* FFNORM lshift */
         }
         else if (filtctl == 0x98)   /* 3 decimation filters */
         {
            data0 = 3; /* FNRM lshift */
            data1 = 4; /* FNRMR lshift */
            data2 = 4; /* FFNORM lshift */
         }
         else if (filtctl == 0x90)   /* 4 decimation filters */
         {
            data0 = 4; /* FNRM lshift */
            data1 = 5; /* FNRMR lshift */
            data2 = 5; /* FFNORM lshift */
         }
         else if (filtctl == 0x80)   /* 5 decimation filters */
         {
            data0 = 6; /* FNRM lshift */
            data1 = 5; /* FNRMR lshift */
            data2 = 5; /* FFNORM lshift */
         }
         else                           /* 6 decimation filters */
         {
            data0 = 7; /* FNRM lshift */
            data1 = 6; /* FNRMR lshift */
            data2 = 6; /* FFNORM lshift */
         }

         val = val2 << data0;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRM, &val);

         val = val2 << data1;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FNRMR, &val);

         val = val2 << data2;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FFNORM, &val);

         val = 0x1F;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CLCTL, &val);

         if (BAST_g2_P_TunerMixpllLostLock_isr(h))
         {
#ifdef BAST_LDPC_DEBUG
            BDBG_MSG(("tuner mix pll not locked"));
#endif
            hChn->tuneMixStatus |= BAST_TUNE_MIX_NEXT_RETRY;
            return BAST_g2_P_LdpcReacquire_isr(h);
         }

         hChn->funct_state = 0xFF;
         if (((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == 0) ||
             ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON) ||
             ((hChn->acqParams.mode == BAST_Mode_eTurbo_scan) && (hChn->turboScanState & BAST_TURBO_SCAN_HP_LOCKED)))
         {
            BAST_g2_P_3StageAgc_isr(h, 0);
            return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 2000, BAST_g2_P_LdpcAcquire1a_isr);
         }

         return BAST_g2_P_LdpcAcquire2_isr(h);

      default:
         BDBG_ERR(("invalid state"));
         BERR_TRACE(retCode = BAST_ERR_AP_IRQ);
         break;
   }

   return retCode;
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire1a_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire1a_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_Enable3StageAgc_isr(h, false); /* disable 3stage during dft freq estimate */
   BAST_g2_P_3StageAgc_isr(h, 0);
   return BAST_g2_P_DoDftFreqEstimate_isr(h, BAST_g2_P_LdpcAcquire2_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire2_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire2_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   static const uint32_t script_ldpc_6[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL7, 0x24),  /* always acquire with tuner not in LO tracking mode */
      BAST_SCRIPT_WRITE(BCHP_SDS_MIXCTL, 0x03),     /* front mixer control */
      BAST_SCRIPT_WRITE(BCHP_SDS_CLCTL, 0x0F),      /* carrier loop is controlled by HP */
      BAST_SCRIPT_WRITE(BCHP_SDS_BRLC, 0x004CE080),  /* open more after acquire time lock */
      BAST_SCRIPT_WRITE(BCHP_SDS_BRIC, 0x00033CF7),
      BAST_SCRIPT_EXIT
   };

   BAST_g2_P_Enable3StageAgc_isr(h, true); /* re-enable 3stage agc after dft freq estimate */
   BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_6);

   hChn->funct_state = 0;
   return BAST_g2_P_LdpcAcquire3_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire3_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire3_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   static const uint32_t script_ldpc_7[] =
   {
      /* BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL8, 0x0b), */
      BAST_SCRIPT_WRITE(BCHP_SDS_HPPDWN, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_WSMAX, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_ONFN, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_BLSCCP, 0xA2),  /* leave min # of times fec_phase is stable as 2 */
      BAST_SCRIPT_WRITE(BCHP_SDS_CORMSK, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_TRNLEN, 0x59),
      BAST_SCRIPT_WRITE(BCHP_SDS_DPTH, 0x00100000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TMTH, 0x00000800),
      BAST_SCRIPT_WRITE(BCHP_SDS_MGTH, 0x000f0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TNRM, 0x000000ff),
      BAST_SCRIPT_WRITE(BCHP_SDS_OFNSEQ, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_MISC2, 0x09010a03),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPBP, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_MSMAX, 0x02010000),  /* magnitude search, not used */
      BAST_SCRIPT_WRITE(BCHP_SDS_PVMAX, 0x02010000),  /* peak verify, not used */
      BAST_SCRIPT_WRITE(BCHP_SDS_SITHRSH, 0x0af01510), /* get back to this due to SpInv acq. under no noise */
      BAST_SCRIPT_EXIT
   };

#ifndef BAST_EXCLUDE_TURBO
   static const uint32_t script_turbo_1[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL5, 0x28),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL6, 0xC0),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL7, 0x48),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL8, 0x03),
      BAST_SCRIPT_WRITE(BCHP_SDS_TFAVG, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_WSMAX, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_IFMAX, 0x00),
      BAST_SCRIPT_WRITE(BCHP_SDS_FMAX, 0x01),
      BAST_SCRIPT_WRITE(BCHP_SDS_FMMAX, 0x04),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_turbo_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_DPTH, 0x00080000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TMTH, 0x00000300),
      BAST_SCRIPT_WRITE(BCHP_SDS_MGTH, 0x000F0000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TNRM, 0x000000FF),
#if 0
      BAST_SCRIPT_WRITE(BCHP_SDS_FNRM, 0x01200000),
      BAST_SCRIPT_WRITE(BCHP_SDS_FNRMR, 0x01200000),
#endif
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_turbo_3[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_MSMAX, 0x03020000),
      BAST_SCRIPT_WRITE(BCHP_SDS_PVMAX, 0x03020000),
      BAST_SCRIPT_WRITE(BCHP_SDS_PSMAX, 0x06020000),
      BAST_SCRIPT_WRITE(BCHP_SDS_TCMAX, 0x05030410),
      BAST_SCRIPT_WRITE(BCHP_SDS_TLMAX, 0x0501000A),
      BAST_SCRIPT_WRITE(BCHP_SDS_RLMAX, 0x20100000),
      BAST_SCRIPT_WRITE(BCHP_SDS_SFTDLY, 0x00010000),
      BAST_SCRIPT_WRITE(BCHP_SDS_PILOTCTL, 0),
      BAST_SCRIPT_WRITE(BCHP_SDS_PLHDRCFG, 0x00000001),
      BAST_SCRIPT_WRITE(BCHP_SDS_OVRDSEL, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_HPCTRL1, 0x03),         /* enable HP */
      BAST_SCRIPT_WRITE(BCHP_SDS_OPLL, 0x0000000F),
      BAST_SCRIPT_WRITE(BCHP_SDS_OPLL2, 0x00000001),
      BAST_SCRIPT_EXIT
   };
#endif

   while (hChn->funct_state != 0xFF)
   {
      switch (hChn->funct_state)
      {
         case 0:
            if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
            {
               /* LDPC */
               val = 0x00;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IFMAX, &val);
               val = 0x00FF0000;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PSMAX, &val);
               val = 0x00010000;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BLEN, &val);
               val = 0x03;

               BAST_g2_P_LdpcSetPlc_isr(h, true);

               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
               hChn->funct_state = 1;
               return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 200, BAST_g2_P_LdpcAcquire3_isr);
            }
#ifndef BAST_EXCLUDE_TURBO
            else
            {
               /* TURBO */
               BAST_g2_P_LdpcSetPlc_isr(h, true);

               val = 0x08;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPMODE, &val);
               val = 0x41;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
               val = 0x00;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL2, &val);
               val = 0x82;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL3, &val);
               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_HPCTRL4, BAST_turbo_hpctrl4);

               BAST_g2_P_ProcessScript_isrsafe(h, script_turbo_1);

               BAST_g2_P_TurboSetOnfn_isr(h);

               val = 0x40;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BLSCCP, &val);
               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_CORMSK, BAST_turbo_cormsk);
               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_TRNLEN, BAST_turbo_trnlen);

               BAST_g2_P_ProcessScript_isrsafe(h, script_turbo_2);

               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_BLEN, BAST_turbo_blen);
               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_OFNSEQ, BAST_turbo_ofnseq);
               val = 0x00040004;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC1, &val);
               val = 0x09010a03;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC2, &val);
               BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_HPBP, BAST_turbo_hpbp);

               BAST_g2_P_ProcessScript_isrsafe(h, script_turbo_3);

               hChn->funct_state = 2;
            }
#endif
            break;

         case 1:
            val = 0x06020000;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SIMAX, &val);
            val = 0x00010001;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_MISC1, &val);

            BAST_g2_P_LdpcSetHpParams_isr(h);

            val = 0x01;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);

            BAST_g2_P_LdpcSetHpmode_isr(h);
            BAST_g2_P_LdpcSetBlen_isr(h);

            val = 0x00;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL2, &val);

            val = 0x82;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL3, &val);

            BAST_g2_P_LdpcSetHpctrl_isr(h);
            BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_7);
            BAST_g2_P_LdpcSetSistep_isr(h);

            val = 0x2ef82e20;  /* phase estimate divider */
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PHDIV1, &val);

            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_PHDIV2, BAST_ldpc_phdiv2);

            val = 0x020F0000; /* fine frequency set up (pilot mode only) */
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FFCNT, &val);

            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF0, BAST_ldpc_ffcoef0);  /* needed for pilot mode only */
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF1, BAST_ldpc_ffcoef1);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF2, BAST_ldpc_ffcoef2);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF3, BAST_ldpc_ffcoef3);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF4, BAST_ldpc_ffcoef4);
            BAST_g2_P_LdpcModeConfig_isr(h, BCHP_SDS_FFCOEF5, BAST_ldpc_ffcoef5);

            val = 0;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FFCOEF6, &val);

            BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_PLHDRCFG, BAST_ldpc_plhdrcfg); /* physical layer scrambler set up */

            BAST_g2_P_LdpcSetPilotctl_isr(h);

            val = 0x03;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);  /* enable HP */

            hChn->funct_state = 2;
            break;

         case 2:
            if (hChn->bFrfTransferred)
            {
               val = 0;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FMAX, &val);
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FMMAX, &val);
            }

            BAST_g2_P_TunerSetDigctl7_isr(h, 0x04);

            if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))  /* LDPC mode */
            {
              if ((BAST_g2_P_LdpcIsPilotOn_isr(h) == false) || BAST_g2_P_LdpcIsPilotPll_isr(h))
              {
                 if (BAST_g2_P_LdpcIs8psk_isr(h))
                    BAST_g2_P_LdpcGenerate8pskPdTable_isr(h);
                 else
                    BAST_g2_P_LdpcGenerateQpskPdTable_isr(h);
              }

              if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
              {
                 /* clear and enable HP reacquire interrupt for modcod */
                 BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_IRQ, 0x02000200);
              }
              else
                 goto set_hp17_timeout;
            }
            else
            {
              /* set timeout for reaching HP state 17 */
              set_hp17_timeout:
              BAST_g2_P_LdpcSetHp17Timeout_isr(h);
            }

            hChn->funct_state = 0xFF;
            break;
      }
   }

   hChn->prev_state = 0xFF;
   hChn->count1 = 1;
   hChn->count2 = 0; /* count2=prev hp states visited */

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("enabling HP state change interrupt"));
#endif

   BAST_g2_P_LdpcEnableHpStateChange_isr(h, true);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_LdpcSetHp17Timeout_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetHp17Timeout_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, timeout;

   if (hChn->actualMode <= BAST_Mode_eLdpc_Qpsk_2_3)
      timeout = 200000;
   else if (hChn->actualMode <= BAST_Mode_eLdpc_Qpsk_9_10)
      timeout = 180000;
   else if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_3_4)
      timeout = 160000;
   else if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_9_10)
      timeout = 140000;
   else if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      if ((hChn->acqParams.mode == BAST_Mode_eTurbo_scan) && (hChn->acqCount < 5))
         timeout = 80000;
      else
         timeout = 200000;
   }
   else
      timeout = 500000;

   BMTH_HILO_32TO64_Mul(timeout, 20000000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &P_hi, &timeout);
   BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, timeout, BAST_g2_P_LdpcHp17Timeout_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcModeConfig_isr()
******************************************************************************/
void BAST_g2_P_LdpcModeConfig_isr(BAST_ChannelHandle h, uint32_t reg, const uint32_t *pVal)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, idx = 0;

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      /* ldpc */
      if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
      {
         if ((hChn->ldpcScanState & BAST_LDPC_SCAN_QPSK) == 0)
            goto mode_8psk;
      }
      else if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
      {
         mode_8psk:
         idx = 1;
      }
   }
   else if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      idx = 1;

   val = pVal[idx];
   BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcGetTunerFs_isr()
******************************************************************************/
void BAST_g2_P_LdpcGetTunerFs_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TUNER_ANACTL14, &val);
   BMTH_HILO_32TO64_Mul(hChn->tunerFddfs, ((val & 1) ? 16 : 12), &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 128, &Q_hi, &(hChn->tunerFs));
}


/******************************************************************************
 BAST_g2_P_LdpcSetQpsk_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetQpsk_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

   static const uint32_t sds_qpsk_table[] =
   {
      0x016A0000, /* LDPC 8PSK 3/5 */
      0x016A0000, /* LDPC 8PSK 2/3 */
      0x01200000, /* LDPC 8PSK 3/4 */
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
      0x016A0000, /* LDPC 8PSK 5/6 */
#else
      0x01400000, /* LDPC 8PSK 5/6 */
#endif
      0x016A0000, /* LDPC 8PSK 8/9 */
      0x016A0000  /* LDPC 8PSK 9/10 */
   };

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_QPSK)
         goto ldpc_set_qpsk_1;
      else
      {
         /* assume 8PSK 3/5 in scan mode */
         i = 0;
         goto ldpc_set_qpsk_0;
      }
   }
   else if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
   {
      /* LDPC 8PSK */
      i = hChn->actualMode - BAST_Mode_eLdpc_8psk_3_5;

      ldpc_set_qpsk_0:
      val = sds_qpsk_table[i];
   }
   else
   {
      /* LDPC QPSK */
      ldpc_set_qpsk_1:
      val = 0x01000000;
   }
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_QPSK, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetHd8psk_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetHd8psk_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, val1, qpsk;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_QPSK)
         goto ldpc_set_hd8psk_1;
      else
         goto ldpc_set_hd8psk_0;
   }
   else if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
   {
      /* LDPC 8PSK */
      ldpc_set_hd8psk_0:
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_QPSK, &qpsk);
      qpsk = (qpsk >> 16) & 0x03FF;
      val = (qpsk * 473) / 362;  /* 0x16A=362 */
      val &= 0x3FFF;
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HD8PSK1, &val1);
      val1 &= 0x0000FFFF;
      val1 |= (val << 16);
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK1, &val1);

      val = (qpsk * 196) / 362;  /* 0xC4=196 */
      val &= 0x3FFF;
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HD8PSK2, &val1);
      val1 &= 0x0000FFFF;
      val1 |= (val << 16);
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK2, &val1);
   }
   else
   {
      /* LDPC QPSK */
      ldpc_set_hd8psk_1:
      val = 0x014E01D9;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK1, &val);
      val = 0x008B00C4;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HD8PSK2, &val);
   }
}


/******************************************************************************
 BAST_g2_P_LdpcSetSftdly_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetSftdly_isr(BAST_ChannelHandle h)
{
   uint32_t val, vlctl3;

   if (BAST_g2_P_LdpcIs8psk_isr(h))
      val = 0x00001635; /* 8PSK */
   else
      val = 0x0000161A; /* QPSK */

   if ((BAST_g2_P_LdpcIsPilotOn_isr(h) == false) || (BAST_g2_P_LdpcIsPilotPll_isr(h)))
   {
      if (BAST_g2_P_LdpcIs8psk_isr(h))
      {
         vlctl3 = 0x06;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL3, &vlctl3);
      }
      val |= 0x00020000;
   }
#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
   else   /* forward/backward loop */
   {
      /* pilot only */
      if (BAST_g2_P_LdpcIs8psk_isr(h))
      {
         vlctl3 = 0x06;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCTL3, &vlctl3);
      }
   }
#endif

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SFTDLY, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetGoldCodeSeq_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetGoldCodeSeq_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = (hChn->ldpcScramblingSeq[0] << 24) | (hChn->ldpcScramblingSeq[1] << 16) |
         (hChn->ldpcScramblingSeq[2] << 8) | hChn->ldpcScramblingSeq[3];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_XSEED, &val);
   val = (hChn->ldpcScramblingSeq[4] << 24) | (hChn->ldpcScramblingSeq[5] << 16) |
         (hChn->ldpcScramblingSeq[6] << 8) | hChn->ldpcScramblingSeq[7];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLHDRSCR1, &val);
   val = (hChn->ldpcScramblingSeq[8] << 24) | (hChn->ldpcScramblingSeq[9] << 16) |
         (hChn->ldpcScramblingSeq[10] << 8) | hChn->ldpcScramblingSeq[11];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLHDRSCR2, &val);
   val = (hChn->ldpcScramblingSeq[12] << 24) | (hChn->ldpcScramblingSeq[13] << 16) |
         (hChn->ldpcScramblingSeq[14] << 8) | hChn->ldpcScramblingSeq[15];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLHDRSCR3, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcIs8psk_isr()
******************************************************************************/
bool BAST_g2_P_LdpcIs8psk_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      /* ldpc scan in progress */
      if ((hChn->ldpcScanState & BAST_LDPC_SCAN_QPSK) == 0)
         return true;
   }
   else if (BAST_MODE_IS_LDPC(hChn->actualMode))
   {
      if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
         return true;
   }
   else if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      return true;
   return false;
}


/******************************************************************************
 BAST_g2_P_LdpcIsPilotOn_isr()
******************************************************************************/
bool BAST_g2_P_LdpcIsPilotOn_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->ldpcScanState & BAST_LDPC_SCAN_MASK)
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_PILOT)
         return true;
   }
   else if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT)
      return true;
   return false;
}


/******************************************************************************
 BAST_g2_P_LdpcIsPilotLoopless_isr()
******************************************************************************/
bool BAST_g2_P_LdpcIsPilotLoopless_isr(BAST_ChannelHandle h)
{
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (BAST_g2_P_LdpcIsPilotOn_isr(h))
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_MASK)
      {
         if (hChn->ldpcScanState & BAST_LDPC_SCAN_FOUND)
         {
            if ((hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2) ||
                (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_3_5) ||
                ((hChn->actualMode >= BAST_Mode_eLdpc_8psk_3_5) && (hChn->actualMode < BAST_Mode_eLdpc_8psk_5_6)))
            {
               return true;
            }
         }
         else
            return false; /* assume pilot loop while scanning */
      }
      else if ((hChn->acqParams.acq_ctl & (BAST_ACQSETTINGS_LDPC_PILOT | BAST_ACQSETTINGS_LDPC_PILOT_PLL)) == BAST_ACQSETTINGS_LDPC_PILOT)
         return true;
   }
   return false;
#else
   return BAST_g2_P_LdpcIsPilotOn_isr(h);
#endif
}


/******************************************************************************
 BAST_g2_P_LdpcIsPilotPll_isr()
******************************************************************************/
bool BAST_g2_P_LdpcIsPilotPll_isr(BAST_ChannelHandle h)
{
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (BAST_g2_P_LdpcIsPilotOn_isr(h))
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_MASK)
      {
         if (hChn->ldpcScanState & BAST_LDPC_SCAN_FOUND)
         {
            if (!((hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2) ||
                (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_3_5) ||
                ((hChn->actualMode >= BAST_Mode_eLdpc_8psk_3_5) && (hChn->actualMode < BAST_Mode_eLdpc_8psk_5_6))))
               return true;
         }
         else
            return true; /* assume pilot loop while scanning */
      }
      else if ((hChn->acqParams.acq_ctl & (BAST_ACQSETTINGS_LDPC_PILOT | BAST_ACQSETTINGS_LDPC_PILOT_PLL)) ==
               (BAST_ACQSETTINGS_LDPC_PILOT | BAST_ACQSETTINGS_LDPC_PILOT_PLL))
         return true;
   }
   return false;
#else
   BSTD_UNUSED(h);
   return false;
#endif
}


/******************************************************************************
 BAST_g2_P_LdpcSetPilotctl_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetPilotctl_isr(BAST_ChannelHandle h)
{
   uint32_t data1, data2, val;

   if (BAST_g2_P_LdpcIs8psk_isr(h))
   {
      if (BAST_g2_P_LdpcIsPilotOn_isr(h))
      {
         /* 8psk with pilot */
         data1 = 0x0e;
         if (BAST_g2_P_LdpcIsPilotLoopless_isr(h))
         {
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
            val = 0x0D;
            data2 = 0x1b;
#else
            val = 0x0F;
            data2 = 0x13;
#endif
         }
         else
         {
            val = 0x2D;
            data2 = 0x00;
         }
      }
      else
      {
         /* 8psk with no pilot */
         BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_PLHDRCFG, 0xFFFFFFDF);

         val = 0x0C;
         data1 = 0x00;
         data2 = 0x00;
      }
   }
   else
   {
      if (BAST_g2_P_LdpcIsPilotOn_isr(h))
      {
         /* qpsk with pilot */
         data1 = 0x16;
         if (BAST_g2_P_LdpcIsPilotLoopless_isr(h))
         {
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
            val = 0x1D;
            data2 = 0x1B;
#else
            val = 0x1F;
            data2 = 0x13;
#endif
         }
         else
         {
            val = 0x3D;
            data2 = 0x00;
         }
      }
      else
      {
         /* qpsk with no pilot */
         BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_PLHDRCFG, 0xFFFFFFDF);

         val = 0x1C;
         data1 = 0x00;
         data2 = 0x00;
      }
   }
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PILOTCTL, &val);   /* write pilot_ctl */

   val = 0x059f2300 | (data1 & 0xFF);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLCTL, &val);

   val = 0x0dc00000 | (data2 << 16);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OVRDSEL, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetSistep_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetSistep_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, 1073741824, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   Q_lo &= 0xFFFFFF00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SISTEP, &Q_lo);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_DELIF, &Q_lo);
}


/******************************************************************************
 BAST_g2_P_LdpcSetHpctrl_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetHpctrl_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, hpctrl5, hpctrl4;

   if (BAST_g2_P_LdpcIsPilotLoopless_isr(h))
   {
      val = 0x4A;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL7, &val);

      /* pilot no pll */
#if ((BCHP_CHIP==7325) || (BCHP_CHIP==7335)) && (BCHP_VER < BCHP_VER_B0)
      hpctrl4 = 0x51;
#else
      if (BAST_g2_P_LdpcIs8psk_isr(h))
         hpctrl4 = 0x5D;
      else
         hpctrl4 = 0x51;
#endif

      hpctrl5 = 0x28;
   }
   else
   {
      /* no pilot or pilot pll */
      if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
         val = 0xC8;
      else
         val = 0xC0;

      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL7, &val);

      if (BAST_g2_P_LdpcIs8psk_isr(h))
      {
         hpctrl4 = 0x1D;
         hpctrl5 = 0x29;
      }
      else
      {
         hpctrl5 = 0x28;
         hpctrl4 = 0x15;
      }
   }
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL4, &hpctrl4);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPCTRL5, &hpctrl5);
}


/******************************************************************************
 BAST_g2_P_LdpcSetBlen_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetBlen_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   bool b8psk = BAST_g2_P_LdpcIs8psk_isr(h);
   bool bPilot = BAST_g2_P_LdpcIsPilotOn_isr(h);

   if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_SHORT_FRAME)
   {
      /* short frames */
      if (b8psk)
      {
         /* short frame 8psk */
         if (bPilot)
            val = 5598;
         else
            val = 5490;
      }
      else
      {
         /* short frame qpsk */
         if (bPilot)
            val = 8370;
         else
            val = 8190;
      }
   }
   else if (b8psk)
   {
      /* normal frame 8psk */
      if (bPilot)
         val = 22194;
      else
         val = 21690;
   }
   else
   {
      /* normal frame qpsk */
      if (bPilot)
         val = 33282;
      else
         val = 32490;
   }

   val--;
   val = val << 8;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BLEN, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetHpmode_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetHpmode_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      /* enable MODCOD search */
      val = 0x79;
   }
   else
      val = 0x09;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HPMODE, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetHpParams_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetHpParams_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   BAST_Mode mode;

   val = 0x00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IFMAX, &val);

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
      mode = BAST_Mode_eLdpc_Qpsk_1_2; /* in scan mode */
   else
      mode = hChn->actualMode;

   if (mode >= BAST_Mode_eLdpc_Qpsk_2_3)   /* high rate QPSK and 8PSK */
      val = 0x08040000;
   else
     val = 0x06020000;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PSMAX, &val);

   val = 0x20100000;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_RLMAX, &val);
   val = 0x05030410;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_TCMAX, &val);
   val = 0x0503000a;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_TLMAX, &val);

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
      val = 0x03;
   else
      val = 0x00;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_TFAVG, &val);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGCTRL, &val);
   if ((val & 0x02) && ((BAST_MODE_IS_LDPC_8PSK(mode)) || (mode <= BAST_Mode_eLdpc_Qpsk_3_5)))
      val = 0x0E; /* undersample mode and (8psk or qpsk_1/2 or qpsk_3/5) */
   else
      val = 0x01;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FMAX, &val);

   val = 0x04;
#if 0
   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      if (BAST_g2_P_LdpcIsPilotOn_isr(h) == false)
         val = 0x10;
   }
#endif
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FMMAX, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcGenerateQpskPdTable_isr()
******************************************************************************/
void BAST_g2_P_LdpcGenerateQpskPdTable_isr(BAST_ChannelHandle h)
{
   uint32_t val, i_idx, q_idx, idx;

   static const uint8_t BAST_pd_qpsk_i[] =
   {
      0x00,
      0x26,
      0x4A,
      0x64,
      0x78,
      0x86,
      0x90,
      0x94,
      0x98,
      0x9A,
      0x9C,
      0x9C,
      0x9E
   };

   static const uint8_t BAST_pd_qpsk_q[] =
   {
      0x00,
      0x13,
      0x25,
      0x32,
      0x3C,
      0x43,
      0x48,
      0x4A,
      0x4C,
      0x4D,
      0x4E,
      0x4E,
      0x4F
   };

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPA, &val);

   for (i_idx = 0; i_idx < 32; i_idx++)
   {
      if (i_idx < 12)
         idx = i_idx;
      else
         idx = 12;
      val = (uint32_t)BAST_pd_qpsk_i[idx] << 16;

      for (q_idx = 0; q_idx < 32; q_idx++)
      {
         if (q_idx < 12)
            idx = q_idx;
         else
            idx = 12;
         val |= ((uint32_t)BAST_pd_qpsk_q[idx] << 8);
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPD, &val);
      }
   }

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPA, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcGenerate8pskPdTable_isr()
******************************************************************************/
void BAST_g2_P_LdpcGenerate8pskPdTable_isr(BAST_ChannelHandle h)
{
   uint32_t val, i, j, n = 0;
   uint16_t s;
   uint8_t x;

   static const uint16_t BAST_pd_8psk[] =
   {
      0x0000,0x0077,0x00AD,0x00C2,0x00CB,0x00CF,0x00D1,0x00D2,
      0x00D3,0x00D3,0x00D3,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,
      0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,
      0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,0x00D4,
      0x7700,0x6969,0x51A1,0x3FBB,0x34C7,0x2DCD,0x2AD0,0x28D2,
      0x27D3,0x27D3,0x26D3,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,
      0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,
      0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,0x26D4,
      0xAD00,0xA151,0x8888,0x6EA8,0x5CBB,0x50C6,0x49CC,0x45D0,
      0x42D2,0x41D3,0x41D3,0x40D3,0x40D3,0x40D4,0x40D4,0x40D4,
      0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,
      0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,0x40D4,
      0xC200,0xBB3F,0xA86E,0x9090,0x7AA9,0x69BA,0x5DC5,0x56CC,
      0x52CF,0x50D1,0x4FD2,0x4ED3,0x4ED3,0x4ED3,0x4ED4,0x4ED4,
      0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,
      0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,0x4ED4,
      0xCB00,0xC734,0xBB5C,0xA97A,0x9494,0x7FA9,0x6EBA,0x63C5,
      0x5CCC,0x58CF,0x56D1,0x55D2,0x54D3,0x54D3,0x54D3,0x54D4,
      0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,
      0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,0x54D4,
      0xCF00,0xCD2D,0xC650,0xBA69,0xA97F,0x9595,0x81A9,0x70BA,
      0x65C5,0x5ECB,0x5ACF,0x58D1,0x57D2,0x57D3,0x56D3,0x56D3,
      0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,
      0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,0x56D4,
      0xD100,0xD02A,0xCC49,0xC55D,0xBA6E,0xA981,0x9595,0x81A9,
      0x71B9,0x66C5,0x5FCB,0x5BCF,0x59D1,0x58D2,0x58D3,0x57D3,
      0x57D3,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,
      0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,
      0xD200,0xD228,0xD045,0xCC56,0xC563,0xBA70,0xA981,0x9696,
      0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,
      0x58D3,0x58D3,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,
      0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,0x57D4,
      0xD300,0xD327,0xD242,0xCF52,0xCC5C,0xC565,0xB971,0xA982,
      0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,
      0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD300,0xD327,0xD341,0xD150,0xCF58,0xCB5E,0xC566,0xB972,
      0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,
      0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD300,0xD326,0xD341,0xD24F,0xD156,0xCF5A,0xCB5F,0xC567,
      0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,
      0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD340,0xD34E,0xD255,0xD158,0xCF5B,0xCB60,
      0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,
      0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD340,0xD34E,0xD354,0xD257,0xD159,0xCF5C,
      0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,
      0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD34E,0xD354,0xD357,0xD258,0xD15A,
      0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,
      0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,
      0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD354,0xD356,0xD358,0xD259,
      0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,
      0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,
      0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD356,0xD357,0xD358,
      0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,
      0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,
      0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD357,0xD358,
      0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,
      0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,
      0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD358,
      0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,
      0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,
      0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,
      0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,
      0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,
      0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,
      0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,
      0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,
      0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,0x58D4,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,
      0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,
      0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,0x58D3,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,
      0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,
      0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,0x58D3,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,
      0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,
      0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,0x58D3,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD358,0xD358,
      0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,
      0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,0x59D2,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD358,
      0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,
      0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,0x5AD1,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,0xC567,
      0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,0x5CCF,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,0xCB60,
      0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,0x60CB,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,0xCF5C,
      0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,0x67C5,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,0xD15A,
      0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,0x72B9,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD458,0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,0xD259,
      0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,0x82A9,
      0xD400,0xD426,0xD440,0xD44E,0xD454,0xD456,0xD457,0xD457,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,0xD458,
      0xD458,0xD458,0xD458,0xD458,0xD458,0xD358,0xD358,0xD358,
      0xD259,0xD15A,0xCF5C,0xCB60,0xC567,0xB972,0xA982,0x9696,
   };

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPA, &val);

   for (i = 0; i < 32; i++)
   {
      for (j = 0; j < 32; j++)
      {
         s = BAST_pd_8psk[n++];
         x = (s >> 8) & 0xFF;
         if (x & 0x80)
            x = (x << 1) | 0x01;
         else
            x = (x << 1);
         val = x << 16;
         if (x & 1)
         {
            val |= 0x01000000;
            val &= 0xFFFE0000;
         }

         x = (s & 0xFF);
         val |= (x << 8);
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPD, &val);
      }
   }

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_LUPA, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcEnableHpStateChange_isr()
******************************************************************************/
void BAST_g2_P_LdpcEnableHpStateChange_isr(BAST_ChannelHandle h, bool bEnable)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t irq_stat;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_IRQ, &irq_stat);
   irq_stat &= 0xFFFF0000; /* retain interrupt enable bits */
   irq_stat |= 0x00000400; /* clear HP state change interrupt */

   if (bEnable)
      irq_stat |= 0x04000000;
   else
      irq_stat &= ~0x04000000;

   /* clear and enable HP state change interrupt */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IRQ, &irq_stat);

   if (bEnable)
      BINT_EnableCallback_isr(hChn->hHpCb);
   else
      BINT_DisableCallback_isr(hChn->hHpCb);
}


/******************************************************************************
 BAST_g2_P_LdpcFailedAcquisition_isr()
******************************************************************************/
void BAST_g2_P_LdpcFailedAcquisition_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("failed to acquire"));
#endif
   hChn->acqState = BAST_AcqState_eIdle;
   BAST_g2_P_IndicateNotLocked_isrsafe(h);
   BAST_g2_P_DisableChannelInterrupts_isr(h, false, false);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire4_isr() - called when HP state is 17 (acquire_ldpc_3)
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire4_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t delay;

   BAST_g2_P_StartBert_isr(h);
   if ((BAST_MODE_IS_TURBO(hChn->acqParams.mode)) && (hChn->acqParams.symbolRate < 10000000))
      delay = 60000; /* TURBO */
   else
      delay = 2000; /* LDPC */
   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, delay, BAST_g2_P_LdpcAcquire5_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire5_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire5_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

#ifndef BAST_EXCLUDE_TURBO
   static const uint32_t script_turbo_4[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_OPLL, 0x000000FF),
      BAST_SCRIPT_WRITE(BCHP_SDS_OPLL2, 0x00000001),
      BAST_SCRIPT_WRITE(BCHP_SDS_OIFCTL3, 0x10),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x88),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x80),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_turbo_5[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_TCIL, 0x00009FCC),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TRSD, 0x00004FCC),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0xC0),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TZPK, 0x03B58F34),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFMT, 0x00028008),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TSYN, 0x0103FEFE),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TPAK, 0x0009BB47),
      BAST_SCRIPT_EXIT
   };
#endif

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      /* reset the LDPC */
      val = 0x00000001;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, &val);
      val = 0x00000000;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, &val);

      val = 0x00000002;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_0, &val);

      BAST_g2_P_LdpcConfig_isr(h, BCHP_AFEC_LDPC_CONFIG_1, BAST_ldpc_config_1);
      BAST_g2_P_LdpcSetConfig2_isr(h);

      val = 0;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_RST, &val);
      val = 0x030F0E0F;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPLCK, &val);

      BAST_g2_P_LdpcConfig_isr(h, BCHP_AFEC_BCH_DECCFG0, BAST_ldpc_bch_deccfg0);
      BAST_g2_P_LdpcConfig_isr(h, BCHP_AFEC_BCH_DECCFG1, BAST_ldpc_bch_deccfg1);

      BAST_g2_P_LdpcSetMpcfg1_isr(h);

      val = 0x02;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_RST, &val);
      val = 0x00;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_RST, &val);

      val = 0xBC47705E; /* orig: 0xBC47105E; */
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG0, &val);
      val = 0x05000502;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_0, &val);

      BAST_g2_P_LdpcSetOpll_isr(h);

#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
      if (BAST_g2_P_LdpcIsPilotOn_isr(h))
      {
         val = 0x92;
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CLFBCTL, &val);
      }
#endif


   }
#ifndef BAST_EXCLUDE_TURBO
   else
   {
      /* start Turbo here */
      BAST_g2_P_ProcessScript_isrsafe(h, script_turbo_4);

      BAST_g2_P_LdpcConfig_isr(h, BCHP_TFEC_TITR, BAST_turbo_titr);

      BAST_g2_P_ProcessScript_isrsafe(h, script_turbo_5);

      BAST_g2_P_TurboSetTtur_isr(h);

      BAST_g2_P_LdpcModeConfig_isr(h, BCHP_TFEC_TZSY, BAST_turbo_tzsy);
      val = 0x40;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, &val);
#if (BCHP_CHIP==7340) || (BCHP_CHIP==7342)
      /* TFEC_TNBLK */
      /* TFEC_TCBLK */
      /* TFEC_0_TBBLK */
      /* TFEC_TCSYM */
#else
      val = 0x04D2162E;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TERR, &val);
      val = 0x00592014;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TCOR, &val);
#endif

      BAST_g2_P_TurboSetTssq_isr(h);

      val = 0x60;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, &val);

      BAST_g2_P_TurboSetOpll_isr(h);
   }
#endif

   if (BAST_g2_P_SetOpll_isr(h) != BERR_SUCCESS)
      return BAST_g2_P_LdpcReacquire_isr(h);

   hChn->initFreqOffset = BAST_g2_P_GetCarrierError_isrsafe(h);

   hChn->funct_state = 0;
   hChn->lockIsrFlag = 0xFF;
   hChn->freqTransferInt = 0;
   hChn->acqState = BAST_AcqState_eWaitForLock;

   return BAST_g2_P_LdpcEnableLockInterrupts_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire6_isr() - start lock timeout
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire6_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t delay;

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("setting timer for lock"));
#endif
   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
      delay = 3000000; /* LDPC */
   else
      delay = 2000000; /* TURBO */

   hChn->noSyncCount = 0;
   if ((hChn->acqParams.mode == BAST_Mode_eTurbo_scan) ||
       (BAST_MODE_IS_LDPC(hChn->actualMode) && BAST_g2_P_LdpcIsPilotOn_isr(h)))
   {
      retCode = BAST_g2_P_LdpcSetupNotLockedMonitor_isr(h);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_WRN(("unable to set timer for no_lock monitoring"));
      }
   }

   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, delay, BAST_g2_P_LdpcAcquire7_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcAcquire7_isr() - ldpc lock timer expired
******************************************************************************/
BERR_Code BAST_g2_P_LdpcAcquire7_isr(BAST_ChannelHandle h)
{
#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("lock timer expired"));
#endif
   BAST_g2_P_IndicateNotLocked_isrsafe(h);
   return BAST_g2_P_LdpcReacquire_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcSetupNotLockedMonitor_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcSetupNotLockedMonitor_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t delay, P_hi, P_lo, Q_hi;

   delay = 1000;
   if (hChn->acqParams.symbolRate < 15000000)
   {
      BMTH_HILO_32TO64_Mul(delay, 15000000, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &delay);
   }
   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBer, delay, BAST_g2_P_LdpcMonitorNotLocked_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcMonitorNotLocked_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcMonitorNotLocked_isr(BAST_ChannelHandle h)
{
   static const uint8_t BAST_noSyncCount_threshold[] =
   {
      50, /* qpsk 1/2 */
      35, /* qpsk 2/3 */
      34, /* qpsk 3/4 */
      31, /* qpsk 5/6 */
      30, /* qpsk 7/8 */
      19, /* 8psk 2/3 */
      18, /* 8psk 3/4 */
      19, /* 8psk 4/5 */
      18, /* 8psk 5/6 */
      18  /* 8psk 8/9 */
   };

   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t irqsts3;
   int32_t frf, frf4, new_frf, div;
   bool bSetTimer = false;

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      /* maintain noSyncCount count */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_IRQSTS3, &irqsts3);
      if (irqsts3 & 0x10)
      {
         hChn->noSyncCount++;
         if (hChn->noSyncCount > BAST_noSyncCount_threshold[hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2])
            retCode = BAST_g2_P_LdpcAcquire7_isr(h);
         else
            bSetTimer = true;
      }
      else
         hChn->noSyncCount = 0;
   }
   else if (BAST_MODE_IS_LDPC(hChn->actualMode) && BAST_g2_P_LdpcIsPilotOn_isr(h))
   {
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FRF, (uint32_t*)&frf);
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_FRF4, (uint32_t*)&frf4);
      if (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2)
         div = 16;
      else
         div = 4;
      new_frf = frf + (frf4/div);
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FRF, (uint32_t*)&new_frf);
      /* BDBG_MSG(("frf=0x%X, frf4=0x%X, div=%d, new_frf=0x%X", frf, frf4, div, new_frf)); */
      bSetTimer = true;
   }

   if (bSetTimer)
      retCode = BAST_g2_P_LdpcSetupNotLockedMonitor_isr(h);

   return retCode;
}


/******************************************************************************
 BAST_g2_P_LdpcEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcEnableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))
   {
      BINT_ClearCallback_isr(hChn->hLdpcLockCb);
      BINT_ClearCallback_isr(hChn->hLdpcNotLockCb);
      BINT_EnableCallback_isr(hChn->hLdpcLockCb);
      BINT_EnableCallback_isr(hChn->hLdpcNotLockCb);
   }
#ifndef BAST_EXCLUDE_TURBO
   else
   {
      BINT_ClearCallback_isr(hChn->hTurboLockCb);
      BINT_ClearCallback_isr(hChn->hTurboNotLockCb);
      BINT_EnableCallback_isr(hChn->hTurboLockCb);
      BINT_EnableCallback_isr(hChn->hTurboNotLockCb);
   }
#endif

   return BAST_g2_P_LdpcAcquire6_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcLockStable_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcLockStable_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBer);

   if (hChn->acqTimeState == 1)
      BAST_g2_P_UpdateAcquisitionTime_isr(h);

   BAST_g2_P_IndicateLocked_isr(h);

   hChn->acqState = BAST_AcqState_eMonitor;

   if (hChn->firstTimeLock == 0)
   {
      hChn->firstTimeLock = 1;
      BAST_g2_P_ResetErrorCounters_isr(h);
   }

/* BDBG_MSG(("BAST_g2_P_LdpcLockStable_isr(): lockFilterTime=%d, acqTime=%d", hChn->lockFilterTime, hChn->acqTime)); */
   hChn->lockFilterTime = BAST_LDPC_LOCK_FILTER_TIME; /* reset lock indication filter time */

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_LdpcLock_isr()
******************************************************************************/
void BAST_g2_P_LdpcLock_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;

   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eAfecLock);
   BDBG_MSG(("BAST_g2_P_LdpcLock_isr"));

   if (hChn->lockIsrFlag == 1)
   {
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("ignoring lock isr"));
#endif
      return;
   }
   hChn->lockIsrFlag = 1;

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud);

   if (hChn->acqState == BAST_AcqState_eWaitForLock)
   {
      /* ldpc locked for the first time */
      hChn->acqState = BAST_AcqState_eMonitor;

      BAST_g2_P_LdpcSetPlc_isr(h, false);

      BAST_g2_P_LdpcSetFinalBaudLoopBw_isr(h);
      BAST_g2_P_LdpcSetPsl_isr(h);

      if (BAST_g2_P_LdpcIs8psk_isr(h))
         val = 0x44;  /* 8psk */
      else
         val = 0x66;  /* qpsk */
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQMU, &val);

      BAST_g2_P_ResyncBert_isr(h);
   }

   BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eGen1, hChn->lockFilterTime, BAST_g2_P_LdpcLockStable_isr);
   BAST_g2_P_LdpcMonitor_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcNotLock_isr()
******************************************************************************/
void BAST_g2_P_LdpcNotLock_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eAfecNotLock);
   BAST_g2_P_LdpcUpdateBlockCounters_isr(h);

   if (hChn->lockIsrFlag == 0)
   {
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("ignoring not lock isr"));
#endif
      return;
   }
   hChn->lockIsrFlag = 0;

   BDBG_MSG(("BAST_g2_P_LdpcNotLock_isr"));

   if (hChn->firstTimeLock)
   {
      hChn->lockFilterTime = hChn->lockFilterTime + BAST_LDPC_LOCK_FILTER_INCREMENT;
      if (hChn->lockFilterTime > BAST_LDPC_LOCK_FILTER_TIME_MAX)
         hChn->lockFilterTime = BAST_LDPC_LOCK_FILTER_TIME_MAX;
#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("lockFilterTime = %d", hChn->lockFilterTime));
#endif
   }

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eGen1);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
   if (((val >> 24) & 0x1F) != 17)
      goto reacquire;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS, &val);
   if ((val & 0x00010000) || (hChn->acqState == BAST_AcqState_eWaitForLock))
   {
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("ldpc_locked=%d, waiting for mpeg lock", (val & 0x00010000) ? 1 : 0));
#endif
      BAST_g2_P_IndicateNotLocked_isrsafe((BAST_ChannelHandle)h);
      BAST_g2_P_LdpcAcquire6_isr(h);
   }
   else if ((hChn->ldpcCtl & BAST_G2_LDPC_CTL_DISABLE_FEC_REACQ) == 0)
   {
      reacquire:
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("reacquiring because LDPC not locked"));
#endif
      BAST_g2_P_IndicateNotLocked_isrsafe((BAST_ChannelHandle)h);
      BAST_g2_P_DisableChannelInterrupts_isr(h, false, false);
      BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, ((BAST_g2_P_Handle *)(h->pDevice->pImpl))->reacqDelay, BAST_g2_P_LdpcReacquire_isr);
   }
}


/******************************************************************************
 BAST_g2_P_ProcessHpState0_isr()
******************************************************************************/
void BAST_g2_P_ProcessHpState0_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;

   if ((hChn->prev_state == 4) || (hChn->prev_state == 5)) /* reset HP if coming from TIM_START */
      BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_HPCTRL1, 0x01);

   /* always acquire with tuner not in LO tracking mode*/
   BAST_g2_P_TunerSetDigctl7_isr(h, 0x04);

   val = 0x03;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_MIXCTL, &val);

   val = 0x0F;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_CLCTL, &val);

   hChn->count2 = 1;
}


/******************************************************************************
 BAST_g2_P_ProcessHpState15_isr()
******************************************************************************/
void BAST_g2_P_ProcessHpState15_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val, data0;

   static const uint32_t script_ldpc_8[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_ABW, 0x03030000),   /* final AGC BW */
#if 0
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL5, 0x01), /* 0x09 causes AGC loop to oscillate slighly */
#endif
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL15, 0xFB),
      BAST_SCRIPT_WRITE(BCHP_TUNER_DIGCTL16, 0xEE),
      BAST_SCRIPT_EXIT
   };

   if (!(hChn->count2 & 1))
      BAST_g2_P_ProcessHpState0_isr(h);
   hChn->count2 |= 2;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_EQBLND, &val);
   data0 = (val & 0x000000FF) & ~0x04; /* disable CMA */
   val &= 0xFFFFFF00;
   val |= data0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQBLND, &val);

   val = 0x06;   /* unfreeze main tap */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE2, &val);
   val = 0x02;   /* unfreeze other taps */
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQFFE2, &val);

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))    /* LDPC mode */
   {
      /* setting up VLC */
      BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_VLCI, BAST_ldpc_vlci);
      BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_VLCQ, BAST_ldpc_vlci);
   }
   else
   {
      /* setting up VLC */
      val = 0x20000000;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCI, &val);
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_VLCQ, &val);
   }

   /* setting up BERT */
   BAST_g2_P_ProcessScript_isrsafe(h, script_ldpc_8);

   if (BAST_MODE_IS_LDPC(hChn->acqParams.mode))    /* LDPC mode */
   {
      BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_SNRHT, BAST_ldpc_snrht);
      BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_SNRLT, BAST_ldpc_snrlt);
   }

   val = 0x01546732;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BEM1, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BEM2, &val);

   BAST_g2_P_InitBert_isr(h);

   val = 0x0FFF05FF;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BEIT, &val);
   val = 0x88;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SNRCTL, &val);
   val = 0x0B;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_SNRCTL, &val);
}


/******************************************************************************
 BAST_g2_P_ProcessHpState16_isr()
******************************************************************************/
void BAST_g2_P_ProcessHpState16_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;

   if (!(hChn->count2 & 2))
      BAST_g2_P_ProcessHpState15_isr(h);
   hChn->count2 |= 4;

   if (hChn->bFrfTransferred)
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FRF, (uint32_t*)&(hChn->frfStep));
}


/******************************************************************************
 BAST_g2_P_LdpcTransferCarrierToTuner_isr()
******************************************************************************/
void BAST_g2_P_LdpcTransferCarrierToTuner_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   BERR_Code retCode;
   int32_t carr_error;
   uint32_t val;
#ifdef BAST_ENABLE_LDPC_DCO_SEPARATION
   int32_t frfStep;
   uint32_t Q_hi, Q_lo, P_hi, P_lo;
   bool bSpinv = false;
#endif

   carr_error = BAST_g2_P_GetCarrierError_isrsafe(h);

   BAST_g2_P_FreezeLoops_isr(h);

   val = 0;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FRF, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FLI, &val);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLI, &val);

   /* retune to get optimal popcap settings */
   hChn->tunerIfStep = carr_error;

#ifdef BAST_ENABLE_LDPC_DCO_SEPARATION
   hChn->tunerOffset = BAST_DCO_OFFSET; /* put in 10KHz offset in MB2 */

   /* put 10KHz offset in FRF to compensate MB2 offset */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HPCTRL1, &val);
   if (val & 0x02)
   {
      /* HP is enabled */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
      if (val & 0x00020000)
         bSpinv = true;
   }

   BMTH_HILO_32TO64_Mul(BAST_DCO_OFFSET, 16777216, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &Q_hi, &Q_lo);
   if (bSpinv)
      frfStep = (int32_t)-Q_lo;
   else
      frfStep = (int32_t)Q_lo;
   hChn->frfStep = frfStep * 256;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FRF, (uint32_t*)&(hChn->frfStep));
#else
   hChn->tunerOffset = 0;
#endif

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("setting tunerIfStep to %d Hz", hChn->tunerIfStep));
#endif
   retCode = BAST_g2_P_TunerQuickTune_isr(h, BAST_g2_P_LdpcTransferCarrierToTuner1_isr);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("BAST_g2_P_TunerQuickTune_isr() error 0x%X", retCode));
	}
}


/******************************************************************************
 BAST_g2_P_LdpcTransferCarrierToTuner1_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcTransferCarrierToTuner1_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;

   hChn->tuneMixStatus |= BAST_TUNE_MIX_RESET_IF_PREVIOUS_SETTING_FAILS;

   BAST_g2_P_UnfreezeLoops_isr(h);
   hChn->bFrfTransferred = true;
   hChn->funct_state = 0;
   return BAST_g2_P_LdpcAcquire3_isr(h);
}


/******************************************************************************
 BAST_g2_P_ProcessHpState17_isr()
******************************************************************************/
void BAST_g2_P_ProcessHpState17_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;
   BAST_g2_FUNCT funct_ptr;

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud);
   BAST_g2_P_LdpcEnableHpStateChange_isr(h, false);

   if (!(hChn->count2 & 4))
      BAST_g2_P_ProcessHpState16_isr(h);
   hChn->count2 |= 8;

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      hChn->turboScanState |= (BAST_TURBO_SCAN_HP_INIT | BAST_TURBO_SCAN_HP_LOCKED);
      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hChn->turboScanState |= BAST_TURBO_SCAN_8PSK_HP_LOCKED;
   }
   else if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 33000, BAST_g2_P_LdpcGetModcod_isr);
      return;
   }

   if (BAST_g2_P_CarrierOffsetOutOfRange_isr(h))
   {
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("carrier freq out of range"));
#endif
      BAST_g2_P_LdpcReacquire_isr(h);
      return;
   }

   if ((hChn->bFrfTransferred == false) && (hChn->bExternalTuner == false))
   {
      BAST_g2_P_LdpcTransferCarrierToTuner_isr(h);
      return;
   }

   /* wait for loops to be stable */
   hChn->funct_state = 0;
   funct_ptr = BAST_g2_P_LdpcAcquire4_isr;

   val = hChn->trackingAnactl4;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TUNER_ANACTL4, &val);


   BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 2000, funct_ptr);
}


/******************************************************************************
 BAST_g2_P_LdpcHp17Timeout_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcHp17Timeout_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("HP state 17 timeout"));
#endif
   BAST_g2_P_DisableChannelInterrupts_isr(h, false, false);

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      if ((hChn->turboScanState & BAST_TURBO_SCAN_HP_INIT) == 0)
      {
         hChn->turboScanState = BAST_TURBO_SCAN_HP_INIT;
         if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
            hChn->turboScanState |= BAST_TURBO_SCAN_8PSK_FAILED;
      }
   }
   return BAST_g2_P_LdpcReacquire_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcHp_isr()
******************************************************************************/
void BAST_g2_P_LdpcHp_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t state, stat, irqstat;
   uint8_t val8;

   BSTD_UNUSED(param);
   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eLdpcHp);

#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("BAST_g2_P_LdpcHp_isr()"));
#endif

   if (hChn->bExternalTuner == false)
      BAST_g2_P_3StageAgc_isr(h, 0);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &stat);
   state = (stat >> 24) & 0x1F;

   process_hp_state:
#ifdef BAST_LDPC_DEBUG
   BDBG_MSG(("HP state %d", state));
#endif

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_IRQ, &irqstat);
   if ((irqstat & 0x02000200) == 0x02000200)
   {
      /* HP reacquire interrupt */
      irqstat &= 0xFDFF0000;
      irqstat |= 0x00000200;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_IRQ, &irqstat);

      val8 = (hChn->ldpcScanState + 1) & 0x03;
      hChn->ldpcScanState &= BAST_LDPC_SCAN_MASK;
      hChn->ldpcScanState |= val8;
#ifdef BAST_LDPC_DEBUG
      BDBG_MSG(("HP reacquire: scan mode %d", val8));
#endif

      if (val8 == 0)
      {
         BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, ((BAST_g2_P_Handle *)(h->pDevice->pImpl))->reacqDelay, BAST_g2_P_LdpcReacquire_isr);
      }
      else
         BAST_g2_P_LdpcReacquire_isr(h);
      return;
   }

   switch (state)
   {
      case 0: /* peak search */
         BAST_g2_P_ProcessHpState0_isr(h);
         break;

      case 15:
         BAST_g2_P_ProcessHpState15_isr(h);
         break;

      case 16:
         BAST_g2_P_ProcessHpState16_isr(h);
         break;

      case 17:
         BAST_g2_P_ProcessHpState17_isr(h);
         break;

      default:
         break;
   }

   hChn->prev_state = state;
   if (state == 17)
      return;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &stat);
   state = (stat >> 24) & 0x1F;
   if (state != hChn->prev_state)
   {
      /* BDBG_MSG(("HP state changed")); */
      goto process_hp_state;
   }

   BAST_g2_P_LdpcEnableHpStateChange_isr(h, true);
}


/******************************************************************************
 BAST_g2_P_LdpcSetConfig2_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetConfig2_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t config2, idx, block_length, val, extra_cycles, P_hi, P_lo, Q_hi, Q_lo;

   static const uint32_t BAST_block_length[] =   /* must by multiplied by 6 */
   {
      105, /* 1/2 */
      132, /* 3/5 */
      100, /* 2/3 */
      105, /* 3/4 */
      108, /* 4/5 */
      110, /* 5/6 */
      90, /* 8/9 */
      90  /* 9/10 */
   };

   static const uint32_t BAST_extra_cycles[] =
   {
      27, /* 1/2 */
      35, /* 3/5 */
      35, /* 2/3 */
      34, /* 3/4 */
      41, /* 4/5 */
      47, /* 5/6 */
      43, /* 8/9 */
      46  /* 9/10 */
   };

   static const uint32_t BAST_ldpc_config_2[] =
   {
      0x00fa00cd, /* QPSK LDPC+BCH 1/2 */
      0x00c7008c, /* QPSK LDPC+BCH 3/5 */
      0x01060083, /* QPSK LDPC+BCH 2/3 */
      0x00fa00a3, /* QPSK LDPC+BCH 3/4 */
      0x00f100bc, /* QPSK LDPC+BCH 4/5 */
      0x00ed00d4, /* QPSK LDPC+BCH 5/6 */
      0x0120010d, /* QPSK LDPC+BCH 8/9 */
      0x0120011b, /* QPSK LDPC+BCH 9/10 */
      0x008400e7, /* 8PSK LDPC+BCH 3/5 */
      0x00ae0122, /* 8PSK LDPC+BCH 2/3 */
      0x00a5018b, /* 8PSK LDPC+BCH 3/4 */
      0x009c0210, /* 8PSK LDPC+BCH 5/6 */
      0x00be0253, /* 8PSK LDPC+BCH 8/9 */
      0x00be0277  /* 8PSK LDPC+BCH 9/10 */
   };

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
      return;

   BAST_g2_P_GetFecFreq_isr(h);
   if (BAST_g2_P_LdpcIs8psk_isr(h))
   {
      val = 10800; /* 8PSK frame_size/2 */
      if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_3_4)
         idx = 1;
      else
         idx = 2;
      idx += (hChn->actualMode - BAST_Mode_eLdpc_8psk_3_5);
   }
   else
   {
      val = 16200; /* QPSK frame size/2 */
      idx = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
   }

   block_length = BAST_block_length[idx] * 12;
   extra_cycles = BAST_extra_cycles[idx];
   BMTH_HILO_32TO64_Mul(hChn->fecFreq, val << 1, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   val = Q_lo - (block_length << 1);
   val /= (block_length + extra_cycles);
   if (val > 150)
      val = 150;

   idx = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
   config2 = (val << 16) | (BAST_ldpc_config_2[idx] & 0x0000FFFF);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_2, &config2);
}


/******************************************************************************
 BAST_g2_P_LdpcSetMpcfg1_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetMpcfg1_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (hChn->xportCtl & BAST_G2_XPORT_CTL_BCH_CHECK)
      val = 0x20;
   else
      val = 0;
   if (hChn->xportCtl & BAST_G2_XPORT_CTL_CRC8_CHECK)
      val |= 0x10;
   if (hChn->xportCtl & BAST_G2_XPORT_CTL_TEI)
      val |= 0x08;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG1, &val);
}


/******************************************************************************
 BAST_g2_P_LdpcSetOpll_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetOpll_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val1, val2, idx, data0, number_of_bits, number_of_symbols, opll, opll2, cgctrl;

   static const uint32_t BAST_ldpc_number_of_bits[] =
   {
      32128, /* 1/2 */
      38608, /* 3/5 */
      42960, /* 2/3 */
      48328, /* 3/4 */
      51568, /* 4/5 */
      53760, /* 5/6 */
      57392, /* 8/9 */
      58112  /* 9/10 */
   };

   if (BAST_g2_P_LdpcIs8psk_isr(h))
   {
      val1 = 21600;
      if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_3_4)
         data0 = 1;
      else
         data0 = 2;
      val2 = 504; /* 14*36 */
      idx = hChn->actualMode - BAST_Mode_eLdpc_8psk_3_5 + data0;
   }
   else
   {
      val1 = 32400;
      val2 = 792; /* 22*36 */
      idx = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
   }

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_CGCTRL, &cgctrl);
   if (cgctrl & 0x02)
      number_of_bits = 2 * BAST_ldpc_number_of_bits[idx];
   else
      number_of_bits = BAST_ldpc_number_of_bits[idx];

   if (BAST_g2_P_LdpcIsPilotOn_isr(h))
      val1 += val2;
   number_of_symbols = val1 + 90;

   opll = (number_of_bits >> 3);
   val1 = number_of_symbols >> 1;

   if ((val1 < opll) && (cgctrl & 0x02))
   {
      number_of_bits = BAST_ldpc_number_of_bits[idx];
      opll = (number_of_bits >> 3);
      val1 = number_of_symbols;
#if (BCHP_CHIP==7325) || (BCHP_CHIP==7335)
      BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_OIFCTL3, 0x20);
#else
      BAST_g2_P_OrRegister_isrsafe(h, BCHP_SDS_OIFCTL3, 0x100);
#endif
   }
   else
      BAST_g2_P_AndRegister_isrsafe(h, BCHP_SDS_OIFCTL3, ~0x320);

   opll2 = val1 - opll;

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OPLL, &opll);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OPLL2, &opll2);
}


/******************************************************************************
 BAST_g2_P_LdpcConfig_isr()
******************************************************************************/
void BAST_g2_P_LdpcConfig_isr(BAST_ChannelHandle h, uint32_t reg, const uint32_t *pVal)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BAST_Mode mode;
   uint32_t val, i;

   if (BAST_MODE_IS_TURBO(hChn->acqParams.mode))
   {
      i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
      goto lookup_register;
   }
   else if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      /* we're in peak_search */
      if ((reg < BCHP_AFEC_LDPC_CONFIG_0) || (reg > BCHP_AFEC_TM_4))
      {
         /* register is not in AFEC core */
         if (reg == BCHP_SDS_PLHDRCFG)
         {
            if (BAST_g2_P_LdpcIsPilotOn_isr(h))
               val = 0x20;
            else
               val = 0x00;

            if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_SHORT_FRAME)
               val |= 0x40;

            if (BAST_g2_P_LdpcIs8psk_isr(h))
               val |= 13; /* pick any 8psk mode */
            else
               val |= 7;  /* pick any qpsk mode */
            BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
         }
         else
         {
            if (BAST_g2_P_LdpcIs8psk_isr(h))
               mode = BAST_Mode_eLdpc_8psk_2_3; /* pick any 8psk mode */
            else
               mode = BAST_Mode_eLdpc_Qpsk_3_4; /* pick any qpsk mode */
            goto set_ldpc_idx;
         }
      }
   }
   else
   {
      mode = hChn->actualMode;

      set_ldpc_idx:
      i = mode - BAST_Mode_eLdpc_Qpsk_1_2;
/* BDBG_MSG(("BAST_g2_P_LdpcConfig_isr(): mode=%d, i=%d", mode, i)); */

      lookup_register:
      val = pVal[i];
      BAST_g2_P_WriteRegister_isrsafe(h, reg, &val);
   }
}


/******************************************************************************
 BAST_g2_P_LdpcReacquire_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcReacquire_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   BDBG_MSG(("BAST_g2_P_LdpcReacquire_isr"));

   BAST_g2_P_IndicateNotLocked_isrsafe(h);
   BAST_g2_P_DisableChannelInterrupts_isr(h, false, false);
   BAST_g2_P_LdpcUpdateBlockCounters_isr(h);
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_g2_P_ResetStatusIndicators_isrsafe(h);
#endif

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
      goto reacquire;
   else
   {
      if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_REACQ_DISABLE) == 0)
      {
         if (hChn->maxReacq && (hChn->acqCount > hChn->maxReacq))
            goto give_up;

         reacquire:
         /* widen filter if using internal tuner */
         if ((hChn->bExternalTuner == false) && (hChn->tunerCutoffWide))
         {
            val = hChn->trackingAnactl4 + (hChn->tunerCutoffWide << 2);
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TUNER_ANACTL4, &val);
         }

         if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
            return BAST_g2_P_TurboAcquire_isr(h);

         if (((hChn->ldpcScanState & 0x03) == 0) && (hChn->blindScanStatus == BAST_BlindScanStatus_eAcquire))
            return BAST_g2_P_BlindReacquire_isr(h);

         if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT_SCAN)
            hChn->acqParams.acq_ctl ^= BAST_ACQSETTINGS_LDPC_PILOT;

         hChn->funct_state = 0;
         if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
         {
            if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MODE_MASK) == 0)
               goto LdpcReacquire_1;

            goto acquire_without_retune;
         }
         else
         {
            if (hChn->blindScanStatus != BAST_BlindScanStatus_eAcquire)
               hChn->acqCount++;

            LdpcReacquire_1:
            if (hChn->acqCount & 1)
            {
               acquire_without_retune:
               retCode = BAST_g2_P_LdpcAcquire1_isr(h);
            }
            else
            {
               hChn->tunerIfStep = 0;
               hChn->acqState = BAST_AcqState_eTuning;
               hChn->acqParams.carrierFreqOffset = 0;
               retCode = BAST_g2_P_TunerSetFreq_isr(h, BAST_g2_P_Acquire_isr);
            }
         }
      }
      else
      {
         give_up:
         BAST_g2_P_LdpcFailedAcquisition_isr(h);
      }
   }

   return retCode;
}


/******************************************************************************
 BAST_g2_P_LdpcSetFinalBaudLoopBw_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetFinalBaudLoopBw_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t lval1, lval2;

   if (hChn->acqParams.symbolRate >= 12000000)
   {
      lval1 = 0x00267000;
      lval2 = 0x000033C0;
   }
   else if (hChn->acqParams.symbolRate >= 4000000)
   {
      lval1 = 0x00140000;
      lval2 = 0x00000190;
   }
   else if (hChn->acqParams.symbolRate >= 2000000)
   {
      lval1 = 0x00418000;
      lval2 = 0x00000431;
   }
   else
   {
      lval1 = 0x00B10000;
      lval2 = 0x00000D99;
   }

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRLC, &lval1);
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_BRIC, &lval2);
}


/******************************************************************************
 BAST_g2_P_LdpcSetPsl_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetPsl_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t psl_ctl, val, iter, x, gain, beta;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_MASK) == BAST_LDPC_SCAN_ON)
   {
      psl_ctl = 0x00000050;
   }
   else
   {
      /* limit max iterations to 150 */
      BAST_g2_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_2, &val);
      iter = (val >> 16) & 0x07FF;
      if (iter > 150)
      {
         iter = 150;
         val = (val & 0x0000FFFF) | (iter << 16);
         BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_CONFIG_2, &val);
      }

      /* set PSL threshold to 68% max iterations for 8psk 3/5
         set PSL threshold to 56% max iterations for all other code rates */
      if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_5)
         x = 68;
      else if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_4)
         x = 75;
      else
         x = 56;
      iter = (((iter * x * 2) / 100) + 1) >> 1;
      iter = iter << 2;

      /* set gain and beta */
      if (hChn->acqParams.symbolRate < 5000000)
      {
         gain = 0x0B; /* gain */
         beta = 0x0A; /* beta */
      }
      else
      {
         gain = 0x09; /* gain */
         beta = 0x07; /* beta */
      }

      psl_ctl = ((beta << 4) & 0xF0) | 0x05;
      psl_ctl |= ((gain & 0x0F) | (((iter & 0x0003) << 6) & 0xC0)) << 8;
      psl_ctl |= ((iter >> 2) & 0xFF) << 16;
      psl_ctl |= ((iter >> 10) & 0x07) << 24;
   }
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_CTL, &psl_ctl);
}


/******************************************************************************
 BAST_g2_P_LdpcRmd_isr()
******************************************************************************/
static uint8_t BAST_g2_P_LdpcRmd_isr(uint32_t enc_msg)
{
   uint32_t mask;
   int i, j, y[32], sum[16], diff[16], Y, I;
   uint8_t dec_msg;

   for (i = 0, mask = 0x80000000; mask; mask = mask >> 1)
      y[i++] = ((enc_msg & mask) ? -1 : 1);

   for (i = 0; i < 5; i++)
   {
      for (j = 0; j < 32; j += 2)
      {
         sum[j >> 1] = y[j] + y[j+1];
         diff[j >> 1] = y[j] - y[j+1];
      }
      for (j = 0; j < 16; j++)
      {
         y[j] = sum[j];
         y[j+16] = diff[j];
      }
   }

   for (i = Y = I = 0; i < 32; i++)
   {
      if (abs(y[i]) > abs(Y))
      {
         I = i;
         Y = y[i];
      }
   }

   I |= (Y <= 0 ? 0x20 : 0);
   dec_msg = 0;
   if (I & 1) dec_msg |= 32;
   if (I & 2) dec_msg |= 16;
   if (I & 4) dec_msg |= 8;
   if (I & 8) dec_msg |= 4;
   if (I & 16) dec_msg |= 2;
   if (I & 32) dec_msg |= 1;

   return dec_msg;
}


/******************************************************************************
 BAST_g2_P_LdpcGetModcod_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcGetModcod_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->funct_state = 0;
   hChn->count1 = 0;
   return BAST_g2_P_LdpcGetModcod1_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcGetModcod1_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcGetModcod1_isr(BAST_ChannelHandle h)
{
   #define descramble_hi (0x719D83C9)
   #define descramble_lo (0x53422DFA)

   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, rmd_in, L4, i, j, lval1, mask, d_hi, d_lo;
   uint8_t mode;
   int8_t I, Q;

   while (hChn->funct_state != 0xFF)
   {
      switch (hChn->funct_state)
      {
         case 0: /* wait for symbols to be ready to read */
            BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HDRD, &val);
            if ((val & 0x0B00) != 0x0300)
            {
               if (hChn->count1++ < 35)
                  return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 1000, BAST_g2_P_LdpcGetModcod1_isr);

               /* modcod search failed */
#ifdef BAST_LDPC_DEBUG
               BDBG_MSG(("modcod search failed"));
#endif
               return BAST_g2_P_LdpcReacquire_isr(h);
            }
            hChn->funct_state = 1;
            break;

          case 1:
            BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_PLHDRSCR2, &val);
            d_hi = descramble_hi ^ val;
            BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_PLHDRSCR1, &val);
            d_lo = descramble_lo ^ val;

            /* skip 26-bit SOF */
            val = 0x1A000000;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_HDRA, &val);

            /* read/slice/descramble into 64-bits, then put odd bits into rmd_in */
            L4 = 0x80000000;
            rmd_in = 0;
            for (i = 2; i; i--)
            {
               lval1 = 0;
               mask = 0x80000000;
               for (j = 32; j; j--, mask = mask >> 1)
               {
                  BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_HDRD, &val); /* [31:24] = I, [23:16] = Q */
/* BDBG_MSG(("HDRD=0x%08X", val)); */
                  I = (int8_t)(val >> 24);
                  Q = (int8_t)((val >> 16) & 0xFF);
                  if (j & 0x01)
                  {
                     /* even */
                     if (I > Q)
                        lval1 |= mask;
                  }
                  else
                  {
                     /* odd */
                     if (I < -Q)
                        lval1 |= mask;
                  }
               }

               if (i == 2)
                  lval1 ^= d_hi;
               else
                  lval1 ^= d_lo;

               for (mask = 0x80000000; mask; mask = mask >> 2)
               {
                  rmd_in |= ((mask & lval1) ? L4 : 0);
                  L4 = L4 >> 1;
               }
            }

            mode = BAST_g2_P_LdpcRmd_isr(rmd_in);
            mode = mode >> 1;

            if ((mode >= 4) && (mode <= 17))
            {
               hChn->ldpcScanState |= BAST_LDPC_SCAN_FOUND;
               hChn->actualMode = BAST_Mode_eLdpc_Qpsk_1_2 + mode - 4;
               hChn->funct_state = 0xFF;
#ifdef BAST_LDPC_DEBUG
               BDBG_MSG(("modcod decoded: modcod=%d, BAST_Mode=%d", mode, hChn->actualMode));
#endif
            }
            else
            {
#ifdef BAST_LDPC_DEBUG
               BDBG_MSG(("bad modcod decoded (%d)", mode));
#endif
               return BAST_g2_P_LdpcReacquire_isr(h);
            }
            break;
      }
   }

   /* reacquire with the known code rate */
   hChn->funct_state = 5;
   return BAST_g2_P_LdpcAcquire1_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcMonitor_isr()
******************************************************************************/
BERR_Code BAST_g2_P_LdpcMonitor_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->bExternalTuner == false)
      BAST_g2_P_3StageAgc_isr(h, 0);

   BAST_g2_P_TransferFreqOffset_isr(h);
   if (hChn->bForceReacq)
      return BAST_g2_P_LdpcReacquire_isr(h);

#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_g2_P_CheckStatusIndicators_isr(h);
#endif

   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 100000, BAST_g2_P_LdpcMonitor_isr);
}


/******************************************************************************
 BAST_g2_P_LdpcUpdateBlockCounters()
******************************************************************************/
void BAST_g2_P_LdpcUpdateBlockCounters(BAST_ChannelHandle h)
{
   BKNI_EnterCriticalSection();
   BAST_g2_P_LdpcUpdateBlockCounters_isr(h);
   BKNI_LeaveCriticalSection();
}


/******************************************************************************
 BAST_g2_P_LdpcUpdateBlockCounters_isr()
******************************************************************************/
void BAST_g2_P_LdpcUpdateBlockCounters_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t corr, bad, total, val = 0;

   if (BAST_g2_P_IsLdpcOn_isrsafe(h))
   {
	   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECCBLK, &corr);
	   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECBBLK, &bad);
	   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECNBLK, &total);

	   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_DECCBLK, &val);
	   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_DECBBLK, &val);
	   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_DECNBLK, &val);

	   hChn->ldpcBadBlocks += bad;
	   hChn->ldpcCorrBlocks += corr;
	   hChn->ldpcTotalBlocks += total;
   }
#ifndef BAST_EXCLUDE_TURBO
   else if (BAST_g2_P_IsTurboOn_isrsafe(h))
   {
      BAST_g2_P_TurboUpdateErrorCounters_isr(h);
   }
#endif
   BAST_g2_P_UpdateErrorCounters_isr(h);
}


/******************************************************************************
 BAST_g2_P_LdpcSetPlc_isr()
******************************************************************************/
void BAST_g2_P_LdpcSetPlc_isr(BAST_ChannelHandle h, bool bAcq)
{
   static const uint8_t * const BAST_acq_default_piloton[] =
   {
      BAST_bw_acq_b0_default_piloton_QPSK_low,
      BAST_damp_acq_b0_default_piloton_QPSK_low,
      BAST_bw_acq_b0_default_piloton_QPSK_high,
      BAST_damp_acq_b0_default_piloton_QPSK_high,
      BAST_bw_acq_b0_default_piloton_8PSK_low,
      BAST_damp_acq_b0_default_piloton_8PSK_low,
      BAST_bw_acq_b0_default_piloton_8PSK_high,
      BAST_damp_acq_b0_default_piloton_8PSK_high
   };

   static const uint8_t * const BAST_acq_euro_piloton[] =
   {
      BAST_bw_acq_b0_euro_piloton_QPSK_low,
      BAST_damp_acq_b0_euro_piloton_QPSK_low,
      BAST_bw_acq_b0_euro_piloton_QPSK_high,
      BAST_damp_acq_b0_euro_piloton_QPSK_high,
      BAST_bw_acq_b0_euro_piloton_8PSK_low,
      BAST_damp_acq_b0_euro_piloton_8PSK_low,
      BAST_bw_acq_b0_euro_piloton_8PSK_high,
      BAST_damp_acq_b0_euro_piloton_8PSK_high
   };

   static const uint8_t * const BAST_acq_default_pilotoff[] =
   {
      BAST_bw_acq_b0_default_pilotoff_QPSK_low,
      BAST_damp_acq_b0_default_pilotoff_QPSK_low,
      BAST_bw_acq_b0_default_pilotoff_QPSK_high,
      BAST_damp_acq_b0_default_pilotoff_QPSK_high,
      BAST_bw_acq_b0_default_pilotoff_8PSK_low,
      BAST_damp_acq_b0_default_pilotoff_8PSK_low,
      BAST_bw_acq_b0_default_pilotoff_8PSK_high,
      BAST_damp_acq_b0_default_pilotoff_8PSK_high
   };

   static const uint8_t * const BAST_acq_euro_pilotoff[] =
   {
      BAST_bw_acq_b0_euro_pilotoff_QPSK_low,
      BAST_damp_acq_b0_euro_pilotoff_QPSK_low,
      BAST_bw_acq_b0_euro_pilotoff_QPSK_high,
      BAST_damp_acq_b0_euro_pilotoff_QPSK_high,
      BAST_bw_acq_b0_euro_pilotoff_8PSK_low,
      BAST_damp_acq_b0_euro_pilotoff_8PSK_low,
      BAST_bw_acq_b0_euro_pilotoff_8PSK_high,
      BAST_damp_acq_b0_euro_pilotoff_8PSK_high
   };

   static const uint8_t * const BAST_trk_default_piloton[] =
   {
      BAST_bw_trk_b0_default_piloton_QPSK_low,
      BAST_damp_trk_b0_default_piloton_QPSK_low,
      BAST_bw_trk_b0_default_piloton_QPSK_high,
      BAST_damp_trk_b0_default_piloton_QPSK_high,
      BAST_bw_trk_b0_default_piloton_8PSK_low,
      BAST_damp_trk_b0_default_piloton_8PSK_low,
      BAST_bw_trk_b0_default_piloton_8PSK_3_4,
      BAST_damp_trk_b0_default_piloton_8PSK_3_4,
      BAST_bw_trk_b0_default_piloton_8PSK_5_6,
      BAST_damp_trk_b0_default_piloton_8PSK_5_6,
      BAST_bw_trk_b0_default_piloton_8PSK_high,
      BAST_damp_trk_b0_default_piloton_8PSK_high
   };

   static const uint8_t * const BAST_trk_euro_piloton[] =
   {
      BAST_bw_trk_b0_euro_piloton_QPSK_low,
      BAST_damp_trk_b0_euro_piloton_QPSK_low,
      BAST_bw_trk_b0_euro_piloton_QPSK_high,
      BAST_damp_trk_b0_euro_piloton_QPSK_high,
      BAST_bw_trk_b0_euro_piloton_8PSK_low,
      BAST_damp_trk_b0_euro_piloton_8PSK_low,
      BAST_bw_trk_b0_euro_piloton_8PSK_3_4,
      BAST_damp_trk_b0_euro_piloton_8PSK_3_4,
      BAST_bw_trk_b0_euro_piloton_8PSK_5_6,
      BAST_damp_trk_b0_euro_piloton_8PSK_5_6,
      BAST_bw_trk_b0_euro_piloton_8PSK_high,
      BAST_damp_trk_b0_euro_piloton_8PSK_high
   };

   static const uint8_t * const BAST_trk_awgn_piloton[] =
   {
      BAST_bw_trk_b0_awgn_piloton_QPSK_low,
      BAST_damp_trk_b0_awgn_piloton_QPSK_low,
      BAST_bw_trk_b0_awgn_piloton_QPSK_high,
      BAST_damp_trk_b0_awgn_piloton_QPSK_high,
      BAST_bw_trk_b0_awgn_piloton_8PSK_low,
      BAST_damp_trk_b0_awgn_piloton_8PSK_low,
      BAST_bw_trk_b0_awgn_piloton_8PSK_3_4,
      BAST_damp_trk_b0_awgn_piloton_8PSK_3_4,
      BAST_bw_trk_b0_awgn_piloton_8PSK_5_6,
      BAST_damp_trk_b0_awgn_piloton_8PSK_5_6,
      BAST_bw_trk_b0_awgn_piloton_8PSK_high,
      BAST_damp_trk_b0_awgn_piloton_8PSK_high
   };

   static const uint8_t * const BAST_trk_default_pilotoff[] =
   {
      BAST_bw_trk_a1_default_pilotoff_QPSK_low,
      BAST_damp_trk_a1_default_pilotoff_QPSK_low,
      BAST_bw_trk_a1_default_pilotoff_QPSK_high,
      BAST_damp_trk_a1_default_pilotoff_QPSK_high,
      BAST_bw_trk_a1_default_pilotoff_8PSK_low,
      BAST_damp_trk_a1_default_pilotoff_8PSK_low,
      BAST_bw_trk_a1_default_pilotoff_8PSK_3_4,
      BAST_damp_trk_a1_default_pilotoff_8PSK_3_4,
      BAST_bw_trk_a1_default_pilotoff_8PSK_5_6,
      BAST_damp_trk_a1_default_pilotoff_8PSK_5_6,
      BAST_bw_trk_a1_default_pilotoff_8PSK_high,
      BAST_damp_trk_a1_default_pilotoff_8PSK_high
   };

   static const uint8_t * const BAST_trk_euro_pilotoff[] =
   {
      BAST_bw_trk_a1_euro_pilotoff_QPSK_low,
      BAST_damp_trk_a1_euro_pilotoff_QPSK_low,
      BAST_bw_trk_a1_euro_pilotoff_QPSK_high,
      BAST_damp_trk_a1_euro_pilotoff_QPSK_high,
      BAST_bw_trk_a1_euro_pilotoff_8PSK_low,
      BAST_damp_trk_a1_euro_pilotoff_8PSK_low,
      BAST_bw_trk_a1_euro_pilotoff_8PSK_3_4,
      BAST_damp_trk_a1_euro_pilotoff_8PSK_3_4,
      BAST_bw_trk_a1_euro_pilotoff_8PSK_5_6,
      BAST_damp_trk_a1_euro_pilotoff_8PSK_5_6,
      BAST_bw_trk_a1_euro_pilotoff_8PSK_high,
      BAST_damp_trk_a1_euro_pilotoff_8PSK_high
   };

   static const uint8_t * const BAST_trk_awgn_pilotoff[] =
   {
      BAST_bw_trk_a1_awgn_pilotoff_QPSK_low,
      BAST_damp_trk_a1_awgn_pilotoff_QPSK_low,
      BAST_bw_trk_a1_awgn_pilotoff_QPSK_high,
      BAST_damp_trk_a1_awgn_pilotoff_QPSK_high,
      BAST_bw_trk_a1_awgn_pilotoff_8PSK_low,
      BAST_damp_trk_a1_awgn_pilotoff_8PSK_low,
      BAST_bw_trk_a1_awgn_pilotoff_8PSK_3_4,
      BAST_damp_trk_a1_awgn_pilotoff_8PSK_3_4,
      BAST_bw_trk_a1_awgn_pilotoff_8PSK_5_6,
      BAST_damp_trk_a1_awgn_pilotoff_8PSK_5_6,
      BAST_bw_trk_a1_awgn_pilotoff_8PSK_high,
      BAST_damp_trk_a1_awgn_pilotoff_8PSK_high
   };

   static const uint8_t * const BAST_trk_turbo[] =
   {
      BAST_bw_trk_turbo_QPSK_low,
      BAST_damp_trk_turbo_QPSK_low,
      BAST_bw_trk_turbo_QPSK_high,
      BAST_damp_trk_turbo_QPSK_high,
      BAST_bw_trk_turbo_8PSK_low,
      BAST_damp_trk_turbo_8PSK_low,
      BAST_bw_trk_turbo_8PSK_high,
      BAST_damp_trk_turbo_8PSK_high
   };

   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, bw1, bw2, damp1, damp2, bw, damp, P_hi, P_lo, Q_hi, Q_lo;
   const uint16_t *pBwScaleTable;
   const uint8_t * const *pTableSet, *pBwTable, *pDampTable;
   bool bTurbo;
   int32_t offset, i = -1;

   bTurbo = BAST_MODE_IS_TURBO(hChn->actualMode);

   if (bAcq)
   {
      /* acquisition */
      pBwScaleTable = BAST_plc_acq_bw_scale;
      if (hChn->ldpcCtl & BAST_G2_LDPC_CTL_USE_ALT_ACQ_PLC)
      {
         bw = hChn->altPlcAcqBw;
         damp = (hChn->altPlcAcqDamp * 4) / BAST_PLC_DAMP_SCALE;
         goto set_plc;
      }
      else if (BAST_g2_P_LdpcIsPilotOn_isr(h) || bTurbo)
      {
         /* acquisition, LDPC pilot on */
         if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_acq_euro_piloton"));
#endif
            pTableSet = BAST_acq_euro_piloton;
         }
         else
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_acq_default_piloton"));
#endif
            pTableSet = BAST_acq_default_piloton; /* turbo uses this table */
         }
      }
      else
      {
         /* acquisition, LDPC no pilot */
         if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_acq_euro_pilotoff"));
#endif
            pTableSet = BAST_acq_euro_pilotoff;
         }
         else
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_acq_default_pilotoff"));
#endif
            pTableSet = BAST_acq_default_pilotoff;
         }
      }

      if (hChn->actualMode == BAST_Mode_eUnknown)
         offset = 4;
      else if (bTurbo)
      {
         if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_4_5)
            offset = 6;
         else if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_2_3)
            offset = 4;
         else if (hChn->actualMode >= BAST_Mode_eTurbo_Qpsk_3_4)
            offset = 2;
         else
            offset = 0;
      }
      else if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_8_9)
         offset = 6;
      else if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_3_5)
         offset = 4;
      else if (hChn->actualMode >= BAST_Mode_eLdpc_Qpsk_3_4)
         offset = 2;
      else
         offset = 0;
   }
   else if (hChn->ldpcCtl & BAST_G2_LDPC_CTL_USE_ALT_TRK_PLC)
   {
      bw = hChn->altPlcTrkBw;
      damp = (hChn->altPlcTrkDamp * 4) / BAST_PLC_DAMP_SCALE;
      goto set_plc;
   }
   else
   {
      /* tracking */
      pBwScaleTable = BAST_plc_trk_bw_scale;
      if (bTurbo)
      {
         /* turbo */
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_turbo"));
#endif
         pBwScaleTable = BAST_plc_turbo_trk_bw_scale;
         pTableSet = BAST_trk_turbo;
      }
      else if (BAST_g2_P_LdpcIsPilotOn_isr(h))
      {
         /* tracking, LDPC pilot on */
         if (hChn->ldpcCtl & BAST_G2_LDPC_CTL_AWGN_PLC)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_awgn_piloton"));
#endif
            pTableSet = BAST_trk_awgn_piloton;
         }
         else if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_euro_piloton"));
#endif
            pTableSet = BAST_trk_euro_piloton;
         }
         else
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_default_piloton"));
#endif
            if ((hChn->actualMode == BAST_Mode_eLdpc_8psk_3_5) &&
                (hChn->acqParams.symbolRate >= 20000000))
            {
               /* originally, bw=0, damp=0 --> PLC=0x00000100 */
               val = 0x00010001; /* improved QEF and no degradation on CCI */
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC, &val);
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC1, &val);
#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FBLC, &val);
#endif
               return;
            }

            pTableSet = BAST_trk_default_piloton;
         }
      }
      else
      {
         /* tracking, LDPC no pilot */
         if (hChn->ldpcCtl & BAST_G2_LDPC_CTL_AWGN_PLC)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_awgn_pilotoff"));
#endif
            pTableSet = BAST_trk_awgn_pilotoff;
         }
         else if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_euro_pilotoff"));
#endif
            pTableSet = BAST_trk_euro_pilotoff;
         }
         else
         {
#ifdef BAST_DEBUG_PLC
            BDBG_MSG(("using BAST_trk_default_pilotoff"));
#endif
            pTableSet = BAST_trk_default_pilotoff;

#if 0
            /* special case for QPSK 1/2 no pilot optimized for ETSI phase noise */
            if (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2)
            {
               val = 0x0415060F;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC, &val);
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_PLC1, &val);
#if ((BCHP_CHIP!=7325) && (BCHP_CHIP!=7335)) || (BCHP_VER >= BCHP_VER_B0)
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_FBLC, &val);
#endif
               val = 0x30;
               BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQMODE, &val);
            }
#endif
         }
      }

      if (bTurbo)
      {
         if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_4_5)
            offset = 6;
         else if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_2_3)
            offset = 4;
         else if (hChn->actualMode >= BAST_Mode_eTurbo_Qpsk_3_4)
            offset = 2;
         else
            offset = 0;
      }
      else if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_8_9)
         offset = 10;
      else if (hChn->actualMode == BAST_Mode_eLdpc_8psk_5_6)
         offset = 8;
      else if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_4)
         offset = 6;
      else if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_3_5)
         offset = 4;
      else if (hChn->actualMode >= BAST_Mode_eLdpc_Qpsk_3_4)
         offset = 2;
      else
         offset = 0;
   }

   pBwTable = pTableSet[offset];
   pDampTable = pTableSet[offset+1];

   for (i = 0; i < BAST_PLC_NUM_SYMRATES; i++)
   {
      if (hChn->acqParams.symbolRate >= BAST_plc_symbol_rate[i])
         break;
   }

   if (i == 0)
      goto dont_do_interpolation;

   if (i >= BAST_PLC_NUM_SYMRATES)
   {
      i = BAST_PLC_NUM_SYMRATES - 1;

      dont_do_interpolation:
      BAST_g2_P_LdpcLookupPlcBwDamp_isr(pBwTable, pBwScaleTable, pDampTable, i, &bw, &damp);
   }
   else
   {
      /* do linear interpolation between i and i-1 */
      BAST_g2_P_LdpcLookupPlcBwDamp_isr(pBwTable, pBwScaleTable, pDampTable, i, &bw1, &damp1);
      BAST_g2_P_LdpcLookupPlcBwDamp_isr(pBwTable, pBwScaleTable, pDampTable, i-1, &bw2, &damp2);

      if (bw2 >= bw1)
         val = bw2 - bw1;
      else
         val = bw1 - bw2;
      BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate - BAST_plc_symbol_rate[i], val, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, BAST_plc_symbol_rate[i-1] - BAST_plc_symbol_rate[i], &Q_hi, &Q_lo);
      if (bw2 >= bw1)
         bw = Q_lo + bw1;
      else
         bw = bw1 - Q_lo;

      if (damp2 >= damp1)
         val = damp2 - damp1;
      else
         val = damp1 - damp2;
      BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate - BAST_plc_symbol_rate[i], val, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, BAST_plc_symbol_rate[i-1] - BAST_plc_symbol_rate[i], &Q_hi, &Q_lo);
      if (damp2 >= damp1)
         damp = Q_lo + damp1;
      else
         damp = damp1 - Q_lo;
   }

   set_plc:
#ifdef BAST_DEBUG_PLC
   BDBG_MSG(("PLC: bAcq=%d,bw=%d,damp(x4)=%d", bAcq, bw, damp));
#endif
   BAST_g2_P_SetBackCarrierBw_isr(h, bw, damp);
}


/******************************************************************************
 BAST_g2_P_LdpcLookupPlcBwDamp_isr()
******************************************************************************/
void BAST_g2_P_LdpcLookupPlcBwDamp_isr(const uint8_t *pBwTable, const uint16_t *pBwScaleTable, const uint8_t *pDampTable, int32_t i, uint32_t *pBw, uint32_t *pDamp)
{
   *pBw = (uint32_t)(pBwTable[i] * pBwScaleTable[i]);
   *pDamp = (uint32_t)((pDampTable[i] * 8) / BAST_PLC_DAMP_SCALE);
   *pDamp = *pDamp + 1;
   *pDamp = *pDamp >> 1;
}
