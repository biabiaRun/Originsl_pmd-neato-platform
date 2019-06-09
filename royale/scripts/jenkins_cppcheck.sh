#!/bin/sh

#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

if [ $# -ge 1 ] ; then
    if [ "$1" = "--help" -o "$1" = "-h" ] ; then
        echo "Options:"
        echo "    --help: this help"
        echo "    --git-html-report: format HTML report in build/cppcheck/(git hash)_(cppcheck version)/index.html"
    elif [ "$1" = "--git-html-report" ] ; then
        if ( ! git rev-parse -q --verify HEAD ) ; then
            echo "Error: Can't find a Git repo here"
            exit
        fi
        if ( ! cppcheck --version ) ; then
            echo "Error: Can't find CppCheck"
            exit
        fi
        GIT_REV_DIR=build/cppcheck/`git rev-parse --short HEAD`_`cppcheck --version`
        mkdir -p "${GIT_REV_DIR}"
        "$0" 2> "${GIT_REV_DIR}"/cppcheck.xml | tee "${GIT_REV_DIR}"/cppcheck.out 
        cppcheck-htmlreport --file="${GIT_REV_DIR}"/cppcheck.xml --report-dir="${GIT_REV_DIR}"
    else
        echo "Unknown argument " -- "$1"
    fi
    exit
fi

# samples/inc is included because it includes the PlatformResources class

cppcheck --enable=all --suppress=missingIncludeSystem --xml-version=2 \
  --max-configs=50 \
  --suppress=noExplicitConstructor:source/core/inc/royale/Variant.hpp \
  -I source/core/test/inc \
  -I source/core/inc \
  -I source/royaleCAPI/test/inc \
  -I source/royaleCAPI/inc \
  -I source/royaleCAPI/test_v220/inc \
  -I source/royale/test/inc \
  -I source/royale/inc \
  -I source/components/temperature/inc \
  -I source/components/factory/test/inc \
  -I source/components/factory/inc \
  -I source/components/buffer/inc \
  -I source/components/imager/test/inc \
  -I source/components/imager/inc \
  -I source/components/config/inc \
  -I source/components/processing/test/inc \
  -I source/components/processing/inc \
  -I source/components/record/test/inc \
  -I source/components/record/clibs/test/inc \
  -I source/components/record/clibs/inc \
  -I source/components/record/inc \
  -I source/components/storage/test/inc \
  -I source/components/storage/inc \
  -I source/components/usb/inc \
  -I samples/inc \
  samples \
  source \
  tools
