#!/usr/bin/env python
"""
Miville Installation script
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
from setuptools import find_packages
from setuptools import setup
__version__ = "0.1.3-a"

# how I generated that list : 
#packages = find_packages(exclude=["test", "miville.py"]),
#print 'PACKAGES:', packages

packages = [
    'miville',
    'miville.connectors',
    'miville.utils',
    'miville.ui',
    'miville.devices',
    'miville.protocols',
    'miville.engines',
    'miville.connectors.basic',
    'miville.connectors.sip',
    'miville.ui.cli',
    'miville.ui.web',
    'miville.ui.web.pages',
    'miville.ui.web.templates',
    'miville.ui.web.js',
    'miville.ui.web.img',
    'miville.ui.web.css',
    'miville.ui.web.widgets',
    'miville.ui.web.templates.default',
    'miville.ui.web.widgets.medias',
    'miville.ui.web.templates.default.js',
    'miville.ui.web.templates.default.xml',
    'miville.ui.web.templates.default.img',
    'miville.ui.web.templates.default.css',
    ]


setup(
    name = "miville",
    version = __version__,
    author = "SAT",
    author_email = "info@sat.qc.ca",
    url = "http://www.sat.qc.ca",
    description = "Miville, interface to the milhouse streaming tool.",
    long_description = """Miville software, a component of the PropulseART project
    Requires: twisted, nevow.""",
    install_requires = ['twisted'], # , 'nevow']
    # The dependency to the 'nevow' package is not specified here, since there is a bug 
    # in nevow, which installs it using easy_install even when the nevow ubuntu package is already installed. 
    # see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=475440
    scripts = ["restart_jackd.py", "mivilled"], #, "miville.py"
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    #packages = ['miville'],
    packages = packages,
    package_data = {
        "":["*.rst", "*.png", "*.jpg", "*.css", "*.js", "*.xml", '*.txt', 'off']
    }
    )

#test_suite='nose.collector',
    
