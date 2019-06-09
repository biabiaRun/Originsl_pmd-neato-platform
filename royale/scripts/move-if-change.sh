#!/usr/bin/bash

#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

# For installations that don't properly set PATH:
export PATH=/usr/bin:/bin

# Move src to dst if they differ, or dst doesn't exist.
# Otherwise, src is just removed.
# This is an independant reimplementation of the namesake script
# which is part of GNU autoconfig.
#

if [ $# != 2 ]
then
    echo "Usage: $0 <src> <dst>"
    exit 1
fi

if [ ! -f "$1" ]
then
    echo "no such file: $1"
    exit 2
fi

if [ -f "$2" ] && cmp "$1" "$2" >/dev/null
then
    rm "$1"
else
    mv "$1" "$2"
fi

exit 0
