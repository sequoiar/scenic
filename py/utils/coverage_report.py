#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
This script take a list of .coverage file created by Twisted Trial and output a
summary for each file with its percentage of coverage.
"""

import sys

def parse(lines, detail=False):
    """
    Parse lines of a coverage file created by Twisted Trial and output the number
    of statments/executed statments and lines numbers of unexecuted statments
    (if detail is True) .
    """
    statments = 0
    executed = 0
    not_covered = []
    for line_number, line in enumerate(lines):
        line = line.strip()
        if line:
            if line[0] == ">":
                statments += 1
                if detail:
                    not_covered.append(line_number)
            elif line[0].isdigit():
                statments += 1
                executed += 1
    
    return statments, executed, not_covered

def format_not_covered(lines):
    """
    Put succesive lines in a start-end pair.
    """
    if len(lines) > 0:
        pairs = []
        start = None
        i = 0
        while i < len(lines):
            if start == None:
                start = lines[i]
            elif lines[i - 1] + 1 != lines[i]:
                pairs.append((start, lines[i - 1]))
                start = lines[i]
            i += 1
        pairs.append((start, lines[i - 1]))
        return string_pairs(pairs)
    return "None"
    
def string_pairs(pairs):
    """
    Convert to string and format the start-end pairs. 
    """
    str_pairs = []
    for pair in pairs:
        if pair[0] == pair[1]:
            str_pairs.append('%s' % pair[0])
        else:
            str_pairs.append('%s-%s' % pair)
    return ", ".join(str_pairs)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "You should specified coverage files to parse"
    else:
        for name in sys.argv[1:]:
            try:
                file_coverage = open(name, 'r')
            except:
                print "ERROR: Could not open this file: %s" % name
            else:
                lines = file_coverage.readlines()
                file_coverage.close()
                if lines:
                    statments, executed, not_covered = parse(lines, True)
                    script_name = name.rpartition('/')[2]
                    script_name = script_name.rpartition('.')[0] + ".py"
                    print script_name
                    print "-----------------------------"
                    print "Statements:", statments
                    print "Executed:", executed
                    print "Covered:", executed * 100 / statments, "%"
                    print "-----------------------------"
                    print "Lines not covered:"
                    print format_not_covered(not_covered)
                    print ""
