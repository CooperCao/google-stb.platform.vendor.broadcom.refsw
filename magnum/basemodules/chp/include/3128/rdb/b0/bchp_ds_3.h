/***************************************************************************
 *     Copyright (c) 1999-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Fri Jul  1 17:27:39 2011
 *                 MD5 Checksum         b6f04dd078056cfbbfc525ddc19ac701
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_DS_3_H__
#define BCHP_DS_3_H__

/***************************************************************************
 *DS_3 - Downstream Receiver 3 Registers
 ***************************************************************************/
#define BCHP_DS_3_GBL                            0x000aa000 /* Global Core Control Register */
#define BCHP_DS_3_PD                             0x000aa010 /* Global Power Down Register */
#define BCHP_DS_3_IRQSTS1                        0x000aa040 /* Interrupt Status Register 1 */
#define BCHP_DS_3_IRQSET1                        0x000aa044 /* Set Interrupt Status Register 1 */
#define BCHP_DS_3_IRQCLR1                        0x000aa048 /* Clear Interrupt Status Register 1 */
#define BCHP_DS_3_IRQMSK1                        0x000aa04c /* Interrupt Mask Register 1 */
#define BCHP_DS_3_IRQMSET1                       0x000aa050 /* Set Interrupt Mask Register 1 */
#define BCHP_DS_3_IRQMCLR1                       0x000aa054 /* Clear Interrupt Mask Register 1 */
#define BCHP_DS_3_IRQSTS2                        0x000aa058 /* Interrupt Status Register 2 */
#define BCHP_DS_3_IRQSET2                        0x000aa05c /* Set Interrupt Status Register 2 */
#define BCHP_DS_3_IRQCLR2                        0x000aa060 /* Clear Interrupt Status Register 2 */
#define BCHP_DS_3_IRQMSK2                        0x000aa064 /* Interrupt Mask Register 2 */
#define BCHP_DS_3_IRQMSET2                       0x000aa068 /* Set Interrupt Mask Register 2 */
#define BCHP_DS_3_IRQMCLR2                       0x000aa06c /* Clear Interrupt Mask Register 2 */
#define BCHP_DS_3_STAT                           0x000aa0fc /* Receiver Status Register */
#define BCHP_DS_3_RST                            0x000aa100 /* Global Reset Register */
#define BCHP_DS_3_FRZ                            0x000aa104 /* Global Freeze Register */
#define BCHP_DS_3_BURST_FRZ                      0x000aa10c /* Global Burst noise detector Freeze  Register */
#define BCHP_DS_3_CLK                            0x000aa184 /* Clock Generation Control Register */
#define BCHP_DS_3_NCOU                           0x000aa190 /* NCO Instantaneous Value (Upper) */
#define BCHP_DS_3_NCOL                           0x000aa194 /* NCO Instantaneous Value (Lower) */
#define BCHP_DS_3_FECIN_NCON                     0x000aa198 /* FEC Clock Counter Numerator Value */
#define BCHP_DS_3_FECIN_NCODL                    0x000aa19c /* FEC Clock Counter Delta Value */
#define BCHP_DS_3_FECOUT_NCON                    0x000aa1a0 /* OI Clock Rate Numerator */
#define BCHP_DS_3_FECOUT_NCODL                   0x000aa1a4 /* OI Clock Rate Delta */
#define BCHP_DS_3_US_FCW_DWELL                   0x000aa1a8 /* Upstream Frequency Control Word Dwell-count register */
#define BCHP_DS_3_SGCG                           0x000aa1ac /* Clockgen Signature Analyzer */
#define BCHP_DS_3_ICB_CTL                        0x000aa200 /* Internal Configuration Bus Control and Status */
#define BCHP_DS_3_MBOX_CSR_P                     0x000aa204 /* Mail Box Command and Status for processor */
#define BCHP_DS_3_MBOX_DATA_P                    0x000aa208 /* Mail Box Data for processor */
#define BCHP_DS_3_HI_TST                         0x000aa214 /* Test configuration for down steam core's bus interface */
#define BCHP_DS_3_MBOX_CSR_S                     0x000aa218 /* Mail Box Command and Status for serial test interface. */
#define BCHP_DS_3_MBOX_DATA_S                    0x000aa21c /* Mail Box Data for serial test interface */
#define BCHP_DS_3_BR                             0x000aa300 /* Baud Receiver Control Register */
#define BCHP_DS_3_AGCB                           0x000aa34c /* Digital AGCB Control Register */
#define BCHP_DS_3_AGCBI                          0x000aa350 /* Digital AGCB Gain Integrator Value */
#define BCHP_DS_3_AGCBLI                         0x000aa354 /* Digital AGCB Power Leaky Integrator Value */
#define BCHP_DS_3_SGBR                           0x000aa358 /* Baud Receiver Signature Analyzer */
#define BCHP_DS_3_QMLPS                          0x000aa400 /* QAM Loop Control */
#define BCHP_DS_3_CFL                            0x000aa410 /* Carrier Frequency Loop Control Register */
#define BCHP_DS_3_CFLC                           0x000aa414 /* Carrier Frequency Loop Coefficient Control Register */
#define BCHP_DS_3_CFLI                           0x000aa418 /* Carrier Frequency Loop Integrator Value */
#define BCHP_DS_3_CFLSP                          0x000aa41c /* Carrier Frequency Loop Sweep Control Register */
#define BCHP_DS_3_CFLFOS                         0x000aa480 /* Carrier Frequency Loop Frequency Offset Control Register */
#define BCHP_DS_3_CFLFO                          0x000aa488 /* Carrier Frequency Loop Filter Output Value */
#define BCHP_DS_3_TL                             0x000aa494 /* Timing Loop Control Register */
#define BCHP_DS_3_TLC                            0x000aa498 /* Timing Loop Coefficient Control Register */
#define BCHP_DS_3_TLI                            0x000aa49c /* Timing Loop Integrator Value */
#define BCHP_DS_3_TLSWP                          0x000aa4a0 /* Timing Loop Sweep Control Value */
#define BCHP_DS_3_TLTHRS                         0x000aa4a4 /* Timing Loop Integrator Threshold Register */
#define BCHP_DS_3_TLFOS                          0x000aa4a8 /* Timing Loop Phase Offset Control Register */
#define BCHP_DS_3_TLFO                           0x000aa4ac /* Timing Loop Filter Output Value */
#define BCHP_DS_3_TLAGC                          0x000aa504 /* Timing Loop AGC Control Register */
#define BCHP_DS_3_TLAGCI                         0x000aa508 /* Timing Loop AGC Integrator Value */
#define BCHP_DS_3_TLAGCL                         0x000aa50c /* Timing Loop AGC Leaky Integrator Value */
#define BCHP_DS_3_PERF                           0x000aa510 /* Performance Monitoring Control/Status Register */
#define BCHP_DS_3_TLDHT                          0x000aa514 /* Timing Lock Detector High Threshold Control Register */
#define BCHP_DS_3_TLDLT                          0x000aa518 /* Timing Lock Detector Low Threshold Control Register */
#define BCHP_DS_3_TLDA                           0x000aa51c /* Timing Lock Detector Accumulator Value */
#define BCHP_DS_3_TLDC                           0x000aa520 /* Timing Lock Detector Maximum Count Control Register */
#define BCHP_DS_3_TLDCI                          0x000aa524 /* Timing Lock Detector Counter Value */
#define BCHP_DS_3_US_IFC                         0x000aa530 /* Upstream/Downstream interface control register */
#define BCHP_DS_3_US_FCW_HI                      0x000aa534 /* Upper-part of the Upstream Frequency Control Word register */
#define BCHP_DS_3_US_FCW_LO                      0x000aa538 /* Lower-part of the Upstream Frequency Control Word register */
#define BCHP_DS_3_US_TL_OFFSET                   0x000aa53c /* Upstream Timing Offset register */
#define BCHP_DS_3_US_DSBCLK                      0x000aa540 /* Upstream baud clock register */
#define BCHP_DS_3_FEC                            0x000aa600 /* FEC Control / Status Register */
#define BCHP_DS_3_FECU                           0x000aa610 /* FEC Initialization Register (Upper) */
#define BCHP_DS_3_FECM                           0x000aa614 /* FEC Initialization Register (Middle) */
#define BCHP_DS_3_FECL                           0x000aa618 /* FEC Initialization Register (Lower) */
#define BCHP_DS_3_SGFEC                          0x000aa620 /* FEC Signature Analyzer */
#define BCHP_DS_3_OI_VCO                         0x000aa640 /* OI VCO Control */
#define BCHP_DS_3_OI_CTL                         0x000aa680 /* OI Control */
#define BCHP_DS_3_OI_OUT                         0x000aa684 /* OI PS Output Control */
#define BCHP_DS_3_OI_ERR                         0x000aa69c /* OI Frame Error Count */
#define BCHP_DS_3_OI_SG                          0x000aa6a0 /* Output Interface Signature Analyzer (Test) */
#define BCHP_DS_3_OI_BER_CTL                     0x000aa6a4 /* OI BER Estimation Control Register */
#define BCHP_DS_3_OI_BER                         0x000aa6a8 /* OI BER Estimation Error Counter Value */
#define BCHP_DS_3_BER                            0x000aa700 /* Pre-FEC BER Estimation Control Register */
#define BCHP_DS_3_BERI                           0x000aa704 /* Pre-FEC BER Estimation Error Counter Value */
#define BCHP_DS_3_CERC1                          0x000aa710 /* FEC RS Corrected Bit Counter */
#define BCHP_DS_3_UERC1                          0x000aa714 /* FEC Uncorrectable RS-Block Counter */
#define BCHP_DS_3_NBERC1                         0x000aa718 /* FEC Clean RS-Block Counter */
#define BCHP_DS_3_CBERC1                         0x000aa71c /* FEC Corrected RS-Block Counter */
#define BCHP_DS_3_BMPG1                          0x000aa720 /* FEC Bad MPEG-Packet Counter */
#define BCHP_DS_3_CERC2                          0x000aa724 /* FEC RS Corrected Bit Counter */
#define BCHP_DS_3_UERC2                          0x000aa728 /* FEC Uncorrectable RS-Block Counter */
#define BCHP_DS_3_NBERC2                         0x000aa72c /* FEC Clean RS-Block Counter */
#define BCHP_DS_3_CBERC2                         0x000aa730 /* FEC Corrected RS-Block Counter */
#define BCHP_DS_3_BMPG2                          0x000aa734 /* FEC Bad MPEG-Packet Counter */
#define BCHP_DS_3_TPFEC                          0x000aa738 /* Testport Control Register for FEC */
#define BCHP_DS_3_EUSEDC1                        0x000aa73c /* FEC Erasure used RS-Block Counter */
#define BCHP_DS_3_EDISCARDC1                     0x000aa740 /* FEC Erasure discarded RS-Block Counter */
#define BCHP_DS_3_EUSEDC2                        0x000aa744 /* FEC Erasure used RS-Block Counter */
#define BCHP_DS_3_EDISCARDC2                     0x000aa748 /* FEC Erasure discarded RS-Block Counter */
#define BCHP_DS_3_FECMON_CTL                     0x000aa74c /* FEC Monitor Control */
#define BCHP_DS_3_FECMON_LOCK_LIMITS             0x000aa750 /* FEC Monitor Lock Limits */
#define BCHP_DS_3_FECMON_UNLOCK_LIMITS           0x000aa754 /* FEC Monitor BUnlock Limits */
#define BCHP_DS_3_FECMON_FALSE_MPEG_DETECT_CTL1  0x000aa758 /* FEC Monitor False MPEG Detect Control Parameters 1 */
#define BCHP_DS_3_FECMON_FALSE_MPEG_DETECT_CTL2  0x000aa75c /* FEC Monitor False MPEG Detect Control Parameters 2 */
#define BCHP_DS_3_FECMON_FALSE_MPEG_DETECT_STATUS 0x000aa760 /* FEC Monitor False MPEG Detect Status */
#define BCHP_DS_3_FECMON_INTERVAL_ALERT_THRESHOLD 0x000aa764 /* FEC Monitor Error Count Alert Thresholds */
#define BCHP_DS_3_FECMON_INTERVAL_ERROR_VALUES   0x000aa768 /* FEC Monitor Error Counts within the last trigger interval */
#define BCHP_DS_3_FSCNT1                         0x000aa790 /* Sample Rate Counter 1 */
#define BCHP_DS_3_FSCNT2                         0x000aa794 /* Sample Rate Counter 2 */
#define BCHP_DS_3_FBCNT1                         0x000aa7c0 /* Baud Rate Counter 1 */
#define BCHP_DS_3_FBCNT2                         0x000aa7c4 /* Baud Rate Counter 2 */
#define BCHP_DS_3_SPARE                          0x000aa7fc /* Reserved for Future Expansion */
#define BCHP_DS_3_BND                            0x000aab00 /* BND Control Register */
#define BCHP_DS_3_BND_THR                        0x000aab04 /* BND threshold value Register */
#define BCHP_DS_3_BND_FRZ                        0x000aab08 /* BND Freeze Control Register */
#define BCHP_DS_3_BND_THRFRZ                     0x000aab0c /* BND threshold value Register */
#define BCHP_DS_3_EQ_CTL                         0x000aac00 /* Equalizer Control Register */
#define BCHP_DS_3_EQ_CWC                         0x000aac04 /* CWC Control Register */
#define BCHP_DS_3_EQ_CWC_FSBAUD                  0x000aac08 /* CWC BAUD SAMPLE RATE */
#define BCHP_DS_3_EQ_CWC_FSCARR                  0x000aac0c /* CWC CARRIER SAMPLE RATE */
#define BCHP_DS_3_EQ_FFE                         0x000aac10 /* FFE Control Register */
#define BCHP_DS_3_EQ_DFE                         0x000aac14 /* DFE Control Register */
#define BCHP_DS_3_EQ_CMA                         0x000aac18 /* Equalizer CMA Modulus Control Register */
#define BCHP_DS_3_EQ_RCA                         0x000aac1c /* Equalizer RCA Gain Control Register */
#define BCHP_DS_3_EQ_FMTHR                       0x000aac20 /* Equalizer Main Tap Threshold Control Register */
#define BCHP_DS_3_EQ_LEAK                        0x000aac24 /* Equalizer Leakage Control Register */
#define BCHP_DS_3_EQ_SOFT                        0x000aac28 /* Equalizer Soft Decision Value */
#define BCHP_DS_3_EQ_SGEQ                        0x000aac2c /* Equalizer Signature Analyzer */
#define BCHP_DS_3_EQ_CPL                         0x000aac30 /* Carrier Phase Loop Control */
#define BCHP_DS_3_EQ_CPLC                        0x000aac34 /* Carrier Phase Loop Coefficients */
#define BCHP_DS_3_EQ_CPLSWP                      0x000aac38 /* Carrier Phase Loop Sweep */
#define BCHP_DS_3_EQ_CPLI                        0x000aac3c /* Carrier Phase Loop Integrator */
#define BCHP_DS_3_EQ_CPLPA                       0x000aac40 /* Carrier Phase Loop Phase Accumulator */
#define BCHP_DS_3_EQ_CPLFO                       0x000aac44 /* Carrier Phase Loop Filter Output */
#define BCHP_DS_3_EQ_SNR                         0x000aac48 /* Slicer SNR */
#define BCHP_DS_3_EQ_SNRHT                       0x000aac4c /* Slicer SNR High Threshold */
#define BCHP_DS_3_EQ_SNRLT                       0x000aac50 /* Slicer SNR Low Threshold */
#define BCHP_DS_3_EQ_CLD                         0x000aac54 /* Carrier Lock Detector */
#define BCHP_DS_3_EQ_CLDHT                       0x000aac58 /* Carrier Lock Detector High Threshold */
#define BCHP_DS_3_EQ_CLDLT                       0x000aac5c /* Carrier Lock Detector Low Threshold */
#define BCHP_DS_3_EQ_CLDA                        0x000aac60 /* Carrier Lock Detector Accumulator */
#define BCHP_DS_3_EQ_CLDC                        0x000aac64 /* Carrier Lock Detector Maximum Count Control */
#define BCHP_DS_3_EQ_CLDCI                       0x000aac68 /* Carrier Lock Detector Counter */
#define BCHP_DS_3_EQ_CWC_LEAK                    0x000aac6c /* CWC LEAK Control Register */
#define BCHP_DS_3_EQ_CWC_FIN1                    0x000aac70 /* CWC 1st tap non-baud-related frequency in Hz */
#define BCHP_DS_3_EQ_CWC_FIN2                    0x000aac74 /* CWC 2nd tap non-baud-related frequency in Hz */
#define BCHP_DS_3_EQ_CWC_FIN3                    0x000aac78 /* CWC 3rd tap non-baud-related frequency in Hz */
#define BCHP_DS_3_EQ_CWC_FIN4                    0x000aac7c /* CWC 4th tap non-baud-related frequency in Hz */
#define BCHP_DS_3_EQ_CWC_FOFS1                   0x000aac80 /* CWC 1st tap baud-related frequency offset control word */
#define BCHP_DS_3_EQ_CWC_FOFS2                   0x000aac84 /* CWC 2nd tap baud-related frequency offset control word */
#define BCHP_DS_3_EQ_CWC_FOFS3                   0x000aac88 /* CWC 3rd tap baud-related frequency offset control word */
#define BCHP_DS_3_EQ_CWC_FOFS4                   0x000aac8c /* CWC 4th tap baud-related frequency offset control word */
#define BCHP_DS_3_EQ_CWC_LFC1                    0x000aac90 /* CWC 1st tap phase/freq loop filter control */
#define BCHP_DS_3_EQ_CWC_LFC2                    0x000aac94 /* CWC 2nd tap phase/freq loop filter control */
#define BCHP_DS_3_EQ_CWC_LFC3                    0x000aac98 /* CWC 3rd tap phase/freq loop filter control */
#define BCHP_DS_3_EQ_CWC_LFC4                    0x000aac9c /* CWC 4th tap phase/freq loop filter control */
#define BCHP_DS_3_EQ_AGC                         0x000aaca4 /* Equalizer Hum-AGC Loop Control */
#define BCHP_DS_3_EQ_AGCC                        0x000aaca8 /* Equalizer Hum-AGC Loop Coefficients */
#define BCHP_DS_3_EQ_AGCI                        0x000aacac /* Equalizer Hum-AGC Loop Filter Integrator */
#define BCHP_DS_3_EQ_AGCPA                       0x000aacb0 /* Equalizer Hum-AGC Loop Gain Accumulator */
#define BCHP_DS_3_EQ_AGCFO                       0x000aacb4 /* Equalizer Hum-AGC Loop Filter Output */
#define BCHP_DS_3_EQ_FN                          0x000aacb8 /* Equalizer Hum-AGC FN Modulus Control Register */
#define BCHP_DS_3_EQ_POWER                       0x000aacbc /* Equalizer Input Power Estimator Register */
#define BCHP_DS_3_EQ_FFEU0                       0x000aad00 /* FFE Coefficient Register 0 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL0                       0x000aad04 /* FFE Coefficient Register 0 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU1                       0x000aad08 /* FFE Coefficient Register 1 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL1                       0x000aad0c /* FFE Coefficient Register 1 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU2                       0x000aad10 /* FFE Coefficient Register 2 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL2                       0x000aad14 /* FFE Coefficient Register 2 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU3                       0x000aad18 /* FFE Coefficient Register 3 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL3                       0x000aad1c /* FFE Coefficient Register 3 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU4                       0x000aad20 /* FFE Coefficient Register 4 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL4                       0x000aad24 /* FFE Coefficient Register 4 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU5                       0x000aad28 /* FFE Coefficient Register 5 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL5                       0x000aad2c /* FFE Coefficient Register 5 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU6                       0x000aad30 /* FFE Coefficient Register 6 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL6                       0x000aad34 /* FFE Coefficient Register 6 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU7                       0x000aad38 /* FFE Coefficient Register 7 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL7                       0x000aad3c /* FFE Coefficient Register 7 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU8                       0x000aad40 /* FFE Coefficient Register 8 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL8                       0x000aad44 /* FFE Coefficient Register 8 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU9                       0x000aad48 /* FFE Coefficient Register 9 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL9                       0x000aad4c /* FFE Coefficient Register 9 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU10                      0x000aad50 /* FFE Coefficient Register 10 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL10                      0x000aad54 /* FFE Coefficient Register 10 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU11                      0x000aad58 /* FFE Coefficient Register 11 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL11                      0x000aad5c /* FFE Coefficient Register 11 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU12                      0x000aad60 /* FFE Coefficient Register 12 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL12                      0x000aad64 /* FFE Coefficient Register 12 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU13                      0x000aad68 /* FFE Coefficient Register 13 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL13                      0x000aad6c /* FFE Coefficient Register 13 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU14                      0x000aad70 /* FFE Coefficient Register 14 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL14                      0x000aad74 /* FFE Coefficient Register 14 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU15                      0x000aad78 /* FFE Coefficient Register 15 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL15                      0x000aad7c /* FFE Coefficient Register 15 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU16                      0x000aad80 /* FFE Coefficient Register 16 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL16                      0x000aad84 /* FFE Coefficient Register 16 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU17                      0x000aad88 /* FFE Coefficient Register 17 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL17                      0x000aad8c /* FFE Coefficient Register 17 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU18                      0x000aad90 /* FFE Coefficient Register 18 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL18                      0x000aad94 /* FFE Coefficient Register 18 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU19                      0x000aad98 /* FFE Coefficient Register 19 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL19                      0x000aad9c /* FFE Coefficient Register 19 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU20                      0x000aada0 /* FFE Coefficient Register 20 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL20                      0x000aada4 /* FFE Coefficient Register 20 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU21                      0x000aada8 /* FFE Coefficient Register 21 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL21                      0x000aadac /* FFE Coefficient Register 21 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU22                      0x000aadb0 /* FFE Coefficient Register 22 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL22                      0x000aadb4 /* FFE Coefficient Register 22 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU23                      0x000aadb8 /* FFE Coefficient Register 23 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL23                      0x000aadbc /* FFE Coefficient Register 23 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU24                      0x000aadc0 /* FFE Coefficient Register 24 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL24                      0x000aadc4 /* FFE Coefficient Register 24 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU25                      0x000aadc8 /* FFE Coefficient Register 25 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL25                      0x000aadcc /* FFE Coefficient Register 25 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU26                      0x000aadd0 /* FFE Coefficient Register 26 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL26                      0x000aadd4 /* FFE Coefficient Register 26 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU27                      0x000aadd8 /* FFE Coefficient Register 27 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL27                      0x000aaddc /* FFE Coefficient Register 27 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU28                      0x000aade0 /* FFE Coefficient Register 28 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL28                      0x000aade4 /* FFE Coefficient Register 28 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU29                      0x000aade8 /* FFE Coefficient Register 29 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL29                      0x000aadec /* FFE Coefficient Register 29 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU30                      0x000aadf0 /* FFE Coefficient Register 30 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL30                      0x000aadf4 /* FFE Coefficient Register 30 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU31                      0x000aadf8 /* FFE Coefficient Register 31 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL31                      0x000aadfc /* FFE Coefficient Register 31 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU32                      0x000aae00 /* FFE Coefficient Register 32 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL32                      0x000aae04 /* FFE Coefficient Register 32 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU33                      0x000aae08 /* FFE Coefficient Register 33 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL33                      0x000aae0c /* FFE Coefficient Register 33 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU34                      0x000aae10 /* FFE Coefficient Register 34 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL34                      0x000aae14 /* FFE Coefficient Register 34 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_FFEU35                      0x000aae18 /* FFE Coefficient Register 35 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_FFEL35                      0x000aae1c /* FFE Coefficient Register 35 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU0                       0x000aae20 /* DFE Coefficient Register 0 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL0                       0x000aae24 /* DFE Coefficient Register 0 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU1                       0x000aae28 /* DFE Coefficient Register 1 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL1                       0x000aae2c /* DFE Coefficient Register 1 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU2                       0x000aae30 /* DFE Coefficient Register 2 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL2                       0x000aae34 /* DFE Coefficient Register 2 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU3                       0x000aae38 /* DFE Coefficient Register 3 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL3                       0x000aae3c /* DFE Coefficient Register 3 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU4                       0x000aae40 /* DFE Coefficient Register 4 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL4                       0x000aae44 /* DFE Coefficient Register 4 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU5                       0x000aae48 /* DFE Coefficient Register 5 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL5                       0x000aae4c /* DFE Coefficient Register 5 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU6                       0x000aae50 /* DFE Coefficient Register 6 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL6                       0x000aae54 /* DFE Coefficient Register 6 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU7                       0x000aae58 /* DFE Coefficient Register 7 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL7                       0x000aae5c /* DFE Coefficient Register 7 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU8                       0x000aae60 /* DFE Coefficient Register 8 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL8                       0x000aae64 /* DFE Coefficient Register 8 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU9                       0x000aae68 /* DFE Coefficient Register 9 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL9                       0x000aae6c /* DFE Coefficient Register 9 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU10                      0x000aae70 /* DFE Coefficient Register 10 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL10                      0x000aae74 /* DFE Coefficient Register 10 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU11                      0x000aae78 /* DFE Coefficient Register 11 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL11                      0x000aae7c /* DFE Coefficient Register 11 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU12                      0x000aae80 /* DFE Coefficient Register 12 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL12                      0x000aae84 /* DFE Coefficient Register 12 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU13                      0x000aae88 /* DFE Coefficient Register 13 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL13                      0x000aae8c /* DFE Coefficient Register 13 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU14                      0x000aae90 /* DFE Coefficient Register 14 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL14                      0x000aae94 /* DFE Coefficient Register 14 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU15                      0x000aae98 /* DFE Coefficient Register 15 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL15                      0x000aae9c /* DFE Coefficient Register 15 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU16                      0x000aaea0 /* DFE Coefficient Register 16 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL16                      0x000aaea4 /* DFE Coefficient Register 16 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU17                      0x000aaea8 /* DFE Coefficient Register 17 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL17                      0x000aaeac /* DFE Coefficient Register 17 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU18                      0x000aaeb0 /* DFE Coefficient Register 18 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL18                      0x000aaeb4 /* DFE Coefficient Register 18 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU19                      0x000aaeb8 /* DFE Coefficient Register 19 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL19                      0x000aaebc /* DFE Coefficient Register 19 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU20                      0x000aaec0 /* DFE Coefficient Register 20 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL20                      0x000aaec4 /* DFE Coefficient Register 20 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU21                      0x000aaec8 /* DFE Coefficient Register 21 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL21                      0x000aaecc /* DFE Coefficient Register 21 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU22                      0x000aaed0 /* DFE Coefficient Register 22 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL22                      0x000aaed4 /* DFE Coefficient Register 22 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU23                      0x000aaed8 /* DFE Coefficient Register 23 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL23                      0x000aaedc /* DFE Coefficient Register 23 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU24                      0x000aaee0 /* DFE Coefficient Register 24 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL24                      0x000aaee4 /* DFE Coefficient Register 24 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU25                      0x000aaee8 /* DFE Coefficient Register 25 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL25                      0x000aaeec /* DFE Coefficient Register 25 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU26                      0x000aaef0 /* DFE Coefficient Register 26 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL26                      0x000aaef4 /* DFE Coefficient Register 26 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU27                      0x000aaef8 /* DFE Coefficient Register 27 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL27                      0x000aaefc /* DFE Coefficient Register 27 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU28                      0x000aaf00 /* DFE Coefficient Register 28 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL28                      0x000aaf04 /* DFE Coefficient Register 28 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU29                      0x000aaf08 /* DFE Coefficient Register 29 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL29                      0x000aaf0c /* DFE Coefficient Register 29 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU30                      0x000aaf10 /* DFE Coefficient Register 30 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL30                      0x000aaf14 /* DFE Coefficient Register 30 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU31                      0x000aaf18 /* DFE Coefficient Register 31 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL31                      0x000aaf1c /* DFE Coefficient Register 31 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU32                      0x000aaf20 /* DFE Coefficient Register 32 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL32                      0x000aaf24 /* DFE Coefficient Register 32 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU33                      0x000aaf28 /* DFE Coefficient Register 33 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL33                      0x000aaf2c /* DFE Coefficient Register 33 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU34                      0x000aaf30 /* DFE Coefficient Register 34 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL34                      0x000aaf34 /* DFE Coefficient Register 34 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_DFEU35                      0x000aaf38 /* DFE Coefficient Register 35 (Upper 16 bits I/Q) */
#define BCHP_DS_3_EQ_DFEL35                      0x000aaf3c /* DFE Coefficient Register 35 (Lower 8 bits I/Q) */
#define BCHP_DS_3_EQ_CWC_INT1                    0x000ab060 /* CWC Tap 1 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_3_EQ_CWC_INT2                    0x000ab064 /* CWC Tap 2 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_3_EQ_CWC_INT3                    0x000ab068 /* CWC Tap 3 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_3_EQ_CWC_INT4                    0x000ab06c /* CWC Tap 4 Phase/Frequency Loop Integrator Register */
#define BCHP_DS_3_FFT_CTL                        0x000ab070 /* FFT Control Register */
#define BCHP_DS_3_FFT_PDETW                      0x000ab074 /* FFT Peak Detection Window register */
#define BCHP_DS_3_FFT_VAL                        0x000ab078 /* FFT Max Peak Value Register */
#define BCHP_DS_3_FFT_BIN                        0x000ab07c /* FFT BIN Register */
#define BCHP_DS_3_FFT_GAIN_STS                   0x000ab080 /* FFT GAIN Register */
#define BCHP_DS_3_FM_START_ADD                   0x000ab084 /* FFT Memory Start Address Register */
#define BCHP_DS_3_FM_CURR_ADD                    0x000ab088 /* FFT Memory current Address Register */
#define BCHP_DS_3_FM_RW_DATA                     0x000ab08c /* FFT Memory Read/Write Register */
#define BCHP_DS_3_EQ_MAIN                        0x000ab090 /* FFE MAIN Coefficient Register (Upper 16 bits I/Q) */

#endif /* #ifndef BCHP_DS_3_H__ */

/* End of File */
