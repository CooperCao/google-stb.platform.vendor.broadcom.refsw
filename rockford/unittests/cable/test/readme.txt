All the Unit Test codes under this directory are built with NEXUS API directly.
To build:
	cp foo.c nexus/examples
	cd nexus/examples
	export BCHP_VER=<>
	export LINUX=<>
	export PLATFORM=<>
	make foo
To Run:
  cd nexus/bin
  ./nexus foo


For irbtest_nexus, Please use the Makefile under test directory only. The export
variables remain same. In addition, use
export XMP2_SUPPORT=y
The resulting binary will be found in 97456_linux under test
