# build steps for rsa utility application

# set SAGE support (if not already set by default in platform settings)
export SAGE_SUPPORT=y

#build nexus
cd nexus/build
make

#build the rsa app
cd BSEAV/lib/security/sage/utility/examples/rsa
make
