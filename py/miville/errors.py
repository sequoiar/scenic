#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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
Exceptions that might be raised within the miville software.
"""
### Settings ###
class StreamsError(Exception):
    """
    Master Exception Class for the streams.
    """
    pass


class SettingsError(Exception):
    """
    Master Exception Class for the settings.
    """
    pass

### Address Book ###
class AddressBookError(Exception):
    """
    Master Exception Class for the address book.
    """
    pass

class AddressBookNameError(AddressBookError):
    """
    Sub Exception Class for the address book.
    Name already in address book.
    """
    pass

class AddressBookAddressError(AddressBookError):
    """
    Sub Exception Class for the address book.
    Address (combination of address and port ?and connector?) already in address book.
    """
    pass

### Utils ###
class InstallFileError(Exception):
    """
    Exception with a preference file.
    """

### Connection ###
class ConnectionError(Exception):
    """
    Master Exception Class for the Connection module.
    """
    pass

class ConnectorError(ConnectionError):
    """
    Sub Exception Class for the Connector module.
    """
    pass


class CommandNotFoundError(Exception):
    """
    raised when a shell command is not found
    """
    pass

class DeviceError(Exception):
    """
    Can be raised at Device.prepare() or when polling.
    """
    pass

