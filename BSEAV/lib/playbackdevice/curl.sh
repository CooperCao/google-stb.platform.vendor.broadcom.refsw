# Build the Net libraries (curl, expat, openssl)

echo	"----"
echo "Building the curl library"
echo


BASE_DIR=${NEXUS}/..
BIN_DIR=${NEXUS}/../BSEAV/lib/playbackdevice/bin

# set curl library version
if [  -z $CURL_VER ] ; then
    CURL_VER=curl-7.21.2

    echo 
    echo "1. Use default CURL_VER = "${CURL_VER}
    echo
else
    echo 
    echo "2. CURL_VER = "${CURL_VER}
    echo
#    exit 0 # Exit with non-zero return code
fi

pushd $BASE_DIR/AppLibs/opensource/libcurl > /dev/null

if [ ! -f ${cln_net_f} ]
then
	
        echo "----"
        echo "Starting \"make clean\" for curl library"
        echo

        if ! make clean; then err_exit "curl library \"make clean\" failed"; fi
        echo "curl library build cleaned on "`date` > ${cln_net_f}
fi

if ! make; then err_exit "curl library \"make\" failed"; fi

echo	"----"
echo "culr library build done"
echo

popd > /dev/null

#
# Copy curl libraries
#

echo	 "----"
echo	 "copying curl libraries"
echo	

pushd $BASE_DIR/AppLibs/opensource/libcurl/${CURL_VER}/lib/.libs > /dev/null

cp -fv libcurl.so* $BIN_DIR

popd > /dev/null
