/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef _ABSTRACT_V3D_H_
#define _ABSTRACT_V3D_H_

EGLAPI BEGL_DriverInterfaces* EGLAPIENTRY BEGL_GetDriverInterfaces(void);

#define v3d_read(REG) (0)
#define v3d_write(REG, X)

#define _di()
#define _ei()

#define L2CACTL      BEGL_HW_L2CACTL
#define SLCACTL      BEGL_HW_SLCACTL
#define INTCTL       BEGL_HW_INTCTL
#define INTENA       BEGL_HW_INTENA
#define INTDIS       BEGL_HW_INTDIS
#define CT0CS        BEGL_HW_CT0CS
#define CT1CS        BEGL_HW_CT1CS
#define CT0EA        BEGL_HW_CT0EA
#define CT1EA        BEGL_HW_CT1EA
#define CT0CA        BEGL_HW_CT0CA
#define CT1CA        BEGL_HW_CT1CA
#define PCS          BEGL_HW_PCS
#define BFC          BEGL_HW_BFC
#define RFC          BEGL_HW_RFC
#define BPOA         BEGL_HW_BPOA
#define BPOS         BEGL_HW_BPOS
#define CACHE_CTRL   BEGL_HW_CACHE_CTRL
#define PCTRC        BEGL_HW_PCTRC
#define PCTRE        BEGL_HW_PCTRE
#define PCTR0        BEGL_HW_PCTR0
#define PCTRS0       BEGL_HW_PCTRS0
#define PCTR1        BEGL_HW_PCTR1
#define PCTRS1       BEGL_HW_PCTRS1
#define PCTR2        BEGL_HW_PCTR2
#define PCTRS2       BEGL_HW_PCTRS2
#define PCTR3        BEGL_HW_PCTR3
#define PCTRS3       BEGL_HW_PCTRS3
#define PCTR4        BEGL_HW_PCTR4
#define PCTRS4       BEGL_HW_PCTRS4
#define PCTR5        BEGL_HW_PCTR5
#define PCTRS5       BEGL_HW_PCTRS5
#define PCTR6        BEGL_HW_PCTR6
#define PCTRS6       BEGL_HW_PCTRS6
#define PCTR7        BEGL_HW_PCTR7
#define PCTRS7       BEGL_HW_PCTRS7
#define PCTR8        BEGL_HW_PCTR8
#define PCTRS8       BEGL_HW_PCTRS8
#define PCTR9        BEGL_HW_PCTR9
#define PCTRS9       BEGL_HW_PCTRS9
#define PCTR10       BEGL_HW_PCTR10
#define PCTRS10      BEGL_HW_PCTRS10
#define PCTR11       BEGL_HW_PCTR11
#define PCTRS11      BEGL_HW_PCTRS11
#define PCTR12       BEGL_HW_PCTR12
#define PCTRS12      BEGL_HW_PCTRS12
#define PCTR13       BEGL_HW_PCTR13
#define PCTRS13      BEGL_HW_PCTRS13
#define PCTR14       BEGL_HW_PCTR14
#define PCTRS14      BEGL_HW_PCTRS14
#define PCTR15       BEGL_HW_PCTR15
#define PCTRS15      BEGL_HW_PCTRS15
#define PM_SEL       BEGL_HW_GCA_PM_SEL
#define BW_CNT       BEGL_HW_GCA_BW_CNT
#define BW_MEM_CNT   BEGL_HW_GCA_BW_MEM_CNT
#define SQRSV0       BEGL_HW_SQRSV0
#define SQRSV1       BEGL_HW_SQRSV1
#define DBQHLT       BEGL_HW_DBQHLT
#define DBQRUN       BEGL_HW_DBQRUN
#define SRQCS        BEGL_HW_SRQCS
#define SRQUA        BEGL_HW_SRQUA
#define SRQPC        BEGL_HW_SRQPC
#define DBQITC       BEGL_HW_DBQITC
#define DBQITE       BEGL_HW_DBQITE
#define IDENT0       BEGL_HW_IDENT0
#define IDENT1             BEGL_HW_IDENT1
#define IDENT2             BEGL_HW_IDENT2
#define IDENT3             BEGL_HW_IDENT3
#define SQCNTL             BEGL_HW_SQCNTL
#define SQCSTAT            BEGL_HW_SQCSTAT
#define SRQPC              BEGL_HW_SRQPC
#define SRQUA              BEGL_HW_SRQUA
#define SRQUL              BEGL_HW_SRQUL
#define SRQCS              BEGL_HW_SRQCS
#define SCRATCH            BEGL_HW_SCRATCH
#define ENDSWP             BEGL_HW_ENDSWP
#define VPACNTL            BEGL_HW_VPACNTL
#define VPMBASE            BEGL_HW_VPMBASE
#define BPCA               BEGL_HW_BPCA
#define BPCS               BEGL_HW_BPCS
#define BXCF               BEGL_HW_BXCF
#define CACHE_ID           BEGL_HW_CACHE_ID
#define CACHE_SWAP_CTRL    BEGL_HW_CACHE_SWAP_CTRL
#define CACHE_SWAP_CTRL_2  BEGL_HW_CACHE_SWAP_CTRL_2
#define CACHE_STATUS       BEGL_HW_CACHE_STATUS
#define PM_CTRL            BEGL_HW_PM_CTRL
#define V3D_BW_CNT         BEGL_HW_V3D_BW_CNT
#define MEM_BW_CNT         BEGL_HW_MEM_BW_CNT
#define LOW_PRI_ID         BEGL_HW_LOW_PRI_ID
#define MEM_PROTECT_0      BEGL_HW_MEM_PROTECT_0
#define MEM_PROTECT_1      BEGL_HW_MEM_PROTECT_1
#define MEM_PROTECT_2      BEGL_HW_MEM_PROTECT_2
#define MEM_PROTECT_3      BEGL_HW_MEM_PROTECT_3
#define MEM_PROTECT_4      BEGL_HW_MEM_PROTECT_4
#define MEM_PROTECT_5      BEGL_HW_MEM_PROTECT_5
#define MEM_PROTECT_6      BEGL_HW_MEM_PROTECT_6
#define MEM_PROTECT_7      BEGL_HW_MEM_PROTECT_7
#define MEM_PROTECT_STATUS BEGL_HW_MEM_PROTECT_STATUS
#define AXI_BRIDGE_STATUS  BEGL_HW_AXI_BRIDGE_STATUS
#define SCRATCH_0          BEGL_HW_SCRATCH_0
#define SAFE_SHUTDOWN      BEGL_HW_SAFE_SHUTDOWN
#define SAFE_SHUTDOWN_ACK  BEGL_HW_SAFE_SHUTDOWN_ACK
#define DBCFG              BEGL_HW_DBCFG
#define DBSCS              BEGL_HW_DBSCS
#define DBSCFG             BEGL_HW_DBSCFG
#define DBSSR              BEGL_HW_DBSSR
#define DBSDR0             BEGL_HW_DBSDR0
#define DBSDR1             BEGL_HW_DBSDR1
#define DBSDR2             BEGL_HW_DBSDR2
#define DBSDR3             BEGL_HW_DBSDR3
#define DBQRUN             BEGL_HW_DBQRUN
#define DBQHLT             BEGL_HW_DBQHLT
#define DBQSTP             BEGL_HW_DBQSTP
#define DBQITE             BEGL_HW_DBQITE
#define DBQITC             BEGL_HW_DBQITC
#define DBQGHC             BEGL_HW_DBQGHC
#define DBQGHG             BEGL_HW_DBQGHG
#define DBQGHH             BEGL_HW_DBQGHH
#define DBGE               BEGL_HW_DBGE
#define FDBGO              BEGL_HW_FDBGO
#define FDBGB              BEGL_HW_FDBGB
#define FDBGR              BEGL_HW_FDBGR
#define FDBGS              BEGL_HW_FDBGS
#define ERRSTAT            BEGL_HW_ERRSTAT
#define CT00RA0            BEGL_HW_CT00RA0
#define CT01RA0            BEGL_HW_CT01RA0
#define CT0LC              BEGL_HW_CT0LC
#define CT1LC              BEGL_HW_CT1LC
#define CT0PC              BEGL_HW_CT0PC
#define CT1PC              BEGL_HW_CT1PC
#define BRIDGE_REVISION    BEGL_HW_BRIDGE_REVISION
#define BRIDGE_CTRL        BEGL_HW_BRIDGE_CTRL
#define BRIDGE_SW_INIT_0   BEGL_HW_BRIDGE_SW_INIT_0
#define BRIDGE_SW_INIT_1   BEGL_HW_BRIDGE_SW_INIT_1
#define INTDONE            BEGL_HW_INTDONE

#define v3d_full_memory_barrier khrn_hw_full_memory_barrier


#endif // _ABSTRACT_V3D_H_
