#! /bin/bash

set -u

CURRENT_DIR=`pwd`
SCRIPT_NAME=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)/$(basename "${BASH_SOURCE[0]}")
SCRIPT_DIR=`dirname "$SCRIPT_NAME"`
ARDUINO_DIR=${SCRIPT_DIR}/../Arduino15

# For debug
DEBUG=No

#
# Common description
#
JSON_FILE=package_spresense_index.json
PACKAGE_NAME=SPRESENSE
BOARD_NAME=spresense

#
# Tools description
#
SDK_NAME=spresense-sdk
GCC_NAME=gcc-arm-none-eabi

#
# Script description
#
SDK_IMPORT=${SCRIPT_DIR}/sdk_import.sh
SDK_EXPORT=${SCRIPT_DIR}/sdk_export.sh
JSN_LOADER=${SCRIPT_DIR}/python/load_tool_variable_from_json.py

#
# Function description
#

# function   : debug_print
# description: Print debug information if DEBUG is 'Yes'
function debug_print()
{
	if [ "${DEBUG}" == "Yes" ]; then
		echo $1
	fi
}

# function   : show_help
# description: Show help and exit.
function show_help()
{
	echo "  Usage:"
	echo "       $0 [OPTION...]"
	echo ""
	echo "  Optional arguments:"
	echo ""
	echo "    For using local archive:"
	echo "       -g: GCC tool archive path (if you use local archive)"
	echo "       -s: Spresense SDK archive path (if you use local archive)"
	echo ""
	echo "    For using local source code:"
	echo "       -S: Spresense SDK build root path (if you use local build)"
	echo "       -v: Spresense variant name (default: spresense)"
	echo "       -c: Spresense SDK configuration. (spresense or spresense_sub) (default: spresense)"
	echo "       -d: Spresense debug build option. (disable or enable) (default: disable)"
	echo "       -M: Manual build configuration by menuconfig (e.g. -M SDK)."
	echo "       -G: Manual build configuration by gconfig (e.g. -G SDK)."
	echo "       -Q: Manual build configuration by qconfig (e.g. -Q SDK)."
	echo "       -i: Skip build configuration"
	echo ""
	echo "    Other option:"
	echo "       -H: Target Arduino IDE Host name for downloading GCC achive (Windows/Linux32/Linux64/Mac)"
	echo "       -p: Don't download GCC archive"
	echo "       -h: Show help (This message)"
	echo ""
	echo ""
	echo "  Example:"
	echo ""
	echo "    Export SDK prebuilt binary for Main core/Debug:Disabled with default configuration:"
	echo "       $0 -S path/to/spresense -p"
	echo ""
	echo "    Export SDK prebuilt binary for Main core/Debug:Enabled with default configuration:"
	echo "       $0 -S path/to/spresense -d enable -p"
	echo ""
	echo "    Export SDK prebuilt binary for Sub core/Debug:Disabled with default configuration:"
	echo "       $0 -S path/to/spresense -c spresense_sub -p"
	echo ""
	echo "    Export SDK prebuilt binary for Sub core/Debug:Enabled with default configuration:"
	echo "       $0 -S path/to/spresense -c spresense_sub -d enable -p"
	echo ""
	exit
}

# function   : install_tool_from_http
# description: Install target tool from HTTP.
#              Tool version and URL is from package_*_index.json(${JSON_FILE})
# $1         : Package name
# $2         : Board name
# $3         : Json file name
# $4         : Arduino IDE Host name(Windows/Linux32/Linux64/Mac)
# $5         : Target tool name
function install_tool_from_http()
{
	PKG_NAME=$1
	BRD_NAME=$2
	JSN_NAME=${ARDUINO_DIR}/$3
	TARGET_HOST=$4
	TOOL_NAME=$5
	TOOL_VERSION=`${JSN_LOADER} -j ${JSN_NAME} -p ${PKG_NAME} -b ${BRD_NAME} -t ${TOOL_NAME} -H ${TARGET_HOST} -k version`
	TOOL_ADDR=`${JSN_LOADER} -j ${JSN_NAME} -p ${PKG_NAME} -b ${BRD_NAME} -t ${TOOL_NAME} -H ${TARGET_HOST} -k url`
	TOOL_PATH=${ARDUINO_DIR}/packages/${PKG_NAME}/tools/${TOOL_NAME}

	# Host tool check
	if [ "`which wget`" == "" ]; then
		echo "ERROR: 'wget' not installed. Please install 'wget'"
		exit
	fi

	echo "Download and Install Tool:${TOOL_NAME}, Version:${TOOL_VERSION}..."
	mkdir -p ${TOOL_PATH}

	debug_print "Download ${TOOL_NAME} into ${TOOL_PATH}"
	wget ${TOOL_ADDR} -O ${TOOL_PATH}/package.tar.gz

	echo "Install ${TOOL_NAME}..."
	tar xzf ${TOOL_PATH}/package.tar.gz -C ${TOOL_PATH}
	rm ${TOOL_PATH}/package.tar.gz

}

# function   : install_tool_from_archive
# description: Install target tool from archive.
# $1         : Package name
# $2         : Target tool name
# $3         : Archive path
function install_tool_from_archive()
{
	PACKAGE_NAME=$1
	TOOL_NAME=$2
	ARCHIVE_PATH=$3

	# Check file exists
	if [ ! -f "${ARCHIVE_PATH}" ]; then
		echo "ERROR: No such file(${ARCHIVE_PATH})"
		exit
	fi

	TOOL_PATH=${ARDUINO_DIR}/packages/${PACKAGE_NAME}/tools/${TOOL_NAME}

	# Create tool path
	mkdir -p ${TOOL_PATH}

	echo "Install ${TOOL_NAME} from archive..."
	debug_print "Install ${TOOL_NAME} to ${TOOL_PATH} from archive ${ARCHIVE_PATH}..."
	tar xzf ${ARCHIVE_PATH} -C ${TOOL_PATH}
}

# function   : install_sdk_from_build
# description: Install target SDK from build.
#              SDK version is from package_*_index.json(${JSON_FILE})
# $1         : Package name
# $2         : Board name
# $3         : Json file name
# $4         : Arduino IDE Host name(Windows/Linux32/Linux64/Mac)
# $5         : Target tool name
function install_sdk_from_build()
{
	PKG_NAME=$1
	BRD_NAME=$2
	JSN_NAME=${ARDUINO_DIR}/$3
	TARGET_HOST=$4
	SDK_NAME=$5
	VARIANT_NAME=$6
	DEBUG_OPT=$7

	echo "Install SDK from Spresense build..."
	debug_print "Using ${SPRESENSE_SDK_PATH}"
	if [ ! -d "${SPRESENSE_SDK_PATH}" ]; then
		echo "No such directory(${SPRESENSE_SDK_PATH})"
		exit
	fi

	# Build type
	if [ "${DEBUG_OPT}" == "disable" ]; then
		BUILD_TYPE="release"
	elif [ "${DEBUG_OPT}" == "enable" ]; then
		BUILD_TYPE="debug"
	fi

	SDK_CONFIG_OPTION=""
	# Add configuration option
	if [ "${CONFIG_EDIT}" != "" ]; then
		CONFIG_OPTION=`echo ${CONFIG_EDIT} | cut -d " " -f 1`
		CONFIG_TARGET=`echo ${CONFIG_EDIT} | cut -d " " -f 2`
		if [ "`echo ${CONFIG_TARGET} | grep -i sdk`" != "" ]; then
			SDK_CONFIG_OPTION="${CONFIG_OPTION}"
		fi
	fi

	# Export SDK build
	if [ ! ${IMPORT_ONLY} ]; then
		${SCRIPT_DIR}/sdk_export.sh -i ${SPRESENSE_SDK_PATH} \
									-s ${SDK_CONF} \
									-S "${SDK_CONFIG_OPTION}" \
									-d ${DEBUG_OPT} \
									-v ${VARIANT_NAME}
	fi

	# Import SDK build into Arduino15
	EXPORT_PACKAGE_NAME=SDK_EXPORT-${VARIANT_NAME}-${SDK_CONF}-${BUILD_TYPE}.tar.gz
	${SCRIPT_DIR}/sdk_import.sh ${SCRIPT_DIR}/out/${EXPORT_PACKAGE_NAME}
}

# Option handler
# -S: Spresense source path "your/path/to/spresense"
# -g: gcc archive path "your/path/to/gcc.tar.gz"
# -s: sdk archive path "your/path/to/sdk.tar.gz"
# -v: board variant (default: spresense)
# -c: SDK configuration (default: spresense)
# -d: Debug configuration (default: disable)
# -H: target Arduino Host (Windows/Linux32/Linux64/Mac)
# -M: manual configuration by menuconfig (Kernel/SDK)
# -G: manual configuration by gconfig (Kernel/SDK)
# -Q: manual configuration by qconfig (Kernel/SDK)
# -p: Use private access (Skip GCC installation)
SPRESENSE_SDK_PATH=""
GCC_ARCHIVE_PATH=""
SDK_ARCHIVE_PATH=""
SDK_VARIANT_NAME="spresense"
SDK_CONF="spresense"
DEBUG_OPT="disable"
AURDUINO_IDE_HOST=""
CONFIG_EDIT=""
IMPORT_ONLY=""
PRIVATE_ACCESS=""
while getopts S:g:s:v:c:d:H:M:G:Q:iph OPT
do
	case $OPT in
		'S' ) SPRESENSE_SDK_PATH=$OPTARG;;
		'g' ) GCC_ARCHIVE_PATH=$OPTARG;;
		's' ) SDK_ARCHIVE_PATH=$OPTARG;;
		'v' ) SDK_VARIANT_NAME=$OPTARG;;
		'c' ) SDK_CONF=$OPTARG;;
		'd' ) DEBUG_OPT=$OPTARG;;
		'H' ) AURDUINO_IDE_HOST=$OPTARG;;
		'M' ) CONFIG_EDIT="m $OPTARG";;
		'G' ) CONFIG_EDIT="g $OPTARG";;
		'Q' ) CONFIG_EDIT="q $OPTARG";;
		'i' ) IMPORT_ONLY=true;;
		'p' ) PRIVATE_ACCESS=true;;
		'h' ) show_help;;
	esac
done

#
# Main stream
#

# Option Errror handling
if [ "${AURDUINO_IDE_HOST}" == "" ]; then
	UNAME=`uname`
	if [ "${UNAME}" == "Linux" ]; then
		AURDUINO_IDE_HOST="Linux64"
	elif [ "${UNAME}" == "Darwin" ]; then
		AURDUINO_IDE_HOST="Mac"
	else
		AURDUINO_IDE_HOST="Windows"
	fi

	echo "Target host auto detect... ${AURDUINO_IDE_HOST}"
fi

# GCC install
if [ "${PRIVATE_ACCESS}" == "true" ]; then
	echo "Skip GCC installation."
elif [ "${GCC_ARCHIVE_PATH}" != "" ]; then
	# Using arvhive install
	install_tool_from_archive ${PACKAGE_NAME} ${GCC_NAME} ${GCC_ARCHIVE_PATH}
else
	# Using HTTP install
	install_tool_from_http ${PACKAGE_NAME} ${BOARD_NAME} ${JSON_FILE} ${AURDUINO_IDE_HOST} ${GCC_NAME}
fi

# SDK install
if [ "${SDK_ARCHIVE_PATH}" != "" ]; then
	# Using arvhive install
	install_tool_from_archive ${PACKAGE_NAME} ${SDK_NAME} ${SDK_ARCHIVE_PATH}
elif [ "${SPRESENSE_SDK_PATH}" != "" ]; then
	# Using SDK build
	install_sdk_from_build ${PACKAGE_NAME} ${BOARD_NAME} ${JSON_FILE} ${AURDUINO_IDE_HOST} ${SDK_NAME} ${SDK_VARIANT_NAME} ${DEBUG_OPT}
else
	# Using HTTP install
	install_tool_from_http ${PACKAGE_NAME} ${BOARD_NAME} ${JSON_FILE} ${AURDUINO_IDE_HOST} ${SDK_NAME}
fi

