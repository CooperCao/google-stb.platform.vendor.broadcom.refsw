#include "bstd.h"
#include "brsp.h"
#include "bchp_misc.h"
#include "bchp_rfm_clk27.h"
#include "bchp_rfm_bbe_clk27.h"
#include "bchp_rfm_grb.h"
#include "bchp_rfm_l2.h"
#include "bchp_rfm_sysclk.h"
#include "bchp_rfm_bbe_sysclk.h"
#include "bchp_int_id_rfm_l2.h"
#include "brfm.h"
#include "../brfm_priv.h"
#include "brfm_scripts_bbe.h"


BDBG_MODULE(rfm);


static const uint32_t BRFM_P_BTSCSetup15kHzLeft_BBE[] = 
{
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF1A1,	0x000C6E58,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF1B1,	0x00089C12,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF1B2,	0x0003989C,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF1C1,	0x000868F2,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF1C2,	0x0003EA90,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2A1,	0x0008C660,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2A2,	0x000353A4,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2B1,	0x00087B92,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2B2,	0x0003CD46,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2C1,	0x00085DAE,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_LCF2C2,	0x0003FA34,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

/* Currently not in use	*/
static const uint32_t BRFM_P_BTSCSetup15kHzRight_BBE[] = 
{
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF1A1,	0x000c6e58,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF1B1,	0x00089c12,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF1B2,	0x0003989c,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF1C1,	0x000868f2,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF1C2,	0x0003ea90,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2A1,	0x0008c660,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2A2,	0x000353a4,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2B1,	0x00087b92,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2B2,	0x0003cd46,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2C1,	0x00085dae,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_RCF2C2,	0x0003fa34,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioSetupStereo_BBE[] = 
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,		0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,		0x08cb9000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_VOLCTL,		0x000000c5,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_AUDIOTONE, SEL_TPVID, 0),
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MODPLT,		0x802630c4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLPLT,		0x00002312,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLPR,		0x000057AC,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLMR,		0x0000af58,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLAM,		0x00010000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATLR,		0xaf5857ac,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATSUM,		0x0000af58,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB0,		0x000fffff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB1,		0x00090001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCA1,		0x00000300,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,		0xb1810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,		0x91810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1A1,		0x000c6e58, 
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B1,		0x00089c12,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B2,		0x0003989c,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C1,		0x000868f2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C2,		0x0003ea90,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A1,		0x0008c660,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A2,		0x000353a4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B1,		0x00087b92,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B2,		0x0003cd46,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C1,		0x00085dae,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C2,		0x0003fa34,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,		0x0003f5c2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,		0x0002f996,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,		0x000d25e8,
	BRSP_ScriptType_eWrite32,	BCHP_RFM_BBE_CLK27_AUDIOBYP,	0x00800300,	
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREA1,		0x000b7678,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREA2,		0x0000d732,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB0,		0x0001b7f4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB1,		0x000ca668,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB2,		0x0001a1ca,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrStereo_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioSetupStereo_BBE,	0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_BTSCSetup15kHzLeft_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000	
};		


static const uint32_t BRFM_P_AudioEncodingScrMonoCommon_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_VOLCTL,	0x000000c5,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_AUDIOTONE, SEL_TPVID, 0),
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MODPLT,	0x002630c4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLPLT,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLPR,	0x000057AC,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLMR,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLAM,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATLR,	0xffffffff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATSUM,	0x0000ffff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1A1,	0x000c6e58,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B1,	0x00089c12,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B2,	0x0003989c,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C1,	0x000868f2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C2,	0x0003ea90,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A1,	0x0008c660,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A2,	0x000353a4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B1,	0x00087b92,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B2,	0x0003cd46,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C1,	0x00085dae,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C2,	0x0003fa34,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_AUDIOBYP,	0x00000300,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoNtsc_BBE[] =
{
	/* NTSC Mono Script, uses 75us Pre-emphasis Filter */
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,	0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,	0x08cb9000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioEncodingScrMonoCommon_BBE,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,	0x0003f5c2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,	0x0002f996,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,	0x000d25e8,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_BTSCSetup15kHzLeft_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoPalSecam_BBE[] =
{
	/* Pal/Secam Mono Script, uses 50us Pre-emphasis Filter */
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,	0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,	0x11973000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioEncodingScrMonoCommon_BBE,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,	0x0003F5C2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,	0x000200FA,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,	0x000E1E84,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_BTSCSetup15kHzLeft_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoPalD_BBE[] =
{
	/* PalD Mono Script, uses 50us Pre-emphasis Filter */
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,	0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,	0x11973000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioEncodingScrMonoCommon_BBE,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,	0x0003F5C2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,	0x000200FA,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,	0x000E1E84,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};


static const uint32_t BRFM_P_AudioEncodingScrSap_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,		0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,		0x08cb9000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_VOLCTL,	0x0000018a,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_AUDIOTONE, SEL_TPVID, 0),
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MODPLT,	0x002630c4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLPLT,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLPR,	0x000057AC,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLMR,	0x0000349A,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLAM,		0x00006117,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATLR,		0xaf58af58,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATSUM,	0x0000af58,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,	0xb1810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,	0x91810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1A1,	0x000C55F2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B1,	0x00086E6A,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1B2,	0x0003AE16,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C1,	0x00083DE0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF1C2,	0x0003EEC0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A1,	0x000896A0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2A2,	0x000377F8,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B1,	0x00084F92,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2B2,	0x0003D776,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C1,	0x000833A4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RCF2C2,	0x0003FB48,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,	0x0003f5c2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,	0x0002f996,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,	0x000d25e8,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB0,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB1,	0x00020000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCA1,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_AUDIOBYP,	0x01800380,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREA1,	0x000b7678,	
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREA2,	0x0000d732,	
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB0,	0x0001b7f4,	
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB1,	0x000ca668,	
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SPREB2,	0x0001a1ca,	
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrSapMono_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioEncodingScrSap_BBE,	0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_BTSCSetup15kHzLeft_BBE,	0x00000000,
};


static const uint32_t BRFM_P_AudioEncodingScrJpMono1_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMCTL,		0x00010fff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMSCL,		0x08cb9000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_VOLCTL,	0x0000018a,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_AUDIOTONE, SEL_TPVID, 0),
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MODPLT,	0x002630c4,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLPLT,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLPR,	0x000057AC,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLLMR,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SCLAM,		0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATLR,		0xffffffff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_SATSUM,	0x0000ffff,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,	0xb1810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_RMGRCTL1,	0x91810F0F,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREA1,	0x0003f5c2,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB0,	0x0002f996,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_MPREB1,	0x000d25e8,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB0,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCB1,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_OVRSPCA1,	0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_AUDIOBYP,	0x00000380,	
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_BTSCSetup15kHzLeft_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,					0x00000000
};

static const uint32_t BRFM_P_ChScrSIF_BBE[] =
{
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_FILSCL, AUDSCL, 0xfff),
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 	0x00002000,*/
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_SYSCLK_MODBYP, BYP_MIX, 1),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_SYSCLK_MODBYP, SEL_MIX, 1),
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableCh3_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableCh4_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableBaseband_BBE[] =
{
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_CLK27_FILSCL, AUDSCL, 0xfff),
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 		0x00002000,*/
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_SYSCLK_MODBYP, BYP_MIX, 1),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_BBE_SYSCLK_MODBYP, SEL_MIX, 1),
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_InitScrDualOutputMode[] = 
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_SYSCLK_MISC, 0x0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_SYSCLK_TSTCNTL, 0x0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_SYSCLK_MODTST, 0x0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_SYSCLK_OUTSCL, 0x0000003f,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DACCTL, TC_OBB_2, 1),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DACCTL, DAC_SEL, 1),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DACCTL, DAC_PWRDN_2, 0),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DACCTL, DAC_VCSC_2, 4),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DACCTL, CLK_INV_2, 1),
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_InitScrNtscOpenCableConfig_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMFREQ, 0x2AAAAAAB,		/* 4.5MHz audio carrier*/
/*	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_GroupDelayFCCScrNtsc, 0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioTrapScrNtsc, 0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | PLL_MULT),
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP1, 0x7ff88000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP2, 0x7ffc8000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIPBYP, 0x00010001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x30002000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL1, 0x12E401B0,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2, 0x70000001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODTST, 0x00000504,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODBYP, 0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL, 0x0000003f,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOBYP, 0x00000000,
#if ((BCHP_CHIP==7400) && (BCHP_VER >= BCHP_VER_B0)) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_TSTCNTL, 0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x28408001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_PLL_MISC1, 0x00000020,
#else
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TSTCNTL, 0x00000000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x3f000001,
#endif
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC, PLL_MISC, */
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_InitScrDualOutputMode, 0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalICh3_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalICh4_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00, 		0x00000000
};

static const uint32_t BRFM_P_ChScrPalIBaseband_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000

};

static const uint32_t BRFM_P_InitScrPalIConfig_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMFREQ, 0x38E38E39,		/* 6.0 MHz audio carrier*/
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | PLL_MULT),
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL, 0x46D00000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioTrapScrPalI, 0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_GroupDelayScrPalI, 0x00000000,
#if ((BCHP_CHIP==7400) && (BCHP_VER >= BCHP_VER_B0)) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x28408001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2, 0x70000001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL, 0x0000001f,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x3f000001,
#endif */
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_InitScrDualOutputMode, 0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDCh3_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDCh4_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDBaseband_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_InitScrPalDConfig_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMFREQ, 0x3DA12F68,	/* 6.5 MHz audio carrier*/
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | PLL_MULT),
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL, 0x32200000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioTrapScrPalD, 0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_GroupDelayScrPalD, 0x00000000,
#if ((BCHP_CHIP==7400) && (BCHP_VER >= BCHP_VER_B0)) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x28408001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2, 0x70000001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL, 0x0000001f,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x3f000001,
#endif */
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_InitScrDualOutputMode, 0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaCh3_BBE[] =
{	
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaCh4_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaBaseband_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_InitScrPalDChinaConfig_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMFREQ, 0x3DA12F68,		/* 6.5 MHz audio carrier*/
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | PLL_MULT),
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL, 0x32200000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioTrapScrPalD, 0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_GroupDelayScrPalD_China, 0x00000000,
#if ((BCHP_CHIP==7400) && (BCHP_VER >= BCHP_VER_B0)) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x28408001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2, 0x70000001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL, 0x0000001f,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else	
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x3f000001,
#endif*/
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_InitScrDualOutputMode, 0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGCh3_BBE[] =
{	
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGCh4_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGBaseband_BBE[] =
{
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_ChScrSIF_BBE,	0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};

static const uint32_t BRFM_P_InitScrPalBGConfig_BBE[] =
{
	BRSP_ScriptType_eWrite32, BCHP_RFM_BBE_CLK27_FMFREQ, 0x3425ED09,		/* 5.5 MHz audio carrier*/
/*	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | PLL_MULT),
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL, 0x32200000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_AudioTrapScrPalBG, 0x00000000,
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_GroupDelayScrPalBG, 0x00000000,
#if ((BCHP_CHIP==7400) && (BCHP_VER >= BCHP_VER_B0)) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x28408001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2, 0x70000001,
	BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL, 0x0000001f,
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
	BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
	BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL, 0x3f000001,
#endif */
	BRSP_ScriptType_eNestedScript, (uint32_t) BRFM_P_InitScrDualOutputMode, 0x00000000,
	BRSP_ScriptType_eEndOfScript, 0x00,			0x00000000
};



static const BRFM_P_ModulationInfo BRFM_P_ModInfo_BBE[] =
{
	{
		BRFM_ModulationType_eNtscOpenCable,
		{
			BRFM_P_ChScrNtscOpenCableBaseband_BBE,
			BRFM_P_ChScrNtscOpenCableCh3_BBE,
			BRFM_P_ChScrNtscOpenCableCh4_BBE,
			BRFM_P_ChScrSIF_BBE
		},
		{
			BRFM_P_AudioEncodingScrMonoNtsc_BBE,
			BRFM_P_AudioEncodingScrStereo_BBE,
			BRFM_P_AudioEncodingScrSap_BBE,					/* SAP only, same as Sap/Mono, but volume for Mono is diabled */
			BRFM_P_AudioEncodingScrSapMono_BBE,			/* Sap/Mono */
			BRFM_P_AudioEncodingScrJpMono1_BBE,			/* Japan Mono1 */
			BRFM_P_AudioEncodingScrMonoNtsc_BBE,			/* Japan Mono2, Same as Mono */
			NULL,
			NULL,										/* No NICAM support on NTSC */
			NULL
		},
		BRFM_P_InitScrNtscOpenCableConfig_BBE
	},
	{
		BRFM_ModulationType_ePalI,
		{
			BRFM_P_ChScrPalIBaseband_BBE,
			BRFM_P_ChScrPalICh3_BBE,
			BRFM_P_ChScrPalICh4_BBE,
			BRFM_P_ChScrSIF_BBE
		},
		{
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL only support Mono */
			NULL,		/* No NICAM support on second BTSC encoder */
			NULL,
			NULL
		},
		BRFM_P_InitScrPalIConfig_BBE
	},
	{
		BRFM_ModulationType_ePalD,
		{
			BRFM_P_ChScrPalDBaseband_BBE,
			BRFM_P_ChScrPalDCh3_BBE,
			BRFM_P_ChScrPalDCh4_BBE,
			BRFM_P_ChScrSIF_BBE
		},
		{
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			NULL,		/* No NICAM support on second BTSC encoder */
			NULL,
			NULL
		},
		BRFM_P_InitScrPalDConfig_BBE
	},
	{
		BRFM_ModulationType_ePalDChina,
		{
			BRFM_P_ChScrPalDChinaBaseband_BBE,
			BRFM_P_ChScrPalDChinaCh3_BBE,
			BRFM_P_ChScrPalDChinaCh4_BBE,
			BRFM_P_ChScrSIF_BBE
		},
		{
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalD_BBE,	/* PAL-D only support Mono */
			NULL,		/* No NICAM support on second BTSC encoder */
			NULL,
			NULL
		},
		BRFM_P_InitScrPalDChinaConfig_BBE
	},
	{
		BRFM_ModulationType_ePalBG,
		{
			BRFM_P_ChScrPalBGBaseband_BBE,
			BRFM_P_ChScrPalBGCh3_BBE,
			BRFM_P_ChScrPalBGCh4_BBE,
			BRFM_P_ChScrSIF_BBE
		},
		{
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			BRFM_P_AudioEncodingScrMonoPalSecam_BBE,	/* PAL-D only support Mono */
			NULL,		/* No NICAM support on second BTSC encoder */
			NULL,
			NULL
		},
		BRFM_P_InitScrPalBGConfig_BBE
	}
};

const BRFM_P_ModulationInfo *BRFM_P_GetModInfoPtr_BBE(
	BRFM_ModulationType modType				/* [in] Requested modulation type */
	)
{
	unsigned idx;
	const BRFM_P_ModulationInfo *retVal;

	retVal = NULL;
	for( idx = 0; idx < (sizeof(BRFM_P_ModInfo_BBE) / sizeof(BRFM_P_ModulationInfo)); idx++ )
	{
		if( BRFM_P_ModInfo_BBE[idx].modType == modType )
		{
			retVal = &BRFM_P_ModInfo_BBE[idx];
			BDBG_MSG(("BRFM_P_GetModInfoPtr_BBE: idx(%d)", idx));
			break;
		}
	}
	return( retVal );
}
