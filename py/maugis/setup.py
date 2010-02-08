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
from scenic import configure

__version__ = configure.VERSION

packages = [
    "scenic",
    "scenic.test",
    "scenic.devices",
    "scenic.data",
    "scenic.data.locale",
    "scenic.data.locale.en_US",
    "scenic.data.locale.en_US.LC_MESSAGES",
    "scenic.data.locale.fr_CA",
    "scenic.data.locale.fr_CA.LC_MESSAGES"
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
    install_requires = [], #'twisted'], 
    scripts = ["scripts/scenic"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = packages,
    package_data = {
        "":["*.png", "*.jpg", '*.txt', '*.glade', '*.pot', '*.h', '*.po', '*.mo']
    }
    )
