#!/bin/bash

set -u

if [ $# != 1 ]; then
	echo "Usage: $0 <SDK_TOPDIR>"
	echo ""
	echo "Example:"
	echo "$1 your/path/to/spresense"
	echo ""
	exit 1
fi

echo "Export SDK from build."
echo "SDK_VERSION=${SDK_VERSION}"
echo "VARIANT_NAME=${VARIANT_NAME}"
NUTTX_EXPORT="sdk-export"
PACKAGE_NAME="${NUTTX_EXPORT}.zip"

SDK_DIR=`cd $1 && pwd`
TMP_DIR=`mktemp -d`

# versioning
cd $SDK_DIR
TOPDIR=$SDK_DIR/nuttx bash sdk/tools/mkversion.sh

# create sdk-export.zip
cd $SDK_DIR/sdk

if [ ! ${IMPORT_ONLY} ]; then
	## clean
	echo "Clean SDK objects..."
	make distcleankernel &>/dev/null
	make distclean &>/dev/null

	## Configuration
	echo "Configure SDK components..."
	echo "Kernel : ${SDK_KERNEL_CONF}"
	echo "SDK    : ${SDK_CONFIG}"
	./tools/config.py --kernel ${SDK_KERNEL_CONF}
	./tools/config.py ${SDK_CONFIG}
fi

## Build kernel
echo "Build kernel..."
make buildkernel &>/dev/null

echo "Export to Arduino..."
make export >/dev/null
if [ $? != 0 ]; then
	echo "make export failed"
fi

# work directory
unzip $PACKAGE_NAME -d $TMP_DIR >/dev/null
rm $PACKAGE_NAME

# create sdk directory
mkdir -p $TMP_DIR/sdk/${SDK_VERSION}/${VARIANT_NAME}

# create arch, include, startup
mv $TMP_DIR/${NUTTX_EXPORT}/nuttx $TMP_DIR/sdk/${SDK_VERSION}/${VARIANT_NAME}/${SDK_KERNEL_CONF}
cd $TMP_DIR/sdk/${SDK_VERSION}/${VARIANT_NAME}/${SDK_KERNEL_CONF}
rm -f .config

mkdir -p ./include/apps
mv $TMP_DIR/${NUTTX_EXPORT}/sdk/modules/include/* ./include/apps
mv $TMP_DIR/${NUTTX_EXPORT}/sdk/bsp/include/sdk ./include

mkdir -p ./libs
mv $TMP_DIR/${NUTTX_EXPORT}/sdk/libs/* ./libs
mv ./libs/libsdk.a ./libs/libnuttx.a

# create debug, release
mkdir -p prebuilt
cp -a build libs prebuilt
rm -rf build libs

cp ${SDK_DIR}/LICENSE ./

cd $TMP_DIR
find . -name .gitignore | xargs rm &>/dev/null
find . -name .fakelnk | xargs rm &>/dev/null
cd sdk
zip -r sdk-export.zip ${SDK_VERSION}/${VARIANT_NAME} >/dev/null
mv sdk-export.zip $SDK_DIR
rm -rf $TMP_DIR

PACKAGE_NAME=`readlink -f $SDK_DIR/sdk-export.zip`
echo "SDK exported to $PACKAGE_NAME"
