#!/usr/bin/env python
"""
Installation script
-------------------

Usage::
  python setup.py build
  sudo python setup.py install --prefix=/usr/local

For developpers::
  sudo python setup.py develop --prefix=/usr/local
  sudo python setup.py develop --prefix=/usr/local --uninstall

For distribution packages::
  python setup.py bdist
  python setup.py bdist_egg
"""
from setuptools import setup

__version__ = "0.1.0"

packages = [
    'propulseart',
    ]

setup(
    name = "scenic",
    version = __version__,
    author = "SAT",
    author_email = "info@sat.qc.ca",
    url = "http://www.sat.qc.ca",
    description = "Scenic, interface to the milhouse streaming tool.",
    long_description = """Scenic software, a component of the PropulseART project
    Requires: twisted, simplejson.""",
    install_requires = [], #'twisted'], # , 'nevow']
    # The dependency to the 'nevow' package is not specified here, since there is a bug 
    # in nevow, which installs it using easy_install even when the nevow ubuntu package is already installed. 
    # see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=475440
    # we removed twisted as well, since we prefer the user to install it himself.
    scripts = ["maugis.py"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = packages,
    package_data = {
        "":["*.png", "*.jpg", '*.txt', '.glade', '.pot', '.h', '.po', '.mo']
    }
    )
