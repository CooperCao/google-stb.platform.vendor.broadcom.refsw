#
# <<Broadcom-WL-IPTag/Proprietary:>>
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

!include $(SDK_INSTALL_DIR)\build\$(SDK_BUILD_ENV)\sdk.env

# Apparently someone modified the (unversioned) sdk.env file above
# within the dev kit with a hardwired reference to Z: which overrides
# our attempt to prefer a local copy. So we re-override here :-(.
MSSdk = $(WIN_DDK_PATH)\bin\win64\x86\amd64

#
# Set the base output name and entry point
#

BASE_NAME         = wl
!IFDEF OLD_SHELL
IMAGE_ENTRY_POINT = _LIBC_Start_Shellapp_A
!ELSE
IMAGE_ENTRY_POINT = _LIBC_Start_A
!ENDIF
#IMAGE_ENTRY_POINT = InitializeTest2Application

#
# Globals needed by master.mak
#

TARGET_APP = $(BASE_NAME)
SOURCE_DIR =
BUILD_DIR  = obj\$(SDK_BUILD_ENV)
WLSOURCE_DIR = ..\..\..\

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

INC = -I $(WLSOURCE_DIR)\include\efi $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

INC = -I $(WLSOURCE_DIR)\include $(INC)
INC = -I $(WLSOURCE_DIR)\..\components\shared $(INC)
INC = -I $(WLSOURCE_DIR)\..\components\shared\devctrl_if $(INC)
INC = -I $(WLSOURCE_DIR)\wl\exe $(INC)
INC = -I $(WLSOURCE_DIR)\shared\bcmwifi\include $(INC)
INC = -I $(WLSOURCE_DIR)\wl\ppr\include $(INC)

#
# Libraries
#

#
# Libraries
#

LIBS = $(LIBS) \
       $(SDK_BUILD_DIR)\lib\libc\libc.lib \
       $(SDK_BUILD_DIR)\lib\libefi\libefi.lib \
       $(SDK_BUILD_DIR)\lib\libefishell\libefishell.lib

#
# Default target
#

all : dirs $(LIBS) $(OBJECTS)

#
# Program object files
#

OBJECTS = $(OBJECTS) \
    $(BUILD_DIR)\wlu_efi.obj \
    $(BUILD_DIR)\wlu.obj \
    $(BUILD_DIR)\wluc_phy.obj \
    $(BUILD_DIR)\wluc_wnm.obj \
    $(BUILD_DIR)\wluc_cac.obj \
    $(BUILD_DIR)\wluc_relmcast.obj \
    $(BUILD_DIR)\wluc_rrm.obj \
    $(BUILD_DIR)\wluc_wowl.obj \
    $(BUILD_DIR)\wluc_pkt_filter.obj \
    $(BUILD_DIR)\wluc_mfp.obj \
    $(BUILD_DIR)\wluc_ota_test.obj \
    $(BUILD_DIR)\wluc_bssload.obj \
    $(BUILD_DIR)\wluc_stf.obj \
    $(BUILD_DIR)\wluc_offloads.obj \
    $(BUILD_DIR)\wluc_tpc.obj \
    $(BUILD_DIR)\wluc_toe.obj \
    $(BUILD_DIR)\wluc_arpoe.obj \
    $(BUILD_DIR)\wluc_keep_alive.obj \
    $(BUILD_DIR)\wluc_ap.obj \
    $(BUILD_DIR)\wluc_ampdu.obj \
    $(BUILD_DIR)\wluc_ampdu_cmn.obj \
    $(BUILD_DIR)\wluc_bmac.obj \
    $(BUILD_DIR)\wluc_ht.obj \
    $(BUILD_DIR)\wluc_wds.obj \
    $(BUILD_DIR)\wluc_keymgmt.obj \
    $(BUILD_DIR)\wluc_scan.obj \
    $(BUILD_DIR)\wluc_obss.obj \
    $(BUILD_DIR)\wluc_prot_obss.obj \
    $(BUILD_DIR)\wluc_lq.obj \
    $(BUILD_DIR)\wluc_seq_cmds.obj \
    $(BUILD_DIR)\wluc_btcx.obj \
    $(BUILD_DIR)\wluc_led.obj \
    $(BUILD_DIR)\wluc_interfere.obj \
    $(BUILD_DIR)\wluc_ltecx.obj \
    $(BUILD_DIR)\wluc_extlog.obj \
    $(BUILD_DIR)\wluc_sdio.obj \
    $(BUILD_DIR)\wluc_ndoe.obj \
    $(BUILD_DIR)\wluc_p2po.obj \
    $(BUILD_DIR)\wluc_anqpo.obj \
    $(BUILD_DIR)\wluc_pfn.obj \
    $(BUILD_DIR)\wluc_p2p.obj \
    $(BUILD_DIR)\wluc_tdls.obj \
    $(BUILD_DIR)\wluc_traffic_mgmt.obj \
    $(BUILD_DIR)\wluc_proxd.obj \
    $(BUILD_DIR)\wluc_he.obj \
    $(BUILD_DIR)\wlu_rates_matrix.obj \
    $(BUILD_DIR)\wlc_ppr.obj \
    $(BUILD_DIR)\wlu_common.obj \
    $(BUILD_DIR)\bcmwifi_channels.obj \
    $(BUILD_DIR)\bcmutils.obj \
    $(BUILD_DIR)\wlu_cmd.obj \
    $(BUILD_DIR)\wlu_iov.obj \
    $(BUILD_DIR)\miniopt.obj \
    $(BUILD_DIR)\bcmxtlv.obj \
    $(BUILD_DIR)\bcm_app_utils.obj \
    $(BUILD_DIR)\wluc_bdo.obj \
    $(BUILD_DIR)\wlu_subcounters.obj
#   $(BUILD_DIR)\wluc_nan.obj

#
# Source file dependencies
#

$(BUILD_DIR)\$(BASE_NAME).obj : $(WLSOURCE_DIR)\wl\exe\wlu_efi.c $(INC_DEPS)
$(BUILD_DIR)\wlu.obj: $(WLSOURCE_DIR)\wl\exe\wlu.c $(INC_DEPS)
$(BUILD_DIR)\wluc_phy.obj: $(WLSOURCE_DIR)\wl\exe\wluc_phy.c $(INC_DEPS)
$(BUILD_DIR)\wluc_wnm.obj: $(WLSOURCE_DIR)\wl\exe\wluc_wnm.c $(INC_DEPS)
$(BUILD_DIR)\wluc_cac.obj: $(WLSOURCE_DIR)\wl\exe\wluc_cac.c $(INC_DEPS)
$(BUILD_DIR)\wluc_relmcast.obj: $(WLSOURCE_DIR)\wl\exe\wluc_relmcast.c $(INC_DEPS)
$(BUILD_DIR)\wluc_rrm.obj: $(WLSOURCE_DIR)\wl\exe\wluc_rrm.c $(INC_DEPS)
$(BUILD_DIR)\wluc_wowl.obj: $(WLSOURCE_DIR)\wl\exe\wluc_wowl.c $(INC_DEPS)
$(BUILD_DIR)\wluc_pkt_filter.obj: $(WLSOURCE_DIR)\wl\exe\wluc_pkt_filter.c $(INC_DEPS)
$(BUILD_DIR)\wluc_mfp.obj: $(WLSOURCE_DIR)\wl\exe\wluc_mfp.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ota_test.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ota_test.c $(INC_DEPS)
$(BUILD_DIR)\wluc_bssload.obj: $(WLSOURCE_DIR)\wl\exe\wluc_bssload.c $(INC_DEPS)
$(BUILD_DIR)\wluc_stf.obj: $(WLSOURCE_DIR)\wl\exe\wluc_stf.c $(INC_DEPS)
$(BUILD_DIR)\wluc_offloads.obj: $(WLSOURCE_DIR)\wl\exe\wluc_offloads.c $(INC_DEPS)
$(BUILD_DIR)\wluc_tpc.obj: $(WLSOURCE_DIR)\wl\exe\wluc_tpc.c $(INC_DEPS)
$(BUILD_DIR)\wluc_toe.obj: $(WLSOURCE_DIR)\wl\exe\wluc_toe.c $(INC_DEPS)
$(BUILD_DIR)\wluc_arpoe.obj: $(WLSOURCE_DIR)\wl\exe\wluc_arpoe.c $(INC_DEPS)
$(BUILD_DIR)\wluc_keep_alive.obj: $(WLSOURCE_DIR)\wl\exe\wluc_keep_alive.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ap.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ap.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ampdu.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ampdu.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ampdu_cmn.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ampdu_cmn.c $(INC_DEPS)
$(BUILD_DIR)\wluc_bmac.obj: $(WLSOURCE_DIR)\wl\exe\wluc_bmac.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ht.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ht.c $(INC_DEPS)
$(BUILD_DIR)\wluc_wds.obj: $(WLSOURCE_DIR)\wl\exe\wluc_wds.c $(INC_DEPS)
$(BUILD_DIR)\wluc_keymgmt.obj: $(WLSOURCE_DIR)\wl\exe\wluc_keymgmt.c $(INC_DEPS)
$(BUILD_DIR)\wluc_scan.obj: $(WLSOURCE_DIR)\wl\exe\wluc_scan.c $(INC_DEPS)
$(BUILD_DIR)\wluc_obss.obj: $(WLSOURCE_DIR)\wl\exe\wluc_obss.c $(INC_DEPS)
$(BUILD_DIR)\wluc_prot_obss.obj: $(WLSOURCE_DIR)\wl\exe\wluc_prot_obss.c $(INC_DEPS)
$(BUILD_DIR)\wluc_lq.obj: $(WLSOURCE_DIR)\wl\exe\wluc_lq.c $(INC_DEPS)
$(BUILD_DIR)\wluc_seq_cmds.obj: $(WLSOURCE_DIR)\wl\exe\wluc_seq_cmds.c $(INC_DEPS)
$(BUILD_DIR)\wluc_btcx.obj: $(WLSOURCE_DIR)\wl\exe\wluc_btcx.c $(INC_DEPS)
$(BUILD_DIR)\wluc_led.obj: $(WLSOURCE_DIR)\wl\exe\wluc_led.c $(INC_DEPS)
$(BUILD_DIR)\wluc_interfere.obj: $(WLSOURCE_DIR)\wl\exe\wluc_interfere.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ltecx.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ltecx.c $(INC_DEPS)
#$(BUILD_DIR)\wluc_nan.obj: $(WLSOURCE_DIR)\wl\exe\wluc_nan.c $(INC_DEPS)
$(BUILD_DIR)\wluc_extlog.obj: $(WLSOURCE_DIR)\wl\exe\wluc_extlog.c $(INC_DEPS)
$(BUILD_DIR)\wluc_sdio.obj: $(WLSOURCE_DIR)\wl\exe\wluc_sdio.c $(INC_DEPS)
$(BUILD_DIR)\wluc_ndoe.obj: $(WLSOURCE_DIR)\wl\exe\wluc_ndoe.c $(INC_DEPS)
$(BUILD_DIR)\wluc_p2po.obj: $(WLSOURCE_DIR)\wl\exe\wluc_p2po.c $(INC_DEPS)
$(BUILD_DIR)\wluc_anqpo.obj: $(WLSOURCE_DIR)\wl\exe\wluc_anqpo.c $(INC_DEPS)
$(BUILD_DIR)\wluc_pfn.obj: $(WLSOURCE_DIR)\wl\exe\wluc_pfn.c $(INC_DEPS)
$(BUILD_DIR)\wluc_p2p.obj: $(WLSOURCE_DIR)\wl\exe\wluc_p2p.c $(INC_DEPS)
$(BUILD_DIR)\wluc_tdls.obj: $(WLSOURCE_DIR)\wl\exe\wluc_tdls.c $(INC_DEPS)
$(BUILD_DIR)\wluc_traffic_mgmt.obj: $(WLSOURCE_DIR)\wl\exe\wluc_traffic_mgmt.c $(INC_DEPS)
$(BUILD_DIR)\wluc_proxd.obj: $(WLSOURCE_DIR)\wl\exe\wluc_proxd.c $(INC_DEPS)
$(BUILD_DIR)\wlu_common.obj: $(WLSOURCE_DIR)\wl\exe\wlu_common.c $(INC_DEPS)
$(BUILD_DIR)\wlu_cmd.obj: $(WLSOURCE_DIR)\wl\exe\wlu_cmd.c $(INC_DEPS)
$(BUILD_DIR)\wlu_rates_matrix.obj: $(WLSOURCE_DIR)\wl\exe\wlu_rates_matrix.c $(INC_DEPS)
$(BUILD_DIR)\wlu_iov.obj: $(WLSOURCE_DIR)\wl\exe\wlu_iov.c $(INC_DEPS)
$(BUILD_DIR)\bcmwifi_channels.obj: $(WLSOURCE_DIR)\shared\bcmwifi\src\bcmwifi_channels.c $(INC_DEPS)
$(BUILD_DIR)\bcmutils.obj: $(WLSOURCE_DIR)\shared\bcmutils.c $(INC_DEPS)
$(BUILD_DIR)\bcmxtlv.obj: $(WLSOURCE_DIR)\shared\bcmxtlv.c $(INC_DEPS)
$(BUILD_DIR)\wlc_ppr.obj: $(WLSOURCE_DIR)\wl\ppr\src\wlc_ppr.c $(INC_DEPS)
$(BUILD_DIR)\wluc_bdo.obj: $(WLSOURCE_DIR)\wl\exe\wluc_bdo.c $(INC_DEPS)
$(BUILD_DIR)\wluc_he.obj: $(WLSOURCE_DIR)\wl\exe\wluc_he.c $(INC_DEPS)
$(BUILD_DIR)\wlu_subcounters.obj: $(WLSOURCE_DIR)\wl\exe\wlu_subcounters.c $(INC_DEPS)

# Additional implicit rule for src\shared.
{$(WLSOURCE_DIR)\shared}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

# Additional implicit rule for bcmwifi.
{$(WLSOURCE_DIR)\shared\bcmwifi\src}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

{$(WLSOURCE_DIR)\wl\exe}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

{$(WLSOURCE_DIR)\wl\ppr\src}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

MODULE_CFLAGS = /D EFI /D EFI_WINBLD /D SROM12

      
#ifdef WLCNT
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLCNT
#endif

# Use newer 11ac ratespec for wl command line
MODULE_CFLAGS = $(MODULE_CFLAGS) /D D11AC_IOTYPES

# Use ppr opaque stucture for wl command line
MODULE_CFLAGS = $(MODULE_CFLAGS) /D PPR_API

##ifdef WL_NAN
#MODULE_CFLAGS = $(MODULE_CFLAGS) /D WL_NAN
##endif

#ifdef WLEXTLOG
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLEXTLOG
#endif


#ifdef WLNDOE
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLNDOE
#endif

#ifdef WLP2PO
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLP2PO
#endif

#ifdef WLANQPO
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLANQPO
#endif

#ifdef WLPFN
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLPFN
#endif

#ifdef WLP2P
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLP2P
#endif

#ifdef WLTDLS
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WLTDLS
#endif

#ifdef TRAFFIC_MGMT
MODULE_CFLAGS = $(MODULE_CFLAGS) /D TRAFFIC_MGMT
#endif

#ifdef WL_PROXDETECT
MODULE_CFLAGS = $(MODULE_CFLAGS) /D WL_PROXDETECT
#endif

#
# Handoff to master.mak
#
BIN_DIR = $(BUILD_DIR)\bin
!include $(SDK_INSTALL_DIR)\build\master.mak
