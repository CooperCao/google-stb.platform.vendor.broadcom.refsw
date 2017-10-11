# -*-tcl-*-

#######################################################################################################
#
# LitePoint Manufacturing test
#
#######################################################################################################
# Example of the server driver location:
# 	ROMTERM_REL_4_218_167\win_mfgtest_dongle_sdio\2009.11.8.0\release\BcmDHD\Bcm_Sdio_DriverOnly\4329B1
#######################################################################################################
# Steps to do ONCE for a NEW user:
#	1) Checkout the cvs UTF files:
#		cd ~
#		cvs co src/tools/unittest
#		cd src/tools/unittest
#	2) UTF authorize the machines:
#		UTF.tcl -utfconf utfconf/mfg_litepoint.tcl UTFMFG authorize
#		UTF.tcl -utfconf utfconf/mfg_litepoint.tcl DUTMFG authorize
#######################################################################################################
# Running the test from the src/tools/unittest:
# 	Test/mfg_litepoint.test -utfconf utfconf/mfg_litepoint.tcl -email "rpeters ynguyen bradleyd mirabadi"
#	Use "staloadonly" parameter to run the STA driver load only without running the LitePoint test
# 	Test/mfg_litepoint.test -utfconf utfconf/mfg_litepoint.tcl -staloadonly
#######################################################################################################
# Running the test from the cron job:
# 	$HOME/src/tools/unittest/utfconf/mfg_litepoint $HOME/src/tools/unittest/Test/mfg_litepoint.test -email hnd-utf-list
#######################################################################################################
# When putting a new HW in the system:
# 	Uninstall the Broadcom driver
# 	Install the Broadcom driver using the inf file (do not copy the files manually)
#		and make sure it gets enabled and then run the UTF
#######################################################################################################

# SummaryDir sets the location for test results
set ::UTF::SummaryDir "/projects/hnd_sig_ext11/CIT/MFG-LitePoint"

# Define power controllers on cart.
package require UTF::Power
UTF::Power::Synaccess 10.19.36.246

# Linux Endpoint - UTF machine bringup158
UTF::Linux UTFMFG -lan_ip 10.19.37.167 -sta {lan eth1}

# Dell D430 Laptop XP with 4329SDIO
UTF::WinDHD DUTMFG \
	-lan_ip 10.19.39.36 \
    -sta "mfg_sdio" \
    -power_sta "10.19.36.246 1"\
    -user user \
    -installer inf \
    -debuginf 1

package require UTF::HSIC

UTF::HSIC MFGHSIC\
	-sta {mfg_hsic eth1}\
	-lan_ip 192.168.1.1\
    -relay lan\
    -lanpeer lan\
    -console "bringup158:40000" \
    -tag "PHO2203RC1_REL_6_25_45"\
    -type "Innsbruck/43342a0/chardonnay.trx"\
	-nvram "Innsbruck/43342a0/chardonnay-u-kk.txt"\
    -brand "linux-external-dongle-usb"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_56"\
    -host_brand linux26-internal-hsic\
    -host_nvram {
		lan_ipaddr=192.168.1.1
		lan_mask=255.255.255.0
		wandevs=dummy
		clkfreq=530,176,88
		watchdog=6000
		console_loglevel=7
    }\
    -wlinitcmds {wl event_msgs 0x10000000000000}\
    -nocal 1

set ::lpdir "\"/cygdrive/c/Program Files/LitePoint/IQfact/IQfact fast Broadcom 432X v0.9.9.0/\""
set ::switchboard_location "/projects/hnd_dvt_bate/bate/src/tools/wlan/emb_dvt/equipment/"
set ::litepoint_location "/projects/hnd_dvt_bate/bate/src/tools/wlan/emb_dvt/litepoint/"
set ::litepoint_csv_results "/projects/hnd_dvt_bate/bate/src/tools/wlan/emb_dvt/litepoint/utf/LitePointTestResults_bringup176_1018Eng3.csv"
set ::switchboard_script_sec_timeout 30
set ::litepoint_script_sec_timeout 7000
set ::switchboard_script "control.py --sw0"

set ::load_failed_reboot_dut 1

#####################################################################################################
#
#	Which DUTs to run test on ( 0 = disabled  ,  1 = enabled )
#
#####################################################################################################

set ::litepoint0_test_enabled 0
set ::litepoint1_test_enabled 0
set ::litepoint2_test_enabled 0
set ::litepoint3_test_enabled 0
set ::litepoint4_test_enabled 1
set ::litepoint5_test_enabled 0

#####################################################################################################
#
#	Enter DUT object name for enabled DUTs
#
#####################################################################################################

set ::dut0_obj "4334hsic"
set ::dut1_obj "4334hsic_TOB"
set ::dut2_obj "43342hsic_TOB"
set ::dut3_obj "DUT_4330WLSDAGB_B2_FALCON_P300"
set ::dut4_obj "4324hsic_TOB"
set ::dut5_obj "DUT_4330FCBGA_B2_FALCON"

#####################################################################################################
#
#	Enter DUT serial number for enabled DUTs
#
#####################################################################################################

# For devices without the OTP programmed, must also use '-project=XXXX -chiprev=AB -dutclass=ZZZZZZ -boardtype=TTTTTT -boardsubtype=SSSSSS -boardrev=PPPP'
# To run a short test, add parameter '-o=./temp -f IQ_Flow_MH_MP_2G_TxFastOnly1Ch1Rt.txt'
# To run the actual test, add parameter '--drivertest'
# Typically use --ignoreversionmismatch since nightly builds have all kinds of different versionss....
# add paramter "--lpt2utf" to create the CSV results file
set ::litepoint0_script "litepoint_test.py -L bringup176_0990 -f IQ_Flow_MH_MP_2G_TxRx_PVT_NoInit.txt --SN=1026 --comment=DUT0 --project=BCM43362 --chiprev=a2 --boardtype=43362sdg --boardsubtype=  --boardrev=P201 --testtype=regression --ignoreversionmismatch=1 -i SDIO --lpt2utf"
set ::litepoint1_script "litepoint_test.py --Litepoint=bringup176_1018Eng3 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=894 --flow=IQlite_Flow_FullTemplate_reorder.py --comment=DUT1 --do_2g=1 --do_5g=1 --do_tx=1 --do_rx=1 --do_bw20in40=0 --do_bw20=1 --do_bw40=1 --coverage=pvt --project=BCM4334 --chiprev=B3 --boardtype=Centennial --boardsubtype=MURATA_MSE  --boardrev=ES6.9 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
set ::litepoint2_script "litepoint_test.py --Litepoint=bringup176_1018Eng3 --Host=10.19.39.36 --cal=DUT0.S2P:1 --SN=948 --flow=IQlite_Flow_FullTemplate_reorder.py --comment=DUT2 --do_2g=1 --do_5g=1 --do_tx=1 --do_rx=1 --do_bw20in40=0 --do_bw20=1 --do_bw40=1 --coverage=pvt --project=BCM43342 --chiprev=A1 --boardtype=Imperial --boardsubtype=MURATA_MT --boardrev=ES3.6 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
set ::litepoint3_script "litepoint_test.py --Litepoint=bringup176_1018Eng3 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=894 --flow=IQlite_Flow_FullTemplate_reorder.py --comment=DUT4 --do_2g=1 --do_5g=1 --do_tx=1 --do_rx=1 --do_bw20in40=0 --do_bw20=1 --do_bw40=1 --coverage=super_quick --project=BCM4334 --chiprev=B3 --boardtype=Centennial --boardsubtype=MURATA_MSE  --boardrev=ES6.9 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
#set ::litepoint4_script "litepoint_test.py --Litepoint=bringup176_4324_2_1_5 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=471 --flow=IQlite_Flow_NxN_2G5G_TxRx_reorder.py --comment=DUT4 --ff_control_string=2G_5G_RX_TX_ANT0_BW20_BW40 --coverage=pvt --project=BCM4324 --chiprev=B5 --boardtype=doppelbock --boardsubtype=USI_KK  --boardrev=ES3.3 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
set ::litepoint4_script "litepoint_test.py --Litepoint=bringup176_4324_2_1_5 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=471 --flow=IQlite_Flow_NxN_2G5G_TxRx_reorder.py --comment=DUT4 --ff_control_string=2G_5G_RX_TX_ANT1_BW20_BW40 --coverage=pvt --project=BCM4324 --dbuxproject=UTF_Nightly_Regression_4324Doppelbock --chiprev=B5 --boardtype=doppelbock --boardsubtype=USI_KK  --boardrev=ES3.3 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
#set ::litepoint4_script "litepoint_test.py --Litepoint=bringup176_4324_2_1_5 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=471 --flow=IQlite_Flow_NxN_2G5G_TxRx_reorder.py --comment=DUT4 --ff_control_string=2G_5G_TX_MIMO_BW20_BW40 --coverage=quick --project=BCM4324 --chiprev=B5 --boardtype=doppelbock --boardsubtype=USI_KK  --boardrev=ES3.3 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1 --lpt2utf"
#set ::litepoint4_script "litepoint_test.py --Litepoint=bringup176_4324_2_1_5 --Host=10.19.39.36 --cal=DUT0.S2P:0 --SN=471 --flow=IQlite_Flow_FullTemplate_reorder.py --comment= --do_2g=1 --do_5g=1 --do_tx=1 --do_rx=1 --do_bw20in40=0 --do_bw20=1 --do_bw40=1 --coverage=pvt --project=BCM4324 --chiprev=B5 --boardtype=doppelbock --boardsubtype=USI_KK  --boardrev=ES3.3 --testtype=regression --ignoreversionmismatch=1 --HSICrev=P220 -i hsic::192.168.1.1"
set ::litepoint5_script "litepoint_test.py -L bringup176_0990 --SN=937 --comment=DUT5 --project=BCM4330 --chiprev=b2 --boardtype=4330fcbga_details --boardsubtype=4330fcbga_details  --boardrev=P300 --testtype=regression --ignoreversionmismatch=1 -i SDIO --lpt2utf"

#####################################################################################################
#
#	DUT objects (must have unique name)
#
#####################################################################################################

#----------------------------------------------------------------------------------
# Active Objects
#----------------------------------------------------------------------------------

#
# UTF copies sdio-mfgtest-seqcmds.bin to \tmp\rtecdc.bin and 
#	then \SystemRoot\system32\drivers\rtecdc.bin therefore make sure
#	"-dongleimage" option is working.
#

mfg_hsic clone 43342hsic_old \
    -tag "PHO2203RC1_REL_6_25_35"\
    -brand "linux-external-dongle-usb"\
    -type "Innsbruck/43342a0/chardonnay.trx"\
	-nvram "Innsbruck/43342a0/chardonnay-u-kk.txt"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_56"

mfg_hsic clone 43342hsic_mfg \
    -tag "PHO2203RC1_REL_6_25_45"\
    -brand "linux-mfgtest-dongle-usb"\
    -type "43342a0-ram/usb-ag-mfgtest-nodis-swdiv-seqcmds.bin.trx"\
	-nvram "src/shared/nvram/bcm943342ChardonnayMurataMT.txt"\
    -customer "bcm"\
    -host_tag "RTRFALCON_REL_5_130_56"

mfg_hsic clone 43342hsic_ext \
    -tag "PHO2203RC1_REL_6_25_45"\
    -brand "linux-external-dongle-usb"\
    -type "Innsbruck/43342a0/chardonnay.trx"\
	-nvram "Innsbruck/43342a0/chardonnay-m-mt.txt"\
    -customer "olympic"\
    -host_tag "RTRFALCON_REL_5_130_56"

mfg_hsic clone 43342hsic_TOB \
    -tag "PHO2203RC1_BRANCH_6_25"\
    -brand "linux-mfgtest-dongle-usb"\
    -type "43342a0-ram/usb-ag-mfgtest-nodis-swdiv-seqcmds.bin.trx"\
	-nvram "src/shared/nvram/bcm943342ChardonnayMurataMT.txt"\
    -customer "bcm"\
    -host_tag "RTRFALCON_REL_5_130_56"
	
mfg_hsic clone 4334hsic_TOB \
    -tag "PHO2203RC1_BRANCH_6_25"\
    -brand "linux-mfgtest-dongle-usb"\
    -type "4334b0-ram/usb-ag-mfgtest-nodis-swdiv-seqcmds.bin.trx"\
	-nvram "src/shared/nvram/bcm94334CentMurataMSE.txt"\
    -customer "bcm"\
    -host_tag "RTRFALCON_REL_5_130_56"
	
mfg_hsic clone 4324hsic_TOB \
    -tag "PHO2203RC1_BRANCH_6_25"\
    -brand "linux-mfgtest-dongle-usb"\
    -type "4324b5-roml/usb-ag-mfgtest-seqcmds-sr-nodis.bin.trx"\
	-nvram "src/shared/nvram/bcm94324SyrahUsiKK.txt"\
    -customer "bcm"\
    -host_tag "RTRFALCON_REL_5_130_56"	
	
mfg_sdio clone DUT_4330B2UNO3_SEMCO_FALCON \
    -tag FALCONPHY_BRANCH_5_92 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330B2/g \
    -nvram bcm94330OlympicUNO3_SEMCO.txt

mfg_sdio clone DUT_4330WLSDAGB_B2_FALCON_P300 \
    -tag FALCONPHY_BRANCH_5_92 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330B2/ag \
    -nvram bcm94330wlsdagb_P300.txt

mfg_sdio clone DUT_4330WLSDAGB_B2_FALCON_P300 \
    -tag FALCONPHY_BRANCH_5_92 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330B2/ag \
    -nvram bcm94330wlsdagb_P300.txt

mfg_sdio clone DUT_4330FCBGA_B2_FALCON  \
    -tag FALCONPHY_BRANCH_5_92 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330B2/ag \
    -nvram bcm94330fcbga_McLarenBypass.txt

#######################################################################################################

#----------------------------------------------------------------------------------
# Non-active Objects
#----------------------------------------------------------------------------------

mfg_sdio clone DUT_N90U_RT2TWIG46 \
    -tag RT2TWIG46_BRANCH_4_221 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4329B1 \
    -altsys "bcmsddhd-mfgseqcmds.sys" \
    -nvram "bcm94329OLYMPICN90U.txt" 
#     -dongleimage 4329b1-sdio-g-cdc-11n-mfgtest-dhdoid-roml-seqcmds \

mfg_sdio clone DUT_4330A0OlympicAMGiPA_F16 \
    -tag F16_BRANCH_5_80 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330A0 \
    -nvram bcm94330OlympicAMG.txt
#    -dongleimage ../../Bcm_Firmware/4330a0-roml/sdio-g-mfgtest-seqcmds \

mfg_sdio clone DUT_4330A0OlympicAMGePA_F16 \
    -tag F16_BRANCH_5_80 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4330A0 \
    -nvram bcm94330OlympicAMGepa.txt
#     -dongleimage ../../Bcm_Firmware/4330a0-roml/sdio-g-mfgtest-seqcmds  \

mfg_sdio clone DUT_UNO_C0_RT2TWIG46 \
    -tag RT2TWIG46_BRANCH_4_221 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4329C0 \
    -altsys "bcmsddhd-mfgseqcmds.sys" \
    -nvram "bcm94329OLYMPICUNO.txt" 
#     -dongleimage ../../Bcm_Firmware/4330a0-roml/sdio-g-mfgtest-seqcmds  \

mfg_sdio clone DUT_X17UC_B1_RT2TWIG46 \
    -tag RT2TWIG46_BRANCH_4_221 \
    -brand win_mfgtest_dongle_sdio \
    -type BcmDHD/Bcm_Sdio_DriverOnly/4329B1 \
    -altsys "bcmsddhd-mfgseqcmds.sys" \
    -nvram "bcm94329OLYMPICX17UC.txt" 
#     -dongleimage ../../Bcm_Firmware/4330a0-roml/sdio-g-mfgtest-seqcmds  \

##########################################################################################
# Cron job test locking queue
# To check the queue: bin/utfqstat bringup158
##########################################################################################
UTF::Q bringup158
