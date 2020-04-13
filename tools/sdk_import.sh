#!/bin/bash

set -eu

CURRENT_DIR=`pwd`
SCRIPT_NAME=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)/$(basename "${BASH_SOURCE[0]}")
SCRIPT_DIR=`dirname "${SCRIPT_NAME}"`

#
# Spresense Arduino compatible package import script
#

# Help message
function show_help
{
	echo "Usage: $0 [OPTION...] <SDK export zip file>"
	echo ""
	echo "Import SDK export archive into Arduino environment."
	echo ""
	echo "  Optional arguments:"
	echo ""
	echo "    -k: Force build type(release/debug)"
	echo "    -o: Prebuilt output directory"
	echo "    -h: Show help (This message)"
	echo ""
}

# Option handling
SDK_DIR="${SCRIPT_DIR}/../Arduino15/packages/SPRESENSE/tools/spresense-sdk/1.0.0"
FORCE_BUILD_TYPE=""
while getopts k:o:h OPT
do
	case $OPT in
		'k' ) FORCE_BUILD_TYPE=$OPTARG ;;
		'o' ) SDK_DIR=`cd $OPTARG && pwd`;;
		'h' ) show_help;;
	esac
done

# Shift argument position after option
shift $(($OPTIND - 1))

if [ $# != 1 ]; then
	echo "Usage: $0 <SDK export file>"
	echo "       SDK export file: Zip file that created by 'make export' in Spresense SDK"
	exit 1
fi

(echo $1 | egrep -q "zip$|ZIP$")
if [ $? != 0 -o ! -f $1 ]; then 
	echo "Please input a ZIP file as SDK package"
	exit 1
fi

mkdir -p ${SDK_DIR}

echo "Local SDK import to ${SDK_DIR}"

ARCHIVE_FILE=$1
VARIANT=spresense

# Extract to temp directory
TMP_DIR=`mktemp -d`
echo "TMP_DIR=${TMP_DIR}"

# Unzip archive file.(sdk-export will create)
unzip ${ARCHIVE_FILE} -d ${TMP_DIR} > /dev/null

EXP_DIR=${TMP_DIR}/sdk-export
IMP_DIR=${TMP_DIR}/sdk/1.0.0/${VARIANT}

# Check MainCore/SubCore
if [ "`grep "CONFIG_CXD56_SUBCORE=y" ${EXP_DIR}/sdk/.config`" != "" ]; then
	SDK_CONF=spresense_sub
else
	SDK_CONF=spresense
fi

# Check release/debug kernel config
if [ "${FORCE_BUILD_TYPE}" == "" ]; then
	if [ "`grep "CONFIG_DEBUG_FEATURES=y" ${EXP_DIR}/nuttx/.config`" != "" ]; then
		CONFIG_ENABLE_DEBUG=true
	else
		CONFIG_ENABLE_DEBUG=false
	fi
else
	if [ "${FORCE_BUILD_TYPE}" == "debug" ]; then
		CONFIG_ENABLE_DEBUG=true
	else
		CONFIG_ENABLE_DEBUG=false
	fi
fi

# Select build type
if [ "${SDK_CONF}" == "spresense" ]; then
	BUILD_TYPE=""
else
	BUILD_TYPE="subcore-"
fi

if [ "${CONFIG_ENABLE_DEBUG}" == "true" ]; then
	BUILD_TYPE="${BUILD_TYPE}debug"
else
	BUILD_TYPE="${BUILD_TYPE}release"
fi

# Temporary extract achive to temp directory
mkdir -p ${IMP_DIR}

# Move nuttx includes into kernel directory
mv ${EXP_DIR}/nuttx ${IMP_DIR}/${BUILD_TYPE}
rm -f ${IMP_DIR}/${BUILD_TYPE}/.config

# Move application includes
mkdir -p ${IMP_DIR}/${BUILD_TYPE}/include/apps
mv ${EXP_DIR}/sdk/modules/include/* ${IMP_DIR}/${BUILD_TYPE}/include/apps
mv ${EXP_DIR}/sdk/bsp/include/sdk ${IMP_DIR}/${BUILD_TYPE}/include

# Move prebuilt binary
mkdir -p ${IMP_DIR}/${BUILD_TYPE}/prebuilt/libs
mv ${EXP_DIR}/sdk/libs/* ${IMP_DIR}/${BUILD_TYPE}/prebuilt/libs/
mv ${IMP_DIR}/${BUILD_TYPE}/prebuilt/libs/libsdk.a ${IMP_DIR}/${BUILD_TYPE}/prebuilt/libs/libnuttx.a

# create debug, release
cp -a ${IMP_DIR}/${BUILD_TYPE}/build ${IMP_DIR}/${BUILD_TYPE}/prebuilt/
rm -rf ${IMP_DIR}/${BUILD_TYPE}/build ${IMP_DIR}/${BUILD_TYPE}/libs

# Move LICENSE file
mv ${EXP_DIR}/LICENSE ${IMP_DIR}/

# Delete git specific files
rm -f `find ${TMP_DIR} -name .gitignore`
rm -f `find ${TMP_DIR} -name .fakelnk`

# Delete sdk-export directory
rm -rf ${EXP_DIR}

# Copy generated files
BOAR_VARIANT_DIR=${SDK_DIR}/${VARIANT}
KERNEL_DIR=${BOAR_VARIANT_DIR}/${BUILD_TYPE}
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

#If target is subcore, copy prebuilt into debug
if [ "${SDK_CONF}" == "spresense_sub" -a "${FORCE_BUILD_TYPE}" == "" ]; then
	DEBUG_KERNEL_DIR=${KERNEL_DIR}/../subcore-debug
	rm -rf ${DEBUG_KERNEL_DIR}
	cp -a ${KERNEL_DIR} ${DEBUG_KERNEL_DIR}
fi

# Remove temporary directory
rm -rf ${TMP_DIR}

echo "Done."
