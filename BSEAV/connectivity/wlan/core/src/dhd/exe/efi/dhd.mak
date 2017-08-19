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

BASE_NAME         = dhd
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
DHDSOURCE_DIR = ..\..\..\

#
# Include paths
#

!include $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)\include\$(EFI_INC_DIR)\$(PROCESSOR) $(INC)

INC = -I $(DHDSOURCE_DIR)\include\efi $(INC)

!include $(SDK_INSTALL_DIR)\include\bsd\makefile.hdr
INC = -I $(SDK_INSTALL_DIR)\include\bsd $(INC)

INC = -I $(DHDSOURCE_DIR)\include $(INC)
INC = -I $(DHDSOURCE_DIR)\..\components\shared $(INC)
INC = -I $(DHDSOURCE_DIR)\..\components\shared\devctrl_if $(INC)
INC = -I $(DHDSOURCE_DIR)\dhd\exe $(INC)
INC = -I $(DHDSOURCE_DIR)\wl\exe $(INC)
INC = -I $(DHDSOURCE_DIR)\shared\bcmwifi\include $(INC)
INC = -I $(DHDSOURCE_DIR)\wl\ppr\include $(INC)

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
    $(BUILD_DIR)\dhdu.obj \
    $(BUILD_DIR)\dhdu_efi.obj \
    $(BUILD_DIR)\bcmutils.obj \
    $(BUILD_DIR)\bcmxtlv.obj \
    $(BUILD_DIR)\bcmwifi_channels.obj \
    $(BUILD_DIR)\miniopt.obj \
    $(BUILD_DIR)\bcm_app_utils.obj \
    $(BUILD_DIR)\ucode_download.obj \
    $(BUILD_DIR)\wlu_common.obj \

#   $(BUILD_DIR)\wluc_nan.obj

#
# Source file dependencies
#

$(BUILD_DIR)\dhdu.obj: $(DHDSOURCE_DIR)\dhd\exe\dhdu.c $(INC_DEPS)
$(BUILD_DIR)\dhdu_efi.obj: $(DHDSOURCE_DIR)\dhd\exe\dhdu_efi.c $(INC_DEPS)
$(BUILD_DIR)\bcmutils.obj: $(DHDSOURCE_DIR)\shared\bcmutils.c $(INC_DEPS)
$(BUILD_DIR)\bcmxtlv.obj: $(DHDSOURCE_DIR)\shared\bcmxtlv.c $(INC_DEPS)
$(BUILD_DIR)\bcmwifi_channels.obj: $(DHDSOURCE_DIR)\shared\bcmwifi\src\bcmwifi_channels.c $(INC_DEPS)
$(BUILD_DIR)\miniopt.obj: $(DHDSOURCE_DIR)\shared\miniopt.c $(INC_DEPS)
$(BUILD_DIR)\bcm_app_utils.obj: $(DHDSOURCE_DIR)\shared\bcm_app_utils.c $(INC_DEPS)
$(BUILD_DIR)\ucode_download.obj: $(DHDSOURCE_DIR)\dhd\exe\ucode_download.c $(INC_DEPS)
$(BUILD_DIR)\wlu_common.obj: $(DHDSOURCE_DIR)\wl\exe\wlu_common.c $(INC_DEPS)


# Additional implicit rule for src\shared.
{$(DHDSOURCE_DIR)\shared}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

# Additional implicit rule for bcmwifi.
{$(DHDSOURCE_DIR)\shared\bcmwifi\src}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

{$(DHDSOURCE_DIR)\dhd\exe}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)


{$(DHDSOURCE_DIR)\wl\exe}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

{$(DHDSOURCE_DIR)\dhd\ppr\src}.c{$(BUILD_DIR)}.obj:
	$(CC_LINE)

MODULE_CFLAGS = /D EFI /D SROM12 /D OLYMPIC_API /D EFI_WINBLD


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
