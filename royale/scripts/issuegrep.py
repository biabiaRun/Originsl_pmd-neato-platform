#!/usr/bin/python3

"""Usage: git grep PROJECT | %prog [exported_jira_issues.xml]

Given a list of Jira tickets (for example the ones resolved in the current
sprint), search the source code for comments that still mention them. This
script is a filter that works similarly to "grep -f".

First get a list of issues:

* Use your web browser to search Jira for a list of issues
* Click on Jira's "Export" drop-down (under the search box)
* Choose "XML", and then save the page to local disk

* For example, the list of all of issues in v3.2.0:
https://jira.ifm.com/issues/?jql=project%20%3D%20royale%20AND%20fixVersion%20%3D%20v3.2.0
* Note: if there are more than 1000 issues in the list, read the advanced_usage.

Then run this (assuming your tickets are all "ROYAL-xxxx"):

* git grep ROYAL | scripts/issuegrep.py list_of_jira_issues.xml

There are two grep operations there, which is much faster than combining them in
to a single one.  The "git grep" searches many lines with a single pattern, and
then issuegrep.py searches a few lines with lots of patterns.

It's also possible to take a list of open issues, and check for any comments
mentioning issues not in the list.  Note that you'll need either the --todo
option or "ROYAL-" in git grep's argument to ignore lines that contain the
name of the project but aren't referring to a Jira issue.

* git grep ROYAL- | scripts/issuegrep.py --invert-match list_of_jira_issues.xml
* git grep ROYAL | scripts/issuegrep.py --todo --invert-match list_of_jira_issues.xml

"""

import optparse
import re
import sys
import xml.etree.ElementTree as ET

def advanced_usage ():
    """Omitted from the main doc:

    The --print-patterns option allows using "grep -f" or "git grep -f", like this:
    * scripts/issuegrep.py --print-patterns jira_issues.xml > jira_issues.greplist
    * git grep -f jira_issues.greplist
    but using "git grep | grep" is much faster, because searches many lines with a
    single pattern, and then few lines with lots of patterns.

    The grep lists terminate each line with "\\>", because this detects end-of word.
    For example, "ROYAL-206\\>" won't match any four-digit issue numbers.

    PMD's Jira server returns a maximum of 1000 issues.  If you want to get the list
    of all resolved issues then it's necessary to split it in to searches which will
    return at most 1000 items:
    project = royale AND resolution is not EMPTY AND key <= "ROYAL-1000"
    project = royale AND resolution is not EMPTY AND key > "ROYAL-1000" AND key <= "ROYAL-2000"
    etc
    """
    return __doc__

def get_keys_from_xml_file (filename):
    keys = []
    try:
        tree = ET.parse (filename)
        keys = tree.getroot().findall ('./channel/item/key')
        if (len (keys) == 0):
            print ("Parsed file " + filename + " but didn't find any Jira issues", file=sys.stderr)
    except ET.ParseError:
        print ("Can't parse file " + filename, file=sys.stderr)
    except FileNotFoundError:
        print ("File not found " + filename, file=sys.stderr)
    return keys

if __name__ == '__main__':
    parser = optparse.OptionParser (__doc__)
    parser.add_option ('--print-patterns', dest='print_patterns', action='store_const', const=True, help='Output the patterns for a separate grep command to read')
    parser.add_option ('--todo', dest='todo_only', action='store_const', const=True, help='Only show lines that include the word "todo"')
    parser.add_option ('--invert-match', dest='invert_match', action='store_const', const=True, help="Only output lines that don't match")
    (optvalues, args) = parser.parse_args()

    # There was a "--filter" option, which is now the default
    optvalues.filter = not optvalues.print_patterns

    if (len (args) == 0):
        parser.print_help()
        exit (1)

    keys = []
    for filename in args:
        keys += get_keys_from_xml_file (filename)

    if optvalues.print_patterns:
        if (optvalues.todo_only):
            for key in keys:
                print ('todo.*' + key.text + '\\>')
                print (key.text + '\\>.*todo')
        else:
            for key in keys:
                print (key.text + '\\>')

    if optvalues.filter:
        patterns = []
        for key in keys:
            # These patterns ensure "ROYAL-123" does not match "ROYAL-1234"
            patterns.append (re.compile (key.text + '(\Z|\D)'))
        for line in sys.stdin.read().split("\n"):
            match = False
            for pattern in patterns:
                if pattern.search (line):
                    match = True
            if optvalues.invert_match:
                match = not match
            if optvalues.todo_only:
                if not ("\\todo" in line) or ("\\TODO" in line):
                    match = False
            if match:
                print (line)
