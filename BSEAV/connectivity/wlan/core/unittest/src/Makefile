# Makefile for UTF compiled tools
#	$Id: 70f7612ac43b4c4dda744c18b001517ddcebef46 $	


CFLAGS=-Wall

all: ush ushd_northstar ushd_49xx ushd_dsl ushd_mips26 ushd_android

.PHONY : clean cstyle

cstyle: ush.c ushd.c
	cstyle $?


# USH client

ush: ush.c
	gcc ${CFLAGS} -o $@ $<


# USH servers

# (for NETBSD see netbsd subdir)

ushd_mips24: ushd.c
	/projects/hnd/tools/linux/hndtools-mipsel-uclibc-3.2.3/bin/mipsel-uclibc-gcc \
		${CFLAGS} -o $@ $<

ushd_mips26: ushd.c
	/projects/hnd/tools/linux/hndtools-mipsel-linux-uclibc-4.2.3/bin/mipsel-linux-uclibc-gcc \
		${CFLAGS} -o $@ $<

ushd_northstar: ushd.c
	/projects/hnd/tools/linux/hndtools-arm-linux-2.6.36-uclibc-4.5.3/bin/arm-brcm-linux-uclibcgnueabi-gcc \
		${CFLAGS} -o $@ $<

ushd_49xx: ushd.c
	tc=/projects/hnd/tools/linux/BCG/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25; \
	LD_LIBRARY_PATH=$${tc}/usr/lib $${tc}/usr/bin/arm-buildroot-linux-gnueabi-gcc \
		${CFLAGS} -o $@ $<

ushd_dsl: ushd.c
	tc=/projects/hnd/tools/linux/BCG/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL; \
	LD_LIBRARY_PATH=$${tc}/usr/lib $${tc}/usr/bin/arm-unknown-linux-uclibcgnueabi-gcc \
		${CFLAGS} -o $@ $<

ushd_stb: ushd.c
	 /projects/hnd/tools/media/stbgcc-4.5.3-2.1/bin/mipsel-linux-gcc \
		${CFLAGS} -o $@ $<	

ushd_astb: ushd.c
	 /projects/hnd/tools/linux/BCG/stbgcc-4.8-0.5/bin/arm-linux-gcc \
		${CFLAGS} -o $@ $<	

ushd_stbap: ushd.c
	 /projects/hnd/tools/linux/BCG/stbgcc-4.8-1.2/bin/arm-linux-gcc \
		${CFLAGS} -o $@ $<	

# For debugging on Android split the compile and link and only put -g
# on the compile stage
ushd_android: ushd.c
	 /projects/hnd/tools/linux/hndtools-arm-eabi-4.2.1/bin/arm-eabi-gcc -c -I/projects/hnd/tools/linux/hndtools-arm-eabi-4.2.1/android-ndk-r3/build/platforms/android-3/arch-arm/usr/include \
		${CFLAGS} -o $@ $<

ushd_pxa: ushd.c
	/projects/hnd/tools/linux/hndtools-arm-linux-3.4.3/bin/arm-linux-gcc \
		${CFLAGS} -o $@ $<


# clean up
clean:
	$(RM) ush ushd_mips24 ushd_mips26 ushd_android ushd_pxa *.o
