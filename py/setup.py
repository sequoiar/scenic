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
    "scenic.data.locale.fr_CA.LC_MESSAGES",
    'rtpmidi',
    'rtpmidi.protocols',
    'rtpmidi.protocols.rtp',
    'rtpmidi.engines',
    'rtpmidi.engines.midi',
    ]

setup(
    name = "scenic",
    version = __version__,
    author = "SAT",
    author_email = "info@sat.qc.ca",
    url = "http://svn.sat.qc.ca/trac/scenic",
    description = "Scenic audio/video streaming tool for GNU/Linux.",
    long_description = """Scenic is a graphical user interface for the milhouse streaming tool software. It is a component of the PropulseART project initiated from by the Society for Arts and Technology. (SAT)""",
    install_requires = [], # twisted, simplejson 
    scripts = ["scripts/scenic", "midistream"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    packages = packages,
    package_data = {
        "": ["*.png", "*.jpg", '*.txt', '*.glade', '*.pot', '*.h', '*.po', '*.mo']
        }
    )
