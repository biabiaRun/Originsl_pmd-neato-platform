#!/bin/sh
appname=$(basename $0 | sed s,\.sh$,,)

dirname=$(dirname $0)
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
$dirname/$appname "$@"
ERRORCODE=$?
if [ $ERRORCODE -eq 127 ]; then
   cat >&2 << EOF
$appname exited with error $ERRORCODE. This probably means that it can not load
all required libraries. To run $appname under Ubuntu at least the following
packages must be installed:
libc6
libgcc1
libqt5core5a
libqt5gui5
libqt5widgets5
libstdc++6
EOF
fi
return $ERRORCODE
