#!/bin/bash

set -eu

CURRENT_DIR=`pwd`
SCRIPT_NAME=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)/$(basename "${BASH_SOURCE[0]}")
SCRIPT_DIR=`dirname "${SCRIPT_NAME}"`

#
# Spresense SDK export script for arduino package
#

# Help message
function show_help
{
	echo "Usage: $0 -i <SDK_TOPDIR> [OPTION...]"
	echo ""
	echo "Generating SDK export package for arduino build environment."
	echo ""
	echo "  Optional arguments:"
	echo ""
	echo "    -i: Using SDK repository directory"
	echo "    -k: Kernel config (release/debug/subcore-release/subcore-debug)"
	echo "    -K: Kernel option (m:menuconfig/g:gconf/q:qconf)"
	echo "    -s: SDK config (spresense/spresense_sub)"
	echo "    -S: SDK option (m:menuconfig/g:gconf/q:qconf)"
	echo "    -v: Variant name (default: spresense)"
	echo "    -V: SDK version (default: 1.0.0)"
	echo "    -h: Show help (This message)"
	echo ""
}

# Option handling
VARIANT_NAME=spresense
KERNEL_CONF=release
SDK_CONF=spresense
KERNEL_OPTION=""
SDK_OPTION=""
SDK_VERSION=1.0.0
while getopts i:k:K:s:S:v:V:h OPT
do
	case $OPT in
		'i' ) SDK_DIR=$OPTARG;;
		'k' ) KERNEL_CONF=$OPTARG;;
		'K' ) KERNEL_OPTION=$OPTARG;;
		's' ) SDK_CONF=$OPTARG;;
		'S' ) SDK_OPTION=$OPTARG;;
		'v' ) VARIANT_NAME=$OPTARG;;
		'V' ) SDK_VERSION=$OPTARG;;
		'h' ) show_help;;
	esac
done

PACKAGE_NAME=SDK_EXPORT-${VARIANT_NAME}-${SDK_CONF}-${KERNEL_CONF}.zip

echo "Creating exported archive ${PACKAGE_NAME}..."

SDK_DIR=`cd ${SDK_DIR} && pwd`
TMP_DIR=`mktemp -d`
OUT_DIR=${SCRIPT_DIR}/out

echo "TMP_DIR = ${TMP_DIR}"

# versioning
cd ${SDK_DIR}
TOPDIR=$SDK_DIR/nuttx bash sdk/tools/mkversion.sh

# Change directory to SDK directory
cd ${SDK_DIR}/sdk

# clean
echo "Clean SDK objects..."
make distcleankernel &>/dev/null
make distclean &>/dev/null

# Extract SDK configuration
SDK_CONFIG_FILE=${SCRIPT_DIR}/configs/${SDK_CONF}.conf
if [ ! -f ${SDK_CONFIG_FILE} ]; then
	echo "SDK config(${SDK_CONFIG_FILE}) not found."
	exit 1
fi

SDK_CONF_LIST=`cat ${SDK_CONFIG_FILE} | head -n 1`

# Configuration options
if [ "${KERNEL_OPTION}" != "" ]; then
	KERNEL_OPTION="-${KERNEL_OPTION}"
fi
if [ "${SDK_OPTION}" != "" ]; then
	SDK_OPTION="-${SDK_OPTION}"
fi

# Configuration
echo "Configure SDK components..."
echo "Kernel : ${KERNEL_CONF}"
echo "SDK    : ${SDK_CONF_LIST}"
./tools/config.py ${KERNEL_OPTION} --kernel ${KERNEL_CONF}
./tools/config.py ${SDK_OPTION} ${SDK_CONF_LIST}

# Build kernel
echo "Build kernel..."
make buildkernel

# Export
echo "Export to Arduino..."
make export

# Check make result
if [ $? != 0 ]; then
	echo "make export failed"
	exit 1
fi

# Move export arcihve into Arduino out directory
mkdir -p ${OUT_DIR}
mv sdk-export.zip ${OUT_DIR}/${PACKAGE_NAME}

# Remove temporary directory
rm -rf ${TMP_DIR}

echo "done."
