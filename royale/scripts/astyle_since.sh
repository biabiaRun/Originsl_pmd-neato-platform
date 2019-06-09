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

if [ "${1:---help}" = "--help" ] ; then
    cat <<EOT
    Usage: $0 <git commitish>

    This reformats source code that's changed since a given git
    commit.

    Example: $0 HEAD
    will reformat only the files that would be committed if you
    did a "git commit -a".

    Example: $0 origin/feature/sprint3
    will reformat all the files that are different to the current
    head of sprint3.
EOT
    exit
fi

# We can't store NUL-separated strings in a Bash variable,
# so instead store the commands that generate them.
LIST_COMMAND="git diff --name-only --no-renames --diff-filter=ACM ${1:-HEAD} -z"
GREP_COMMAND='grep --null --null-data \.\(c\|h\|cpp\|hpp\|cs\)$'

echo "Files updated since reference point (${1:-HEAD}):"
$LIST_COMMAND | $GREP_COMMAND | xargs -0 -n1 echo

echo
echo "Files reformatted:"
$LIST_COMMAND | $GREP_COMMAND | xargs -0 \
  astyle -Q --suffix=none --style=allman --indent=spaces=4 --align-pointer=name --align-reference=name --indent-switches --indent-cases --pad-oper --pad-paren-out --pad-header --unpad-paren --indent-namespaces --add-brackets --convert-tabs --mode=c
