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
* Module Description: This loads a given RTS for given chip and chip revision
*                     without reloading the CFE.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "framework.h"

#include "framework_board_bvdc.h"

#include "bchp_memc_arb_0.h"
#if BCHP_MEMC_ARB_1_REG_START
#include "bchp_memc_arb_1.h"
#endif
#if BCHP_MEMC_ARB_2_REG_START
#include "bchp_memc_arb_2.h"
#endif

BDBG_MODULE(LOAD_RTS);

#if (BCHP_CHIP==7439) || (BCHP_CHIP==7252) || (BCHP_CHIP==7445)
#define BTST_LOAD_RTS_RTS_ARRAY_SIZE  256
#else
#define BTST_LOAD_RTS_RTS_ARRAY_SIZE  128
#endif

static uint32_t aulSingleEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x0031e009,  /*0*/
   0x8031002d,  /*1*/
   0x8037c00b,  /*2*/
   0x80ad2036,  /*3*/
   0x803bf00f,  /*4*/
   0x81b9201d,  /*5*/
   0x8037c00c,  /*6*/
   0x80834035,  /*7*/
   0x8016403d,  /*8*/
   0x8018f029,  /*9*/
   0x80707033,  /*10*/
   0x804c1030,  /*11*/
   0x0024f006,  /*12*/
   0x8016403e,  /*13*/
   0x8018f02a,  /*14*/
   0x806e3032,  /*15*/
   0x804c1031,  /*16*/
   0x0024f007,  /*17*/
   0x801a8002,  /*18*/
   0x80579014,  /*19*/
   0x80646015,  /*20*/
   0x800ab03a,  /*21*/
   0x83618023,  /*22*/
   0x800b103b,  /*23*/
   0x800b103c,  /*24*/
   0x07fff04f,  /*25*/
   0x80828034,  /*26*/
   0x87fff04b,  /*27*/
   0x80186028,  /*28*/
   0x84429025,  /*29*/
   0x07fff050,  /*30*/
   0x811db038,  /*31*/
   0x07fff051,  /*32*/
   0x02807021,  /*33*/
   0x02807020,  /*34*/
   0x87fff049,  /*35*/
   0x87fff048,  /*36*/
   0x802d3008,  /*37*/
   0x80b2b037,  /*38*/
   0x07fff052,  /*39*/
   0x8046c013,  /*40*/
   0x8181a039,  /*41*/
   0x83957024,  /*42*/
   0x802f102b,  /*43*/
   0x87fff044,  /*44*/
   0x802f102c,  /*45*/
   0x87fff045,  /*46*/
   0x07fff053,  /*47*/
   0x07fff054,  /*48*/
   0x8033d02e,  /*49*/
   0x009e4019,  /*50*/
   0x80113001,  /*51*/
   0x07fff07f,  /*52*/
   0x8033d00a,  /*53*/
   0x07fff07f,  /*54*/
   0x07fff07f,  /*55*/
   0x003cd011,  /*56*/
   0x003cd012,  /*57*/
   0x003cd010,  /*58*/
   0x001e5003,  /*59*/
   0x07fff07f,  /*60*/
   0x07fff07f,  /*61*/
   0x07fff07f,  /*62*/
   0x07fff07f,  /*63*/
   0x00a0001a,  /*64*/
   0x07fff07f,  /*65*/
   0x001fb004,  /*66*/
   0x07fff07f,  /*67*/
   0x07fff07f,  /*68*/
   0x016e801c,  /*69*/
   0x07fff07f,  /*70*/
   0x001fe005,  /*71*/
   0x003af00d,  /*72*/
   0x003af00e,  /*73*/
   0x07fff07f,  /*74*/
   0x07fff07f,  /*75*/
   0x07fff07f,  /*76*/
   0x07fff07f,  /*77*/
   0x07fff07f,  /*78*/
   0x00999018,  /*79*/
   0x000fc000,  /*80*/
   0x07fff055,  /*81*/
   0x0221301f,  /*82*/
   0x0221301e,  /*83*/
   0x00803016,  /*84*/
   0x07fff056,  /*85*/
   0x07fff057,  /*86*/
   0x8037902f,  /*87*/
   0x87fff04d,  /*88*/
   0x87fff04c,  /*89*/
   0x87fff046,  /*90*/
   0x07fff058,  /*91*/
   0x07fff059,  /*92*/
   0x07fff05a,  /*93*/
   0x87fff04a,  /*94*/
   0x07fff07f,  /*95*/
   0x07fff07f,  /*96*/
   0x07fff07f,  /*97*/
   0x07fff07f,  /*98*/
   0x07fff07f,  /*99*/
   0x07fff07f,  /*100*/
   0x07fff07f,  /*101*/
   0x07fff07f,  /*102*/
   0x80feb01b,  /*103*/
   0x07fff07f,  /*104*/
   0x849da026,  /*105*/
   0x82e26022,  /*106*/
   0x07fff07f,  /*107*/
   0x07fff07f,  /*108*/
   0x07fff07f,  /*109*/
   0x07fff07f,  /*110*/
   0x07fff05b,  /*111*/
   0x07fff05c,  /*112*/
   0x07fff05d,  /*113*/
   0x07fff05e,  /*114*/
   0x07fff05f,  /*115*/
   0x07fff07f,  /*116*/
   0x07fff060,  /*117*/
   0x87fff047,  /*118*/
   0x87fff04e,  /*119*/
   0x8000003f,  /*120*/
   0x80000040,  /*121*/
   0x80000041,  /*122*/
   0x07fff07f,  /*123*/
   0x800c2027,  /*124*/
   0x80000042,  /*125*/
   0x87fff043,  /*126*/
   0x00872017   /*127*/
};

#if BCHP_MEMC_ARB_1_REG_START
static uint32_t aulSingleEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x07fff07f,  /*0*/
   0x07fff07f,  /*1*/
   0x07fff07f,  /*2*/
   0x07fff07f,  /*3*/
   0x07fff07f,  /*4*/
   0x07fff07f,  /*5*/
   0x07fff07f,  /*6*/
   0x07fff07f,  /*7*/
   0x80164028,  /*8*/
   0x07fff07f,  /*9*/
   0x07fff07f,  /*10*/
   0x07fff07f,  /*11*/
   0x07fff07f,  /*12*/
   0x80164029,  /*13*/
   0x07fff07f,  /*14*/
   0x07fff07f,  /*15*/
   0x07fff07f,  /*16*/
   0x07fff07f,  /*17*/
   0x07fff07f,  /*18*/
   0x07fff07f,  /*19*/
   0x07fff07f,  /*20*/
   0x800ab025,  /*21*/
   0x83618018,  /*22*/
   0x800b1026,  /*23*/
   0x800b1027,  /*24*/
   0x07fff042,  /*25*/
   0x8082801f,  /*26*/
   0x07fff07f,  /*27*/
   0x80186023,  /*28*/
   0x8442901b,  /*29*/
   0x07fff043,  /*30*/
   0x811db021,  /*31*/
   0x07fff044,  /*32*/
   0x07fff07f,  /*33*/
   0x07fff07f,  /*34*/
   0x87fff03d,  /*35*/
   0x87fff03c,  /*36*/
   0x802d3008,  /*37*/
   0x80b2b020,  /*38*/
   0x07fff045,  /*39*/
   0x8046c010,  /*40*/
   0x8181a022,  /*41*/
   0x83957019,  /*42*/
   0x802f101c,  /*43*/
   0x87fff037,  /*44*/
   0x802f101d,  /*45*/
   0x87fff038,  /*46*/
   0x07fff046,  /*47*/
   0x07fff047,  /*48*/
   0x07fff07f,  /*49*/
   0x07fff07f,  /*50*/
   0x07fff07f,  /*51*/
   0x87fff035,  /*52*/
   0x07fff07f,  /*53*/
   0x87fff034,  /*54*/
   0x87fff033,  /*55*/
   0x07fff07f,  /*56*/
   0x07fff07f,  /*57*/
   0x07fff07f,  /*58*/
   0x07fff07f,  /*59*/
   0x003cd00a,  /*60*/
   0x001e5001,  /*61*/
   0x001fe005,  /*62*/
   0x00a00014,  /*63*/
   0x07fff07f,  /*64*/
   0x001fb004,  /*65*/
   0x07fff07f,  /*66*/
   0x001e5002,  /*67*/
   0x016e8017,  /*68*/
   0x07fff07f,  /*69*/
   0x001fe006,  /*70*/
   0x07fff07f,  /*71*/
   0x07fff07f,  /*72*/
   0x07fff07f,  /*73*/
   0x00395009,  /*74*/
   0x001c9000,  /*75*/
   0x00288007,  /*76*/
   0x007f4011,  /*77*/
   0x00983013,  /*78*/
   0x07fff07f,  /*79*/
   0x07fff07f,  /*80*/
   0x07fff048,  /*81*/
   0x07fff07f,  /*82*/
   0x07fff07f,  /*83*/
   0x07fff07f,  /*84*/
   0x07fff049,  /*85*/
   0x07fff04a,  /*86*/
   0x8037901e,  /*87*/
   0x87fff040,  /*88*/
   0x87fff03f,  /*89*/
   0x87fff03a,  /*90*/
   0x07fff04b,  /*91*/
   0x07fff04c,  /*92*/
   0x07fff04d,  /*93*/
   0x87fff03e,  /*94*/
   0x803fc00c,  /*95*/
   0x803fc00d,  /*96*/
   0x803fc00e,  /*97*/
   0x8041a00f,  /*98*/
   0x840ea01a,  /*99*/
   0x813d8016,  /*100*/
   0x801f8003,  /*101*/
   0x803f600b,  /*102*/
   0x07fff07f,  /*103*/
   0x8000002a,  /*104*/
   0x07fff07f,  /*105*/
   0x07fff07f,  /*106*/
   0x80b07015,  /*107*/
   0x87fff030,  /*108*/
   0x87fff031,  /*109*/
   0x87fff032,  /*110*/
   0x07fff04e,  /*111*/
   0x07fff04f,  /*112*/
   0x07fff050,  /*113*/
   0x07fff051,  /*114*/
   0x07fff052,  /*115*/
   0x87fff039,  /*116*/
   0x07fff053,  /*117*/
   0x87fff03b,  /*118*/
   0x87fff041,  /*119*/
   0x8000002c,  /*120*/
   0x8000002d,  /*121*/
   0x8000002e,  /*122*/
   0x8000002b,  /*123*/
   0x800c2024,  /*124*/
   0x8000002f,  /*125*/
   0x87fff036,  /*126*/
   0x00872012   /*127*/
};
#endif

static uint32_t aulDualEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x0031e009,  /*0*/
   0x8031002d,  /*1*/
   0x8037c00b,  /*2*/
   0x80ad2036,  /*3*/
   0x803bf00f,  /*4*/
   0x81b9201d,  /*5*/
   0x8037c00c,  /*6*/
   0x80834035,  /*7*/
   0x8016403d,  /*8*/
   0x8018f029,  /*9*/
   0x80707033,  /*10*/
   0x804c1030,  /*11*/
   0x0024f006,  /*12*/
   0x8016403e,  /*13*/
   0x8018f02a,  /*14*/
   0x806e3032,  /*15*/
   0x804c1031,  /*16*/
   0x0024f007,  /*17*/
   0x801a8002,  /*18*/
   0x80579014,  /*19*/
   0x80646015,  /*20*/
   0x800ab03a,  /*21*/
   0x83618023,  /*22*/
   0x800b103b,  /*23*/
   0x800b103c,  /*24*/
   0x07fff04f,  /*25*/
   0x80828034,  /*26*/
   0x87fff04b,  /*27*/
   0x80186028,  /*28*/
   0x84429025,  /*29*/
   0x07fff050,  /*30*/
   0x811db038,  /*31*/
   0x07fff051,  /*32*/
   0x02807021,  /*33*/
   0x02807020,  /*34*/
   0x87fff049,  /*35*/
   0x87fff048,  /*36*/
   0x802d3008,  /*37*/
   0x80b2b037,  /*38*/
   0x07fff052,  /*39*/
   0x8046c013,  /*40*/
   0x8181a039,  /*41*/
   0x83957024,  /*42*/
   0x802f102b,  /*43*/
   0x87fff044,  /*44*/
   0x802f102c,  /*45*/
   0x87fff045,  /*46*/
   0x07fff053,  /*47*/
   0x07fff054,  /*48*/
   0x8033d02e,  /*49*/
   0x009e4019,  /*50*/
   0x80113001,  /*51*/
   0x07fff07f,  /*52*/
   0x8033d00a,  /*53*/
   0x07fff07f,  /*54*/
   0x07fff07f,  /*55*/
   0x003cd011,  /*56*/
   0x003cd012,  /*57*/
   0x003cd010,  /*58*/
   0x001e5003,  /*59*/
   0x07fff07f,  /*60*/
   0x07fff07f,  /*61*/
   0x07fff07f,  /*62*/
   0x07fff07f,  /*63*/
   0x00a0001a,  /*64*/
   0x07fff07f,  /*65*/
   0x001fb004,  /*66*/
   0x07fff07f,  /*67*/
   0x07fff07f,  /*68*/
   0x016e801c,  /*69*/
   0x07fff07f,  /*70*/
   0x001fe005,  /*71*/
   0x003af00d,  /*72*/
   0x003af00e,  /*73*/
   0x07fff07f,  /*74*/
   0x07fff07f,  /*75*/
   0x07fff07f,  /*76*/
   0x07fff07f,  /*77*/
   0x07fff07f,  /*78*/
   0x00999018,  /*79*/
   0x000fc000,  /*80*/
   0x07fff055,  /*81*/
   0x0221301f,  /*82*/
   0x0221301e,  /*83*/
   0x00803016,  /*84*/
   0x07fff056,  /*85*/
   0x07fff057,  /*86*/
   0x8037902f,  /*87*/
   0x87fff04d,  /*88*/
   0x87fff04c,  /*89*/
   0x87fff046,  /*90*/
   0x07fff058,  /*91*/
   0x07fff059,  /*92*/
   0x07fff05a,  /*93*/
   0x87fff04a,  /*94*/
   0x07fff07f,  /*95*/
   0x07fff07f,  /*96*/
   0x07fff07f,  /*97*/
   0x07fff07f,  /*98*/
   0x07fff07f,  /*99*/
   0x07fff07f,  /*100*/
   0x07fff07f,  /*101*/
   0x07fff07f,  /*102*/
   0x80feb01b,  /*103*/
   0x07fff07f,  /*104*/
   0x849da026,  /*105*/
   0x82e26022,  /*106*/
   0x07fff07f,  /*107*/
   0x07fff07f,  /*108*/
   0x07fff07f,  /*109*/
   0x07fff07f,  /*110*/
   0x07fff05b,  /*111*/
   0x07fff05c,  /*112*/
   0x07fff05d,  /*113*/
   0x07fff05e,  /*114*/
   0x07fff05f,  /*115*/
   0x07fff07f,  /*116*/
   0x07fff060,  /*117*/
   0x87fff047,  /*118*/
   0x87fff04e,  /*119*/
   0x8000003f,  /*120*/
   0x80000040,  /*121*/
   0x80000041,  /*122*/
   0x07fff07f,  /*123*/
   0x800c2027,  /*124*/
   0x80000042,  /*125*/
   0x87fff043,  /*126*/
   0x00872017   /*127*/
};

#if BCHP_MEMC_ARB_1_REG_START
static uint32_t aulDualEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x07fff07f,  /*0*/
   0x07fff07f,  /*1*/
   0x07fff07f,  /*2*/
   0x07fff07f,  /*3*/
   0x07fff07f,  /*4*/
   0x07fff07f,  /*5*/
   0x07fff07f,  /*6*/
   0x07fff07f,  /*7*/
   0x8016402e,  /*8*/
   0x07fff07f,  /*9*/
   0x07fff07f,  /*10*/
   0x07fff07f,  /*11*/
   0x07fff07f,  /*12*/
   0x8016402f,  /*13*/
   0x07fff07f,  /*14*/
   0x07fff07f,  /*15*/
   0x07fff07f,  /*16*/
   0x07fff07f,  /*17*/
   0x07fff07f,  /*18*/
   0x07fff07f,  /*19*/
   0x07fff07f,  /*20*/
   0x800ab02b,  /*21*/
   0x8361801e,  /*22*/
   0x800b102c,  /*23*/
   0x800b102d,  /*24*/
   0x07fff042,  /*25*/
   0x80828025,  /*26*/
   0x07fff07f,  /*27*/
   0x80186029,  /*28*/
   0x84429021,  /*29*/
   0x07fff043,  /*30*/
   0x811db027,  /*31*/
   0x07fff044,  /*32*/
   0x07fff07f,  /*33*/
   0x07fff07f,  /*34*/
   0x87fff03d,  /*35*/
   0x87fff03c,  /*36*/
   0x802d3002,  /*37*/
   0x80b2b026,  /*38*/
   0x07fff045,  /*39*/
   0x8046c00a,  /*40*/
   0x8181a028,  /*41*/
   0x8395701f,  /*42*/
   0x802f1022,  /*43*/
   0x87fff037,  /*44*/
   0x802f1023,  /*45*/
   0x87fff038,  /*46*/
   0x07fff046,  /*47*/
   0x07fff047,  /*48*/
   0x07fff07f,  /*49*/
   0x07fff07f,  /*50*/
   0x07fff07f,  /*51*/
   0x0123a01a,  /*52*/
   0x07fff07f,  /*53*/
   0x0091c014,  /*54*/
   0x00cd0018,  /*55*/
   0x07fff07f,  /*56*/
   0x07fff07f,  /*57*/
   0x07fff07f,  /*58*/
   0x07fff07f,  /*59*/
   0x003cd003,  /*60*/
   0x001e5000,  /*61*/
   0x003fe008,  /*62*/
   0x00a00016,  /*63*/
   0x07fff07f,  /*64*/
   0x003fe007,  /*65*/
   0x07fff07f,  /*66*/
   0x003cf005,  /*67*/
   0x016e801d,  /*68*/
   0x07fff07f,  /*69*/
   0x003cf004,  /*70*/
   0x07fff07f,  /*71*/
   0x07fff07f,  /*72*/
   0x07fff07f,  /*73*/
   0x0123a01b,  /*74*/
   0x0091c015,  /*75*/
   0x00cd0019,  /*76*/
   0x007ff012,  /*77*/
   0x007ff011,  /*78*/
   0x07fff07f,  /*79*/
   0x07fff07f,  /*80*/
   0x07fff048,  /*81*/
   0x07fff07f,  /*82*/
   0x07fff07f,  /*83*/
   0x07fff07f,  /*84*/
   0x07fff049,  /*85*/
   0x07fff04a,  /*86*/
   0x80379024,  /*87*/
   0x87fff040,  /*88*/
   0x87fff03f,  /*89*/
   0x87fff03a,  /*90*/
   0x07fff04b,  /*91*/
   0x07fff04c,  /*92*/
   0x07fff04d,  /*93*/
   0x87fff03e,  /*94*/
   0x807ff00e,  /*95*/
   0x807ff00f,  /*96*/
   0x807ff010,  /*97*/
   0x8041a009,  /*98*/
   0x840ea020,  /*99*/
   0x813d801c,  /*100*/
   0x801f8001,  /*101*/
   0x803f6006,  /*102*/
   0x07fff07f,  /*103*/
   0x80000030,  /*104*/
   0x07fff07f,  /*105*/
   0x07fff07f,  /*106*/
   0x80b07017,  /*107*/
   0x807ff00b,  /*108*/
   0x807ff00c,  /*109*/
   0x807ff00d,  /*110*/
   0x07fff04e,  /*111*/
   0x07fff04f,  /*112*/
   0x07fff050,  /*113*/
   0x07fff051,  /*114*/
   0x07fff052,  /*115*/
   0x87fff039,  /*116*/
   0x07fff053,  /*117*/
   0x87fff03b,  /*118*/
   0x87fff041,  /*119*/
   0x80000032,  /*120*/
   0x80000033,  /*121*/
   0x80000034,  /*122*/
   0x80000031,  /*123*/
   0x800c202a,  /*124*/
   0x80000035,  /*125*/
   0x87fff036,  /*126*/
   0x00872013   /*127*/
};
#endif

static uint32_t aul7422Usage0_SingleEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x0027d00d,  /*0*/
   0x802f200f,  /*1*/
   0x80316010,  /*2*/
   0x80abf022,  /*3*/
   0x8064701a,  /*4*/
   0x8515c02d,  /*5*/
   0x80316011,  /*6*/
   0x805a9018,  /*7*/
   0x8016403e,  /*8*/
   0x8017802f,  /*9*/
   0x8038a035,  /*10*/
   0x0050a016,  /*11*/
   0x0024f00b,  /*12*/
   0x8016403f,  /*13*/
   0x80178030,  /*14*/
   0x8038a036,  /*15*/
   0x0050a017,  /*16*/
   0x0024f00c,  /*17*/
   0x8016d001,  /*18*/
   0x808c201d,  /*19*/
   0x8060a019,  /*20*/
   0x800ab03b,  /*21*/
   0x8361802a,  /*22*/
   0x800b103c,  /*23*/
   0x800b103d,  /*24*/
   0x07fff03f,  /*25*/
   0x80828037,  /*26*/
   0x87fff04d,  /*27*/
   0x80186031,  /*28*/
   0x8442902c,  /*29*/
   0x07fff03f,  /*30*/
   0x811db039,  /*31*/
   0x07fff03f,  /*32*/
   0x02807029,  /*33*/
   0x02807028,  /*34*/
   0x87fff04b,  /*35*/
   0x87fff04a,  /*36*/
   0x802d300e,  /*37*/
   0x80b2b038,  /*38*/
   0x07fff03f,  /*39*/
   0x8046c015,  /*40*/
   0x8181a03a,  /*41*/
   0x8395702b,  /*42*/
   0x802f1032,  /*43*/
   0x87fff045,  /*44*/
   0x802f1033,  /*45*/
   0x87fff046,  /*46*/
   0x07fff03f,  /*47*/
   0x07fff03f,  /*48*/
   0x0023000a,  /*49*/
   0x018c5025,  /*50*/
   0x87fff047,  /*51*/
   0x07fff03f,  /*52*/
   0x0033d012,  /*53*/
   0x07fff03f,  /*54*/
   0x07fff03f,  /*55*/
   0x07fff03f,  /*56*/
   0x07fff03f,  /*57*/
   0x003cd014,  /*58*/
   0x001e5003,  /*59*/
   0x07fff013,  /*60*/
   0x07fff002,  /*61*/
   0x07fff03f,  /*62*/
   0x07fff021,  /*63*/
   0x07fff020,  /*64*/
   0x07fff005,  /*65*/
   0x07fff004,  /*66*/
   0x07fff03f,  /*67*/
   0x07fff024,  /*68*/
   0x07fff023,  /*69*/
   0x07fff007,  /*70*/
   0x07fff006,  /*71*/
   0x07fff008,  /*72*/
   0x07fff009,  /*73*/
   0x07fff03f,  /*74*/
   0x07fff03f,  /*75*/
   0x07fff03f,  /*76*/
   0x07fff03f,  /*77*/
   0x07fff01e,  /*78*/
   0x0099901f,  /*79*/
   0x000fc000,  /*80*/
   0x07fff03f,  /*81*/
   0x02213027,  /*82*/
   0x02213026,  /*83*/
   0x07fff01c,  /*84*/
   0x07fff03f,  /*85*/
   0x07fff03f,  /*86*/
   0x80379034,  /*87*/
   0x87fff04f,  /*88*/
   0x87fff04e,  /*89*/
   0x87fff048,  /*90*/
   0x07fff03f,  /*91*/
   0x07fff03f,  /*92*/
   0x07fff03f,  /*93*/
   0x87fff04c,  /*94*/
   0x07fff03f,  /*95*/
   0x07fff03f,  /*96*/
   0x07fff03f,  /*97*/
   0x07fff03f,  /*98*/
   0x07fff03f,  /*99*/
   0x07fff03f,  /*100*/
   0x07fff03f,  /*101*/
   0x07fff03f,  /*102*/
   0x07fff03f,  /*103*/
   0x07fff03f,  /*104*/
   0x07fff03f,  /*105*/
   0x07fff03f,  /*106*/
   0x07fff03f,  /*107*/
   0x07fff03f,  /*108*/
   0x07fff03f,  /*109*/
   0x07fff03f,  /*110*/
   0x07fff03f,  /*111*/
   0x07fff03f,  /*112*/
   0x07fff03f,  /*113*/
   0x07fff03f,  /*114*/
   0x07fff03f,  /*115*/
   0x07fff03f,  /*116*/
   0x07fff03f,  /*117*/
   0x87fff049,  /*118*/
   0x87fff050,  /*119*/
   0x80000040,  /*120*/
   0x80000041,  /*121*/
   0x80000042,  /*122*/
   0x07fff03f,  /*123*/
   0x800c202e,  /*124*/
   0x80000043,  /*125*/
   0x87fff044,  /*126*/
   0x0087201b   /*127*/
};

#if BCHP_MEMC_ARB_1_REG_START
static uint32_t aul7422Usage0_SingleEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x07fff00d,  /*0*/
   0x07fff00f,  /*1*/
   0x07fff010,  /*2*/
   0x07fff022,  /*3*/
   0x07fff01a,  /*4*/
   0x07fff02d,  /*5*/
   0x07fff011,  /*6*/
   0x07fff018,  /*7*/
   0x8016403e,  /*8*/
   0x07fff02f,  /*9*/
   0x07fff035,  /*10*/
   0x07fff016,  /*11*/
   0x07fff00b,  /*12*/
   0x8016403f,  /*13*/
   0x07fff030,  /*14*/
   0x07fff036,  /*15*/
   0x07fff017,  /*16*/
   0x07fff00c,  /*17*/
   0x07fff001,  /*18*/
   0x07fff01d,  /*19*/
   0x07fff019,  /*20*/
   0x800ab03b,  /*21*/
   0x8361802a,  /*22*/
   0x800b103c,  /*23*/
   0x800b103d,  /*24*/
   0x07fff03f,  /*25*/
   0x80828037,  /*26*/
   0x07fff04d,  /*27*/
   0x80186031,  /*28*/
   0x8442902c,  /*29*/
   0x07fff03f,  /*30*/
   0x811db039,  /*31*/
   0x07fff03f,  /*32*/
   0x07fff029,  /*33*/
   0x07fff028,  /*34*/
   0x87fff04b,  /*35*/
   0x87fff04a,  /*36*/
   0x802d300e,  /*37*/
   0x80b2b038,  /*38*/
   0x07fff03f,  /*39*/
   0x8046c015,  /*40*/
   0x8181a03a,  /*41*/
   0x8395702b,  /*42*/
   0x802f1032,  /*43*/
   0x87fff045,  /*44*/
   0x802f1033,  /*45*/
   0x87fff046,  /*46*/
   0x07fff03f,  /*47*/
   0x07fff03f,  /*48*/
   0x07fff00a,  /*49*/
   0x07fff025,  /*50*/
   0x07fff047,  /*51*/
   0x07fff03f,  /*52*/
   0x0033d012,  /*53*/
   0x07fff03f,  /*54*/
   0x07fff03f,  /*55*/
   0x07fff03f,  /*56*/
   0x07fff03f,  /*57*/
   0x07fff014,  /*58*/
   0x07fff003,  /*59*/
   0x003cd013,  /*60*/
   0x001e5002,  /*61*/
   0x07fff03f,  /*62*/
   0x00a00021,  /*63*/
   0x00a00020,  /*64*/
   0x001fe005,  /*65*/
   0x001fe004,  /*66*/
   0x07fff03f,  /*67*/
   0x016e8024,  /*68*/
   0x016e8023,  /*69*/
   0x001fe007,  /*70*/
   0x001fe006,  /*71*/
   0x00221008,  /*72*/
   0x00221009,  /*73*/
   0x07fff03f,  /*74*/
   0x07fff03f,  /*75*/
   0x07fff03f,  /*76*/
   0x07fff03f,  /*77*/
   0x0098301e,  /*78*/
   0x07fff01f,  /*79*/
   0x07fff000,  /*80*/
   0x07fff03f,  /*81*/
   0x07fff027,  /*82*/
   0x07fff026,  /*83*/
   0x0088a01c,  /*84*/
   0x07fff03f,  /*85*/
   0x07fff03f,  /*86*/
   0x80379034,  /*87*/
   0x87fff04f,  /*88*/
   0x87fff04e,  /*89*/
   0x87fff048,  /*90*/
   0x07fff03f,  /*91*/
   0x07fff03f,  /*92*/
   0x07fff03f,  /*93*/
   0x87fff04c,  /*94*/
   0x07fff03f,  /*95*/
   0x07fff03f,  /*96*/
   0x07fff03f,  /*97*/
   0x07fff03f,  /*98*/
   0x07fff03f,  /*99*/
   0x07fff03f,  /*100*/
   0x07fff03f,  /*101*/
   0x07fff03f,  /*102*/
   0x07fff03f,  /*103*/
   0x07fff03f,  /*104*/
   0x07fff03f,  /*105*/
   0x07fff03f,  /*106*/
   0x07fff03f,  /*107*/
   0x07fff03f,  /*108*/
   0x07fff03f,  /*109*/
   0x07fff03f,  /*110*/
   0x07fff03f,  /*111*/
   0x07fff03f,  /*112*/
   0x07fff03f,  /*113*/
   0x07fff03f,  /*114*/
   0x07fff03f,  /*115*/
   0x07fff03f,  /*116*/
   0x07fff03f,  /*117*/
   0x87fff049,  /*118*/
   0x87fff050,  /*119*/
   0x80000040,  /*120*/
   0x80000041,  /*121*/
   0x80000042,  /*122*/
   0x07fff03f,  /*123*/
   0x800c202e,  /*124*/
   0x80000043,  /*125*/
   0x87fff044,  /*126*/
   0x0087201b   /*127*/
};
#endif

static uint32_t aul7422Usage1_SingleEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x0027d00d,  /*0*/
   0x802f200f,  /*1*/
   0x80316010,  /*2*/
   0x80abf022,  /*3*/
   0x8064701a,  /*4*/
   0x8515c02d,  /*5*/
   0x80316011,  /*6*/
   0x805a9018,  /*7*/
   0x8016403e,  /*8*/
   0x8017802f,  /*9*/
   0x8038a035,  /*10*/
   0x0050a016,  /*11*/
   0x0024f00b,  /*12*/
   0x8016403f,  /*13*/
   0x80178030,  /*14*/
   0x8038a036,  /*15*/
   0x0050a017,  /*16*/
   0x0024f00c,  /*17*/
   0x8016d001,  /*18*/
   0x808c201d,  /*19*/
   0x8060a019,  /*20*/
   0x800ab03b,  /*21*/
   0x8361802a,  /*22*/
   0x800b103c,  /*23*/
   0x800b103d,  /*24*/
   0x07fff03f,  /*25*/
   0x80828037,  /*26*/
   0x87fff04d,  /*27*/
   0x80186031,  /*28*/
   0x8442902c,  /*29*/
   0x07fff03f,  /*30*/
   0x811db039,  /*31*/
   0x07fff03f,  /*32*/
   0x02807029,  /*33*/
   0x02807028,  /*34*/
   0x87fff04b,  /*35*/
   0x87fff04a,  /*36*/
   0x802d300e,  /*37*/
   0x80b2b038,  /*38*/
   0x07fff03f,  /*39*/
   0x8046c015,  /*40*/
   0x8181a03a,  /*41*/
   0x8395702b,  /*42*/
   0x802f1032,  /*43*/
   0x87fff045,  /*44*/
   0x802f1033,  /*45*/
   0x87fff046,  /*46*/
   0x07fff03f,  /*47*/
   0x07fff03f,  /*48*/
   0x0023000a,  /*49*/
   0x018c5025,  /*50*/
   0x87fff047,  /*51*/
   0x07fff03f,  /*52*/
   0x0033d012,  /*53*/
   0x07fff03f,  /*54*/
   0x07fff03f,  /*55*/
   0x07fff03f,  /*56*/
   0x07fff03f,  /*57*/
   0x003cd014,  /*58*/
   0x001e5003,  /*59*/
   0x07fff013,  /*60*/
   0x07fff002,  /*61*/
   0x07fff03f,  /*62*/
   0x07fff021,  /*63*/
   0x07fff020,  /*64*/
   0x07fff005,  /*65*/
   0x001fe004,  /*66*/
   0x07fff03f,  /*67*/
   0x07fff024,  /*68*/
   0x07fff023,  /*69*/
   0x07fff007,  /*70*/
   0x001fe006,  /*71*/
   0x00221008,  /*72*/
   0x00221009,  /*73*/
   0x07fff03f,  /*74*/
   0x07fff03f,  /*75*/
   0x07fff03f,  /*76*/
   0x07fff03f,  /*77*/
   0x07fff01e,  /*78*/
   0x07fff01f,  /*79*/
   0x07fff000,  /*80*/
   0x07fff03f,  /*81*/
   0x02213027,  /*82*/
   0x02213026,  /*83*/
   0x0088a01c,  /*84*/
   0x07fff03f,  /*85*/
   0x07fff03f,  /*86*/
   0x80379034,  /*87*/
   0x87fff04f,  /*88*/
   0x87fff04e,  /*89*/
   0x87fff048,  /*90*/
   0x07fff03f,  /*91*/
   0x07fff03f,  /*92*/
   0x07fff03f,  /*93*/
   0x87fff04c,  /*94*/
   0x07fff03f,  /*95*/
   0x07fff03f,  /*96*/
   0x07fff03f,  /*97*/
   0x07fff03f,  /*98*/
   0x07fff03f,  /*99*/
   0x07fff03f,  /*100*/
   0x07fff03f,  /*101*/
   0x07fff03f,  /*102*/
   0x07fff03f,  /*103*/
   0x07fff03f,  /*104*/
   0x07fff03f,  /*105*/
   0x07fff03f,  /*106*/
   0x07fff03f,  /*107*/
   0x07fff03f,  /*108*/
   0x07fff03f,  /*109*/
   0x07fff03f,  /*110*/
   0x07fff03f,  /*111*/
   0x07fff03f,  /*112*/
   0x07fff03f,  /*113*/
   0x07fff03f,  /*114*/
   0x07fff03f,  /*115*/
   0x07fff03f,  /*116*/
   0x07fff03f,  /*117*/
   0x87fff049,  /*118*/
   0x87fff050,  /*119*/
   0x80000040,  /*120*/
   0x80000041,  /*121*/
   0x80000042,  /*122*/
   0x07fff03f,  /*123*/
   0x800c202e,  /*124*/
   0x80000043,  /*125*/
   0x87fff044,  /*126*/
   0x0087201b   /*127*/
};

#if BCHP_MEMC_ARB_1_REG_START
static uint32_t aul7422Usage1_SingleEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
   0x07fff00d,  /*0*/
   0x07fff00f,  /*1*/
   0x07fff010,  /*2*/
   0x07fff022,  /*3*/
   0x07fff01a,  /*4*/
   0x07fff02d,  /*5*/
   0x07fff011,  /*6*/
   0x07fff018,  /*7*/
   0x8016403e,  /*8*/
   0x07fff02f,  /*9*/
   0x07fff035,  /*10*/
   0x07fff016,  /*11*/
   0x07fff00b,  /*12*/
   0x8016403f,  /*13*/
   0x07fff030,  /*14*/
   0x07fff036,  /*15*/
   0x07fff017,  /*16*/
   0x07fff00c,  /*17*/
   0x07fff001,  /*18*/
   0x07fff01d,  /*19*/
   0x07fff019,  /*20*/
   0x800ab03b,  /*21*/
   0x8361802a,  /*22*/
   0x800b103c,  /*23*/
   0x800b103d,  /*24*/
   0x07fff03f,  /*25*/
   0x80828037,  /*26*/
   0x07fff04d,  /*27*/
   0x80186031,  /*28*/
   0x8442902c,  /*29*/
   0x07fff03f,  /*30*/
   0x811db039,  /*31*/
   0x07fff03f,  /*32*/
   0x07fff029,  /*33*/
   0x07fff028,  /*34*/
   0x87fff04b,  /*35*/
   0x87fff04a,  /*36*/
   0x802d300e,  /*37*/
   0x80b2b038,  /*38*/
   0x07fff03f,  /*39*/
   0x8046c015,  /*40*/
   0x8181a03a,  /*41*/
   0x8395702b,  /*42*/
   0x802f1032,  /*43*/
   0x87fff045,  /*44*/
   0x802f1033,  /*45*/
   0x87fff046,  /*46*/
   0x07fff03f,  /*47*/
   0x07fff03f,  /*48*/
   0x07fff00a,  /*49*/
   0x07fff025,  /*50*/
   0x07fff047,  /*51*/
   0x07fff03f,  /*52*/
   0x0033d012,  /*53*/
   0x07fff03f,  /*54*/
   0x07fff03f,  /*55*/
   0x07fff03f,  /*56*/
   0x07fff03f,  /*57*/
   0x07fff014,  /*58*/
   0x07fff003,  /*59*/
   0x003cd013,  /*60*/
   0x001e5002,  /*61*/
   0x07fff03f,  /*62*/
   0x00a00021,  /*63*/
   0x00a00020,  /*64*/
   0x001fe005,  /*65*/
   0x07fff004,  /*66*/
   0x07fff03f,  /*67*/
   0x016e8024,  /*68*/
   0x016e8023,  /*69*/
   0x001fe007,  /*70*/
   0x07fff006,  /*71*/
   0x07fff008,  /*72*/
   0x07fff009,  /*73*/
   0x07fff03f,  /*74*/
   0x07fff03f,  /*75*/
   0x07fff03f,  /*76*/
   0x07fff03f,  /*77*/
   0x0098301e,  /*78*/
   0x0099901f,  /*79*/
   0x000fc000,  /*80*/
   0x07fff03f,  /*81*/
   0x07fff027,  /*82*/
   0x07fff026,  /*83*/
   0x07fff01c,  /*84*/
   0x07fff03f,  /*85*/
   0x07fff03f,  /*86*/
   0x80379034,  /*87*/
   0x87fff04f,  /*88*/
   0x87fff04e,  /*89*/
   0x87fff048,  /*90*/
   0x07fff03f,  /*91*/
   0x07fff03f,  /*92*/
   0x07fff03f,  /*93*/
   0x87fff04c,  /*94*/
   0x07fff03f,  /*95*/
   0x07fff03f,  /*96*/
   0x07fff03f,  /*97*/
   0x07fff03f,  /*98*/
   0x07fff03f,  /*99*/
   0x07fff03f,  /*100*/
   0x07fff03f,  /*101*/
   0x07fff03f,  /*102*/
   0x07fff03f,  /*103*/
   0x07fff03f,  /*104*/
   0x07fff03f,  /*105*/
   0x07fff03f,  /*106*/
   0x07fff03f,  /*107*/
   0x07fff03f,  /*108*/
   0x07fff03f,  /*109*/
   0x07fff03f,  /*110*/
   0x07fff03f,  /*111*/
   0x07fff03f,  /*112*/
   0x07fff03f,  /*113*/
   0x07fff03f,  /*114*/
   0x07fff03f,  /*115*/
   0x07fff03f,  /*116*/
   0x07fff03f,  /*117*/
   0x87fff049,  /*118*/
   0x87fff050,  /*119*/
   0x80000040,  /*120*/
   0x80000041,  /*121*/
   0x80000042,  /*122*/
   0x07fff03f,  /*123*/
   0x800c202e,  /*124*/
   0x80000043,  /*125*/
   0x87fff044,  /*126*/
   0x0087201b   /*127*/
};
#endif

static uint32_t aul7439_2u0tRts_Memc0[] =
{
	0x0016c003,  /* XPT_WR_RS 1130ns */
	0x802d002a,  /* XPT_WR_XC RR 2230ns */
	0x80355012,  /* XPT_WR_CDB RR 2640ns */
	0x80907034,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f017,  /* XPT_RD_RS RR 4450ns */
	0x8136b01e,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x80355011,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b9030,  /* XPT_RD_PB RR 6110ns */
	0x8076102f,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c035,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fa00d,  /* GENET0_WR RR 2360ns */
	0x80871033,  /* GENET0_RD RR 7079ns */
	0x8052d015,  /* GENET1_WR RR 4096ns */
	0x81a1c037,  /* GENET1_RD RR 21875ns */
	0x3ffff052,  /* UNASSIGNED off */
	0x3ffff053,  /* UNASSIGNED off */
	0x84312025,  /* MOCA_MIPS RR 53000ns */
	0x8029b029,  /* SATA RR 2190ns */
	0x3ffff054,  /* UNASSIGNED off */
	0x3ffff055,  /* UNASSIGNED off */
	0x3ffff056,  /* UNASSIGNED off */
	0x3ffff057,  /* UNASSIGNED off */
	0x83f46024,  /* BSP RR 50000ns */
	0x80822031,  /* SAGE RR 6820ns */
	0x84b36038,  /* FLASH_DMA RR 63000ns */
	0x814e3036,  /* HIF_PCIe RR 17500ns */
	0x84b3603a,  /* SDIO_EMMC RR 63000ns */
	0x84b36039,  /* SDIO_CARD RR 63000ns */
	0xbffff04b,  /* TPCAP RR */
	0x02ed2020,  /* MCIF_RD_0 37000ns */
	0x02ed2022,  /* MCIF_WR_0 37000ns */
	0xbffff040,  /* UART_DMA_RD RR */
	0xbffff041,  /* UART_DMA_WR RR */
	0x8037102b,  /* USB_HI_0 RR 2890ns */
	0xbffff043,  /* USB_LO_0 RR */
	0x806e402e,  /* USB_X_WRITE_0 RR 5780ns */
	0x806e402d,  /* USB_X_READ_0 RR 5780ns */
	0x810d301d,  /* USB_X_CTRL_0 RR 13300ns */
	0x8037102c,  /* USB_HI_1 RR 2890ns */
	0xbffff044,  /* USB_LO_1 RR */
	0x02ed2021,  /* MCIF_RD_1 37000ns */
	0x02ed2023,  /* MCIF_WR_1 37000ns */
	0x003ca013,  /* RAAGA 3000ns */
	0x00142002,  /* RAAGA_1 1000ns */
	0x008c6019,  /* AUD_AIO 6940ns */
	0x3ffff0f0,  /* VICE_CME_RMB_CMB off */
	0x3ffff0f8,  /* VICE_CME_CSC off */
	0x3ffff0f1,  /* VICE_FME_CSC off */
	0x3ffff0f3,  /* VICE_FME_Luma_CMB off */
	0x3ffff0f2,  /* VICE_FME_Chroma_CMB off */
	0x3ffff0fb,  /* VICE_SG off */
	0x3ffff0f9,  /* VICE_DBLK off */
	0x3ffff0f7,  /* VICE_CABAC0 off */
	0x3ffff0ef,  /* VICE_CABAC1 off */
	0x3ffff0f6,  /* VICE_ARCSS0 off */
	0x3ffff0fc,  /* VICE_VIP0_INST0 off */
	0x3ffff0fe,  /* VICE_VIP1_INST0 off */
	0x3ffff0fd,  /* VICE_VIP0_INST1 off */
	0x3ffff0ff,  /* VICE_VIP1_INST1 off */
	0x3ffff058,  /* UNASSIGNED off */
	0x3ffff059,  /* UNASSIGNED off */
	0x3ffff05a,  /* UNASSIGNED off */
	0x3ffff05b,  /* UNASSIGNED off */
	0x3ffff05c,  /* UNASSIGNED off */
	0x3ffff05d,  /* UNASSIGNED off */
	0x3ffff05e,  /* UNASSIGNED off */
	0x3ffff05f,  /* UNASSIGNED off */
	0x3ffff060,  /* UNASSIGNED off */
	0x3ffff061,  /* UNASSIGNED off */
	0x3ffff062,  /* UNASSIGNED off */
	0x3ffff063,  /* UNASSIGNED off */
	0x3ffff064,  /* UNASSIGNED off */
	0x3ffff065,  /* UNASSIGNED off */
	0x8010e03c,  /* HVD_DBLK_0 RR 0ns */
	0x8008603b,  /* HVD_DBLK_1 RR 0ns */
	0x801d4028,  /* HVD_ILCPU RR 1451ns */
	0x80838032,  /* HVD_OLCPU RR 6893ns */
	0x005e5018,  /* HVD_CAB 4666ns */
	0x0054e016,  /* HVD_ILSI 4199ns */
	0x3ffff066,  /* UNASSIGNED off */
	0x3ffff067,  /* UNASSIGNED off */
	0x3ffff068,  /* UNASSIGNED off */
	0x3ffff069,  /* UNASSIGNED off */
	0x3ffff06a,  /* UNASSIGNED off */
	0x3ffff06b,  /* UNASSIGNED off */
	0x3ffff06c,  /* UNASSIGNED off */
	0x3ffff06d,  /* UNASSIGNED off */
	0xbffff04a,  /* SID RR */
	0x3ffff06e,  /* UNASSIGNED off */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x0046f014,  /* MADR_RD 3511ns */
	0x0031b00e,  /* MADR_QM 2460ns */
	0x008e001a,  /* MADR_WR 1756ns */
	0x3ffff074,  /* UNASSIGNED off */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0x0035500f,  /* BVNF_MFD0_0 2640ns */
	0x00238005,  /* BVNF_MFD0_1 1760ns */
	0x00355010,  /* BVNF_MFD1_0 2640ns */
	0x00238006,  /* BVNF_MFD1_1 1760ns */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x00255007,  /* BVNF_VFD0 1850ns */
	0x3ffff050,  /* BVNF_VFD1 off */
	0x00255008,  /* BVNF_VFD2 1850ns */
	0x3ffff051,  /* BVNF_VFD3 off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x00255009,  /* BVNF_CAP0 1850ns */
	0x3ffff0f4,  /* BVNF_CAP1 off */
	0x0025500a,  /* BVNF_CAP2 1850ns */
	0x3ffff0f5,  /* BVNF_CAP3 off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x00128000,  /* BVNF_GFD0 920ns */
	0x00128001,  /* BVNF_GFD1 920ns */
	0x3ffff04f,  /* BVNF_GFD2 off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x0027e00c,  /* MCVP0 1976ns */
	0x0027e00b,  /* MCVP1 1976ns */
	0x009fd01c,  /* MCVP_QM 7900ns */
	0x001c3004,  /* BVNF_RDC 1400ns */
	0x027dc01f,  /* VEC_VBI_ENC 31500ns */
	0x3ffff091,  /* UNASSIGNED off */
	0xbffff045,  /* V3D_0 RR */
	0xbffff04c,  /* V3D_1 RR */
	0xbffff046,  /* M2MC RR */
	0xbffff047,  /* M2MC1 RR */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x800e3026,  /* MCP_RD_HIGH RR 750ns */
	0x8000003e,  /* MCP_RD_LOW RR */
	0x800e3027,  /* MCP_WR_HIGH RR 750ns */
	0x8000003f,  /* MCP_WR_LOW RR */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x8000003d,  /* HVD_PFRI RR 0ns */
	0x3ffff0cf,  /* VICE_PFRI off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0xbffff049,  /* MEMC_TRACELOG RR */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0xbffff048,  /* MEMC_MSA RR */
	0xbffff04d,  /* MEMC_DIS0 RR */
	0xbffff04e,  /* MEMC_DIS1 RR */
	0xbffff042,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e101b,  /* REFRESH 7812.5ns */
};

static uint32_t aul7439_2tRts_Memc0[] =
{
	0x0016c002, /* XPT_WR_RS 1130ns */
	0x802d0031, /* XPT_WR_XC RR 2230ns */
	0x80355012, /* XPT_WR_CDB RR 2640ns */
	0x8090703c, /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f019, /* XPT_RD_RS RR 4450ns */
	0x8136b020, /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x80355011, /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b9038, /* XPT_RD_PB RR 6110ns */
	0x80761037, /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c041, /* XPT_RD_MEMDMA RR 13760ns */
	0x802fa00d, /* GENET0_WR RR 2360ns */
	0x8087103b, /* GENET0_RD RR 7079ns */
	0x8052d017, /* GENET1_WR RR 4096ns */
	0x81a1c043, /* GENET1_RD RR 21875ns */
	0x3ffff061, /* UNASSIGNED off */
	0x3ffff062, /* UNASSIGNED off */
	0x8431202b, /* MOCA_MIPS RR 53000ns */
	0x8029b030, /* SATA RR 2190ns */
	0x3ffff063, /* UNASSIGNED off */
	0x3ffff064, /* UNASSIGNED off */
	0x3ffff065, /* UNASSIGNED off */
	0x3ffff066, /* UNASSIGNED off */
	0x83f4602a, /* BSP RR 50000ns */
	0x80822039, /* SAGE RR 6820ns */
	0x84b36045, /* FLASH_DMA RR 63000ns */
	0x814e3042, /* HIF_PCIe RR 17500ns */
	0x84b36047, /* SDIO_EMMC RR 63000ns */
	0x84b36046, /* SDIO_CARD RR 63000ns */
	0xbffff05a, /* TPCAP RR */
	0x02ed2025, /* MCIF_RD_0 37000ns */
	0x02ed2027, /* MCIF_WR_0 37000ns */
	0xbffff04f, /* UART_DMA_RD RR */
	0xbffff050, /* UART_DMA_WR RR */
	0x80371032, /* USB_HI_0 RR 2890ns */
	0xbffff052, /* USB_LO_0 RR */
	0x806e4036, /* USB_X_WRITE_0 RR 5780ns */
	0x806e4035, /* USB_X_READ_0 RR 5780ns */
	0x810d301f, /* USB_X_CTRL_0 RR 13300ns */
	0x80371033, /* USB_HI_1 RR 2890ns */
	0xbffff053, /* USB_LO_1 RR */
	0x02ed2026, /* MCIF_RD_1 37000ns */
	0x02ed2028, /* MCIF_WR_1 37000ns */
	0x003ca013, /* RAAGA 3000ns */
	0x00142001, /* RAAGA_1 1000ns */
	0x008c601b, /* AUD_AIO 6940ns */
	0x8055d034, /* VICE_CME_RMB_CMB RR 4500ns */
	0x82afa044, /* VICE_CME_CSC RR 36000ns */
	0x809f003d, /* VICE_FME_CSC RR 8330ns */
	0x809f003f, /* VICE_FME_Luma_CMB RR 8330ns */
	0x809f003e, /* VICE_FME_Chroma_CMB RR 8330ns */
	0x81c1d023, /* VICE_SG RR 22221.6666666667ns */
	0x8000004c, /* VICE_DBLK RR 0ns */
	0x83402029, /* VICE_CABAC0 RR 41100ns */
	0x85c6202c, /* VICE_CABAC1 RR 73000ns */
	0x80ffe040, /* VICE_ARCSS0 RR 13400ns */
	0x804ac015, /* VICE_VIP0_INST0 RR 3700ns */
	0x81c16021, /* VICE_VIP1_INST0 RR 22200ns */
	0x804ac016, /* VICE_VIP0_INST1 RR 3700ns */
	0x81c16022, /* VICE_VIP1_INST1 RR 22200ns */
	0x3ffff067, /* UNASSIGNED off */
	0x3ffff068, /* UNASSIGNED off */
	0x3ffff069, /* UNASSIGNED off */
	0x3ffff06a, /* UNASSIGNED off */
	0x3ffff06b, /* UNASSIGNED off */
	0x3ffff06c, /* UNASSIGNED off */
	0x3ffff06d, /* UNASSIGNED off */
	0x3ffff06e, /* UNASSIGNED off */
	0x3ffff06f, /* UNASSIGNED off */
	0x3ffff070, /* UNASSIGNED off */
	0x3ffff071, /* UNASSIGNED off */
	0x3ffff072, /* UNASSIGNED off */
	0x3ffff073, /* UNASSIGNED off */
	0x3ffff074, /* UNASSIGNED off */
	0x8010e049, /* HVD_DBLK_0 RR 0ns */
	0x80086048, /* HVD_DBLK_1 RR 0ns */
	0x801d402f, /* HVD_ILCPU RR 1451ns */
	0x8083803a, /* HVD_OLCPU RR 6893ns */
	0x005e501a, /* HVD_CAB 4666ns */
	0x0054e018, /* HVD_ILSI 4199ns */
	0x3ffff075, /* UNASSIGNED off */
	0x3ffff076, /* UNASSIGNED off */
	0x3ffff077, /* UNASSIGNED off */
	0x3ffff078, /* UNASSIGNED off */
	0x3ffff079, /* UNASSIGNED off */
	0x3ffff07a, /* UNASSIGNED off */
	0x3ffff07b, /* UNASSIGNED off */
	0x3ffff07c, /* UNASSIGNED off */
	0xbffff059, /* SID RR */
	0x3ffff07d, /* UNASSIGNED off */
	0x3ffff07e, /* UNASSIGNED off */
	0x3ffff07f, /* UNASSIGNED off */
	0x3ffff080, /* UNASSIGNED off */
	0x3ffff081, /* UNASSIGNED off */
	0x3ffff082, /* UNASSIGNED off */
	0x0046f014, /* MADR_RD 3511ns */
	0x0031b00e, /* MADR_QM 2460ns */
	0x008e001c, /* MADR_WR 1756ns */
	0x3ffff083, /* UNASSIGNED off */
	0x3ffff084, /* UNASSIGNED off */
	0x3ffff085, /* UNASSIGNED off */
	0x3ffff086, /* UNASSIGNED off */
	0x3ffff087, /* UNASSIGNED off */
	0x3ffff088, /* UNASSIGNED off */
	0x3ffff089, /* UNASSIGNED off */
	0x3ffff08a, /* UNASSIGNED off */
	0x3ffff08b, /* UNASSIGNED off */
	0x0035500f, /* BVNF_MFD0_0 2640ns */
	0x00238004, /* BVNF_MFD0_1 1760ns */
	0x00355010, /* BVNF_MFD1_0 2640ns */
	0x00238005, /* BVNF_MFD1_1 1760ns */
	0x3ffff08c, /* UNASSIGNED off */
	0x3ffff08d, /* UNASSIGNED off */
	0x3ffff08e, /* UNASSIGNED off */
	0x3ffff08f, /* UNASSIGNED off */
	0x3ffff090, /* UNASSIGNED off */
	0x3ffff091, /* UNASSIGNED off */
	0x3ffff092, /* UNASSIGNED off */
	0x3ffff093, /* UNASSIGNED off */
	0x00255006, /* BVNF_VFD0 1850ns */
	0x00255007, /* BVNF_VFD1 1850ns */
	0x3ffff05f, /* BVNF_VFD2 off */
	0x3ffff060, /* BVNF_VFD3 off */
	0x3ffff094, /* UNASSIGNED off */
	0x3ffff095, /* UNASSIGNED off */
	0x3ffff096, /* UNASSIGNED off */
	0x3ffff097, /* UNASSIGNED off */
	0x00255008, /* BVNF_CAP0 1850ns */
	0x00255009, /* BVNF_CAP1 1850ns */
	0x3ffff0fe, /* BVNF_CAP2 off */
	0x3ffff0ff, /* BVNF_CAP3 off */
	0x3ffff098, /* UNASSIGNED off */
	0x3ffff099, /* UNASSIGNED off */
	0x3ffff09a, /* UNASSIGNED off */
	0x3ffff09b, /* UNASSIGNED off */
	0x00128000, /* BVNF_GFD0 920ns */
	0x3ffff05e, /* BVNF_GFD1 off */
	0x0028c00c, /* BVNF_GFD2 2020ns */
	0x3ffff09c, /* UNASSIGNED off */
	0x3ffff09d, /* UNASSIGNED off */
	0x3ffff09e, /* UNASSIGNED off */
	0x3ffff09f, /* UNASSIGNED off */
	0x0027e00b, /* MCVP0 1976ns */
	0x0027e00a, /* MCVP1 1976ns */
	0x009fd01e, /* MCVP_QM 7900ns */
	0x001c3003, /* BVNF_RDC 1400ns */
	0x027dc024, /* VEC_VBI_ENC 31500ns */
	0x3ffff0a0, /* UNASSIGNED off */
	0xbffff054, /* V3D_0 RR */
	0xbffff05b, /* V3D_1 RR */
	0xbffff055, /* M2MC RR */
	0xbffff056, /* M2MC1 RR */
	0x3ffff0a1, /* UNASSIGNED off */
	0x3ffff0a2, /* UNASSIGNED off */
	0x3ffff0a3, /* UNASSIGNED off */
	0x3ffff0a4, /* UNASSIGNED off */
	0x3ffff0a5, /* UNASSIGNED off */
	0x3ffff0a6, /* UNASSIGNED off */
	0x3ffff0a7, /* UNASSIGNED off */
	0x3ffff0a8, /* UNASSIGNED off */
	0x3ffff0a9, /* UNASSIGNED off */
	0x3ffff0aa, /* UNASSIGNED off */
	0x3ffff0ab, /* UNASSIGNED off */
	0x3ffff0ac, /* UNASSIGNED off */
	0x3ffff0ad, /* UNASSIGNED off */
	0x3ffff0ae, /* UNASSIGNED off */
	0x3ffff0af, /* UNASSIGNED off */
	0x3ffff0b0, /* UNASSIGNED off */
	0x3ffff0b1, /* UNASSIGNED off */
	0x3ffff0b2, /* UNASSIGNED off */
	0x3ffff0b3, /* UNASSIGNED off */
	0x3ffff0b4, /* UNASSIGNED off */
	0x3ffff0b5, /* UNASSIGNED off */
	0x3ffff0b6, /* UNASSIGNED off */
	0x3ffff0b7, /* UNASSIGNED off */
	0x3ffff0b8, /* UNASSIGNED off */
	0x3ffff0b9, /* UNASSIGNED off */
	0x3ffff0ba, /* UNASSIGNED off */
	0x3ffff0bb, /* UNASSIGNED off */
	0x3ffff0bc, /* UNASSIGNED off */
	0x3ffff0bd, /* UNASSIGNED off */
	0x3ffff0be, /* UNASSIGNED off */
	0x3ffff0bf, /* UNASSIGNED off */
	0x3ffff0c0, /* UNASSIGNED off */
	0x3ffff0c1, /* UNASSIGNED off */
	0x3ffff0c2, /* UNASSIGNED off */
	0x3ffff0c3, /* UNASSIGNED off */
	0x3ffff0c4, /* UNASSIGNED off */
	0x3ffff0c5, /* UNASSIGNED off */
	0x3ffff0c6, /* UNASSIGNED off */
	0x3ffff0c7, /* UNASSIGNED off */
	0x3ffff0c8, /* UNASSIGNED off */
	0x3ffff0c9, /* UNASSIGNED off */
	0x3ffff0ca, /* UNASSIGNED off */
	0x3ffff0cb, /* UNASSIGNED off */
	0x3ffff0cc, /* UNASSIGNED off */
	0x3ffff0cd, /* UNASSIGNED off */
	0x3ffff0ce, /* UNASSIGNED off */
	0x3ffff0cf, /* UNASSIGNED off */
	0x3ffff0d0, /* UNASSIGNED off */
	0x3ffff0d1, /* UNASSIGNED off */
	0x800e302d, /* MCP_RD_HIGH RR 750ns */
	0x8000004d, /* MCP_RD_LOW RR */
	0x800e302e, /* MCP_WR_HIGH RR 750ns */
	0x8000004e, /* MCP_WR_LOW RR */
	0x3ffff0d2, /* UNASSIGNED off */
	0x3ffff0d3, /* UNASSIGNED off */
	0x3ffff0d4, /* UNASSIGNED off */
	0x3ffff0d5, /* UNASSIGNED off */
	0x3ffff0d6, /* UNASSIGNED off */
	0x3ffff0d7, /* UNASSIGNED off */
	0x3ffff0d8, /* UNASSIGNED off */
	0x3ffff0d9, /* UNASSIGNED off */
	0x3ffff0da, /* UNASSIGNED off */
	0x3ffff0db, /* UNASSIGNED off */
	0x3ffff0dc, /* UNASSIGNED off */
	0x3ffff0dd, /* UNASSIGNED off */
	0x8000004a, /* HVD_PFRI RR 0ns */
	0x8000004b, /* VICE_PFRI RR 0ns */
	0x3ffff0df, /* UNASSIGNED off */
	0x8000004b, /* UNASSIGNED off */
	0x3ffff0e0, /* UNASSIGNED off */
	0x3ffff0e1, /* UNASSIGNED off */
	0x3ffff0e2, /* UNASSIGNED off */
	0x3ffff0e3, /* UNASSIGNED off */
	0x3ffff0e4, /* UNASSIGNED off */
	0x3ffff0e5, /* UNASSIGNED off */
	0x3ffff0e6, /* UNASSIGNED off */
	0x3ffff0e7, /* UNASSIGNED off */
	0x3ffff0e8, /* UNASSIGNED off */
	0x3ffff0e9, /* UNASSIGNED off */
	0x3ffff0ea, /* UNASSIGNED off */
	0x3ffff0eb, /* UNASSIGNED off */
	0x3ffff0ec, /* UNASSIGNED off */
	0x3ffff0ed, /* UNASSIGNED off */
	0x3ffff0ee, /* UNASSIGNED off */
	0x3ffff0ef, /* UNASSIGNED off */
	0x3ffff0f0, /* UNASSIGNED off */
	0x3ffff0f1, /* UNASSIGNED off */
	0x3ffff0f2, /* UNASSIGNED off */
	0x3ffff0f3, /* UNASSIGNED off */
	0x3ffff0f4, /* UNASSIGNED off */
	0x3ffff0f5, /* UNASSIGNED off */
	0x3ffff0f6, /* UNASSIGNED off */
	0x3ffff0f7, /* UNASSIGNED off */
	0x3ffff0f8, /* UNASSIGNED off */
	0x3ffff0f9, /* UNASSIGNED off */
	0x3ffff0fa, /* UNASSIGNED off */
	0x3ffff0fb, /* UNASSIGNED off */
	0xbffff058, /* MEMC_TRACELOG RR */
	0x3ffff0fc, /* UNASSIGNED off */
	0x3ffff0fd, /* UNASSIGNED off */
	0xbffff057, /* MEMC_MSA RR */
	0xbffff05c, /* MEMC_DIS0 RR */
	0xbffff05d, /* MEMC_DIS1 RR */
	0xbffff051, /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e101d, /* REFRESH 7812.5ns */
};

static uint32_t aul7439_1u1tRts_Memc0[] =
{
	0x0016c002,  /* XPT_WR_RS 1130ns */
	0x802d0034,  /* XPT_WR_XC RR 2230ns */
	0x8035500f,  /* XPT_WR_CDB RR 2640ns */
	0x8090703e,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f016,  /* XPT_RD_RS RR 4450ns */
	0x8136b01f,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x8035500e,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b903a,  /* XPT_RD_PB RR 6110ns */
	0x80761039,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c041,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fa00b,  /* GENET0_WR RR 2360ns */
	0x8087103d,  /* GENET0_RD RR 7079ns */
	0x8052d014,  /* GENET1_WR RR 4096ns */
	0x81a1c046,  /* GENET1_RD RR 21875ns */
	0x3ffff061,  /* UNASSIGNED off */
	0x3ffff062,  /* UNASSIGNED off */
	0x8431202d,  /* MOCA_MIPS RR 53000ns */
	0x8029b033,  /* SATA RR 2190ns */
	0x3ffff063,  /* UNASSIGNED off */
	0x3ffff064,  /* UNASSIGNED off */
	0x3ffff065,  /* UNASSIGNED off */
	0x3ffff066,  /* UNASSIGNED off */
	0x83f4602c,  /* BSP RR 50000ns */
	0x8082203b,  /* SAGE RR 6820ns */
	0x84b36047,  /* FLASH_DMA RR 63000ns */
	0x814e3045,  /* HIF_PCIe RR 17500ns */
	0x84b36049,  /* SDIO_EMMC RR 63000ns */
	0x84b36048,  /* SDIO_CARD RR 63000ns */
	0xbffff05d,  /* TPCAP RR */
	0x02ed2026,  /* MCIF_RD_0 37000ns */
	0x02ed2028,  /* MCIF_WR_0 37000ns */
	0xbffff052,  /* UART_DMA_RD RR */
	0xbffff053,  /* UART_DMA_WR RR */
	0x80371035,  /* USB_HI_0 RR 2890ns */
	0xbffff055,  /* USB_LO_0 RR */
	0x806e4038,  /* USB_X_WRITE_0 RR 5780ns */
	0x806e4037,  /* USB_X_READ_0 RR 5780ns */
	0x810d301e,  /* USB_X_CTRL_0 RR 13300ns */
	0x80371036,  /* USB_HI_1 RR 2890ns */
	0xbffff056,  /* USB_LO_1 RR */
	0x02ed2027,  /* MCIF_RD_1 37000ns */
	0x02ed2029,  /* MCIF_WR_1 37000ns */
	0x003ca010,  /* RAAGA 3000ns */
	0x00142001,  /* RAAGA_1 1000ns */
	0x008c6018,  /* AUD_AIO 6940ns */
	0x80abd03f,  /* VICE_CME_RMB_CMB RR 9000ns */
	0x855f504a,  /* VICE_CME_CSC RR 72000ns */
	0x813e2042,  /* VICE_FME_CSC RR 16660ns */
	0x813e2044,  /* VICE_FME_Luma_CMB RR 16660ns */
	0x813e2043,  /* VICE_FME_Chroma_CMB RR 16660ns */
	0x8383d02b,  /* VICE_SG RR 44443.3333333333ns */
	0x8000004f,  /* VICE_DBLK RR 0ns */
	0x8680602e,  /* VICE_CABAC0 RR 82200ns */
	0x8b8c602f,  /* VICE_CABAC1 RR 146000ns */
	0x80ffe040,  /* VICE_ARCSS0 RR 13400ns */
	0x804ac013,  /* VICE_VIP0_INST0 RR 3700ns */
	0x81c16023,  /* VICE_VIP1_INST0 RR 22200ns */
	0x3ffff0fe,  /* VICE_VIP0_INST1 off */
	0x3ffff0ff,  /* VICE_VIP1_INST1 off */
	0x3ffff067,  /* UNASSIGNED off */
	0x3ffff068,  /* UNASSIGNED off */
	0x3ffff069,  /* UNASSIGNED off */
	0x3ffff06a,  /* UNASSIGNED off */
	0x3ffff06b,  /* UNASSIGNED off */
	0x3ffff06c,  /* UNASSIGNED off */
	0x3ffff06d,  /* UNASSIGNED off */
	0x3ffff06e,  /* UNASSIGNED off */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x3ffff074,  /* UNASSIGNED off */
	0x8010e04c,  /* HVD_DBLK_0 RR 0ns */
	0x8008604b,  /* HVD_DBLK_1 RR 0ns */
	0x801d4032,  /* HVD_ILCPU RR 1451ns */
	0x8083803c,  /* HVD_OLCPU RR 6893ns */
	0x005e5017,  /* HVD_CAB 4666ns */
	0x0054e015,  /* HVD_ILSI 4199ns */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0xbffff05c,  /* SID RR */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x0195e020,  /* MADR_RD 20030ns */
	0x023b5024,  /* MADR_QM 14110ns */
	0x032be02a,  /* MADR_WR 10010ns */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x004ac011,  /* BVNF_MFD0_0 3700ns */
	0x0031b00c,  /* BVNF_MFD0_1 2460ns */
	0x004ac012,  /* BVNF_MFD1_0 3700ns */
	0x0031b00d,  /* BVNF_MFD1_1 2460ns */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x00255004,  /* BVNF_VFD0 1850ns */
	0x00255005,  /* BVNF_VFD1 1850ns */
	0x01acc021,  /* BVNF_VFD2 21180ns */
	0x01acc022,  /* BVNF_VFD3 21180ns */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x00255006,  /* BVNF_CAP0 1850ns */
	0x00255007,  /* BVNF_CAP1 1850ns */
	0x00bb301c,  /* BVNF_CAP2 9250ns */
	0x00bb301d,  /* BVNF_CAP3 9250ns */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff09b,  /* UNASSIGNED off */
	0x00128000,  /* BVNF_GFD0 920ns */
	0x00b3b01b,  /* BVNF_GFD1 8880ns */
	0x0028c00a,  /* BVNF_GFD2 2020ns */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x0027e009,  /* MCVP0 1976ns */
	0x0027e008,  /* MCVP1 1976ns */
	0x009fd01a,  /* MCVP_QM 7900ns */
	0x001c3003,  /* BVNF_RDC 1400ns */
	0x027dc025,  /* VEC_VBI_ENC 31500ns */
	0x3ffff0a0,  /* UNASSIGNED off */
	0xbffff057,  /* V3D_0 RR */
	0xbffff05e,  /* V3D_1 RR */
	0xbffff058,  /* M2MC RR */
	0xbffff059,  /* M2MC1 RR */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x800e3030,  /* MCP_RD_HIGH RR 750ns */
	0x80000050,  /* MCP_RD_LOW RR */
	0x800e3031,  /* MCP_WR_HIGH RR 750ns */
	0x80000051,  /* MCP_WR_LOW RR */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x8000004d,  /* HVD_PFRI RR 0ns */
	0x8000004e,  /* VICE_PFRI RR 0ns */
	0x3ffff0df,  /* UNASSIGNED off */
	0x8000004e,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0xbffff05b,  /* MEMC_TRACELOG RR */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff05a,  /* MEMC_MSA RR */
	0xbffff05f,  /* MEMC_DIS0 RR */
	0xbffff060,  /* MEMC_DIS1 RR */
	0xbffff054,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e1019,  /* REFRESH 7812.5ns */
};

static uint32_t aul7252_1u2tRts_Memc0[] =
{
	0x0016c002,  /* XPT_WR_RS 1130ns */
	0x802d0043,  /* XPT_WR_XC RR 2230ns */
	0x80355010,  /* XPT_WR_CDB RR 2640ns */
	0x8090704c,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f022,  /* XPT_RD_RS RR 4450ns */
	0x8136b02f,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x8035500f,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b9047,  /* XPT_RD_PB RR 6110ns */
	0x80761046,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c054,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd008,  /* GENET0_WR RR 2370ns */
	0x80c1c04d,  /* GENET0_RD RR 10150ns */
	0x80675025,  /* GENET1_WR RR 5110ns */
	0x80c1c04e,  /* GENET1_RD RR 10150ns */
	0x802fd009,  /* GENET2_WR RR 2370ns */
	0x80c1c04f,  /* GENET2_RD RR 10150ns */
	0x8431203d,  /* MOCA_MIPS RR 53000ns */
	0x804c9044,  /* SATA RR 4015ns */
	0x804c9045,  /* SATA_1 RR 4015ns */
	0x02ed2037,  /* MCIF2_RD 37000ns */
	0x02ed2039,  /* MCIF2_WR 37000ns */
	0x3ffff078,  /* UNASSIGNED off */
	0x83f4603b,  /* BSP RR 50000ns */
	0x80822048,  /* SAGE RR 6820ns */
	0x84b36056,  /* FLASH_DMA RR 63000ns */
	0x81090055,  /* HIF_PCIe RR 13880ns */
	0x84b36058,  /* SDIO_EMMC RR 63000ns */
	0x84b36057,  /* SDIO_CARD RR 63000ns */
	0xbffff074,  /* TPCAP RR */
	0x02ed2038,  /* MCIF_RD 37000ns */
	0x02ed203a,  /* MCIF_WR 37000ns */
	0xbffff069,  /* UART_DMA_RD RR */
	0xbffff06a,  /* UART_DMA_WR RR */
	0x80ca4050,  /* USB_HI_0 RR 10593ns */
	0xbffff06c,  /* USB_LO_0 RR */
	0x81053053,  /* USB_X_WRITE_0 RR 13680ns */
	0x81053052,  /* USB_X_READ_0 RR 13680ns */
	0x80828049,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca4051,  /* USB_HI_1 RR 10593ns */
	0xbffff06d,  /* USB_LO_1 RR */
	0x003ca016,  /* RAAGA 3000ns */
	0x00142001,  /* RAAGA_1 1000ns */
	0x00796028,  /* RAAGA1 6000ns */
	0x00142000,  /* RAAGA1_1 1000ns */
	0x008c6029,  /* AUD_AIO 6940ns */
	0x3ffff059,  /* VICE_CME_RMB_CMB off */
	0x3ffff05e,  /* VICE_CME_CSC off */
	0x3ffff05a,  /* VICE_FME_CSC off */
	0x3ffff05c,  /* VICE_FME_Luma_CMB off */
	0x3ffff05b,  /* VICE_FME_Chroma_CMB off */
	0x8176e032,  /* VICE_SG RR 18518.6666666667ns */
	0x3ffff064,  /* VICE_DBLK off */
	0x8759c03e,  /* VICE_CABAC0 RR 92933.3333333333ns */
	0x8428c03c,  /* VICE_CABAC1 RR 52586.6666666667ns */
	0x3ffff05d,  /* VICE_ARCSS0 off */
	0x3ffff02a,  /* VICE_VIP0_INST0 off */
	0x3ffff033,  /* VICE_VIP1_INST0 off */
	0x3ffff02b,  /* VICE_VIP0_INST1 off */
	0x3ffff034,  /* VICE_VIP1_INST1 off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x8007f062,  /* SHVD0_DBLK_0 RR 10ns */
	0x8003e060,  /* SHVD0_DBLK_1 RR 10ns */
	0x801d4042,  /* SHVD0_ILCPU RR 1451ns */
	0x8083804b,  /* SHVD0_OLCPU RR 6893ns */
	0x005e5024,  /* SHVD0_CAB 4666ns */
	0x0054e021,  /* SHVD0_ILSI 4199ns */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x80077061,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a05f,  /* HVD1_DBLK_1 RR 10ns */
	0x801d3041,  /* HVD1_ILCPU RR 1450ns */
	0x8083804a,  /* HVD1_OLCPU RR 6893ns */
	0x005e5023,  /* HVD1_CAB 4666ns */
	0x0054e020,  /* HVD1_ILSI 4199ns */
	0xbffff073,  /* SID RR */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff007,  /* MADR_RD off */
	0x3ffff00e,  /* MADR_QM off */
	0x3ffff01b,  /* MADR_WR off */
	0x00236006,  /* MADR1_RD 1755ns */
	0x0031d00d,  /* MADR1_QM 2469ns */
	0x0046f01a,  /* MADR1_WR 3511ns */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x004ae01c,  /* BVNF_MFD0_0 3704ns */
	0x0018e003,  /* BVNF_MFD0_1 1235ns */
	0x004ae01d,  /* BVNF_MFD1_0 3704ns */
	0x0031d00a,  /* BVNF_MFD1_1 2469ns */
	0x3ffff01e,  /* BVNF_MFD2_0 off */
	0x3ffff00b,  /* BVNF_MFD2_1 off */
	0x004ae01f,  /* BVNF_MFD3_0 3704ns */
	0x0031d00c,  /* BVNF_MFD3_1 2469ns */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff004,  /* BVNF_VFD0 off */
	0x3ffff099,  /* UNASSIGNED off */
	0x01413030,  /* BVNF_VFD2 15870ns */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff012,  /* BVNF_VFD4 off */
	0x003bd013,  /* BVNF_VFD5 2960ns */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff005,  /* BVNF_CAP0 off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x01413031,  /* BVNF_CAP2 15870ns */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff014,  /* BVNF_CAP4 off */
	0x003bd015,  /* BVNF_CAP5 2960ns */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff011,  /* BVNF_GFD0 off */
	0x3ffff02e,  /* BVNF_GFD1 off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff026,  /* BVNF_GFD3 off */
	0x006ce027,  /* BVNF_GFD4 5384ns */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff019,  /* MCVP0 off */
	0x3ffff018,  /* MCVP1 off */
	0x3ffff02c,  /* MCVP_QM off */
	0x00414017,  /* BVNF_RDC 3230ns */
	0x027dc035,  /* VEC_VBI_ENC0 31500ns */
	0x027dc036,  /* VEC_VBI_ENC1 31500ns */
	0xbffff06e,  /* V3D_0 RR */
	0xbffff075,  /* V3D_1 RR */
	0xbffff06f,  /* M2MC RR */
	0xbffff070,  /* M2MC1 RR */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x800f103f,  /* MCP_RD_HIGH RR 750ns */
	0x80000066,  /* MCP_RD_LOW RR 0ns */
	0x801c8040,  /* MCP_WR_HIGH RR 1500ns */
	0x80000068,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x80000067,  /* SHVD0_PFRI RR 9ns */
	0x80000065,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff063,  /* VICE_PFRI off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff072,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff071,  /* MEMC_MSA RR */
	0xbffff076,  /* MEMC_DIS0 RR */
	0xbffff077,  /* MEMC_DIS1 RR */
	0xbffff06b,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e102d,  /* REFRESH 7812.5ns */
};

static uint32_t aul7252_4K1tRts_Memc0[] =
{
	0x0016c002,  /* XPT_WR_RS 1130ns */
	0x802d0039,  /* XPT_WR_XC RR 2230ns */
	0x8035500c,  /* XPT_WR_CDB RR 2640ns */
	0x80907042,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f019,  /* XPT_RD_RS RR 4450ns */
	0x8136b025,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x8035500b,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b903d,  /* XPT_RD_PB RR 6110ns */
	0x8076103c,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c04a,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd007,  /* GENET0_WR RR 2370ns */
	0x80c1c043,  /* GENET0_RD RR 10150ns */
	0x8067501c,  /* GENET1_WR RR 5110ns */
	0x80c1c044,  /* GENET1_RD RR 10150ns */
	0x802fd008,  /* GENET2_WR RR 2370ns */
	0x80c1c045,  /* GENET2_RD RR 10150ns */
	0x84312033,  /* MOCA_MIPS RR 53000ns */
	0x804c903a,  /* SATA RR 4015ns */
	0x804c903b,  /* SATA_1 RR 4015ns */
	0x02ed202d,  /* MCIF2_RD 37000ns */
	0x02ed202f,  /* MCIF2_WR 37000ns */
	0x3ffff06e,  /* UNASSIGNED off */
	0x83f46031,  /* BSP RR 50000ns */
	0x8082203e,  /* SAGE RR 6820ns */
	0x84b3604c,  /* FLASH_DMA RR 63000ns */
	0x8109004b,  /* HIF_PCIe RR 13880ns */
	0x84b3604e,  /* SDIO_EMMC RR 63000ns */
	0x84b3604d,  /* SDIO_CARD RR 63000ns */
	0xbffff06a,  /* TPCAP RR */
	0x02ed202e,  /* MCIF_RD 37000ns */
	0x02ed2030,  /* MCIF_WR 37000ns */
	0xbffff05f,  /* UART_DMA_RD RR */
	0xbffff060,  /* UART_DMA_WR RR */
	0x80ca4046,  /* USB_HI_0 RR 10593ns */
	0xbffff062,  /* USB_LO_0 RR */
	0x81053049,  /* USB_X_WRITE_0 RR 13680ns */
	0x81053048,  /* USB_X_READ_0 RR 13680ns */
	0x8082803f,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca4047,  /* USB_HI_1 RR 10593ns */
	0xbffff063,  /* USB_LO_1 RR */
	0x003ca010,  /* RAAGA 3000ns */
	0x00142001,  /* RAAGA_1 1000ns */
	0x0079601e,  /* RAAGA1 6000ns */
	0x00142000,  /* RAAGA1_1 1000ns */
	0x008c601f,  /* AUD_AIO 6940ns */
	0x3ffff04f,  /* VICE_CME_RMB_CMB off */
	0x3ffff054,  /* VICE_CME_CSC off */
	0x3ffff050,  /* VICE_FME_CSC off */
	0x3ffff052,  /* VICE_FME_Luma_CMB off */
	0x3ffff051,  /* VICE_FME_Chroma_CMB off */
	0x8176e028,  /* VICE_SG RR 18518.6666666667ns */
	0x3ffff05a,  /* VICE_DBLK off */
	0x8759c034,  /* VICE_CABAC0 RR 92933.3333333333ns */
	0x8428c032,  /* VICE_CABAC1 RR 52586.6666666667ns */
	0x3ffff053,  /* VICE_ARCSS0 off */
	0x3ffff020,  /* VICE_VIP0_INST0 off */
	0x3ffff029,  /* VICE_VIP1_INST0 off */
	0x3ffff021,  /* VICE_VIP0_INST1 off */
	0x3ffff02a,  /* VICE_VIP1_INST1 off */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x3ffff074,  /* UNASSIGNED off */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0x8007f058,  /* SHVD0_DBLK_0 RR 10ns */
	0x8003e056,  /* SHVD0_DBLK_1 RR 10ns */
	0x801d4038,  /* SHVD0_ILCPU RR 1451ns */
	0x80838041,  /* SHVD0_OLCPU RR 6893ns */
	0x005e501b,  /* SHVD0_CAB 4666ns */
	0x0054e018,  /* SHVD0_ILSI 4199ns */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x80077057,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a055,  /* HVD1_DBLK_1 RR 10ns */
	0x801d3037,  /* HVD1_ILCPU RR 1450ns */
	0x80838040,  /* HVD1_OLCPU RR 6893ns */
	0x005e501a,  /* HVD1_CAB 4666ns */
	0x0054e017,  /* HVD1_ILSI 4199ns */
	0xbffff069,  /* SID RR */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x00236006,  /* MADR1_RD 1755ns */
	0x0031d00a,  /* MADR1_QM 2469ns */
	0x0046f014,  /* MADR1_WR 3511ns */
	0x3ffff088,  /* UNASSIGNED off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x004ae015,  /* BVNF_MFD0_0 3704ns */
	0x0018e003,  /* BVNF_MFD0_1 1235ns */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff016,  /* BVNF_MFD2_0 off */
	0x3ffff009,  /* BVNF_MFD2_1 off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff004,  /* BVNF_VFD0 off */
	0x3ffff096,  /* UNASSIGNED off */
	0x01413026,  /* BVNF_VFD2 15870ns */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x003bd00e,  /* BVNF_VFD5 2960ns */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff005,  /* BVNF_CAP0 off */
	0x3ffff09b,  /* UNASSIGNED off */
	0x01413027,  /* BVNF_CAP2 15870ns */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x003bd00f,  /* BVNF_CAP5 2960ns */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff00d,  /* BVNF_GFD0 off */
	0x010dc024,  /* BVNF_GFD1 13330ns */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x006ce01d,  /* BVNF_GFD4 5384ns */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff013,  /* MCVP0 off */
	0x3ffff012,  /* MCVP1 off */
	0x3ffff022,  /* MCVP_QM off */
	0x00414011,  /* BVNF_RDC 3230ns */
	0x027dc02b,  /* VEC_VBI_ENC0 31500ns */
	0x027dc02c,  /* VEC_VBI_ENC1 31500ns */
	0xbffff064,  /* V3D_0 RR */
	0xbffff06b,  /* V3D_1 RR */
	0xbffff065,  /* M2MC RR */
	0xbffff066,  /* M2MC1 RR */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x800f1035,  /* MCP_RD_HIGH RR 750ns */
	0x8000005c,  /* MCP_RD_LOW RR 0ns */
	0x801c8036,  /* MCP_WR_HIGH RR 1500ns */
	0x8000005e,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x8000005d,  /* SHVD0_PFRI RR 9ns */
	0x8000005b,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff059,  /* VICE_PFRI off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff068,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff067,  /* MEMC_MSA RR */
	0xbffff06c,  /* MEMC_DIS0 RR */
	0xbffff06d,  /* MEMC_DIS1 RR */
	0xbffff061,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e1023,  /* REFRESH 7812.5ns */
};

static uint32_t aul7252_4KstbRts_Memc0[] =
{
	0x0016c002,  /* XPT_WR_RS 1130ns */
	0x802d0030,  /* XPT_WR_XC RR 2230ns */
	0x8035500b,  /* XPT_WR_CDB RR 2640ns */
	0x80907039,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x8059f016,  /* XPT_RD_RS RR 4450ns */
	0x8136b01f,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x8035500a,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x807b9034,  /* XPT_RD_PB RR 6110ns */
	0x80761033,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c041,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd007,  /* GENET0_WR RR 2370ns */
	0x80c1c03a,  /* GENET0_RD RR 10150ns */
	0x80675019,  /* GENET1_WR RR 5110ns */
	0x80c1c03b,  /* GENET1_RD RR 10150ns */
	0x802fd008,  /* GENET2_WR RR 2370ns */
	0x80c1c03c,  /* GENET2_RD RR 10150ns */
	0x8431202b,  /* MOCA_MIPS RR 53000ns */
	0x804c9031,  /* SATA RR 4015ns */
	0x804c9032,  /* SATA_1 RR 4015ns */
	0x02ed2026,  /* MCIF2_RD 37000ns */
	0x02ed2028,  /* MCIF2_WR 37000ns */
	0x3ffff05d,  /* UNASSIGNED off */
	0x83f4602a,  /* BSP RR 50000ns */
	0x80822035,  /* SAGE RR 6820ns */
	0x84b36043,  /* FLASH_DMA RR 63000ns */
	0x81090042,  /* HIF_PCIe RR 13880ns */
	0x84b36045,  /* SDIO_EMMC RR 63000ns */
	0x84b36044,  /* SDIO_CARD RR 63000ns */
	0xbffff059,  /* TPCAP RR */
	0x02ed2027,  /* MCIF_RD 37000ns */
	0x02ed2029,  /* MCIF_WR 37000ns */
	0xbffff04e,  /* UART_DMA_RD RR */
	0xbffff04f,  /* UART_DMA_WR RR */
	0x80ca403d,  /* USB_HI_0 RR 10593ns */
	0xbffff051,  /* USB_LO_0 RR */
	0x81053040,  /* USB_X_WRITE_0 RR 13680ns */
	0x8105303f,  /* USB_X_READ_0 RR 13680ns */
	0x80828036,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca403e,  /* USB_HI_1 RR 10593ns */
	0xbffff052,  /* USB_LO_1 RR */
	0x003ca00e,  /* RAAGA 3000ns */
	0x00142001,  /* RAAGA_1 1000ns */
	0x0079601a,  /* RAAGA1 6000ns */
	0x00142000,  /* RAAGA1_1 1000ns */
	0x008c601b,  /* AUD_AIO 6940ns */
	0x3ffff05e,  /* UNASSIGNED off */
	0x3ffff05f,  /* UNASSIGNED off */
	0x3ffff060,  /* UNASSIGNED off */
	0x3ffff061,  /* UNASSIGNED off */
	0x3ffff062,  /* UNASSIGNED off */
	0x3ffff063,  /* UNASSIGNED off */
	0x3ffff064,  /* UNASSIGNED off */
	0x3ffff065,  /* UNASSIGNED off */
	0x3ffff066,  /* UNASSIGNED off */
	0x3ffff067,  /* UNASSIGNED off */
	0x3ffff068,  /* UNASSIGNED off */
	0x3ffff069,  /* UNASSIGNED off */
	0x3ffff06a,  /* UNASSIGNED off */
	0x3ffff06b,  /* UNASSIGNED off */
	0x3ffff06c,  /* UNASSIGNED off */
	0x3ffff06d,  /* UNASSIGNED off */
	0x3ffff06e,  /* UNASSIGNED off */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x3ffff074,  /* UNASSIGNED off */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x8007f049,  /* SHVD0_DBLK_0 RR 10ns */
	0x8003e047,  /* SHVD0_DBLK_1 RR 10ns */
	0x801d402f,  /* SHVD0_ILCPU RR 1451ns */
	0x80838038,  /* SHVD0_OLCPU RR 6893ns */
	0x005e5018,  /* SHVD0_CAB 4666ns */
	0x0054e015,  /* SHVD0_ILSI 4199ns */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x80077048,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a046,  /* HVD1_DBLK_1 RR 10ns */
	0x801d302e,  /* HVD1_ILCPU RR 1450ns */
	0x80838037,  /* HVD1_OLCPU RR 6893ns */
	0x005e5017,  /* HVD1_CAB 4666ns */
	0x0054e014,  /* HVD1_ILSI 4199ns */
	0xbffff058,  /* SID RR */
	0x3ffff07c,  /* UNASSIGNED off */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x004ae012,  /* BVNF_MFD0_0 3704ns */
	0x0018e003,  /* BVNF_MFD0_1 1235ns */
	0x3ffff013,  /* BVNF_MFD1_0 off */
	0x3ffff009,  /* BVNF_MFD1_1 off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff004,  /* BVNF_VFD0 off */
	0x001dd005,  /* BVNF_VFD1 1480ns */
	0x01413021,  /* BVNF_VFD2 15870ns */
	0x013f6020,  /* BVNF_VFD3 15780ns */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff006,  /* BVNF_CAP0 off */
	0x003bd00d,  /* BVNF_CAP1 2960ns */
	0x01413022,  /* BVNF_CAP2 15870ns */
	0x02829025,  /* BVNF_CAP3 31740ns */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x3ffff00c,  /* BVNF_GFD0 off */
	0x010dc01e,  /* BVNF_GFD1 13330ns */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff011,  /* MCVP0 off */
	0x3ffff010,  /* MCVP1 off */
	0x3ffff01c,  /* MCVP_QM off */
	0x0041400f,  /* BVNF_RDC 3230ns */
	0x027dc023,  /* VEC_VBI_ENC0 31500ns */
	0x027dc024,  /* VEC_VBI_ENC1 31500ns */
	0xbffff053,  /* V3D_0 RR */
	0xbffff05a,  /* V3D_1 RR */
	0xbffff054,  /* M2MC RR */
	0xbffff055,  /* M2MC1 RR */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x800f102c,  /* MCP_RD_HIGH RR 750ns */
	0x8000004b,  /* MCP_RD_LOW RR 0ns */
	0x801c802d,  /* MCP_WR_HIGH RR 1500ns */
	0x8000004d,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x8000004c,  /* SHVD0_PFRI RR 9ns */
	0x8000004a,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff057,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff056,  /* MEMC_MSA RR */
	0xbffff05b,  /* MEMC_DIS0 RR */
	0xbffff05c,  /* MEMC_DIS1 RR */
	0xbffff050,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e101d,  /* REFRESH 7812.5ns */
};

#if BCHP_MEMC_ARB_1_REG_START
static uint32_t aul7252_1u2tRts_Memc1[] =
{
	0x3ffff002,  /* XPT_WR_RS 1130ns */
	0x3ffff043,  /* XPT_WR_XC RR 2230ns */
	0x3ffff010,  /* XPT_WR_CDB RR 2640ns */
	0x3ffff04c,  /* XPT_WR_ITB_MSG RR 7140ns */
	0x3ffff022,  /* XPT_RD_RS RR 4450ns */
	0x3ffff02f,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
	0x3ffff00f,  /* XPT_RD_XC_RAVE RR 2640ns */
	0x3ffff047,  /* XPT_RD_PB RR 6110ns */
	0x80761046,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c054,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd008,  /* GENET0_WR RR 2370ns */
	0x80c1c04d,  /* GENET0_RD RR 10150ns */
	0x80675025,  /* GENET1_WR RR 5110ns */
	0x80c1c04e,  /* GENET1_RD RR 10150ns */
	0x802fd009,  /* GENET2_WR RR 2370ns */
	0x80c1c04f,  /* GENET2_RD RR 10150ns */
	0x8431203d,  /* MOCA_MIPS RR 53000ns */
	0x804c9044,  /* SATA RR 4015ns */
	0x804c9045,  /* SATA_1 RR 4015ns */
	0x3ffff037,  /* MCIF2_RD 37000ns */
	0x3ffff039,  /* MCIF2_WR 37000ns */
	0x3ffff078,  /* UNASSIGNED off */
	0x83f4603b,  /* BSP RR 50000ns */
	0x80822048,  /* SAGE RR 6820ns */
	0x84b36056,  /* FLASH_DMA RR 63000ns */
	0x81090055,  /* HIF_PCIe RR 13880ns */
	0x84b36058,  /* SDIO_EMMC RR 63000ns */
	0x84b36057,  /* SDIO_CARD RR 63000ns */
	0x3ffff074,  /* TPCAP RR */
	0x3ffff038,  /* MCIF_RD 37000ns */
	0x3ffff03a,  /* MCIF_WR 37000ns */
	0x3ffff069,  /* UART_DMA_RD RR */
	0x3ffff06a,  /* UART_DMA_WR RR */
	0x80ca4050,  /* USB_HI_0 RR 10593ns */
	0xbffff06c,  /* USB_LO_0 RR */
	0x81053053,  /* USB_X_WRITE_0 RR 13680ns */
	0x81053052,  /* USB_X_READ_0 RR 13680ns */
	0x80828049,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca4051,  /* USB_HI_1 RR 10593ns */
	0xbffff06d,  /* USB_LO_1 RR */
	0x3ffff016,  /* RAAGA 3000ns */
	0x3ffff001,  /* RAAGA_1 1000ns */
	0x3ffff028,  /* RAAGA1 6000ns */
	0x3ffff000,  /* RAAGA1_1 1000ns */
	0x3ffff029,  /* AUD_AIO 6940ns */
	0x80567059,  /* VICE_CME_RMB_CMB off */
	0x82b4b05e,  /* VICE_CME_CSC off */
	0x809ec05a,  /* VICE_FME_CSC off */
	0x804f205c,  /* VICE_FME_Luma_CMB off */
	0x809ec05b,  /* VICE_FME_Chroma_CMB off */
	0x3ffff032,  /* VICE_SG RR 18518.6666666667ns */
	0x80001064,  /* VICE_DBLK off */
	0x3ffff03e,  /* VICE_CABAC0 RR 92933.3333333333ns */
	0x3ffff03c,  /* VICE_CABAC1 RR 52586.6666666667ns */
	0x803a205d,  /* VICE_ARCSS0 off */
	0x8095d02a,  /* VICE_VIP0_INST0 off */
	0x81c1d033,  /* VICE_VIP1_INST0 off */
	0x8095d02b,  /* VICE_VIP0_INST1 off */
	0x81c1d034,  /* VICE_VIP1_INST1 off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff062,  /* SHVD0_DBLK_0 RR 10ns */
	0x3ffff060,  /* SHVD0_DBLK_1 RR 10ns */
	0x3ffff042,  /* SHVD0_ILCPU RR 1451ns */
	0x3ffff04b,  /* SHVD0_OLCPU RR 6893ns */
	0x3ffff024,  /* SHVD0_CAB 4666ns */
	0x3ffff021,  /* SHVD0_ILSI 4199ns */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x80077061,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a05f,  /* HVD1_DBLK_1 RR 10ns */
	0x3ffff041,  /* HVD1_ILCPU RR 1450ns */
	0x3ffff04a,  /* HVD1_OLCPU RR 6893ns */
	0x3ffff023,  /* HVD1_CAB 4666ns */
	0x3ffff020,  /* HVD1_ILSI 4199ns */
	0xbffff073,  /* SID RR */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x00236007,  /* MADR_RD off */
	0x0031d00e,  /* MADR_QM off */
	0x0046f01b,  /* MADR_WR off */
	0x3ffff006,  /* MADR1_RD 1755ns */
	0x3ffff00d,  /* MADR1_QM 2469ns */
	0x3ffff01a,  /* MADR1_WR 3511ns */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff01c,  /* BVNF_MFD0_0 3704ns */
	0x3ffff003,  /* BVNF_MFD0_1 1235ns */
	0x004ae01d,  /* BVNF_MFD1_0 3704ns */
	0x0031d00a,  /* BVNF_MFD1_1 2469ns */
	0x004ae01e,  /* BVNF_MFD2_0 off */
	0x0031d00b,  /* BVNF_MFD2_1 off */
	0x3ffff01f,  /* BVNF_MFD3_0 3704ns */
	0x3ffff00c,  /* BVNF_MFD3_1 2469ns */
	0x3ffff095,  /* UNASSIGNED off */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x001dd004,  /* BVNF_VFD0 off */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff030,  /* BVNF_VFD2 15870ns */
	0x3ffff09a,  /* UNASSIGNED off */
	0x003bd012,  /* BVNF_VFD4 off */
	0x3ffff013,  /* BVNF_VFD5 2960ns */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x001dd005,  /* BVNF_CAP0 off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x3ffff031,  /* BVNF_CAP2 15870ns */
	0x3ffff09e,  /* UNASSIGNED off */
	0x003bd014,  /* BVNF_CAP4 off */
	0x3ffff015,  /* BVNF_CAP5 2960ns */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x00365011,  /* BVNF_GFD0 off */
	0x010dc02e,  /* BVNF_GFD1 off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x006ce026,  /* BVNF_GFD3 off */
	0x3ffff027,  /* BVNF_GFD4 5384ns */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x00450019,  /* MCVP0 off */
	0x00450018,  /* MCVP1 off */
	0x009b502c,  /* MCVP_QM off */
	0x3ffff017,  /* BVNF_RDC 3230ns */
	0x3ffff035,  /* VEC_VBI_ENC0 31500ns */
	0x3ffff036,  /* VEC_VBI_ENC1 31500ns */
	0xbffff06e,  /* V3D_0 RR */
	0xbffff075,  /* V3D_1 RR */
	0xbffff06f,  /* M2MC RR */
	0xbffff070,  /* M2MC1 RR */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x800f103f,  /* MCP_RD_HIGH RR 750ns */
	0x80000066,  /* MCP_RD_LOW RR 0ns */
	0x801c8040,  /* MCP_WR_HIGH RR 1500ns */
	0x80000068,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff067,  /* SHVD0_PFRI RR 9ns */
	0x80000065,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x80001063,  /* VICE_PFRI off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff072,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff071,  /* MEMC_MSA RR */
	0xbffff076,  /* MEMC_DIS0 RR */
	0xbffff077,  /* MEMC_DIS1 RR */
	0xbffff06b,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e102d,  /* REFRESH 7812.5ns */
};

static uint32_t aul7252_4K1tRts_Memc1[] =
{
	0x3ffff002,  /* XPT_WR_RS off */
	0x3ffff039,  /* XPT_WR_XC off */
	0x3ffff00c,  /* XPT_WR_CDB off */
	0x3ffff042,  /* XPT_WR_ITB_MSG off */
	0x3ffff019,  /* XPT_RD_RS off */
	0x3ffff025,  /* XPT_RD_XC_RMX_MSG off */
	0x3ffff00b,  /* XPT_RD_XC_RAVE off */
	0x3ffff03d,  /* XPT_RD_PB off */
	0x8076103c,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c04a,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd007,  /* GENET0_WR RR 2370ns */
	0x80c1c043,  /* GENET0_RD RR 10150ns */
	0x8067501c,  /* GENET1_WR RR 5110ns */
	0x80c1c044,  /* GENET1_RD RR 10150ns */
	0x802fd008,  /* GENET2_WR RR 2370ns */
	0x80c1c045,  /* GENET2_RD RR 10150ns */
	0x84312033,  /* MOCA_MIPS RR 53000ns */
	0x804c903a,  /* SATA RR 4015ns */
	0x804c903b,  /* SATA_1 RR 4015ns */
	0x3ffff02d,  /* MCIF2_RD off */
	0x3ffff02f,  /* MCIF2_WR off */
	0x3ffff06e,  /* UNASSIGNED off */
	0x83f46031,  /* BSP RR 50000ns */
	0x8082203e,  /* SAGE RR 6820ns */
	0x84b3604c,  /* FLASH_DMA RR 63000ns */
	0x8109004b,  /* HIF_PCIe RR 13880ns */
	0x84b3604e,  /* SDIO_EMMC RR 63000ns */
	0x84b3604d,  /* SDIO_CARD RR 63000ns */
	0x3ffff06a,  /* TPCAP off */
	0x3ffff02e,  /* MCIF_RD off */
	0x3ffff030,  /* MCIF_WR off */
	0x3ffff05f,  /* UART_DMA_RD off */
	0x3ffff060,  /* UART_DMA_WR off */
	0x80ca4046,  /* USB_HI_0 RR 10593ns */
	0xbffff062,  /* USB_LO_0 RR */
	0x81053049,  /* USB_X_WRITE_0 RR 13680ns */
	0x81053048,  /* USB_X_READ_0 RR 13680ns */
	0x8082803f,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca4047,  /* USB_HI_1 RR 10593ns */
	0xbffff063,  /* USB_LO_1 RR */
	0x3ffff010,  /* RAAGA off */
	0x3ffff001,  /* RAAGA_1 off */
	0x3ffff01e,  /* RAAGA1 off */
	0x3ffff000,  /* RAAGA1_1 off */
	0x3ffff01f,  /* AUD_AIO off */
	0x8056704f,  /* VICE_CME_RMB_CMB RR 4533.33333333333ns */
	0x82b4b054,  /* VICE_CME_CSC RR 36266.6666666667ns */
	0x809ec050,  /* VICE_FME_CSC RR 8318.66666666667ns */
	0x804f2052,  /* VICE_FME_Luma_CMB RR 4148ns */
	0x809ec051,  /* VICE_FME_Chroma_CMB RR 8318.66666666667ns */
	0x3ffff028,  /* VICE_SG off */
	0x8000105a,  /* VICE_DBLK RR 10ns */
	0x3ffff034,  /* VICE_CABAC0 off */
	0x3ffff032,  /* VICE_CABAC1 off */
	0x803a2053,  /* VICE_ARCSS0 RR 3050ns */
	0x8095d020,  /* VICE_VIP0_INST0 RR 7407ns */
	0x81c1d029,  /* VICE_VIP1_INST0 RR 22222ns */
	0x8095d021,  /* VICE_VIP0_INST1 RR 7407ns */
	0x81c1d02a,  /* VICE_VIP1_INST1 RR 22222ns */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x3ffff074,  /* UNASSIGNED off */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x3ffff07c,  /* UNASSIGNED off */
	0x3ffff058,  /* SHVD0_DBLK_0 off */
	0x3ffff056,  /* SHVD0_DBLK_1 off */
	0x3ffff038,  /* SHVD0_ILCPU off */
	0x3ffff041,  /* SHVD0_OLCPU off */
	0x3ffff01b,  /* SHVD0_CAB off */
	0x3ffff018,  /* SHVD0_ILSI off */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x80077057,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a055,  /* HVD1_DBLK_1 RR 10ns */
	0x3ffff037,  /* HVD1_ILCPU off */
	0x3ffff040,  /* HVD1_OLCPU off */
	0x3ffff01a,  /* HVD1_CAB off */
	0x3ffff017,  /* HVD1_ILSI off */
	0xbffff069,  /* SID RR */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff006,  /* MADR1_RD off */
	0x3ffff00a,  /* MADR1_QM off */
	0x3ffff014,  /* MADR1_WR off */
	0x3ffff088,  /* UNASSIGNED off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff015,  /* BVNF_MFD0_0 off */
	0x3ffff003,  /* BVNF_MFD0_1 off */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x004ae016,  /* BVNF_MFD2_0 3704ns */
	0x0031d009,  /* BVNF_MFD2_1 2469ns */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x001dd004,  /* BVNF_VFD0 1480ns */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff026,  /* BVNF_VFD2 off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff00e,  /* BVNF_VFD5 off */
	0x3ffff099,  /* UNASSIGNED off */
	0x3ffff09a,  /* UNASSIGNED off */
	0x001dd005,  /* BVNF_CAP0 1480ns */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff027,  /* BVNF_CAP2 off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x3ffff00f,  /* BVNF_CAP5 off */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x0036500d,  /* BVNF_GFD0 2691.2ns */
	0x010dc024,  /* BVNF_GFD1 13330ns */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff01d,  /* BVNF_GFD4 off */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x00450013,  /* MCVP0 3415ns */
	0x00450012,  /* MCVP1 3415ns */
	0x009b5022,  /* MCVP_QM 7676ns */
	0x3ffff011,  /* BVNF_RDC off */
	0x3ffff02b,  /* VEC_VBI_ENC0 off */
	0x3ffff02c,  /* VEC_VBI_ENC1 off */
	0xbffff064,  /* V3D_0 RR */
	0xbffff06b,  /* V3D_1 RR */
	0xbffff065,  /* M2MC RR */
	0xbffff066,  /* M2MC1 RR */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x800f1035,  /* MCP_RD_HIGH RR 750ns */
	0x8000005c,  /* MCP_RD_LOW RR 0ns */
	0x801c8036,  /* MCP_WR_HIGH RR 1500ns */
	0x8000005e,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff05d,  /* SHVD0_PFRI off */
	0x8000005b,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x80001059,  /* VICE_PFRI RR 10ns */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff068,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff067,  /* MEMC_MSA RR */
	0xbffff06c,  /* MEMC_DIS0 RR */
	0xbffff06d,  /* MEMC_DIS1 RR */
	0xbffff061,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e1023,  /* REFRESH 7812.5ns */
};

static uint32_t aul7252_4KstbRts_Memc1[] =
{
	0x3ffff002,  /* XPT_WR_RS off */
	0x3ffff030,  /* XPT_WR_XC off */
	0x3ffff00b,  /* XPT_WR_CDB off */
	0x3ffff039,  /* XPT_WR_ITB_MSG off */
	0x3ffff016,  /* XPT_RD_RS off */
	0x3ffff01f,  /* XPT_RD_XC_RMX_MSG off */
	0x3ffff00a,  /* XPT_RD_XC_RAVE off */
	0x3ffff034,  /* XPT_RD_PB off */
	0x80761033,  /* XPT_WR_MEMDMA RR 6188ns */
	0x8106c041,  /* XPT_RD_MEMDMA RR 13760ns */
	0x802fd007,  /* GENET0_WR RR 2370ns */
	0x80c1c03a,  /* GENET0_RD RR 10150ns */
	0x80675019,  /* GENET1_WR RR 5110ns */
	0x80c1c03b,  /* GENET1_RD RR 10150ns */
	0x802fd008,  /* GENET2_WR RR 2370ns */
	0x80c1c03c,  /* GENET2_RD RR 10150ns */
	0x8431202b,  /* MOCA_MIPS RR 53000ns */
	0x804c9031,  /* SATA RR 4015ns */
	0x804c9032,  /* SATA_1 RR 4015ns */
	0x3ffff026,  /* MCIF2_RD off */
	0x3ffff028,  /* MCIF2_WR off */
	0x3ffff05d,  /* UNASSIGNED off */
	0x83f4602a,  /* BSP RR 50000ns */
	0x80822035,  /* SAGE RR 6820ns */
	0x84b36043,  /* FLASH_DMA RR 63000ns */
	0x81090042,  /* HIF_PCIe RR 13880ns */
	0x84b36045,  /* SDIO_EMMC RR 63000ns */
	0x84b36044,  /* SDIO_CARD RR 63000ns */
	0x3ffff059,  /* TPCAP off */
	0x3ffff027,  /* MCIF_RD off */
	0x3ffff029,  /* MCIF_WR off */
	0x3ffff04e,  /* UART_DMA_RD off */
	0x3ffff04f,  /* UART_DMA_WR off */
	0x80ca403d,  /* USB_HI_0 RR 10593ns */
	0xbffff051,  /* USB_LO_0 RR */
	0x81053040,  /* USB_X_WRITE_0 RR 13680ns */
	0x8105303f,  /* USB_X_READ_0 RR 13680ns */
	0x80828036,  /* USB_X_CTRL_0 RR 6840ns */
	0x80ca403e,  /* USB_HI_1 RR 10593ns */
	0xbffff052,  /* USB_LO_1 RR */
	0x3ffff00e,  /* RAAGA off */
	0x3ffff001,  /* RAAGA_1 off */
	0x3ffff01a,  /* RAAGA1 off */
	0x3ffff000,  /* RAAGA1_1 off */
	0x3ffff01b,  /* AUD_AIO off */
	0x3ffff05e,  /* UNASSIGNED off */
	0x3ffff05f,  /* UNASSIGNED off */
	0x3ffff060,  /* UNASSIGNED off */
	0x3ffff061,  /* UNASSIGNED off */
	0x3ffff062,  /* UNASSIGNED off */
	0x3ffff063,  /* UNASSIGNED off */
	0x3ffff064,  /* UNASSIGNED off */
	0x3ffff065,  /* UNASSIGNED off */
	0x3ffff066,  /* UNASSIGNED off */
	0x3ffff067,  /* UNASSIGNED off */
	0x3ffff068,  /* UNASSIGNED off */
	0x3ffff069,  /* UNASSIGNED off */
	0x3ffff06a,  /* UNASSIGNED off */
	0x3ffff06b,  /* UNASSIGNED off */
	0x3ffff06c,  /* UNASSIGNED off */
	0x3ffff06d,  /* UNASSIGNED off */
	0x3ffff06e,  /* UNASSIGNED off */
	0x3ffff06f,  /* UNASSIGNED off */
	0x3ffff070,  /* UNASSIGNED off */
	0x3ffff071,  /* UNASSIGNED off */
	0x3ffff072,  /* UNASSIGNED off */
	0x3ffff073,  /* UNASSIGNED off */
	0x3ffff074,  /* UNASSIGNED off */
	0x3ffff075,  /* UNASSIGNED off */
	0x3ffff076,  /* UNASSIGNED off */
	0x3ffff077,  /* UNASSIGNED off */
	0x3ffff078,  /* UNASSIGNED off */
	0x3ffff079,  /* UNASSIGNED off */
	0x3ffff049,  /* SHVD0_DBLK_0 off */
	0x3ffff047,  /* SHVD0_DBLK_1 off */
	0x3ffff02f,  /* SHVD0_ILCPU off */
	0x3ffff038,  /* SHVD0_OLCPU off */
	0x3ffff018,  /* SHVD0_CAB off */
	0x3ffff015,  /* SHVD0_ILSI off */
	0x3ffff07a,  /* UNASSIGNED off */
	0x3ffff07b,  /* UNASSIGNED off */
	0x80077048,  /* HVD1_DBLK_0 RR 10ns */
	0x8003a046,  /* HVD1_DBLK_1 RR 10ns */
	0x3ffff02e,  /* HVD1_ILCPU off */
	0x3ffff037,  /* HVD1_OLCPU off */
	0x3ffff017,  /* HVD1_CAB off */
	0x3ffff014,  /* HVD1_ILSI off */
	0xbffff058,  /* SID RR */
	0x3ffff07c,  /* UNASSIGNED off */
	0x3ffff07d,  /* UNASSIGNED off */
	0x3ffff07e,  /* UNASSIGNED off */
	0x3ffff07f,  /* UNASSIGNED off */
	0x3ffff080,  /* UNASSIGNED off */
	0x3ffff081,  /* UNASSIGNED off */
	0x3ffff082,  /* UNASSIGNED off */
	0x3ffff083,  /* UNASSIGNED off */
	0x3ffff084,  /* UNASSIGNED off */
	0x3ffff085,  /* UNASSIGNED off */
	0x3ffff086,  /* UNASSIGNED off */
	0x3ffff087,  /* UNASSIGNED off */
	0x3ffff088,  /* UNASSIGNED off */
	0x3ffff089,  /* UNASSIGNED off */
	0x3ffff08a,  /* UNASSIGNED off */
	0x3ffff08b,  /* UNASSIGNED off */
	0x3ffff08c,  /* UNASSIGNED off */
	0x3ffff08d,  /* UNASSIGNED off */
	0x3ffff012,  /* BVNF_MFD0_0 off */
	0x3ffff003,  /* BVNF_MFD0_1 off */
	0x004ae013,  /* BVNF_MFD1_0 3704ns */
	0x0031d009,  /* BVNF_MFD1_1 2469ns */
	0x3ffff08e,  /* UNASSIGNED off */
	0x3ffff08f,  /* UNASSIGNED off */
	0x3ffff090,  /* UNASSIGNED off */
	0x3ffff091,  /* UNASSIGNED off */
	0x3ffff092,  /* UNASSIGNED off */
	0x3ffff093,  /* UNASSIGNED off */
	0x3ffff094,  /* UNASSIGNED off */
	0x3ffff095,  /* UNASSIGNED off */
	0x001dd004,  /* BVNF_VFD0 1480ns */
	0x3ffff005,  /* BVNF_VFD1 off */
	0x3ffff021,  /* BVNF_VFD2 off */
	0x3ffff020,  /* BVNF_VFD3 off */
	0x3ffff096,  /* UNASSIGNED off */
	0x3ffff097,  /* UNASSIGNED off */
	0x3ffff098,  /* UNASSIGNED off */
	0x3ffff099,  /* UNASSIGNED off */
	0x001dd006,  /* BVNF_CAP0 1480ns */
	0x3ffff00d,  /* BVNF_CAP1 off */
	0x3ffff022,  /* BVNF_CAP2 off */
	0x3ffff025,  /* BVNF_CAP3 off */
	0x3ffff09a,  /* UNASSIGNED off */
	0x3ffff09b,  /* UNASSIGNED off */
	0x3ffff09c,  /* UNASSIGNED off */
	0x3ffff09d,  /* UNASSIGNED off */
	0x0036500c,  /* BVNF_GFD0 2691.2ns */
	0x3ffff01e,  /* BVNF_GFD1 off */
	0x3ffff09e,  /* UNASSIGNED off */
	0x3ffff09f,  /* UNASSIGNED off */
	0x3ffff0a0,  /* UNASSIGNED off */
	0x3ffff0a1,  /* UNASSIGNED off */
	0x3ffff0a2,  /* UNASSIGNED off */
	0x00450011,  /* MCVP0 3415ns */
	0x00450010,  /* MCVP1 3415ns */
	0x009b501c,  /* MCVP_QM 7676ns */
	0x3ffff00f,  /* BVNF_RDC off */
	0x3ffff023,  /* VEC_VBI_ENC0 off */
	0x3ffff024,  /* VEC_VBI_ENC1 off */
	0xbffff053,  /* V3D_0 RR */
	0xbffff05a,  /* V3D_1 RR */
	0xbffff054,  /* M2MC RR */
	0xbffff055,  /* M2MC1 RR */
	0x3ffff0a3,  /* UNASSIGNED off */
	0x3ffff0a4,  /* UNASSIGNED off */
	0x3ffff0a5,  /* UNASSIGNED off */
	0x3ffff0a6,  /* UNASSIGNED off */
	0x3ffff0a7,  /* UNASSIGNED off */
	0x3ffff0a8,  /* UNASSIGNED off */
	0x3ffff0a9,  /* UNASSIGNED off */
	0x3ffff0aa,  /* UNASSIGNED off */
	0x3ffff0ab,  /* UNASSIGNED off */
	0x3ffff0ac,  /* UNASSIGNED off */
	0x3ffff0ad,  /* UNASSIGNED off */
	0x3ffff0ae,  /* UNASSIGNED off */
	0x3ffff0af,  /* UNASSIGNED off */
	0x3ffff0b0,  /* UNASSIGNED off */
	0x3ffff0b1,  /* UNASSIGNED off */
	0x3ffff0b2,  /* UNASSIGNED off */
	0x3ffff0b3,  /* UNASSIGNED off */
	0x3ffff0b4,  /* UNASSIGNED off */
	0x3ffff0b5,  /* UNASSIGNED off */
	0x3ffff0b6,  /* UNASSIGNED off */
	0x3ffff0b7,  /* UNASSIGNED off */
	0x3ffff0b8,  /* UNASSIGNED off */
	0x3ffff0b9,  /* UNASSIGNED off */
	0x3ffff0ba,  /* UNASSIGNED off */
	0x3ffff0bb,  /* UNASSIGNED off */
	0x3ffff0bc,  /* UNASSIGNED off */
	0x3ffff0bd,  /* UNASSIGNED off */
	0x3ffff0be,  /* UNASSIGNED off */
	0x3ffff0bf,  /* UNASSIGNED off */
	0x3ffff0c0,  /* UNASSIGNED off */
	0x3ffff0c1,  /* UNASSIGNED off */
	0x3ffff0c2,  /* UNASSIGNED off */
	0x3ffff0c3,  /* UNASSIGNED off */
	0x3ffff0c4,  /* UNASSIGNED off */
	0x3ffff0c5,  /* UNASSIGNED off */
	0x3ffff0c6,  /* UNASSIGNED off */
	0x3ffff0c7,  /* UNASSIGNED off */
	0x3ffff0c8,  /* UNASSIGNED off */
	0x3ffff0c9,  /* UNASSIGNED off */
	0x3ffff0ca,  /* UNASSIGNED off */
	0x3ffff0cb,  /* UNASSIGNED off */
	0x3ffff0cc,  /* UNASSIGNED off */
	0x3ffff0cd,  /* UNASSIGNED off */
	0x3ffff0ce,  /* UNASSIGNED off */
	0x3ffff0cf,  /* UNASSIGNED off */
	0x3ffff0d0,  /* UNASSIGNED off */
	0x3ffff0d1,  /* UNASSIGNED off */
	0x3ffff0d2,  /* UNASSIGNED off */
	0x3ffff0d3,  /* UNASSIGNED off */
	0x800f102c,  /* MCP_RD_HIGH RR 750ns */
	0x8000004b,  /* MCP_RD_LOW RR 0ns */
	0x801c802d,  /* MCP_WR_HIGH RR 1500ns */
	0x8000004d,  /* MCP_WR_LOW RR 0ns */
	0x3ffff0d4,  /* UNASSIGNED off */
	0x3ffff0d5,  /* UNASSIGNED off */
	0x3ffff0d6,  /* UNASSIGNED off */
	0x3ffff0d7,  /* UNASSIGNED off */
	0x3ffff0d8,  /* UNASSIGNED off */
	0x3ffff0d9,  /* UNASSIGNED off */
	0x3ffff0da,  /* UNASSIGNED off */
	0x3ffff0db,  /* UNASSIGNED off */
	0x3ffff0dc,  /* UNASSIGNED off */
	0x3ffff0dd,  /* UNASSIGNED off */
	0x3ffff0de,  /* UNASSIGNED off */
	0x3ffff0df,  /* UNASSIGNED off */
	0x3ffff04c,  /* SHVD0_PFRI off */
	0x8000004a,  /* HVD1_PFRI RR 0ns */
	0x3ffff0e0,  /* UNASSIGNED off */
	0x3ffff0e1,  /* UNASSIGNED off */
	0x3ffff0e2,  /* UNASSIGNED off */
	0x3ffff0e3,  /* UNASSIGNED off */
	0x3ffff0e4,  /* UNASSIGNED off */
	0x3ffff0e5,  /* UNASSIGNED off */
	0x3ffff0e6,  /* UNASSIGNED off */
	0x3ffff0e7,  /* UNASSIGNED off */
	0x3ffff0e8,  /* UNASSIGNED off */
	0x3ffff0e9,  /* UNASSIGNED off */
	0x3ffff0ea,  /* UNASSIGNED off */
	0x3ffff0eb,  /* UNASSIGNED off */
	0x3ffff0ec,  /* UNASSIGNED off */
	0x3ffff0ed,  /* UNASSIGNED off */
	0x3ffff0ee,  /* UNASSIGNED off */
	0x3ffff0ef,  /* UNASSIGNED off */
	0x3ffff0f0,  /* UNASSIGNED off */
	0x3ffff0f1,  /* UNASSIGNED off */
	0x3ffff0f2,  /* UNASSIGNED off */
	0x3ffff0f3,  /* UNASSIGNED off */
	0x3ffff0f4,  /* UNASSIGNED off */
	0x3ffff0f5,  /* UNASSIGNED off */
	0x3ffff0f6,  /* UNASSIGNED off */
	0x3ffff0f7,  /* UNASSIGNED off */
	0x3ffff0f8,  /* UNASSIGNED off */
	0x3ffff0f9,  /* UNASSIGNED off */
	0x3ffff0fa,  /* UNASSIGNED off */
	0x3ffff0fb,  /* UNASSIGNED off */
	0x3ffff0fc,  /* UNASSIGNED off */
	0x3ffff0fd,  /* UNASSIGNED off */
	0xbffff057,  /* MEMC_TRACELOG RR */
	0x3ffff0fe,  /* UNASSIGNED off */
	0x3ffff0ff,  /* UNASSIGNED off */
	0xbffff056,  /* MEMC_MSA RR */
	0xbffff05b,  /* MEMC_DIS0 RR */
	0xbffff05c,  /* MEMC_DIS1 RR */
	0xbffff050,  /* MEMC_DRAM_INIT_ZQCS RR */
	0x009e101d,  /* REFRESH 7812.5ns */
};
#endif

static uint32_t aul7445SingleEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x0016c003,  /* XPT_WR_RS 1130ns */
  0x802d0063,  /* XPT_WR_XC RR 2230ns */
  0x80355022,  /* XPT_WR_CDB RR 2640ns */
  0x80907073,  /* XPT_WR_ITB_MSG RR 7140ns */
  0x8059f03a,  /* XPT_RD_RS RR 4450ns */
  0x8136b04e,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
  0x80355023,  /* XPT_RD_XC_RAVE RR 2640ns */
  0x807b906b,  /* XPT_RD_PB RR 6110ns */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x87fff09e,  /* TPCAP RR */
  0x02ed2058,  /* MCIF_RD 37000ns */
  0x02ed2057,  /* MCIF_WR 37000ns */
  0x87fff094,  /* UART_DMA_RD RR */
  0x87fff095,  /* UART_DMA_WR RR */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x003ca02c,  /* RAAGA 3000ns */
  0x00142002,  /* RAAGA_1 1000ns */
  0x00796042,  /* RAAGA1 6000ns */
  0x00142001,  /* RAAGA1_1 1000ns */
  0x008c6043,  /* AUD_AIO 6940ns */
  0x80261062,  /* VICE_CME_RMB_CMB RR 2000ns */
  0x81318079,  /* VICE_CME_CSC RR 16000ns */
  0x80460067,  /* VICE_FME_CSC RR 3670ns */
  0x80460069,  /* VICE_FME_Luma_CMB RR 3670ns */
  0x80460068,  /* VICE_FME_Chroma_CMB RR 3670ns */
  0x80a5504b,  /* VICE_SG RR 8170ns */
  0x80001085,  /* VICE_DBLK RR 10ns */
  0x833e205a,  /* VICE_CABAC0 RR 41000ns */
  0x81d5a052,  /* VICE_CABAC1 RR 23200ns */
  0x803a2066,  /* VICE_ARCSS0 RR 3050ns */
  0x8025500e,  /* VICE_VIP0_INST0 RR 1850ns */
  0x8095e046,  /* VICE_VIP1_INST0 RR 7410ns */
  0x8025500f,  /* VICE_VIP0_INST1 RR 1850ns */
  0x8095e047,  /* VICE_VIP1_INST1 RR 7410ns */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x80a5504a,  /* VICE1_SG RR 8170ns */
  0x07fff08f,  /* VICE1_DBLK off */
  0x833e2059,  /* VICE1_CABAC0 RR 41000ns */
  0x81d5a051,  /* VICE1_CABAC1 RR 23200ns */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x801d4061,  /* SHVD0_ILCPU RR 1451ns */
  0x8080f06c,  /* SHVD0_OLCPU RR 6756ns */
  0x005e503d,  /* SHVD0_CAB 4666ns */
  0x0054e039,  /* SHVD0_ILSI 4199ns */
  0x802e9012,  /* SHVD0_BLCPU RR 2308ns */
  0x8065003e,  /* SHVD0_BLSI RR 4996ns */
  0x80147088,  /* HVD1_DBLK_0 RR 10ns */
  0x800a2087,  /* HVD1_DBLK_1 RR 10ns */
  0x801d405f,  /* HVD1_ILCPU RR 1451ns */
  0x8083806e,  /* HVD1_OLCPU RR 6893ns */
  0x005e503b,  /* HVD1_CAB 4666ns */
  0x0054e037,  /* HVD1_ILSI 4199ns */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x801d4060,  /* HVD2_ILCPU RR 1451ns */
  0x8083806f,  /* HVD2_OLCPU RR 6893ns */
  0x005e503c,  /* HVD2_CAB 4666ns */
  0x0054e038,  /* HVD2_ILSI 4199ns */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x004ae035,  /* BVNF_MFD4_0 3704ns */
  0x0031d020,  /* BVNF_MFD4_1 2469ns */
  0x004ae036,  /* BVNF_MFD5_0 3704ns */
  0x0031d021,  /* BVNF_MFD5_1 2469ns */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x0031a015,  /* BVNF_GFD3 2459.142ns */
  0x0031a016,  /* BVNF_GFD4 2459.142ns */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x027dc053,  /* VEC_VBI_ENC0 31500ns */
  0x027dc054,  /* VEC_VBI_ENC1 31500ns */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x80000091,  /* HVD1_PFRI RR 9ns */
  0x07fff08b,  /* HVD2_PFRI off */
  0x80001086,  /* VICE_PFRI RR 10ns */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048  /* REFRESH 7812.5ns */
};

static uint32_t aul7445SingleEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x80261081,  /* VICE1_CME_RMB_CMB RR 2000ns */
  0x81318080,  /* VICE1_CME_CSC RR 16000ns */
  0x80460082,  /* VICE1_FME_CSC RR 3670ns */
  0x80460084,  /* VICE1_FME_Luma_CMB RR 3670ns */
  0x80460083,  /* VICE1_FME_Chroma_CMB RR 3670ns */
  0x07fff04a,  /* VICE1_SG off */
  0x8000108f,  /* VICE1_DBLK RR 10ns */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x803a207f,  /* VICE1_ARCSS0 RR 3050ns */
  0x8025500c,  /* VICE1_VIP0_INST0 RR 1850ns */
  0x8095e044,  /* VICE1_VIP1_INST0 RR 7410ns */
  0x8025500d,  /* VICE1_VIP0_INST1 RR 1850ns */
  0x8095e045,  /* VICE1_VIP1_INST1 RR 7410ns */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x80147089,  /* HVD2_DBLK_0 RR 10ns */
  0x800a208a,  /* HVD2_DBLK_1 RR 10ns */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x0023600b,  /* MADR_RD 1755ns */
  0x0031d01c,  /* MADR_QM 2469ns */
  0x0046f030,  /* MADR_WR 3511ns */
  0x00236008,  /* MADR1_RD 1755ns */
  0x0031d019,  /* MADR1_QM 2469ns */
  0x0046f02d,  /* MADR1_WR 3511ns */
  0x00236009,  /* MADR2_RD 1755ns */
  0x0031d01a,  /* MADR2_QM 2469ns */
  0x0046f02e,  /* MADR2_WR 3511ns */
  0x0023600a,  /* MADR3_RD 1755ns */
  0x0031d01b,  /* MADR3_QM 2469ns */
  0x0046f02f,  /* MADR3_WR 3511ns */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x004ae033,  /* BVNF_MFD2_0 3704ns */
  0x0031d01e,  /* BVNF_MFD2_1 2469ns */
  0x004ae034,  /* BVNF_MFD3_0 3704ns */
  0x0031d01f,  /* BVNF_MFD3_1 2469ns */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x001dd007,  /* BVNF_VFD0 1480ns */
  0x0077c041,  /* BVNF_VFD1 5920ns */
  0x02829055,  /* BVNF_VFD2 31740ns */
  0x02829056,  /* BVNF_VFD3 31740ns */
  0x003bd028,  /* BVNF_VFD4 2960ns */
  0x003bd029,  /* BVNF_VFD5 2960ns */
  0x003bd02a,  /* BVNF_VFD6 2960ns */
  0x003bd02b,  /* BVNF_VFD7 2960ns */
  0x001dd006,  /* BVNF_CAP0 1480ns */
  0x0077c040,  /* BVNF_CAP1 5920ns */
  0x0141304f,  /* BVNF_CAP2 15870ns */
  0x01413050,  /* BVNF_CAP3 15870ns */
  0x003bd024,  /* BVNF_CAP4 2960ns */
  0x003bd025,  /* BVNF_CAP5 2960ns */
  0x003bd026,  /* BVNF_CAP6 2960ns */
  0x003bd027,  /* BVNF_CAP7 2960ns */
  0x0010e000,  /* BVNF_GFD0 841ns */
  0x00b3b04c,  /* BVNF_GFD1 8880ns */
  0x00b3b04d,  /* BVNF_GFD2 8880ns */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x0031a017,  /* BVNF_GFD5 2459.142ns */
  0x0031a018,  /* BVNF_GFD6 2459.142ns */
  0x0027e010,  /* MCVP0 1976ns */
  0x0027e011,  /* MCVP1 1976ns */
  0x009fd049,  /* MCVP_QM 7900ns */
  0x001c3005,  /* BVNF_RDC 1400ns */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x07fff091,  /* HVD1_PFRI off */
  0x8000008b,  /* HVD2_PFRI RR 9ns */
  0x07fff086,  /* VICE_PFRI off */
  0x80001090,  /* VICE1_PFRI RR 10ns */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048   /* REFRESH 7812.5ns */
};

static uint32_t aul7445SingleEncodeRts_Memc2[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x07fff04a,  /* VICE1_SG off */
  0x07fff08f,  /* VICE1_DBLK off */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x800cd08c,  /* SHVD0_DBLK_0 RR 10ns */
  0x8006508d,  /* SHVD0_DBLK_1 RR 10ns */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x004ae031,  /* BVNF_MFD0_0 3704ns */
  0x0018e004,  /* BVNF_MFD0_1 1235ns */
  0x004ae032,  /* BVNF_MFD1_0 3704ns */
  0x0031d01d,  /* BVNF_MFD1_1 2469ns */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x8000008e,  /* SHVD0_PFRI RR 9ns */
  0x07fff091,  /* HVD1_PFRI off */
  0x07fff08b,  /* HVD2_PFRI off */
  0x07fff086,  /* VICE_PFRI off */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048  /* REFRESH 7812.5ns */
};

static uint32_t aul7445DualEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x0016c003,  /* XPT_WR_RS 1130ns */
  0x802d0063,  /* XPT_WR_XC RR 2230ns */
  0x80355022,  /* XPT_WR_CDB RR 2640ns */
  0x80907073,  /* XPT_WR_ITB_MSG RR 7140ns */
  0x8059f03a,  /* XPT_RD_RS RR 4450ns */
  0x8136b04e,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
  0x80355023,  /* XPT_RD_XC_RAVE RR 2640ns */
  0x807b906b,  /* XPT_RD_PB RR 6110ns */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x87fff09e,  /* TPCAP RR */
  0x02ed2058,  /* MCIF_RD 37000ns */
  0x02ed2057,  /* MCIF_WR 37000ns */
  0x87fff094,  /* UART_DMA_RD RR */
  0x87fff095,  /* UART_DMA_WR RR */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x003ca02c,  /* RAAGA 3000ns */
  0x00142002,  /* RAAGA_1 1000ns */
  0x00796042,  /* RAAGA1 6000ns */
  0x00142001,  /* RAAGA1_1 1000ns */
  0x008c6043,  /* AUD_AIO 6940ns */
  0x80261062,  /* VICE_CME_RMB_CMB RR 2000ns */
  0x81318079,  /* VICE_CME_CSC RR 16000ns */
  0x80460067,  /* VICE_FME_CSC RR 3670ns */
  0x80460069,  /* VICE_FME_Luma_CMB RR 3670ns */
  0x80460068,  /* VICE_FME_Chroma_CMB RR 3670ns */
  0x80a5504b,  /* VICE_SG RR 8170ns */
  0x80001085,  /* VICE_DBLK RR 10ns */
  0x833e205a,  /* VICE_CABAC0 RR 41000ns */
  0x81d5a052,  /* VICE_CABAC1 RR 23200ns */
  0x803a2066,  /* VICE_ARCSS0 RR 3050ns */
  0x8025500e,  /* VICE_VIP0_INST0 RR 1850ns */
  0x8095e046,  /* VICE_VIP1_INST0 RR 7410ns */
  0x8025500f,  /* VICE_VIP0_INST1 RR 1850ns */
  0x8095e047,  /* VICE_VIP1_INST1 RR 7410ns */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x80a5504a,  /* VICE1_SG RR 8170ns */
  0x07fff08f,  /* VICE1_DBLK off */
  0x833e2059,  /* VICE1_CABAC0 RR 41000ns */
  0x81d5a051,  /* VICE1_CABAC1 RR 23200ns */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x801d4061,  /* SHVD0_ILCPU RR 1451ns */
  0x8080f06c,  /* SHVD0_OLCPU RR 6756ns */
  0x005e503d,  /* SHVD0_CAB 4666ns */
  0x0054e039,  /* SHVD0_ILSI 4199ns */
  0x802e9012,  /* SHVD0_BLCPU RR 2308ns */
  0x8065003e,  /* SHVD0_BLSI RR 4996ns */
  0x80147088,  /* HVD1_DBLK_0 RR 10ns */
  0x800a2087,  /* HVD1_DBLK_1 RR 10ns */
  0x801d405f,  /* HVD1_ILCPU RR 1451ns */
  0x8083806e,  /* HVD1_OLCPU RR 6893ns */
  0x005e503b,  /* HVD1_CAB 4666ns */
  0x0054e037,  /* HVD1_ILSI 4199ns */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x801d4060,  /* HVD2_ILCPU RR 1451ns */
  0x8083806f,  /* HVD2_OLCPU RR 6893ns */
  0x005e503c,  /* HVD2_CAB 4666ns */
  0x0054e038,  /* HVD2_ILSI 4199ns */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x004ae035,  /* BVNF_MFD4_0 3704ns */
  0x0031d020,  /* BVNF_MFD4_1 2469ns */
  0x004ae036,  /* BVNF_MFD5_0 3704ns */
  0x0031d021,  /* BVNF_MFD5_1 2469ns */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x0031a015,  /* BVNF_GFD3 2459.142ns */
  0x0031a016,  /* BVNF_GFD4 2459.142ns */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x027dc053,  /* VEC_VBI_ENC0 31500ns */
  0x027dc054,  /* VEC_VBI_ENC1 31500ns */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x80000091,  /* HVD1_PFRI RR 9ns */
  0x07fff08b,  /* HVD2_PFRI off */
  0x80001086,  /* VICE_PFRI RR 10ns */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048   /* REFRESH 7812.5ns */
};

static uint32_t aul7445DualEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x80261081,  /* VICE1_CME_RMB_CMB RR 2000ns */
  0x81318080,  /* VICE1_CME_CSC RR 16000ns */
  0x80460082,  /* VICE1_FME_CSC RR 3670ns */
  0x80460084,  /* VICE1_FME_Luma_CMB RR 3670ns */
  0x80460083,  /* VICE1_FME_Chroma_CMB RR 3670ns */
  0x07fff04a,  /* VICE1_SG off */
  0x8000108f,  /* VICE1_DBLK RR 10ns */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x803a207f,  /* VICE1_ARCSS0 RR 3050ns */
  0x8025500c,  /* VICE1_VIP0_INST0 RR 1850ns */
  0x8095e044,  /* VICE1_VIP1_INST0 RR 7410ns */
  0x8025500d,  /* VICE1_VIP0_INST1 RR 1850ns */
  0x8095e045,  /* VICE1_VIP1_INST1 RR 7410ns */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x80147089,  /* HVD2_DBLK_0 RR 10ns */
  0x800a208a,  /* HVD2_DBLK_1 RR 10ns */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x0023600b,  /* MADR_RD 1755ns */
  0x0031d01c,  /* MADR_QM 2469ns */
  0x0046f030,  /* MADR_WR 3511ns */
  0x00236008,  /* MADR1_RD 1755ns */
  0x0031d019,  /* MADR1_QM 2469ns */
  0x0046f02d,  /* MADR1_WR 3511ns */
  0x00236009,  /* MADR2_RD 1755ns */
  0x0031d01a,  /* MADR2_QM 2469ns */
  0x0046f02e,  /* MADR2_WR 3511ns */
  0x0023600a,  /* MADR3_RD 1755ns */
  0x0031d01b,  /* MADR3_QM 2469ns */
  0x0046f02f,  /* MADR3_WR 3511ns */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x004ae033,  /* BVNF_MFD2_0 3704ns */
  0x0031d01e,  /* BVNF_MFD2_1 2469ns */
  0x004ae034,  /* BVNF_MFD3_0 3704ns */
  0x0031d01f,  /* BVNF_MFD3_1 2469ns */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x003bd028,  /* BVNF_VFD4 2960ns */
  0x003bd029,  /* BVNF_VFD5 2960ns */
  0x003bd02a,  /* BVNF_VFD6 2960ns */
  0x003bd02b,  /* BVNF_VFD7 2960ns */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x003bd024,  /* BVNF_CAP4 2960ns */
  0x003bd025,  /* BVNF_CAP5 2960ns */
  0x003bd026,  /* BVNF_CAP6 2960ns */
  0x003bd027,  /* BVNF_CAP7 2960ns */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x0031a017,  /* BVNF_GFD5 2459.142ns */
  0x0031a018,  /* BVNF_GFD6 2459.142ns */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x07fff091,  /* HVD1_PFRI off */
  0x8000008b,  /* HVD2_PFRI RR 9ns */
  0x07fff086,  /* VICE_PFRI off */
  0x80001090,  /* VICE1_PFRI RR 10ns */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048  /* REFRESH 7812.5ns */
};

static uint32_t aul7445DualEncodeRts_Memc2[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x07fff04a,  /* VICE1_SG off */
  0x07fff08f,  /* VICE1_DBLK off */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x800cd08c,  /* SHVD0_DBLK_0 RR 10ns */
  0x8006508d,  /* SHVD0_DBLK_1 RR 10ns */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x004ae031,  /* BVNF_MFD0_0 3704ns */
  0x0018e004,  /* BVNF_MFD0_1 1235ns */
  0x004ae032,  /* BVNF_MFD1_0 3704ns */
  0x0031d01d,  /* BVNF_MFD1_1 2469ns */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x001dd007,  /* BVNF_VFD0 1480ns */
  0x0077c041,  /* BVNF_VFD1 5920ns */
  0x02829055,  /* BVNF_VFD2 31740ns */
  0x02829056,  /* BVNF_VFD3 31740ns */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x001dd006,  /* BVNF_CAP0 1480ns */
  0x0077c040,  /* BVNF_CAP1 5920ns */
  0x0141304f,  /* BVNF_CAP2 15870ns */
  0x01413050,  /* BVNF_CAP3 15870ns */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x0010e000,  /* BVNF_GFD0 841ns */
  0x00b3b04c,  /* BVNF_GFD1 8880ns */
  0x00b3b04d,  /* BVNF_GFD2 8880ns */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x0027e010,  /* MCVP0 1976ns */
  0x0027e011,  /* MCVP1 1976ns */
  0x009fd049,  /* MCVP_QM 7900ns */
  0x001c3005,  /* BVNF_RDC 1400ns */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x8000008e,  /* SHVD0_PFRI RR 9ns */
  0x07fff091,  /* HVD1_PFRI off */
  0x07fff08b,  /* HVD2_PFRI off */
  0x07fff086,  /* VICE_PFRI off */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048   /* REFRESH 7812.5ns */
};

static uint32_t aul7445QuadEncodeRts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x0016c003,  /* XPT_WR_RS 1130ns */
  0x802d0063,  /* XPT_WR_XC RR 2230ns */
  0x80355022,  /* XPT_WR_CDB RR 2640ns */
  0x80907073,  /* XPT_WR_ITB_MSG RR 7140ns */
  0x8059f03a,  /* XPT_RD_RS RR 4450ns */
  0x8136b04e,  /* XPT_RD_XC_RMX_MSG RR 15350ns */
  0x80355023,  /* XPT_RD_XC_RAVE RR 2640ns */
  0x807b906b,  /* XPT_RD_PB RR 6110ns */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x87fff09e,  /* TPCAP RR */
  0x02ed2058,  /* MCIF_RD 37000ns */
  0x02ed2057,  /* MCIF_WR 37000ns */
  0x87fff094,  /* UART_DMA_RD RR */
  0x87fff095,  /* UART_DMA_WR RR */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x003ca02c,  /* RAAGA 3000ns */
  0x00142002,  /* RAAGA_1 1000ns */
  0x00796042,  /* RAAGA1 6000ns */
  0x00142001,  /* RAAGA1_1 1000ns */
  0x008c6043,  /* AUD_AIO 6940ns */
  0x80261062,  /* VICE_CME_RMB_CMB RR 2000ns */
  0x81318079,  /* VICE_CME_CSC RR 16000ns */
  0x80460067,  /* VICE_FME_CSC RR 3670ns */
  0x80460069,  /* VICE_FME_Luma_CMB RR 3670ns */
  0x80460068,  /* VICE_FME_Chroma_CMB RR 3670ns */
  0x80a5504b,  /* VICE_SG RR 8170ns */
  0x80001085,  /* VICE_DBLK RR 10ns */
  0x833e205a,  /* VICE_CABAC0 RR 41000ns */
  0x81d5a052,  /* VICE_CABAC1 RR 23200ns */
  0x803a2066,  /* VICE_ARCSS0 RR 3050ns */
  0x8025500e,  /* VICE_VIP0_INST0 RR 1850ns */
  0x8095e046,  /* VICE_VIP1_INST0 RR 7410ns */
  0x8025500f,  /* VICE_VIP0_INST1 RR 1850ns */
  0x8095e047,  /* VICE_VIP1_INST1 RR 7410ns */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x80a5504a,  /* VICE1_SG RR 8170ns */
  0x07fff08f,  /* VICE1_DBLK off */
  0x833e2059,  /* VICE1_CABAC0 RR 41000ns */
  0x81d5a051,  /* VICE1_CABAC1 RR 23200ns */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x801d4061,  /* SHVD0_ILCPU RR 1451ns */
  0x8080f06c,  /* SHVD0_OLCPU RR 6756ns */
  0x005e503d,  /* SHVD0_CAB 4666ns */
  0x0054e039,  /* SHVD0_ILSI 4199ns */
  0x802e9012,  /* SHVD0_BLCPU RR 2308ns */
  0x8065003e,  /* SHVD0_BLSI RR 4996ns */
  0x80147088,  /* HVD1_DBLK_0 RR 10ns */
  0x800a2087,  /* HVD1_DBLK_1 RR 10ns */
  0x801d405f,  /* HVD1_ILCPU RR 1451ns */
  0x8083806e,  /* HVD1_OLCPU RR 6893ns */
  0x005e503b,  /* HVD1_CAB 4666ns */
  0x0054e037,  /* HVD1_ILSI 4199ns */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x801d4060,  /* HVD2_ILCPU RR 1451ns */
  0x8083806f,  /* HVD2_OLCPU RR 6893ns */
  0x005e503c,  /* HVD2_CAB 4666ns */
  0x0054e038,  /* HVD2_ILSI 4199ns */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x004ae035,  /* BVNF_MFD4_0 3704ns */
  0x0031d020,  /* BVNF_MFD4_1 2469ns */
  0x004ae036,  /* BVNF_MFD5_0 3704ns */
  0x0031d021,  /* BVNF_MFD5_1 2469ns */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x0031a015,  /* BVNF_GFD3 2459.142ns */
  0x0031a016,  /* BVNF_GFD4 2459.142ns */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x027dc053,  /* VEC_VBI_ENC0 31500ns */
  0x027dc054,  /* VEC_VBI_ENC1 31500ns */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x80000091,  /* HVD1_PFRI RR 9ns */
  0x07fff08b,  /* HVD2_PFRI off */
  0x80001086,  /* VICE_PFRI RR 10ns */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048   /* REFRESH 7812.5ns */
};

static uint32_t aul7445QuadEncodeRts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x80261081,  /* VICE1_CME_RMB_CMB RR 2000ns */
  0x81318080,  /* VICE1_CME_CSC RR 16000ns */
  0x80460082,  /* VICE1_FME_CSC RR 3670ns */
  0x80460084,  /* VICE1_FME_Luma_CMB RR 3670ns */
  0x80460083,  /* VICE1_FME_Chroma_CMB RR 3670ns */
  0x07fff04a,  /* VICE1_SG off */
  0x8000108f,  /* VICE1_DBLK RR 10ns */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x803a207f,  /* VICE1_ARCSS0 RR 3050ns */
  0x8025500c,  /* VICE1_VIP0_INST0 RR 1850ns */
  0x8095e044,  /* VICE1_VIP1_INST0 RR 7410ns */
  0x8025500d,  /* VICE1_VIP0_INST1 RR 1850ns */
  0x8095e045,  /* VICE1_VIP1_INST1 RR 7410ns */
  0x07fff08c,  /* SHVD0_DBLK_0 off */
  0x07fff08d,  /* SHVD0_DBLK_1 off */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x80147089,  /* HVD2_DBLK_0 RR 10ns */
  0x800a208a,  /* HVD2_DBLK_1 RR 10ns */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x0023600b,  /* MADR_RD 1755ns */
  0x0031d01c,  /* MADR_QM 2469ns */
  0x0046f030,  /* MADR_WR 3511ns */
  0x00236008,  /* MADR1_RD 1755ns */
  0x0031d019,  /* MADR1_QM 2469ns */
  0x0046f02d,  /* MADR1_WR 3511ns */
  0x00236009,  /* MADR2_RD 1755ns */
  0x0031d01a,  /* MADR2_QM 2469ns */
  0x0046f02e,  /* MADR2_WR 3511ns */
  0x0023600a,  /* MADR3_RD 1755ns */
  0x0031d01b,  /* MADR3_QM 2469ns */
  0x0046f02f,  /* MADR3_WR 3511ns */
  0x07fff031,  /* BVNF_MFD0_0 off */
  0x07fff004,  /* BVNF_MFD0_1 off */
  0x07fff032,  /* BVNF_MFD1_0 off */
  0x07fff01d,  /* BVNF_MFD1_1 off */
  0x004ae033,  /* BVNF_MFD2_0 3704ns */
  0x0031d01e,  /* BVNF_MFD2_1 2469ns */
  0x004ae034,  /* BVNF_MFD3_0 3704ns */
  0x0031d01f,  /* BVNF_MFD3_1 2469ns */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x07fff007,  /* BVNF_VFD0 off */
  0x07fff041,  /* BVNF_VFD1 off */
  0x07fff055,  /* BVNF_VFD2 off */
  0x07fff056,  /* BVNF_VFD3 off */
  0x003bd028,  /* BVNF_VFD4 2960ns */
  0x003bd029,  /* BVNF_VFD5 2960ns */
  0x003bd02a,  /* BVNF_VFD6 2960ns */
  0x003bd02b,  /* BVNF_VFD7 2960ns */
  0x07fff006,  /* BVNF_CAP0 off */
  0x07fff040,  /* BVNF_CAP1 off */
  0x07fff04f,  /* BVNF_CAP2 off */
  0x07fff050,  /* BVNF_CAP3 off */
  0x003bd024,  /* BVNF_CAP4 2960ns */
  0x003bd025,  /* BVNF_CAP5 2960ns */
  0x003bd026,  /* BVNF_CAP6 2960ns */
  0x003bd027,  /* BVNF_CAP7 2960ns */
  0x07fff000,  /* BVNF_GFD0 off */
  0x07fff04c,  /* BVNF_GFD1 off */
  0x07fff04d,  /* BVNF_GFD2 off */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x0031a017,  /* BVNF_GFD5 2459.142ns */
  0x0031a018,  /* BVNF_GFD6 2459.142ns */
  0x07fff010,  /* MCVP0 off */
  0x07fff011,  /* MCVP1 off */
  0x07fff049,  /* MCVP_QM off */
  0x07fff005,  /* BVNF_RDC off */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x07fff08e,  /* SHVD0_PFRI off */
  0x07fff091,  /* HVD1_PFRI off */
  0x8000008b,  /* HVD2_PFRI RR 9ns */
  0x07fff086,  /* VICE_PFRI off */
  0x80001090,  /* VICE1_PFRI RR 10ns */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048  /* REFRESH 7812.5ns */
};

static uint32_t aul7445QuadEncodeRts_Memc2[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
  0x07fff003,  /* XPT_WR_RS off */
  0x07fff063,  /* XPT_WR_XC off */
  0x07fff022,  /* XPT_WR_CDB off */
  0x07fff073,  /* XPT_WR_ITB_MSG off */
  0x07fff03a,  /* XPT_RD_RS off */
  0x07fff04e,  /* XPT_RD_XC_RMX_MSG off */
  0x07fff023,  /* XPT_RD_XC_RAVE off */
  0x07fff06b,  /* XPT_RD_PB off */
  0x8076106a,  /* XPT_WR_MEMDMA RR 6188ns */
  0x8106c075,  /* XPT_RD_MEMDMA RR 13760ns */
  0x802fd013,  /* GENET0_WR RR 2370ns */
  0x808fb070,  /* GENET0_RD RR 7530ns */
  0x8067503f,  /* GENET1_WR RR 5110ns */
  0x808fb071,  /* GENET1_RD RR 7530ns */
  0x802fd014,  /* GENET2_WR RR 2370ns */
  0x808fb072,  /* GENET2_RD RR 7530ns */
  0x8431205c,  /* MOCA_MIPS RR 53000ns */
  0x802de064,  /* SATA RR 2409ns */
  0x802de065,  /* SATA_1 RR 2409ns */
  0x07fff0a3,  /* UNASSIGNED off */
  0x07fff0a4,  /* UNASSIGNED off */
  0x07fff0a5,  /* UNASSIGNED off */
  0x83f4605b,  /* BSP RR 50000ns */
  0x8082206d,  /* SAGE RR 6820ns */
  0x84b3607c,  /* FLASH_DMA RR 63000ns */
  0x81090076,  /* HIF_PCIe RR 13880ns */
  0x84b3607e,  /* SDIO_EMMC RR 63000ns */
  0x84b3607d,  /* SDIO_CARD RR 63000ns */
  0x07fff09e,  /* TPCAP off */
  0x07fff058,  /* MCIF_RD off */
  0x07fff057,  /* MCIF_WR off */
  0x07fff094,  /* UART_DMA_RD off */
  0x07fff095,  /* UART_DMA_WR off */
  0x81221077,  /* USB_HI_0 RR 15190ns */
  0x87fff097,  /* USB_LO_0 RR */
  0x8150707b,  /* USB_X_WRITE_0 RR 17620ns */
  0x8150707a,  /* USB_X_READ_0 RR 17620ns */
  0x809ea074,  /* USB_X_CTRL_0 RR 8310ns */
  0x81221078,  /* USB_HI_1 RR 15190ns */
  0x87fff098,  /* USB_LO_1 RR */
  0x07fff02c,  /* RAAGA off */
  0x07fff002,  /* RAAGA_1 off */
  0x07fff042,  /* RAAGA1 off */
  0x07fff001,  /* RAAGA1_1 off */
  0x07fff043,  /* AUD_AIO off */
  0x07fff062,  /* VICE_CME_RMB_CMB off */
  0x07fff079,  /* VICE_CME_CSC off */
  0x07fff067,  /* VICE_FME_CSC off */
  0x07fff069,  /* VICE_FME_Luma_CMB off */
  0x07fff068,  /* VICE_FME_Chroma_CMB off */
  0x07fff04b,  /* VICE_SG off */
  0x07fff085,  /* VICE_DBLK off */
  0x07fff05a,  /* VICE_CABAC0 off */
  0x07fff052,  /* VICE_CABAC1 off */
  0x07fff066,  /* VICE_ARCSS0 off */
  0x07fff00e,  /* VICE_VIP0_INST0 off */
  0x07fff046,  /* VICE_VIP1_INST0 off */
  0x07fff00f,  /* VICE_VIP0_INST1 off */
  0x07fff047,  /* VICE_VIP1_INST1 off */
  0x07fff081,  /* VICE1_CME_RMB_CMB off */
  0x07fff080,  /* VICE1_CME_CSC off */
  0x07fff082,  /* VICE1_FME_CSC off */
  0x07fff084,  /* VICE1_FME_Luma_CMB off */
  0x07fff083,  /* VICE1_FME_Chroma_CMB off */
  0x07fff04a,  /* VICE1_SG off */
  0x07fff08f,  /* VICE1_DBLK off */
  0x07fff059,  /* VICE1_CABAC0 off */
  0x07fff051,  /* VICE1_CABAC1 off */
  0x07fff07f,  /* VICE1_ARCSS0 off */
  0x07fff00c,  /* VICE1_VIP0_INST0 off */
  0x07fff044,  /* VICE1_VIP1_INST0 off */
  0x07fff00d,  /* VICE1_VIP0_INST1 off */
  0x07fff045,  /* VICE1_VIP1_INST1 off */
  0x800cd08c,  /* SHVD0_DBLK_0 RR 10ns */
  0x8006508d,  /* SHVD0_DBLK_1 RR 10ns */
  0x07fff061,  /* SHVD0_ILCPU off */
  0x07fff06c,  /* SHVD0_OLCPU off */
  0x07fff03d,  /* SHVD0_CAB off */
  0x07fff039,  /* SHVD0_ILSI off */
  0x07fff012,  /* SHVD0_BLCPU off */
  0x07fff03e,  /* SHVD0_BLSI off */
  0x07fff088,  /* HVD1_DBLK_0 off */
  0x07fff087,  /* HVD1_DBLK_1 off */
  0x07fff05f,  /* HVD1_ILCPU off */
  0x07fff06e,  /* HVD1_OLCPU off */
  0x07fff03b,  /* HVD1_CAB off */
  0x07fff037,  /* HVD1_ILSI off */
  0x87fff09d,  /* SID RR */
  0x07fff089,  /* HVD2_DBLK_0 off */
  0x07fff08a,  /* HVD2_DBLK_1 off */
  0x07fff060,  /* HVD2_ILCPU off */
  0x07fff06f,  /* HVD2_OLCPU off */
  0x07fff03c,  /* HVD2_CAB off */
  0x07fff038,  /* HVD2_ILSI off */
  0x07fff00b,  /* MADR_RD off */
  0x07fff01c,  /* MADR_QM off */
  0x07fff030,  /* MADR_WR off */
  0x07fff008,  /* MADR1_RD off */
  0x07fff019,  /* MADR1_QM off */
  0x07fff02d,  /* MADR1_WR off */
  0x07fff009,  /* MADR2_RD off */
  0x07fff01a,  /* MADR2_QM off */
  0x07fff02e,  /* MADR2_WR off */
  0x07fff00a,  /* MADR3_RD off */
  0x07fff01b,  /* MADR3_QM off */
  0x07fff02f,  /* MADR3_WR off */
  0x004ae031,  /* BVNF_MFD0_0 3704ns */
  0x0018e004,  /* BVNF_MFD0_1 1235ns */
  0x004ae032,  /* BVNF_MFD1_0 3704ns */
  0x0031d01d,  /* BVNF_MFD1_1 2469ns */
  0x07fff033,  /* BVNF_MFD2_0 off */
  0x07fff01e,  /* BVNF_MFD2_1 off */
  0x07fff034,  /* BVNF_MFD3_0 off */
  0x07fff01f,  /* BVNF_MFD3_1 off */
  0x07fff035,  /* BVNF_MFD4_0 off */
  0x07fff020,  /* BVNF_MFD4_1 off */
  0x07fff036,  /* BVNF_MFD5_0 off */
  0x07fff021,  /* BVNF_MFD5_1 off */
  0x001dd007,  /* BVNF_VFD0 1480ns */
  0x0077c041,  /* BVNF_VFD1 5920ns */
  0x02829055,  /* BVNF_VFD2 31740ns */
  0x02829056,  /* BVNF_VFD3 31740ns */
  0x07fff028,  /* BVNF_VFD4 off */
  0x07fff029,  /* BVNF_VFD5 off */
  0x07fff02a,  /* BVNF_VFD6 off */
  0x07fff02b,  /* BVNF_VFD7 off */
  0x001dd006,  /* BVNF_CAP0 1480ns */
  0x0077c040,  /* BVNF_CAP1 5920ns */
  0x0141304f,  /* BVNF_CAP2 15870ns */
  0x01413050,  /* BVNF_CAP3 15870ns */
  0x07fff024,  /* BVNF_CAP4 off */
  0x07fff025,  /* BVNF_CAP5 off */
  0x07fff026,  /* BVNF_CAP6 off */
  0x07fff027,  /* BVNF_CAP7 off */
  0x0010e000,  /* BVNF_GFD0 841ns */
  0x00b3b04c,  /* BVNF_GFD1 8880ns */
  0x00b3b04d,  /* BVNF_GFD2 8880ns */
  0x07fff015,  /* BVNF_GFD3 off */
  0x07fff016,  /* BVNF_GFD4 off */
  0x07fff017,  /* BVNF_GFD5 off */
  0x07fff018,  /* BVNF_GFD6 off */
  0x0027e010,  /* MCVP0 1976ns */
  0x0027e011,  /* MCVP1 1976ns */
  0x009fd049,  /* MCVP_QM 7900ns */
  0x001c3005,  /* BVNF_RDC 1400ns */
  0x07fff053,  /* VEC_VBI_ENC0 off */
  0x07fff054,  /* VEC_VBI_ENC1 off */
  0x87fff0a0,  /* V3D_0 RR */
  0x87fff09f,  /* V3D_1 RR */
  0x87fff099,  /* M2MC RR */
  0x87fff09a,  /* M2MC1 RR */
  0x07fff0a6,  /* UNASSIGNED off */
  0x07fff0a7,  /* UNASSIGNED off */
  0x07fff0a8,  /* UNASSIGNED off */
  0x07fff0a9,  /* UNASSIGNED off */
  0x07fff0aa,  /* UNASSIGNED off */
  0x07fff0ab,  /* UNASSIGNED off */
  0x07fff0ac,  /* UNASSIGNED off */
  0x07fff0ad,  /* UNASSIGNED off */
  0x07fff0ae,  /* UNASSIGNED off */
  0x07fff0af,  /* UNASSIGNED off */
  0x07fff0b0,  /* UNASSIGNED off */
  0x07fff0b1,  /* UNASSIGNED off */
  0x07fff0b2,  /* UNASSIGNED off */
  0x07fff0b3,  /* UNASSIGNED off */
  0x07fff0b4,  /* UNASSIGNED off */
  0x07fff0b5,  /* UNASSIGNED off */
  0x07fff0b6,  /* UNASSIGNED off */
  0x07fff0b7,  /* UNASSIGNED off */
  0x07fff0b8,  /* UNASSIGNED off */
  0x07fff0b9,  /* UNASSIGNED off */
  0x07fff0ba,  /* UNASSIGNED off */
  0x07fff0bb,  /* UNASSIGNED off */
  0x07fff0bc,  /* UNASSIGNED off */
  0x07fff0bd,  /* UNASSIGNED off */
  0x07fff0be,  /* UNASSIGNED off */
  0x07fff0bf,  /* UNASSIGNED off */
  0x07fff0c0,  /* UNASSIGNED off */
  0x07fff0c1,  /* UNASSIGNED off */
  0x07fff0c2,  /* UNASSIGNED off */
  0x07fff0c3,  /* UNASSIGNED off */
  0x07fff0c4,  /* UNASSIGNED off */
  0x07fff0c5,  /* UNASSIGNED off */
  0x07fff0c6,  /* UNASSIGNED off */
  0x07fff0c7,  /* UNASSIGNED off */
  0x07fff0c8,  /* UNASSIGNED off */
  0x07fff0c9,  /* UNASSIGNED off */
  0x07fff0ca,  /* UNASSIGNED off */
  0x07fff0cb,  /* UNASSIGNED off */
  0x07fff0cc,  /* UNASSIGNED off */
  0x07fff0cd,  /* UNASSIGNED off */
  0x07fff0ce,  /* UNASSIGNED off */
  0x07fff0cf,  /* UNASSIGNED off */
  0x07fff0d0,  /* UNASSIGNED off */
  0x07fff0d1,  /* UNASSIGNED off */
  0x07fff0d2,  /* UNASSIGNED off */
  0x07fff0d3,  /* UNASSIGNED off */
  0x07fff0d4,  /* UNASSIGNED off */
  0x07fff0d5,  /* UNASSIGNED off */
  0x07fff0d6,  /* UNASSIGNED off */
  0x800f105d,  /* MCP_RD_HIGH RR 750ns */
  0x80000093,  /* MCP_RD_LOW RR 0ns */
  0x801c805e,  /* MCP_WR_HIGH RR 1500ns */
  0x80000092,  /* MCP_WR_LOW RR 0ns */
  0x07fff0d7,  /* UNASSIGNED off */
  0x07fff0d8,  /* UNASSIGNED off */
  0x07fff0d9,  /* UNASSIGNED off */
  0x07fff0da,  /* UNASSIGNED off */
  0x07fff0db,  /* UNASSIGNED off */
  0x07fff0dc,  /* UNASSIGNED off */
  0x07fff0dd,  /* UNASSIGNED off */
  0x07fff0de,  /* UNASSIGNED off */
  0x07fff0df,  /* UNASSIGNED off */
  0x07fff0e0,  /* UNASSIGNED off */
  0x07fff0e1,  /* UNASSIGNED off */
  0x07fff0e2,  /* UNASSIGNED off */
  0x8000008e,  /* SHVD0_PFRI RR 9ns */
  0x07fff091,  /* HVD1_PFRI off */
  0x07fff08b,  /* HVD2_PFRI off */
  0x07fff086,  /* VICE_PFRI off */
  0x07fff090,  /* VICE1_PFRI off */
  0x07fff0e3,  /* UNASSIGNED off */
  0x07fff0e4,  /* UNASSIGNED off */
  0x07fff0e5,  /* UNASSIGNED off */
  0x07fff0e6,  /* UNASSIGNED off */
  0x07fff0e7,  /* UNASSIGNED off */
  0x07fff0e8,  /* UNASSIGNED off */
  0x07fff0e9,  /* UNASSIGNED off */
  0x07fff0ea,  /* UNASSIGNED off */
  0x07fff0eb,  /* UNASSIGNED off */
  0x07fff0ec,  /* UNASSIGNED off */
  0x07fff0ed,  /* UNASSIGNED off */
  0x07fff0ee,  /* UNASSIGNED off */
  0x07fff0ef,  /* UNASSIGNED off */
  0x07fff0f0,  /* UNASSIGNED off */
  0x07fff0f1,  /* UNASSIGNED off */
  0x07fff0f2,  /* UNASSIGNED off */
  0x07fff0f3,  /* UNASSIGNED off */
  0x07fff0f4,  /* UNASSIGNED off */
  0x07fff0f5,  /* UNASSIGNED off */
  0x07fff0f6,  /* UNASSIGNED off */
  0x07fff0f7,  /* UNASSIGNED off */
  0x07fff0f8,  /* UNASSIGNED off */
  0x07fff0f9,  /* UNASSIGNED off */
  0x07fff0fa,  /* UNASSIGNED off */
  0x07fff0fb,  /* UNASSIGNED off */
  0x07fff0fc,  /* UNASSIGNED off */
  0x07fff0fd,  /* UNASSIGNED off */
  0x87fff09c,  /* MEMC_TRACELOG RR */
  0x07fff0fe,  /* UNASSIGNED off */
  0x07fff0ff,  /* UNASSIGNED off */
  0x87fff09b,  /* MEMC_MSA RR */
  0x87fff0a1,  /* MEMC_DIS0 RR */
  0x87fff0a2,  /* MEMC_DIS1 RR */
  0x87fff096,  /* MEMC_DRAM_INIT_ZQCS RR */
  0x009e1048   /* REFRESH 7812.5ns */
};

static uint32_t aul7445BoxMode1Rts_Memc0[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
    0x001e7002,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_0  XPT_WR_RS 1134.2383107089ns */
    0x803b703b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_1  XPT_WR_XC RR 2339.03576982893ns */
    0x804b1006,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_2  XPT_WR_CDB RR 2785.18518518519ns */
    0x80bc9044,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_3  XPT_WR_ITB_MSG RR 7408.86699507389ns */
    0x80780010,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_4  XPT_RD_RS RR 4449.70414201183ns */
    0x819e3024,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_5  XPT_RD_XC_RMX_MSG RR 15346.9387755102ns */
    0x804b1005,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_6  XPT_RD_XC_RAVE RR 2785.18518518519ns */
    0x80aae040,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_7  XPT_RD_PB RR 6714.75409836066ns */
    0x80fd7047,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_8  XPT_WR_MEMDMA RR 9955.55555555556ns */
    0x82438051,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_9  XPT_RD_MEMDMA RR 22755.5555555556ns */
    0x803fd004,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_10  SYSPORT_WR RR 2370ns */
    0x80ae9043,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_11  SYSPORT_RD RR 6860ns */
    0x3ffff074,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_12  UNASSIGNED off */
    0x3ffff075,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_13  UNASSIGNED off */
    0x3ffff076,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_14  UNASSIGNED off */
    0x8161704e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_15  HIF_PCIe1 RR 13880ns */
    0x8596e032,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_16  MOCA_MIPS RR 53000ns */
    0x8066203c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_17  SATA RR 4015.6862745098ns */
    0x8066203d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_18  SATA_1 RR 4015.6862745098ns */
    0x03e6e02c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_19  MCIF2_RD 37000ns */
    0x03e6e02e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_20  MCIF2_WR 37000ns */
    0x3ffff077,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_21  UNASSIGNED off */
    0x8545e031,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_22  BSP RR 50000ns */
    0x80ad9041,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_23  SAGE RR 6820ns */
    0x86449054,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_24  FLASH_DMA RR 63000ns */
    0x8161704d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_25  HIF_PCIe RR 13880ns */
    0x86449056,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_26  SDIO_EMMC RR 63000ns */
    0x86449055,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_27  SDIO_CARD RR 63000ns */
    0xbffff06f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_28  TPCAP RR */
    0x03e6e02d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_29  MCIF_RD 37000ns */
    0x03e6e02f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_30  MCIF_WR 37000ns */
    0xbffff05f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_31  UART_DMA_RD RR */
    0xbffff060,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_32  UART_DMA_WR RR */
    0x810db048,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_33  USB_HI_0 RR 10593ns */
    0xbffff062,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_34  USB_LO_0 RR */
    0x815c504c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_35  USB_X_WRITE_0 RR 13680ns */
    0x815c504b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_36  USB_X_READ_0 RR 13680ns */
    0x80ae1042,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_37  USB_X_CTRL_0 RR 6840ns */
    0x810db049,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_38  USB_HI_1 RR 10593ns */
    0xbffff063,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_39  USB_LO_1 RR */
    0x0050e007,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_40  RAAGA 3000ns */
    0x001ae001,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_41  RAAGA_1 1000ns */
    0x00a1e01a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_42  RAAGA1 6000ns */
    0x001ae000,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_43  RAAGA1_1 1000ns */
    0x00bb401b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_44  AUD_AIO 6940ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_45  VICE_CME_RMB_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_46  VICE_CME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_47  VICE_FME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_48  VICE_FME_Luma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_49  VICE_FME_Chroma_CMB off */
    0x81b8d025,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_50  VICE_SG RR 16333.3333333333ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_51  VICE_DBLK off */
    0x83dc102b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_52  VICE_CABAC0 RR 36600ns */
    0x86f07033,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_53  VICE_CABAC1 RR 65800ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_54  VICE_ARCSS0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_55  VICE_VIP0_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_56  VICE_VIP1_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_57  VICE_VIP0_INST1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_58  VICE_VIP1_INST1 off */
    0x80e52046,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_59  VICE1_CME_RMB_CMB RR 9000ns */
    0x8729d057,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_60  VICE1_CME_CSC RR 72000ns */
    0x81a6b04f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_61  VICE1_FME_CSC RR 16600ns */
    0x83501053,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_62  VICE1_FME_Luma_CMB RR 33300ns */
    0x83501052,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_63  VICE1_FME_Chroma_CMB RR 33300ns */
    0x84acd030,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_64  VICE1_SG RR 44333.3333333333ns */
    0x80002059,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_65  VICE1_DBLK RR 10ns */
    0x87b84034,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_66  VICE1_CABAC0 RR 73200ns */
    0x8de11035,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_67  VICE1_CABAC1 RR 131600ns */
    0x8152a04a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_68  VICE1_ARCSS0 RR 13300ns */
    0x80c7d01c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_69  VICE1_VIP0_INST0 RR 7406ns */
    0x82574026,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_70  VICE1_VIP1_INST0 RR 22200ns */
    0x80c7d01d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_71  VICE1_VIP0_INST1 RR 7406ns */
    0x82574027,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_72  VICE1_VIP1_INST1 RR 22200ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_73  HVD0_DBLK_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_74  HVD0_DBLK_1 off */
    0x80270038,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_75  HVD0_ILCPU RR 1451ns */
    0x809ee03f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_76  HVD0_OLCPU RR 6242ns */
    0x005a200a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_77  HVD0_CAB 3343ns */
    0x0071a00c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_78  HVD0_ILSI 4214ns */
    0x81fea050,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_79  HVD0_ILCPU_p2 RR 18917ns */
    0x0072600e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_80  HVD0_ILSI_p2 4242ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_81  HVD1_DBLK_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_82  HVD1_DBLK_1 off */
    0x80270039,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_83  HVD1_ILCPU RR 1451ns */
    0x80d9b045,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_84  HVD1_OLCPU RR 8553ns */
    0x007de011,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_85  HVD1_CAB 4667ns */
    0x0071a00d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_86  HVD1_ILSI 4214ns */
    0xbffff06e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_87  SID RR */
    0x8000005b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_88  HVD2_DBLK_0 RR 0ns */
    0x8000005c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_89  HVD2_DBLK_1 RR 0ns */
    0x8027003a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_90  HVD2_ILCPU RR 1451ns */
    0x8070a03e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_91  HVD2_OLCPU RR 4427ns */
    0x0071400b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_92  HVD2_CAB 4200ns */
    0x0072d00f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_93  HVD2_ILSI 4258ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_94  BVN_MAD_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_95  BVN_MAD_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_96  BVN_MAD_PIX_CAP off */
    0x3ffff078,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_97  UNASSIGNED off */
    0x3ffff079,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_98  UNASSIGNED off */
    0x3ffff07a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_99  UNASSIGNED off */
    0x3ffff07b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_100  UNASSIGNED off */
    0x3ffff07c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_101  UNASSIGNED off */
    0x3ffff07d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_102  UNASSIGNED off */
    0x008e1014,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_103  BVN_MAD3_PIX_FD 5266.5ns */
    0x00c7d01f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_104  BVN_MAD3_QUANT 7407ns */
    0x011c4022,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_105  BVN_MAD3_PIX_CAP 10534.5ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_106  BVN_MFD0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_107  BVN_MFD0_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_108  BVN_MFD1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_109  BVN_MFD1_1 off */
    0x3ffff07e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_110  UNASSIGNED off */
    0x3ffff07f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_111  UNASSIGNED off */
    0x3ffff080,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_112  UNASSIGNED off */
    0x3ffff081,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_113  UNASSIGNED off */
    0x00853012,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_114  BVN_MFD4 4938ns */
    0x0058c008,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_115  BVN_MFD4_1 3292ns */
    0x00853013,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_116  BVN_MFD5 4938ns */
    0x0058c009,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_117  BVN_MFD5_1 3292ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_118  BVN_VFD0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_119  BVN_VFD1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_120  BVN_VFD2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_121  BVN_VFD3 off */
    0x3ffff082,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_122  UNASSIGNED off */
    0x3ffff083,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_123  UNASSIGNED off */
    0x0095d016,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_124  BVN_VFD6 5555ns */
    0x0095d017,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_125  BVN_VFD7 5555ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_126  BVN_CAP0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_127  BVN_CAP1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_128  BVN_CAP2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_129  BVN_CAP3 off */
    0x3ffff084,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_130  UNASSIGNED off */
    0x3ffff085,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_131  UNASSIGNED off */
    0x0095d018,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_132  BVN_CAP6 5555ns */
    0x0095d019,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_133  BVN_CAP7 5555ns */
    0x0031d003,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_134  BVN_GFD0 1851ns */
    0x3ffff086,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_135  UNASSIGNED off */
    0x3ffff087,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_136  UNASSIGNED off */
    0x3ffff088,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_137  UNASSIGNED off */
    0x3ffff089,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_138  UNASSIGNED off */
    0x3ffff08a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_139  UNASSIGNED off */
    0x3ffff08b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_140  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_141  BVN_MCVP0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_142  BVN_MCVP1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_143  BVN_MCVP2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_144  BVN_RDC off */
    0x03526029,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_145  VEC_VBI_ENC0 31500ns */
    0x0352602a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_146  VEC_VBI_ENC1 31500ns */
    0xbffff069,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_147  M2MC_0 RR */
    0xbffff06a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_148  M2MC_1 RR */
    0xbffff06b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_149  M2MC_2 RR */
    0x3ffff08c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_150  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_151  VICE_VIP0_INST2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_152  VICE_VIP1_INST2 off */
    0x80c7d01e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_153  VICE1_VIP0_INST2 RR 7406ns */
    0x82574028,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_154  VICE1_VIP1_INST2 RR 22200ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_155  HVD0_DBLK_p2_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_156  HVD0_DBLK_p2_1 off */
    0x008e1015,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_157  BVN_MAD4_PIX_FD 5266.5ns */
    0x00c7d020,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_158  BVN_MAD4_QUANT 7407ns */
    0x011c4023,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_159  BVN_MAD4_PIX_CAP 10534.5ns */
    0xbffff066,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_160  M2MC1_0 RR */
    0xbffff067,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_161  M2MC1_1 RR */
    0xbffff068,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_162  M2MC1_2 RR */
    0x3ffff08d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_163  UNASSIGNED off */
    0x3ffff08e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_164  UNASSIGNED off */
    0x3ffff08f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_165  UNASSIGNED off */
    0x3ffff090,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_166  UNASSIGNED off */
    0x3ffff091,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_167  UNASSIGNED off */
    0x3ffff092,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_168  UNASSIGNED off */
    0x3ffff093,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_169  UNASSIGNED off */
    0x3ffff094,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_170  UNASSIGNED off */
    0x3ffff095,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_171  UNASSIGNED off */
    0x3ffff096,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_172  UNASSIGNED off */
    0x3ffff097,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_173  UNASSIGNED off */
    0x3ffff098,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_174  UNASSIGNED off */
    0x3ffff099,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_175  UNASSIGNED off */
    0x3ffff09a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_176  UNASSIGNED off */
    0x3ffff09b,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_177  UNASSIGNED off */
    0x3ffff09c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_178  UNASSIGNED off */
    0x3ffff09d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_179  UNASSIGNED off */
    0x3ffff09e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_180  UNASSIGNED off */
    0x3ffff09f,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_181  UNASSIGNED off */
    0x3ffff0a0,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_182  UNASSIGNED off */
    0x3ffff0a1,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_183  UNASSIGNED off */
    0x3ffff0a2,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_184  UNASSIGNED off */
    0x3ffff0a3,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_185  UNASSIGNED off */
    0x3ffff0a4,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_186  UNASSIGNED off */
    0x3ffff0a5,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_187  UNASSIGNED off */
    0x3ffff0a6,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_188  UNASSIGNED off */
    0x3ffff0a7,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_189  UNASSIGNED off */
    0x3ffff0a8,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_190  UNASSIGNED off */
    0x3ffff0a9,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_191  UNASSIGNED off */
    0x3ffff0aa,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_192  UNASSIGNED off */
    0x3ffff0ab,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_193  UNASSIGNED off */
    0x3ffff0ac,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_194  UNASSIGNED off */
    0x3ffff0ad,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_195  UNASSIGNED off */
    0x3ffff0ae,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_196  UNASSIGNED off */
    0x3ffff0af,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_197  UNASSIGNED off */
    0x3ffff0b0,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_198  UNASSIGNED off */
    0x3ffff0b1,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_199  UNASSIGNED off */
    0x80142036,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_200  CPU_MCP_RD_HIGH RR 750ns */
    0x8000005d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_201  CPU_MCP_RD_LOW RR */
    0x80261037,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_202  CPU_MCP_WR_HIGH RR 1500ns */
    0x8000005e,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_203  CPU_MCP_WR_LOW RR */
    0xbffff064,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_204  V3D_MCP_RD_HIGH RR */
    0xbffff065,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_205  V3D_MCP_RD_LOW RR */
    0xbffff070,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_206  V3D_MCP_WR_HIGH RR */
    0xbffff071,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_207  V3D_MCP_WR_LOW RR */
    0x3ffff0b2,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_208  UNASSIGNED off */
    0x3ffff0b3,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_209  UNASSIGNED off */
    0x3ffff0b4,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_210  UNASSIGNED off */
    0x3ffff0b5,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_211  UNASSIGNED off */
    0x3ffff0b6,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_212  UNASSIGNED off */
    0x3ffff0b7,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_213  UNASSIGNED off */
    0x3ffff0b8,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_214  UNASSIGNED off */
    0x3ffff0b9,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_215  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_216  HVD0_PFRI off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_217  HVD1_PFRI off */
    0x8000005a,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_218  HVD2_PFRI RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_219  VICE_PFRI off */
    0x80002058,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_220  VICE1_PFRI RR 10ns */
    0x3ffff0ba,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_221  UNASSIGNED off */
    0x3ffff0bb,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_222  UNASSIGNED off */
    0x3ffff0bc,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_223  UNASSIGNED off */
    0x3ffff0bd,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_224  UNASSIGNED off */
    0x3ffff0be,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_225  UNASSIGNED off */
    0x3ffff0bf,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_226  UNASSIGNED off */
    0x3ffff0c0,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_227  UNASSIGNED off */
    0x3ffff0c1,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_228  UNASSIGNED off */
    0x3ffff0c2,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_229  UNASSIGNED off */
    0x3ffff0c3,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_230  UNASSIGNED off */
    0x3ffff0c4,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_231  UNASSIGNED off */
    0x3ffff0c5,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_232  UNASSIGNED off */
    0x3ffff0c6,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_233  UNASSIGNED off */
    0x3ffff0c7,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_234  UNASSIGNED off */
    0x3ffff0c8,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_235  UNASSIGNED off */
    0x3ffff0c9,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_236  UNASSIGNED off */
    0x3ffff0ca,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_237  UNASSIGNED off */
    0x3ffff0cb,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_238  UNASSIGNED off */
    0x3ffff0cc,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_239  UNASSIGNED off */
    0x3ffff0cd,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_240  UNASSIGNED off */
    0x3ffff0ce,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_241  UNASSIGNED off */
    0x3ffff0cf,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_242  UNASSIGNED off */
    0x3ffff0d0,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_243  UNASSIGNED off */
    0x3ffff0d1,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_244  UNASSIGNED off */
    0x3ffff0d2,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_245  UNASSIGNED off */
    0x3ffff0d3,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_246  UNASSIGNED off */
    0x3ffff0d4,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_247  UNASSIGNED off */
    0xbffff06d,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_248  MEMC_TRACELOG RR */
    0x3ffff0d5,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_249  UNASSIGNED off */
    0x3ffff0d6,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_250  UNASSIGNED off */
    0xbffff06c,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_251  MEMC_MSA RR */
    0xbffff072,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_252  MEMC_DIS0 RR */
    0xbffff073,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_253  MEMC_DIS1 RR */
    0xbffff061,  /* BCHP_MEMC_ARB_0_CLIENT_INFO_254  MEMC_DRAM_INIT_ZQCS RR */
    0x00d2d021   /* BCHP_MEMC_ARB_0_CLIENT_INFO_255  REFRESH 7812.5ns */
};

static uint32_t aul7445BoxMode1Rts_Memc1[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_0  XPT_WR_RS off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_1  XPT_WR_XC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_2  XPT_WR_CDB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_3  XPT_WR_ITB_MSG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_4  XPT_RD_RS off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_5  XPT_RD_XC_RMX_MSG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_6  XPT_RD_XC_RAVE off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_7  XPT_RD_PB off */
    0x80fd701a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_8  XPT_WR_MEMDMA RR 9955.55555555556ns */
    0x82438022,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_9  XPT_RD_MEMDMA RR 22755.5555555556ns */
    0x803fd003,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_10  SYSPORT_WR RR 2370ns */
    0x80ae9019,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_11  SYSPORT_RD RR 6860ns */
    0x3ffff04a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_12  UNASSIGNED off */
    0x3ffff04b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_13  UNASSIGNED off */
    0x3ffff04c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_14  UNASSIGNED off */
    0x81617020,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_15  HIF_PCIe1 RR 13880ns */
    0x8596e012,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_16  MOCA_MIPS RR 53000ns */
    0x80662015,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_17  SATA RR 4015.6862745098ns */
    0x80662016,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_18  SATA_1 RR 4015.6862745098ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_19  MCIF2_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_20  MCIF2_WR off */
    0x3ffff04d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_21  UNASSIGNED off */
    0x8545e011,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_22  BSP RR 50000ns */
    0x80ad9017,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_23  SAGE RR 6820ns */
    0x86449023,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_24  FLASH_DMA RR 63000ns */
    0x8161701f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_25  HIF_PCIe RR 13880ns */
    0x86449025,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_26  SDIO_EMMC RR 63000ns */
    0x86449024,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_27  SDIO_CARD RR 63000ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_28  TPCAP off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_29  MCIF_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_30  MCIF_WR off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_31  UART_DMA_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_32  UART_DMA_WR off */
    0x810db01b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_33  USB_HI_0 RR 10593ns */
    0xbffff039,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_34  USB_LO_0 RR */
    0x815c501e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_35  USB_X_WRITE_0 RR 13680ns */
    0x815c501d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_36  USB_X_READ_0 RR 13680ns */
    0x80ae1018,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_37  USB_X_CTRL_0 RR 6840ns */
    0x810db01c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_38  USB_HI_1 RR 10593ns */
    0xbffff03a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_39  USB_LO_1 RR */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_40  RAAGA off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_41  RAAGA_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_42  RAAGA1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_43  RAAGA1_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_44  AUD_AIO off */
    0x8065c026,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_45  VICE_CME_RMB_CMB RR 4000ns */
    0x832ef02b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_46  VICE_CME_CSC RR 32000ns */
    0x80bb1027,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_47  VICE_FME_CSC RR 7350ns */
    0x80bb1029,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_48  VICE_FME_Luma_CMB RR 7350ns */
    0x80bb1028,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_49  VICE_FME_Chroma_CMB RR 7350ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_50  VICE_SG off */
    0x80002035,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_51  VICE_DBLK RR 10ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_52  VICE_CABAC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_53  VICE_CABAC1 off */
    0x80a2e02a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_54  VICE_ARCSS0 RR 6400ns */
    0x8063d007,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_55  VICE_VIP0_INST0 RR 3703ns */
    0x80c7a00b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_56  VICE_VIP1_INST0 RR 7400ns */
    0x8063d008,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_57  VICE_VIP0_INST1 RR 3703ns */
    0x80c7a00c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_58  VICE_VIP1_INST1 RR 7400ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_59  VICE1_CME_RMB_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_60  VICE1_CME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_61  VICE1_FME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_62  VICE1_FME_Luma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_63  VICE1_FME_Chroma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_64  VICE1_SG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_65  VICE1_DBLK off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_66  VICE1_CABAC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_67  VICE1_CABAC1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_68  VICE1_ARCSS0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_69  VICE1_VIP0_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_70  VICE1_VIP1_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_71  VICE1_VIP0_INST1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_72  VICE1_VIP1_INST1 off */
    0x8000002f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_73  HVD0_DBLK_Ch_0 RR 0ns */
    0x80000030,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_74  HVD0_DBLK_Ch_1 RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_75  HVD0_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_76  HVD0_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_77  HVD0_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_78  HVD0_ILSI off */
    0x81fea021,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_79  HVD0_ILCPU_p2 RR 18917ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_80  HVD0_ILSI_p2 off */
    0x80000033,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_81  HVD1_DBLK_0 RR 0ns */
    0x80000034,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_82  HVD1_DBLK_1 RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_83  HVD1_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_84  HVD1_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_85  HVD1_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_86  HVD1_ILSI off */
    0xbffff045,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_87  SID RR */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_88  HVD2_DBLK_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_89  HVD2_DBLK_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_90  HVD2_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_91  HVD2_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_92  HVD2_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_93  HVD2_ILSI off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_94  BVN_MAD_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_95  BVN_MAD_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_96  BVN_MAD_PIX_CAP off */
    0x3ffff04e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_97  UNASSIGNED off */
    0x3ffff04f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_98  UNASSIGNED off */
    0x3ffff050,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_99  UNASSIGNED off */
    0x3ffff051,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_100  UNASSIGNED off */
    0x3ffff052,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_101  UNASSIGNED off */
    0x3ffff053,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_102  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_103  BVN_MAD3_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_104  BVN_MAD3_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_105  BVN_MAD3_PIX_CAP off */
    0x00c7d00e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_106  BVN_MFD0_Ch 7407ns */
    0x0038f002,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_107  BVN_MFD0_Ch_1 2115ns */
    0x0085300a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_108  BVN_MFD1 4938ns */
    0x0058c006,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_109  BVN_MFD1_1 3292ns */
    0x3ffff054,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_110  UNASSIGNED off */
    0x3ffff055,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_111  UNASSIGNED off */
    0x3ffff056,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_112  UNASSIGNED off */
    0x3ffff057,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_113  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_114  BVN_MFD4 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_115  BVN_MFD4_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_116  BVN_MFD5 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_117  BVN_MFD5_1 off */
    0x00211000,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_118  BVN_VFD0 1230ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_119  BVN_VFD1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_120  BVN_VFD2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_121  BVN_VFD3 off */
    0x3ffff058,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_122  UNASSIGNED off */
    0x3ffff059,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_123  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_124  BVN_VFD6 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_125  BVN_VFD7 off */
    0x00211001,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_126  BVN_CAP0 1230ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_127  BVN_CAP1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_128  BVN_CAP2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_129  BVN_CAP3 off */
    0x3ffff05a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_130  UNASSIGNED off */
    0x3ffff05b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_131  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_132  BVN_CAP6 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_133  BVN_CAP7 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_134  BVN_GFD0 off */
    0x3ffff05c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_135  UNASSIGNED off */
    0x3ffff05d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_136  UNASSIGNED off */
    0x3ffff05e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_137  UNASSIGNED off */
    0x3ffff05f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_138  UNASSIGNED off */
    0x3ffff060,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_139  UNASSIGNED off */
    0x3ffff061,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_140  UNASSIGNED off */
    0x004b5005,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_141  BVN_MCVP0 2794ns */
    0x004b5004,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_142  BVN_MCVP1 2794ns */
    0x00d3d010,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_143  BVN_MCVP2 7850ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_144  BVN_RDC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_145  VEC_VBI_ENC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_146  VEC_VBI_ENC1 off */
    0xbffff040,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_147  M2MC_0 RR */
    0xbffff041,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_148  M2MC_1 RR */
    0xbffff042,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_149  M2MC_2 RR */
    0x3ffff062,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_150  UNASSIGNED off */
    0x8063d009,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_151  VICE_VIP0_INST2 RR 3703ns */
    0x80c7a00d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_152  VICE_VIP1_INST2 RR 7400ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_153  VICE1_VIP0_INST2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_154  VICE1_VIP1_INST2 off */
    0x80000031,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_155  HVD0_DBLK_p2_Ch_0 RR 0ns */
    0x80000032,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_156  HVD0_DBLK_p2_Ch_1 RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_157  BVN_MAD4_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_158  BVN_MAD4_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_159  BVN_MAD4_PIX_CAP off */
    0xbffff03d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_160  M2MC1_0 RR */
    0xbffff03e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_161  M2MC1_1 RR */
    0xbffff03f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_162  M2MC1_2 RR */
    0x3ffff063,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_163  UNASSIGNED off */
    0x3ffff064,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_164  UNASSIGNED off */
    0x3ffff065,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_165  UNASSIGNED off */
    0x3ffff066,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_166  UNASSIGNED off */
    0x3ffff067,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_167  UNASSIGNED off */
    0x3ffff068,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_168  UNASSIGNED off */
    0x3ffff069,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_169  UNASSIGNED off */
    0x3ffff06a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_170  UNASSIGNED off */
    0x3ffff06b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_171  UNASSIGNED off */
    0x3ffff06c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_172  UNASSIGNED off */
    0x3ffff06d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_173  UNASSIGNED off */
    0x3ffff06e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_174  UNASSIGNED off */
    0x3ffff06f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_175  UNASSIGNED off */
    0x3ffff070,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_176  UNASSIGNED off */
    0x3ffff071,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_177  UNASSIGNED off */
    0x3ffff072,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_178  UNASSIGNED off */
    0x3ffff073,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_179  UNASSIGNED off */
    0x3ffff074,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_180  UNASSIGNED off */
    0x3ffff075,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_181  UNASSIGNED off */
    0x3ffff076,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_182  UNASSIGNED off */
    0x3ffff077,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_183  UNASSIGNED off */
    0x3ffff078,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_184  UNASSIGNED off */
    0x3ffff079,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_185  UNASSIGNED off */
    0x3ffff07a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_186  UNASSIGNED off */
    0x3ffff07b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_187  UNASSIGNED off */
    0x3ffff07c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_188  UNASSIGNED off */
    0x3ffff07d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_189  UNASSIGNED off */
    0x3ffff07e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_190  UNASSIGNED off */
    0x3ffff07f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_191  UNASSIGNED off */
    0x3ffff080,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_192  UNASSIGNED off */
    0x3ffff081,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_193  UNASSIGNED off */
    0x3ffff082,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_194  UNASSIGNED off */
    0x3ffff083,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_195  UNASSIGNED off */
    0x3ffff084,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_196  UNASSIGNED off */
    0x3ffff085,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_197  UNASSIGNED off */
    0x3ffff086,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_198  UNASSIGNED off */
    0x3ffff087,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_199  UNASSIGNED off */
    0x80142013,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_200  CPU_MCP_RD_HIGH RR 750ns */
    0x80000036,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_201  CPU_MCP_RD_LOW RR */
    0x80261014,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_202  CPU_MCP_WR_HIGH RR 1500ns */
    0x80000037,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_203  CPU_MCP_WR_LOW RR */
    0xbffff03b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_204  V3D_MCP_RD_HIGH RR */
    0xbffff03c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_205  V3D_MCP_RD_LOW RR */
    0xbffff046,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_206  V3D_MCP_WR_HIGH RR */
    0xbffff047,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_207  V3D_MCP_WR_LOW RR */
    0x3ffff088,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_208  UNASSIGNED off */
    0x3ffff089,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_209  UNASSIGNED off */
    0x3ffff08a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_210  UNASSIGNED off */
    0x3ffff08b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_211  UNASSIGNED off */
    0x3ffff08c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_212  UNASSIGNED off */
    0x3ffff08d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_213  UNASSIGNED off */
    0x3ffff08e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_214  UNASSIGNED off */
    0x3ffff08f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_215  UNASSIGNED off */
    0x8000002c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_216  HVD0_PFRI_Ch RR 0ns */
    0x8000002d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_217  HVD1_PFRI RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_218  HVD2_PFRI off */
    0x8000202e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_219  VICE_PFRI RR 10ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_220  VICE1_PFRI off */
    0x3ffff090,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_221  UNASSIGNED off */
    0x3ffff091,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_222  UNASSIGNED off */
    0x3ffff092,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_223  UNASSIGNED off */
    0x3ffff093,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_224  UNASSIGNED off */
    0x3ffff094,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_225  UNASSIGNED off */
    0x3ffff095,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_226  UNASSIGNED off */
    0x3ffff096,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_227  UNASSIGNED off */
    0x3ffff097,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_228  UNASSIGNED off */
    0x3ffff098,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_229  UNASSIGNED off */
    0x3ffff099,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_230  UNASSIGNED off */
    0x3ffff09a,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_231  UNASSIGNED off */
    0x3ffff09b,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_232  UNASSIGNED off */
    0x3ffff09c,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_233  UNASSIGNED off */
    0x3ffff09d,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_234  UNASSIGNED off */
    0x3ffff09e,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_235  UNASSIGNED off */
    0x3ffff09f,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_236  UNASSIGNED off */
    0x3ffff0a0,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_237  UNASSIGNED off */
    0x3ffff0a1,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_238  UNASSIGNED off */
    0x3ffff0a2,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_239  UNASSIGNED off */
    0x3ffff0a3,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_240  UNASSIGNED off */
    0x3ffff0a4,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_241  UNASSIGNED off */
    0x3ffff0a5,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_242  UNASSIGNED off */
    0x3ffff0a6,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_243  UNASSIGNED off */
    0x3ffff0a7,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_244  UNASSIGNED off */
    0x3ffff0a8,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_245  UNASSIGNED off */
    0x3ffff0a9,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_246  UNASSIGNED off */
    0x3ffff0aa,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_247  UNASSIGNED off */
    0xbffff044,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_248  MEMC_TRACELOG RR */
    0x3ffff0ab,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_249  UNASSIGNED off */
    0x3ffff0ac,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_250  UNASSIGNED off */
    0xbffff043,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_251  MEMC_MSA RR */
    0xbffff048,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_252  MEMC_DIS0 RR */
    0xbffff049,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_253  MEMC_DIS1 RR */
    0xbffff038,  /* BCHP_MEMC_ARB_1_CLIENT_INFO_254  MEMC_DRAM_INIT_ZQCS RR */
    0x00d2d00f   /* BCHP_MEMC_ARB_1_CLIENT_INFO_255  REFRESH 7812.5ns */
};

static uint32_t aul7445BoxMode1Rts_Memc2[BTST_LOAD_RTS_RTS_ARRAY_SIZE] =
{
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_0  XPT_WR_RS off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_1  XPT_WR_XC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_2  XPT_WR_CDB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_3  XPT_WR_ITB_MSG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_4  XPT_RD_RS off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_5  XPT_RD_XC_RMX_MSG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_6  XPT_RD_XC_RAVE off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_7  XPT_RD_PB off */
    0x80fd7017,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_8  XPT_WR_MEMDMA RR 9955.55555555556ns */
    0x8243801f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_9  XPT_RD_MEMDMA RR 22755.5555555556ns */
    0x803fd001,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_10  SYSPORT_WR RR 2370ns */
    0x80ae9016,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_11  SYSPORT_RD RR 6860ns */
    0x3ffff03c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_12  UNASSIGNED off */
    0x3ffff03d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_13  UNASSIGNED off */
    0x3ffff03e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_14  UNASSIGNED off */
    0x8161701d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_15  HIF_PCIe1 RR 13880ns */
    0x8596e00e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_16  MOCA_MIPS RR 53000ns */
    0x80662012,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_17  SATA RR 4015.6862745098ns */
    0x80662013,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_18  SATA_1 RR 4015.6862745098ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_19  MCIF2_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_20  MCIF2_WR off */
    0x3ffff03f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_21  UNASSIGNED off */
    0x8545e00d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_22  BSP RR 50000ns */
    0x80ad9014,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_23  SAGE RR 6820ns */
    0x86449020,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_24  FLASH_DMA RR 63000ns */
    0x8161701c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_25  HIF_PCIe RR 13880ns */
    0x86449022,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_26  SDIO_EMMC RR 63000ns */
    0x86449021,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_27  SDIO_CARD RR 63000ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_28  TPCAP off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_29  MCIF_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_30  MCIF_WR off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_31  UART_DMA_RD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_32  UART_DMA_WR off */
    0x810db018,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_33  USB_HI_0 RR 10593ns */
    0xbffff02b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_34  USB_LO_0 RR */
    0x815c501b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_35  USB_X_WRITE_0 RR 13680ns */
    0x815c501a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_36  USB_X_READ_0 RR 13680ns */
    0x80ae1015,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_37  USB_X_CTRL_0 RR 6840ns */
    0x810db019,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_38  USB_HI_1 RR 10593ns */
    0xbffff02c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_39  USB_LO_1 RR */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_40  RAAGA off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_41  RAAGA_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_42  RAAGA1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_43  RAAGA1_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_44  AUD_AIO off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_45  VICE_CME_RMB_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_46  VICE_CME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_47  VICE_FME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_48  VICE_FME_Luma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_49  VICE_FME_Chroma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_50  VICE_SG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_51  VICE_DBLK off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_52  VICE_CABAC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_53  VICE_CABAC1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_54  VICE_ARCSS0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_55  VICE_VIP0_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_56  VICE_VIP1_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_57  VICE_VIP0_INST1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_58  VICE_VIP1_INST1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_59  VICE1_CME_RMB_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_60  VICE1_CME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_61  VICE1_FME_CSC off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_62  VICE1_FME_Luma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_63  VICE1_FME_Chroma_CMB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_64  VICE1_SG off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_65  VICE1_DBLK off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_66  VICE1_CABAC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_67  VICE1_CABAC1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_68  VICE1_ARCSS0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_69  VICE1_VIP0_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_70  VICE1_VIP1_INST0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_71  VICE1_VIP0_INST1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_72  VICE1_VIP1_INST1 off */
    0x80000024,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_73  HVD0_DBLK_0 RR 0ns */
    0x80000025,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_74  HVD0_DBLK_1 RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_75  HVD0_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_76  HVD0_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_77  HVD0_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_78  HVD0_ILSI off */
    0x81fea01e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_79  HVD0_ILCPU_p2 RR 18917ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_80  HVD0_ILSI_p2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_81  HVD1_DBLK_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_82  HVD1_DBLK_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_83  HVD1_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_84  HVD1_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_85  HVD1_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_86  HVD1_ILSI off */
    0xbffff037,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_87  SID RR */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_88  HVD2_DBLK_0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_89  HVD2_DBLK_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_90  HVD2_ILCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_91  HVD2_OLCPU off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_92  HVD2_CAB off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_93  HVD2_ILSI off */
    0x005ea005,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_94  BVN_MAD_PIX_FD 3511ns */
    0x00853006,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_95  BVN_MAD_QUANT 4938ns */
    0x00bd7008,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_96  BVN_MAD_PIX_CAP 7023ns */
    0x3ffff040,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_97  UNASSIGNED off */
    0x3ffff041,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_98  UNASSIGNED off */
    0x3ffff042,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_99  UNASSIGNED off */
    0x3ffff043,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_100  UNASSIGNED off */
    0x3ffff044,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_101  UNASSIGNED off */
    0x3ffff045,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_102  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_103  BVN_MAD3_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_104  BVN_MAD3_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_105  BVN_MAD3_PIX_CAP off */
    0x00428003,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_106  BVN_MFD0 2469ns */
    0x001c6000,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_107  BVN_MFD0_1 1057.5ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_108  BVN_MFD1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_109  BVN_MFD1_1 off */
    0x3ffff046,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_110  UNASSIGNED off */
    0x3ffff047,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_111  UNASSIGNED off */
    0x3ffff048,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_112  UNASSIGNED off */
    0x3ffff049,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_113  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_114  BVN_MFD4 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_115  BVN_MFD4_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_116  BVN_MFD5 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_117  BVN_MFD5_1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_118  BVN_VFD0 off */
    0x00424002,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_119  BVN_VFD1 2460ns */
    0x01aa700a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_120  BVN_VFD2 15800ns */
    0x01aa700b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_121  BVN_VFD3 15800ns */
    0x3ffff04a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_122  UNASSIGNED off */
    0x3ffff04b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_123  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_124  BVN_VFD6 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_125  BVN_VFD7 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_126  BVN_CAP0 off */
    0x00854007,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_127  BVN_CAP1 4940ns */
    0x01aa700c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_128  BVN_CAP2 15800ns */
    0x06aa400f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_129  BVN_CAP3 63200ns */
    0x3ffff04c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_130  UNASSIGNED off */
    0x3ffff04d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_131  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_132  BVN_CAP6 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_133  BVN_CAP7 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_134  BVN_GFD0 off */
    0x3ffff04e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_135  UNASSIGNED off */
    0x3ffff04f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_136  UNASSIGNED off */
    0x3ffff050,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_137  UNASSIGNED off */
    0x3ffff051,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_138  UNASSIGNED off */
    0x3ffff052,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_139  UNASSIGNED off */
    0x3ffff053,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_140  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_141  BVN_MCVP0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_142  BVN_MCVP1 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_143  BVN_MCVP2 off */
    0x00571004,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_144  BVN_RDC 3230ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_145  VEC_VBI_ENC0 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_146  VEC_VBI_ENC1 off */
    0xbffff032,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_147  M2MC_0 RR */
    0xbffff033,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_148  M2MC_1 RR */
    0xbffff034,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_149  M2MC_2 RR */
    0x3ffff054,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_150  UNASSIGNED off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_151  VICE_VIP0_INST2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_152  VICE_VIP1_INST2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_153  VICE1_VIP0_INST2 off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_154  VICE1_VIP1_INST2 off */
    0x80000026,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_155  HVD0_DBLK_p2_0 RR 0ns */
    0x80000027,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_156  HVD0_DBLK_p2_1 RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_157  BVN_MAD4_PIX_FD off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_158  BVN_MAD4_QUANT off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_159  BVN_MAD4_PIX_CAP off */
    0xbffff02f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_160  M2MC1_0 RR */
    0xbffff030,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_161  M2MC1_1 RR */
    0xbffff031,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_162  M2MC1_2 RR */
    0x3ffff055,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_163  UNASSIGNED off */
    0x3ffff056,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_164  UNASSIGNED off */
    0x3ffff057,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_165  UNASSIGNED off */
    0x3ffff058,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_166  UNASSIGNED off */
    0x3ffff059,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_167  UNASSIGNED off */
    0x3ffff05a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_168  UNASSIGNED off */
    0x3ffff05b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_169  UNASSIGNED off */
    0x3ffff05c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_170  UNASSIGNED off */
    0x3ffff05d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_171  UNASSIGNED off */
    0x3ffff05e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_172  UNASSIGNED off */
    0x3ffff05f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_173  UNASSIGNED off */
    0x3ffff060,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_174  UNASSIGNED off */
    0x3ffff061,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_175  UNASSIGNED off */
    0x3ffff062,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_176  UNASSIGNED off */
    0x3ffff063,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_177  UNASSIGNED off */
    0x3ffff064,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_178  UNASSIGNED off */
    0x3ffff065,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_179  UNASSIGNED off */
    0x3ffff066,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_180  UNASSIGNED off */
    0x3ffff067,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_181  UNASSIGNED off */
    0x3ffff068,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_182  UNASSIGNED off */
    0x3ffff069,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_183  UNASSIGNED off */
    0x3ffff06a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_184  UNASSIGNED off */
    0x3ffff06b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_185  UNASSIGNED off */
    0x3ffff06c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_186  UNASSIGNED off */
    0x3ffff06d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_187  UNASSIGNED off */
    0x3ffff06e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_188  UNASSIGNED off */
    0x3ffff06f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_189  UNASSIGNED off */
    0x3ffff070,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_190  UNASSIGNED off */
    0x3ffff071,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_191  UNASSIGNED off */
    0x3ffff072,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_192  UNASSIGNED off */
    0x3ffff073,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_193  UNASSIGNED off */
    0x3ffff074,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_194  UNASSIGNED off */
    0x3ffff075,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_195  UNASSIGNED off */
    0x3ffff076,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_196  UNASSIGNED off */
    0x3ffff077,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_197  UNASSIGNED off */
    0x3ffff078,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_198  UNASSIGNED off */
    0x3ffff079,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_199  UNASSIGNED off */
    0x80142010,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_200  CPU_MCP_RD_HIGH RR 750ns */
    0x80000028,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_201  CPU_MCP_RD_LOW RR */
    0x80261011,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_202  CPU_MCP_WR_HIGH RR 1500ns */
    0x80000029,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_203  CPU_MCP_WR_LOW RR */
    0xbffff02d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_204  V3D_MCP_RD_HIGH RR */
    0xbffff02e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_205  V3D_MCP_RD_LOW RR */
    0xbffff038,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_206  V3D_MCP_WR_HIGH RR */
    0xbffff039,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_207  V3D_MCP_WR_LOW RR */
    0x3ffff07a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_208  UNASSIGNED off */
    0x3ffff07b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_209  UNASSIGNED off */
    0x3ffff07c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_210  UNASSIGNED off */
    0x3ffff07d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_211  UNASSIGNED off */
    0x3ffff07e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_212  UNASSIGNED off */
    0x3ffff07f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_213  UNASSIGNED off */
    0x3ffff080,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_214  UNASSIGNED off */
    0x3ffff081,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_215  UNASSIGNED off */
    0x80000023,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_216  HVD0_PFRI RR 0ns */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_217  HVD1_PFRI off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_218  HVD2_PFRI off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_219  VICE_PFRI off */
    0x3ffff0ff,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_220  VICE1_PFRI off */
    0x3ffff082,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_221  UNASSIGNED off */
    0x3ffff083,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_222  UNASSIGNED off */
    0x3ffff084,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_223  UNASSIGNED off */
    0x3ffff085,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_224  UNASSIGNED off */
    0x3ffff086,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_225  UNASSIGNED off */
    0x3ffff087,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_226  UNASSIGNED off */
    0x3ffff088,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_227  UNASSIGNED off */
    0x3ffff089,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_228  UNASSIGNED off */
    0x3ffff08a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_229  UNASSIGNED off */
    0x3ffff08b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_230  UNASSIGNED off */
    0x3ffff08c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_231  UNASSIGNED off */
    0x3ffff08d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_232  UNASSIGNED off */
    0x3ffff08e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_233  UNASSIGNED off */
    0x3ffff08f,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_234  UNASSIGNED off */
    0x3ffff090,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_235  UNASSIGNED off */
    0x3ffff091,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_236  UNASSIGNED off */
    0x3ffff092,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_237  UNASSIGNED off */
    0x3ffff093,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_238  UNASSIGNED off */
    0x3ffff094,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_239  UNASSIGNED off */
    0x3ffff095,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_240  UNASSIGNED off */
    0x3ffff096,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_241  UNASSIGNED off */
    0x3ffff097,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_242  UNASSIGNED off */
    0x3ffff098,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_243  UNASSIGNED off */
    0x3ffff099,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_244  UNASSIGNED off */
    0x3ffff09a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_245  UNASSIGNED off */
    0x3ffff09b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_246  UNASSIGNED off */
    0x3ffff09c,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_247  UNASSIGNED off */
    0xbffff036,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_248  MEMC_TRACELOG RR */
    0x3ffff09d,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_249  UNASSIGNED off */
    0x3ffff09e,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_250  UNASSIGNED off */
    0xbffff035,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_251  MEMC_MSA RR */
    0xbffff03a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_252  MEMC_DIS0 RR */
    0xbffff03b,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_253  MEMC_DIS1 RR */
    0xbffff02a,  /* BCHP_MEMC_ARB_2_CLIENT_INFO_254  MEMC_DRAM_INIT_ZQCS RR */
    0x00d2d009  /*  BCHP_MEMC_ARB_2_CLIENT_INFO_255  REFRESH 7812.5ns */
};


void BTST_LoadRts
	( const BREG_Handle                hRegister,
  	  const int                        encode,
  	  const int                        usage )
{
	int i;

	for (i=0; i<BTST_LOAD_RTS_RTS_ARRAY_SIZE; i++)
	{
		if (usage ==7439)
		{
			if ((encode==0) || (encode==1))
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7439_1u1tRts_Memc0[i]);
			}
			else if (encode==2)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7439_2u0tRts_Memc0[i]);
			}
			else
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7439_2tRts_Memc0[i]);
			}
		}
		else if (usage == 7252)
		{
			if (encode==0)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7252_1u2tRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7252_1u2tRts_Memc1[i]);
#endif
			}
			else if (encode==1)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7252_4K1tRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7252_4K1tRts_Memc1[i]);
#endif
			}
			else
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7252_4KstbRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7252_4KstbRts_Memc1[i]);
#endif
			}
		}
		else if (usage == 7445)
		{
			if (encode==0)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7445SingleEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7445SingleEncodeRts_Memc1[i]);
#endif
#if BCHP_MEMC_ARB_2_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_2_CLIENT_INFO_0+(i*4), aul7445SingleEncodeRts_Memc2[i]);
#endif
			}
			else if (encode==1)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7445DualEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7445DualEncodeRts_Memc1[i]);
#endif
#if BCHP_MEMC_ARB_2_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_2_CLIENT_INFO_0+(i*4), aul7445DualEncodeRts_Memc2[i]);
#endif
			}
			else if (encode==2)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7445QuadEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7445QuadEncodeRts_Memc1[i]);
#endif
#if BCHP_MEMC_ARB_2_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_2_CLIENT_INFO_0+(i*4), aul7445QuadEncodeRts_Memc2[i]);
#endif
			}
			else if (encode==3)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7445BoxMode1Rts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7445BoxMode1Rts_Memc1[i]);
#endif
#if BCHP_MEMC_ARB_2_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_2_CLIENT_INFO_0+(i*4), aul7445BoxMode1Rts_Memc2[i]);
#endif
			}
		}
		else if (usage == 99)
		{
			if (encode==1)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aulSingleEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aulSingleEncodeRts_Memc1[i]);
#endif
			}
			else
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aulDualEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aulDualEncodeRts_Memc1[i]);
#endif
			}
		}
		else
		{
			if (usage == 0)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7422Usage0_SingleEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7422Usage0_SingleEncodeRts_Memc1[i]);
#endif
			}
			else if (usage == 1)
			{
				BREG_Write32(hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_0+(i*4), aul7422Usage1_SingleEncodeRts_Memc0[i]);
#if BCHP_MEMC_ARB_1_REG_START
				BREG_Write32(hRegister, BCHP_MEMC_ARB_1_CLIENT_INFO_0+(i*4), aul7422Usage1_SingleEncodeRts_Memc1[i]);
#endif
			}
		}
	}
}


/**************************************************************************/
int app_main( int argc, char **argv )
{
	BSystem_Info sysInfo;
	BFramework_Info frmInfo;
	int encode, platform, usage, iErr;
	bool bGoodEntry = false;

	BERR_Code               err = BERR_SUCCESS;

	do
	{
#if (BCHP_CHIP==7439)
                if (argc ==2) {
                   encode = atoi(argv[1]);
                   if (encode >= 4) {
                       printf(" \tNot a valid Entry!\n");
                       return -1;
                   }
                }
                else {
                   printf("7439 Usage: \n");
                   printf("\t 0: Single User Box w/ PiP (1stb0t) \n");
                   printf("\t 1: Single User Box w/ Single Transcode (1u1t) \n");
                   printf("\t 2: Dual HDMI Box (no Pip; no Xcode) (2u0t) \n");
                   printf("\t 3: Headless Dual-Xcode Box (no display) (2t) \n");
                   printf("Which RTS: ");
                   scanf("%d", &encode);
                }

		bGoodEntry = (encode==0 || encode==1 || encode==2 || encode==3) ? true : false;

                switch (encode) {
                case 0:
                   printf("\t 0: Single User Box w/ PiP (1stb0t) \n");
                   break;
                case 1:
                   printf("\t 1: Single User Box w/ Single Transcode (1u1t) \n");
                   break;
                case 2:
                   printf("\t 2: Dual HDMI Box (no Pip; no Xcode) (2u0t) \n");
                   break;
                case 3:
                   printf("\t 3: Headless Dual-Xcode Box (no display) (2t) \n");
                   break;
                default:
                   printf("\tNot supported\n");
                   return -1;
                }

		usage = 7439;
		BSTD_UNUSED(platform);
#elif (BCHP_CHIP==7252)
                if (argc ==2) {
                   encode = atoi(argv[1]);
                   if (encode >= 3) {
                       printf(" \tNot a valid Entry!\n");
                       return -1;
                   }
                }
                else {
                   printf("7252 Usage: \n");
                   printf("\t 0: 1u2t \n");
                   printf("\t 1: 4K1t \n");
                   printf("\t 2: 4Kstb \n");
                   printf("Which RTS: ");
                   scanf("%d", &encode);
                }

		bGoodEntry = (encode==0 || encode==1 || encode==2) ? true : false;

                switch (encode) {
                case 0:
                   printf("\t 0: 1u2t \n");
                   break;
                case 1:
                   printf("\t 1: 4K1t \n");
                   break;
                case 2:
                   printf("\t 2: 4Kstb \n");
                   break;
                default:
                   printf("\tNot supported\n");
                   return -1;
                }

		usage = 7252;
		BSTD_UNUSED(platform);
#elif (BCHP_CHIP==7445)
        if (argc ==2) {
           encode = atoi(argv[1]);
           if (encode >= 5) {
               printf(" \tNot a valid Entry!\n");
               return -1;
           }
        }
        else {
           printf("7445 Usage: \n");
           printf("\t 0: C0 1t \n");
           printf("\t 1: C0 2t \n");
           printf("\t 2: C0 4t \n");
           printf("\t 3: D0 Box Mode 1 \n");
           printf("Which RTS: ");
           scanf("%d", &encode);
        }

        bGoodEntry = (encode==1 || encode==2 || encode==3 || encode==4) ? true : false;

        switch (encode) {
        case 0:
           printf("\t 0: C0 1t \n");
           break;
        case 1:
           printf("\t 1: C0 2t \n");
           break;
        case 2:
           printf("\t 2: C0 4t \n");
           break;
        case 3:
           printf("\t 3: D0 Box Mode 1 \n");
           break;
        default:
           printf("\tNot supported\n");
           return -1;
        }

        usage = 7445;
        BSTD_UNUSED(platform);
#else
		printf("7425(0) or 7422(1): ");
		scanf("%d", &platform);
		if (platform == 1)
		{
			printf("Usage 0(0) or Usage 1(1): ");
			scanf("%d", &usage);

			bGoodEntry = (usage == 0 || usage == 1) ? true : false;

		}
		else
		{
			printf("Single encode(1) or dual encode(2): ");
			scanf("%d", &encode);

			bGoodEntry = (encode == 1 || encode == 2) ? true : false;

			usage = 99;
		}
#endif
	} while (!bGoodEntry);

	/* System Init (interrupts/memory mapping) */
	iErr = BSystem_Init( argc, argv, &sysInfo );
	if ( iErr )
	{
		BDBG_ERR(( "System init FAILED!" ));
		return iErr;
	}

	/* Framework init (base modules) */
	iErr = BFramework_Init( &sysInfo, &frmInfo );
	if ( iErr )
	{
		BDBG_ERR(( "Framework init FAILED!" ));
		return iErr;
	}

	BTST_LoadRts(frmInfo.hReg, encode, usage);

	BFramework_Uninit(&frmInfo);
	BSystem_Uninit(&sysInfo);

	BDBG_MSG(("RTS loaded."));

	return err;
}

/* End of file */
