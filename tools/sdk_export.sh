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
	echo "    -s: SDK config (spresense/spresense_sub)"
	echo "    -S: SDK option (m:menuconfig/g:gconf/q:qconf)"
	echo "    -v: Variant name (default: spresense)"
	echo "    -V: SDK version (default: 1.0.0)"
	echo "    -h: Show help (This message)"
	echo ""
}

# Option handling
SDK_DIR=""
VARIANT_NAME=spresense
SDK_CONF=spresense
SDK_OPTION=""
SDK_VERSION=1.0.0
while getopts i:s:S:v:V:h OPT
do
	case $OPT in
		'i' ) SDK_DIR=$OPTARG;;
		's' ) SDK_CONF=$OPTARG;;
		'S' ) SDK_OPTION=$OPTARG;;
		'v' ) VARIANT_NAME=$OPTARG;;
		'V' ) SDK_VERSION=$OPTARG;;
		'h' ) show_help;;
	esac
done

# Option check
if [ "${SDK_DIR}" == "" -o ! -d "${SDK_DIR}" ]; then
	echo "Please choose correct SDK path (${SDK_DIR})"
	exit
fi

PACKAGE_NAME=SDK_EXPORT-${VARIANT_NAME}-${SDK_CONF}.tar.gz

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
make distclean &>/dev/null

# Extract SDK configuration
SDK_CONFIG_FILE=${SCRIPT_DIR}/configs/${SDK_CONF}.conf
if [ ! -f ${SDK_CONFIG_FILE} ]; then
	echo "SDK config(${SDK_CONFIG_FILE}) not found."
	exit 1
fi

SDK_CONF_LIST=`cat ${SDK_CONFIG_FILE} | head -n 1`

# Configuration options
if [ "${SDK_OPTION}" != "" ]; then
	SDK_OPTION="-${SDK_OPTION}"
fi

# Configuration
echo "Configure SDK components..."
echo "SDK    : ${SDK_CONF_LIST}"
./tools/config.py ${SDK_OPTION} ${SDK_CONF_LIST}

# Export
echo "SDK Export to Arduino..."
make exportsdk

# Check make result
if [ $? != 0 ]; then
	echo "make export failed"
	exit 1
fi

# Move export arcihve into Arduino out directory
mkdir -p ${OUT_DIR}
mv sdk-export.tar.gz ${OUT_DIR}/${PACKAGE_NAME}

# Remove temporary directory
rm -rf ${TMP_DIR}

echo "done."
