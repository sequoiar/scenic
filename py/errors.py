# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.


### Settings ###
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


