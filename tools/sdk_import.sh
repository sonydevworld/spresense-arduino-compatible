#!/bin/bash
if [ $# != 1 ]; then
	echo "Usage: $0 <SDK_Package_file>"
	exit 1
fi

(echo $1 | egrep -q "zip$|ZIP$")
if [ $? != 0 -o ! -f $1 ]; then 
	echo "Please input a ZIP file as SDK package"
	exit 1
fi

T=`readlink -f $0`
SCRIPT_DIR=`dirname $T`
SDK_DIR="${SCRIPT_DIR}/../Arduino15/packages/SPRESENSE/tools/spresense-sdk"
SDK_DIR=`readlink -f $SDK_DIR`

echo "Local SDK import to $SDK_DIR"

rm -rf $SDK_DIR/${SDK_VERSION}/${VARIANT_NAME}/${SDL_KERNEL_CONF}/*
rm -rf $SDK_DIR/${SDK_VERSION}/${VARIANT_NAME}/firmware
mkdir -p $SDK_DIR
unzip $1 -d $SDK_DIR > /dev/null
