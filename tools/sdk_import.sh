#!/bin/bash

set -u

CURRENT_DIR=`pwd`
SCRIPT_NAME=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)/$(basename "${BASH_SOURCE[0]}")
SCRIPT_DIR=`dirname "${SCRIPT_NAME}"`

#
# Spresense Arduino compatible package import script
#

# Help message
function show_help
{
	echo "Usage: $0 [OPTION...] <SDK export tar.gz file>"
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
	echo "       SDK export file: Tar.gz file that created by 'make exportsdk' in Spresense SDK"
	exit 1
fi

(echo $1 | egrep -q "tar.gz$")
if [ $? != 0 -o ! -f $1 ]; then 
	echo "Please input a Tar.gz file as SDK package"
	exit 1
fi

mkdir -p ${SDK_DIR}

echo "Local SDK import to ${SDK_DIR}"

ARCHIVE_FILE=$1

# Check archive file name
ARCHIVE_NAME=`basename ${ARCHIVE_FILE} .tar.gz`
EXP_LABEL=`echo ${ARCHIVE_NAME} | cut -d "-" -f 1`
VARIANT=`echo ${ARCHIVE_NAME} | cut -d "-" -f 2`
SDK_CONF=`echo ${ARCHIVE_NAME} | cut -d "-" -f 3`
DBG_TYPE=`echo ${ARCHIVE_NAME} | cut -d "-" -f 4`

if [ "${EXP_LABEL}" != "SDK_EXPORT" ]; then
	echo "Error: Please use correct SDK export archive (${ARCHIVE_NAME})"
	exit
fi

echo "Arduino SW variant         : ${VARIANT}"
echo "Spresense SDK configuration: ${SDK_CONF}"
echo "SW debug variant           : ${DBG_TYPE}"

# Extract to temp directory
TMP_DIR=`mktemp -d`
echo "TMP_DIR=${TMP_DIR}"

# Unzip archive file.(sdk-export will create)
tar xvzf ${ARCHIVE_FILE} -C ${TMP_DIR} > /dev/null

EXP_DIR=${TMP_DIR}/sdk-export
IMP_DIR=${SDK_DIR}/${VARIANT}

# Check release/debug kernel config
if [ "${FORCE_BUILD_TYPE}" == "" ]; then
	if [ "${DBG_TYPE}" == "debug" ]; then
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

# Remove previous prebuilt files
rm -rf ${IMP_DIR}/${BUILD_TYPE}

# Move SDK export into SDK prebuilt directory
mv ${EXP_DIR} ${IMP_DIR}/${BUILD_TYPE}

# Remove temporary directory
rm -rf ${TMP_DIR}

echo "Done."
