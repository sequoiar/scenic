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
#!/usr/bin/env python
#
"""
Installation script for Miville.
--------------------------------

For developpers::
  python setup.py build
  sudo python setup.py develop --prefix=/usr/local
  sudo python setup.py develop --prefix=/usr/local --uninstall

For users::
  python setup.py build
  sudo python setup.py install --prefix=/usr/local

(There is no easy way to uninstall a Python package once installed...)
"""
from setuptools import find_packages
from setuptools import setup

setup(
    name = "miville",
    version = "0.1.3-a1",
    author = "Society for Arts and Technology",
    author_email = "info@sat.qc.ca",
    url = "http://sat.qc.ca/",
    description = "Miville telepresence application for venues and live performances",
    long_description = "Miville provides 1-8 multichannel audio and h264, mpeg4, h263 in SD format",
    install_requires = [], # "twisted", "nevow", 
    scripts = ["miville.py", "restart_jackd.py"], 
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = find_packages(exclude=["test", "docs", "_trial_temp", "miville.py"]),
    include_package_data = True,    # include everything in source control
    # ...but exclude Makefile.am from all packages
    exclude_package_data = { '': ['Makefile.am'] },
    )
# ---------------------------
#test_suite='nose.collector',
#package_data = {
#    "":["*.ttf", "*.rst", "*.png", "*.jpg", "*.css", "*.xml", "*.js", "*/*/*/*/*/*/*.css", "*/*/*/*/*.css"]
#}
