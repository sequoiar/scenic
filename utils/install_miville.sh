#!/bin/bash
echo "This script must be run in the miville/trunk/py directory."
echo "Installing miville in /usr/local..."
echo "python setup.py clean"
python setup.py clean
echo "python setup.py build"
python setup.py build
echo "sudo python setup.py install --prefix=/usr/local"
sudo python setup.py install --prefix=/usr/local
