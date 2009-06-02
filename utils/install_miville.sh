#!/bin/bash
echo This script must be run in the miville/trunk/py directory.
echo Installing miville in /usr/local...
python setup.py clean
python setup.py build
sudo python setup.py install --prefix=/usr/local
