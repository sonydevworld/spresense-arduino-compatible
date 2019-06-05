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

echo "Local SDK import to ${SDK_DIR}"

BOAR_VARIANT_DIR=${SDK_DIR}/${SDK_VERSION}/${VARIANT_NAME}
KERNEL_DIR=${BOAR_VARIANT_DIR}/${SDK_KERNEL_CONF}
FIRMWARE_DIR=${BOAR_VARIANT_DIR}/firmware
COMMON_DIR=${BOAR_VARIANT_DIR}/common

rm -rf ${KERNEL_DIR}/*
rm -rf ${FIRMWARE_DIR}
rm -rf ${COMMON_DIR}

mkdir -p ${SDK_DIR}
unzip $1 -d ${SDK_DIR} > /dev/null

# Move common header files into common directory
mkdir -p ${COMMON_DIR}
COMMONS=(arch include tools Make.defs)
for common in ${COMMONS[@]}
do
	mv ${KERNEL_DIR}/${common} ${COMMON_DIR}/
done

# Move kernel independent header files
mkdir -p ${KERNEL_DIR}/include
CONFIGFILES=(nuttx/config.h sdk/config.h)
for cfile in ${CONFIGFILES[@]}
do
	mkdir -p ${KERNEL_DIR}/include/`dirname ${cfile}`
	mv ${COMMON_DIR}/include/${cfile} ${KERNEL_DIR}/include/${cfile}
done

# Move LICENSE file
mv ${KERNEL_DIR}/LICENSE ${BOAR_VARIANT_DIR}/
