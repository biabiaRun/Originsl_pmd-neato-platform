#!/bin/bash -e

#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************


function help () {
EXITCODE=$1
cat <<EOF
$0 [<options>] [<targets>]

This command can be used to configure, compile, test and package royale for
Android. The Jenkins uses this script to build for Android, so if you use the
script, the resulting binaries are build with the same build definitions that
jenkins uses.
The script provides several targets similar to the cmake targets. You can call
several of the targets to run the targets consecutive. If you do not provide
any target on the commandline the targets configure, build and package are run.

The script provides the following targets:

    configure
        Runs cmake configure steps in the build directory and the halide host
        build directory. The installation directory of halide must be
        configured in the environment variable HALIDE_ROOT or Halide_ROOT.

    clean
        Runs cmake build step with target clean in the build directory and the
        halide host build directory.

    build
        Runs cmake build step in the build directory.

    test
        Copies binaries and testdata to an Android test device and runs cmake
        build step with target test in the build directory. The test device and
        the test directory on the test device must be configured via the
        environment variables TARGET_DEVICE_IP and TARGET_DEVICE_DIR.
        TARGET_DEVICE_IP has to include the device ip and the port separated by
        a colon. TARGET_DEVICE_DIR must be the absolute path on the test device
        to the test directory.

    install
        Runs cmake build step with target install in the build directory.

    package
        Runs cmake build step with target package in the build directory.

The script has the follwing options:
    -v, --verbose
        Instruct cmake to to use verbose output
    -h, --help
        Print this help message
    -b <dir>, --builddir <dir>
        The path to the build directory. If omitted the default path is the
        current directory.
    -s <dir>, --sourcedir <dir>
        Path to the source directory. If omitted the default path is
        "../royal"
    -a <bitsize> --bitsize <bitsize>
        Architecture bitsize that should be build. This can be "32" or "64".
        With 32 spectre is build for an Arm7 neon vfpv4 32Bit platform, 64
        builds an Arm8 64Bit platform. Default is 32. 
    -j <int>, --jobs <int>
        Number of jobs that make should run in parallel during build.
        Default is 1
EOF
exit $EXITCODE
}

function error() {
    echo $@ 1>&2
}

function configure() {
    SOURCEDIR="$1"
    BITSIZE="$2"
    TOOLCHAINFILE="$SOURCEDIR/cmake/toolchains/android_arm_32.cmake"
    if [ "$BITSIZE" == "64" ]; then
        TOOLCHAINFILE="$SOURCEDIR/cmake/toolchains/android_arm_64.cmake"
    fi
    (set -x
    cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAINFILE" -DCMAKE_BUILD_TYPE=RELEASE "$SOURCEDIR")
}

function clean() {
    (set -x
    cmake  --build . --target clean
    )
}

function build () {
    VERBOSE=$1
    JOBS=$2
    (set -x
    cmake --build . -- --jobs=$JOBS VERBOSE=$VERBOSE
    )
}

function runInstall () {
    VERBOSE=$1
    JOBS=$2
    (set -x
    cmake --build . --target install -- --jobs=$JOBS VERBOSE=$VERBOSE
    )
}

function package () {
    VERBOSE=$1
    JOBS=$2
    (set -x
    cmake --build . --target package -- --jobs=$JOBS VERBOSE=$VERBOSE
    )
}

function runTest () {
    error "Tests on android are not implemented for royale"
}

SOURCEDIR="../royal"
BUILDDIR="./"
SHARED="true"
SYSTARGET="arm"
BITSIZE=32
VERBOSE=0
JOBS=1
CONFIG=false
BUILD=false
TEST=false
INSTALL=false
PACKAGE=false
HELP=false
CLEAN=false

# options may be followed by one colon to indicate they have a required argument
if ! options=$(getopt -o s:b:a:vj:h -l sourcedir:,builddir:,bitsize:,verbose,jobs:,help -- "$@")
then
    # something went wrong, getopt will put out an error message for us
    help 1
fi
eval set -- $options
while [ $# -gt 0 ]
do
    case $1 in
    # for options with required arguments, an additional shift is required
    -v|--verbose) VERBOSE=1;;
    -h|--help) HELP=true;;
    -s|--sourcedir) SOURCEDIR="$2"; shift;;
    -b|--builddir) BUILDDIR="$2"; shift;;
    -a|--bitsize) BITSIZE="$2"; shift;;
    -j|--jobs) JOBS="$2"; shift;;
    (--) shift; break;;
    (-*) error "$0: error - unrecognized option $1"; help 1;;
    (*)  break;;
    esac
    shift
done

if [ $# -eq 0 ]; then
    CONFIG=true
    BUILD=true
    PACKAGE=true
else
    while [ $# -gt 0 ]
    do
        case ${1,,} in
            configure) CONFIG=true ;;
            build) BUILD=true ;;
            test) TEST=true ;;
            install) INSTALL=true ;;
            package) PACKAGE=true ;;
            clean) CLEAN=true ;;
            (--) shift; break;;
            (*)  error Unknown target $1; help 1; break;;
        esac
        shift
    done
fi

echo "SOURCEDIR is $SOURCEDIR"
echo "BUILDDIR is $BUILDDIR"

SOURCEDIR=$(realpath -s -m "$SOURCEDIR")

#Read proxyconfig from file if available
#Normaly used on jenkins
if [ -n "$PROXYCONFIG" -a -r "$PROXYCONFIG" ]; then
    source "$PROXYCONFIG"
fi

pushd "$BUILDDIR"
if [ $HELP == true ]; then
    help 0
fi
if [ $CLEAN == true ]; then
    clean
fi
if [ $CONFIG == true ]; then
    configure "$SOURCEDIR" "$BITSIZE"
fi
if [ $BUILD == true ]; then
    build "$VERBOSE" "$JOBS"
fi
if [ $TEST == true ]; then
    runTest "$SOURCEDIR"
fi
if [ $INSTALL == true ]; then
    runInstall "$VERBOSE" "$JOBS"
fi
if [ $PACKAGE == true ]; then
    package "$VERBOSE" "$JOBS"
fi
popd
