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
"""
from setuptools import find_packages
from setuptools import setup

setup(
    name = "miville",
    version = "0.1.3-a2",
    author = "SAT",
    author_email = "info@sat.qc.ca",
    url = "http://www.sat.qc.ca",
    description = "Miville software, a component of Telesceno, PropulseART",
    long_description = """PropulseART 
    Requires: twisted, nevow.""",
    install_requires = ['twisted', 'nevow'], 
    # scripts = ["miville.py"], #, "osc_send.py", "osc_receive.py"],
    license = "GPL",
    platforms = ["any"],
    zip_safe = False,
    #packages = ['miville'],
    packages = find_packages(exclude=["test/*", "miville.py"]),
    package_data = {
        "":["*.rst", "*.png", "*.jpg", ".css", ".js", ".xml", 'txt']
    }
    )

#test_suite='nose.collector',
    
