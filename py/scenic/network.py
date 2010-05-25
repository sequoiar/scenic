#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.
"""
Network validation and testing.
"""
import re

def _has_alpha_xor_digits(ip):
    """
    Ensures that an IP address' fields (separated by .'s)
    are either strictly alphabetic characters of strictly 
    numeric characters.
    @param address: str IP to check
    @ret bool
    """
    has_alpha = False
    has_digit = False
    for c in ip:
        if c != '.':
            has_alpha |= str.isalpha(c)
            has_digit |= str.isdigit(c)
            if has_digit and has_alpha:
                return False
    return True

def validate_address(address):
    """
    Validates a hostname or a IPv4 address.
    @param address: str IP or hostname.
    @rtype: bool
    """
    valid_ipv4_address_regex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"
    valid_hostname_regex = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$"
    if re.match(valid_hostname_regex, address) is not None:
        return True
    elif re.match(valid_ipv4_address_regex, address) is not None:
        return _has_alpha_xor_digits(address)   # rudimentary check
    else:
        return False
