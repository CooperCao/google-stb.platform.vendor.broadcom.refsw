override PROD_TAG = $Name: not supported by cvs2svn $


LOCAL_SOURCES := src/include
SOURCES := src/include/crypto src/include/ctf src/include/dsp src/include/dsp_lib src/include/efi \
	src/include/emf src/include/ndd src/include/prerelease src/common/include/proto src/include/rts

ifeq ("$(MK_TYPE)", "SRC")
MOD_VERSION = wl_version
endif
ifeq ("$(MK_TYPE)", "REL_SRC")
MOD_VERSION = wl_version
endif

include src/tools/release/components/makerules.mk

ifeq ("$(MOD_VERSION)", "wl_version")
wl_version:
	make TAG=$(TAG_NAME) -C src/include
endif
