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
$0 [<options>] [-- <tool> [<tooloptions>]]

This command starts a docker container with the Royale Android build
environment. The script binds the current work directory into the docker
container. To compile Royale the easiest way is to check out royale, create a
build and a host directory beside the royale directory and then call this
command in the parent directory of these directories. If you use --, everything
after that will be executed as command inside the build environment.

The script has the follwing options:
    -h, --help
        Print this help message
    -v, --verbose
        Make the command more verbose
    -w <dir>, --workdir <dir>
        Set the working directory for the docker container explicetly.
        Default is the current working directory
    -r, --root
        Normaly the command instructs docker to use UID and GID of the current
        user inside the container. Use this option to run as root inside the
        container. Beware, All files written inside the container mounted
        working directory will belong to root.
    -e <variable_name>=<value>,--env <variable_name>=<value>
        Set an environment variable inside the started docker container.
    -i <image name>, --image <image name>
        Start the the image <image name> instead of the default one.
    --
        Pass remaining parameters to the build environment
EOF
exit $EXITCODE
}

function error() {
    echo $@ 1>&2
}

function readImageName() {
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    DONAME=$(grep 'dockerImage *=' "$SCRIPTDIR/android_arm32.groovy" | cut -f 2 -d= | cut -d\' -f2)
    DOREG=$(grep 'dockerRegistry *=' "$SCRIPTDIR/android_arm32.groovy" | cut -f 2 -d= | cut -d\' -f2)
    echo "$DOREG/$DONAME"
}

#Set default values for options
USEROPT="--user $(id -u):$(id -g)"
WORKDIR=$(pwd)
VERBOSE=false
HELP=false
ROOT=false
DOCKER_IMAGE="$(readImageName)"
declare -a ENVVARS

#Parse command line parameters
# options may be followed by one colon to indicate they have a required argument
if ! options=$(getopt -o w:e:rvhi: -l workdir:,root,verbose,help,env:,image: -- "$@")
then
    # something went wrong, getopt will put out an error message for us
    help 1
fi
eval set -- $options
while [ $# -gt 0 ]
do
    case $1 in
    -v|--verbose) VERBOSE=true;;
    -h|--help) HELP=true;;
    -r|--root) ROOT=true;;
    # for options with required arguments, an additional shift is required
    -w|--workdir) WORKDIR="$2"; shift;;
    -i|--image) DOCKER_IMAGE="$2"; shift;;
    -e|--env) ENVVARS+=("-e" "$2"); shift;;
    (--) shift; break;;
    (-*) error "$0: error - unrecognized option $1"; help 1;;
    (*)  break;;
    esac
    shift
done

#Set option value depending on commandline or print help
WORKDIR=$(realpath -s -m "$WORKDIR")
HOPTIONP="-e"
HOPTIONV="HOME=$WORKDIR"
if [ $HELP == true ]; then
    help 0
fi
if [ $ROOT == true ]; then
    unset USEROPT
    unset HOPTIONP
    unset HOPTIONV
fi
if [ $VERBOSE == true ]; then
    set -x
fi

#execute build environment
docker run -it --rm $USEROPT -w "$WORKDIR" -v "$WORKDIR:$WORKDIR:rw,z" ${HOPTIONP:+"$HOPTIONP"} ${HOPTIONV:+"$HOPTIONV"} "${ENVVARS[@]}" "$DOCKER_IMAGE" "$@"
