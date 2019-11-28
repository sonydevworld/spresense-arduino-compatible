#!/bin/bash

set -eu

CURRENT_DIR=`pwd`
SCRIPT_NAME=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)/$(basename "${BASH_SOURCE[0]}")
SCRIPT_DIR=`dirname "${SCRIPT_NAME}"`

#
# Spresense Arduino compatible package import script
#

if [ $# != 1 ]; then
	echo "Usage: $0 <SDK_Package_file>"
	echo "       <SDK_Package_file> = SDK_EXPORT-<variant name>-<SDK config name>-<Kernel config name>.zip"
	exit 1
fi

(echo $1 | egrep -q "zip$|ZIP$")
if [ $? != 0 -o ! -f $1 ]; then 
	echo "Please input a ZIP file as SDK package"
	exit 1
fi

# Parse file name
ARCHIVE_FILE=$1
PACKAGE_TYPE=`basename ${ARCHIVE_FILE} | cut -d "-" -f 1`
VARIANT=`basename ${ARCHIVE_FILE} | cut -d "-" -f 2`
SDK_CONF=`basename ${ARCHIVE_FILE} | cut -d "-" -f 3`
KERNEL_CONF=`basename ${ARCHIVE_FILE} | cut -d "-" -f 4- | cut -d "." -f 1`

if [ "${PACKAGE_TYPE}" != "SDK_EXPORT" ]; then
	echo "Archive name invalid. Correct file name is.. SDK_EXPORT-<variant name>-<SDK config name>-<Kernel config name>.zip"
	exit 1
fi

SDK_DIR="${SCRIPT_DIR}/../Arduino15/packages/SPRESENSE/tools/spresense-sdk/1.0.0"
mkdir -p ${SDK_DIR}

echo "Local SDK import to ${SDK_DIR}"

# Extract to temp directory
TMP_DIR=`mktemp -d`
echo "TMP_DIR=${TMP_DIR}"

# Unzip archive file.(sdk-export will create)
unzip ${ARCHIVE_FILE} -d ${TMP_DIR} > /dev/null

EXP_DIR=${TMP_DIR}/sdk-export
IMP_DIR=${TMP_DIR}/sdk/1.0.0/${VARIANT}

# Temporary extract achive to temp directory
mkdir -p ${IMP_DIR}

# Move nuttx includes into kernel directory
mv ${EXP_DIR}/nuttx ${IMP_DIR}/${KERNEL_CONF}
rm -f ${IMP_DIR}/${KERNEL_CONF}/.config

# Move application includes
mkdir -p ${IMP_DIR}/${KERNEL_CONF}/include/apps
mv ${EXP_DIR}/sdk/modules/include/* ${IMP_DIR}/${KERNEL_CONF}/include/apps
mv ${EXP_DIR}/sdk/bsp/include/sdk ${IMP_DIR}/${KERNEL_CONF}/include

# Move prebuilt binary
mkdir -p ${IMP_DIR}/${KERNEL_CONF}/prebuilt/libs
mv ${EXP_DIR}/sdk/libs/* ${IMP_DIR}/${KERNEL_CONF}/prebuilt/libs/
mv ${IMP_DIR}/${KERNEL_CONF}/prebuilt/libs/libsdk.a ${IMP_DIR}/${KERNEL_CONF}/prebuilt/libs/libnuttx.a

# create debug, release
cp -a ${IMP_DIR}/${KERNEL_CONF}/build ${IMP_DIR}/${KERNEL_CONF}/prebuilt/
rm -rf ${IMP_DIR}/${KERNEL_CONF}/build ${IMP_DIR}/${KERNEL_CONF}/libs

# Move LICENSE file
mv ${EXP_DIR}/LICENSE ${IMP_DIR}/

# Delete git specific files
rm -f `find ${TMP_DIR} -name .gitignore`
rm -f `find ${TMP_DIR} -name .fakelnk`

# Delete sdk-export directory
rm -rf ${EXP_DIR}

# Copy generated files
BOAR_VARIANT_DIR=${SDK_DIR}/${VARIANT}
KERNEL_DIR=${BOAR_VARIANT_DIR}/${KERNEL_CONF}
COMMON_DIR=${BOAR_VARIANT_DIR}/common

# Delete previous built result
rm -rf ${KERNEL_DIR}/*

cp -a ${IMP_DIR} ${SDK_DIR}/

# Move common header files into common directory
mkdir -p ${COMMON_DIR}
COMMONS=(arch include tools Make.defs)
for common in ${COMMONS[@]}
do
	cp -a ${KERNEL_DIR}/${common} ${COMMON_DIR}/
	rm -rf ${KERNEL_DIR}/${common}
done

# Move kernel independent header files
mkdir -p ${KERNEL_DIR}/include
CONFIGFILES=(nuttx/config.h sdk/config.h)
for cfile in ${CONFIGFILES[@]}
do
	mkdir -p ${KERNEL_DIR}/include/`dirname ${cfile}`
	mv ${COMMON_DIR}/include/${cfile} ${KERNEL_DIR}/include/${cfile}
done

echo "Done."
