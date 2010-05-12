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
Common module for the internationalization in Scenic.

Provides the _(...) function.
"""

import gtk.glade
import gettext
from scenic import configure
from scenic import logger

log = logger.start(name="i18n")

_ = gettext.gettext

def setup_i18n():
    """
    Bind the domain to the locale directory localedir. More concretely, gettext will look for binary .mo  files for the given domain using the path (on Unix): localedir/language/LC_MESSAGES/domain.mo, where languages is searched for in the environment variables LANGUAGE, LC_ALL, LC_MESSAGES, and LANG  respectively.
    """
    gettext.bindtextdomain(configure.APPNAME, configure.LOCALE_DIR)
    gettext.textdomain(configure.APPNAME)
    gtk.glade.bindtextdomain(configure.APPNAME, configure.LOCALE_DIR)
    gtk.glade.textdomain(configure.APPNAME)

    log.debug("i18n has been setup with domain %s and path %s." % (gettext.textdomain(), configure.LOCALE_DIR))

