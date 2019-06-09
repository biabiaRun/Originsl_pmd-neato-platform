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

# This file does a pretty format for a given source file

if [ $# -gt 0 ]
then
  echo "Formatting $@..."
  astyle --style=allman --indent=spaces=4 --align-pointer=name --align-reference=name --indent-switches --indent-cases --pad-oper --pad-paren-out --pad-header --unpad-paren --indent-namespaces --add-brackets --convert-tabs --mode=c "$@"
else
  echo "Please set a filename ..."
fi
exit
