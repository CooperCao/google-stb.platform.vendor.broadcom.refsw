# build steps for pvr utility application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the pvr app
cd BSEAV/lib/security/sage/utility/examples/pvr
make
